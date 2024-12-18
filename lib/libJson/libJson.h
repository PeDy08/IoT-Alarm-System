/**
 * @file libJson.h
 * @brief Contains functions and definitions for managing JSON files in ESP32 LittleFS.
 * 
 * Contains functions and definitions for managing JSON files in ESP32 LittleFS.
 */

#ifndef LIBJSON_H_DEFINITION
#define LIBJSON_H_DEFINITION

#include <SD.h>
#include <ArduinoJson.h>

#include "utils.h"
#include "mainAppDefinitions.h"

#define CONFIG_FILE_NAME "config.json"
#define CONFIG_FILE_UPLOAD_NAME "upload_config.json"
#define CONFIG_FILE_PATH "/config/"
#define CONFIG_FILE String(String(CONFIG_FILE_PATH)+String(CONFIG_FILE_NAME)).c_str()
#define CONFIG_UPLOAD_FILE String(String(CONFIG_FILE_PATH)+String(CONFIG_FILE_UPLOAD_NAME)).c_str()

/**
 * @brief Resets the configuration data to default values.
 *
 * This function sets the default values for all configuration parameters in the
 * provided `g_config` structure. It resets the Wi-Fi, MQTT, and alarm-related
 * settings to predefined default values. This function is typically called when
 * initializing or resetting the configuration.
 *
 * @param g_config Pointer to the global configuration structure (g_config_t)
 *                 that will be reset to default values.
 *
 * @return None
 *
 * @details This function updates the following fields of the `g_config` structure:
 *          - Wi-Fi settings: SSID, password, IP address, gateway, and subnet mask.
 *          - MQTT settings: Broker address, port, client ID, topic, username, password, and certificate.
 *          - Alarm settings: Countdown time, threshold values, and telephone number for the alarm.
 *          Default values are set for each of these fields as specified.
 *          
 *          Example usage:
 *          @code
 *          g_config_t config;
 *          setDefaultConfig(&config);
 *          @endcode
 */
void setDefaultConfig(g_config_t *g_config);

/**
 * @brief Sets the configuration data to invalid or placeholder values.
 *
 * This function sets all fields of the provided `g_config` structure to invalid
 * or placeholder values. It is typically used for error handling or when an invalid
 * configuration needs to be flagged. It sets Wi-Fi, MQTT, and alarm-related settings
 * to values that can indicate an invalid or unconfigured state.
 *
 * @param g_config Pointer to the global configuration structure (g_config_t)
 *                 that will be set to invalid values.
 *
 * @return None
 *
 * @details This function updates the following fields of the `g_config` structure:
 *          - Wi-Fi settings: SSID, password, IP address, gateway, and subnet mask to "INVALID".
 *          - MQTT settings: Broker address, port, client ID, topic, username, password, and certificate to "INVALID".
 *          - Alarm settings: Countdown time, threshold values, and telephone number for the alarm to "INVALID".
 *          
 *          Example usage:
 *          @code
 *          g_config_t config;
 *          setInvalidonfig(&config);
 *          @endcode
 */
void setInvalidonfig(g_config_t *g_config);

/**
 * @brief Saves the configuration data to a configuration file on the SD card.
 *
 * This function serializes the current configuration stored in the `g_config` structure and writes it to a JSON file
 * on the SD card. If the configuration file already exists, it will be overwritten. The configuration includes Wi-Fi,
 * MQTT, and alarm settings. If an error occurs during the process, the function returns `false`; otherwise, it returns `true`.
 *
 * @param g_config Pointer to the global configuration structure (`g_config_t`) containing the settings to be saved.
 *
 * @return `true` if the configuration was successfully saved; `false` otherwise.
 *
 * @details This function performs the following steps:
 *          - Checks if a configuration file already exists on the SD card and removes it if it does.
 *          - Creates a new file on the SD card for saving the configuration.
 *          - Serializes the configuration data from the `g_config` structure into a JSON format.
 *          - Writes the serialized JSON data to the configuration file on the SD card.
 *          - If serialization or file operations fail, an error message is logged, and the function returns `false`.
 *          
 *          Example usage:
 *          @code
 *          g_config_t config;
 *          // Fill config with values
 *          if (saveConfig(&config)) {
 *              Serial.println("Configuration saved successfully.");
 *          } else {
 *              Serial.println("Failed to save configuration.");
 *          }
 *          @endcode
 */
bool saveConfig(g_config_t *g_config);

/**
 * @brief Loads configuration data from a JSON file and saves it to the configuration file on the SD card.
 *
 * This function loads configuration data from a JSON file specified by `CONFIG_UPLOAD_FILE`, and if the data is successfully loaded,
 * it then saves the configuration to the default configuration file using the `saveConfig` function. If either loading or saving
 * fails, the function logs an appropriate warning and returns `false`; otherwise, it returns `true`.
 *
 * @param g_config Pointer to the global configuration structure (`g_config_t`) that will hold the loaded configuration.
 *
 * @return `true` if the configuration was successfully loaded and saved; `false` otherwise.
 *
 * @details This function performs the following steps:
 *          - Attempts to load the configuration from the upload file (`CONFIG_UPLOAD_FILE`) using the `loadConfig` function.
 *          - If loading is successful, the configuration is saved to the default configuration file using the `saveConfig` function.
 *          - Logs appropriate messages at each step of the process, including success or failure of loading and saving the configuration.
 *          
 *          Example usage:
 *          @code
 *          g_config_t config;
 *          if (saveConfigFromJSON(&config)) {
 *              Serial.println("Configuration successfully saved from uploaded file.");
 *          } else {
 *              Serial.println("Failed to save configuration from uploaded file.");
 *          }
 *          @endcode
 */
bool saveConfigFromJSON(g_config_t *g_config);

/**
 * @brief Rewrites configuration data from a source configuration to a destination configuration.
 *
 * This function loads the destination configuration data from the configuration file and updates it with values from the source
 * configuration, as long as the source values are not marked as invalid. It selectively updates fields in the destination 
 * configuration based on the validity of the corresponding fields in the source configuration.
 *
 * @param src Pointer to the source configuration structure (`g_config_t`) containing the new values to be copied.
 * @param dst Pointer to the destination configuration structure (`g_config_t`) where the updated values will be stored.
 *
 * @return `true` if the destination configuration was successfully loaded and updated with valid values from the source; 
 *         `false` if there was an error loading the destination configuration or if invalid data was encountered.
 *
 * @details This function performs the following steps:
 *          - Attempts to load the existing configuration data from the configuration file into the destination configuration (`dst`).
 *          - If loading is successful, it updates the destination configuration (`dst`) with values from the source configuration (`src`),
 *            but only if the values in the source are not marked as invalid (e.g., `"INVALID"` or `-1`).
 *          - Logs appropriate messages at each step, indicating success or failure of loading and saving the configuration.
 *
 *          Example usage:
 *          @code
 *          g_config_t srcConfig;
 *          g_config_t dstConfig;
 *          if (rewriteConfig(&srcConfig, &dstConfig)) {
 *              Serial.println("Configuration successfully rewritten.");
 *          } else {
 *              Serial.println("Failed to rewrite configuration.");
 *          }
 *          @endcode
 */
bool rewriteConfig(g_config_t *src, g_config_t *dst);

/**
 * @brief Loads configuration data from a file on the SD card.
 *
 * This function attempts to load configuration data from a specified file on the SD card. If the file does not exist or is 
 * corrupted, it creates a new configuration file with default values. The function parses the file in JSON format and 
 * validates the required fields. If any required fields are missing or the file is unreadable, it resets the configuration 
 * to default values and saves it to the file.
 *
 * @param g_config Pointer to the configuration structure (`g_config_t`) where the loaded configuration will be stored.
 * @param filepath The path to the configuration file to load. If the file path matches the predefined `CONFIG_FILE`, 
 *                 it attempts to load from the default configuration file.
 *
 * @return `true` if the configuration data was successfully loaded and parsed; 
 *         `false` if there was an error during file loading, parsing, or validation, or if the file is invalid.
 *
 * @details This function performs the following steps:
 *          1. It checks if the file exists on the SD card. If not, it creates a new file with default values.
 *          2. If the file exists, it attempts to open and read it. If the file cannot be opened, it removes the file and resets the configuration to default values.
 *          3. It parses the JSON content from the file and checks for the presence of required fields. If any required fields are missing or invalid, it removes the file and resets the configuration.
 *          4. The valid configuration values are then assigned to the `g_config` structure.
 *          5. Finally, it logs the result of the loading process.
 *
 * @note The function expects a JSON structure with the following fields:
 *       - wifi: ssid, password, ip, gateway, subnet
 *       - mqtt: mqtt_tls, mqtt_broker, mqtt_port, mqtt_id, mqtt_topic, mqtt_username, mqtt_password, mqtt_cert
 *       - alarm: alarm_countdown, alarm_countdown_e, alarm_threshold_w, alarm_threshold_e, alarm_telephone
 *
 *          Example usage:
 *          @code
 *          g_config_t myConfig;
 *          if (loadConfig(&myConfig, CONFIG_FILE)) {
 *              Serial.println("Configuration successfully loaded.");
 *          } else {
 *              Serial.println("Failed to load configuration.");
 *          }
 *          @endcode
 */
bool loadConfig(g_config_t * g_config, const char* filepath);

#endif