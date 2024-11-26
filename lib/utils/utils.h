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

void cropSelection(int * selection, int selection_max);
void cycleSelection(int * selection, int selection_max);

/**
 * @brief Reboots ESP32.
 * 
 * Prints to serial information about reboot. Waits 2 seconds and then reboots.
 */
void rebootESP();

/**
 * @brief Special logging function that besides writing to serial monitor logs the same data to file.
 * @returns True if logging was succesfull, otherwise false.
 * 
 * Prints to serial monitor and also write logs to file. Use for logging the informations.
 */
bool esplogI(const char* tag, const char* fx, const char* format, ...);

/**
 * @brief Special logging function that besides writing to serial monitor logs the same data to file.
 * @returns True if logging was succesfull, otherwise false.
 * 
 * Prints to serial monitor and also write logs to file. Use for logging the warnings.
 */
bool esplogW(const char* tag, const char* fx, const char* format, ...);

/**
 * @brief Special logging function that besides writing to serial monitor logs the same data to file.
 * @returns True if logging was succesfull, otherwise false.
 * 
 * Prints to serial monitor and also write logs to file. Use for logging the errors.
 */
bool esplogE(const char* tag, const char* fx, const char* format, ...);

#endif