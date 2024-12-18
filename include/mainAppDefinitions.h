/**
 * @file mainAppDefinitions.h
 * @brief Contains all definitions and enumerations for IoT Alarm main application.
 * 
 * Contains all definitions and enumerations for IoT Alarm main application.
 */

#ifndef MAINAPPDEFINITIONS_H_DEFINITION
#define MAINAPPDEFINITIONS_H_DEFINITION

#include <Arduino.h>

/**
 * @brief Enumerates the different states of the application.
 * 
 * @details This enumeration defines the various states that the application can be in. Each state corresponds to 
 *          a specific phase of operation, from initialization, setup, alarm handling, testing, to reset procedures.
 *          These states help manage the flow of the application and ensure that the system responds appropriately 
 *          to user interactions, alarm status changes, and configuration tasks.
 */
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

/**
 * @brief Enumerates the possible statuses of the alarm system.
 * 
 * @details This enumeration defines the various states of the alarm system, which include its operational status,
 *          countdown phases, and mode types (e.g., testing, emergency, warning). The status indicates whether the alarm 
 *          is active, in countdown, in testing mode, or in a triggered state such as emergency or warning.
 */
enum AlarmStatus {
    ALARM_STATUS_OFF,                   // alarm is turned off, fire/water/electricity notifications still running
    ALARM_STATUS_STARTING,              // alarm initial countdown running
    ALARM_STATUS_OK,                    // alarm is running, warnings has not been triggered
    ALARM_STATUS_WARN,                  // alarm is running, warnings has been triggered and is counting for emergency
    ALARM_STATUS_EMERG,                 // alarm is running, emergency has been triggered
    ALARM_STATUS_TESTING,               // alarm is running in a testing mode
    ALARM_STATUS_MAX,
};

/**
 * @brief Returns a human-readable string representation of the given state.
 *
 * This function provides a string describing the state passed as an argument. 
 * Optionally, it can return a more user-friendly, "pretty" version of the string 
 * when the `pretty` parameter is set to true.
 *
 * @param state The state whose description is to be returned. It is of type `States`, 
 *              which represents various states in the system.
 * @param pretty A boolean flag to indicate whether to return a user-friendly description. 
 *               If true, a more readable string is returned; otherwise, the default state name 
 *               (e.g., "STATE_SETUP") is returned. The default value is false.
 * 
 * @return A constant character pointer to the string representation of the state.
 *         The returned string will either be the name of the state or a more readable version 
 *         depending on the value of the `pretty` parameter.
 * 
 * @details
 * This function handles all states defined in the `States` enum. It maps each enum value to 
 * either a default string representation or a human-friendly description, depending on the 
 * value of the `pretty` flag.
 * 
 * @code
 * // Example usage:
 * const char* stateText = getStateText(STATE_ALARM_OK);
 * Serial.println(stateText); // Output: "Alarm ON" (when pretty = true) or "STATE_ALARM_OK" (when pretty = false)
 * @endcode
 */
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

/**
 * @brief Enum for the selection menu in the `STATE_INIT` state.
 * 
 * This enum defines the different options available in the initialization menu, 
 * allowing the user to choose between setup, alarm, test, and reboot options.
 */
enum selectionInit {
    SELECTION_INIT_SETUP,
    SELECTION_INIT_ALARM,
    SELECTION_INIT_TEST,
    SELECTION_INIT_REBOOT,
    SELECTION_INIT_MAX,
};

/**
 * @brief Enum for the selection menu in the `STATE_SETUP` state.
 * 
 * This enum defines the different options available in the setup menu, allowing 
 * the user to choose between various setup actions, such as starting Wi-Fi, 
 * managing Zigbee settings, adding/removing RFID, and performing resets.
 */
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

/**
 * @brief Enum for the selection menu in the `STATE_ALARM_IDLE` state.
 * 
 * This enum defines the different options available in the idle state of the alarm, 
 * allowing the user to lock the alarm, change the password, reboot the system, 
 * or return to the previous menu.
 */
enum selectionAlarmIdle {
    SELECTION_ALARM_IDLE_LOCK,
    SELECTION_ALARM_IDLE_CHANGE_PASSWORD,
    SELECTION_ALARM_IDLE_REBOOT,
    SELECTION_ALARM_IDLE_RETURN,
    SELECTION_ALARM_IDLE_MAX,
};

/**
 * @brief Enum for the selection menu in the `STATE_TEST_IDLE` state.
 * 
 * This enum defines the different options available in the idle state of the test mode, 
 * allowing the user to lock the test, change the password, reboot the system, 
 * or return to the previous menu.
 */
enum selectionTestIdle {
    SELECTION_TEST_IDLE_LOCK,
    SELECTION_TEST_IDLE_CHANGE_PASSWORD,
    SELECTION_TEST_IDLE_REBOOT,
    SELECTION_TEST_IDLE_RETURN,
    SELECTION_TEST_IDLE_MAX,
};

/**
 * @brief Returns the string representation of a selection option based on the current state and selection.
 * 
 * This function generates a textual representation of a selection in a menu system, formatted according to
 * the current state of the system. The returned string will either be a simple identifier (e.g., "SELECTION_INIT_SETUP")
 * or a more user-friendly text (e.g., "1. setup"), depending on the `pretty` flag.
 * 
 * @param state The current state of the system, which dictates the valid selection options.
 *              - `STATE_INIT` corresponds to the initialization state.
 *              - `STATE_SETUP` corresponds to the setup state.
 *              - `STATE_ALARM_IDLE` corresponds to the alarm idle state.
 *              - `STATE_TEST_IDLE` corresponds to the test idle state.
 * @param selection The selected option within the given state. The value should be one of the enumerated selection options
 *                  that match the current state (e.g., `SELECTION_INIT_SETUP` for the `STATE_INIT` state).
 * @param pretty A boolean flag that controls the format of the returned string. If `true`, the returned string will
 *               be more user-friendly (e.g., "1. setup"), and if `false`, the returned string will be a raw identifier
 *               (e.g., "SELECTION_INIT_SETUP"). Default is `false`.
 * 
 * @return A `const char*` string representing the selection text. If the selection is not recognized, "Unknown Selection"
 *         or "Unknown State" will be returned based on the context.
 * 
 * @details This function processes the `state` and `selection` parameters to map to predefined selection options, which
 *          are grouped by state. The function supports a human-readable format for UI or debug purposes when `pretty` is true.
 *          If an invalid state or selection is provided, the function will return a default error message indicating the unknown
 *          state or selection.
 * 
 * @code
 * // Example usage:
 * const char* setupText = getSelectionText(STATE_SETUP, SELECTION_SETUP_START_STA, true);
 * // Returns: "1. start WiFi AP"
 * 
 * const char* alarmText = getSelectionText(STATE_ALARM_IDLE, SELECTION_ALARM_IDLE_LOCK, false);
 * // Returns: "SELECTION_ALARM_IDLE_LOCK"
 * @endcode
 */
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

typedef struct {
    bool alarm_fire;
    bool alarm_water;
    bool alarm_electricity;
    bool alarm_intrusion;
    int alarm_events;
    bool notification_fire;
    bool notification_water;
    bool notification_electricity;
    bool notification_intrusion;
    bool notification_warning;
    bool notification_emergency;
    AlarmStatus alarm_status;
} alarm_t;

/**
 * @brief Struct representing global system variables for state management and user input.
 * 
 * This struct holds various system states, user input flags, network status, and device settings, 
 * providing a central location for tracking the current state of the system, menu selections, 
 * Wi-Fi and GSM status, battery levels, alarm information, and more.
 */
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
    bool power_mode;                    // power mode flag (true -> powerline, false -> battery)

    unsigned long datetime;             // actual seconds after time epoch
    String date;                        // actual time (HH:MM)
    String time;                        // actual date (DD:MM:YYYY)

    String pin;                         // typed PIN code (user input)
    int attempts;                       // failed attempts for PIN (user input)
    alarm_t alarm;                      // all alarm variables in one struct
    unsigned long time_temp;            // temporary time variable (for countdowns)
};

/**
 * @brief Checks if any display refresh flag is set to true.
 * 
 * This function checks multiple flags within the `g_vars` structure to determine if any display-related refresh
 * action is required. It returns `true` if any of the refresh flags in the `g_vars.refresh_display` structure are set to
 * `true`, indicating that the display should be refreshed, otherwise returns `false`.
 * 
 * @param refresh_display A `refresh_display_t` structure containing the display refresh flags (currently not used
 *                        in the function body but included as part of the signature for potential future use).
 * @param g_vars A `g_vars_t` structure containing various flags under the `refresh_display` substructure that control
 *               whether different parts of the display need to be refreshed.
 * 
 * @return `true` if any of the refresh flags in `g_vars.refresh_display` are set to `true`, otherwise `false`.
 * 
 * @details The function evaluates the following refresh flags within `g_vars.refresh_display`:
 *          - `refresh`: General display refresh.
 *          - `refresh_selection`: Refresh flag for display selection.
 *          - `refresh_datetime`: Refresh flag for the datetime display.
 *          - `refresh_status`: Refresh flag for the status display.
 *          - `refresh_pin`: Refresh flag for PIN-related information.
 *          - `refresh_attempts`: Refresh flag for display related to attempts.
 *          - `refresh_alarm_status`: Refresh flag for the alarm status display.
 *          - `refresh_events`: Refresh flag for event display.
 *          - `refresh_countdown`: Refresh flag for countdown-related display.
 * 
 * @code
 * // Example usage:
 * g_vars_t g_vars;
 * g_vars.refresh_display.refresh = true;
 * bool should_refresh = refresh_display_any(refresh_display, g_vars);
 * // Returns: true since g_vars.refresh_display.refresh is true
 * @endcode
 */
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

/**
 * @brief Struct representing global configuration settings for the system.
 * 
 * This struct stores various configuration parameters related to Wi-Fi, MQTT settings, alarm settings,
 * and emergency notification thresholds, providing an organized way to manage system configurations.
 */
struct g_config_t {
    String wifi_ssid;                   // wifi ssid
    String wifi_pswd;                   // wifi password
    String wifi_ip;                     // wifi ip adress
    String wifi_gtw;                    // wifi gateway address
    String wifi_sbnt;                   // wifi subnet mask

    int mqtt_tls;
    String mqtt_broker;
    int mqtt_port;
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
