#include "libKeypad.h"

const uint8_t KEYPAD_I2C_ADDRESS = 0x20;
I2CKeyPad keypad(KEYPAD_I2C_ADDRESS);

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
void keyFxMenu(char key, g_vars_t * g_vars) {
    switch (key) {
        // menu up/next
        case '4':
        case '8':
            g_vars->selection++;
            cycleSelection(&g_vars->selection, g_vars->selection_max);
            break;

        // menu down/prev
        case '2':
        case '6':
            g_vars->selection--;
            cycleSelection(&g_vars->selection, g_vars->selection_max);
            break;

        // menu enter/confirm
        case '5':
        case '#':
            g_vars->confirm = true;
            break;

        case '*':
            // TODO
            g_vars->abort = true;
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
void keyFxConfirm(char key, g_vars_t * g_vars) {
    switch (key) {
        case '*':
            g_vars->abort = true;
            break;
        
        default:
            g_vars->confirm = true;
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
void keyFxRecord(char key, g_vars_t * g_vars) {
    if (key == '#') {
        // key is #
        g_vars->pin+="#";
        g_vars->confirm = true;
    } else if (key == '*') {
        // key is *
        if (g_vars->pin.length() == 0 || g_vars->pin.endsWith("#")) {
            g_vars->abort = true;
        } else {
            g_vars->pin.remove(g_vars->pin.length()-1);
        }
    } else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
        // key is number 0..9
        g_vars->pin+=key;
    } else {
        // key is A..D etc...
        // g_vars->pin+=key;
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
void keyFxRecordTest(char key, g_vars_t * g_vars, g_config_t * g_config) {
    if (key == '#') {
        // key is #
        g_vars->pin+="#";
        g_vars->confirm = true;
        updateScreen(g_vars, g_config, UPDATE_PIN);
    } else if (key == '*') {
        // key is *
        if (g_vars->pin.length() == 0 || g_vars->pin.endsWith("#")) {
            g_vars->abort = true;
        } else {
            g_vars->pin.remove(g_vars->pin.length()-1);
        }
        updateScreen(g_vars, g_config, UPDATE_PIN);
    } else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
        // key is number 0..9
        g_vars->pin+=key;
        updateScreen(g_vars, g_config, UPDATE_PIN);
    } else {
        // key is A..D etc...
        switch (key) {
            // event ++
            case 'A':
                g_vars->alarm_events++;
                updateScreen(g_vars, g_config, UPDATE_EVENTS);
                break;
            // event --
            case 'B':
                g_vars->alarm_events--;
                updateScreen(g_vars, g_config, UPDATE_EVENTS);
                break;
            // attempt ++
            case 'C':
                g_vars->attempts++;
                updateScreen(g_vars, g_config, UPDATE_ATTEMPTS);
                break;
            // attempt --
            case 'D':
                g_vars->attempts--;
                updateScreen(g_vars, g_config, UPDATE_ATTEMPTS);
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

void keypadEvent(g_vars_t * g_vars, g_config_t * g_config, char key) {
  switch (g_vars->state) {
    case STATE_INIT:
    case STATE_SETUP:
    case STATE_ALARM_IDLE:
    case STATE_TEST_IDLE:
        keyFxMenu(key, g_vars);
        updateScreen(g_vars, g_config, UPDATE_SELECTION);
        break;

    case STATE_SETUP_AP:
        loadScreen(g_vars, g_config, true);
        rebootESP();
        break;

    case STATE_SETUP_HARD_RESET:
    case STATE_SETUP_RFID_ADD:
    case STATE_SETUP_RFID_DEL:
    case STATE_SETUP_RFID_CHECK:
        keyFxConfirm(key, g_vars);
        break;

    case STATE_ALARM_OK:
    case STATE_ALARM_W:
    case STATE_ALARM_E:
        keyFxRecord(key, g_vars);
        updateScreen(g_vars, g_config, UPDATE_PIN);
        break;

    case STATE_TEST_OK:
    case STATE_TEST_W:
    case STATE_TEST_E:
        keyFxRecordTest(key, g_vars, g_config);
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
        keyFxRecord(key, g_vars);
        updateScreen(g_vars, g_config, UPDATE_PIN);
        break;
    
    default:
        break;
  }
}
