#include "libKeypad.h"

const uint8_t KEYPAD_I2C_ADDRESS = 0x20;
I2CKeyPad8x8 keypad(KEYPAD_I2C_ADDRESS);

bool isValidChar(char input) {
    const char invalidChars[] = {'\0', ' ', 'N', 'F'};
    for (char invalid : invalidChars) {
        if (input == invalid) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Callback function for default keypad event - menu.
 * 
 * This function can be used when any button is pressed.
 * Checks if any of the buttons controls current menu.
 * 
 * 2 --> down/prev
 * 5 --> enter/confirm
 * 8 --> up/next
 * 
 */
void keyFxMenu(char key, g_vars_t * g_vars) {
    switch (key) {
        // menu up/next
        case '2':
            g_vars->selection--;
            cycleSelection(&g_vars->selection, g_vars->selection_max);
            break;

        // menu down/prev
        case '8':
            g_vars->selection++;
            cycleSelection(&g_vars->selection, g_vars->selection_max);
            break;

        // menu enter/confirm
        case '5':
            g_vars->confirm = true;
            break;
        
        default:
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
        g_vars->pin+="#";
        g_vars->confirm = true;
    } else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
        // key is number 0..9
        g_vars->pin+=key;
    } else {
        // key is A..D etc...
        // g_vars->pin+=key;
    }
}

void keyFxPressInit(char key, g_vars_t * g_vars) {
    keyFxMenu(key, g_vars);
}

void keyFxPressSetup(char key, g_vars_t * g_vars) {
    keyFxMenu(key, g_vars);
}

void keyFxPressAlarmIdle(char key, g_vars_t * g_vars) {
    keyFxMenu(key, g_vars);
}

void keyFxPressTestIdle(char key, g_vars_t * g_vars) {
    keyFxMenu(key, g_vars);
}

void keyFxPressAlarmLockEnterPin(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressTestLockEnterPin(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressAlarmUnlockEnterPin(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressTestUnlockEnterPin(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressAlarmChangeEnterPin1(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressTestChangeEnterPin1(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressAlarmChangeEnterPin2(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressTestChangeEnterPin2(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressAlarmChangeEnterPin3(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}

void keyFxPressTestChangeEnterPin3(char key, g_vars_t * g_vars) {
    keyFxRecord(key, g_vars);
}
