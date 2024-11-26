/**
 * @file main.h
 * @brief Main header file containing functions declarations.
 */

#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "mainAppDefinitions.h"

#include "utils.h"
#include "libKeypad.h"
#include "libJson.h"
#include "libWiFi.h"
#include "libAuth.h"
#include "libGsm.h"
#include "libZigbee.h"
#include "libMqtt.h"

#ifdef EINK
#include "libDisplayEINK.h"
#endif

#ifdef LCD
#include "libDisplayLCD.h"
extern LiquidCrystal_I2C display;
#endif

#define IIC_SDA 21
#define IIC_CLK 22
#define SPI_MOSI 16
#define SPI_MISO 4
#define SPI_CLK 17
#define SD_CS_PIN 2

extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

extern g_vars_t g_vars;
extern g_config_t g_config;

void setup();
void loop();

// WORKING TASKS
extern TaskHandle_t handleTaskMenu;
void rtosMenu(void* parameters);

extern TaskHandle_t handleTaskAlarm;
void rtosAlarm(void* testmode);

extern TaskHandle_t handleTaskKeypad;
void rtosKeypad(void* parameters);

extern TaskHandle_t handleTaskNet;
void rtosNet(void* parameters);

extern TaskHandle_t handleTaskDatetime;
void rtosDatetime(void* parameters);

extern TaskHandle_t handleTaskSetup;
void rtosWifiSetup(void* parameters);

extern TaskHandle_t handleTaskRfid;
void rtosRfid(void* parameters);

extern TaskHandle_t handleTaskDisplay;
void rtosDisplay(void* parameters);

extern TaskHandle_t handleTaskGsm;
void rtosGsm(void* parameters);

extern TaskHandle_t handleTaskZigbee;
void rtosZigbee(void* parameters);

extern TaskHandle_t handleTaskMqtt;
void rtosMqtt(void* parameters);

// REFRESH TASKS
extern TaskHandle_t handleTaskMenuRefresh;
void rtosMenuRefresh(void* parameters);

extern TaskHandle_t handleTaskRfidRefresh;
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
    g_vars.pin = pin;
  }

  if (attempts != -1) {
    g_vars.attempts = attempts;
  }

  g_vars.refresh_display.refresh = true;
}
