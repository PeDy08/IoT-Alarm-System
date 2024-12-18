/**
 * @file libKeypad.h
 * @brief Contains functions and definitions for managing 4x4 keypad module.
 * 
 * Contains functions and definitions for managing 4x4 keypad module.
 */

#ifndef LIBKEYPAD_H_DEFINITION
#define LIBKEYPAD_H_DEFINITION

#include <Wire.h>
#include <I2CKeyPad.h>

#include "mainAppDefinitions.h"
#include "libAuth.h"
#include "libDisplayEINK.h"
#include "utils.h"

extern const uint8_t KEYPAD_I2C_ADDRESS;
extern I2CKeyPad keypad;

/**
 * @brief Validates whether a character is allowed.
 *
 * This function checks if the given character is not part of a predefined set of invalid characters.
 * The invalid characters are: null character (`'\0'`), space (`' '`), `'N'`, and `'F'`. 
 * If the input character matches any of these, the function returns `false`. Otherwise, it returns `true`.
 *
 * @param input The character to be validated.
 * - The function checks if `input` is one of the invalid characters.
 * 
 * @return `true` if the character is valid, `false` if it is invalid.
 * 
 * @details This function helps in ensuring that only valid characters are processed in the system. It provides a quick check against a small set of characters defined as invalid.
 */
bool isValidChar(char input);

/**
 * @brief Handles keypad events based on the current system state.
 *
 * This function processes key inputs from the keypad and triggers appropriate actions based on the system's current state. The function performs different operations depending on whether the system is in an initialization, setup, alarm, test, or PIN entry state. 
 * It also handles turning off secondary alarm triggers (`alarm_fire`, `alarm_water`, `alarm_electricity`) when certain conditions are met and logs this action.
 *
 * @param key The character representing the pressed key.
 * - Depending on the current state, the function may call different key handling functions such as `keyFxMenu`, `keyFxConfirm`, `keyFxRecord`, or `keyFxRecordTest`.
 * - In some states, the function may trigger a system reboot or restart of displays.
 * 
 * @return void
 * 
 * @details 
 * The function checks the current state of the system (`g_vars_ptr->state`) and performs corresponding actions:
 * - In states like `STATE_INIT`, `STATE_SETUP`, `STATE_ALARM_IDLE`, and `STATE_TEST_IDLE`, it processes the key press for menu navigation and refreshes the display.
 * - In states related to system setup (like `STATE_SETUP_AP`), it triggers a system restart.
 * - In alarm and test states, the function records key input related to PIN management or test modes.
 * - If a key from `A` to `D` is pressed while certain alarms are active, it turns off the secondary alarm triggers (`alarm_fire`, `alarm_water`, `alarm_electricity`) and logs the action.
 * 
 * The function ensures that appropriate actions are taken in response to keypad events while reflecting the system's status and needs.
 */
void keypadEvent(char key);

#endif