/**
 * @file libDisplay.h
 * @brief Contains functions and definitions for controlling display.
 * 
 * Contains functions and definitions for controlling display.
 */

#ifndef LIBDISPLAY_H_DEFINITION
#define LIBDISPLAY_H_DEFINITION

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "utils.h"
#include "libAuth.h"
#include "mainAppDefinitions.h"

#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_ADDR 0x27

extern LiquidCrystal_I2C lcd;

void authScreenC(g_vars_t * g_vars);
void authScreenE(g_vars_t * g_vars);
void authScreenS(g_vars_t * g_vars);
void rfidScreenC(g_vars_t * g_vars, const char * uid);
void rfidScreenE(g_vars_t * g_vars, const char * uid);
void rfidScreenA(g_vars_t * g_vars, const char * uid);
void rfidScreenD(g_vars_t * g_vars, const char * uid);
void loadScreen(g_vars_t * g_vars, g_config_t * g_config, bool reboot = false);

void loadScreenInit(g_vars_t * g_vars);
void loadScreenSetup(g_vars_t * g_vars);
void loadScreenSetupAP(g_vars_t * g_vars);
void loadScreenSetupRfid(g_vars_t * g_vars);

void loadScreenEnterPin(g_vars_t * g_vars);
void loadScreenEnterPinFirst(g_vars_t * g_vars);
void loadScreenEnterPinSecond(g_vars_t * g_vars);

void loadScreenAlarm(g_vars_t * g_vars);
void loadScreenTest(g_vars_t * g_vars);

#endif