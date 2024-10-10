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
#include "libDisplay.h"
#include "libGsm.h"

#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_CLK 18
#define SD_CS_PIN 16

extern g_config_t g_config;
extern g_vars_t g_vars;

void setup();
void loop();

// WORKING TASKS
TaskHandle_t handleTaskMenu = NULL;
void rtosMenu(void* parameters);

TaskHandle_t handleTaskKeypad = NULL;
void rtosKeypad(void* parameters);

TaskHandle_t handleTaskNet = NULL;
void rtosNet(void* parameters);

TaskHandle_t handleTaskSetup = NULL;
void rtosSetup(void* parameters);

TaskHandle_t handleTaskRfid = NULL;
void rtosRfid(void* parameters);

TaskHandle_t handleTaskGsm = NULL;
void rtosGsm(void* parameters);

// REFRESH TASKS
TaskHandle_t handleTaskMenuRefresh = NULL;
void rtosMenuRefresh(void* parameters);

TaskHandle_t handleTaskRfidRefresh = NULL;
void rtosRfidRefresh(void* parameters);

/**
 * @brief Sets various global variables depending on the provided arguments. This function modifies
 *        the current state, selection, maximum selection, pin, and attempts based on the input parameters.
 *        Any parameter not provided (or set to its default value) will leave the corresponding variable unchanged.
 *
 * @param state         The new state to be set. Defaults to `STATE_MAX` which leaves the current state unchanged.
 *                      If a valid state is provided, the global state variable `g_vars.state` will be updated.
 *
 * @param selection     The current selection index. Defaults to `-1` which leaves the current selection unchanged.
 *                      If a valid selection is provided, the global variable `g_vars.selection` will be updated.
 *
 * @param selection_max The maximum allowed selection index. Defaults to `-1` which leaves the current max selection unchanged.
 *                      If a valid max selection is provided, the global variable `g_vars.selection_max` will be updated.
 *
 * @param pin           The current PIN value. Defaults to `"NULL"` which leaves the current PIN unchanged.
 *                      If a valid PIN is provided, the global variable `g_vars.pin` will be updated.
 *
 * @param attempts      The number of remaining attempts for some operation. Defaults to `-1` which leaves the current 
 *                      attempt count unchanged. If a valid attempt count is provided, the global variable `g_vars.attempts`
 *                      will be updated.
 *
 * @note The function allows selective updating of global variables, ensuring that any parameter left at its default
 *       value does not alter the corresponding global variable.
 */
inline void setState(States state = STATE_MAX, int selection = -1, int selection_max = -1, String pin = "NULL", int attempts = -1) {
  if (state != STATE_MAX) {
    g_vars.state_prev = g_vars.state;
    g_vars.selection_prev = g_vars.selection;
    g_vars.state = state;
  }

  if (selection != -1) {
    g_vars.selection = selection;
  }

  if (selection_max != -1) {
    g_vars.selection_max_prev = g_vars.selection_max;
    g_vars.selection_max = selection_max;
  }

  if (pin != "NULL") {
    Serial.println(pin);
    g_vars.pin = pin;
  }

  if (attempts != -1) {
    g_vars.attempts = attempts;
  }
}
