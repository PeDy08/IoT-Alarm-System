/**
 * @file mainAppDefinitions.h
 * @brief Contains all definitions and enumerations for IoT Alarm main application.
 * 
 * Contains all definitions and enumerations for IoT Alarm main application.
 */

#ifndef MAINAPPDEFINITIONS_H_DEFINITION
#define MAINAPPDEFINITIONS_H_DEFINITION

#include <Arduino.h>

// main app states
enum States {
  STATE_INIT,                           // initial state of the aplication
  STATE_SETUP,                          // setup menu state
  STATE_SETUP_AP_ENTER_PIN,
  STATE_SETUP_AP,                       // AP wifi setup state
  STATE_SETUP_HARD_RESET_ENTER_PIN,     // hard reset pin check state
  STATE_SETUP_HARD_RESET,               // hard reset
  STATE_SETUP_PIN1,                     // pin setup check state
  STATE_SETUP_PIN2,                     // pin setup pin state
  STATE_SETUP_PIN3,                     // pin setup pin confirm state
  STATE_SETUP_RFID_ADD,                 // rfid setup add
  STATE_SETUP_RFID_ADD_ENTER_PIN,       // rfid setup add check state
  STATE_SETUP_RFID_DEL,                 // rfid setup del
  STATE_SETUP_RFID_DEL_ENTER_PIN,       // rfid setup del check state
  STATE_SETUP_RFID_CHECK,               // rfid setup check
  STATE_ALARM_IDLE,                     // alarm menu state
  STATE_ALARM_LOCK_ENTER_PIN,           // alarm lock check state
  STATE_ALARM_UNLOCK_ENTER_PIN,         // alarm unlock check state
  STATE_ALARM_CHANGE_ENTER_PIN1,        // alarm change pin check state
  STATE_ALARM_CHANGE_ENTER_PIN2,        // alarm change pin state
  STATE_ALARM_CHANGE_ENTER_PIN3,        // alarm change pin confirm state
  STATE_ALARM_OK,                       // alarm running - ok state
  STATE_ALARM_C,                        // alarm starting
  STATE_ALARM_W,                        // alarm running - warning state
  STATE_ALARM_E,                        // alarm running - emergency state
  STATE_TEST_IDLE,                      // test menu state
  STATE_TEST_LOCK_ENTER_PIN,            // test lock check state
  STATE_TEST_UNLOCK_ENTER_PIN,          // test unlock check state
  STATE_TEST_CHANGE_ENTER_PIN1,         // test change pin check state
  STATE_TEST_CHANGE_ENTER_PIN2,         // test change pin state
  STATE_TEST_CHANGE_ENTER_PIN3,         // alarm change pin confirm state
  STATE_TEST_OK,                        // test running - ok state
  STATE_TEST_C,                         // test starting
  STATE_TEST_W,                         // test running - warning state
  STATE_TEST_E,                         // test running - emergency state
  STATE_MAX,
};

// function to get the state text
inline const char* getStateText(States state, bool pretty = false) {
    switch(state) {
        case STATE_INIT: return pretty ? "Main menu:" : "STATE_INIT";
        case STATE_SETUP: return pretty ? "Setup:" : "STATE_SETUP";
        case STATE_SETUP_AP: return pretty ? "Starting WiFi AP..." : "STATE_SETUP_AP";
        case STATE_SETUP_AP_ENTER_PIN: return pretty ? "Starting WiFi AP..." : "STATE_SETUP_AP_ENTER_PIN";
        case STATE_SETUP_HARD_RESET_ENTER_PIN: return pretty ? "Hard reset..." : "STATE_SETUP_HARD_RESET_ENTER_PIN";
        case STATE_SETUP_HARD_RESET: return pretty ? "Hard reset..." : "STATE_SETUP_HARD_RESET";
        case STATE_SETUP_PIN1: return pretty ? "Setting new pin..." : "STATE_SETUP_PIN1";
        case STATE_SETUP_PIN2: return pretty ? "Setting new pin..." : "STATE_SETUP_PIN2";
        case STATE_SETUP_PIN3: return pretty ? "Setting new pin..." : "STATE_SETUP_PIN3";
        case STATE_SETUP_RFID_ADD: return pretty ? "Adding new RFID..." : "STATE_SETUP_RFID_ADD";
        case STATE_SETUP_RFID_ADD_ENTER_PIN: return pretty ? "Adding new RFID..." : "STATE_SETUP_RFID_ADD_ENTER_PIN";
        case STATE_SETUP_RFID_DEL: return pretty ? "Deleting RFID..." : "STATE_SETUP_RFID_DEL";
        case STATE_SETUP_RFID_DEL_ENTER_PIN: return pretty ? "Deleting RFID..." : "STATE_SETUP_RFID_DEL_ENTER_PIN";
        case STATE_SETUP_RFID_CHECK: return pretty ? "Check RFID..." : "STATE_SETUP_RFID_CHECK";
        case STATE_ALARM_IDLE: return pretty ? "Alarm menu:" : "STATE_ALARM_IDLE";
        case STATE_ALARM_LOCK_ENTER_PIN: return pretty ? "Locking alarm..." : "STATE_ALARM_LOCK_ENTER_PIN";
        case STATE_ALARM_UNLOCK_ENTER_PIN: return pretty ? "Unlocking alarm..." : "STATE_ALARM_UNLOCK_ENTER_PIN";
        case STATE_ALARM_CHANGE_ENTER_PIN1: return pretty ? "Setting new pin..." : "STATE_ALARM_CHANGE_ENTER_PIN1";
        case STATE_ALARM_CHANGE_ENTER_PIN2: return pretty ? "Setting new pin..." : "STATE_ALARM_CHANGE_ENTER_PIN2";
        case STATE_ALARM_CHANGE_ENTER_PIN3: return pretty ? "Setting new pin..." : "STATE_ALARM_CHANGE_ENTER_PIN3";
        case STATE_ALARM_OK: return pretty ? "Alarm ON" : "STATE_ALARM_OK";
        case STATE_ALARM_C: return pretty ? "Starting alarm..." : "STATE_ALARM_C";
        case STATE_ALARM_W: return pretty ? "Alarm ON" : "STATE_ALARM_W";
        case STATE_ALARM_E: return pretty ? "Alarm ON" : "STATE_ALARM_E";
        case STATE_TEST_IDLE: return pretty ? "(T) Alarm menu:" : "STATE_TEST_IDLE";
        case STATE_TEST_LOCK_ENTER_PIN: return pretty ? "(T) Locking alarm..." : "STATE_TEST_LOCK_ENTER_PIN";
        case STATE_TEST_UNLOCK_ENTER_PIN: return pretty ? "(T) Unlocking alarm" : "STATE_TEST_UNLOCK_ENTER_PIN";
        case STATE_TEST_CHANGE_ENTER_PIN1: return pretty ? "(T) Setting new pin" : "STATE_TEST_CHANGE_ENTER_PIN1";
        case STATE_TEST_CHANGE_ENTER_PIN2: return pretty ? "(T) Setting new pin" : "STATE_TEST_CHANGE_ENTER_PIN2";
        case STATE_TEST_CHANGE_ENTER_PIN3: return pretty ? "(T) Setting new pin" : "STATE_TEST_CHANGE_ENTER_PIN3";
        case STATE_TEST_OK: return pretty ? "(T) Alarm ON" : "STATE_TEST_OK";
        case STATE_TEST_C: return pretty ? "(T) Starting alarm" : "STATE_TEST_C";
        case STATE_TEST_W: return pretty ? "(T) Alarm ON" : "STATE_TEST_W";
        case STATE_TEST_E: return pretty ? "(T) Alarm ON" : "STATE_TEST_E";
        default: return "Unknown State";
    }
}

// selection menu for STATE_INIT
enum selectionInit {
    SELECTION_INIT_SETUP,
    SELECTION_INIT_ALARM,
    SELECTION_INIT_TEST,
    SELECTION_INIT_REBOOT,
    SELECTION_INIT_MAX,
};

// selection menu for STATE_SETUP
enum selectionSetup {
    SELECTION_SETUP_START_STA,
    SELECTION_SETUP_OPEN_ZB,
    SELECTION_SETUP_CLOSE_ZB,
    SELECTION_SETUP_CLEAR_ZB,
    SELECTION_SETUP_RESET_ZB,
    SELECTION_SETUP_ADD_RFID,
    SELECTION_SETUP_DEL_RFID,
    SELECTION_SETUP_CHECK_RFID,
    SELECTION_SETUP_HARD_RESET,
    SELECTION_SETUP_RETURN,
    SELECTION_SETUP_MAX,
};

// selection menu for STATE_ALARM_IDLE
enum selectionAlarmIdle {
    SELECTION_ALARM_IDLE_LOCK,
    SELECTION_ALARM_IDLE_CHANGE_PASSWORD,
    SELECTION_ALARM_IDLE_REBOOT,
    SELECTION_ALARM_IDLE_RETURN,
    SELECTION_ALARM_IDLE_MAX,
};

// selection menu for STATE_TEST_IDLE
enum selectionTestIdle {
    SELECTION_TEST_IDLE_LOCK,
    SELECTION_TEST_IDLE_CHANGE_PASSWORD,
    SELECTION_TEST_IDLE_REBOOT,
    SELECTION_TEST_IDLE_RETURN,
    SELECTION_TEST_IDLE_MAX,
};

// function to get the selection text based on state and selection
inline const char* getSelectionText(States state, int selection, bool pretty = false) {
    switch(state) {
        case STATE_INIT: {
            switch(static_cast<selectionInit>(selection)) {
                case SELECTION_INIT_SETUP: return pretty ? "1. setup" : "SELECTION_INIT_SETUP";
                case SELECTION_INIT_ALARM: return pretty ? "2. alarm" : "SELECTION_INIT_ALARM";
                case SELECTION_INIT_TEST: return pretty ? "3. (T) alarm" : "SELECTION_INIT_TEST";
                case SELECTION_INIT_REBOOT: return pretty ? "4. reboot" : "SELECTION_INIT_REBOOT";
                default: return "Unknown Selection";
            }
        }
        case STATE_SETUP: {
            switch(static_cast<selectionSetup>(selection)) {
                case SELECTION_SETUP_START_STA: return pretty ? "1. start WiFi AP" : "SELECTION_SETUP_START_STA";
                case SELECTION_SETUP_OPEN_ZB: return pretty ? "2. open ZIGBEE" : "SELECTION_SETUP_OPEN_ZB";
                case SELECTION_SETUP_CLOSE_ZB: return pretty ? "2. close ZIGBEE" : "SELECTION_SETUP_CLOSE_ZB";
                case SELECTION_SETUP_CLEAR_ZB: return pretty ? "2. clear ZIGBEE" : "SELECTION_SETUP_CLEAR_ZB";
                case SELECTION_SETUP_RESET_ZB: return pretty ? "2. reset ZIGBEE" : "SELECTION_SETUP_RESET_ZB";
                case SELECTION_SETUP_ADD_RFID: return pretty ? "3. add RFID" : "SELECTION_SETUP_ADD_RFID";
                case SELECTION_SETUP_DEL_RFID: return pretty ? "3. delete RFID" : "SELECTION_SETUP_DEL_RFID";
                case SELECTION_SETUP_CHECK_RFID: return pretty ? "3 check RFID" : "SELECTION_SETUP_CHECK_RFID";
                case SELECTION_SETUP_HARD_RESET: return pretty ? "6. hard reset" : "SELECTION_SETUP_HARD_RESET";
                case SELECTION_SETUP_RETURN: return pretty ? "7. return" : "SELECTION_SETUP_RETURN";
                default: return "Unknown Selection";
            }
        }
        case STATE_ALARM_IDLE: {
            switch(static_cast<selectionAlarmIdle>(selection)) {
                case SELECTION_ALARM_IDLE_LOCK: return pretty ? "1. lock alarm" : "SELECTION_ALARM_IDLE_LOCK";
                case SELECTION_ALARM_IDLE_CHANGE_PASSWORD: return pretty ? "2. set PIN" : "SELECTION_ALARM_IDLE_CHANGE_PASSWORD";
                case SELECTION_ALARM_IDLE_RETURN: return pretty ? "3. return" : "SELECTION_ALARM_IDLE_RETURN";
                case SELECTION_ALARM_IDLE_REBOOT: return pretty ? "4. reboot" : "SELECTION_ALARM_IDLE_REBOOT";
                default: return "Unknown Selection";
            }
        }
        case STATE_TEST_IDLE: {
            switch(static_cast<selectionTestIdle>(selection)) {
                case SELECTION_TEST_IDLE_LOCK: return pretty ? "1. lock alarm" : "SELECTION_TEST_IDLE_LOCK";
                case SELECTION_TEST_IDLE_CHANGE_PASSWORD: return pretty ? "2. set PIN" : "SELECTION_TEST_IDLE_CHANGE_PASSWORD";
                case SELECTION_TEST_IDLE_RETURN: return pretty ? "3. return" : "SELECTION_TEST_IDLE_RETURN";
                case SELECTION_TEST_IDLE_REBOOT: return pretty ? "4. reboot" : "SELECTION_TEST_IDLE_REBOOT";
                default: return "Unknown Selection";
            }
        }
        default: return "Unknown State";
    }
}

typedef struct {
    bool refresh;
    bool refresh_selection;
    bool refresh_datetime;
    bool refresh_status;
    bool refresh_pin;
    bool refresh_attempts;
    bool refresh_alarm_status;
    bool refresh_events;
    bool refresh_countdown;
} refresh_display_t;

struct g_vars_t {
    States state;                       // state
    States state_prev;                  // previous state

    int selection;                      // menu selection
    int selection_prev;                 // previous state menu selection

    int selection_max;                  // menu items
    int selection_max_prev;             // previous state menu items

    bool confirm;                       // confirm flag (user input)
    bool abort;                         // abort flag (user input)
    bool refresh;                       // refresh flag (user input)
    refresh_display_t refresh_display;  // refresh flags for display

    int wifi_status;                    // wifi status (connected, disconnected...)
    int wifi_mode;                      // wifi mode (STA, AP)

    int wifi_strength;                  // strength of WiFi signal (RSSI)
    int gsm_strength;                   // strength of gsm signal (RSSI)
    int battery_level;                  // battery percentage
    bool power_mode;                    // power mode flag (powerline or battery)

    unsigned long datetime;             // actual seconds after time epoch
    String date;                        // actual time (HH:MM)
    String time;                        // actual date (DD:MM:YYYY)

    String pin;                         // typed PIN code (user input)
    int attempts;                       // failed attempts for PIN (user input)
    int alarm_events;                   // recorded alarm events (zigbee sensors)
    bool alarm_event_fire;              // detected fire alarm event (zigbee sensors)
    bool alarm_event_water;             // detected water leakage alarm event (zigbee sensors)
    unsigned long time_temp;            // temporary time variable (for countdowns)
};

inline bool refresh_display_any(refresh_display_t refresh_display, g_vars_t g_vars) {
    return  g_vars.refresh_display.refresh ||
            g_vars.refresh_display.refresh_selection ||
            g_vars.refresh_display.refresh_datetime ||
            g_vars.refresh_display.refresh_status ||
            g_vars.refresh_display.refresh_pin ||
            g_vars.refresh_display.refresh_attempts ||
            g_vars.refresh_display.refresh_alarm_status ||
            g_vars.refresh_display.refresh_events ||
            g_vars.refresh_display.refresh_countdown;
}

// every param needs to be added and handeled in:
//  - wifimanager.html
//  - libJson.cpp (loadConfig(), saveConfig(), setDefaultConfig())
//  - libWiFi.cpp (startWifiSetupMode())
struct g_config_t {
    String wifi_ssid;                   // wifi ssid
    String wifi_pswd;                   // wifi password
    String wifi_ip;                     // wifi ip adress
    String wifi_gtw;                    // wifi gateway address
    String wifi_sbnt;                   // wifi subnet mask

    bool mqtt_tls;
    String mqtt_broker;
    uint16_t mqtt_port;
    String mqtt_id;
    String mqtt_topic;
    String mqtt_username;
    String mqtt_password;
    String mqtt_cert;

    int alarm_countdown_s;              // countdown before alarm is started after locking process
    int alarm_e_countdown_s;            // TODO add new param to app - countdown before alarm start emergency notifications after 'alarm_e_threshold' is reached
    int alarm_w_threshold;              // TODO add new param to app - number of alarm events before warning state is triggered
    int alarm_e_threshold;              // TODO add new param to app - number of alarm events before emergency state is triggered

    String alarm_telephone;             // TODO add new param to app - alarm notifiactions telephone numbers
};

#endif
