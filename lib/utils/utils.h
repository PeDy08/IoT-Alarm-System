/**
 * @file utils.h
 * @brief Contains utility functions for IoT Alarm main application.
 * 
 * Contains utility functions for IoT Alarm main application.
 */

#ifndef UTILS_H_DEFINITION
#define UTILS_H_DEFINITION

#include <Arduino.h>
#include <LittleFS.h>

#define LOG_FILE_NAME "logfile.txt"
#define LOG_FILE_OLD_NAME "old_logfile.txt"
#define LOG_FILE_PATH "/"
#define LOG_FILE String(String(LOG_FILE_PATH)+String(LOG_FILE_NAME)).c_str()
#define LOG_FILE_OLD String(String(LOG_FILE_PATH)+String(LOG_FILE_OLD_NAME)).c_str()
#define LOG_FILE_MAX_SIZE 10 * 1024 // maximal size of log file 10 kB

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
bool esplogI(const char* format, ...);

/**
 * @brief Special logging function that besides writing to serial monitor logs the same data to file.
 * @returns True if logging was succesfull, otherwise false.
 * 
 * Prints to serial monitor and also write logs to file. Use for logging the warnings.
 */
bool esplogW(const char* format, ...);

/**
 * @brief Special logging function that besides writing to serial monitor logs the same data to file.
 * @returns True if logging was succesfull, otherwise false.
 * 
 * Prints to serial monitor and also write logs to file. Use for logging the errors.
 */
bool esplogE(const char* format, ...);

#endif