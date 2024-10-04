/**
 * @file libKeypad.h
 * @brief Contains functions and definitions for managing 4x4 keypad module.
 * 
 * Contains functions and definitions for managing 4x4 keypad module.
 */

#ifndef LIBKEYPAD_H_DEFINITION
#define LIBKEYPAD_H_DEFINITION

#include <Wire.h>
#include <I2CKeyPad8x8.h>

#include "mainAppDefinitions.h"
#include "libHash.h"
#include "utils.h"

extern const uint8_t KEYPAD_I2C_ADDRESS;
extern I2CKeyPad8x8 keypad;

/**
 * @brief Function for determing if char is correctly mapped and valid.
 * 
 * If char on input is "\0", " ", "N" or "F" function will return false.
 * Otherwise true.
 */
bool isValidChar(char input);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_INIT is active.
 */
void keyFxPressInit(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_SETUP is active.
 */
void keyFxPressSetup(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_ALARM_IDLE is active.
 */
void keyFxPressAlarmIdle(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_TEST_IDLE is active.
 */
void keyFxPressTestIdle(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_ALARM_LOCK_ENTER_PIN is active.
 */
void keyFxPressAlarmLockEnterPin(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_TEST_LOCK_ENTER_PIN is active.
 */
void keyFxPressTestLockEnterPin(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_ALARM_UNLOCK_ENTER_PIN is active.
 */
void keyFxPressAlarmUnlockEnterPin(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_TEST_UNLOCK_ENTER_PIN is active.
 */
void keyFxPressTestUnlockEnterPin(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_ALARM_CHANGE_ENTER_PIN1 is active.
 */
void keyFxPressAlarmChangeEnterPin1(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_TEST_CHANGE_ENTER_PIN1 is active.
 */
void keyFxPressTestChangeEnterPin1(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_ALARM_CHANGE_ENTER_PIN2 is active.
 */
void keyFxPressAlarmChangeEnterPin2(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_TEST_CHANGE_ENTER_PIN2 is active.
 */
void keyFxPressTestChangeEnterPin2(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_ALARM_CHANGE_ENTER_PIN3 is active.
 */
void keyFxPressAlarmChangeEnterPin3(char key, g_vars_t * g_vars);

/**
 * @brief Callback function for keypad event.
 * 
 * This function is called when any keypad button is pressed and STATE_TEST_CHANGE_ENTER_PIN3 is active.
 */
void keyFxPressTestChangeEnterPin3(char key, g_vars_t * g_vars);

#endif