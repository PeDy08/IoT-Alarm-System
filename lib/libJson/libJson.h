/**
 * @file libJson.h
 * @brief Contains functions and definitions for managing JSON files in ESP32 LittleFS.
 * 
 * Contains functions and definitions for managing JSON files in ESP32 LittleFS.
 */

#ifndef LIBJSON_H_DEFINITION
#define LIBJSON_H_DEFINITION

#include <LittleFS.h>
#include <ArduinoJson.h>

#include "utils.h"
#include "mainAppDefinitions.h"

#define CONFIG_FILE_NAME "config.json"
#define CONFIG_FILE_UPLOAD_NAME "config.json"
#define CONFIG_FILE_PATH "/"
#define CONFIG_FILE String(String(CONFIG_FILE_PATH)+String(CONFIG_FILE_NAME)).c_str()
#define CONFIG_UPLOAD_FILE String(String(CONFIG_FILE_PATH)+String(CONFIG_FILE_UPLOAD_NAME)).c_str()

/**
 * @brief Sets default configuration values.
 * 
 * This function sets the default values for configuration values in the `g_config_t` structure.
 * It is automatically called when no configuration file is found or when the file is corrupted.
 */
void setDefaultConfig(g_config_t *g_config);

/**
 * @brief Saves the current configuration to a file.
 * 
 * This function creates or opens the file `CONFIG_FILE` and writes the current configuration values
 * to it in JSON format. If the file cannot be opened or written to, 
 * it prints an error message.
 */
bool saveConfig(g_config_t *g_config);

/**
 * @brief Saves the configuration from a `CONFIG_UPLOAD_FILE` file.
 * 
 * This function opens the file `CONFIG_UPLOAD_FILE` and writes the configuration values from it
 * to `CONFIG_FILE` in JSON format. If the file cannot be opened or written to, 
 * it prints an error message.
 */
bool saveConfigFromJSON(g_config_t *g_config);

/**
 * @brief Loads the configuration from a given file.
 * @return Bool value defining if opration was successfull or unexpected error happened.
 * 
 * This function attempts to open and read the `filepath` file from the LittleFS file system.
 * If the file does not exist, is corrupted, or cannot be read, it creates a new file with 
 * default values.
 * 
 * This function should be always called at the application setup! It initialises the configuration variables!
 */
bool loadConfig(g_config_t * g_config, const char* filepath);

#endif