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

#ifdef EINK
#include "libDisplayEINK.h"
#endif

#ifdef LCD
#include "libDisplayLCD.h"
#endif

#define WIFI_AP_SSID "IoT Alarm Setup"
#define WIFI_AP_PSWD ""

/**
 * @brief Starts the WiFi setup mode on the ESP32, configuring it as an Access Point (AP) and hosting a web server.
 * 
 * This function configures the ESP32 in WiFi AP mode, with the specified SSID and password. It then starts a web server that serves 
 * HTML files from the SD card, allowing the user to configure the WiFi settings (SSID, password, IP, gateway, and subnet mask) via 
 * a web interface. The settings are collected, validated, saved, and applied by the device, followed by a restart.
 * 
 * @param None
 * 
 * @return None
 * 
 * @details 
 * The function:
 * 1. Sets the ESP32 as an AP with the configured SSID and password.
 * 2. Displays the AP details (SSID, password, and IP address) on the notification screen.
 * 3. Configures the web server to serve an HTML page for WiFi setup and handle configuration submissions.
 * 4. Upon receiving the configuration, the WiFi settings are validated and saved to the device.
 * 5. If the configuration is successfully saved, the device restarts with the new settings.
 * 
 * @code
 * startWifiSetupMode();
 * @endcode
 */
void startWifiSetupMode();

/**
 * @brief Initializes the ESP32 Wi-Fi in STA mode and starts an HTTP server to handle configuration and file download/upload requests.
 *
 * This function sets up the ESP32 to operate in Wi-Fi Station (STA) mode, allowing it to connect to a Wi-Fi network.
 * It attempts to configure advanced Wi-Fi settings if provided in the configuration struct, and then begins the connection process
 * using the SSID and password from the configuration. It also sets up various routes on the HTTP server to serve web pages, handle
 * login/logout actions, accept configuration uploads, and allow downloading of specific files from the SD card. After saving the
 * configuration, the device will restart to apply the new settings.
 *
 * @param g_vars A pointer to the global variables structure containing current Wi-Fi mode and other system variables.
 * @param g_config A pointer to the configuration structure containing Wi-Fi settings, MQTT parameters, and alarm settings.
 * 
 * @return None.
 *
 * @details
 * This function performs the following tasks:
 * 1. Configures the Wi-Fi in Station mode (`WIFI_MODE_STA`) and attempts to apply advanced Wi-Fi settings if valid.
 * 2. Begins the connection to the Wi-Fi network using the SSID and password from `g_config`.
 * 3. Sets up an HTTP server with several routes:
 *    - `/` serves the main page, requiring authentication.
 *    - `/login` and `/logout` handle login/logout actions.
 *    - `/setup` serves a setup page and handles POST requests to update the configuration.
 *    - `/download/*` allows downloading various files (logs, password, RFID, configuration).
 *    - `/upload/config` accepts a configuration file and writes it to the SD card, then restarts the device.
 * 4. On successful configuration update, the system saves the configuration and restarts to apply the changes.
 *
 * @code
 * // Example of calling startWiFiServerMode
 * startWiFiServerMode(&globalVars, &deviceConfig);
 * @endcode
 */
void startWiFiServerMode();

#endif
