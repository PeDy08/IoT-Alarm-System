/**
 * @file libWiFi.h
 * @brief Contains functions and definitions for managing WiFi functionality on ESP32.
 * 
 * Contains functions and definitions for managing WiFi functionality on ESP32.
 */

#ifndef LIBWIFI_H_DEFINITION
#define LIBWIFI_H_DEFINITION

#include <WiFi.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include "mainAppDefinitions.h"
#include "libJson.h"
#include "libAuth.h"
#include "utils.h"

#define WIFI_AP_SSID "IoT Alarm Setup"
#define WIFI_AP_PSWD ""

/**
 * @brief Starts the Wi-Fi Access Point and web server for configuration.
 * 
 * This function switches the ESP32 to Wi-Fi AP mode and starts the web server.
 * On the web page, users will be able to enter a new config values.
 */
void startWifiSetupMode();

/**
 * @brief Starts the Wi-Fi Access Point and web server for configuration.
 * @param g_vars Pointer to global variable structure.
 * @param g_config Pointer to global configuration structure.
 * 
 * This function switches the ESP32 to Wi-Fi STA mode and starts the web server.
 * Web server provides user defined nodes. To configure, open `libWiFi.cpp` file.
 */
void startWiFiServerMode(g_vars_t * g_vars, g_config_t * g_config);

#endif
