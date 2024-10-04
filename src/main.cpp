/**
 * @file main.cpp
 * @brief Main file containing setup and tasks for the application.
 */

#include "main.h"

// global variables
g_config_t g_config;
g_vars_t g_vars = {
  .state = STATE_INIT,
  .selection = 0,
  .selection_max = SELECTION_INIT_MAX,

  .confirm = false,
  .refresh = false,

  .wifi_status = WL_IDLE_STATUS,
  .wifi_mode = WIFI_MODE_NULL,
  // TODO gprs
  .gprs = 0,
  .electricity = false,

  .pin = "",
  .attempts = 0,
};

// -------------------------------------------------------------------------------------------------------------
/* ASYNC EVENT CONTROLL */
void keypadEvent(char key) {
  switch (g_vars.state) {
    case STATE_INIT:
      keyFxPressInit(key, &g_vars);
      break;

    case STATE_SETUP:
      keyFxPressSetup(key, &g_vars);
      break;

    case STATE_SETUP_AP:
      rebootESP();
      break;

    case STATE_ALARM_IDLE:
      keyFxPressAlarmIdle(key, &g_vars);
      break;
    
    case STATE_TEST_IDLE:
      keyFxPressTestIdle(key, &g_vars);
      break;

    case STATE_ALARM_LOCK_ENTER_PIN:
      keyFxPressAlarmLockEnterPin(key, &g_vars);
      break;

    case STATE_TEST_LOCK_ENTER_PIN:
      keyFxPressTestLockEnterPin(key, &g_vars);
      break;

    case STATE_ALARM_UNLOCK_ENTER_PIN:
      keyFxPressAlarmUnlockEnterPin(key, &g_vars);
      break;

    case STATE_TEST_UNLOCK_ENTER_PIN:
      keyFxPressTestUnlockEnterPin(key, &g_vars);
      break;

    case STATE_ALARM_CHANGE_ENTER_PIN1:
      keyFxPressAlarmChangeEnterPin1(key, &g_vars);
      break;

    case STATE_TEST_CHANGE_ENTER_PIN1:
      keyFxPressTestChangeEnterPin1(key, &g_vars);
      break;

    case STATE_ALARM_CHANGE_ENTER_PIN2:
      keyFxPressAlarmChangeEnterPin2(key, &g_vars);
      break;

    case STATE_TEST_CHANGE_ENTER_PIN2:
      keyFxPressTestChangeEnterPin2(key, &g_vars);
      break;

    case STATE_ALARM_CHANGE_ENTER_PIN3:
      keyFxPressAlarmChangeEnterPin3(key, &g_vars);
      break;

    case STATE_TEST_CHANGE_ENTER_PIN3:
      keyFxPressTestChangeEnterPin3(key, &g_vars);
      break;
    
    default:
      break;
  }
  g_vars.refresh = true;
}

// -------------------------------------------------------------------------------------------------------------
/* MAIN APPLICATION SETUP */
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  SPI.begin();

  // init file system
  if (!LittleFS.begin()) {
      Serial.print("\033[1;31m");
      Serial.printf("[setup]: Failed to mount file system!\n");
      Serial.print("\033[1;39m");
      for(;;) {delay(1000);}
  }

  // clear esp memory from previous app run
  LittleFS.remove(LOG_FILE);
  LittleFS.remove(LOG_FILE_OLD);
  Serial.println();
  esplogI("[setup]: ESP Started\n\n");

  // load configuration
  loadConfig(&g_config, CONFIG_FILE);
  esplogI("[setup]: Config:\n - ssid: %s\n - pswd: %s\n - ip: %s\n - gtw: %s\n - sbnt: %s\n - countdown: %d\n", g_config.wifi_ssid, g_config.wifi_pswd.c_str(), g_config.wifi_ip.c_str(), g_config.wifi_gtw.c_str(), g_config.wifi_sbnt.c_str(), g_config.alarm_countdown_s);

  // init keypad
  if (!keypad.begin()) {
    esplogE("[setup]: Failed to initialise keypad! Rebooting...\n");
  }

  // init rfid
  mfrc522.PCD_Init(RFID_CS_PIN);
  esplogI("[setup]: RFID reader: ");
  mfrc522.PCD_DumpVersionToSerial();

  // start tasks
  xTaskCreate(rtosMenu, "menu", 8192, NULL, 2, &handleTaskMenu);
  xTaskCreate(rtosKeypad, "keypad", 8192, NULL, 1, &handleTaskKeypad);
  xTaskCreatePinnedToCore(rtosNet, "net", 8192, NULL, 1, &handleTaskNet, CONFIG_ARDUINO_RUNNING_CORE);
  esplogI("[setup]: All tasks created successfully!\n");
  esplogI("--------------------------------------------------------------------------------\n");
  // TODO create Display task
}

// -------------------------------------------------------------------------------------------------------------
/* LOOP FUNCTION */
void loop() {}

// -------------------------------------------------------------------------------------------------------------
/* STATE AUTO REFRESHER */
void rtosMenuRefresh(void* parameters) {
  for(;;) {
    g_vars.refresh = true;
    g_vars.confirm = true;
  }
}

// -------------------------------------------------------------------------------------------------------------
/* KEYPAD SCANNER */
void rtosKeypad(void* parameters) {
  char keymap[] = "741#    8520    963*    ABCD                                    NF";
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
        keypadEvent(key);
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
    vTaskDelay(500 / portTICK_PERIOD_MS);
    if (!mfrc522.PICC_IsNewCardPresent()) {
      continue;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
      continue;
    }

    // card detected
    String rfid_card = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
      rfid_card.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      rfid_card.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    rfid_card.toUpperCase();
    esplogI("[rfid]: Card detected! UID: %s\n", rfid_card.c_str());

    // TODO add rfid authorisation UID from file
    // if (g_vars.state == STATE_FOR_RFID_ADD) {}
    // TODO delete rfid authorisation UID from file
    // if (g_vars.state == STATE_FOR_RFID_DEL) {}

    // TODO fx that authorise RFID from file
    if (true) {
      esplogI("Card was authorised!\n");
      switch (g_vars.state) {
        case STATE_ALARM_LOCK_ENTER_PIN:
        case STATE_TEST_LOCK_ENTER_PIN:
        case STATE_ALARM_UNLOCK_ENTER_PIN:
        case STATE_TEST_UNLOCK_ENTER_PIN:
        case STATE_ALARM_CHANGE_ENTER_PIN1:
        case STATE_TEST_CHANGE_ENTER_PIN1:
          break;

        default:
          break;
      }
    } else {
      esplogI("Card was not authorised!\n");
    }
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
/* MAIN APPLICATION STUCTURE */
void rtosMenu(void* parameters) {
  unsigned long lock_time = 0;
  for (;;) {
    if (g_vars.refresh) {
      unsigned long curr_time = millis();
      if (g_vars.confirm) {
        switch (g_vars.state) {
          // ***************************
          // startup menu
          case STATE_INIT:
            switch (g_vars.selection) {
              case SELECTION_INIT_SETUP:
                g_vars.state = STATE_SETUP;
                g_vars.selection_max = SELECTION_SETUP_MAX;
                break;
              case SELECTION_INIT_ALARM:
                g_vars.state = STATE_ALARM_IDLE;
                g_vars.selection_max = SELECTION_ALARM_IDLE_MAX;
                break;
              case SELECTION_INIT_TEST:
                g_vars.state = STATE_TEST_IDLE;
                g_vars.selection_max = SELECTION_TEST_IDLE_MAX;
                break;
              case SELECTION_INIT_REBOOT:
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
                g_vars.state = STATE_SETUP_AP;
                g_vars.selection_max = 0;
                xTaskCreatePinnedToCore(rtosSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
                vTaskSuspend(NULL);
                break;
              case SELECTION_SETUP_HARD_RESET:
                esplogI("[menu]: Hard reseting IoT Alarm! Re-creating configuration data.\n");
                LittleFS.remove(CONFIG_FILE);
                LittleFS.remove(LOG_FILE);
                LittleFS.remove(LOG_FILE_OLD);
                rebootESP();
                break;
              case SELECTION_SETUP_RETURN:
                g_vars.state = STATE_INIT;
                g_vars.selection_max = SELECTION_INIT_MAX;
                break;
            }
            break;

          // ***************************
          // alarm menu
          case STATE_ALARM_IDLE:
            switch (g_vars.selection) {
              case SELECTION_ALARM_IDLE_LOCK:
                if (existsPassword()) {
                  g_vars.state = STATE_ALARM_LOCK_ENTER_PIN;
                  g_vars.selection_max = 0;
                  xTaskCreate(rtosRfid, "rfid", 2048, NULL, 3, &handleTaskRfid);
                } else {
                  g_vars.state = STATE_ALARM_CHANGE_ENTER_PIN2;
                  g_vars.selection_max = 0;
                }
                break;
              case SELECTION_ALARM_IDLE_CHANGE_PASSWORD:
                if (existsPassword()) {
                  g_vars.state = STATE_ALARM_CHANGE_ENTER_PIN1;
                  g_vars.selection_max = 0;
                  xTaskCreate(rtosRfid, "rfid", 2048, NULL, 3, &handleTaskRfid);
                } else {
                  g_vars.state = STATE_ALARM_CHANGE_ENTER_PIN2;
                  g_vars.selection_max = 0;
                }
                break;
              case SELECTION_ALARM_IDLE_RETURN:
                g_vars.state = STATE_INIT;
                g_vars.selection_max = SELECTION_INIT_MAX;
                break;
              case SELECTION_ALARM_IDLE_REBOOT:
                rebootESP();
                break;
            }
            break;

          // test menu
          case STATE_TEST_IDLE:
            switch (g_vars.selection) {
              case SELECTION_TEST_IDLE_LOCK:
                if (existsPassword()) {
                  g_vars.state = STATE_TEST_LOCK_ENTER_PIN;
                  g_vars.selection_max = 0;
                  xTaskCreate(rtosRfid, "rfid", 2048, NULL, 3, &handleTaskRfid);
                } else {
                  g_vars.state = STATE_TEST_CHANGE_ENTER_PIN2;
                  g_vars.selection_max = 0;
                }
                break;
              case SELECTION_TEST_IDLE_CHANGE_PASSWORD:
                if (existsPassword()) {
                  g_vars.state = STATE_TEST_CHANGE_ENTER_PIN1;
                  g_vars.selection_max = 0;
                  xTaskCreate(rtosRfid, "rfid", 2048, NULL, 3, &handleTaskRfid);
                } else {
                  g_vars.state = STATE_TEST_CHANGE_ENTER_PIN2;
                  g_vars.selection_max = 0;
                }
                break;
              case SELECTION_TEST_IDLE_RETURN:
                g_vars.state = STATE_INIT;
                g_vars.selection_max = SELECTION_INIT_MAX;
                break;
              case SELECTION_TEST_IDLE_REBOOT:
                rebootESP();
                break;
            }
            break;

          // ***************************
          // alarm cooldown state
          case STATE_ALARM_C:
            if (curr_time >= lock_time + g_config.alarm_countdown_s*1000) {
              lock_time = 0;
              g_vars.state = STATE_ALARM_OK;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskMenuRefresh);
            }
            break;

          // test cooldown state
          case STATE_TEST_C:
            if (curr_time >= lock_time + g_config.alarm_countdown_s*1000) {
              lock_time = 0;
              g_vars.state = STATE_TEST_OK;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskMenuRefresh);
            }
            break;

          // ***************************
          // alarm ok state; alarm warning state; alarm emengency state TODO
          case STATE_ALARM_OK:
          case STATE_ALARM_W:
          case STATE_ALARM_E:
            // vTaskSuspend(NULL);
            // xTaskCreate(rtosRfid, "rfid", 2048, NULL, 3, &handleTaskRfid);
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
            // xTaskCreate(rtosRfid, "rfid", 2048, NULL, 3, &handleTaskRfid);
            // enter pin
            // check if pin is correct
            // g_vars.state = STATE_TEST_IDLE;
            // g_vars.selection_max = SELECTION_TEST_IDLE_MAX;
            break;

          // ***************************
          // state for locking the alarm
          case STATE_ALARM_LOCK_ENTER_PIN:
            // pin already entered
            // check if pin is correct
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              lock_time = millis();
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_ALARM_C;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskRfid);
              xTaskCreate(rtosMenuRefresh, "menurefresh", 1024, NULL, 2, &handleTaskMenuRefresh);
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_ALARM_IDLE;
              g_vars.selection_max = SELECTION_ALARM_IDLE_MAX;
              vTaskDelete(handleTaskRfid);
            }
            break;

          // state for locking the test
          case STATE_TEST_LOCK_ENTER_PIN:
            // pin already entered
            // check if pin is correct
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              lock_time = millis();
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_TEST_C;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskRfid);
              xTaskCreate(rtosMenuRefresh, "menurefresh", 1024, NULL, 2, &handleTaskMenuRefresh);
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_TEST_IDLE;
              g_vars.selection_max = SELECTION_TEST_IDLE_MAX;
              vTaskDelete(handleTaskRfid);
            }
            break;

          // state for unlocking the alarm
          case STATE_ALARM_UNLOCK_ENTER_PIN:
            // pin already entered
            // check if pin is correct
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_ALARM_IDLE;
              g_vars.selection_max = SELECTION_ALARM_IDLE_MAX;
              vTaskDelete(handleTaskRfid);
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_ALARM_UNLOCK_ENTER_PIN;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskRfid);
            }
            break;

          // state for unlocking the test
          case STATE_TEST_UNLOCK_ENTER_PIN:
            // pin already entered
            // check if pin is correct
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_TEST_IDLE;
              g_vars.selection_max = SELECTION_TEST_IDLE_MAX;
              vTaskDelete(handleTaskRfid);
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_TEST_UNLOCK_ENTER_PIN;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskRfid);
            }
            break;

          // state for unlocking the pasword change feature
          case STATE_ALARM_CHANGE_ENTER_PIN1:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_ALARM_CHANGE_ENTER_PIN2;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskRfid);
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_ALARM_IDLE;
              g_vars.selection_max = SELECTION_ALARM_IDLE_MAX;
              vTaskDelete(handleTaskRfid);
            }
            break;

          // state for unlocking the pasword change feature
          case STATE_TEST_CHANGE_ENTER_PIN1:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_TEST_CHANGE_ENTER_PIN2;
              g_vars.selection_max = 0;
              vTaskDelete(handleTaskRfid);
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_TEST_IDLE;
              g_vars.selection_max = SELECTION_TEST_IDLE_MAX;
              vTaskDelete(handleTaskRfid);
            }
            break;

          // state for changing the password
          case STATE_ALARM_CHANGE_ENTER_PIN2:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            g_vars.state = STATE_ALARM_CHANGE_ENTER_PIN3;
            g_vars.selection_max = 0;
            break;

          // state for changing the password
          case STATE_TEST_CHANGE_ENTER_PIN2:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            g_vars.state = STATE_TEST_CHANGE_ENTER_PIN3;
            g_vars.selection_max = 0;
            break;

          // state for confirmation of the password change
          case STATE_ALARM_CHANGE_ENTER_PIN3:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (saveNewPassword(g_vars.pin)) {
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_ALARM_IDLE;
              g_vars.selection_max = SELECTION_ALARM_IDLE_MAX;
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_ALARM_CHANGE_ENTER_PIN2;
              g_vars.selection_max = 0;
            }
            break;

          // state for confirmation of the password change
          case STATE_TEST_CHANGE_ENTER_PIN3:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (saveNewPassword(g_vars.pin)) {
              g_vars.pin = "";
              g_vars.attempts = 0;
              g_vars.state = STATE_TEST_IDLE;
              g_vars.selection_max = SELECTION_TEST_IDLE_MAX;
            } else {
              g_vars.pin = "";
              g_vars.attempts++;
              g_vars.state = STATE_TEST_CHANGE_ENTER_PIN2;
              g_vars.selection_max = 0;
            }
            break;
        
          default:
            break;
        }
        g_vars.selection = 0;
        g_vars.confirm = false;
      }
      // TODO: refresh display
      esplogI("[menu]: State: %s  |  Selection: %s\n", getStateName(g_vars.state), getSelectionName(g_vars.state, g_vars.selection));
      g_vars.refresh = false;

    } else {
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
}
