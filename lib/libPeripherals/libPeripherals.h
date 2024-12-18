/**
 * @file libPeripherals.h
 * @brief Contains functions and definitions for controling peripherals.
 * 
 * Contains functions and definitions for controling other output peripherals
 * than the display (LEDs, piezzo).
 */

#ifndef LIBOUT_H_DEFINITION
#define LIBOUT_H_DEFINITION

#include <Arduino.h>
#include <FastLED.h>
#include <PCF8574.h>

#include "utils.h"
#include "mainAppDefinitions.h"

// TODO set it up properly
#define LED_DATA_PIN 13                 // pin for led data input signal
#define LED_COUNT 2                     // number of used leds
#define LED_BRIGHTNESS 16               // led brightness (0 -> 255)

#define PIEZZO_DATA_PIN 0               // pin for buzzer on/off signal

#define BATTERY_VOLTAGE_PIN 36          // pin for analog measurements of battery voltage
#define BATTERY_FULL_VOLTAGE 4.2f       // Fully charged voltage
#define BATTERY_EMPTY_VOLTAGE 3.0f      // Discharged voltage threshold

#define DC_VOLTAGE_PIN 36               // pin for digital measurements of DC IN voltage
#define DC_VOLTAGE_THRESHOLD 4.5f       // minimal voltage for defining dc input as power source

extern CRGB leds;
extern const uint8_t UTILS_I2C_ADDRESS;
extern PCF8574 gpio_extender;

/**
 * @brief Initializes output devices and GPIO components.
 *
 * This function initializes various output devices and related GPIO components for the system. It performs the
 * following tasks:
 * 1. Initializes the RGB LED strip (SK6812).
 * 2. Initializes the GPIO extender for controlling additional pins.
 * 3. Configures analog/digital input pins for measuring battery and DC voltage.
 *
 * @return True if all output devices and components were initialized successfully, False if any initialization 
 *         step failed.
 *
 * @details
 * - The RGB LED strip is initialized using the FastLED library. The LEDs are configured with the SK6812 
 *   driver and the GRB color order. The LED array is cleared, and the strip is prepared for use.
 * - The GPIO extender is initialized to control additional pins beyond the ESP32's GPIO limitations.
 * - The analog/digital input pins for battery and DC voltage measurements are configured with the `pinMode()` 
 *   function to enable the readings of battery voltage and DC input voltage.
 * 
 * If any initialization step fails, the function returns `false` to indicate the failure. If all devices are 
 * successfully initialized, it returns `true`.
 *
 * Example Usage:
 * @code
 * if (initOutputDevices()) {
 *     Serial.println("Output devices initialized successfully.");
 * } else {
 *     Serial.println("Failed to initialize output devices.");
 * }
 * @endcode
 */
bool initOutputDevices();

// *********************************************************************************************************************

/**
 * @brief Turns on an individual LED with a specified color.
 *
 * This set of overloaded functions turns on an individual LED in the RGB LED strip and sets its color.
 * The color is determined either by a color code, RGB values, or the previously set color.
 * 
 * There are three overloads of this function:
 * 1. Turns on an LED at a specific index using the previously set color.
 * 2. Turns on an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Turns on an LED at a specific index and sets the color using individual red, green, and blue values.
 * 
 * @param index The index of the LED to turn on (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second overload).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third overload).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third overload).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third overload).
 *
 * @details
 * - The LED at the specified index will be turned on, and its color will be set.
 * - The color is applied with a brightness factor, defined by the `LED_BRIGHTNESS` constant.
 *   The brightness factor modifies the red, green, and blue values by scaling them according to the `LED_BRIGHTNESS` value.
 * - If a valid index is provided (within bounds of the LED strip), the corresponding LED is updated.
 * - The FastLED library’s `show()` function is called to apply the changes to the LED strip.
 * 
 * If an invalid index (out of bounds) is provided, no action is taken.
 * 
 * Example Usage:
 * @code
 * ledOn(0, 0xFF0000);       // Turn on the first LED with red color using color code
 * ledOn(1, 255, 0, 0);      // Turn on the second LED with red color using RGB values
 * ledOn(2);                 // Turn on the third LED with the previously set color
 * @endcode
 */
void ledOn(uint8_t index);

/**
 * @brief Turns on an individual LED with a specified color.
 *
 * This set of overloaded functions turns on an individual LED in the RGB LED strip and sets its color.
 * The color is determined either by a color code, RGB values, or the previously set color.
 * 
 * There are three overloads of this function:
 * 1. Turns on an LED at a specific index using the previously set color.
 * 2. Turns on an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Turns on an LED at a specific index and sets the color using individual red, green, and blue values.
 * 
 * @param index The index of the LED to turn on (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second overload).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third overload).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third overload).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third overload).
 *
 * @details
 * - The LED at the specified index will be turned on, and its color will be set.
 * - The color is applied with a brightness factor, defined by the `LED_BRIGHTNESS` constant.
 *   The brightness factor modifies the red, green, and blue values by scaling them according to the `LED_BRIGHTNESS` value.
 * - If a valid index is provided (within bounds of the LED strip), the corresponding LED is updated.
 * - The FastLED library’s `show()` function is called to apply the changes to the LED strip.
 * 
 * If an invalid index (out of bounds) is provided, no action is taken.
 * 
 * Example Usage:
 * @code
 * ledOn(0, 0xFF0000);       // Turn on the first LED with red color using color code
 * ledOn(1, 255, 0, 0);      // Turn on the second LED with red color using RGB values
 * ledOn(2);                 // Turn on the third LED with the previously set color
 * @endcode
 */
void ledOn(uint8_t index, uint32_t colorcode);

/**
 * @brief Turns on an individual LED with a specified color.
 *
 * This set of overloaded functions turns on an individual LED in the RGB LED strip and sets its color.
 * The color is determined either by a color code, RGB values, or the previously set color.
 * 
 * There are three overloads of this function:
 * 1. Turns on an LED at a specific index using the previously set color.
 * 2. Turns on an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Turns on an LED at a specific index and sets the color using individual red, green, and blue values.
 * 
 * @param index The index of the LED to turn on (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second overload).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third overload).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third overload).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third overload).
 *
 * @details
 * - The LED at the specified index will be turned on, and its color will be set.
 * - The color is applied with a brightness factor, defined by the `LED_BRIGHTNESS` constant.
 *   The brightness factor modifies the red, green, and blue values by scaling them according to the `LED_BRIGHTNESS` value.
 * - If a valid index is provided (within bounds of the LED strip), the corresponding LED is updated.
 * - The FastLED library’s `show()` function is called to apply the changes to the LED strip.
 * 
 * If an invalid index (out of bounds) is provided, no action is taken.
 * 
 * Example Usage:
 * @code
 * ledOn(0, 0xFF0000);       // Turn on the first LED with red color using color code
 * ledOn(1, 255, 0, 0);      // Turn on the second LED with red color using RGB values
 * ledOn(2);                 // Turn on the third LED with the previously set color
 * @endcode
 */
void ledOn(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

// *********************************************************************************************************************

/**
 * @brief Blinks an individual LED for a specified duration.
 * 
 * This set of overloaded functions causes an LED to blink by turning it on, waiting for a specified duration, and then turning it off. 
 * The LED can blink in different colors or use the previously set color.
 * 
 * There are multiple overloads of this function:
 * 1. Blinks an LED at a specific index using the previously set color.
 * 2. Blinks an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Blinks an LED at a specific index and sets the color using individual red, green, and blue values.
 * 4. Blinks an LED at a specific index for a given duration and repeats the blinking a set number of times.
 * 5. Blinks an LED at a specific index and sets the color using a 24-bit color code for a given duration and repeats the blinking a set number of times.
 * 6. Blinks an LED at a specific index and sets the color using individual RGB values for a given duration and repeats the blinking a set number of times.
 * 
 * @param index The index of the LED to blink (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second and fifth overloads).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param duration The duration (in milliseconds) for the LED to stay on before turning off.
 * @param count The number of times to blink the LED (used in the fourth, fifth, and sixth overloads).
 * 
 * @details
 * - The LED at the specified `index` will blink for the given duration.
 * - In the first three overloads, the LED blinks once and then turns off.
 * - In the last three overloads, the LED blinks a specified number of times (`count`) with the given duration between each blink.
 * - The LED color can be controlled using a 24-bit color code or individual RGB values, depending on the function overload used.
 * - The function uses `vTaskDelay` to create the delay for the duration of the blink. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * ledBlink(0, 1000);             // Blink the first LED with the previously set color for 1 second.
 * ledBlink(1, 0xFF0000, 1000);   // Blink the second LED with red color (using color code) for 1 second.
 * ledBlink(2, 255, 0, 0, 1000);  // Blink the third LED with red color (using RGB) for 1 second.
 * ledBlink(0, 1000, 3);          // Blink the first LED 3 times, each blink lasting 1 second.
 * @endcode
 */
void ledBlink(uint8_t index, int duration);

/**
 * @brief Blinks an individual LED for a specified duration.
 * 
 * This set of overloaded functions causes an LED to blink by turning it on, waiting for a specified duration, and then turning it off. 
 * The LED can blink in different colors or use the previously set color.
 * 
 * There are multiple overloads of this function:
 * 1. Blinks an LED at a specific index using the previously set color.
 * 2. Blinks an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Blinks an LED at a specific index and sets the color using individual red, green, and blue values.
 * 4. Blinks an LED at a specific index for a given duration and repeats the blinking a set number of times.
 * 5. Blinks an LED at a specific index and sets the color using a 24-bit color code for a given duration and repeats the blinking a set number of times.
 * 6. Blinks an LED at a specific index and sets the color using individual RGB values for a given duration and repeats the blinking a set number of times.
 * 
 * @param index The index of the LED to blink (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second and fifth overloads).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param duration The duration (in milliseconds) for the LED to stay on before turning off.
 * @param count The number of times to blink the LED (used in the fourth, fifth, and sixth overloads).
 * 
 * @details
 * - The LED at the specified `index` will blink for the given duration.
 * - In the first three overloads, the LED blinks once and then turns off.
 * - In the last three overloads, the LED blinks a specified number of times (`count`) with the given duration between each blink.
 * - The LED color can be controlled using a 24-bit color code or individual RGB values, depending on the function overload used.
 * - The function uses `vTaskDelay` to create the delay for the duration of the blink. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * ledBlink(0, 1000);             // Blink the first LED with the previously set color for 1 second.
 * ledBlink(1, 0xFF0000, 1000);   // Blink the second LED with red color (using color code) for 1 second.
 * ledBlink(2, 255, 0, 0, 1000);  // Blink the third LED with red color (using RGB) for 1 second.
 * ledBlink(0, 1000, 3);          // Blink the first LED 3 times, each blink lasting 1 second.
 * @endcode
 */
void ledBlink(uint8_t index, uint32_t colorcode, int duration);

/**
 * @brief Blinks an individual LED for a specified duration.
 * 
 * This set of overloaded functions causes an LED to blink by turning it on, waiting for a specified duration, and then turning it off. 
 * The LED can blink in different colors or use the previously set color.
 * 
 * There are multiple overloads of this function:
 * 1. Blinks an LED at a specific index using the previously set color.
 * 2. Blinks an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Blinks an LED at a specific index and sets the color using individual red, green, and blue values.
 * 4. Blinks an LED at a specific index for a given duration and repeats the blinking a set number of times.
 * 5. Blinks an LED at a specific index and sets the color using a 24-bit color code for a given duration and repeats the blinking a set number of times.
 * 6. Blinks an LED at a specific index and sets the color using individual RGB values for a given duration and repeats the blinking a set number of times.
 * 
 * @param index The index of the LED to blink (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second and fifth overloads).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param duration The duration (in milliseconds) for the LED to stay on before turning off.
 * @param count The number of times to blink the LED (used in the fourth, fifth, and sixth overloads).
 * 
 * @details
 * - The LED at the specified `index` will blink for the given duration.
 * - In the first three overloads, the LED blinks once and then turns off.
 * - In the last three overloads, the LED blinks a specified number of times (`count`) with the given duration between each blink.
 * - The LED color can be controlled using a 24-bit color code or individual RGB values, depending on the function overload used.
 * - The function uses `vTaskDelay` to create the delay for the duration of the blink. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * ledBlink(0, 1000);             // Blink the first LED with the previously set color for 1 second.
 * ledBlink(1, 0xFF0000, 1000);   // Blink the second LED with red color (using color code) for 1 second.
 * ledBlink(2, 255, 0, 0, 1000);  // Blink the third LED with red color (using RGB) for 1 second.
 * ledBlink(0, 1000, 3);          // Blink the first LED 3 times, each blink lasting 1 second.
 * @endcode
 */
void ledBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, int duration);

/**
 * @brief Blinks an individual LED for a specified duration.
 * 
 * This set of overloaded functions causes an LED to blink by turning it on, waiting for a specified duration, and then turning it off. 
 * The LED can blink in different colors or use the previously set color.
 * 
 * There are multiple overloads of this function:
 * 1. Blinks an LED at a specific index using the previously set color.
 * 2. Blinks an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Blinks an LED at a specific index and sets the color using individual red, green, and blue values.
 * 4. Blinks an LED at a specific index for a given duration and repeats the blinking a set number of times.
 * 5. Blinks an LED at a specific index and sets the color using a 24-bit color code for a given duration and repeats the blinking a set number of times.
 * 6. Blinks an LED at a specific index and sets the color using individual RGB values for a given duration and repeats the blinking a set number of times.
 * 
 * @param index The index of the LED to blink (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second and fifth overloads).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param duration The duration (in milliseconds) for the LED to stay on before turning off.
 * @param count The number of times to blink the LED (used in the fourth, fifth, and sixth overloads).
 * 
 * @details
 * - The LED at the specified `index` will blink for the given duration.
 * - In the first three overloads, the LED blinks once and then turns off.
 * - In the last three overloads, the LED blinks a specified number of times (`count`) with the given duration between each blink.
 * - The LED color can be controlled using a 24-bit color code or individual RGB values, depending on the function overload used.
 * - The function uses `vTaskDelay` to create the delay for the duration of the blink. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * ledBlink(0, 1000);             // Blink the first LED with the previously set color for 1 second.
 * ledBlink(1, 0xFF0000, 1000);   // Blink the second LED with red color (using color code) for 1 second.
 * ledBlink(2, 255, 0, 0, 1000);  // Blink the third LED with red color (using RGB) for 1 second.
 * ledBlink(0, 1000, 3);          // Blink the first LED 3 times, each blink lasting 1 second.
 * @endcode
 */
void ledBlink(uint8_t index, int duration, int count);

/**
 * @brief Blinks an individual LED for a specified duration.
 * 
 * This set of overloaded functions causes an LED to blink by turning it on, waiting for a specified duration, and then turning it off. 
 * The LED can blink in different colors or use the previously set color.
 * 
 * There are multiple overloads of this function:
 * 1. Blinks an LED at a specific index using the previously set color.
 * 2. Blinks an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Blinks an LED at a specific index and sets the color using individual red, green, and blue values.
 * 4. Blinks an LED at a specific index for a given duration and repeats the blinking a set number of times.
 * 5. Blinks an LED at a specific index and sets the color using a 24-bit color code for a given duration and repeats the blinking a set number of times.
 * 6. Blinks an LED at a specific index and sets the color using individual RGB values for a given duration and repeats the blinking a set number of times.
 * 
 * @param index The index of the LED to blink (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second and fifth overloads).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param duration The duration (in milliseconds) for the LED to stay on before turning off.
 * @param count The number of times to blink the LED (used in the fourth, fifth, and sixth overloads).
 * 
 * @details
 * - The LED at the specified `index` will blink for the given duration.
 * - In the first three overloads, the LED blinks once and then turns off.
 * - In the last three overloads, the LED blinks a specified number of times (`count`) with the given duration between each blink.
 * - The LED color can be controlled using a 24-bit color code or individual RGB values, depending on the function overload used.
 * - The function uses `vTaskDelay` to create the delay for the duration of the blink. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * ledBlink(0, 1000);             // Blink the first LED with the previously set color for 1 second.
 * ledBlink(1, 0xFF0000, 1000);   // Blink the second LED with red color (using color code) for 1 second.
 * ledBlink(2, 255, 0, 0, 1000);  // Blink the third LED with red color (using RGB) for 1 second.
 * ledBlink(0, 1000, 3);          // Blink the first LED 3 times, each blink lasting 1 second.
 * @endcode
 */
void ledBlink(uint8_t index, uint32_t colorcode, int duration, int count);

/**
 * @brief Blinks an individual LED for a specified duration.
 * 
 * This set of overloaded functions causes an LED to blink by turning it on, waiting for a specified duration, and then turning it off. 
 * The LED can blink in different colors or use the previously set color.
 * 
 * There are multiple overloads of this function:
 * 1. Blinks an LED at a specific index using the previously set color.
 * 2. Blinks an LED at a specific index and sets the color using a 24-bit color code.
 * 3. Blinks an LED at a specific index and sets the color using individual red, green, and blue values.
 * 4. Blinks an LED at a specific index for a given duration and repeats the blinking a set number of times.
 * 5. Blinks an LED at a specific index and sets the color using a 24-bit color code for a given duration and repeats the blinking a set number of times.
 * 6. Blinks an LED at a specific index and sets the color using individual RGB values for a given duration and repeats the blinking a set number of times.
 * 
 * @param index The index of the LED to blink (0-based, where 0 is the first LED).
 * @param colorcode (optional) A 24-bit color code to set the LED's color (used in the second and fifth overloads).
 * @param r (optional) The red value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param g (optional) The green value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param b (optional) The blue value (0-255) to set the LED's color (used in the third and sixth overloads).
 * @param duration The duration (in milliseconds) for the LED to stay on before turning off.
 * @param count The number of times to blink the LED (used in the fourth, fifth, and sixth overloads).
 * 
 * @details
 * - The LED at the specified `index` will blink for the given duration.
 * - In the first three overloads, the LED blinks once and then turns off.
 * - In the last three overloads, the LED blinks a specified number of times (`count`) with the given duration between each blink.
 * - The LED color can be controlled using a 24-bit color code or individual RGB values, depending on the function overload used.
 * - The function uses `vTaskDelay` to create the delay for the duration of the blink. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * ledBlink(0, 1000);             // Blink the first LED with the previously set color for 1 second.
 * ledBlink(1, 0xFF0000, 1000);   // Blink the second LED with red color (using color code) for 1 second.
 * ledBlink(2, 255, 0, 0, 1000);  // Blink the third LED with red color (using RGB) for 1 second.
 * ledBlink(0, 1000, 3);          // Blink the first LED 3 times, each blink lasting 1 second.
 * @endcode
 */
void ledBlink(uint8_t index, uint8_t r, uint8_t g, uint8_t b, int duration, int count);

// *********************************************************************************************************************

/**
 * @brief Turns off an individual LED.
 * 
 * This function turns off the LED at the specified index by setting its color to black (i.e., RGB values of 0 for red, green, and blue).
 * After turning off the LED, it updates the display by calling `FastLED.show()`.
 * 
 * @param index The index of the LED to turn off (0-based, where 0 is the first LED).
 * 
 * @details
 * - The function checks if the `index` is valid (i.e., it is less than the total number of LEDs defined by `LED_COUNT`).
 * - The LED at the specified `index` is turned off by setting its color to black using the `CRGB::Black` constant.
 * - The `FastLED.show()` function is called to immediately update the LED strip and reflect the change.
 * 
 * Example Usage:
 * @code
 * ledOff(0);  // Turn off the first LED.
 * ledOff(2);  // Turn off the third LED.
 * @endcode
 */
void ledOff(uint8_t index);

// *********************************************************************************************************************

/**
 * @brief Controls the LED behavior based on the system's current state.
 *
 * This function updates the state of the LED (on, off, or specific color) based on the current state of the system. 
 * The LED color and behavior change according to predefined states, providing visual feedback to the user 
 * about the system's current operation.
 *
 * @details
 * The function uses the global `g_vars_ptr->state` to determine the current system state and sets the LED color 
 * accordingly. The LED colors represent different operations or stages of the system, including:
 * 1. **Menu States**: Default state for system initialization or idle operations.
 * 2. **RFID Operations**: States related to adding, deleting, or checking RFID tags.
 * 3. **PIN Entry Operations**: States related to entering or confirming PIN codes.
 * 4. **Alarm and Test Operations**: Various alarm and test states that indicate specific conditions.
 *
 * LED colors are defined using RGB values (Red, Green, Blue), and the LED behavior (such as blinking) is set based 
 * on the current state.
 *
 * @note The `ledOn()` function is used to turn on the LED with specific color values, and the `ledOff()` function 
 *       is used to turn off the LED.
 *
 * @see ledOn(), ledOff()
 */
void lightLedByState();

/**
 * @brief Controls the LED based on the system's power mode and battery level.
 *
 * This function adjusts the behavior of LED 0 depending on the system's power mode and battery level.
 * It provides visual feedback about the current battery level, changing the LED color from red to green
 * based on the battery charge percentage. The LED is turned off when the system is in power-saving mode.
 *
 * @details
 * The function checks the system's `g_vars_ptr->power_mode` and `g_vars_ptr->battery_level` to determine 
 * the LED behavior:
 * 1. **Power Mode**: If the system is in power-saving mode (`g_vars_ptr->power_mode` is true), the LED is turned off.
 * 2. **Battery Level**: The LED color represents the battery level. The color transitions from red (low battery) to green (high battery). 
 *    The exact color is determined by the battery percentage:
 *    - Red (255, 0, 0) when the battery is at 0%.
 *    - Green (0, 255, 0) when the battery is at 100%.
 *    - A gradient from red to green for intermediate levels.
 *
 * @note The function uses the `ledOn()` function to set the LED color based on the battery level. 
 *       The LED may blink for 5 seconds when the system is not in power-saving mode.
 *
 * @see ledOn()
 */
void ledByBattery();

// *********************************************************************************************************************

/**
 * @brief Starts the LED blinking task.
 * 
 * This function checks if the LED blinking task (`ledTask`) is already running. If it is, the task is deleted and restarted to ensure it starts fresh.
 * A new task is then created to manage the LED blinking behavior.
 * 
 * @details
 * - If the task handle `handleTaskLed` is not `NULL`, the existing task is deleted using `vTaskDelete`.
 * - A new task (`ledTask`) is created to manage LED blinking, using `xTaskCreate`.
 * 
 * Example Usage:
 * @code
 * ledBlinkStart();  // Start the LED blinking task.
 * @endcode
 */
void ledBlinkStart();

/**
 * @brief Stops the LED blinking task and resets related parameters.
 * 
 * This function stops the LED blinking task by resetting parameters associated with the blinking, such as `ledRunning`, `ledDuration`, and `ledBlinkColors`.
 * It also ensures that the task handle `handleTaskLed` is deleted, effectively stopping the task from running.
 * 
 * @details
 * - Resets the states of all LEDs and their blinking durations.
 * - Deletes the existing LED task if it's running, ensuring no LED task continues after stopping.
 * 
 * Example Usage:
 * @code
 * ledBlinkStop();  // Stop the LED blinking task and reset LED states.
 * @endcode
 */
void ledBlinkStop();

// *********************************************************************************************************************

/**
 * @brief Turns the buzzer on by setting the corresponding GPIO pin high.
 *
 * This function activates the buzzer by setting the `PIEZZO_DATA_PIN` to a high state
 * using the `gpio_extender.write()` function. This action will turn the buzzer on, producing sound.
 *
 * @details
 * The function writes a HIGH signal to the buzzer control pin (`PIEZZO_DATA_PIN`) through the GPIO extender.
 * This is typically used to initiate sound generation in a piezo buzzer connected to the GPIO extender.
 * The exact duration and pattern of the sound are not handled by this function; it only activates the buzzer.
 *
 */
void buzzerOn();

/**
 * @brief Turns the buzzer off by setting the corresponding GPIO pin low.
 *
 * This function deactivates the buzzer by setting the `PIEZZO_DATA_PIN` to a low state
 * using the `gpio_extender.write()` function. This action will turn the buzzer off, stopping the sound.
 *
 * @details
 * The function writes a LOW signal to the buzzer control pin (`PIEZZO_DATA_PIN`) through the GPIO extender.
 * This is typically used to stop the sound generation in a piezo buzzer connected to the GPIO extender.
 * The buzzer will remain off as long as the pin remains LOW.
 *
 */
void buzzerOff();

/**
 * @brief Beeps the buzzer for a specified duration.
 * 
 * This set of overloaded functions causes the buzzer to beep by turning it on for a specified duration and then turning it off. 
 * The buzzer can beep a specified number of times with a delay between each beep.
 * 
 * There are two overloads of this function:
 * 1. Beeps the buzzer for a specified duration and turns it off.
 * 2. Beeps the buzzer for a specified duration and repeats the beeping a set number of times.
 * 
 * @param duration The duration (in milliseconds) for which the buzzer will be on before turning off.
 * @param count The number of times to beep the buzzer (used in the second overload).
 * 
 * @details
 * - The buzzer will be turned on for the specified `duration` and then turned off.
 * - In the first overload, the buzzer beeps once.
 * - In the second overload, the buzzer beeps for the specified `duration` and repeats the beeping a set number of times (`count`).
 * - The function uses `vTaskDelay` to create the delay for the duration of the beep. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * buzzerBeep(500);           // Beep the buzzer for 500 milliseconds.
 * buzzerBeep(500, 3);        // Beep the buzzer 3 times, each beep lasting 500 milliseconds.
 * @endcode
 */
void buzzerBeep(int duration);

/**
 * @brief Beeps the buzzer for a specified duration.
 * 
 * This set of overloaded functions causes the buzzer to beep by turning it on for a specified duration and then turning it off. 
 * The buzzer can beep a specified number of times with a delay between each beep.
 * 
 * There are two overloads of this function:
 * 1. Beeps the buzzer for a specified duration and turns it off.
 * 2. Beeps the buzzer for a specified duration and repeats the beeping a set number of times.
 * 
 * @param duration The duration (in milliseconds) for which the buzzer will be on before turning off.
 * @param count The number of times to beep the buzzer (used in the second overload).
 * 
 * @details
 * - The buzzer will be turned on for the specified `duration` and then turned off.
 * - In the first overload, the buzzer beeps once.
 * - In the second overload, the buzzer beeps for the specified `duration` and repeats the beeping a set number of times (`count`).
 * - The function uses `vTaskDelay` to create the delay for the duration of the beep. The delay is given in milliseconds and adjusted to the FreeRTOS tick rate using `portTICK_PERIOD_MS`.
 * 
 * Example Usage:
 * @code
 * buzzerBeep(500);           // Beep the buzzer for 500 milliseconds.
 * buzzerBeep(500, 3);        // Beep the buzzer 3 times, each beep lasting 500 milliseconds.
 * @endcode
 */
void buzzerBeep(int duration, int count);

// *********************************************************************************************************************

/**
 * @brief Starts the buzzer beep task with a specified duration.
 * 
 * This function starts a FreeRTOS task to control the buzzer's beeping behavior. The task will repeatedly beep the buzzer for the specified duration.
 * If the buzzer task is already running, it is stopped and restarted with the new duration.
 * 
 * @param duration The duration of each beep in milliseconds.
 * 
 * @details
 * - If the buzzer task is already running, it will be stopped by setting `buzzerRunning` to `false` and deleting the existing task using `vTaskDelete`.
 * - A new task (`buzzerTask`) is created to handle the beeping with the given duration.
 * - The task will continue running until `buzzerRunning` is set to `false`.
 * 
 * Example Usage:
 * @code
 * buzzerBeepStart(500);  // Start the buzzer beeping with a 500ms duration per beep.
 * @endcode
 */
void buzzerBeepStart(int duration);

/**
 * @brief Stops the buzzer beep task.
 * 
 * This function stops the buzzer beep task by setting `buzzerRunning` to `false`. 
 * If the buzzer task is running, it will stop the beeping and delete the task.
 * 
 * @details
 * - Stops the buzzer task if it is running by setting `buzzerRunning` to `false`.
 * - Does not delete the task explicitly but ensures that the task will exit cleanly by checking `buzzerRunning`.
 * 
 * Example Usage:
 * @code
 * buzzerBeepStop();  // Stop the buzzer beeping task.
 * @endcode
 */
void buzzerBeepStop();

// *********************************************************************************************************************

/**
 * @brief Reads and calculates the battery voltage from the specified analog pin.
 *
 * This function reads the battery voltage from the specified analog input pin (`BATTERY_VOLTAGE_PIN`) using 
 * the ADC (Analog-to-Digital Converter). It then calculates the battery's actual voltage using a voltage 
 * divider circuit, which is factored into the calculation. The result is the battery voltage in volts.
 * 
 * The ADC value is scaled from a range of 0-4095, corresponding to an input voltage range of 0.1V to 3.2V.
 * A voltage divider is applied to adjust the measurement, with the calculated voltage output scaled by a factor 
 * of 4.25 to account for the divider circuit.
 *
 * @return The calculated battery voltage in volts. If there is an error in reading the battery voltage, 
 *         the function returns -1.
 *
 * @details
 * The function performs the following steps:
 * 1. Reads the ADC value from the `BATTERY_VOLTAGE_PIN`.
 * 2. If the ADC value is greater than 0, it calculates the battery voltage using the formula:
 *    \[
 *    \text{{battery\_voltage}} = \left( \frac{{\text{{meas\_battery}}}}{{4095}} \times 3.15 + 0.15 \right) \times 4.25
 *    \]
 *    where `meas_battery` is the ADC reading.
 * 3. If the ADC value is 0, the function returns -1 to indicate an error in measurement.
 *
 * The formula takes into account both the ADC reading and the voltage divider used in the hardware circuit,
 * resulting in a more accurate battery voltage reading.
 *
 * @note This function assumes the input voltage range of the ADC is between 0.1V and 3.2V, and the voltage 
 *       divider factor is 4.25. Ensure that the `BATTERY_VOLTAGE_PIN` is connected to the correct analog 
 *       input for accurate readings.
 *
 * Example Usage:
 * @code
 * float batteryVoltage = getBatteryVoltage();
 * if (batteryVoltage != -1) {
 *     Serial.print("Battery voltage: ");
 *     Serial.println(batteryVoltage);
 * } else {
 *     Serial.println("Failed to read battery voltage.");
 * }
 * @endcode
 */
float getBatteryVoltage();

/**
 * @brief Measures the DC voltage on the input pin and returns the calculated voltage.
 *
 * This function reads the analog value from the specified pin (`DC_VOLTAGE_PIN`) and converts it to a 
 * corresponding voltage based on the known scaling factor. The conversion is done by first calculating 
 * a preliminary voltage using the ADC value, and then applying a multiplier to scale it appropriately.
 * If the ADC reading is non-positive, the function returns `-1` to indicate an error or invalid reading.
 *
 * @return The measured DC voltage (in volts). If the measurement is valid, it returns a positive float value;
 *         if the measurement is invalid, it returns `-1`.
 *
 * @details
 * - The analog read value from the ADC ranges from 0 to 4095 (12-bit resolution).
 * - The calculation assumes a voltage divider or scaling factor where the input voltage is within the range
 *   of 0.1V to 3.2V for the ADC, and the function scales it to a higher voltage value using the formula:
 *     - `((measured_adc_value / 4095) * 3.15 + 0.15) * 4.25`
 * - If the ADC measurement is greater than 0, the function calculates the voltage and returns it.
 * - If the ADC value is `0` (or lower), the function returns `-1`, indicating a failure to measure the voltage.
 *
 * Example Usage:
 * @code
 * float voltage = getDCVoltage();
 * if (voltage > 0) {
 *     Serial.printf("Measured DC voltage: %.2f V\n", voltage);
 * } else {
 *     Serial.println("Failed to measure DC voltage.");
 * }
 * @endcode
 */
float getDCVoltage();

// *********************************************************************************************************************

/**
 * @brief Refreshes the battery level based on the current battery voltage.
 *
 * This function reads the current battery voltage using the `getBatteryVoltage()` function, then 
 * determines the battery percentage based on a predefined voltage-to-percentage lookup table. 
 * The lookup table contains voltage thresholds and corresponding battery percentages. 
 * Linear interpolation is used to calculate the battery percentage for values between defined thresholds.
 * 
 * The resulting battery percentage is stored in the global variable `g_vars_ptr->battery_level`.
 *
 * @details
 * - The function uses a predefined table of 21 voltage thresholds (`voltageTable`) and corresponding
 *   battery percentages (`percentageTable`).
 * - The voltage is clamped within the range of 4.20V (100%) to 3.00V (0%) to ensure valid values.
 * - If the voltage is within the table range, the battery percentage is determined by linear interpolation.
 * - If the voltage is above the maximum threshold (4.20V), the battery level is set to 100%.
 * - If the voltage is below the minimum threshold (3.00V), the battery level is set to 0%.
 * 
 * @return None
 *
 * Example Usage:
 * @code
 * refreshBatteryLevel();
 * Serial.printf("Battery level: %d%%\n", g_vars_ptr->battery_level);
 * @endcode
 */
void refreshBatteryLevel();

/**
 * @brief Refreshes the power mode based on the current DC voltage.
 *
 * This function reads the current DC voltage using the `getDCVoltage()` function and updates the 
 * system's power mode accordingly. If the DC voltage exceeds a defined threshold (`DC_VOLTAGE_THRESHOLD`), 
 * the power mode is set to DC (indicating the system is powered by the DC input). If the voltage is below the 
 * threshold, the system switches to battery power mode.
 * 
 * The resulting power mode is stored in the global variable `g_vars_ptr->power_mode`.
 *
 * @details
 * - The function uses the DC voltage value obtained from the `getDCVoltage()` function.
 * - If the DC voltage is greater than or equal to the threshold (`DC_VOLTAGE_THRESHOLD`), the power mode is set to `true`, indicating DC power mode.
 * - If the DC voltage is below the threshold, the power mode is set to `false`, indicating battery power mode.
 * - The power mode is logged for debugging purposes.
 * 
 * @return None
 *
 * Example Usage:
 * @code
 * refreshPowerMode();
 * if (g_vars_ptr->power_mode) {
 *     Serial.println("Powered by DC.");
 * } else {
 *     Serial.println("Powered by Battery.");
 * }
 * @endcode
 */
void refreshPowerMode();

#endif