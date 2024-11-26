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
 * @brief Callback function for keypad event - entering pin.
 * 
 * This function can be used when any button is pressed.
 * Records pressed key. If '*' or '#' pressed the pin is applied.
 * 
 * 0..9 --> record pin
 * '*' or '#' --> apply
 * 
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
 * @brief Callback function for keypad event - entering pin in testing mode.
 * 
 * This function can be used when any button is pressed.
 * Records pressed key. If '*' or '#' pressed the pin is applied.
 * 
 * 0..9 --> record pin
 * '*' or '#' --> apply
 * 'A', 'B', 'C', 'D' --> special behaviour for testing mode
 * 
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
                g_vars_ptr->alarm_events++;
                g_vars_ptr->refresh_display.refresh_events = true;
                break;
            // event --
            case 'B':
                g_vars_ptr->alarm_events--;
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
    case STATE_ALARM_W:
    case STATE_ALARM_E:
        keyFxRecord(key);
        g_vars_ptr->refresh_display.refresh_pin = true;
        break;

    case STATE_TEST_OK:
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
