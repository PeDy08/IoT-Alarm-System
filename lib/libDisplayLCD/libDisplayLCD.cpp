#include "libDisplayLCD.h"

LiquidCrystal_I2C display(LCD_ADDR, LCD_COLS, LCD_ROWS);

void initLcd() {
    Serial.printf("LCD display initialisation\n");
    display.init();
    display.backlight();
    display.setCursor(0, 0);
    display.print("IoT Alarm System");
    display.setCursor(0, 1);
    display.print("version: 0.1");
    display.setCursor(0, 2);
    display.print("");
    display.setCursor(0, 3);
    display.print("Petr Zerzan");
}

/* void authScreenC(g_vars_t * g_vars) {
    display.clear();
    display.setCursor(0, 0);
    display.print("auth: SUCCESS");
    display.setCursor(0, 1);
    display.printf("Tries: %d", g_vars->attempts+1);
    display.setCursor(0, 2);
    delay(1000);
}

void authScreenE(g_vars_t * g_vars) {
    display.clear();
    display.setCursor(0, 0);
    display.print("auth: ERROR");
    display.setCursor(0, 1);
    display.printf("Tries: %d", g_vars->attempts+1);
    display.setCursor(0, 2);
    delay(1000);
}

void authScreenS(g_vars_t * g_vars) {
    display.clear();
    display.setCursor(0, 0);
    display.print("auth: SET");
    display.setCursor(0, 1);
    display.printf("Tries: %d", g_vars->attempts+1);
    display.setCursor(0, 2);
    display.printf("Length: %d", lengthPassword());
    display.setCursor(0, 3);
    delay(1000);
}

void rfidScreenC(g_vars_t * g_vars, const char * uid) {
    display.clear();
    display.setCursor(0, 0);
    display.print("rfid: SUCCESS");
    display.setCursor(0, 1);
    display.printf("UID: %s", uid);
    display.setCursor(0, 2);
    delay(1000);
}

void rfidScreenE(g_vars_t * g_vars, const char * uid) {
    display.clear();
    display.setCursor(0, 0);
    display.print("rfid: ERROR");
    display.setCursor(0, 1);
    display.printf("UID: %s", uid);
    display.setCursor(0, 2);
    delay(1000);
}

void rfidScreenA(g_vars_t * g_vars, const char * uid) {
    display.clear();
    display.setCursor(0, 0);
    display.print("rfid: ADDED");
    display.setCursor(0, 1);
    display.printf("UID: %s", uid);
    display.setCursor(0, 2);
    delay(1000);
}

void rfidScreenD(g_vars_t * g_vars, const char * uid) {
    display.clear();
    display.setCursor(0, 0);
    display.print("rfid: DELETED");
    display.setCursor(0, 1);
    display.printf("UID: %s", uid);
    display.setCursor(0, 2);
    delay(1000);
} */

void loadScreen(g_vars_t * g_vars, g_config_t * g_config, bool reboot) {
    display.clear();
    display.setCursor(0, 0);

    if (reboot) {
        display.print("Rebooting...");
        return;
    }

    display.print(getStateText(g_vars->state, true));
    display.setCursor(1, 1);
    switch (g_vars->state) {
        case STATE_INIT:
            loadScreenInit(g_vars);
            break;
        case STATE_SETUP:
            loadScreenSetup(g_vars);
            break;
        case STATE_SETUP_AP:
            loadScreenSetupAP(g_vars);
            break;
        
        case STATE_SETUP_HARD_RESET:
            display.print("Confirm:");
            display.setCursor(1, 2);
            break;

        case STATE_SETUP_RFID_ADD:
        case STATE_SETUP_RFID_DEL:
        case STATE_SETUP_RFID_CHECK:
            display.print("Insert RFID card:");
            display.setCursor(1, 2);
            break;

        case STATE_ALARM_IDLE:
            loadScreenAlarm(g_vars);
            break;

        case STATE_TEST_IDLE:
            loadScreenTest(g_vars);
            break;

        case STATE_ALARM_OK:
        case STATE_TEST_OK:
            display.print("Level: ok");
            display.setCursor(1, 2);
            break;

        case STATE_ALARM_C:
        case STATE_TEST_C:
            display.printf("Remaining: %d", g_config->alarm_countdown_s - int(g_vars->time_temp/1000));
            display.setCursor(1, 2);
            break;

        case STATE_ALARM_W:
        case STATE_TEST_W:
            display.print("Level: warning");
            display.setCursor(1, 2);
            break;

        case STATE_ALARM_E:
        case STATE_TEST_E:
            display.print("Level: emergency");
            display.setCursor(1, 2);
            break;

        case STATE_SETUP_HARD_RESET_ENTER_PIN:
        case STATE_SETUP_RFID_ADD_ENTER_PIN:
        case STATE_SETUP_RFID_DEL_ENTER_PIN:
        case STATE_ALARM_LOCK_ENTER_PIN:
        case STATE_TEST_LOCK_ENTER_PIN:
        case STATE_ALARM_UNLOCK_ENTER_PIN:
        case STATE_TEST_UNLOCK_ENTER_PIN:
        case STATE_ALARM_CHANGE_ENTER_PIN1:
        case STATE_TEST_CHANGE_ENTER_PIN1:
        case STATE_SETUP_PIN1:
            loadScreenEnterPin(g_vars);
            break;

        case STATE_ALARM_CHANGE_ENTER_PIN2:
        case STATE_TEST_CHANGE_ENTER_PIN2:
        case STATE_SETUP_PIN2:
            loadScreenEnterPinFirst(g_vars);
            break;

        case STATE_ALARM_CHANGE_ENTER_PIN3:
        case STATE_TEST_CHANGE_ENTER_PIN3:
        case STATE_SETUP_PIN3:
            loadScreenEnterPinSecond(g_vars);
            break;

        default:
            display.print("State was not recognised!");
            display.setCursor(1, 2);
            break;
    }
}

void loadScreenInit(g_vars_t * g_vars) {
    display.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    display.setCursor(1, 2);
}

void loadScreenSetup(g_vars_t * g_vars) {
    display.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    display.setCursor(1, 2);
}

void loadScreenSetupAP(g_vars_t * g_vars) {

}

void loadScreenSetupRfid(g_vars_t * g_vars) {
    display.print("Insert RFID card:");
    display.setCursor(1, 2);
}

void loadScreenEnterPin(g_vars_t * g_vars) {
    display.print("Enter PIN or RFID:");
    display.setCursor(1, 2);

    int length = g_vars->pin.length();
    for (int i = 0; i < length; i++) {
        display.print("*");
    }
    display.setCursor(1, 3);
}

void loadScreenEnterPinFirst(g_vars_t * g_vars) {
    display.print("Enter new PIN:");
    display.setCursor(1, 2);

    int length = g_vars->pin.length();
    for (int i = 0; i < length; i++) {
        display.print("*");
    }
    display.setCursor(1, 3);
}

void loadScreenEnterPinSecond(g_vars_t * g_vars) {
    display.print("Repeat new PIN:");
    display.setCursor(1, 2);

    int delimiter = g_vars->pin.indexOf('#');
    String pin = g_vars->pin.substring(delimiter+1);
    int length = pin.length();
    for (int i = 0; i < length; i++) {
        display.print("*");
    }
    display.setCursor(1, 3);
}

void loadScreenAlarm(g_vars_t * g_vars) {
    display.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    display.setCursor(1, 2);
}

void loadScreenTest(g_vars_t * g_vars) {
    display.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    display.setCursor(1, 2);
}
