/**
 * @file main.cpp
 * @brief Main file containing setup and tasks for the application.
 */

#include "main.h"

TaskHandle_t handleTaskMenu = NULL;
TaskHandle_t handleTaskAlarm = NULL;
TaskHandle_t handleTaskKeypad = NULL;
TaskHandle_t handleTaskNet = NULL;
TaskHandle_t handleTaskDatetime = NULL;
TaskHandle_t handleTaskSetup = NULL;
TaskHandle_t handleTaskRfid = NULL;
TaskHandle_t handleTaskDisplay = NULL;
TaskHandle_t handleTaskGsm = NULL;
TaskHandle_t handleTaskZigbee = NULL;
TaskHandle_t handleTaskMqtt = NULL;
TaskHandle_t handleTaskMenuRefresh = NULL;
TaskHandle_t handleTaskRfidRefresh = NULL;

// QueueHandle_t queueMqtt;
QueueHandle_t queueNotification;

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
  .alarm_event_fire = 0,
  .alarm_event_water = 0,
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
  esplogI(TAG_SETUP, NULL, "ESP Started");

  // load configuration
  loadConfig(&g_config, CONFIG_FILE);
  esplogI(TAG_SETUP, NULL, "Config:\n - ssid: %s\n - pswd: %s\n - ip: %s\n - gtw: %s\n - sbnt: %s", g_config.wifi_ssid, g_config.wifi_pswd.c_str(), g_config.wifi_ip.c_str(), g_config.wifi_gtw.c_str(), g_config.wifi_sbnt.c_str());

  // init display EINK
#ifdef EINK
  esplogI(TAG_SETUP, NULL, "Display mode: EINK");
  initEink();
#endif

  // init display LCD
#ifdef LCD
  esplogI(TAG_SETUP, NULL, " Display mode: LCD");
  initLcd();
#endif

  // init keypad
  if (!keypad.begin()) {
    esplogE(TAG_SETUP, NULL, "Failed to initialise keypad! Rebooting...");
  }

  // init GSM module
  // if (!initSerialGSM()) {
  //   esplogE("[setup]: Failed to initialise GSM module!\n");
  // }

  // init Zigbee module
  tx_buffer = (uint8_t*)malloc(TX_BUF_SIZE + 1);
  rx_buffer = (uint8_t*)malloc(RX_BUF_SIZE + 1);
  if (!initSerialZigbee()) {
    esplogE(TAG_SETUP, NULL, "Failed to initialise Zigbee module!");
  }

  // init rfid
  mfrc522.PCD_Init(RFID_CS_PIN, RFID_RST_PIN);
  esplogI(TAG_SETUP, NULL, "RFID reader: ");
  mfrc522.PCD_DumpVersionToSerial();

  // automatically start AP setup if no ssid is provided
  if (g_config.wifi_ssid == "") {
    esplogI(TAG_SETUP, NULL, "No SSID has been configured, starting AP setup!");
    notificationScreenTemplate("No WiFi SSID configured", "Please open WiFi setup!");
    g_vars.wifi_mode = WIFI_MODE_AP;
    startWifiSetupMode();
    for (;;) {vTaskDelay(1000 / portTICK_PERIOD_MS);}
  }

  // queue initialisation
  // queueMqtt = xQueueCreate(10, sizeof(mqtt_message_t));
  queueNotification = xQueueCreate(10, sizeof(notification_t));

  // start support tasks
  xTaskCreate(rtosKeypad, "keypad", 8192, NULL, 3, &handleTaskKeypad);
  xTaskCreate(rtosRfid, "rfid", 4096, NULL, 3, &handleTaskRfid);
  xTaskCreate(rtosDisplay, "display", 8192, NULL, 4, &handleTaskDisplay);
  // xTaskCreate(rtosGsm, "gsm", 8192, NULL, 2, &handleTaskGsm);
  xTaskCreate(rtosZigbee, "zigbee", 8192, NULL, 4, &handleTaskZigbee);
  xTaskCreatePinnedToCore(rtosMqtt, "mqtt", 8192, NULL, 2, &handleTaskMqtt, CONFIG_ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(rtosDatetime, "datetime", 4096, NULL, 1, &handleTaskDatetime, CONFIG_ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(rtosNet, "net", 8192, NULL, 1, &handleTaskNet, CONFIG_ARDUINO_RUNNING_CORE);

  // start refresher tasks
  xTaskCreate(rtosMenuRefresh, "menurefresh", 1024, NULL, 1, &handleTaskMenuRefresh);
  xTaskCreate(rtosRfidRefresh, "rfidrefresh", 1024, NULL, 3, &handleTaskRfidRefresh);

  esplogI(TAG_SETUP, NULL, "All tasks created successfully!");
  esplogI(TAG_SETUP, NULL, "--------------------------------------------------------------------------------");

  // start application
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  xTaskCreate(rtosMenu, "menu", 16384, NULL, 5, &handleTaskMenu);
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
void rtosWifiSetup(void* parameters) {
  // esplogI(TAG_SETUP, NULL, "rtosSetup task was created!");
  esplogI(TAG_RTOS_WIFI, NULL, "WiFi setup mode is active!");
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
  timeClient.setUpdateInterval(60 * 60 * 1000);

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
    esplogI(TAG_RTOS_DATETIME, NULL, "Time has been updated! %s %s", g_vars.date, g_vars.time);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* WIFI/INTERNET HANDLER */
void rtosNet(void* parameters) {
  // esplogI("[setup]: rtosNet task was created!\n");

  // TODO think about replacing "while(true) and wifi.state()"" with "esp wifi events"
  // dont start the WiFi task if config has not been set
  if (g_config.wifi_ssid == "\0") {
    // task will only continue if setup state is triggered
    esplogW(TAG_RTOS_WIFI, NULL, "WiFi will start only after configuration is done!");
    goto skiploop;
  }

  startWiFiServerMode(&g_vars, &g_config);
  vTaskDelay(10000 / portTICK_PERIOD_MS);

loop:
  for(;;) {
    // check wifi state
    g_vars.wifi_status = WiFi.status();
    switch (g_vars.wifi_status) {
      // WiFi is connected
      case WL_CONNECTED:
        esplogI(TAG_RTOS_WIFI, NULL, "WiFi periodic check passed!\n - status: WL_CONNECTED\n - rssi: %d\n - ip: %s", WiFi.RSSI(), WiFi.localIP().toString().c_str());
        g_vars.wifi_strength = WiFi.RSSI();
        updateScreen(&g_vars, &g_config, UPDATE_STATUS);
        // TODO is it good idea? what if robber shut down internet connection on purpose? --- than alarm will work only as gsm notifier!
        vTaskDelay(5 * 60 * 1000 / portTICK_PERIOD_MS);
        break;

      // WiFi SSID not available
      case WL_NO_SSID_AVAIL:
        g_vars.wifi_strength = 1;
        esplogW(TAG_RTOS_WIFI, NULL, "WiFi connection failed! WiFi SSID was not found! Please open setup and reconfigure!");
        goto skiploop;
        break;

      // WiFi connection was unsuccessfull
      case WL_CONNECT_FAILED:
        g_vars.wifi_strength = 2;
        // task will only continue if setup state is triggered
        esplogW(TAG_RTOS_WIFI, NULL, "WiFi connection failed! This could be due to wrong password, bad connection or router error. Please reboot or open setup and reconfigure!");
        goto skiploop;
        break;

      // WiFi connection has been lost
      case WL_CONNECTION_LOST:
        g_vars.wifi_strength = 3;
        esplogW(TAG_RTOS_WIFI, NULL, "WiFi conection has been lost! Trying to reconect.");
        break;

      // case WL_DISCONNECTED: // <-- this should never happen!
      //   break;

      // WL_IDLE_STATUS, WL_SCAN_COMPLETED, (WL_DISCONNECTED)
      default:
        g_vars.wifi_strength = 99;
        esplogW(TAG_RTOS_WIFI, NULL, "Unexpected WiFi status!\n - status: %d", g_vars.wifi_status);
        break;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

skiploop:
  esplogW(TAG_RTOS_WIFI, NULL, "WiFi task terminated!");
  for (;;) {
    // WiFi.reconnect();
    // goto loop; // if connected back
    // acustic notification via buzzer (here should the code go when the wifi will disconnect)
    // TODO
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// -------------------------------------------------------------------------------------------------------------
/* DISPLAY REFRESHER HANDELER */
void rtosDisplay(void* parameters) {
  // esplogI("[setup]: rtosDisplay task was created!\n");
  for (;;) {
    // if state hasn't changed && no notification is in queue -> delay task and loop
    // based on current state prepare display content
    // based on notification queue prepare display content
    // send data to display all at once
    // wait till display is ready
    // loop

    // delete following line (delay from display busy is more significant)
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
/* MQTT SUBSCRIBE HANDELER */
void rtosMqtt(void* parameters) {
  // esplogI("[setup]: rtosMqtt task was created!\n");

  while (g_config.mqtt_broker.length() <= 0 || g_config.mqtt_id.length() <= 0) {
    esplogW(TAG_RTOS_MQTT, NULL, "MQTT setup failed, please fill in MQTT configuration!");
    vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
  }

  while (g_vars.wifi_status != WL_CONNECTED) {vTaskDelay(2000 / portTICK_PERIOD_MS);}

  if (g_config.mqtt_tls) {
    mqttwificlientsecure.setCACert(g_config.mqtt_cert.c_str());
    mqtt.setClient(mqttwificlientsecure);
    mqtt.setServer(g_config.mqtt_broker.c_str(), g_config.mqtt_port);
  } else {
    mqtt.setClient(mqttwificlient);
    mqtt.setServer(g_config.mqtt_broker.c_str(), g_config.mqtt_port);
  }
  
  mqtt.setCallback(mqtt_callback);
  mqtt.setBufferSize(1024);

  for(;;) {
    // if disconnected, reconnect
    while (!mqtt.connected()) {
      if (mqtt.connect(g_config.mqtt_id.c_str(), g_config.mqtt_username.c_str(), g_config.mqtt_password.c_str())) {
        esplogI(TAG_RTOS_MQTT, NULL, "MQTT server connected!");
        // subscribe to topics
        if (mqtt.subscribe(String(g_config.mqtt_topic + String("/read/in/#")).c_str())) {
          esplogI(TAG_RTOS_MQTT, NULL, "Subscribed to: %s", String(g_config.mqtt_topic + String("/read/in")).c_str());
        }

        if (mqtt.subscribe(String(g_config.mqtt_topic + String("/write/in/#")).c_str())) {
          esplogI(TAG_RTOS_MQTT, NULL, "Subscribed to: %s", String(g_config.mqtt_topic + String("/write/in")).c_str());
        }
        
      } else {
        esplogW(TAG_RTOS_MQTT, NULL, "Failed to connect to MQTT server! (%s)", g_config.mqtt_broker.c_str());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
      }
    }

    /* mqtt_message_t * message;
    if (xQueueReceive(mqttQueue, &message, 0) == pdTRUE) {
      esplogI(TAG_RTOS_MQTT, NULL, "MQTT message has been poped from queue! [%s]: %s", message->topic, message->load);
      mqtt_publish(message->topic, message->load);

      if (message != NULL) {
        if (message->load != NULL) {
          free(message->load);
        }

        if (message->topic != NULL) {
          free(message->topic);
        }

        free(message);
      }
    } */

    mqtt.loop();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
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
  iot_alarm_attr_load_t * msg_load_prev = create_attr("\0", "\0", "\0", 0, ieee_addr, 0, 0, 0, 0, 0, ESP_ZB_ZCL_ATTR_TYPE_U8, 0);

  if (msg == NULL || msg_ack == NULL || msg_load == NULL || msg_load_prev == NULL) {
    esplogE(TAG_RTOS_ZIGBEE, NULL, "Failed to allocate memory for zigbee messages! Rebooting...");
  }
  
  for(;;) {
    if (SerialZigbee.available()) {
      int rx_bytes = receive_message(UART, rx_buffer, &msg, RX_BUF_SIZE-1);

      // handeling of unusable messages
      if (msg->dir >= IOT_ALARM_MSGDIR_MAX || msg->dir < 0 ||
          msg->id >= IOT_ALARM_MSGTYPE_MAX || msg->id <= 0 ||
          msg->st >= IOT_ALARM_MSGSTATUS_MAX || msg->st < 0) {
        esplogW(TAG_RTOS_ZIGBEE, NULL, "Invalid message has been received!");
        continue;
      }

      // handeling of acknowladges
      if (msg->dir == IOT_ALARM_MSGDIR_COMMAND_ACK || msg->dir == IOT_ALARM_MSGDIR_NOTIFICATION_ACK) {
        esplogI(TAG_RTOS_ZIGBEE, NULL,"Acknowledgement message has been received!");
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
        esplogI(TAG_RTOS_ZIGBEE, NULL,"Message (length: %d) received: DIR: %d, ID: %d, STATUS: %d, LEN: %d, LOAD: %s", rx_bytes, msg->dir, msg->id, msg->st, msg->length, msg->load);
        uint8_t duration;
        uint8_t device_count;
        switch (msg->id) {
          case IOT_ALARM_MSGTYPE_DEV_COUNT:
            if (msg->load != NULL) {
              device_count = (uint8_t)atoi(msg->load);
              zigbeeScreenN(&g_vars, &g_config, device_count);
            }
            break;

          case IOT_ALARM_MSGTYPE_ZB_DATA_READ:
            if (msg->load != NULL) {
              deserialize_attr(&msg_load, (uint8_t*)msg->load, RX_BUF_SIZE-1);
              if (compare_attr(*msg_load, *msg_load_prev)) {continue;}

              // push to mqtt
              String load;
              if (pack_attr(msg_load, &load)) {
                  char ieee_str[41];
                  sprintf(ieee_str, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                      msg_load->ieee_addr[7], msg_load->ieee_addr[6], msg_load->ieee_addr[5], msg_load->ieee_addr[4],
                      msg_load->ieee_addr[3], msg_load->ieee_addr[2], msg_load->ieee_addr[1], msg_load->ieee_addr[0]);
                  String topic = g_config.mqtt_topic + String("/read/out/") + ieee_str;
                  mqtt_publish(topic, load);
              }

              zigbeeAttrReadWriteHandler(msg_load);
              copy_attr(msg_load, msg_load_prev);
            }
            break;

          case IOT_ALARM_MSGTYPE_ZB_DATA_WRITE:
            if (msg->load != NULL) {
              deserialize_attr(&msg_load, (uint8_t*)msg->load, RX_BUF_SIZE-1);
              if (compare_attr(*msg_load, *msg_load_prev)) {continue;}

              // push to mqtt
              String load;
              if (pack_attr(msg_load, &load)) {
                  char ieee_str[41];
                  sprintf(ieee_str, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                      msg_load->ieee_addr[7], msg_load->ieee_addr[6], msg_load->ieee_addr[5], msg_load->ieee_addr[4],
                      msg_load->ieee_addr[3], msg_load->ieee_addr[2], msg_load->ieee_addr[1], msg_load->ieee_addr[0]);
                  String topic = g_config.mqtt_topic + String("/write/out/") + ieee_str;
                  mqtt_publish(topic, load);
              }

              zigbeeAttrReadWriteHandler(msg_load);
              copy_attr(msg_load, msg_load_prev);
            }
            break;

          case IOT_ALARM_MSGTYPE_ZB_DATA_REPORT:
            if (msg->load != NULL) {
              deserialize_attr(&msg_load, (uint8_t*)msg->load, RX_BUF_SIZE-1);
              if (compare_attr(*msg_load, *msg_load_prev)) {continue;}

              esplogI(TAG_RTOS_ZIGBEE, NULL, "Attr report obtained: short: %04hx, ieee: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X, dev_id: %d, ep_id: %d, cluster_id: %04hx, attr_id: %04hx, value: %lu",
                  msg_load->short_addr,
                  msg_load->ieee_addr[7], msg_load->ieee_addr[6], msg_load->ieee_addr[5], msg_load->ieee_addr[4],
                  msg_load->ieee_addr[3], msg_load->ieee_addr[2], msg_load->ieee_addr[1], msg_load->ieee_addr[0],
                  msg_load->device_id, msg_load->endpoint_id, msg_load->cluster_id, msg_load->attr_id, msg_load->value);

              // push to mqtt
              String load;
              if (pack_attr(msg_load, &load)) {
                  char ieee_str[41];
                  sprintf(ieee_str, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                      msg_load->ieee_addr[7], msg_load->ieee_addr[6], msg_load->ieee_addr[5], msg_load->ieee_addr[4],
                      msg_load->ieee_addr[3], msg_load->ieee_addr[2], msg_load->ieee_addr[1], msg_load->ieee_addr[0]);
                  String topic = g_config.mqtt_topic + String("/report/") + ieee_str;
                  mqtt_publish(topic, load);
              }

              zigbeeAttrReportHandler(msg_load);
              copy_attr(msg_load, msg_load_prev);
            }
            break;

          case IOT_ALARM_MSGTYPE_ZB_DEV_LOCK:
            zigbeeScreenC(&g_vars, &g_config);
            break;

          case IOT_ALARM_MSGTYPE_ZB_DEV_UNLOCK:
            duration = (uint8_t)atoi(msg->load);
            zigbeeScreenO(&g_vars, &g_config, duration);
            break;

          // case IOT_ALARM_MSGTYPE_ZB_DEV_NEW:
          // case IOT_ALARM_MSGTYPE_ZB_DEV_LEAVE:
          //   break;
          
          default:
            break;
        }
      }

    } else {
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }

  destroy_message(&msg);
  destroy_message(&msg_ack);
  destroy_attr(&msg_load);
  destroy_attr(&msg_load_prev);
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
      esplogW(TAG_RTOS_ALARM, NULL,"EMERGENCY status!");
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
    esplogI(TAG_RTOS_RFID, NULL, "Card detected! UID: %s", rfid_card.c_str());

    if (g_vars.state == STATE_SETUP_RFID_ADD) {
      esplogI(TAG_RTOS_RFID, NULL, "Adding new UID: %s", rfid_card.c_str());
      addRfid(rfid_card);
      rfidScreenA(&g_vars, rfid_card.c_str());
      setState(STATE_SETUP, -1, SELECTION_SETUP_MAX);
    }

    else if (g_vars.state == STATE_SETUP_RFID_DEL) {
      esplogI(TAG_RTOS_RFID, NULL, "Deleting UID: %s", rfid_card.c_str());
      delRfid(rfid_card);
      rfidScreenD(&g_vars, rfid_card.c_str());
      setState(STATE_SETUP, -1, SELECTION_SETUP_MAX);
    }

    else if (checkRfid(rfid_card)) {
      esplogI(TAG_RTOS_RFID, NULL, "Card was authorised!");
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
          esplogI(TAG_RTOS_MAIN, NULL, "Starting WiFi Setup Mode!");
          setState(STATE_SETUP_AP, 0, 0, "", 0);
          xTaskCreatePinnedToCore(rtosWifiSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
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
          esplogW(TAG_RTOS_RFID, NULL, "RFID task was running in unknown state! Terminating...");
          break;
      }
      vTaskSuspend(handleTaskRfidRefresh);
    } else {
      esplogI(TAG_RTOS_RFID, NULL, "Card was not authorised!");
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
                  esplogI(TAG_RTOS_MAIN, NULL, "Starting WiFi Setup Mode!");
                  setState(STATE_SETUP_AP, 0, 0);
                  xTaskCreatePinnedToCore(rtosWifiSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
                  vTaskSuspend(NULL);
                }
                break;

              // TODO add notification window, handle the actions, add password controll
              case SELECTION_SETUP_OPEN_ZB:
                setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                zigbeeOpen();
                break;  
              case SELECTION_SETUP_CLOSE_ZB:
                setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                zigbeeClose();
                break;  
              case SELECTION_SETUP_CLEAR_ZB:
                setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                zigbeeClear();
                break;  
              case SELECTION_SETUP_RESET_ZB:
                setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                zigbeeReset();
                break;

              /* case SELECTION_SETUP_SET_PIN:
                if (existsPassword()) {
                  setState(STATE_SETUP_PIN1, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  setState(STATE_SETUP_PIN2, 0, 0);
                }
                break; */
              case SELECTION_SETUP_ADD_RFID:
                if (existsPassword()) {
                  setState(STATE_SETUP_RFID_ADD_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  esplogW(TAG_RTOS_MAIN, NULL, "Before setting RFID authentication, please set PIN!");
                  setState(STATE_SETUP_PIN2, 0, 0);
                }
                break;
              case SELECTION_SETUP_DEL_RFID:
                if (!existsRfid()) {
                  esplogI(TAG_RTOS_MAIN, NULL, "Trying to delete RFID, but any was set yet!");
                  setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                  break;
                }
                if (existsPassword()) {
                  setState(STATE_SETUP_RFID_DEL_ENTER_PIN, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  esplogW(TAG_RTOS_MAIN, NULL, "Unexpected behaviour! There should not be RFID set when no PIN was set yet!");
                  setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                }
                break;
              case SELECTION_SETUP_CHECK_RFID:
                if (!existsRfid()) {
                  esplogI(TAG_RTOS_MAIN, NULL, "Trying to check RFID, but any was set yet!");
                  setState(STATE_SETUP, 0, SELECTION_SETUP_MAX);
                  break;
                }
                if (existsPassword()) {
                  setState(STATE_SETUP_RFID_CHECK, 0, 0);
                  vTaskResume(handleTaskRfidRefresh);
                } else {
                  esplogW(TAG_RTOS_MAIN, NULL, "Unexpected behaviour! There should not be RFID set when no PIN was set yet!");
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              esplogI(TAG_RTOS_MAIN, NULL, "Starting WiFi Setup Mode!");
              authScreenC(&g_vars);
              setState(STATE_SETUP_AP, 0, 0, "", 0);
              vTaskSuspend(handleTaskRfidRefresh);
              xTaskCreatePinnedToCore(rtosWifiSetup, "wifisetup", 8192, NULL, 1, &handleTaskSetup, CONFIG_ARDUINO_RUNNING_CORE);
              vTaskSuspend(NULL);
            } else {
              authScreenE(&g_vars);
              setState(STATE_SETUP, 0, SELECTION_SETUP_MAX, "", g_vars.attempts+1);
              vTaskSuspend(handleTaskRfidRefresh);
            }
            break;

          case STATE_SETUP_HARD_RESET_ENTER_PIN:
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Hard reseting IoT Alarm! Re-creating configuration data.");
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
              g_vars.alarm_events = 0;
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
              g_vars.alarm_events = 0;
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_ALARM_IDLE, 0, SELECTION_ALARM_IDLE_MAX, "", 0);
              vTaskDelete(handleTaskAlarm);
              vTaskSuspend(handleTaskRfidRefresh);
              g_vars.alarm_events = 0;
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
            if (checkPassword(g_vars.pin)) {
              authScreenC(&g_vars);
              setState(STATE_TEST_IDLE, 0, SELECTION_TEST_IDLE_MAX, "", 0);
              vTaskDelete(handleTaskAlarm);
              vTaskSuspend(handleTaskRfidRefresh);
              g_vars.alarm_events = 0;
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
            setState(STATE_ALARM_CHANGE_ENTER_PIN3, 0, 0);
            break;

          // state for changing the password
          case STATE_TEST_CHANGE_ENTER_PIN2:
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
            setState(STATE_TEST_CHANGE_ENTER_PIN3, 0, 0);
            break;

          // state for changing the password
          case STATE_SETUP_PIN2:
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
            setState(STATE_SETUP_PIN3, 0, 0);
            break;

          // state for confirmation of the password change
          case STATE_ALARM_CHANGE_ENTER_PIN3:
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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
            esplogI(TAG_RTOS_MAIN, NULL, "Entered pin: %s", g_vars.pin.c_str());
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

      esplogI(TAG_RTOS_MAIN, NULL, "State: %s  |  Selection: %s", getStateText(g_vars.state), getSelectionText(g_vars.state, g_vars.selection));
      g_vars.refresh = false;

    } else {
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
}
