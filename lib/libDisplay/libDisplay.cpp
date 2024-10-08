#include "libDisplay.h"

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void authScreenC(g_vars_t * g_vars) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("auth: SUCCESS");
    lcd.setCursor(0, 1);
    lcd.printf("Tries: %d", g_vars->attempts+1);
    lcd.setCursor(0, 2);
    delay(1000);
}

void authScreenE(g_vars_t * g_vars) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("auth: ERROR");
    lcd.setCursor(0, 1);
    lcd.printf("Tries: %d", g_vars->attempts+1);
    lcd.setCursor(0, 2);
    delay(1000);
}

void authScreenS(g_vars_t * g_vars) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("auth: SET");
    lcd.setCursor(0, 1);
    lcd.printf("Tries: %d", g_vars->attempts+1);
    lcd.setCursor(0, 2);
    lcd.printf("Length: %d", lengthPassword());
    lcd.setCursor(0, 3);
    delay(1000);
}

void rfidScreenC(g_vars_t * g_vars, const char * uid) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("rfid: SUCCESS");
    lcd.setCursor(0, 1);
    lcd.printf("UID: %s", uid);
    lcd.setCursor(0, 2);
    delay(1000);
}

void rfidScreenE(g_vars_t * g_vars, const char * uid) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("rfid: ERROR");
    lcd.setCursor(0, 1);
    lcd.printf("UID: %s", uid);
    lcd.setCursor(0, 2);
    delay(1000);
}

void rfidScreenA(g_vars_t * g_vars, const char * uid) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("rfid: ADDED");
    lcd.setCursor(0, 1);
    lcd.printf("UID: %s", uid);
    lcd.setCursor(0, 2);
    delay(1000);
}

void rfidScreenD(g_vars_t * g_vars, const char * uid) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("rfid: DELETED");
    lcd.setCursor(0, 1);
    lcd.printf("UID: %s", uid);
    lcd.setCursor(0, 2);
    delay(1000);
}

void loadScreen(g_vars_t * g_vars, g_config_t * g_config, bool reboot) {
    lcd.clear();
    lcd.setCursor(0, 0);

    if (reboot) {
        lcd.print("Rebooting...");
        return;
    }

    lcd.print(getStateText(g_vars->state, true));
    lcd.setCursor(1, 1);
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
            lcd.print("Confirm:");
            lcd.setCursor(1, 2);
            break;

        case STATE_SETUP_RFID_ADD:
        case STATE_SETUP_RFID_DEL:
        case STATE_SETUP_RFID_CHECK:
            lcd.print("Insert RFID card:");
            lcd.setCursor(1, 2);
            break;

        case STATE_ALARM_IDLE:
            loadScreenAlarm(g_vars);
            break;

        case STATE_TEST_IDLE:
            loadScreenTest(g_vars);
            break;

        case STATE_ALARM_OK:
        case STATE_TEST_OK:
            lcd.print("Level: ok");
            lcd.setCursor(1, 2);
            break;

        case STATE_ALARM_C:
        case STATE_TEST_C:
            lcd.printf("Remaining: %d", g_config->alarm_countdown_s - int(g_vars->time/1000));
            lcd.setCursor(1, 2);
            break;

        case STATE_ALARM_W:
        case STATE_TEST_W:
            lcd.print("Level: warning");
            lcd.setCursor(1, 2);
            break;

        case STATE_ALARM_E:
        case STATE_TEST_E:
            lcd.print("Level: emergency");
            lcd.setCursor(1, 2);
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
            lcd.print("State was not recognised!");
            lcd.setCursor(1, 2);
            break;
    }
}

void loadScreenInit(g_vars_t * g_vars) {
    lcd.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    lcd.setCursor(1, 2);
}

void loadScreenSetup(g_vars_t * g_vars) {
    lcd.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    lcd.setCursor(1, 2);
}

void loadScreenSetupAP(g_vars_t * g_vars) {

}

void loadScreenSetupRfid(g_vars_t * g_vars) {
    lcd.print("Insert RFID card:");
    lcd.setCursor(1, 2);
}

void loadScreenEnterPin(g_vars_t * g_vars) {
    lcd.print("Enter PIN or RFID:");
    lcd.setCursor(1, 2);

    int length = g_vars->pin.length();
    for (int i = 0; i < length; i++) {
        lcd.print("*");
    }
    lcd.setCursor(1, 3);
}

void loadScreenEnterPinFirst(g_vars_t * g_vars) {
    lcd.print("Enter new PIN:");
    lcd.setCursor(1, 2);

    int length = g_vars->pin.length();
    for (int i = 0; i < length; i++) {
        lcd.print("*");
    }
    lcd.setCursor(1, 3);
}

void loadScreenEnterPinSecond(g_vars_t * g_vars) {
    lcd.print("Repeat new PIN:");
    lcd.setCursor(1, 2);

    int delimiter = g_vars->pin.indexOf('#');
    String pin = g_vars->pin.substring(delimiter+1);
    int length = pin.length();
    for (int i = 0; i < length; i++) {
        lcd.print("*");
    }
    lcd.setCursor(1, 3);
}

void loadScreenAlarm(g_vars_t * g_vars) {
    lcd.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    lcd.setCursor(1, 2);
}

void loadScreenTest(g_vars_t * g_vars) {
    lcd.printf("%s", getSelectionText(g_vars->state, g_vars->selection, true));
    lcd.setCursor(1, 2);
}
