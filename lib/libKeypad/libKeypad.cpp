#include "libKeypad.h"

const uint8_t KEYPAD_I2C_ADDRESS = 0x20;
I2CKeyPad keypad(KEYPAD_I2C_ADDRESS);

extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

/**
 * @brief Callback function for default keypad event - menu.
 * 
 * This function can be used when any button is pressed.
 * Checks if any of the buttons controls current menu.
 * 
 * 4, 8     --> down/prev
 * 5, *, #  --> enter/confirm
 * 2, 6     --> up/next
 * 
 */
void keyFxMenu(char key) {
    switch (key) {
        // menu up/next
        case '4':
        case '8':
            g_vars_ptr->selection++;
            cycleSelection(&g_vars_ptr->selection, g_vars_ptr->selection_max);
            break;

        // menu down/prev
        case '2':
        case '6':
            g_vars_ptr->selection--;
            cycleSelection(&g_vars_ptr->selection, g_vars_ptr->selection_max);
            break;

        // menu enter/confirm
        case '5':
        case '#':
            g_vars_ptr->confirm = true;
            break;

        case '*':
            // TODO
            g_vars_ptr->abort = true;
            break;
        
        default:
            break;
    }
}

/**
 * @brief Callback function for keypad event - waiting to press any key.
 * 
 * This function can be used when any button is pressed.
 * Records pressed key. And if so, changes the global vars confirm to true.
 * 
 */
void keyFxConfirm(char key) {
    switch (key) {
        case '*':
            g_vars_ptr->abort = true;
            break;
        
        default:
            g_vars_ptr->confirm = true;
            break;
    }
}

/**
 * @brief Handles the logic for recording key inputs and updating the PIN.
 *
 * This function processes a key input and updates the `pin` stored in `g_vars_ptr` accordingly.
 * - If the key is `#`, it appends the character to the `pin` and sets the `confirm` flag to `true`.
 * - If the key is `*`, it checks if the `pin` is empty or ends with a `#`, in which case it sets the `abort` flag to `true`. Otherwise, it removes the last character from the `pin`.
 * - If the key is a number (0-9), it appends the number to the `pin`.
 * - If the key is one of the special characters `A`, `B`, `C`, or `D`, no action is performed on the `pin`.
 *
 * @param key The character representing the pressed key.
 * - `#`: Confirm the PIN entry.
 * - `*`: Delete the last character or abort if conditions are met.
 * - `0`-`9`: Append the number to the PIN.
 * - `A`-`D`: No action on the PIN.
 * 
 * @return void
 * 
 * @details This function is designed for handling PIN input where special characters are used for confirmation (`#`) and deletion (`*`).
 * If a non-numeric key other than `A`, `B`, `C`, or `D` is pressed, it is ignored. The function modifies the `pin` in `g_vars_ptr` directly.
 */
void keyFxRecord(char key) {
    if (key == '#') {
        // key is #
        g_vars_ptr->pin+="#";
        g_vars_ptr->confirm = true;
    } else if (key == '*') {
        // key is *
        if (g_vars_ptr->pin.length() == 0 || g_vars_ptr->pin.endsWith("#")) {
            g_vars_ptr->abort = true;
        } else {
            g_vars_ptr->pin.remove(g_vars_ptr->pin.length()-1);
        }
    } else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
        // key is number 0..9
        g_vars_ptr->pin+=key;
    } else {
        // key is A..D etc...
        // g_vars_ptr->pin+=key;
    }
}

/**
 * @brief Handles the logic for recording key inputs and updating the PIN and related system parameters.
 *
 * This function processes a key input and updates various system states and flags based on the key pressed:
 * - If the key is `#`, it appends `#` to the `pin`, sets the `confirm` flag to `true`, and marks the `pin` display for refresh.
 * - If the key is `*`, it removes the last character from the `pin` or sets the `abort` flag if the `pin` is empty or ends with a `#`. It also triggers a refresh of the `pin` display.
 * - If the key is a number (0-9), it appends the number to the `pin` and marks the `pin` display for refresh.
 * - If the key is one of the special characters `A`, `B`, `C`, or `D`, it updates system counters (such as `alarm_events` and `attempts`) and triggers refreshes of related displays.
 *
 * @param key The character representing the pressed key.
 * - `#`: Confirm the PIN entry and trigger the refresh of the `pin` display.
 * - `*`: Delete the last character or abort if conditions are met, and trigger the refresh of the `pin` display.
 * - `0`-`9`: Append the number to the PIN and trigger the refresh of the `pin` display.
 * - `A`: Increment the `alarm_events` counter and trigger the refresh of the events display.
 * - `B`: Decrement the `alarm_events` counter and trigger the refresh of the events display.
 * - `C`: Increment the `attempts` counter and trigger the refresh of the attempts display.
 * - `D`: Decrement the `attempts` counter and trigger the refresh of the attempts display.
 * 
 * @return void
 * 
 * @details This function is designed for handling PIN input with the ability to modify alarm events and attempts based on special key presses (`A`, `B`, `C`, `D`). The function also triggers the appropriate display refresh flags depending on the changes made to the system.
 */
void keyFxRecordTest(char key) {
    if (key == '#') {
        // key is #
        g_vars_ptr->pin+="#";
        g_vars_ptr->confirm = true;
        g_vars_ptr->refresh_display.refresh_pin = true;
    } else if (key == '*') {
        // key is *
        if (g_vars_ptr->pin.length() == 0 || g_vars_ptr->pin.endsWith("#")) {
            g_vars_ptr->abort = true;
        } else {
            g_vars_ptr->pin.remove(g_vars_ptr->pin.length()-1);
        }
        g_vars_ptr->refresh_display.refresh_pin = true;
    } else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
        // key is number 0..9
        g_vars_ptr->pin+=key;
        g_vars_ptr->refresh_display.refresh_pin = true;
    } else {
        // key is A..D etc...
        switch (key) {
            // event ++
            case 'A':
                g_vars_ptr->alarm.alarm_events++;
                g_vars_ptr->refresh_display.refresh_events = true;
                break;
            // event --
            case 'B':
                g_vars_ptr->alarm.alarm_events--;
                g_vars_ptr->refresh_display.refresh_events = true;
                break;
            // attempt ++
            case 'C':
                g_vars_ptr->attempts++;
                g_vars_ptr->refresh_display.refresh_attempts = true;
                break;
            // attempt --
            case 'D':
                g_vars_ptr->attempts--;
                g_vars_ptr->refresh_display.refresh_attempts = true;
                break;
        }
    }
}

bool isValidChar(char input) {
    const char invalidChars[] = {'\0', ' ', 'N', 'F'};
    for (char invalid : invalidChars) {
        if (input == invalid) {
            return false;
        }
    }
    return true;
}

void keypadEvent(char key) {
    if ((key == 'A' || key == 'B' || key == 'C' || key == 'D') && 
        (g_vars_ptr->state != STATE_ALARM_OK || g_vars_ptr->state != STATE_ALARM_W || g_vars_ptr->state != STATE_ALARM_E || g_vars_ptr->state != STATE_ALARM_C) &&
        (g_vars_ptr->alarm.alarm_fire || g_vars_ptr->alarm.alarm_water || g_vars_ptr->alarm.alarm_electricity)) {
        esplogI(TAG_LIB_KEYPAD, NULL, "Turning off all secondary alarm triggerers!");
        g_vars_ptr->alarm.alarm_fire = false;
        g_vars_ptr->alarm.alarm_water = false;
        g_vars_ptr->alarm.alarm_electricity = false;
    }

    switch (g_vars_ptr->state) {
        case STATE_INIT:
        case STATE_SETUP:
        case STATE_ALARM_IDLE:
        case STATE_TEST_IDLE:
            keyFxMenu(key);
            g_vars_ptr->refresh_display.refresh_selection = true;
            break;

        case STATE_SETUP_AP:
            displayRestart();
            rebootESP();
            break;

        case STATE_SETUP_HARD_RESET:
        case STATE_SETUP_RFID_ADD:
        case STATE_SETUP_RFID_DEL:
        case STATE_SETUP_RFID_CHECK:
            keyFxConfirm(key);
            break;

        case STATE_ALARM_OK:
        case STATE_ALARM_C:
        case STATE_ALARM_W:
        case STATE_ALARM_E:
            keyFxRecord(key);
            g_vars_ptr->refresh_display.refresh_pin = true;
            break;

        case STATE_TEST_OK:
        case STATE_TEST_C:
        case STATE_TEST_W:
        case STATE_TEST_E:
            keyFxRecordTest(key);
            break;

        case STATE_ALARM_LOCK_ENTER_PIN:
        case STATE_SETUP_AP_ENTER_PIN:
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
        case STATE_SETUP_RFID_ADD_ENTER_PIN:
        case STATE_SETUP_RFID_DEL_ENTER_PIN:
        case STATE_SETUP_HARD_RESET_ENTER_PIN:
            keyFxRecord(key);
            g_vars_ptr->refresh_display.refresh_pin = true;
            break;
        
        default:
            break;
    }
}
