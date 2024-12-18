/**
 * @file utils.h
 * @brief Contains utility functions for IoT Alarm main application.
 * 
 * Contains utility functions for IoT Alarm main application.
 */

#ifndef UTILS_H_DEFINITION
#define UTILS_H_DEFINITION

#include <Arduino.h>
#include <SD.h>

#define LOG_FILE_NAME "logfile.txt"
#define LOG_FILE_OLD_NAME "old_logfile.txt"
#define LOG_FILE_PATH "/log/"
#define LOG_FILE String(String(LOG_FILE_PATH)+String(LOG_FILE_NAME)).c_str()
#define LOG_FILE_OLD String(String(LOG_FILE_PATH)+String(LOG_FILE_OLD_NAME)).c_str()
#define LOG_FILE_MAX_SIZE 10 * 1024 // maximal size of log file 10 kB

extern const char *TAG_SETUP;
extern const char *TAG_RTOS_MAIN;
extern const char *TAG_RTOS_ALARM;
extern const char *TAG_RTOS_KEYPAD;
extern const char *TAG_RTOS_WIFI;
extern const char *TAG_RTOS_DATETIME;
extern const char *TAG_RTOS_RFID;
extern const char *TAG_RTOS_GSM;
extern const char *TAG_RTOS_ZIGBEE;
extern const char *TAG_RTOS_MQTT;
extern const char *TAG_RTOS_DISPLAY;
extern const char *TAG_RTOS_PERIPHERALS;

extern const char *TAG_SERVER;

extern const char *TAG_LIB_AUTH;
extern const char *TAG_LIB_DISPLAY;
extern const char *TAG_LIB_GSM;
extern const char *TAG_LIB_JSON;
extern const char *TAG_LIB_KEYPAD;
extern const char *TAG_LIB_MQTT;
extern const char *TAG_LIB_WIFI;
extern const char *TAG_LIB_ZIGBEE;
extern const char *TAG_LIB_UTILS;
extern const char *TAG_LIB_PERIPHERALS;

/**
 * @brief Crops the given selection value to ensure it is within the valid range.
 * 
 * This function ensures that the selection value falls within the range of 0 to `selection_max - 1`. 
 * If the current selection is outside this range, it adjusts the value accordingly.
 * 
 * @param selection Pointer to the current selection value. The value will be modified if it's out of bounds.
 * @param selection_max The maximum valid selection value (exclusive). The selection will be cropped to the range [0, selection_max-1].
 * 
 * @return void
 * 
 * @details
 * The function performs the following checks:
 * - If `selection_max` is 0, the selection is set to 0.
 * - If the selection is greater than or equal to `selection_max`, it is set to `selection_max - 1`.
 * - If the selection is less than 0, it is set to 0.
 * 
 * This function is useful for limiting a selection index to a valid range, such as when navigating through items in a list or menu.
 */
void cropSelection(int * selection, int selection_max);

/**
 * @brief Cycles the given selection value within the valid range.
 * 
 * This function adjusts the selection value to ensure it stays within the range [0, `selection_max - 1`]. 
 * If the selection exceeds the bounds, it wraps around to the opposite end of the range.
 * 
 * @param selection Pointer to the current selection value. The value will be modified to stay within the range.
 * @param selection_max The maximum valid selection value (exclusive). The selection will be cycled within the range [0, selection_max-1].
 * 
 * @return void
 * 
 * @details
 * The function performs the following checks:
 * - If the selection is greater than or equal to `selection_max`, it is reset to 0.
 * - If the selection is less than 0, it is set to `selection_max - 1` (wraps around to the end).
 * 
 * This function is useful when implementing features like cyclic navigation in menus or item lists, where the selection should loop back around after reaching the end.
 */
void cycleSelection(int * selection, int selection_max);

/**
 * @brief Reboots the ESP32 device.
 * 
 * This function logs a message indicating that the device is rebooting, waits for 2 seconds to allow for any necessary processes, 
 * and then restarts the ESP32 device using the `ESP.restart()` function.
 * 
 * @param void
 * 
 * @return void
 * 
 * @details
 * The function performs the following actions:
 * - Logs a reboot message using the `esplogW` function.
 * - Waits for 2 seconds using the `delay` function to allow the log message to be processed.
 * - Calls `ESP.restart()` to trigger a soft reboot of the ESP32.
 * 
 * This function is typically used for restarting the ESP32 after configuration changes or critical errors, ensuring the system restarts cleanly.
 */
void rebootESP();

/**
 * @brief Checks the size of the log file and manages it if the size exceeds the maximum allowed.
 * 
 * This function opens the log file, checks its size, and if it exceeds the maximum allowed size, it renames the current log file 
 * to keep a backup and creates a new log file for continued logging.
 * 
 * @param void
 * 
 * @return bool
 * - `true` if the log file size check and management were successfully performed.
 * - `false` if the log file could not be opened.
 * 
 * @details
 * The function performs the following actions:
 * - Attempts to open the log file specified by `LOG_FILE`.
 * - If the file cannot be opened, it logs an error message and returns `false`.
 * - If the file is successfully opened, it checks the file size.
 * - If the size of the file exceeds or equals the maximum size (`LOG_FILE_MAX_SIZE`), it renames the log file to `LOG_FILE_OLD` and logs that a new log file has been created.
 * - After checking or renaming, it closes the file and returns `true`.
 * 
 * This function is useful for managing log file size and ensuring that logging continues without exceeding disk space.
 */
bool checkLogFileSize();

/**
 * @brief Logs an informational message to both the serial monitor and a log file.
 * 
 * This function formats an informational log message, including a timestamp, and outputs it to the serial monitor. 
 * It also appends the log message to a file on the SD card. If the log file cannot be opened, an error message is printed to the serial monitor.
 * 
 * @param tag Optional string representing the tag associated with the log message. If provided, it will be printed before the message.
 * @param fx Optional string representing the function name where the log is being generated. If provided, it will be printed in parentheses after the message.
 * @param format A format string for the log message, similar to `printf` format. Additional parameters can be passed to format the message.
 * 
 * @return bool
 * - `true` if the log message was successfully written to both the serial monitor and the log file.
 * - `false` if there was an error formatting the message or opening the log file.
 * 
 * @details
 * The function performs the following actions:
 * - Retrieves the current timestamp in milliseconds using `millis()`.
 * - Uses `vasprintf` to format the log message based on the provided `format` string and additional arguments.
 * - Prints the log message to the serial monitor with a color-coded format indicating it is an informational log (green).
 * - Attempts to open the log file (`LOG_FILE`) on the SD card in append mode. If the file cannot be opened, an error message is printed to the serial monitor, and the function returns `false`.
 * - Appends the formatted log message to the log file, including the timestamp, tag (if provided), and function name (if provided).
 * - Frees the dynamically allocated memory for the formatted message and closes the log file.
 * 
 * This function is useful for generating logs that include timestamps, tags, and function names, helping with debugging and monitoring system behavior.
 */
bool esplogI(const char* tag, const char* fx, const char* format, ...);

/**
 * @brief Logs a warning message to both the serial monitor and a log file.
 * 
 * This function formats a warning log message, including a timestamp, and outputs it to the serial monitor. 
 * It also appends the log message to a file on the SD card. If the log file cannot be opened, an error message is printed to the serial monitor.
 * 
 * @param tag Optional string representing the tag associated with the log message. If provided, it will be printed before the message.
 * @param fx Optional string representing the function name where the log is being generated. If provided, it will be printed in parentheses after the message.
 * @param format A format string for the log message, similar to `printf` format. Additional parameters can be passed to format the message.
 * 
 * @return bool
 * - `true` if the log message was successfully written to both the serial monitor and the log file.
 * - `false` if there was an error formatting the message or opening the log file.
 * 
 * @details
 * The function performs the following actions:
 * - Retrieves the current timestamp in milliseconds using `millis()`.
 * - Uses `vasprintf` to format the log message based on the provided `format` string and additional arguments.
 * - Prints the log message to the serial monitor with a color-coded format indicating it is a warning log (yellow).
 * - Attempts to open the log file (`LOG_FILE`) on the SD card in append mode. If the file cannot be opened, an error message is printed to the serial monitor, and the function returns `false`.
 * - Appends the formatted log message to the log file, including the timestamp, tag (if provided), and function name (if provided).
 * - Frees the dynamically allocated memory for the formatted message and closes the log file.
 * 
 * This function is useful for generating logs that highlight warnings or potential issues, including relevant context like tags and function names.
 */
bool esplogW(const char* tag, const char* fx, const char* format, ...);

/**
 * @brief Logs an error message to both the serial monitor and a log file, and reboots the system.
 * 
 * This function formats an error log message, including a timestamp, and outputs it to the serial monitor in red, indicating it is an error message. 
 * It also appends the log message to a file on the SD card. If the log file cannot be opened, an error message is printed to the serial monitor. 
 * After logging the error, the system is rebooted using the `rebootESP` function.
 * 
 * @param tag Optional string representing the tag associated with the log message. If provided, it will be printed before the message.
 * @param fx Optional string representing the function name where the log is being generated. If provided, it will be printed in parentheses after the message.
 * @param format A format string for the log message, similar to `printf` format. Additional parameters can be passed to format the message.
 * 
 * @return bool
 * - `true` if the log message was successfully written to both the serial monitor and the log file.
 * - `false` if there was an error formatting the message or opening the log file.
 * 
 * @details
 * The function performs the following actions:
 * - Retrieves the current timestamp in milliseconds using `millis()`.
 * - Uses `vasprintf` to format the log message based on the provided `format` string and additional arguments.
 * - Prints the log message to the serial monitor with a color-coded format indicating it is an error log (red).
 * - Attempts to open the log file (`LOG_FILE`) on the SD card in append mode. If the file cannot be opened, an error message is printed to the serial monitor, and the function returns `false`.
 * - Appends the formatted log message to the log file, including the timestamp, tag (if provided), and function name (if provided).
 * - Frees the dynamically allocated memory for the formatted message and closes the log file.
 * - Calls the `rebootESP` function to reboot the system after logging the error.
 * 
 * This function is useful for logging critical errors that require the system to reboot in order to recover.
 */
bool esplogE(const char* tag, const char* fx, const char* format, ...);

#endif