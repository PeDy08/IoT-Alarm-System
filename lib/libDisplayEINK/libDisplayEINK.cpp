#include "libDisplayEINK.h"

#include <Arduino.h>
#include <SPI.h>

#define SPI_MOSI 16
#define SPI_MISO 4
#define SPI_CLK 17

#define EPD_CS 5
#define EPD_RST 19
#define EPD_DC 18
#define EPD_BUSY 23

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <U8g2_for_Adafruit_GFX.h>

#include "GxEPD2_display_selection.h"
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

extern QueueHandle_t queueNotification;

void initScreenTemplate(const char * label);
void menuScreenTemplate(const char * label, int selection, bool test, const char * option1, const char * option2, const char * option3, const char * option4, const char * time, const char * date, int wifi, int gsm, int battery);
void authScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, String pin, int attempts, const char * time, const char * date, int wifi, int gsm, int battery);
void rfidScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, int attempts, const char * time, const char * date, int wifi, int gsm, int battery);
void alarmScreenTemplate(const char * label, bool test, const char * status, const char * data, String pin, int attempts, int data_load, const char * time, const char * date, int wifi, int gsm, int battery);

void updateStatusIcons(int wifi, int gsm, int battery);
void updateDatetime(const char * date, const char * time);
void updatePin(String pin, int x, int y);
void updateAttempts(int attempts, int x, int y);
void updateSelection(int selection_id);
int getSelectionId(States state, int selection);
void waitReady();

void initEink() {
    Serial.printf("EINK display initialisation\n");
    pinMode(EPD_BUSY, INPUT_PULLDOWN);

    // set up communication
    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI);
    display.init(115200, true, 2, false, SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    u8g2Fonts.begin(display);

    if (display.pages() > 1) {
        Serial.print("Eink display: pages = ");
        Serial.print(display.pages());
        Serial.print(" page height = ");
        Serial.println(display.pageHeight());
    }

    // set default settings
    display.setRotation(3);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
    display.setPartialWindow(0, 0, display.width(), display.height());
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

    // show init screen
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawRect(0, 0, display.width(), display.height()-6, GxEPD_BLACK);
        initScreenTemplate("Petr Zerzan");
    } while (display.nextPage());
}

void displayNotification(notificationScreenId id, int param, int duration) {
    notification_t * notification = (notification_t *)malloc(sizeof(notification_t));
    if (notification == NULL) {
        esplogW(TAG_LIB_DISPLAY, "(displayNotification)", "Failed to allocate memory for notification queue!");
        return;
    }

    notification->id = id;
    notification->param = param;
    notification->duration = duration;

    if (xQueueSend(queueNotification, &notification, pdMS_TO_TICKS(10)) != pdPASS) {
        esplogW(TAG_LIB_MQTT, "(displayNotification)", "Failed to send notification to queue!");
        free(notification);
        return;
    } else {
        esplogI(TAG_LIB_MQTT, "(displayNotification)", "Notification has been enqueued! (id: %d)", *notification);
    }
}

void displayNotificationHandler(notificationScreenId notification, int param) {
    char string[48];
    switch (notification) {
        case NOTIFICATION_AUTH_CHECK_SUCCESS:
            notificationScreenTemplate("Correct PIN", "Access permited!");
            break;
        case NOTIFICATION_AUTH_CHECK_ERROR:
            notificationScreenTemplate("Wrong PIN", "Access denied!");
            break;
        case NOTIFICATION_AUTH_SET_SUCCESS:
            notificationScreenTemplate("PIN set", "New PIN was set!");
            break;
        case NOTIFICATION_AUTH_SET_ERROR:
            notificationScreenTemplate("PIN error", "PIN set failed!");
            break;
        case NOTIFICATION_RFID_CHECK_SUCCESS:
            notificationScreenTemplate("Correct RFID", "RFID card recognised!");
            break;
        case NOTIFICATION_RFID_CHECK_ERROR:
            notificationScreenTemplate("Wrong RFID", "RFID card not recognised!");
            break;
        case NOTIFICATION_RFID_ADD_SUCCESS:
            notificationScreenTemplate("RFID added", "RFID card added!");
            break;
        case NOTIFICATION_RFID_ADD_ERROR:
            notificationScreenTemplate("RFID add error", "RFID card add failed!");
            break;
        case NOTIFICATION_RFID_DEL_SUCCESS:
            notificationScreenTemplate("RFID deleted", "RFID card deleted!");
            break;
        case NOTIFICATION_RFID_DEL_ERROR:
            notificationScreenTemplate("RFID delete error", "RFID card delete failed!");
            break;
        case NOTIFICATION_ZIGBEE_NET_OPEN:
            sprintf(string, "network joining is now open for %d seconds!", param);
            notificationScreenTemplate("ZIGBEE open", string);
            break;
        case NOTIFICATION_ZIGBEE_NET_CLOSE:
            notificationScreenTemplate("ZIGBEE closed", "network joining is now closed!");
            break;
        case NOTIFICATION_ZIGBEE_NET_CLEAR:
            notificationScreenTemplate("ZIGBEE cleared", "network has been cleaned!");
            break;
        case NOTIFICATION_ZIGBEE_NET_RESET:
            break;
        case NOTIFICATION_ZIGBEE_ATTR_REPORT:
            notificationScreenTemplate("ZIGBEE report", "alarm event has been triggered!");
            break;
        case NOTIFICATION_ZIGBEE_DEV_ANNCE:
            notificationScreenTemplate("ZIGBEE join", "zigbee device has joined network!");
            break;
        case NOTIFICATION_ZIGBEE_DEV_LEAVE:
            notificationScreenTemplate("ZIGBEE leave", "zigbee device has leaved network!");
            break;
        case NOTIFICATION_ZIGBEE_DEV_COUNT:
            sprintf(string, "%d devices are connected!", param);
            notificationScreenTemplate("ZIGBEE count", string);
            break;
        default:
            break;
    }

    waitReady();
}

void displayRestart() {
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawRect(0, 0, display.width(), display.height()-6, GxEPD_BLACK);
        initScreenTemplate("Rebooting...");
    } while (display.nextPage());
}

void displayLoad() {
    if (g_vars_ptr->refresh_display.refresh) {
        g_vars_ptr->refresh_display.refresh = false;

        display.setPartialWindow(0, 0, display.width(), display.height());
        display.firstPage();
        do {
            display.fillScreen(GxEPD_WHITE);

            // display border rectangle
            display.drawRect(0, 0, display.width(), display.height()-6, GxEPD_BLACK); // <- my screen has obviously different height than class expects

            int selection_id = getSelectionId(g_vars_ptr->state, g_vars_ptr->selection);
            switch (g_vars_ptr->state) {
                case STATE_INIT:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, false, "setup", "alarm", "test mode", "reboot", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_SETUP:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, false, "WiFi setup", "ZIGBEE setup", "RFID setup", "hard reset", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_SETUP_AP:
                    initScreenTemplate("WiFi AP is now active...");
                    break;

                case STATE_SETUP_HARD_RESET:
                    initScreenTemplate("Please confirm hard reset...");
                    break;

                case STATE_SETUP_RFID_ADD:
                case STATE_SETUP_RFID_DEL:
                case STATE_SETUP_RFID_CHECK:
                    rfidScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, insert RFID card:", "", g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_IDLE:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, false, "lock", "PIN setup", "reboot", "", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_IDLE:
                    menuScreenTemplate(getStateText(g_vars_ptr->state, true), selection_id, true, "lock", "PIN setup", "reboot", "", g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_OK:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: OK", "events", g_vars_ptr->pin.c_str(), g_vars_ptr->attempts, g_vars_ptr->alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_OK:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: OK", "events", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_C:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: STARTING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_C:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: STARTING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_W:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: WARNING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_e_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_W:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: WARNING", "remaining", g_vars_ptr->pin, g_vars_ptr->attempts, (g_config_ptr->alarm_e_countdown_s*1000-g_vars_ptr->time_temp)/1000, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_E:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), false, "status: EMERGENCY", "events", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_TEST_E:
                    alarmScreenTemplate(getStateText(g_vars_ptr->state, true), true, "status: EMERGENCY", "events", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->alarm_events, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_SETUP_HARD_RESET_ENTER_PIN:
                case STATE_SETUP_AP_ENTER_PIN:
                case STATE_SETUP_RFID_ADD_ENTER_PIN:
                case STATE_SETUP_RFID_DEL_ENTER_PIN:
                case STATE_ALARM_LOCK_ENTER_PIN:
                case STATE_TEST_LOCK_ENTER_PIN:
                case STATE_ALARM_UNLOCK_ENTER_PIN:
                case STATE_TEST_UNLOCK_ENTER_PIN:
                case STATE_ALARM_CHANGE_ENTER_PIN1:
                case STATE_TEST_CHANGE_ENTER_PIN1:
                case STATE_SETUP_PIN1:
                    authScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, type in PIN code,", "or use RFID card:", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_CHANGE_ENTER_PIN2:
                case STATE_TEST_CHANGE_ENTER_PIN2:
                case STATE_SETUP_PIN2:
                    authScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, type in new PIN code:", "", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                case STATE_ALARM_CHANGE_ENTER_PIN3:
                case STATE_TEST_CHANGE_ENTER_PIN3:
                case STATE_SETUP_PIN3:
                    authScreenTemplate(getStateText(g_vars_ptr->state, true), false, "Please, repeat previously", "set PIN code:", g_vars_ptr->pin, g_vars_ptr->attempts, g_vars_ptr->time.c_str(), g_vars_ptr->date.c_str(), g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
                    break;

                default:
                    // lcd.print("State was not recognised!");
                    esplogW(TAG_LIB_DISPLAY, "(loadScreen)", "Unrecognised state for loading display data!\n");
                    break;
            }

        } while (display.nextPage());
    }

    else {
        String data;
        int x, y;

        if (g_vars_ptr->refresh_display.refresh_selection) {
            g_vars_ptr->refresh_display.refresh_selection = false;

            // selection
            display.setPartialWindow(10, 32, 20, 80);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                updateSelection(getSelectionId(g_vars_ptr->state, g_vars_ptr->selection));
            } while (display.nextPage());

            // special selections for state STATE_SETUP
            // TODO add the selections for zigbee
            if (g_vars_ptr->state == STATE_SETUP) {
                display.setPartialWindow(30, 48, 150, 40);
                // display.setPartialWindow(30, 72, 150, 16);
                display.firstPage();
                do {
                    display.fillScreen(GxEPD_WHITE);
                    u8g2Fonts.setFont(u8g2_font_courB10_tr);

                    u8g2Fonts.setCursor(36, 65);
                    switch (g_vars_ptr->selection) {
                        case SELECTION_SETUP_OPEN_ZB:
                            u8g2Fonts.print("ZIGBEE open");
                            break;
                        case SELECTION_SETUP_CLOSE_ZB:
                            u8g2Fonts.print("ZIGBEE close");
                            break;
                        case SELECTION_SETUP_CLEAR_ZB:
                            u8g2Fonts.print("ZIGBEE clear");
                            break;
                        case SELECTION_SETUP_RESET_ZB:
                            u8g2Fonts.print("ZIGBEE reset");
                            break;
                        
                        default:
                            u8g2Fonts.print("ZIGBEE setup");
                            break;
                    }


                    u8g2Fonts.setCursor(36, 83);
                    switch (g_vars_ptr->selection) {
                        case SELECTION_SETUP_ADD_RFID:
                            u8g2Fonts.print("RFID add");
                            break;
                        case SELECTION_SETUP_DEL_RFID:
                            u8g2Fonts.print("RFID remove");
                            break;
                        case SELECTION_SETUP_CHECK_RFID:
                            u8g2Fonts.print("RFID check");
                            break;
                        
                        default:
                            u8g2Fonts.print("RFID setup");
                            break;
                    }
                } while (display.nextPage());
            }
        }

        if (g_vars_ptr->refresh_display.refresh_status) {
            g_vars_ptr->refresh_display.refresh_status = false;

            // status icons
            display.setPartialWindow(200, 8, 44, 16);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                updateStatusIcons(g_vars_ptr->wifi_strength, g_vars_ptr->gsm_strength, g_vars_ptr->battery_level);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_datetime) {
            g_vars_ptr->refresh_display.refresh_datetime = false;
            // datetime
            display.setPartialWindow(180, 88, 64, 32);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                updateDatetime(g_vars_ptr->date.c_str(), g_vars_ptr->time.c_str());
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_pin) {
            g_vars_ptr->refresh_display.refresh_pin = false;

            // pin
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                switch (g_vars_ptr->state) {
                    case STATE_SETUP_HARD_RESET_ENTER_PIN:
                    case STATE_SETUP_AP_ENTER_PIN:
                    case STATE_SETUP_RFID_ADD_ENTER_PIN:
                    case STATE_SETUP_RFID_DEL_ENTER_PIN:
                    case STATE_ALARM_LOCK_ENTER_PIN:
                    case STATE_TEST_LOCK_ENTER_PIN:
                    case STATE_ALARM_UNLOCK_ENTER_PIN:
                    case STATE_TEST_UNLOCK_ENTER_PIN:
                    case STATE_ALARM_CHANGE_ENTER_PIN1:
                    case STATE_TEST_CHANGE_ENTER_PIN1:
                    case STATE_SETUP_PIN1:
                    case STATE_ALARM_CHANGE_ENTER_PIN2:
                    case STATE_TEST_CHANGE_ENTER_PIN2:
                    case STATE_SETUP_PIN2:
                    case STATE_ALARM_CHANGE_ENTER_PIN3:
                    case STATE_TEST_CHANGE_ENTER_PIN3:
                    case STATE_SETUP_PIN3:
                        display.setPartialWindow(20, 64, 180, 24);
                        x = 20; y = 82;
                        break;

                    case STATE_ALARM_OK:
                    case STATE_TEST_OK:
                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                    case STATE_ALARM_E:
                    case STATE_TEST_E:
                        display.setPartialWindow(20, 72, 180, 24);
                        x = 20; y = 94;
                        break;
                    
                    default:
                        return;
                }
                updatePin(g_vars_ptr->pin, x, y);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_attempts) {
            g_vars_ptr->refresh_display.refresh_attempts = false;

            // attempts
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                switch (g_vars_ptr->state) {
                    case STATE_SETUP_HARD_RESET_ENTER_PIN:
                    case STATE_SETUP_AP_ENTER_PIN:
                    case STATE_SETUP_RFID_ADD_ENTER_PIN:
                    case STATE_SETUP_RFID_DEL_ENTER_PIN:
                    case STATE_ALARM_LOCK_ENTER_PIN:
                    case STATE_TEST_LOCK_ENTER_PIN:
                    case STATE_ALARM_UNLOCK_ENTER_PIN:
                    case STATE_TEST_UNLOCK_ENTER_PIN:
                    case STATE_ALARM_CHANGE_ENTER_PIN1:
                    case STATE_TEST_CHANGE_ENTER_PIN1:
                    case STATE_SETUP_PIN1:
                    case STATE_ALARM_CHANGE_ENTER_PIN2:
                    case STATE_TEST_CHANGE_ENTER_PIN2:
                    case STATE_SETUP_PIN2:
                    case STATE_ALARM_CHANGE_ENTER_PIN3:
                    case STATE_TEST_CHANGE_ENTER_PIN3:
                    case STATE_SETUP_PIN3:
                        display.setPartialWindow(20, 96, 130, 16);
                        x = 20; y = 102;
                        break;

                    case STATE_ALARM_OK:
                    case STATE_TEST_OK:
                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                    case STATE_ALARM_E:
                    case STATE_TEST_E:
                        display.setPartialWindow(20, 104, 130, 16);
                        x = 20; y = 112;
                        break;
                    
                    default:
                        return;
                }
                updateAttempts(g_vars_ptr->attempts, x, y);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_countdown) {
            g_vars_ptr->refresh_display.refresh_countdown = false;

            // events / countdown
            display.setPartialWindow(20, 32, 140, 16);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                u8g2Fonts.setFont(u8g2_font_courB10_tr);
                u8g2Fonts.setCursor(20, 42);
                switch (g_vars_ptr->state) {
                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                        data = String(String("remaining: ") + String((g_config_ptr->alarm_countdown_s*1000-g_vars_ptr->time_temp)/1000));
                        break;

                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                        data = String(String("remaining: ") + String((g_config_ptr->alarm_e_countdown_s*1000-g_vars_ptr->time_temp)/1000));
                        break;

                    default:
                        return;
                }
                u8g2Fonts.print(data);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_events) {
            g_vars_ptr->refresh_display.refresh_events = false;

            // events / countdown
            display.setPartialWindow(20, 32, 140, 16);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                u8g2Fonts.setFont(u8g2_font_courB10_tr);
                u8g2Fonts.setCursor(20, 42);
                data = String(String("events: ") + String(g_vars_ptr->alarm_events));
                u8g2Fonts.print(data);
            } while (display.nextPage());
        }

        if (g_vars_ptr->refresh_display.refresh_alarm_status) {
            g_vars_ptr->refresh_display.refresh_alarm_status = false;

            // alarm status
            display.setPartialWindow(20, 56, 140, 16);
            display.firstPage();
            do {
                display.fillScreen(GxEPD_WHITE);
                u8g2Fonts.setFont(u8g2_font_courB10_tr);
                u8g2Fonts.setCursor(20, 60);
                switch (g_vars_ptr->state) {
                    case STATE_ALARM_OK:
                    case STATE_TEST_OK:
                        data = String("status: OK");
                        break;

                    case STATE_ALARM_C:
                    case STATE_TEST_C:
                        data = String("status: STARTING");

                    case STATE_ALARM_W:
                    case STATE_TEST_W:
                        data = String("status: WARNING");

                    case STATE_ALARM_E:
                    case STATE_TEST_E:
                        data = String("status: EMERGENCY");
                        break;

                    default:
                        return;
                }
                u8g2Fonts.print(data);
            } while (display.nextPage());
        }   
    }

    waitReady();
}

// ---------------------------------------------------------------------------
// TEMPLATE FUNCTIONS

void menuScreenTemplate(const char * label, int selection_id, bool test, const char * option1, const char * option2, const char * option3, const char * option4, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h;
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(7, 18);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25, display.width() - 10, GxEPD_BLACK);

    // options
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(36, 47);
    u8g2Fonts.print(option1);
    u8g2Fonts.setCursor(36, 65);
    u8g2Fonts.print(option2);
    u8g2Fonts.setCursor(36, 83);
    u8g2Fonts.print(option3);
    u8g2Fonts.setCursor(36, 101);
    u8g2Fonts.print(option4);

    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36);
        u8g2Fonts.print("(testing mode)");   
    }

    // date & time
    updateDatetime(date, time);

    // selection
    updateSelection(selection_id);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void rfidScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, int attempts, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h; 
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(5, 18);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25, display.width() - 10, GxEPD_BLACK);

    // instructions
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 36);
    u8g2Fonts.print(instructions1);
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 48);
    u8g2Fonts.print(instructions2);

    // attempts
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(20, 102);
    u8g2Fonts.printf("attempts: %d", attempts);
    
    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36);
        u8g2Fonts.print("(testing mode)");
    }

    // date & time
    updateDatetime(date, time);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void authScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, String pin, int attempts, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h; 
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(5, 18);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25, display.width() - 10, GxEPD_BLACK);

    // instructions
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 36);
    u8g2Fonts.print(instructions1);
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(7, 48);
    u8g2Fonts.print(instructions2);
    
    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36);
        u8g2Fonts.print("(testing mode)");
    }

    // pin
    updatePin(pin, 20, 82);

    // attempts
    updateAttempts(attempts, 20, 102);

    // date & time
    updateDatetime(date, time);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void alarmScreenTemplate(const char * label, bool test, const char * status, const char * data, String pin, int attempts, int data_load, const char * time, const char * date, int wifi, int gsm, int battery) {
    uint16_t x, y, w, h; 
    int16_t tx, ty, tw, th;

    // main label
    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    u8g2Fonts.setCursor(7, 18);
    u8g2Fonts.print(label);
    display.drawFastHLine(5, 25, display.width() - 10, GxEPD_BLACK);

    // data (events / countdown)
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(20, 42);
    u8g2Fonts.printf("%s: %d", data, data_load);
    u8g2Fonts.setCursor(20, 60);
    u8g2Fonts.print(status);

    // testing mode label
    if (test) {
        u8g2Fonts.setFont(u8g2_font_courB08_tr);
        u8g2Fonts.setCursor(163, 36);
        u8g2Fonts.print("(testing mode)");   
    }

    // attempts
    updateAttempts(attempts, 20, 112);

    // pin
    updatePin(pin, 20, 94);

    // date & time
    updateDatetime(date, time);

    // status icons
    updateStatusIcons(wifi, gsm, battery);
}

void initScreenTemplate(const char * label) {
    uint16_t x, y, w, h;
    int16_t tx, ty, tw, th;

    u8g2Fonts.setFont(u8g2_font_maniac_tr);
    tw = u8g2Fonts.getUTF8Width("IoT Alarm");
    th = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
    tx = (display.width() - tw)/2;
    ty = 40;
    u8g2Fonts.setCursor(tx, ty);
    u8g2Fonts.println("IoT Alarm");

    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    tw = u8g2Fonts.getUTF8Width("version 1.0");
    th = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
    tx = (display.width() - tw)/2;
    ty = 60;
    u8g2Fonts.setCursor(tx, ty);
    u8g2Fonts.println("version 1.0");

    tw = u8g2Fonts.getUTF8Width(label);
    th = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
    tx = (display.width() - tw)/2;
    ty = 105;
    u8g2Fonts.setCursor(tx, ty);
    u8g2Fonts.println(label);
}

void notificationScreenTemplate(const char * label, const char * data) {
    uint16_t x, y, w, h;
    int16_t tx1, ty1, tw1, th1;
    int16_t tx2, ty2, tw2, th2;

    u8g2Fonts.setFont(u8g2_font_courB14_tr);
    tw1 = u8g2Fonts.getUTF8Width(label);
    th1 = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
    
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    tw2 = u8g2Fonts.getUTF8Width(data);
    th2 = (u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());

    w = (tw1 > tw2 ? tw1 : tw2) + 8;
    w = w > display.width() ? display.width() : w;
    h = th1 + th2 + 4 + 8;
    h = h + (8 - h%8);
    h = h > display.height()-6 ? display.height()-6 : h;
    x = (display.width() - w)/2;
    y = (display.height()-6 - h)/2;
    y = y + (8 - y%8);

    tx1 = (display.width() - tw1)/2;
    ty1 = y + th1 + 4;

    tx2 = (display.width() - tw2)/2;
    ty2 = y + th1 + th2 + 4 + 4;

    // Serial.printf("%d %d %d %d\n", tw1, th1, tw2, th2);
    // Serial.printf("%d %d %d %d\n", x, y, w, h);
    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawRect(x, y, w, h, GxEPD_BLACK);

        u8g2Fonts.setFont(u8g2_font_courB14_tr);
        u8g2Fonts.setCursor(tx1, ty1);
        u8g2Fonts.print(label);
        u8g2Fonts.setFont(u8g2_font_courB10_tr);
        u8g2Fonts.setCursor(tx2, ty2);
        u8g2Fonts.print(data);
        // todo bitmaps
    } while (display.nextPage());
}

// ---------------------------------------------------------------------------
// UPDATE FUNCTIONS

void updateSelection(int selection_id) {
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    if (selection_id < 0) {
        u8g2Fonts.setCursor(10, 47);
        u8g2Fonts.print("<");
    } else {
        switch (selection_id) {
            case 0:
                u8g2Fonts.setCursor(20, 47);
                break;
            case 1:
                u8g2Fonts.setCursor(20, 65);
                break;
            case 2:
                u8g2Fonts.setCursor(20, 83);
                break;
            case 3:
                u8g2Fonts.setCursor(20, 101);
                break;
        }
        u8g2Fonts.print(">");
    }
}

void updateDatetime(const char * date, const char * time) {
    u8g2Fonts.setFont(u8g2_font_courB08_tr);
    u8g2Fonts.setCursor(185, 115);
    u8g2Fonts.print(date);
    u8g2Fonts.setCursor(215, 101);
    u8g2Fonts.print(time);
}

void updatePin(String pin, int x, int y) {
    int delimiter = pin.indexOf('#');
    String pin_temp;
    if (delimiter > 0) {
        pin_temp = pin.substring(delimiter+1);
    } else {
        pin_temp = pin;
    }

    String safe_pin;
    int length = pin_temp.length();
    for (int i = 0; i < length; i++) {
        if (pin.charAt(i) == '#') {
            safe_pin+="";
        } else {
            safe_pin+="#";
        }
    }

    u8g2Fonts.setFont(u8g2_font_courB18_tr);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.printf("PIN:%s", safe_pin);
}

void updateAttempts(int attempts, int x, int y) {
    u8g2Fonts.setFont(u8g2_font_courB10_tr);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.printf("attempts: %d", attempts);
}

void updateStatusIcons(int wifi, int gsm, int battery) {
    u8g2Fonts.setFont(u8g2_font_siji_t_6x10);
    u8g2Fonts.setCursor(232, 16);
    if (wifi > 0) {
        // wifi is not connected
        u8g2Fonts.print("\ue217");
    }

    if (wifi > -60) {
        u8g2Fonts.print("\ue21a");
    } else if (wifi > -70) {
        u8g2Fonts.print("\ue219");
    } else if (wifi > -85) {
        u8g2Fonts.print("\ue218");
    } else {
        u8g2Fonts.print("\ue217");
    }

    u8g2Fonts.setCursor(217, 16);
    switch (battery) {
        case 1:
            u8g2Fonts.print("\ue24d");
            break;
        case 2:
            u8g2Fonts.print("\ue24e");
            break;
        case 3:
            u8g2Fonts.print("\ue24f");
            break;
        case 4:
            u8g2Fonts.print("\ue250");
            break;
        case 5:
            u8g2Fonts.print("\ue251");
            break;
        case 6:
            u8g2Fonts.print("\ue252");
            break;
        case 7:
            u8g2Fonts.print("\ue253");
            break;
        case 8:
            u8g2Fonts.print("\ue254");
            break;
        case 0:
        default:
            u8g2Fonts.print("\ue24c");
            break;
    }

    u8g2Fonts.setCursor(202, 16);
    if (gsm == 99) {
        // todo
        // signal not known or detectable
    }
    if (gsm < 6) {
        u8g2Fonts.print("\ue25c");
    } else if (gsm < 12) {
        u8g2Fonts.print("\ue25b");
    } else if (gsm < 18) {
        u8g2Fonts.print("\ue25a");
    } else if (gsm < 24) {
        u8g2Fonts.print("\ue259");
    } else {
        u8g2Fonts.print("\ue258");
    }
}

// ---------------------------------------------------------------------------
// HELPER FUNCTIONS

int getSelectionId(States state, int selection) {
    switch (state) {
        case STATE_INIT:
        switch (selection) {
            case SELECTION_INIT_SETUP:                  return 0;
            case SELECTION_INIT_ALARM:                  return 1;
            case SELECTION_INIT_TEST:                   return 2;
            case SELECTION_INIT_REBOOT:                 return 3;
            default:                                    return -2;
        }

        case STATE_SETUP:
        switch (selection) {
            case SELECTION_SETUP_START_STA:             return 0;
            case SELECTION_SETUP_OPEN_ZB:               return 1;
            case SELECTION_SETUP_CLOSE_ZB:              return 1;
            case SELECTION_SETUP_CLEAR_ZB:              return 1;
            case SELECTION_SETUP_RESET_ZB:              return 1;
            case SELECTION_SETUP_ADD_RFID:              return 2;
            case SELECTION_SETUP_DEL_RFID:              return 2;
            case SELECTION_SETUP_CHECK_RFID:            return 2;
            case SELECTION_SETUP_HARD_RESET:            return 3;
            case SELECTION_SETUP_RETURN:                return -1;
            default:                                    return -2;
        }

        case STATE_ALARM_IDLE:
        switch (selection) {
            case SELECTION_ALARM_IDLE_LOCK:             return 0;
            case SELECTION_ALARM_IDLE_CHANGE_PASSWORD:  return 1;
            case SELECTION_ALARM_IDLE_REBOOT:           return 2;
            case SELECTION_ALARM_IDLE_RETURN:           return -1;
            default:                                    return -2;
        }

        case STATE_TEST_IDLE:
        switch (selection) {
            case SELECTION_TEST_IDLE_LOCK:             return 0;
            case SELECTION_TEST_IDLE_CHANGE_PASSWORD:  return 1;
            case SELECTION_TEST_IDLE_REBOOT:           return 2;
            case SELECTION_TEST_IDLE_RETURN:           return -1;
            default:                                   return -2;
        }
        
        default:
            return -3;
    }
}

void waitReady() {
    while (digitalRead(EPD_BUSY)) {
        vTaskDelay(75 / portTICK_PERIOD_MS);
    }
    return;
}
