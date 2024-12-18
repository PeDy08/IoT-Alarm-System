#include "libPeripherals.h"

extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

CRGB rgb_leds[LED_COUNT];
const uint8_t UTILS_I2C_ADDRESS = 0x21;
PCF8574 gpio_extender(UTILS_I2C_ADDRESS);

bool ledRunning[LED_COUNT];
int ledDuration[LED_COUNT];
uint32_t ledColors[LED_COUNT];
uint32_t ledBlinkColors[LED_COUNT];

bool buzzerRunning = false;

TaskHandle_t handleTaskLed = NULL;
TaskHandle_t handleTaskBuzzer = NULL;

/**
 * @brief LED blinking task that controls the LED behaviors.
 * 
 * This task controls the behavior of LEDs based on their blinking duration and color. It updates the brightness of each LED using a sine wave function to create smooth blinking effects.
 * The task runs in a loop and periodically checks each LED's status and adjusts its brightness accordingly.
 * 
 * @param parameters The task parameters (unused in this case, set to `NULL`).
 * 
 * @details
 * - The task runs continuously, checking the elapsed time for each LED and updating its brightness based on the sine wave function.
 * - The `ledDuration` and `ledColors` arrays are used to determine how long and with what color each LED should blink.
 * - The task uses `FastLED.show()` to update the LED strip and display the changes.
 * - The task has a delay of 100 milliseconds to control the blinking rate.
 * 
 * Example Usage:
 * This function is used internally within the system to manage LED behavior. It is not called directly by the user.
 */
void ledTask(void *parameters);

/**
 * @brief Buzzer task that handles the beeping behavior.
 * 
 * This is the FreeRTOS task that controls the buzzer's beeping. It repeatedly beeps the buzzer for the specified duration, with a delay between each beep.
 * The task continues to beep as long as `buzzerRunning` is `true`.
 * 
 * @param parameters The duration of the beep in milliseconds passed from the `buzzerBeepStart` function.
 * 
 * @details
 * - The task uses the `buzzerBeep` function to beep the buzzer for the specified duration.
 * - After each beep, the task sleeps for the same duration, creating a repeating beep pattern.
 * - The task will continue running until `buzzerRunning` is set to `false`, at which point it will exit.
 * - The task cleans up itself by deleting the task handle when finished.
 * 
 * Example Usage:
 * This function is used internally to handle the buzzer task. It is not meant to be called directly by the user.
 */
void buzzerTask(void *parameters);

bool initOutputDevices() {
    bool ret = true;

    // init RGB leds
    if (ret) {
        // FastLED.addLeds<WS2801, LED_DATA_PIN, GRB>(rgb_leds, LED_COUNT));
        FastLED.addLeds<SK6812, LED_DATA_PIN, GRB>(rgb_leds, LED_COUNT);
        FastLED.clear();
        FastLED.show();

        for (int i = 0; i < LED_COUNT; i++) {
            ledRunning[i] = false;
            ledDuration[i] = -1;
            ledColors[i] = 0;
            ledBlinkColors[i] = 0;
        }

        ledBlinkStart();
    }

    // init GPIO extender
    if (ret) {
        ret = gpio_extender.begin();
    }

    // init analog/digital measurements pins
    if (ret) {
        pinMode(BATTERY_VOLTAGE_PIN, INPUT);
        pinMode(DC_VOLTAGE_PIN, INPUT);
    }

    return ret;
}

// --------------------------------------------- LEDS ---------------------------------------------

void ledOn(uint8_t index) {
    if (index < LED_COUNT) {
        uint8_t red = (ledColors[index] >> 16 & 0xFF) * LED_BRIGHTNESS / 255;
        uint8_t green = (ledColors[index] >> 8 & 0xFF) * LED_BRIGHTNESS / 255;
        uint8_t blue = (ledColors[index] & 0xFF) * LED_BRIGHTNESS / 255;

        rgb_leds[index] = CRGB(red, green, blue);
        FastLED.show();
    }
}

void ledOn(uint8_t index, uint32_t colorcode) {
    if (index < LED_COUNT) {
        ledColors[index] = colorcode;
        ledOn(index);
    }
}

void ledOn(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < LED_COUNT) {
        ledColors[index] = uint32_t(0xff000000) | (uint32_t{r} << 16) | (uint32_t{g} << 8) | uint32_t{b};
        ledOn(index);
    }
}

void ledOff(uint8_t index) {
    if (index < LED_COUNT) {
        rgb_leds[index] = CRGB::Black;
        FastLED.show();
    }
}

void ledBlink(uint8_t index, int duration) {
    ledOn(index);
    vTaskDelay(duration / portTICK_PERIOD_MS);
    ledOff(index);
}

void ledBlink(uint8_t index, uint32_t colorcode, int duration) {
    ledOn(index, colorcode);
    vTaskDelay(duration / portTICK_PERIOD_MS);
    ledOff(index);
}

void ledBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, int duration) {
    ledOn(index, r, g, b);
    vTaskDelay(duration / portTICK_PERIOD_MS);
    ledOff(index);
}

void ledBlink(uint8_t index, int duration, int count) {
    for (int i = 0; i < count; i++) {
        ledBlink(index, duration);
        vTaskDelay(duration);
    }
}

void ledBlink(uint8_t index, uint32_t colorcode, int duration, int count) {
    for (int i = 0; i < count; i++) {
        ledBlink(index, colorcode, duration);
        vTaskDelay(duration);
    }
}

void ledBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, int duration, int count) {
    for (int i = 0; i < count; i++) {
        ledBlink(index, r, g, b, duration);
        vTaskDelay(duration);
    }
}

void ledBlinkStart() {
    if (handleTaskLed != NULL) {
        vTaskDelete(handleTaskLed);
        handleTaskLed = NULL;
    }

    if (handleTaskLed == NULL) {
        xTaskCreate(ledTask, "LedTask", 2048, NULL, 1, &handleTaskLed);
    }
}


void ledBlinkStop() {
    for (int i = 0; i < LED_COUNT; i++) {
        ledRunning[i] = false;
        ledDuration[i] = -1;
        ledBlinkColors[i] = 0;
    }

    if (handleTaskBuzzer != NULL) {
        vTaskDelete(handleTaskLed);
        handleTaskBuzzer = NULL;
    }
}

void ledTask(void *parameters) {
    unsigned long current_time;
    unsigned long last_period_time[LED_COUNT];
    for (;;) {
        current_time = millis();
        for (int i = 0; i < LED_COUNT; i++) {
            if (ledRunning[i]) {
                unsigned long elapsed_time = current_time - last_period_time[i];
                if (elapsed_time >= ledDuration[i]) {
                    elapsed_time = 0;
                    last_period_time[i] = current_time;
                }

                float sine_value = sin(((float)elapsed_time / ledDuration[i]) * 2 * PI);
                int ledBrightness = (int)((sine_value + 1.0) * 127.5);
                rgb_leds[i] = CRGB(
                    ((ledColors[i] >> 16 & 0xFF) * ledBrightness / 255) * LED_BRIGHTNESS / 255,  // Red component
                    ((ledColors[i] >> 8 & 0xFF) * ledBrightness / 255) * LED_BRIGHTNESS / 255,   // Green component
                    ((ledColors[i] & 0xFF) * ledBrightness / 255) * LED_BRIGHTNESS / 255         // Blue component
                );
            }
        }

        FastLED.show();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(handleTaskLed);
    handleTaskLed = NULL;
}

void lightLedByState() {
    // esplogI(TAG_LIB_PERIPHERALS, "(lightLedByState)", "Lighting LED 1 by state: %d", g_vars_ptr->state);

    switch (g_vars_ptr->state) {
        // menu
        case STATE_INIT:
        case STATE_SETUP:
        case STATE_SETUP_AP:
        case STATE_SETUP_HARD_RESET:
        case STATE_ALARM_IDLE:
        case STATE_TEST_IDLE:
            ledOn(1, 3, 252, 248);
            break;
        
        // RFID operations
        case STATE_SETUP_RFID_ADD:
        case STATE_SETUP_RFID_DEL:
        case STATE_SETUP_RFID_CHECK:
            ledOn(1, 252, 3, 186);
            ledRunning[1] = true;
            ledDuration[1] = 3000;
            break;

        // entering PIN operations
        case STATE_SETUP_AP_ENTER_PIN:
        case STATE_SETUP_HARD_RESET_ENTER_PIN:
        case STATE_SETUP_PIN1:
        case STATE_SETUP_RFID_ADD_ENTER_PIN:
        case STATE_SETUP_RFID_DEL_ENTER_PIN:
        case STATE_ALARM_LOCK_ENTER_PIN:
        case STATE_ALARM_UNLOCK_ENTER_PIN:
        case STATE_ALARM_CHANGE_ENTER_PIN1:
        case STATE_TEST_LOCK_ENTER_PIN:
        case STATE_TEST_UNLOCK_ENTER_PIN:
        case STATE_TEST_CHANGE_ENTER_PIN1:
            ledOn(1, 119, 0, 255);
            ledRunning[1] = true;
            ledDuration[1] = 3000;
            break;

        // creating new PIN operations
        case STATE_SETUP_PIN2:
        case STATE_ALARM_CHANGE_ENTER_PIN2:
        case STATE_TEST_CHANGE_ENTER_PIN2:
            ledOn(1, 0, 255, 200);
            ledRunning[1] = true;
            ledDuration[1] = 3000;
            break;
        
        // confirming new PIN operations
        case STATE_SETUP_PIN3:
        case STATE_ALARM_CHANGE_ENTER_PIN3:
        case STATE_TEST_CHANGE_ENTER_PIN3:
            ledOn(1, 0, 255, 200);
            ledRunning[1] = true;
            ledDuration[1] = 3000;
            break;

        // C
        case STATE_ALARM_C:
        case STATE_TEST_C:
            ledOn(1, 3, 252, 248);
            ledRunning[1] = true;
            ledDuration[1] = 3000;
            break;
        
        // OK
        case STATE_ALARM_OK:
        case STATE_TEST_OK:
            ledOn(1, 3, 252, 20);
            break;
        
        // W
        case STATE_ALARM_W:
        case STATE_TEST_W:
            ledOn(1, 252, 202, 3);
            ledRunning[1] = true;
            ledDuration[1] = 3000;
            break;

        // E
        case STATE_ALARM_E:
        case STATE_TEST_E:
            ledOn(1, 255, 0, 0);
            break;

        default:
            ledOff(1);
            break;
    }

}

void ledByBattery() {
    // esplogI(TAG_LIB_PERIPHERALS, "(ledByBattery)", "Lighting LED 0 - POWER: %d, BATTERY: %d%%", g_vars_ptr->power_mode, g_vars_ptr->battery_level);

    if (g_vars_ptr->power_mode) {
        ledRunning[0] = false;
        ledDuration[0] = 0;
    } else {
        ledRunning[0] = true;
        ledDuration[0] = 5000;
    }

    ledOn(0, 255 * (1 - g_vars_ptr->battery_level / 100.0), 255 * (g_vars_ptr->battery_level / 100.0), 0);
}


// -------------------------------------------- BUZZER --------------------------------------------
void buzzerOn() {
    gpio_extender.write(PIEZZO_DATA_PIN, HIGH);
}

void buzzerOff() {
    gpio_extender.write(PIEZZO_DATA_PIN, LOW);
}

void buzzerBeep(int duration) {
    buzzerOn();
    vTaskDelay(duration / portTICK_PERIOD_MS);
    buzzerOff();
}

void buzzerBeep(int duration, int count) {
    for (int i = 0; i < count; i++) {
        buzzerBeep(duration);
        vTaskDelay(duration);
    }
}

void buzzerBeepStart(int duration) {
    if (handleTaskBuzzer != NULL) {
        buzzerRunning = false;
        vTaskDelete(handleTaskBuzzer);
        handleTaskBuzzer = NULL;
    }

    if (handleTaskBuzzer == NULL) {
        xTaskCreate(buzzerTask, "BuzzerTask", 2048, (void *)&duration, 1, &handleTaskBuzzer);
        buzzerRunning = true;
    }
}

void buzzerBeepStop() {
    if (handleTaskBuzzer != NULL) {
        buzzerRunning = false;
    }
}

void buzzerTask(void *parameters) {
    int duration = *(int*)parameters;

    while (buzzerRunning) {
        buzzerBeep(duration);
        vTaskDelay(duration / portTICK_PERIOD_MS);
    }

    vTaskDelete(handleTaskBuzzer);
    handleTaskBuzzer = NULL;
}

// ------------------------------------------ ANALOG IN -------------------------------------------

float getBatteryVoltage() {
    // value between 0 - 4095 (0.1V - 3.2V) but using voltage divider
    float battery_voltage;
    uint16_t meas_battery = analogRead(BATTERY_VOLTAGE_PIN);

    if (meas_battery > 0) {
        battery_voltage = ((float)meas_battery / 4095 * 3.15 + 0.15) * 4.25;
    } else {
        battery_voltage = -1;
    }

    // esplogI(TAG_LIB_PERIPHERALS, "(getBatteryVoltage)", "Measuring current battery voltage: %f (0x%04hx, %f)", battery_voltage, meas_battery, (float)meas_battery / 4095 * 3.15 + 0.15);
    return battery_voltage;
}

float getDCVoltage() {
    // value between 0 - 4095 (0.1V - 3.2V)
    float dc_voltage;
    uint16_t meas_dc = analogRead(DC_VOLTAGE_PIN);

    if (meas_dc > 0) {
        dc_voltage = ((float)meas_dc / 4095 * 3.15 + 0.15) * 4.25;
    } else {
        dc_voltage = -1;
    }
    // esplogI(TAG_LIB_PERIPHERALS, "(getDCVoltage)", "Measuring current DC IN voltage: %f (0x%04hx, %f)", dc_voltage, meas_dc, (float)meas_dc / 4095 * 3.15 + 0.15);

    // bool value if on/off
    // int meas_dc = digitalRead(MAINS_VOLTAGE_PIN);
    // esplogI(TAG_LIB_PERIPHERALS, "(getDCVoltage)", "Measuring current DC IN voltage: %d (ON/OFF)", meas_dc);

    return dc_voltage;
}

void refreshBatteryLevel() {
    float battery_voltage = getBatteryVoltage();

    const float voltageTable[21] = {
        4.20, 4.15, 4.10, 4.05, 4.00, 3.95, 3.90, 3.85, 3.80, 3.75, 
        3.70, 3.65, 3.60, 3.55, 3.50, 3.45, 3.40, 3.35, 3.30, 3.20, 3.00
    };

    const int percentageTable[21] = {
        100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 
        50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0
    };

    // Clamp voltage within the valid range
    if (battery_voltage >= voltageTable[0]) {
        g_vars_ptr->battery_level = 100;
        esplogI(TAG_LIB_PERIPHERALS, "(refreshBatteryLevel)", "Current battery percentage: %d %%", g_vars_ptr->battery_level);
        return;
    }

    if (battery_voltage <= voltageTable[20]) {
        g_vars_ptr->battery_level = 0;
        esplogI(TAG_LIB_PERIPHERALS, "(refreshBatteryLevel)", "Current battery percentage: %d %%", g_vars_ptr->battery_level);
        return;
    }

    // Find the corresponding percentage using linear interpolation
    for (int i = 0; i < 21 - 1; i++) {
        if (battery_voltage <= voltageTable[i] && battery_voltage > voltageTable[i + 1]) {
            float v1 = voltageTable[i];
            float v2 = voltageTable[i + 1];
            int p1 = percentageTable[i];
            int p2 = percentageTable[i + 1];

            // Linear interpolation between the two points
            g_vars_ptr->battery_level = p1 + (battery_voltage - v1) * (p2 - p1) / (v2 - v1);
            break;
        }
    }
    esplogI(TAG_LIB_PERIPHERALS, "(refreshBatteryLevel)", "Current battery percentage: %d %%", g_vars_ptr->battery_level);
}

void refreshPowerMode() {
    float dc_voltage = getDCVoltage();
    if (dc_voltage >= DC_VOLTAGE_THRESHOLD) {
        g_vars_ptr->power_mode = true;
    } else {
        g_vars_ptr->power_mode = false;
    }
    esplogI(TAG_LIB_PERIPHERALS, "(refreshPowerMode)", "Current power-mode: %s", g_vars_ptr->power_mode ? "DC" : "BAT");
}
