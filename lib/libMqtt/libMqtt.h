/**
 * @file libMqtt.h
 * @brief Contains functions and definitions for publishing and subscribing to mqtt topics.
 * 
 * Contains functions and definitions for publishing and subscribing to mqtt topics.
 */

#ifndef LIBMQTT_H_DEFINITION
#define LIBMQTT_H_DEFINITION

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>

#include "utils.h"
#include "libZigbee.h"
#include "mainAppDefinitions.h"

#define MQTT_LOG_FILES_PATH "/mqtt"    // MQTT logs directory
#define MQTT_LOG_KEEP_MONTHS 2          // number of months to retain MQTT logs

extern WiFiClient mqttwificlient;
extern WiFiClientSecure mqttwificlientsecure;
extern PubSubClient mqtt;

extern NTPClient timeClient;

/**
 * @brief Callback function to handle incoming MQTT messages.
 *
 * This function processes incoming MQTT messages based on the topic of the message.
 * It distinguishes between "write" and "read" commands, performs the appropriate action
 * (such as writing or reading Zigbee attributes), and logs the results. The message payload
 * is unpacked and processed using `unpack_attr()` and either `zigbeeAttrWrite()` or `zigbeeAttrRead()`
 * is called depending on the command.
 *
 * @param topic The MQTT topic associated with the received message. This topic is used to determine
 *              if the message is a "write" or "read" command.
 * @param message The payload of the received MQTT message. This contains the data to be processed.
 * @param length The length of the received message.
 *
 * @return None
 *
 * @details The function processes the following types of MQTT messages:
 * - If the topic starts with the configured "write" prefix, the function will unpack the payload 
 *   into an attribute structure and call `zigbeeAttrWrite()` to write the Zigbee attribute.
 * - If the topic starts with the configured "read" prefix, the function will unpack the payload 
 *   into an attribute structure and call `zigbeeAttrRead()` to read the Zigbee attribute.
 * - If the topic does not match either of these prefixes, the function logs a warning indicating 
 *   an undefined message.
 *
 * The `unpack_attr()` function is used to unpack the MQTT payload into a `iot_alarm_attr_load_t` structure.
 * If the unpacking is successful, the corresponding Zigbee function (`zigbeeAttrWrite()` or `zigbeeAttrRead()`)
 * is called. After processing, the allocated attribute memory is freed using `destroy_attr()`.
 *
 * @note The function assumes that the global configuration (`g_config_ptr`) contains the MQTT topic prefix
 *       which is used to match the incoming topics for "write" and "read" commands.
 *
 * Example Usage:
 * @code
 * void mqtt_callback(char* topic, byte* message, unsigned int length) {
 *     // Handle MQTT message
 * }
 * @endcode
 */
void mqtt_callback(char* topic, byte* message, unsigned int length);

/**
 * @brief Publishes an MQTT message to a specified topic.
 *
 * This function attempts to publish an MQTT message to the given topic. If the message exceeds the
 * maximum allowed size, it will be split into smaller chunks and sent sequentially. The function
 * ensures that each chunk is published successfully before proceeding to the next. Once the message
 * or chunks are published, it logs the success or failure of the operation.
 *
 * @param topic The MQTT topic to which the message will be published. This is a string that identifies
 *              the destination of the message.
 * @param load The payload to be published. This is a string containing the data to be sent over MQTT.
 *
 * @return True if the message was successfully published (either as a single message or in chunks),
 *         False if there was an error in publishing.
 *
 * @details
 * The function first checks if the MQTT client is connected. If the connection is active, it proceeds
 * to publish the message:
 * - If the message size exceeds the predefined maximum size (`maxMessageSize`), the message will be split
 *   into smaller chunks and published one by one. Each chunk is sent in sequence, and if all chunks are
 *   successfully published, the function returns `true`. If any chunk fails, the function returns `false`.
 * - If the message is small enough to fit within the allowed size, it will be published as a single message.
 *
 * The function uses `mqtt.beginPublish()` to initiate the message publication, `mqtt.print()` to send
 * each chunk (if necessary), and `mqtt.endPublish()` to finalize the message sending. It logs the status
 * of the operation using `esplogI()` and `esplogW()` for success and failure messages respectively.
 * 
 * After the publishing attempt (whether successful or not), the payload message is logged using the 
 * `logMqttMessage()` function.
 *
 * @note The maximum allowed message size (`maxMessageSize`) is set to 200 bytes. If the payload exceeds
 *       this size, the message is split into chunks. This can be adjusted by changing the `maxMessageSize`
 *       value.
 * 
 * Example Usage:
 * @code
 * String topic = "home/temperature";
 * String payload = "{\"temp\": 22.5, \"humidity\": 45}";
 * if (mqtt_publish(topic, payload)) {
 *     Serial.println("Message successfully published.");
 * } else {
 *     Serial.println("Failed to publish message.");
 * }
 * @endcode
 */
bool mqtt_publish(String topic, String load);

/**
 * @brief Logs an MQTT message to a daily log file on the SD card.
 *
 * This function logs the received MQTT message to a JSON file on the SD card. The log file is organized
 * by month and day, creating directories and files as needed. If the log file for the current day exists,
 * the message is appended to the file. If not, a new file is created. The function also ensures that the
 * folder structure (year/month) is created if it doesn't exist.
 *
 * @param load The MQTT message payload to be logged. This is a string containing the message to be written
 *             to the log file in JSON format.
 *
 * @return True if the MQTT message was successfully logged to the SD card, False if any error occurs
 *         during the logging process (e.g., incorrect datetime, failed file creation, or write failure).
 *
 * @details
 * The function uses the system's datetime (retrieved from `g_vars_ptr->datetime`) to create a folder structure
 * based on the year and month (e.g., `/2024-12`). It then creates or appends to a daily log file (e.g., `2024-12-17.json`)
 * within that folder. The MQTT message is written in JSON format and is properly enclosed in a JSON array. If the file
 * already exists, the function appends the new message, ensuring that the array format remains valid by adding a comma
 * between messages. If the file doesn't exist, it creates the file and starts the JSON array.
 *
 * The log file and folder paths are created dynamically based on the current date and time. If any issue occurs during
 * folder or file creation, or if the SD card cannot be accessed, the function logs an appropriate warning and returns `false`.
 * If everything is successful, the function returns `true`.
 *
 * Example Usage:
 * @code
 * String message = "{\"topic\": \"home/temperature\", \"value\": 22.5}";
 * if (logMqttMessage(message)) {
 *     Serial.println("Message successfully logged.");
 * } else {
 *     Serial.println("Failed to log message.");
 * }
 * @endcode
 */
bool logMqttMessage(const String mqttMessage);

/**
 * @brief Cleans up old MQTT log directories from the SD card.
 *
 * This function removes log directories that are older than a specified number of months. It compares the
 * current date with the directory names, which are formatted as year-month (e.g., `2024-12`). If a directory
 * is older than the cutoff date, it is deleted from the SD card. The number of months to keep the logs is 
 * defined by the constant `MQTT_LOG_KEEP_MONTHS`.
 *
 * @return True if the old log directories were successfully cleaned, False if any error occurs during the process
 *         (e.g., the logs directory doesn't exist, incorrect datetime, or failure to remove directories).
 *
 * @details
 * The function retrieves the current date and time from `g_vars_ptr->datetime`. It then subtracts the number
 * of months defined by `MQTT_LOG_KEEP_MONTHS` from the current month to determine the cutoff date. The function
 * then iterates through the directories inside the log folder (`MQTT_LOG_FILES_PATH`), comparing each directory
 * name (formatted as `YYYY-MM`) with the cutoff date. If the directory is older than the cutoff, it is deleted
 * using `SD.rmdir()`.
 *
 * The function logs an informational message whenever an old directory is deleted, and a warning if the logs 
 * directory doesn't exist or if the datetime is invalid. If successful, a success message is logged indicating 
 * the completion of the cleanup process.
 *
 * Example Usage:
 * @code
 * if (cleanOldLogs()) {
 *     Serial.println("Old logs cleaned successfully.");
 * } else {
 *     Serial.println("Failed to clean old logs.");
 * }
 * @endcode
 */
bool cleanOldLogs();

#endif