/**
 * @file mainAppDefinitions.h
 * @brief Contains all definitions and enumerations for IoT Alarm main application.
 * 
 * Contains all definitions and enumerations for IoT Alarm main application.
 */

#ifndef MAINAPPDEFINITIONS_H_DEFINITION
#define MAINAPPDEFINITIONS_H_DEFINITION

// main app states
enum States {
  STATE_INIT,                           // initial state of the aplication
  STATE_SETUP,                          // setup menu state
  STATE_SETUP_AP,                       // AP wifi setup state
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
  STATE_TEST_CHANGE_ENTER_PIN3,        // alarm change pin confirm state
  STATE_TEST_OK,                        // test running - ok state
  STATE_TEST_C,                         // test starting
  STATE_TEST_W,                         // test running - warning state
  STATE_TEST_E,                         // test running - emergency state
  STATE_MAX,
};

// function to get the state name
inline const char* getStateName(States state) {
    switch(state) {
        case STATE_INIT: return "STATE_INIT";
        case STATE_SETUP: return "STATE_SETUP";
        case STATE_SETUP_AP: return "STATE_SETUP_AP";
        case STATE_ALARM_IDLE: return "STATE_ALARM_IDLE";
        case STATE_ALARM_LOCK_ENTER_PIN: return "STATE_ALARM_LOCK_ENTER_PIN";
        case STATE_ALARM_UNLOCK_ENTER_PIN: return "STATE_ALARM_UNLOCK_ENTER_PIN";
        case STATE_ALARM_CHANGE_ENTER_PIN1: return "STATE_ALARM_CHANGE_ENTER_PIN1";
        case STATE_ALARM_CHANGE_ENTER_PIN2: return "STATE_ALARM_CHANGE_ENTER_PIN2";
        case STATE_ALARM_CHANGE_ENTER_PIN3: return "STATE_ALARM_CHANGE_ENTER_PIN3";
        case STATE_ALARM_OK: return "STATE_ALARM_OK";
        case STATE_ALARM_C: return "STATE_ALARM_C";
        case STATE_ALARM_W: return "STATE_ALARM_W";
        case STATE_ALARM_E: return "STATE_ALARM_E";
        case STATE_TEST_IDLE: return "STATE_TEST_IDLE";
        case STATE_TEST_LOCK_ENTER_PIN: return "STATE_TEST_LOCK_ENTER_PIN";
        case STATE_TEST_UNLOCK_ENTER_PIN: return "STATE_TEST_UNLOCK_ENTER_PIN";
        case STATE_TEST_CHANGE_ENTER_PIN1: return "STATE_TEST_CHANGE_ENTER_PIN1";
        case STATE_TEST_CHANGE_ENTER_PIN2: return "STATE_TEST_CHANGE_ENTER_PIN2";
        case STATE_TEST_CHANGE_ENTER_PIN3: return "STATE_TEST_CHANGE_ENTER_PIN3";
        case STATE_TEST_OK: return "STATE_TEST_OK";
        case STATE_TEST_C: return "STATE_TEST_C";
        case STATE_TEST_W: return "STATE_TEST_W";
        case STATE_TEST_E: return "STATE_TEST_E";
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
    SELECTION_SETUP_HARD_RESET,
    SELECTION_SETUP_RETURN,
    SELECTION_SETUP_MAX,
};

// selection menu for STATE_ALARM_IDLE
enum selectionAlarmIdle {
    SELECTION_ALARM_IDLE_LOCK,
    SELECTION_ALARM_IDLE_CHANGE_PASSWORD,
    SELECTION_ALARM_IDLE_RETURN,
    SELECTION_ALARM_IDLE_REBOOT,
    SELECTION_ALARM_IDLE_MAX,
};

// selection menu for STATE_TEST_IDLE
enum selectionTestIdle {
    SELECTION_TEST_IDLE_LOCK,
    SELECTION_TEST_IDLE_CHANGE_PASSWORD,
    SELECTION_TEST_IDLE_RETURN,
    SELECTION_TEST_IDLE_REBOOT,
    SELECTION_TEST_IDLE_MAX,
};

// function to get the selection name based on state and selection
inline const char* getSelectionName(States state, int selection) {
    switch(state) {
        case STATE_INIT: {
            switch(static_cast<selectionInit>(selection)) {
                case SELECTION_INIT_SETUP: return "SELECTION_INIT_SETUP";
                case SELECTION_INIT_ALARM: return "SELECTION_INIT_ALARM";
                case SELECTION_INIT_TEST: return "SELECTION_INIT_TEST";
                case SELECTION_INIT_REBOOT: return "SELECTION_INIT_REBOOT";
                default: return "Unknown Selection";
            }
        }
        case STATE_SETUP: {
            switch(static_cast<selectionSetup>(selection)) {
                case SELECTION_SETUP_START_STA: return "SELECTION_SETUP_START_STA";
                case SELECTION_SETUP_HARD_RESET: return "SELECTION_SETUP_HARD_RESET";
                case SELECTION_SETUP_RETURN: return "SELECTION_SETUP_RETURN";
                default: return "Unknown Selection";
            }
        }
        case STATE_ALARM_IDLE: {
            switch(static_cast<selectionAlarmIdle>(selection)) {
                case SELECTION_ALARM_IDLE_LOCK: return "SELECTION_ALARM_IDLE_LOCK";
                case SELECTION_ALARM_IDLE_CHANGE_PASSWORD: return "SELECTION_ALARM_IDLE_CHANGE_PASSWORD";
                case SELECTION_ALARM_IDLE_RETURN: return "SELECTION_ALARM_IDLE_RETURN";
                case SELECTION_ALARM_IDLE_REBOOT: return "SELECTION_ALARM_IDLE_REBOOT";
                default: return "Unknown Selection";
            }
        }
        case STATE_TEST_IDLE: {
            switch(static_cast<selectionTestIdle>(selection)) {
                case SELECTION_TEST_IDLE_LOCK: return "SELECTION_TEST_IDLE_LOCK";
                case SELECTION_TEST_IDLE_CHANGE_PASSWORD: return "SELECTION_TEST_IDLE_CHANGE_PASSWORD";
                case SELECTION_TEST_IDLE_RETURN: return "SELECTION_TEST_IDLE_RETURN";
                case SELECTION_TEST_IDLE_REBOOT: return "SELECTION_TEST_IDLE_REBOOT";
                default: return "Unknown Selection";
            }
        }
        default: return "Unknown State";
    }
}

struct g_vars_t {
    States state;
    int selection;
    int selection_max;

    bool confirm;
    bool refresh;

    int wifi_status;
    int wifi_mode;
    int gprs;
    bool electricity;

    String pin;
    int attempts;
};

// every param needs to be added and handeled in:
//  - wifimanager.html
//  - libJson.cpp (loadConfig(), saveConfig(), setDefaultConfig())
//  - libWiFi.cpp (startWifiSetupMode())
struct g_config_t {
    String wifi_ssid;
    String wifi_pswd;
    String wifi_ip;
    String wifi_gtw;
    String wifi_sbnt;
    int alarm_countdown_s;
};

#endif
