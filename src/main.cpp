/**
 * @file main.cpp
 * @brief Main file containing setup and tasks for the application.
 */

#include "main.h"

// global variables
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
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
  
  .wifi_strength = 1,
  .gsm_strength = 99,
  .battery_level = 0,
  .power_mode = 0,

  .datetime = 0,
  .date = "00/00/0000",
  .time = "00:00",

  .pin = "",
  .attempts = 0,
  .alarm_events = 0,
  .time_temp = 0,
};

// -------------------------------------------------------------------------------------------------------------
/* MAIN APPLICATION SETUP */
void setup() {
  Serial.begin(115200);
  Wire.begin(IIC_SDA, IIC_CLK);
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
  esplogI("[setup]: Config:\n - ssid: %s\n - pswd: %s\n - ip: %s\n - gtw: %s\n - sbnt: %s\n", g_config.wifi_ssid, g_config.wifi_pswd.c_str(), g_config.wifi_ip.c_str(), g_config.wifi_gtw.c_str(), g_config.wifi_sbnt.c_str());

  // init display EINK
#ifdef EINK
  esplogI("[setup]: Display mode: EINK\n");
  initEink();
#endif

  // init display LCD
#ifdef LCD
  esplogI("[setup]: Display mode: LCD\n");
  initLcd();
#endif

  // init keypad
  if (!keypad.begin()) {
    esplogE("[setup]: Failed to initialise keypad! Rebooting...\n");
  }

  // init GSM module
  // if (!initSerialGSM()) {
  //   esplogE("[setup]: Failed to initialise GSM module!\n");
  // }

  // init Zigbee module
  tx_buffer = (uint8_t*)malloc(TX_BUF_SIZE + 1);
  rx_buffer = (uint8_t*)malloc(RX_BUF_SIZE + 1);
  if (!initSerialZigbee()) {
    esplogE("[setup]: Failed to initialise Zigbee module!\n");
  }

  // init rfid
  mfrc522.PCD_Init(RFID_CS_PIN, RFID_RST_PIN);
  esplogI("[setup]: RFID reader: ");
  mfrc522.PCD_DumpVersionToSerial();

  // automatically start AP setup if no ssid is provided
  if (g_config.wifi_ssid == "") {
    esplogI("[setup]: No SSID has been configured, starting AP setup!\n");
    notificationScreenTemplate("No WiFi SSID configured", "Please open WiFi setup!");
    g_vars.wifi_mode = WIFI_MODE_AP;
    startWifiSetupMode();
    for (;;) {vTaskDelay(1000 / portTICK_PERIOD_MS);}
  }

  // start support tasks
  xTaskCreate(rtosKeypad, "keypad", 8192, NULL, 3, &handleTaskKeypad);
  xTaskCreate(rtosRfid, "rfid", 4096, NULL, 3, &handleTaskRfid);
  // xTaskCreate(rtosGsm, "gsm", 8192, NULL, 2, &handleTaskGsm);
  xTaskCreate(rtosZigbee, "zigbee", 8192, NULL, 4, &handleTaskZigbee);
  xTaskCreate(rtosDatetime, "datetime", 4096, NULL, 1, &handleTaskDatetime);
  xTaskCreatePinnedToCore(rtosNet, "net", 8192, NULL, 1, &handleTaskNet, CONFIG_ARDUINO_RUNNING_CORE);

  // start refresher tasks
  xTaskCreate(rtosMenuRefresh, "menurefresh", 1024, NULL, 1, &handleTaskMenuRefresh);
  xTaskCreate(rtosRfidRefresh, "rfidrefresh", 1024, NULL, 3, &handleTaskRfidRefresh);

  // start application
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  xTaskCreate(rtosMenu, "menu", 16384, NULL, 5, &handleTaskMenu);

  esplogI("[setup]: All tasks created successfully!\n");
  esplogI("--------------------------------------------------------------------------------\n");
}

// -------------------------------------------------------------------------------------------------------------
/* LOOP FUNCTION */
void loop() {
  vTaskDelay(100 * 60 * 1000 / portTICK_PERIOD_MS);
}

// -------------------------------------------------------------------------------------------------------------
/* STATE AUTO REFRESHER */
void rtosMenuRefresh(void* parameters) {
  // esplogI("[setup]: rtosMenuRefresh task was created!\n");
  vTaskSuspend(NULL);
  for(;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    g_vars.refresh = true;
    g_vars.confirm = true;
  }
}

// -------------------------------------------------------------------------------------------------------------
/* RFID AUTO REFRESHER */
void rtosRfidRefresh(void* parameters) {
  // esplogI("[setup]: rtosRfidRefresh task was created!\n");
  vTaskSuspend(NULL);
  for(;;) {
    vTaskDelay(250 / portTICK_PERIOD_MS);
    xTaskNotifyGive(handleTaskRfid);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* KEYPAD SCANNER */
void rtosKeypad(void* parameters) {
  // esplogI("[setup]: rtosKeypad task was created!\n");
  char keymap[] = "147*2580369#ABCDNF";
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
        keypadEvent(&g_vars, &g_config, key);
        g_vars.refresh = true;
      }

      key_last = key;
      key_last_t = key_t;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* WIFI AP SETUP HANDLER */
void rtosSetup(void* parameters) {
  // esplogI("[setup]: rtosSetup task was created!\n");
  esplogI("[wifi]: WiFi setup mode is active!\n");
  // vTaskDelete(handleTaskKeypad); <- for possibility to reboot esp by pressing any key
  vTaskDelete(handleTaskMenu);
  vTaskDelete(handleTaskNet);
  vTaskDelete(handleTaskDatetime);
  vTaskDelete(handleTaskZigbee);
  vTaskDelete(handleTaskGsm);
  g_vars.wifi_mode = WIFI_MODE_AP;

  startWifiSetupMode();
  for (;;) {vTaskDelay(100 / portTICK_PERIOD_MS);}
}

// -------------------------------------------------------------------------------------------------------------
/* DATETIME UPDATE HANDLER */
void rtosDatetime(void* parameters) {

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  timeClient.begin();
  timeClient.setTimeOffset(3600);

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  for (;;) {
    g_vars.datetime = timeClient.getEpochTime();
    time_t rawTime = g_vars.datetime;
    struct tm * timeInfo = localtime(&rawTime);

    char dateBuffer[11];
    strftime(dateBuffer, sizeof(dateBuffer), "%d/%m/%Y", timeInfo);
    char timeBuffer[6];
    strftime(timeBuffer, sizeof(timeBuffer), "%H:%M", timeInfo);
    
    g_vars.time = String(timeBuffer);
    g_vars.date = String(dateBuffer);

    updateScreen(&g_vars, &g_config, UPDATE_DATETIME);
    vTaskDelay(60 * 1000 / portTICK_PERIOD_MS);
    esplogI("[time]: Time has been updated! %s %s\n", g_vars.date, g_vars.time);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* WIFI/INTERNET HANDLER */
void rtosNet(void* parameters) {
  // esplogI("[setup]: rtosNet task was created!\n");

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
        g_vars.wifi_strength = WiFi.RSSI();
        updateScreen(&g_vars, &g_config, UPDATE_STATUS);
        // TODO is it good idea? what if robber shut down internet connection on purpose? --- than alarm will work only as gsm notifier!
        vTaskDelay(5 * 60 * 1000 / portTICK_PERIOD_MS);
        break;

      // WiFi SSID not available
      case WL_NO_SSID_AVAIL:
        g_vars.wifi_strength = 1;
        esplogW("[wifi]: WiFi connection failed! WiFi SSID was not found! Please open setup and reconfigure!\n");
        vTaskSuspend(NULL);
        break;

      // WiFi connection was unsuccessfull
      case WL_CONNECT_FAILED:
        g_vars.wifi_strength = 2;
        // task will only continue if setup state is triggered
        esplogW("[wifi]: WiFi connection failed! This could be due to wrong password, bad connection or router error. Please reboot or open setup and reconfigure!\n");
        vTaskSuspend(NULL);
        break;

      // WiFi connection has been lost
      case WL_CONNECTION_LOST:
        g_vars.wifi_strength = 3;
        esplogW("[wifi]: WiFi conection has been lost! Trying to reconect.\n");
        break;

      // case WL_DISCONNECTED: // <-- this should never happen!
      //   break;

      // WL_IDLE_STATUS, WL_SCAN_COMPLETED, (WL_DISCONNECTED)
      default:
        g_vars.wifi_strength = 99;
        esplogW("[wifi]: Unexpected WiFi status!\n - status: %d\n", g_vars.wifi_status);
        break;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* GSM COMMUNICATION HANDELER */
void rtosGsm(void* parameters) {
  // esplogI("[setup]: rtosGsm task was created!\n");
  // TODO
  for(;;) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* ZIGBEE COMMUNICATION HANDELER */
void rtosZigbee(void* parameters) {
  // esplogI("[setup]: rtosZigbee task was created!\n");
  esp_zb_ieee_addr_t ieee_addr;
  iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");
  iot_alarm_message_t * msg_ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");
  iot_alarm_attr_load_t * msg_load = create_attr("\0", "\0", "\0", 0, ieee_addr, 0, 0, 0, 0, 0, ESP_ZB_ZCL_ATTR_TYPE_U8, 0);

  for(;;) {
    if (SerialZigbee.available()) {
      int rx_bytes = receive_message(UART, rx_buffer, &msg, 1024-1);

      // handeling of unusable messages
      if (msg->dir >= IOT_ALARM_MSGDIR_MAX || msg->dir < 0 ||
          msg->id >= IOT_ALARM_MSGTYPE_MAX || msg->id <= 0 ||
          msg->st >= IOT_ALARM_MSGSTATUS_MAX || msg->st < 0) {
        esplogW("[zigbee] Invalid message has been received!\n");
        continue;
      }

      // handeling of acknowladges
      if (msg->dir == IOT_ALARM_MSGDIR_COMMAND_ACK || msg->dir == IOT_ALARM_MSGDIR_NOTIFICATION_ACK) {
        esplogI("[zigbee] Acknowledgement message has been received!\n");
        continue;
      }

      // send acknowledge
      if (msg->dir == IOT_ALARM_MSGDIR_NOTIFICATION) {
        msg_ack->dir = IOT_ALARM_MSGDIR_NOTIFICATION_ACK; msg_ack->id = msg->id; msg_ack->st = IOT_ALARM_MSGSTATUS_SUCCESS;
        send_message(UART, tx_buffer, msg_ack);
      } else if (msg->dir == IOT_ALARM_MSGDIR_COMMAND) {
        msg_ack->dir = IOT_ALARM_MSGDIR_COMMAND_ACK; msg_ack->id = msg->id; msg_ack->st = IOT_ALARM_MSGSTATUS_SUCCESS;
        send_message(UART, tx_buffer, msg_ack);
      }

      // handle received messages
      if (rx_bytes > 0) {
        esplogI("[zigbee]: Message (length: %d) received: DIR: %d, ID: %d, STATUS: %d, LEN: %d, LOAD: %s\n", rx_bytes, msg->dir, msg->id, msg->st, msg->length, msg->load);
        switch (msg->id) {
          case IOT_ALARM_MSGTYPE_DEV_COUNT:
            break;

          case IOT_ALARM_MSGTYPE_ZB_DATA_READ:
          case IOT_ALARM_MSGTYPE_ZB_DATA_WRITE:
          case IOT_ALARM_MSGTYPE_ZB_DATA_REPORT:
            if (msg->load != NULL) {
              deserialize_attr(&msg_load, (uint8_t*)msg->load);
              esplogI("[zigbee]: Device data: %s - %s [%s]\n", msg_load->manuf, msg_load->name, msg_load->type);
              esplogI("[zigbee]: Attribute data: short: %04hx/%d, cluster: %04hx, attribute: %04hx, type: %d, value: ",
                msg_load->short_addr, msg_load->endpoint_id, msg_load->cluster_id, msg_load->attr_id, msg_load->value_type);

              switch (msg_load->value_type) {
                case ESP_ZB_ZCL_ATTR_TYPE_8BIT:
                case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:
                case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:
                case ESP_ZB_ZCL_ATTR_TYPE_U8:

                case ESP_ZB_ZCL_ATTR_TYPE_16BIT:
                case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:
                case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:
                case ESP_ZB_ZCL_ATTR_TYPE_U16:

                case ESP_ZB_ZCL_ATTR_TYPE_32BIT:
                case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:
                case ESP_ZB_ZCL_ATTR_TYPE_U32:
                  Serial.printf("%lu\n", msg_load->value);
                  break;

                default:
                  Serial.printf("\n");
              }
            }
            break;

          // case IOT_ALARM_MSGTYPE_ZB_DEV_NEW:
          // case IOT_ALARM_MSGTYPE_ZB_DEV_LEAVE:
          //   break;
          
          default:
            break;
        }

        Serial.println();
      }

    } else {
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }

  destroy_message(&msg);
  destroy_message(&msg_ack);
  destroy_attr(&msg_load);
}

// -------------------------------------------------------------------------------------------------------------
/* ALARM APPLICATION HANDELER */
void rtosAlarm(void* testmode) {
  // esplogI("[setup]: rtosAlarm task was created!\n");
  unsigned long w_time = 0;
  bool testing = (bool)testmode;
  for (;;) {
    unsigned long curr_time = millis();

    // handle alarm events if state is OK
    if (g_vars.state == STATE_ALARM_OK || g_vars.state == STATE_TEST_OK) {
      if (g_vars.alarm_events >= g_config.alarm_w_threshold) {
        w_time = millis();
        if (testing) {
          setState(STATE_TEST_W, 0, 0);
        } else {
          setState(STATE_ALARM_W, 0, 0);
        }
      }

      if (g_vars.alarm_events >= g_config.alarm_e_threshold) {
        w_time = 0;
        g_vars.time_temp = 0;
        if (testing) {
          setState(STATE_TEST_E, 0, 0);
        } else {
          setState(STATE_ALARM_E, 0, 0);
        }
      }

    // handle alarm events if state is W
    } else if (g_vars.state == STATE_ALARM_W  || g_vars.state == STATE_TEST_W) {
      if (g_vars.alarm_events >= g_config.alarm_e_threshold) {
        w_time = 0;
        g_vars.time_temp = 0;
        if (testing) {
          setState(STATE_TEST_E, 0, 0);
        } else {
          setState(STATE_ALARM_E, 0, 0);
        }
      }

      if (w_time > 0) {
        if (curr_time >= w_time + g_config.alarm_e_countdown_s*1000) {
          w_time = 0;
          g_vars.time_temp = 0;
          if (testing) {
            setState(STATE_TEST_E, 0, 0);
          } else {
            setState(STATE_ALARM_E, 0, 0);
          }
        } else {
          g_vars.time_temp = curr_time - w_time;
          updateScreen(&g_vars, &g_config, UPDATE_COUNTDOWN);
          continue;
        }
      }

    // handle alarm events if state is E
    } else if (g_vars.state == STATE_ALARM_E  || g_vars.state == STATE_TEST_E) {
      // nothing has to be handeled
      // maybe some delays and notifications
      esplogW("[alarm]: EMERGENCY status!\n");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* RFID READER HANDLER */
void rtosRfid(void* parameters) {
  // esplogI("[setup]: rtosRfid task was created!\n");
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

        case STATE_SETUP_AP_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          esplogI("[menu]: Starting WiFi Setup Mode!\n");
          setState(STATE_SETUP_AP, 0, 0, "", 0);
          xTaskCreatePinnedToCore(rtosSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
          vTaskSuspend(handleTaskRfidRefresh);
          break;

        case STATE_SETUP_HARD_RESET_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_SETUP_HARD_RESET, -1, 0, "", 0);
          break;

        case STATE_ALARM_LOCK_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_ALARM_C, -1, 0, "", 0);
          vTaskResume(handleTaskMenuRefresh);
          break;

        case STATE_TEST_LOCK_ENTER_PIN:
          rfidScreenC(&g_vars, rfid_card.c_str());
          setState(STATE_TEST_C, -1, 0, "", 0);
          vTaskResume(handleTaskMenuRefresh);
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

        case STATE_ALARM_OK:
        case STATE_ALARM_W:
        case STATE_ALARM_E:
          authScreenC(&g_vars);
          setState(STATE_ALARM_IDLE, -1, SELECTION_ALARM_IDLE_MAX, "", 0);
          vTaskDelete(handleTaskAlarm);
          break;

        case STATE_TEST_OK:
        case STATE_TEST_W:
        case STATE_TEST_E:
          authScreenC(&g_vars);
          setState(STATE_TEST_IDLE, -1, SELECTION_TEST_IDLE_MAX, "", 0);
          vTaskDelete(handleTaskAlarm);
          break;

        default:
          esplogW("[rfid]: RFID task was running in unknown state! Terminating...\n");
          break;
      }
      vTaskSuspend(handleTaskRfidRefresh);
    } else {
      esplogI("[rfid]: Card was not authorised!\n");
      rfidScreenE(&g_vars, rfid_card.c_str());
      setState(STATE_MAX, -1, -1, "NULL", g_vars.attempts+1);
    }
    g_vars.refresh = true;
  }
}

// -------------------------------------------------------------------------------------------------------------
/* MAIN APPLICATION STUCTURE */
void rtosMenu(void* parameters) {
  // esplogI("[setup]: rtosMenu task was created!\n");
  vTaskDelay(500 / portTICK_PERIOD_MS);
  setState(STATE_INIT, 0, SELECTION_INIT_MAX, "", 0);
  unsigned long lock_time = 0;
  for (;;) {
    if (g_vars.refresh) {
      unsigned long curr_time = millis();
      if (g_vars.abort && g_vars.state_prev != STATE_MAX) {
        if (g_vars.state_prev == STATE_INIT ||
            g_vars.state_prev == STATE_SETUP ||
            g_vars.state_prev == STATE_SETUP_PIN2 ||
            g_vars.state_prev == STATE_ALARM_IDLE ||
            g_vars.state_prev == STATE_ALARM_CHANGE_ENTER_PIN2 ||
            g_vars.state_prev == STATE_TEST_IDLE ||
            g_vars.state_prev == STATE_TEST_CHANGE_ENTER_PIN2) {
          setState(g_vars.state_prev, g_vars.selection_prev, g_vars.selection_max_prev, "", 0);
          vTaskSuspend(handleTaskMenuRefresh);
          vTaskSuspend(handleTaskRfidRefresh);
        }
        else if (g_vars.state == STATE_SETUP_HARD_RESET) {
          loadScreen(&g_vars, &g_config, true);
          rebootESP();
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
                if (existsPassword()) {
                  setState(STATE_SETUP_AP_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  esplogI("[menu]: Starting WiFi Setup Mode!\n");
                  setState(STATE_SETUP_AP, 0, 0);
                  xTaskCreatePinnedToCore(rtosSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
                  vTaskSuspend(NULL);
                }
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

          case STATE_SETUP_AP_ENTER_PIN:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              esplogI("[menu]: Starting WiFi Setup Mode!\n");
              authScreenC(&g_vars);
              setState(STATE_SETUP_AP, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
              xTaskCreatePinnedToCore(rtosSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
              vTaskSuspend(NULL);
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
              g_vars.time_temp = 0;
              setState(STATE_ALARM_OK, 0, 0);
              vTaskSuspend(handleTaskMenuRefresh);
              xTaskCreate(rtosAlarm, "alarm", 4096, (void*)false, 5, &handleTaskAlarm);
              vTaskResume(handleTaskRfidRefresh);
            } else {
              g_vars.refresh = false;
              g_vars.confirm = false;
              g_vars.time_temp = curr_time - lock_time;
              updateScreen(&g_vars, &g_config, UPDATE_COUNTDOWN);
              continue;
            }
            break;

          // test cooldown state
          case STATE_TEST_C:
            if (curr_time >= lock_time + g_config.alarm_countdown_s*1000) {
              lock_time = 0;
              g_vars.time_temp = 0;
              setState(STATE_TEST_OK, 0, 0);
              vTaskSuspend(handleTaskMenuRefresh);
              xTaskCreate(rtosAlarm, "alarm", 4096, (void*)true, 5, &handleTaskAlarm);
              vTaskResume(handleTaskRfidRefresh);
            } else {
              g_vars.refresh = false;
              g_vars.confirm = false;
              g_vars.time_temp = curr_time - lock_time;
              updateScreen(&g_vars, &g_config, UPDATE_COUNTDOWN);
              continue;
            }
            break;

          // ***************************
          // alarm ok state; alarm warning state; alarm emengency state TODO
          case STATE_ALARM_OK:
          case STATE_ALARM_W:
          case STATE_ALARM_E:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_ALARM_IDLE, 0, SELECTION_ALARM_IDLE_MAX, "", 0);
              vTaskDelete(handleTaskAlarm);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_MAX, -1, -1, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
              vTaskResume(handleTaskRfidRefresh);
            }
            break;

          // test ok state; test warning state; test emengency state TODO
          case STATE_TEST_OK:
          case STATE_TEST_W:
          case STATE_TEST_E:
            esplogI("[menu]: Entered pin: %s\n", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_TEST_IDLE, 0, SELECTION_TEST_IDLE_MAX, "", 0);
              vTaskDelete(handleTaskAlarm);
              vTaskSuspend(handleTaskRfidRefresh);
            } else {
              authScreenE(&g_vars);
              setState(STATE_MAX, -1, -1, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
              vTaskResume(handleTaskRfidRefresh);
            }
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

      esplogI("[menu]: State: %s  |  Selection: %s\n", getStateText(g_vars.state), getSelectionText(g_vars.state, g_vars.selection));
      g_vars.refresh = false;

    } else {
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
}
