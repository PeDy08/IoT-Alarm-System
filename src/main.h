/**
 * @file main.h
 * @brief Main header file containing functions declarations.
 */

#include <Arduino.h>

#include "mainAppDefinitions.h"

#include "utils.h"
#include "libKeypad.h"
#include "libJson.h"
#include "libWiFi.h"
#include "libAuth.h"

void keypadEvent(char key);
void setup();
void loop();

TaskHandle_t handleTaskMenu = NULL;
void rtosMenu(void* parameters);

TaskHandle_t handleTaskKeypad = NULL;
void rtosKeypad(void* parameters);

TaskHandle_t handleTaskNet = NULL;
void rtosNet(void* parameters);

TaskHandle_t handleTaskSetup = NULL;
void rtosSetup(void* parameters);

TaskHandle_t handleTaskMenuRefresh = NULL;
void rtosMenuRefresh(void* parameters);

TaskHandle_t handleTaskRfid = NULL;
void rtosRfid(void* parameters);