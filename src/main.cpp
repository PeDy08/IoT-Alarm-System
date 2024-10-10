/**
 * @file main.cpp
 * @brief Main file containing setup and tasks for the application.
 */

#include "main.h"

// global variables
g_config_t g_config;
g_vars_t g_vars = {
  .state = STATE_INIT,
  .state_prev = STATE_MAX,

  .selection = 0,
  .selection_prev = 0,

  .selection_max = SELECTION_INIT_MAX,
  .selection_max_prev = 0,

  .confirm = false,
  .abort = false,
  .refresh = true,

  .wifi_status = WL_IDLE_STATUS,
  .wifi_mode = WIFI_MODE_NULL,
  // TODO gprs
  .gprs = 0,
  .electricity = false,

  .pin = "",
  .attempts = 0,
  .time = 0,
};

// -------------------------------------------------------------------------------------------------------------
/* MAIN APPLICATION SETUP */
void setup() {
  Serial.begin(115200);
  Wire.begin();
  // Wire.setClock(400000);
  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI);

  // init SD card
  if (!SD.begin(SD_CS_PIN)) {
      Serial.print("\033[1;31m");
      Serial.printf("[setup]: Failed to mount SD card file system!\n");
      Serial.print("\033[1;39m");
      for(;;) {delay(1000);}
  }

  while (SD.cardType() == CARD_NONE) {
      Serial.print("\033[1;31m");
      Serial.printf("[setup]: No SD card was inserted! Please insert card!\n");
      Serial.print("\033[1;39m");
      delay(1000);
  }

  // clear esp memory from previous app run
  SD.remove(LOG_FILE);
  SD.remove(LOG_FILE_OLD);
  Serial.println();
  esplogI("[setup]: ESP Started\n\n");

  // load configuration
  loadConfig(&g_config, CONFIG_FILE);
  esplogI("[setup]: Config:\n - ssid: %s\n - pswd: %s\n - ip: %s\n - gtw: %s\n - sbnt: %s\n - countdown: %d\n", g_config.wifi_ssid, g_config.wifi_pswd.c_str(), g_config.wifi_ip.c_str(), g_config.wifi_gtw.c_str(), g_config.wifi_sbnt.c_str(), g_config.alarm_countdown_s);

  // init display
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("IoT Alarm System");
  lcd.setCursor(0, 1);
  lcd.print("version: 0.1");
  lcd.setCursor(0, 2);
  lcd.print("");
  lcd.setCursor(0, 3);
  lcd.print("Petr Zerzan");

  // init GSM module
  if (!initSerialGSM()) {
    esplogE("[setup]: Failed to initialise GSM module!\n");
  }

  // init keypad
  if (!keypad.begin()) {
    esplogE("[setup]: Failed to initialise keypad! Rebooting...\n");
  }

  // init rfid
  mfrc522.PCD_Init(RFID_CS_PIN, RFID_RST_PIN);
  esplogI("[setup]: RFID reader: ");
  mfrc522.PCD_DumpVersionToSerial();

  // start main tasks
  xTaskCreate(rtosMenu, "menu", 16384, NULL, 2, &handleTaskMenu);
  xTaskCreate(rtosKeypad, "keypad", 8192, NULL, 1, &handleTaskKeypad);
  xTaskCreate(rtosRfid, "rfid", 4096, NULL, 3, &handleTaskRfid);
  xTaskCreate(rtosGsm, "gsm", 8192, NULL, 2, &handleTaskGsm);
  xTaskCreatePinnedToCore(rtosNet, "net", 8192, NULL, 1, &handleTaskNet, CONFIG_ARDUINO_RUNNING_CORE);

  // start and suspend refresher tasks
  xTaskCreate(rtosMenuRefresh, "menurefresh", 1024, NULL, 2, &handleTaskMenuRefresh);
  xTaskCreate(rtosRfidRefresh, "rfidrefresh", 1024, NULL, 2, &handleTaskRfidRefresh);
  vTaskSuspend(handleTaskMenuRefresh);
  vTaskSuspend(handleTaskRfidRefresh);

  esplogI("[setup]: All tasks created successfully!\n");
  esplogI("--------------------------------------------------------------------------------\n");
}

// -------------------------------------------------------------------------------------------------------------
/* LOOP FUNCTION */
void loop() {
  vTaskDelay(10 * 60 * 1000 / portTICK_PERIOD_MS);
}

// -------------------------------------------------------------------------------------------------------------
/* STATE AUTO REFRESHER */
void rtosMenuRefresh(void* parameters) {
  for(;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    g_vars.refresh = true;
    g_vars.confirm = true;
  }
}

// -------------------------------------------------------------------------------------------------------------
/* RFID AUTO REFRESHER */
void rtosRfidRefresh(void* parameters) {
  for(;;) {
    vTaskDelay(250 / portTICK_PERIOD_MS);
    xTaskNotifyGive(handleTaskRfid);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* KEYPAD SCANNER */
void rtosKeypad(void* parameters) {
  char keymap[] = "123A    456B    789C    *0#D                                    NF";
  char key = '\0';
  char key_last = '\0';
  unsigned long key_t = 0;
  unsigned long key_last_t = 0;

  keypad.loadKeyMap(keymap);
  for(;;) {
    key = keypad.getChar();
    key_t = millis();

    if (key != key_last) {
      if (isValidChar(key)) {
        keypadEvent(&g_vars, key);
        g_vars.refresh = true;
      }

      key_last = key;
      key_last_t = key_t;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* RFID READER HANDLER */
void rtosRfid(void* parameters) {
  for (;;) {
    // wait for notification from refresher
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (!mfrc522.PICC_IsNewCardPresent()) {
      continue;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
      continue;
    }

    // card detected
    String rfid_card = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      rfid_card.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      rfid_card.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    rfid_card.toUpperCase();
    esplogI("[rfid]: Card detected! UID: %s\n", rfid_card.c_str());

    if (g_vars.state == STATE_SETUP_RFID_ADD) {
      esplogI("[rfid]: Adding new UID: %s\n", rfid_card.c_str());
      addRfid(rfid_card);
      rfidScreenA(&g_vars, rfid_card.c_str());
      setState(STATE_SETUP, -1, SELECTION_SETUP_MAX);
    }

    else if (g_vars.state == STATE_SETUP_RFID_DEL) {
      esplogI("[rfid]: Deleting UID: %s\n", rfid_card.c_str());
      delRfid(rfid_card);
      rfidScreenD(&g_vars, rfid_card.c_str());
      setState(STATE_SETUP, -1, SELECTION_SETUP_MAX);
    }

    else if (checkRfid(rfid_card)) {
      esplogI("[rfid]: Card was authorised!\n");
      switch (g_vars.state) {
        case STATE_SETUP_RFID_ADD_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_SETUP_RFID_ADD, -1, 0, "", 0);
          g_vars.refresh = true;
          continue;
          break;

        case STATE_SETUP_RFID_DEL_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_SETUP_RFID_DEL, -1, 0, "", 0);
          g_vars.refresh = true;
          continue;
          break;

        case STATE_SETUP_HARD_RESET_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_SETUP_HARD_RESET, -1, 0, "", 0);
          break;

        case STATE_ALARM_LOCK_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_ALARM_C, -1, 0, "", 0);
          break;

        case STATE_TEST_LOCK_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_TEST_C, -1, 0, "", 0);
          break;

        case STATE_ALARM_UNLOCK_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_ALARM_IDLE, -1, SELECTION_ALARM_IDLE_MAX, "", 0);
          break;

        case STATE_TEST_UNLOCK_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_TEST_IDLE, -1, SELECTION_TEST_IDLE_MAX, "", 0);
          break;

        case STATE_ALARM_CHANGE_ENTER_PIN1:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_ALARM_CHANGE_ENTER_PIN2, -1, 0, "", 0);
          break;

        case STATE_TEST_CHANGE_ENTER_PIN1:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_TEST_CHANGE_ENTER_PIN2, -1, 0, "", 0);
          break;

        case STATE_SETUP_PIN1:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_SETUP_PIN2, -1, 0, "", 0);
          break;

        case STATE_SETUP_RFID_CHECK:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_SETUP, -1, SELECTION_SETUP_MAX, "", 0);
          break;

        default:
          esplogW("[rfid]: RFID task was running in unknown state! Terminating...\n");
          break;
      }
      vTaskSuspend(handleTaskRfidRefresh);
    } else {
      esplogI("[rfid]: Card was not authorised!\n");
      rfidScreenE(&g_vars, rfid_card.c_str());
    }
    g_vars.refresh = true;
  }
}


// -------------------------------------------------------------------------------------------------------------
/* WIFI AP SETUP HANDLER */
void rtosSetup(void* parameters) {
  esplogI("[wifi]: WiFi setup mode is active!\n");
  // vTaskDelete(handleTaskKeypad); <- for possibility to reboot esp by pressing any key
  vTaskDelete(handleTaskMenu);
  vTaskDelete(handleTaskNet);
  g_vars.wifi_mode = WIFI_MODE_AP;

  startWifiSetupMode();
  for (;;) {vTaskDelay(100 / portTICK_PERIOD_MS);}
}

// -------------------------------------------------------------------------------------------------------------
/* WIFI/INTERNET HANDLER */
void rtosNet(void* parameters) {
  // TODO think about replacing "while(true) and wifi.state()"" with "esp wifi events"
  // dont start the WiFi task if config has not been set
  if (g_config.wifi_ssid == "\0" || g_config.wifi_pswd == "\0") {
    // task will only continue if setup state is triggered
    esplogW("[wifi]: WiFi will start only after configuration is done!\n");
    vTaskSuspend(NULL);
  }

  startWiFiServerMode(&g_vars, &g_config);
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  for(;;) {
    // check wifi state
    g_vars.wifi_status = WiFi.status();
    switch (g_vars.wifi_status) {
      // WiFi is connected
      case WL_CONNECTED:
        esplogI("[wifi]: WiFi periodic check passed!\n - status: WL_CONNECTED\n - rssi: %d\n - ip: %s\n", WiFi.RSSI(), WiFi.localIP().toString().c_str());
        // TODO is it good idea? what if robber shut down internet connection on purpose?
        vTaskDelay(5 * 60 * 1000 / portTICK_PERIOD_MS);
        break;

      // WiFi SSID not available
      case WL_NO_SSID_AVAIL:
        esplogW("[wifi]: WiFi connection failed! WiFi SSID was not found! Please open setup and reconfigure!\n");
        vTaskSuspend(NULL);
        break;

      // WiFi connection was unsuccessfull
      case WL_CONNECT_FAILED:
        // task will only continue if setup state is triggered
        esplogW("[wifi]: WiFi connection failed! This could be due to wrong password, bad connection or router error. Please reboot or open setup and reconfigure!\n");
        vTaskSuspend(NULL);
        break;

      // WiFi connection has been lost
      case WL_CONNECTION_LOST:
        esplogW("[wifi]: WiFi conection has been lost! Trying to reconect.\n");
        break;

      // case WL_DISCONNECTED: // <-- this should never happen!
      //   break;

      // WL_IDLE_STATUS, WL_SCAN_COMPLETED, (WL_DISCONNECTED)
      default:
        esplogW("[wifi]: Unexpected WiFi status!\n - status: %d\n", g_vars.wifi_status);
        break;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* GSM COMMUNICATION HANDELER */
void rtosGsm(void* parameters) {
  // TODO
}

// -------------------------------------------------------------------------------------------------------------
/* MAIN APPLICATION STUCTURE */
void rtosMenu(void* parameters) {
  unsigned long lock_time = 0;
  vTaskDelay(200 / portTICK_PERIOD_MS);
  for (;;) {
    if (g_vars.refresh) {
      unsigned long curr_time = millis();
      if (g_vars.abort && g_vars.state_prev != STATE_MAX) {
        switch (g_vars.state_prev) {
          case STATE_INIT:
          case STATE_SETUP:
          case STATE_SETUP_PIN2:
          case STATE_ALARM_IDLE:
          case STATE_ALARM_CHANGE_ENTER_PIN2:
          case STATE_TEST_IDLE:
          case STATE_TEST_CHANGE_ENTER_PIN2:
            setState(g_vars.state_prev, g_vars.selection_prev, g_vars.selection_max_prev, "", 0);
            vTaskSuspend(handleTaskMenuRefresh);
            vTaskSuspend(handleTaskRfidRefresh);
            break;
          
          default:
            break;
        }
        g_vars.abort = false;
      }

      else if (g_vars.confirm) {
        switch (g_vars.state) {
          // ***************************
          // startup menu
          case STATE_INIT:
            switch (g_vars.selection) {
              case SELECTION_INIT_SETUP:
                setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                break;
              case SELECTION_INIT_ALARM:
                setState(STATE_ALARM_IDLE, 0, SELECTION_ALARM_IDLE_MAX);
                break;
              case SELECTION_INIT_TEST:
                setState(STATE_TEST_IDLE, 0, SELECTION_TEST_IDLE_MAX);
                break;
              case SELECTION_INIT_REBOOT:
                loadScreen(&g_vars, &g_config, true);
                rebootESP();
                break;
            }
            break;
          
          // ***************************
          // configuration menu
          case STATE_SETUP:
            switch (g_vars.selection) {
              case SELECTION_SETUP_START_STA:
                esplogI("[menu]: Starting WiFi Setup Mode!\n");
                setState(STATE_SETUP_AP, 0, 0);
                loadScreen(&g_vars, &g_config);
                xTaskCreatePinnedToCore(rtosSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
                vTaskSuspend(NULL);
                break;
              case SELECTION_SETUP_SET_PIN:
                if (existsPassword()) {
                  setState(STATE_SETUP_PIN1, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  setState(STATE_SETUP_PIN2, 0, 0);
                }
                break;
              case SELECTION_SETUP_ADD_RFID:
                if (existsPassword()) {
                  setState(STATE_SETUP_RFID_ADD_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  esplogW("[menu]: Before setting RFID authentication, please set PIN!\n");
                  setState(STATE_SETUP_PIN2, 0, 0);
                }
                break;
              case SELECTION_SETUP_DEL_RFID:
                if (!existsRfid()) {
                  esplogI("[menu]: Trying to delete RFID, but any was set yet!\n");
                  setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                  break;
                }
                if (existsPassword()) {
                  setState(STATE_SETUP_RFID_DEL_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  esplogW("[menu]: Unexpected behaviour! There should not be RFID set when no PIN was set yet!\n");
                  setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                }
                break;
              case SELECTION_SETUP_CHECK_RFID:
                if (!existsRfid()) {
                  esplogI("[menu]: Trying to check RFID, but any was set yet!\n");
                  setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                  break;
                }
                if (existsPassword()) {
                  setState(STATE_SETUP_RFID_CHECK, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  esplogW("[menu]: Unexpected behaviour! There should not be RFID set when no PIN was set yet!\n");
                  setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                }                break;
              case SELECTION_SETUP_HARD_RESET:
                if (existsPassword()) {
                  setState(STATE_SETUP_HARD_RESET_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  setState(STATE_SETUP_HARD_RESET, 0, 0);
                }
                break;
              case SELECTION_SETUP_RETURN:
                setState(STATE_INIT, 0, SELECTION_INIT_MAX);
                break;
            }
            break;

          case STATE_SETUP_RFID_ADD_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_SETUP_RFID_ADD, 0, 0, "", 0);
              vTaskResume(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_SETUP, 0, SELECTION_SETUP_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          case STATE_SETUP_RFID_DEL_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_SETUP_RFID_DEL, 0, 0, "", 0);
              vTaskResume(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_SETUP, 0, SELECTION_SETUP_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          case STATE_SETUP_HARD_RESET_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_SETUP_HARD_RESET, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_SETUP, 0, SELECTION_SETUP_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          case STATE_SETUP_HARD_RESET:
            esplogI("[menu]: Hard reseting IoT Alarm! Re-creating configuration data.\n");
            SD.remove(CONFIG_FILE);
            SD.remove(LOG_FILE);
            SD.remove(LOG_FILE_OLD);
            SD.remove(LOCK_FILE);
            SD.remove(RFID_FILE);
            loadScreen(&g_vars, &g_config, true);
            rebootESP();
            break;

          case STATE_SETUP_RFID_ADD:
          case STATE_SETUP_RFID_DEL:
          case STATE_SETUP_RFID_CHECK:
            setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
            vTaskSuspend(handleTaskRfidRefresh);
            break;

          // ***************************
          // alarm menu
          case STATE_ALARM_IDLE:
            switch (g_vars.selection) {
              case SELECTION_ALARM_IDLE_LOCK:
                if (existsPassword()) {
                  setState(STATE_ALARM_LOCK_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  setState(STATE_ALARM_CHANGE_ENTER_PIN2, 0, 0);
                }
                break;
              case SELECTION_ALARM_IDLE_CHANGE_PASSWORD:
                if (existsPassword()) {
                  setState(STATE_ALARM_CHANGE_ENTER_PIN1, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  setState(STATE_ALARM_CHANGE_ENTER_PIN2, 0, 0);
                }
                break;
              case SELECTION_ALARM_IDLE_RETURN:
                setState(STATE_INIT, 0, SELECTION_INIT_MAX);
                break;
              case SELECTION_ALARM_IDLE_REBOOT:
                loadScreen(&g_vars, &g_config, true);
                rebootESP();
                break;
            }
            break;

          // test menu
          case STATE_TEST_IDLE:
            switch (g_vars.selection) {
              case SELECTION_TEST_IDLE_LOCK:
                if (existsPassword()) {
                  setState(STATE_TEST_LOCK_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  setState(STATE_TEST_CHANGE_ENTER_PIN2, 0, 0);
                }
                break;
              case SELECTION_TEST_IDLE_CHANGE_PASSWORD:
                if (existsPassword()) {
                  setState(STATE_TEST_CHANGE_ENTER_PIN1, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  setState(STATE_TEST_CHANGE_ENTER_PIN2, 0, 0);
                }
                break;
              case SELECTION_TEST_IDLE_RETURN:
                setState(STATE_INIT, 0, SELECTION_INIT_MAX);
                break;
              case SELECTION_TEST_IDLE_REBOOT:
                loadScreen(&g_vars, &g_config, true);
                rebootESP();
                break;
            }
            break;

          // ***************************
          // alarm cooldown state
          case STATE_ALARM_C:
            if (curr_time >= lock_time + g_config.alarm_countdown_s*1000) {
              lock_time = 0;
              g_vars.time = 0;
              setState(STATE_ALARM_OK, 0, 0);
              vTaskSuspend(handleTaskMenuRefresh);
            } else {
              g_vars.refresh = false;
              g_vars.confirm = false;
              g_vars.time = curr_time - lock_time;
              loadScreen(&g_vars, &g_config);
              continue;
            }
            break;

          // test cooldown state
          case STATE_TEST_C:
            if (curr_time >= lock_time + g_config.alarm_countdown_s*1000) {
              lock_time = 0;
              g_vars.time = 0;
              setState(STATE_TEST_OK, 0, 0);
              vTaskSuspend(handleTaskMenuRefresh);
            } else {
              g_vars.refresh = false;
              g_vars.confirm = false;
              g_vars.time = curr_time - lock_time;
              loadScreen(&g_vars, &g_config);
              continue;
            }
            break;

          // ***************************
          // alarm ok state; alarm warning state; alarm emengency state TODO
          case STATE_ALARM_OK:
          case STATE_ALARM_W:
          case STATE_ALARM_E:
            // vTaskSuspend(NULL);
            // vTaskResume(handleTaskRfidRefresh);
            // enter pin
            // check if pin is correct
            // g_vars.state = STATE_ALARM_IDLE;
            // g_vars.selection_max = SELECTION_ALARM_IDLE_MAX;
            break;

          // test ok state; test warning state; test emengency state TODO
          case STATE_TEST_OK:
          case STATE_TEST_W:
          case STATE_TEST_E:
            // vTaskSuspend(NULL);
            // vTaskResume(handleTaskRfidRefresh);
            // enter pin
            // check if pin is correct
            // g_vars.state = STATE_TEST_IDLE;
            // g_vars.selection_max = SELECTION_TEST_IDLE_MAX;
            break;

          // ***************************
          // state for locking the alarm
          case STATE_ALARM_LOCK_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              lock_time = millis();
              setState(STATE_ALARM_C, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
              vTaskResume(handleTaskMenuRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_ALARM_IDLE, 0, SELECTION_ALARM_IDLE_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          // state for locking the test
          case STATE_TEST_LOCK_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              lock_time = millis();
              setState(STATE_TEST_C, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
              vTaskResume(handleTaskMenuRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_TEST_IDLE, 0, SELECTION_TEST_IDLE_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          // state for unlocking the alarm
          case STATE_ALARM_UNLOCK_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_ALARM_IDLE, 0, SELECTION_ALARM_IDLE_MAX, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_ALARM_UNLOCK_ENTER_PIN, 0, 0, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          // state for unlocking the test
          case STATE_TEST_UNLOCK_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_TEST_IDLE, 0, SELECTION_TEST_IDLE_MAX, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_TEST_UNLOCK_ENTER_PIN, 0, 0, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          // state for unlocking the pasword change feature
          case STATE_ALARM_CHANGE_ENTER_PIN1:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_ALARM_CHANGE_ENTER_PIN2, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_ALARM_IDLE, 0, SELECTION_ALARM_IDLE_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          // state for unlocking the pasword change feature
          case STATE_TEST_CHANGE_ENTER_PIN1:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_TEST_CHANGE_ENTER_PIN2, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_TEST_IDLE, 0, SELECTION_TEST_IDLE_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          // state for unlocking the pasword change feature
          case STATE_SETUP_PIN1:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_SETUP_PIN2, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_SETUP, 0, SELECTION_SETUP_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          // state for changing the password
          case STATE_ALARM_CHANGE_ENTER_PIN2:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            setState(STATE_ALARM_CHANGE_ENTER_PIN3, 0, 0);
            break;

          // state for changing the password
          case STATE_TEST_CHANGE_ENTER_PIN2:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            setState(STATE_TEST_CHANGE_ENTER_PIN3, 0, 0);
            break;

          // state for changing the password
          case STATE_SETUP_PIN2:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            setState(STATE_SETUP_PIN3, 0, 0);
            break;

          // state for confirmation of the password change
          case STATE_ALARM_CHANGE_ENTER_PIN3:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (saveNewPassword(g_vars.pin)) {
              authScreenS(&g_vars);
              setState(STATE_ALARM_IDLE, 0, SELECTION_ALARM_IDLE_MAX, "", 0);
            } else {
              authScreenE(&g_vars);
              setState(STATE_ALARM_CHANGE_ENTER_PIN2, 0, 0, "", g_vars.attempts+1);
            }
            break;

          // state for confirmation of the password change
          case STATE_TEST_CHANGE_ENTER_PIN3:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (saveNewPassword(g_vars.pin)) {
              authScreenS(&g_vars);
              setState(STATE_TEST_IDLE, 0, SELECTION_TEST_IDLE_MAX, "", 0);
            } else {
              authScreenE(&g_vars);
              setState(STATE_TEST_CHANGE_ENTER_PIN2, 0, 0, "", g_vars.attempts+1);
            }
            break;

          // state for confirmation of the password change
          case STATE_SETUP_PIN3:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (saveNewPassword(g_vars.pin)) {
              authScreenS(&g_vars);
              setState(STATE_SETUP, 0, SELECTION_SETUP_MAX, "", 0);
            } else {
              authScreenE(&g_vars);
              setState(STATE_SETUP_PIN2, 0, 0, "", g_vars.attempts+1);
            }
            break;
        
          default:
            break;
        }
        g_vars.confirm = false;
      }

      // TODO: refresh display
      loadScreen(&g_vars, &g_config);
      esplogI("[menu]: State: %s  |  Selection: %s\n", getStateText(g_vars.state), getSelectionText(g_vars.state, g_vars.selection));
      g_vars.refresh = false;

    } else {
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
}
