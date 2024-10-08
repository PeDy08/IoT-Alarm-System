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
#include "libAuth.h"
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
 * @brief Function for handeling asynd keypad events.
 * 
 * Recognises the active state and decides what to do with keypad event.
 */
void keypadEvent(g_vars_t * g_vars, char key);

#endif