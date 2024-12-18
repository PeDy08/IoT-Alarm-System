/**
 * @file libZigbee.h
 * @brief Contains functions and definitions for comunicating with zigbee module ESP32H2.
 * 
 * Contains functions and definitions for comunicating with zigbee module ESP32H2.
 */

#ifndef LIBZIGBEE_H_DEFINITION
#define LIBZIGBEE_H_DEFINITION

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>

#include "utils.h"
#include "mainAppDefinitions.h"

#ifdef EINK
#include "libDisplayEINK.h"
#endif

#ifdef LCD
#include "libDisplayLCD.h"
#endif

#include "string.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ZIGBEE_RX_PIN 25
#define ZIGBEE_TX_PIN 26
// #define ZIGBEE_EN_PIN 13 // <- will be at pcf8574
#define ZIGBEE_BAUDRATE 115200
#define ZIGBEE_TIMEOUT 1000

#define TXD_PIN ZIGBEE_TX_PIN
#define RXD_PIN ZIGBEE_RX_PIN
#define UART UART_NUM_2

extern HardwareSerial SerialZigbee;

extern const int RX_BUF_SIZE;
extern const int TX_BUF_SIZE;

extern uint8_t* tx_buffer;
extern uint8_t* rx_buffer;

/**
 * @brief ZCL attribute data type values
 * @anchor esp_zb_zcl_attr_type
 */
typedef enum {
    ESP_ZB_ZCL_ATTR_TYPE_NULL               = 0x00U,        /*!< Null data type */
    ESP_ZB_ZCL_ATTR_TYPE_8BIT               = 0x08U,        /*!< 8-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_16BIT              = 0x09U,        /*!< 16-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_24BIT              = 0x0aU,        /*!< 24-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_32BIT              = 0x0bU,        /*!< 32-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_40BIT              = 0x0cU,        /*!< 40-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_48BIT              = 0x0dU,        /*!< 48-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_56BIT              = 0x0eU,        /*!< 56-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_64BIT              = 0x0fU,        /*!< 64-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_BOOL               = 0x10U,        /*!< Boolean data type */
    ESP_ZB_ZCL_ATTR_TYPE_8BITMAP            = 0x18U,        /*!< 8-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_16BITMAP           = 0x19U,        /*!< 16-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_24BITMAP           = 0x1aU,        /*!< 24-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_32BITMAP           = 0x1bU,        /*!< 32-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_40BITMAP           = 0x1cU,        /*!< 40-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_48BITMAP           = 0x1dU,        /*!< 48-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_56BITMAP           = 0x1eU,        /*!< 56-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_64BITMAP           = 0x1fU,        /*!< 64-bit bitmap data type */
    ESP_ZB_ZCL_ATTR_TYPE_U8                 = 0x20U,        /*!< Unsigned 8-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_U16                = 0x21U,        /*!< Unsigned 16-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_U24                = 0x22U,        /*!< Unsigned 24-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_U32                = 0x23U,        /*!< Unsigned 32-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_U40                = 0x24U,        /*!< Unsigned 40-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_U48                = 0x25U,        /*!< Unsigned 48-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_U56                = 0x26U,        /*!< Unsigned 56-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_U64                = 0x27U,        /*!< Unsigned 64-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S8                 = 0x28U,        /*!< Signed 8-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S16                = 0x29U,        /*!< Signed 16-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S24                = 0x2aU,        /*!< Signed 24-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S32                = 0x2bU,        /*!< Signed 32-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S40                = 0x2cU,        /*!< Signed 40-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S48                = 0x2dU,        /*!< Signed 48-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S56                = 0x2eU,        /*!< Signed 56-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_S64                = 0x2fU,        /*!< Signed 64-bit value data type */
    ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM          = 0x30U,        /*!< 8-bit enumeration (U8 discrete) data type */
    ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM         = 0x31U,        /*!< 16-bit enumeration (U16 discrete) data type */
    ESP_ZB_ZCL_ATTR_TYPE_SEMI               = 0x38U,        /*!< 2 byte floating point */
    ESP_ZB_ZCL_ATTR_TYPE_SINGLE             = 0x39U,        /*!< 4 byte floating point */
    ESP_ZB_ZCL_ATTR_TYPE_DOUBLE             = 0x3aU,        /*!< 8 byte floating point */
    ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING       = 0x41U,        /*!< Octet string data type */
    ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING        = 0x42U,        /*!< Character string (array) data type */
    ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING  = 0x43U,        /*!< Long octet string */
    ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING   = 0x44U,        /*!< Long character string */
    ESP_ZB_ZCL_ATTR_TYPE_ARRAY              = 0x48U,        /*!< Array data with 8bit type, size = 2 + sum of content len */
    ESP_ZB_ZCL_ATTR_TYPE_16BIT_ARRAY        = 0x49U,        /*!< Array data with 16bit type, size = 2 + sum of content len */
    ESP_ZB_ZCL_ATTR_TYPE_32BIT_ARRAY        = 0x4aU,        /*!< Array data with 32bit type, size = 2 + sum of content len */
    ESP_ZB_ZCL_ATTR_TYPE_STRUCTURE          = 0x4cU,        /*!< Structure data type 2 + sum of content len */
    ESP_ZB_ZCL_ATTR_TYPE_SET                = 0x50U,        /*!< Collection:set, size = sum of len of content */
    ESP_ZB_ZCL_ATTR_TYPE_BAG                = 0x51U,        /*!< Collection:bag, size = sum of len of content */
    ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY        = 0xe0U,        /*!< Time of day, 4 bytes */
    ESP_ZB_ZCL_ATTR_TYPE_DATE               = 0xe1U,        /*!< Date, 4 bytes */
    ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME           = 0xe2U,        /*!< UTC Time, 4 bytes */
    ESP_ZB_ZCL_ATTR_TYPE_CLUSTER_ID         = 0xe8U,        /*!< Cluster ID, 2 bytes */
    ESP_ZB_ZCL_ATTR_TYPE_ATTRIBUTE_ID       = 0xe9U,        /*!< Attribute ID, 2 bytes */
    ESP_ZB_ZCL_ATTR_TYPE_BACNET_OID         = 0xeaU,        /*!< BACnet OID, 4 bytes */
    ESP_ZB_ZCL_ATTR_TYPE_IEEE_ADDR          = 0xf0U,        /*!< IEEE address (U64) type */
    ESP_ZB_ZCL_ATTR_TYPE_128_BIT_KEY        = 0xf1U,        /*!< 128-bit security key */
    ESP_ZB_ZCL_ATTR_TYPE_INVALID            = 0xffU,        /*!< Invalid data type */
} esp_zb_zcl_attr_type_t;

typedef enum {
    IOT_ALARM_MSGDIR_COMMAND                = 0x00U,
    IOT_ALARM_MSGDIR_COMMAND_ACK            = 0x01U,
    IOT_ALARM_MSGDIR_NOTIFICATION           = 0x02U,
    IOT_ALARM_MSGDIR_NOTIFICATION_ACK       = 0x03U,
    IOT_ALARM_MSGDIR_MAX                    = 0x04U,
} message_direction_t;

typedef enum {
    IOT_ALARM_MSGSTATUS_SUCCESS             = 0x00U,
    IOT_ALARM_MSGSTATUS_ERROR               = 0x01U,
    IOT_ALARM_MSGSTATUS_MAX                 = 0x02U,
} message_status_t;

typedef enum {
    IOT_ALARM_MSGTYPE_CTL_EMPTY             = 0x00U,        // empty
    IOT_ALARM_MSGTYPE_ECHO                  = 0x01U,        // echo
    IOT_ALARM_MSGTYPE_CTL_RESTART           = 0x02U,        // command (reset NCP)
    IOT_ALARM_MSGTYPE_CTL_FACTORY           = 0x03U,        // command (factory-reset NCP)

    IOT_ALARM_MSGTYPE_ZB_DEV_UNLOCK         = 0x04U,        // command (close network joining status)
    IOT_ALARM_MSGTYPE_ZB_DEV_LOCK           = 0x05U,        // command (open network joining  status)
    IOT_ALARM_MSGTYPE_ZB_DEV_CLEAR          = 0x06U,        // command (clear all connected devices in the network)
    IOT_ALARM_MSGTYPE_ZB_DEV_NEW            = 0x07U,        // notification (new device joined)
    IOT_ALARM_MSGTYPE_ZB_DEV_LEAVE          = 0x08U,        // notification (device leaved)
    IOT_ALARM_MSGTYPE_DEV_COUNT             = 0x09U,        // command (get current count of zigbee devices)

    IOT_ALARM_MSGTYPE_ZB_DATA_READ          = 0x0aU,        // command (read attribute on zigbee device)
    IOT_ALARM_MSGTYPE_ZB_DATA_WRITE         = 0x0bU,        // command (set attribute on zigbee device)
    IOT_ALARM_MSGTYPE_ZB_DATA_REPORT        = 0x0cU,        // notification (attribute report)

    IOT_ALARM_MSGTYPE_MAX                   = 0x0dU,
} message_type_t;

typedef struct {
    message_direction_t dir;            // message direction (command / notification / acknowledgement)
    message_status_t st;                // message status (success / error)
    message_type_t id;                  // message type ID (notification / command type ID)
    uint32_t length;                    // load length
    char* load;                         // message load (integer, string, attribute data, etc...)
} iot_alarm_message_t;

typedef uint8_t esp_zb_64bit_addr_t[8];
typedef esp_zb_64bit_addr_t esp_zb_ieee_addr_t;

typedef struct {
    uint16_t cluster_id;                // ZCL CLuster ID
    uint8_t endpoint_id;                // ZCL Endpoint ID
} user_ctx_t;

typedef struct {
    uint16_t cluster_id;                // ZCL Cluster ID
} cluster_info_t;

typedef struct {
    uint8_t endpoint_id;                // Endpoint ID
    uint16_t app_profile_id;            // Application Profile ID
    uint16_t app_device_id;             // Application Device ID
    uint8_t app_device_version;         // Application Device Version
    uint8_t input_cluster_count;        // Number of input clusters
    uint8_t output_cluster_count;       // Number of output clusters
    cluster_info_t *input_clusters;     // Pointer to dynamically allocated input clusters
    cluster_info_t *output_clusters;    // Pointer to dynamically allocated output clusters
} endpoint_info_t;

typedef struct {
    bool usefull;                       // flag, if the device is usefull
    char manuf[50];                     // manufacturer name
    char name[50];                      // device model name
    char type[50];                      // device type
    uint32_t type_id;                   // device type ID
    esp_zb_ieee_addr_t ieee_addr;       // IEEE address of the device
    uint16_t short_addr;                // short address of the device
    uint8_t endpoint_count;             // Number of active endpoints
    endpoint_info_t *endpoints;         // Pointer to dynamically allocated endpoints
} device_info_t;

typedef struct {
    char manuf[50];                     // manufacturer name
    char name[50];                      // device model name
    char type[50];                      // device type
    uint32_t type_id;                   // device type ID
    esp_zb_ieee_addr_t ieee_addr;       // IEEE address of the device
    uint16_t short_addr;                // short address of the device
    uint8_t device_id;                  // zigbee network device ID
    uint8_t endpoint_id;                // ZCL Endpoint ID
    uint16_t cluster_id;                // ZCL Cluster ID
    uint16_t attr_id;                   // ZCL Attribute ID
    esp_zb_zcl_attr_type_t value_type;  // value type ID (int / uint, 8 / 16 / 32 bit)
    uint32_t value;                     // value data
} iot_alarm_attr_load_t;

/* typedef struct {
    uint8_t device_id;
    uint8_t devices_len;
    device_info_t device;
} iot_alarm_dev_load_t; */

/**
 * @brief Updates the serial communication between the ESP32's primary serial interface and the Zigbee serial interface.
 *
 * This function facilitates bidirectional data transfer between the main serial interface (`Serial`) and a secondary Zigbee serial interface (`SerialZigbee`).
 * It reads incoming data from `Serial` (the primary serial) and writes it to `SerialZigbee` (the Zigbee serial interface), and also reads incoming data from `SerialZigbee` and writes it to `Serial` for monitoring.
 * This allows data received from the Zigbee network to be visible via the main serial monitor, and data sent to the main serial interface to be transmitted over the Zigbee network.
 *
 * @return None.
 *
 * @details
 * The function performs the following tasks:
 * 1. It continuously checks if there is data available in the main serial input buffer (`Serial.available()`).
 *    - If data is available, it is read from `Serial` and written to the Zigbee serial interface (`SerialZigbee`).
 * 2. It then checks if there is any data available in the Zigbee serial interface (`SerialZigbee.available()`).
 *    - If data is available, it is read from `SerialZigbee` and written to the main serial output (`Serial`).
 * 
 * This loop ensures that the ESP32 can relay messages both from and to the Zigbee network while interacting with the user via the main serial port.
 * This function could be useful in situations where a Zigbee network device is being controlled or monitored using a serial interface.
 * 
 * @code
 * // Example of calling updateSerialZigbee
 * updateSerialZigbee();
 * @endcode
 */
void updateSerialZigbee();

/**
 * @brief Initializes the Zigbee module by configuring the UART interface and verifying communication.
 *
 * This function sets up the UART interface for the Zigbee module, configures its baud rate, and verifies
 * successful communication by sending an echo command and waiting for the correct acknowledgment.
 *
 * @return `true` if the Zigbee module initializes successfully; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Configures the UART interface with `SerialZigbee.begin()`.
 *  2. Sets a timeout for UART communication using `SerialZigbee.setTimeout()`.
 *  3. Toggles the enable pin (`ZIGBEE_EN_PIN`) if defined, to reset the Zigbee module.
 *  4. Sends an echo command using `waitForCorrectResponseZigbee()` to ensure the module is responsive.
 *  5. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note The function logs information and warnings during the initialization process.
 *
 * @warning If the Zigbee module is not found or communication fails, the function returns `false`.
 *
 * Example Usage:
 * @code
 * if (!initSerialZigbee()) {
 *     Serial.println("Zigbee module initialization failed!");
 * } else {
 *     Serial.println("Zigbee module initialized successfully!");
 * }
 * @endcode
 */
bool initSerialZigbee();

/**
 * @brief Sends a reset command to the Zigbee module and verifies the response.
 *
 * This function sends a reset (restart) command to the Zigbee module using the `IOT_ALARM_MSGTYPE_CTL_RESTART` 
 * message type. It waits for an acknowledgment from the module to confirm the reset was successful.
 *
 * @return `true` if the reset command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Creates a command message to restart the Zigbee module.
 *  2. Sends the message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  3. Logs an informational message if the reset command is sent successfully.
 *  4. Logs a warning if the reset command fails.
 *  5. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note The function relies on `waitForCorrectResponseZigbee()` to handle communication with the Zigbee module.
 *
 * @warning If the communication fails or the acknowledgment is not received, the function returns `false`.
 *
 * Example Usage:
 * @code
 * if (!zigbeeReset()) {
 *     Serial.println("Failed to reset Zigbee module!");
 * } else {
 *     Serial.println("Zigbee module reset successfully!");
 * }
 * @endcode
 */

/**
 * @brief Sends a reset command to the Zigbee module and verifies the response.
 *
 * This function sends a reset (restart) command to the Zigbee module using the `IOT_ALARM_MSGTYPE_CTL_RESTART` 
 * message type. It waits for an acknowledgment from the module to confirm the reset was successful.
 *
 * @return `true` if the reset command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Creates a command message to restart the Zigbee module.
 *  2. Sends the message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  3. Logs an informational message if the reset command is sent successfully.
 *  4. Logs a warning if the reset command fails.
 *  5. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note The function relies on `waitForCorrectResponseZigbee()` to handle communication with the Zigbee module.
 *
 * @warning If the communication fails or the acknowledgment is not received, the function returns `false`.
 *
 * Example Usage:
 * @code
 * if (!zigbeeReset()) {
 *     Serial.println("Failed to reset Zigbee module!");
 * } else {
 *     Serial.println("Zigbee module reset successfully!");
 * }
 * @endcode
 */
bool zigbeeReset();

/**
 * @brief Sends a factory reset command to the Zigbee module and verifies the response.
 *
 * This function sends a factory reset command to the Zigbee module using the `IOT_ALARM_MSGTYPE_CTL_FACTORY`
 * message type. It waits for an acknowledgment from the module to confirm that the factory reset was successful.
 *
 * @return `true` if the factory reset command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Creates a command message to initiate a factory reset of the Zigbee module.
 *  2. Sends the message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  3. Logs an informational message if the factory reset command is sent successfully.
 *  4. Logs a warning if the factory reset command fails.
 *  5. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note The function relies on `waitForCorrectResponseZigbee()` to handle communication with the Zigbee module.
 *
 * @warning Performing a factory reset will clear all configurations and settings in the Zigbee module.
 *
 * Example Usage:
 * @code
 * if (!zigbeeFactory()) {
 *     Serial.println("Failed to factory reset Zigbee module!");
 * } else {
 *     Serial.println("Zigbee module factory reset successfully!");
 * }
 * @endcode
 */
bool zigbeeFactory();

/**
 * @brief Sends a device count request command to the Zigbee module and verifies the response.
 *
 * This function sends a device count command to the Zigbee module using the `IOT_ALARM_MSGTYPE_DEV_COUNT`
 * message type. It waits for an acknowledgment from the module to confirm that the device count request
 * was received successfully.
 *
 * @return `true` if the device count command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Creates a command message to request the number of connected devices from the Zigbee module.
 *  2. Sends the message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  3. Logs an informational message if the device count command is sent successfully.
 *  4. Logs a warning if the device count command fails.
 *  5. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note The function relies on `waitForCorrectResponseZigbee()` to handle communication with the Zigbee module.
 *
 * @warning Ensure that the Zigbee module is properly initialized before calling this function.
 *
 * Example Usage:
 * @code
 * if (!zigbeeCount()) {
 *     Serial.println("Failed to send device count request to Zigbee module!");
 * } else {
 *     Serial.println("Device count request sent successfully!");
 * }
 * @endcode
 */
bool zigbeeCount();

/**
 * @brief Opens the Zigbee network for a specified duration to allow new devices to join.
 *
 * This function sends a command to the Zigbee module to unlock the Zigbee network, enabling
 * new devices to join the network for a specified duration.
 *
 * @param duration The duration (in seconds) for which the Zigbee network should remain open for joining.
 *
 * @return `true` if the open command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Converts the `duration` to a string and loads it into the message payload.
 *  2. Creates a command message with the `IOT_ALARM_MSGTYPE_ZB_DEV_UNLOCK` type.
 *  3. Sends the message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  4. Logs an informational message if the command is sent successfully.
 *  5. Logs a warning if the command fails to send or no acknowledgment is received.
 *  6. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note Ensure the Zigbee module is initialized before calling this function.
 *
 * @warning If the Zigbee network remains open for too long, it may pose security risks.
 *
 * Example Usage:
 * @code
 * if (!zigbeeOpen(60)) {
 *     Serial.println("Failed to open Zigbee network for joining!");
 * } else {
 *     Serial.println("Zigbee network is open for 60 seconds.");
 * }
 * @endcode
 */
bool zigbeeOpen(uint8_t duration = 180);

/**
 * @brief Closes the Zigbee network to prevent new devices from joining.
 *
 * This function sends a command to the Zigbee module to lock the Zigbee network, preventing
 * any new devices from joining the network.
 *
 * @return `true` if the close command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Creates a command message with the `IOT_ALARM_MSGTYPE_ZB_DEV_LOCK` type to lock the network.
 *  2. Sends the message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  3. Logs an informational message if the command is sent successfully.
 *  4. Logs a warning if the command fails to send or no acknowledgment is received.
 *  5. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note Ensure the Zigbee module is initialized before calling this function.
 *
 * Example Usage:
 * @code
 * if (!zigbeeClose()) {
 *     Serial.println("Failed to close Zigbee network!");
 * } else {
 *     Serial.println("Zigbee network is now closed to new devices.");
 * }
 * @endcode
 */
bool zigbeeClose();

/**
 * @brief Clears the Zigbee network by removing all devices.
 *
 * This function sends a command to the Zigbee module to clear the Zigbee network, effectively removing
 * all connected devices. This may be useful for reinitializing or resetting the Zigbee network.
 *
 * @return `true` if the clear command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Creates a command message with the `IOT_ALARM_MSGTYPE_ZB_DEV_CLEAR` type to clear the network.
 *  2. Sends the message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  3. Logs an informational message if the command is sent successfully.
 *  4. Displays a notification (`NOTIFICATION_ZIGBEE_NET_CLEAR`) after successfully sending the clear command.
 *  5. Logs a warning if the command fails to send or no acknowledgment is received.
 *  6. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note Ensure the Zigbee module is initialized before calling this function.
 *
 * Example Usage:
 * @code
 * if (!zigbeeClear()) {
 *     Serial.println("Failed to clear Zigbee network!");
 * } else {
 *     Serial.println("Zigbee network cleared successfully.");
 * }
 * @endcode
 */
bool zigbeeClear();

/**
 * @brief Reads an attribute from a Zigbee device.
 *
 * This function sends a command to the Zigbee module to read a specified attribute from a Zigbee device.
 * It serializes the attribute data, sends the command to the Zigbee module, and waits for a response. 
 * Upon successful communication, it returns `true`, otherwise, it returns `false`.
 *
 * @param attr A pointer to an `iot_alarm_attr_load_t` structure that contains the attribute to be read.
 * 
 * @return `true` if the read command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Serializes the attribute data provided in the `attr` parameter into a buffer (`serialized_load`).
 *  2. Creates a command message with the `IOT_ALARM_MSGTYPE_ZB_DATA_READ` type to request the attribute read.
 *  3. Sends the command message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  4. Logs an informational message if the command is sent successfully.
 *  5. Logs a warning if the command fails to send or no acknowledgment is received.
 *  6. Cleans up dynamically allocated messages using `destroy_message()`.
 *
 * @note Ensure that the Zigbee module is initialized and that the attribute to be read is properly specified in the `attr` structure.
 *
 * Example Usage:
 * @code
 * iot_alarm_attr_load_t my_attr = { ... }; // Initialize attribute
 * if (!zigbeeAttrRead(&my_attr)) {
 *     Serial.println("Failed to read attribute from Zigbee device.");
 * } else {
 *     Serial.println("Attribute read successfully.");
 * }
 * @endcode
 */
bool zigbeeAttrRead(iot_alarm_attr_load_t * attr);

/**
 * @brief Writes an attribute to a Zigbee device.
 *
 * This function sends a command to the Zigbee module to write a specified attribute to a Zigbee device.
 * It serializes the attribute data, sends the write command to the Zigbee module, and waits for a response.
 * Upon successful communication, it returns `true`, otherwise, it returns `false`.
 *
 * @param attr A pointer to an `iot_alarm_attr_load_t` structure that contains the attribute to be written.
 * 
 * @return `true` if the write command was sent successfully and the correct acknowledgment was received; otherwise, `false`.
 *
 * @details The function performs the following steps:
 *  1. Serializes the attribute data provided in the `attr` parameter into a buffer (`serialized_load`).
 *  2. Creates a command message with the `IOT_ALARM_MSGTYPE_ZB_DATA_WRITE` type to request the attribute write.
 *  3. Sends the command message and waits for the correct acknowledgment within a 10-second timeout using `waitForCorrectResponseZigbee()`.
 *  4. Logs an informational message if the command is sent successfully.
 *  5. Logs a warning if the command fails to send or no acknowledgment is received.
 *  6. Cleans up dynamically allocated messages and attributes using `destroy_message()` and `destroy_attr()`.
 *
 * @note Ensure that the Zigbee module is initialized and that the attribute to be written is properly specified in the `attr` structure.
 *
 * Example Usage:
 * @code
 * iot_alarm_attr_load_t my_attr = { ... }; // Initialize attribute to write
 * if (!zigbeeAttrWrite(&my_attr)) {
 *     Serial.println("Failed to write attribute to Zigbee device.");
 * } else {
 *     Serial.println("Attribute written successfully.");
 * }
 * @endcode
 */
bool zigbeeAttrWrite(iot_alarm_attr_load_t * attr);

// TODO
bool zigbeeAttrReadWriteHandler(iot_alarm_attr_load_t * attr);

/**
 * @brief Handles incoming Zigbee attribute reports and triggers actions based on attribute values.
 *
 * This function processes incoming attribute reports from Zigbee devices, such as alarms, occupancy sensors,
 * fire sensors, and water leakage sensors. It performs specific actions when certain attribute IDs and values
 * are reported, such as triggering alarm events or updating the state of the system.
 * 
 * @param attr A pointer to an `iot_alarm_attr_load_t` structure containing the attribute data from the Zigbee device.
 * 
 * @return `true` if the attribute report was handled successfully; otherwise, `false`.
 *
 * @details The function checks the type of attribute report based on `attr->type_id` and processes it accordingly:
 * - For IAS zone reports (type IDs: 0x0500000D, 0x05000015, 0x0500002D, 0x05000225), if the attribute ID is 0x0002 and the value is 1, it triggers an alarm event.
 * - For occupancy sensor reports (type IDs: 0x04060000, 0x04060001, 0x04060002), if the attribute ID is 0x0000 and the value is 1, it triggers an alarm event.
 * - For fire sensor reports (type IDs: 0x05000028, 0x0500002B), if the attribute ID is 0x0002 and the value is 1, it triggers a fire alarm event.
 * - For water-leakage sensor reports (type ID: 0x0500002A), if the attribute ID is 0x0002 and the value is 1, it triggers a water leakage alarm event.
 * 
 * Additionally, the function increments the alarm event counter (`alarm.alarm_events`) and updates the states for
 * fire and water alarms (`alarm.alarm_fire` and `alarm.alarm_water`), if applicable.
 *
 * The function logs warning messages when an alarm event is triggered and displays a notification.
 *
 * @note This function expects the `attr` parameter to be a valid pointer to an attribute structure containing the
 * relevant data for processing the attribute report. Ensure that the Zigbee device has been properly paired and is
 * actively sending attribute reports.
 *
 * Example Usage:
 * @code
 * iot_alarm_attr_load_t incoming_attr = { ... }; // Initialize with incoming attribute data
 * if (zigbeeAttrReportHandler(&incoming_attr)) {
 *     Serial.println("Attribute report handled successfully.");
 * } else {
 *     Serial.println("Failed to handle attribute report.");
 * }
 * @endcode
 */
bool zigbeeAttrReportHandler(iot_alarm_attr_load_t * attr);

// *********************************************************************************************************************

/**
 * @brief Serializes an `iot_alarm_message_t` structure into a byte buffer for transmission.
 *
 * This function converts a message structure (`iot_alarm_message_t`) into a byte buffer suitable for sending over a communication medium. It ensures that all relevant fields in the message are copied into the buffer, including the direction, status, ID, length, and the optional load (payload). It also ensures proper handling of the buffer's size.
 *
 * @param msg A pointer to the `iot_alarm_message_t` structure containing the message data to be serialized.
 * @param buffer A pointer to a byte array where the serialized message will be stored.
 * @param bytes A pointer to a variable where the number of bytes written to the buffer will be stored.
 *
 * @return void
 *
 * @details This function performs the following steps:
 * - It checks for `NULL` pointers for both the message and the buffer, logging warnings if either is `NULL`.
 * - It copies each field from the `iot_alarm_message_t` structure into the `buffer`, updating the buffer's offset as it progresses.
 * - If the message contains a load (payload), it copies the load into the buffer as well.
 * - It adds a null terminator (`'\0'`) at the end of the buffer to ensure proper string termination.
 * - It updates the `bytes` pointer with the total number of bytes written to the buffer.
 *
 * The following fields of the message structure are serialized:
 * - `msg->dir`: The direction of the message (e.g., incoming or outgoing).
 * - `msg->st`: The status of the message (e.g., success or error).
 * - `msg->id`: The identifier of the message.
 * - `msg->length`: The length of the message's payload (if any).
 * - `msg->load`: The payload, if the message has one. It is copied into the buffer, even if it is empty (in which case `"\0"` will be copied).
 *
 * @note Ensure that the provided buffer is large enough to hold the serialized message, including the payload.
 * The function does not check the size of the buffer before writing to it.
 *
 * Example Usage:
 * @code
 * iot_alarm_message_t my_msg = { .dir = 1, .st = 0, .id = 123, .length = 5, .load = "hello" };
 * uint8_t buffer[1024];
 * size_t bytes;
 * serialize_message(&my_msg, buffer, &bytes);
 * Serial.println("Serialized message bytes: ");
 * for (size_t i = 0; i < bytes; i++) {
 *     Serial.print(buffer[i], HEX);
 *     Serial.print(" ");
 * }
 * @endcode
 */
void serialize_message(iot_alarm_message_t *msg, uint8_t *buffer, size_t *bytes);

/**
 * @brief Deserializes a byte buffer into an `iot_alarm_message_t` structure.
 *
 * This function takes a byte buffer containing a serialized message and reconstructs it into an `iot_alarm_message_t` structure. It extracts the message's fields (direction, status, ID, length, and payload) and assigns them to the appropriate members of the structure.
 * 
 * @param msg A pointer to a pointer of `iot_alarm_message_t`, where the deserialized message will be stored.
 * @param buffer A pointer to the byte buffer containing the serialized message.
 * @param buffer_len The length of the buffer.
 *
 * @return void
 *
 * @details This function performs the following steps:
 * - It checks if any of the pointers (`msg` or `buffer`) are `NULL` and logs a warning if so.
 * - It extracts the message direction, status, ID, and payload length from the buffer, ensuring there is enough data in the buffer at each step.
 * - If the buffer length is insufficient for any field, a warning is logged, and the function exits without modifying the message.
 * - It allocates memory for the message load (payload) based on the extracted length. If allocation fails, an error is logged.
 * - The extracted fields are assigned to a new `iot_alarm_message_t` structure, which is then returned through the `msg` pointer.
 * - If the message is successfully deserialized, the memory for the load (payload) is properly managed.
 *
 * @note The function assumes that the buffer contains a complete and correctly serialized message. If the buffer length is smaller than expected, the function will log warnings and not perform the deserialization.
 * 
 * Example Usage:
 * @code
 * uint8_t buffer[1024];
 * size_t buffer_len = 256; // Example buffer length
 * iot_alarm_message_t *msg = NULL;
 * deserialize_message(&msg, buffer, buffer_len);
 * if (msg != NULL) {
 *     Serial.println("Message deserialized successfully!");
 *     // Process the message...
 *     free(msg->load);
 *     free(msg);
 * }
 * @endcode
 */
void deserialize_message(iot_alarm_message_t **msg, uint8_t *buffer, size_t buffer_len);

/**
 * @brief Creates a new `iot_alarm_message_t` structure and initializes it with the provided values.
 *
 * This function dynamically allocates memory for a new `iot_alarm_message_t` structure and its associated payload (load), initializes the structure with the provided values (direction, status, ID, length, and load), and returns a pointer to the newly created message.
 * 
 * @param dir The message direction, of type `message_direction_t`.
 * @param st The message status, of type `message_status_t`.
 * @param id The message ID, of type `message_type_t`.
 * @param length The length of the message load (payload).
 * @param load A pointer to the message load (payload) as a string.
 *
 * @return A pointer to the newly created `iot_alarm_message_t` structure, or `NULL` if memory allocation fails.
 *
 * @details This function performs the following:
 * - Allocates memory for the `iot_alarm_message_t` structure.
 * - Initializes the fields of the structure (`dir`, `st`, `id`, `length`, and `load`).
 * - Allocates memory for the load field, which contains the actual data of the message.
 * - Copies the content of the `load` string into the newly allocated memory.
 * - The `load` field is null-terminated.
 * 
 * @note If memory allocation for the message structure or its load field fails, the function logs a warning and returns `NULL`.
 * 
 * Example Usage:
 * @code
 * const char* message_data = "This is a test message";
 * iot_alarm_message_t* msg = create_message(DIRECTION_OUTGOING, STATUS_OK, MESSAGE_TYPE_ALARM, strlen(message_data), message_data);
 * if (msg != NULL) {
 *     Serial.println("Message created successfully!");
 *     // Process the message...
 *     destroy_message(&msg);
 * }
 * @endcode
 */
iot_alarm_message_t * create_message(message_direction_t dir, message_status_t st, message_type_t id, uint32_t length, const char* load);

/**
 * @brief Frees the memory allocated for an `iot_alarm_message_t` structure and its payload.
 *
 * This function frees the memory used by the `iot_alarm_message_t` structure and its associated payload (load), effectively destroying the message and preventing memory leaks.
 *
 * @param msg A pointer to the pointer of the `iot_alarm_message_t` structure to be freed.
 *
 * @return void
 *
 * @details This function performs the following:
 * - Frees the memory allocated for the message load (payload) if it is not `NULL`.
 * - Frees the memory allocated for the message structure itself.
 * - Sets the message pointer to `NULL` to avoid dangling pointers.
 * 
 * @note If the provided pointer to the message is `NULL`, the function does nothing.
 * 
 * Example Usage:
 * @code
 * iot_alarm_message_t* msg = create_message(DIRECTION_OUTGOING, STATUS_OK, MESSAGE_TYPE_ALARM, 10, "Test message");
 * if (msg != NULL) {
 *     // Use the message...
 *     destroy_message(&msg); // Clean up after usage
 * }
 * @endcode
 */
void destroy_message(iot_alarm_message_t **msg);

// *********************************************************************************************************************

/**
 * @brief Reads data from the specified UART interface into a buffer.
 *
 * This function reads data from the specified UART interface and stores the received bytes into the provided buffer.
 * It reads until the buffer is full, or there is no more data available, or the maximum read length is reached.
 * 
 * @param uart The UART port to read from (e.g., `UART_NUM_0`, `UART_NUM_1`, etc.).
 * @param rx_buffer A pointer to the buffer where the received data will be stored.
 * @param max_len The maximum number of bytes to read from the UART interface.
 *
 * @return The total number of bytes successfully read from the UART, or 0 if no data was read.
 *
 * @details This function performs the following:
 * - It first checks the available data in the UART's buffer using `uart_get_buffered_data_len`.
 * - It reads data in chunks into the provided `rx_buffer`, up to the specified `max_len` limit.
 * - If data is available, it reads it into the buffer using `uart_read_bytes`.
 * - The function continues reading data until one of the following conditions is met:
 *   1. The buffer is full (`bytes_read < max_len`).
 *   2. There is no more data available in the UART buffer (`available == 0`).
 *   3. The specified timeout period for reading (500ms) is reached.
 * 
 * @note The function uses `vTaskDelay` to introduce a brief delay before checking the UART buffer, which allows other tasks to run while waiting for data.
 * 
 * Example Usage:
 * @code
 * uint8_t buffer[100];
 * int bytes_read = read_uart(UART_NUM_1, buffer, sizeof(buffer));
 * if (bytes_read > 0) {
 *     // Process the data stored in buffer
 * } else {
 *     // Handle timeout or no data condition
 * }
 * @endcode
 */
int read_uart(uart_port_t uart, uint8_t* rx_buffer, size_t max_len);

/**
 * @brief Sends a serialized message over UART.
 *
 * This function serializes the provided message into a byte buffer and then sends it over the specified UART interface.
 * The function checks if the entire message was successfully sent and logs the outcome.
 * 
 * @param uart The UART port to send the message on (e.g., `UART_NUM_0`, `UART_NUM_1`, etc.).
 * @param tx_buffer A pointer to the buffer where the serialized message will be stored before sending.
 * @param msg A pointer to the `iot_alarm_message_t` structure containing the message to be sent.
 *
 * @return The number of bytes actually sent via UART. If successful, this should be equal to the length of the message.
 *
 * @details The function performs the following steps:
 * 1. Serializes the message using the `serialize_message` function, converting the `iot_alarm_message_t` structure into a byte array (`tx_buffer`).
 * 2. Writes the serialized message to the UART interface using `uart_write_bytes`.
 * 3. If the number of bytes written equals the length of the serialized message, the message is considered successfully sent. Otherwise, a warning message is logged indicating the failure.
 *
 * @note The function does not block the calling task indefinitely; if the message cannot be sent completely, a warning will be logged.
 * 
 * Example Usage:
 * @code
 * uint8_t tx_buffer[256];
 * iot_alarm_message_t *msg = create_message(DIRECTION_SEND, STATUS_OK, TYPE_ALARM, 10, "Test message");
 * int result = send_message(UART_NUM_1, tx_buffer, msg);
 * if (result > 0) {
 *     // Message sent successfully
 * } else {
 *     // Error in sending the message
 * }
 * @endcode
 */
int send_message(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_message_t *msg);

/**
 * @brief Receives a serialized message over UART and deserializes it.
 *
 * This function reads data from the specified UART interface, deserializes it into an `iot_alarm_message_t` structure,
 * and stores it in the provided message pointer. The function also handles buffer overflows and logs errors if any issues
 * occur during the reception or deserialization of the message.
 * 
 * @param uart The UART port to receive the message from (e.g., `UART_NUM_0`, `UART_NUM_1`, etc.).
 * @param rx_buffer A pointer to the buffer where received data will be stored.
 * @param msg A pointer to a pointer of the `iot_alarm_message_t` structure where the deserialized message will be stored.
 * @param max_len The maximum length of the buffer to store the received data.
 *
 * @return The number of bytes successfully received. If no data was received, returns 0.
 *
 * @details The function performs the following steps:
 * 1. Clears the receive buffer and attempts to read up to `max_len` bytes from the UART interface using the `read_uart` function.
 * 2. If the data exceeds the buffer size, a warning is logged, and the buffer is truncated.
 * 3. If any data was successfully received, the function proceeds to deserialize the received data into the provided message structure (`msg`).
 * 4. The function logs warnings if there are issues with the buffer or the message structure.
 *
 * @note This function does not block indefinitely. If no data is available, it returns 0 bytes read.
 *
 * Example Usage:
 * @code
 * uint8_t rx_buffer[256];
 * iot_alarm_message_t *msg = NULL;
 * int result = receive_message(UART_NUM_1, rx_buffer, &msg, sizeof(rx_buffer));
 * if (result > 0) {
 *     // Message received and deserialized
 * } else {
 *     // Error or no data received
 * }
 * @endcode
 */
int receive_message(uart_port_t uart, uint8_t* rx_buffer, iot_alarm_message_t **msg, size_t max_len);

/**
 * @brief Sends an attribute message over UART and waits for a response.
 *
 * This function serializes an attribute load, creates a message with the appropriate direction and status, and sends it
 * over UART. The function then waits for an acknowledgment (ACK) message and returns the number of bytes sent or a failure
 * code if no acknowledgment is received within the timeout.
 *
 * @param uart The UART port to send the message through (e.g., `UART_NUM_0`, `UART_NUM_1`, etc.).
 * @param tx_buffer A pointer to the buffer where the serialized message will be stored before transmission.
 * @param load A pointer to the attribute load that needs to be serialized and sent.
 * @param id The message type identifier, which determines the message format and direction.
 *
 * @return The number of bytes sent on success. If no acknowledgment is received within the timeout, returns a failure code.
 *
 * @details The function performs the following steps:
 * 1. Serializes the attribute load into a buffer (`serialized_load`).
 * 2. Sets the message direction based on the message type (`id`).
 * 3. Creates an acknowledgment (ACK) message using the `create_message` function.
 * 4. Sends the message using the `send_message` function.
 * 5. Waits for a correct response (ACK) message using the `waitForCorrectResponseZigbee` function with a 10-second timeout.
 * 6. Logs a warning if no acknowledgment is received within the timeout.
 * 7. Destroys the acknowledgment message and returns the number of bytes sent.
 *
 * @note This function depends on the `waitForCorrectResponseZigbee` function to wait for an acknowledgment, so the caller should
 *       ensure that the Zigbee coordinator or device is expected to respond to the message.
 *
 * Example Usage:
 * @code
 * uint8_t tx_buffer[256];
 * iot_alarm_attr_load_t load = { ... };
 * int result = send_attr(UART_NUM_1, tx_buffer, &load, IOT_ALARM_MSGTYPE_ZB_DATA_READ);
 * if (result > 0) {
 *     // Attribute data sent successfully and acknowledgment received
 * } else {
 *     // Error occurred or no acknowledgment received
 * }
 * @endcode
 */
int send_attr(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_attr_load_t * load, message_type_t id);

// TODO
// int send_dev(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_dev_load_t * load, message_type_t id);

/**
 * @brief Serializes an attribute load structure into a buffer for transmission.
 *
 * This function serializes the content of an `iot_alarm_attr_load_t` structure into a buffer, preparing it for sending over
 * a communication medium (e.g., UART). The function handles different attribute value types, copying the appropriate data 
 * into the buffer, and calculating the total number of bytes serialized.
 *
 * @param attr A pointer to the `iot_alarm_attr_load_t` structure that contains the attribute data to be serialized.
 * @param buffer A pointer to the buffer where the serialized data will be stored.
 * @param bytes A pointer to a variable where the total number of serialized bytes will be stored.
 *
 * @return None.
 * 
 * @details The function performs the following steps:
 * 1. Validates the input parameters (`attr` and `buffer`). If either is `NULL`, a warning is logged and the function returns early.
 * 2. Serializes each field from the `iot_alarm_attr_load_t` structure into the `buffer` using `memcpy`.
 * 3. The fields serialized include:
 *    - `ieee_addr`: IEEE address of the device.
 *    - `short_addr`: Short address of the device.
 *    - `device_id`: The device ID.
 *    - `endpoint_id`: The endpoint ID.
 *    - `cluster_id`: The cluster ID.
 *    - `attr_id`: The attribute ID.
 *    - `value_type`: The type of the attribute value.
 *    - `value`: The actual attribute value, serialized according to its type.
 *    - `type_id`: The type ID of the attribute.
 *    - `type`: The type of the attribute.
 *    - `manuf`: Manufacturer name (null-terminated).
 *    - `name`: Name of the attribute (null-terminated).
 * 4. The total number of serialized bytes is stored in the `bytes` parameter.
 * 5. The buffer is null-terminated at appropriate places to ensure string fields are properly terminated.
 * 
 * @note The function assumes that the attribute's value can be of different types, including 8-bit, 16-bit, 32-bit, and other types.
 *       The serialization process ensures that each type is correctly handled based on the `value_type`.
 * 
 * Example Usage:
 * @code
 * iot_alarm_attr_load_t attr = { ... };
 * char buffer[1024];
 * size_t bytes;
 * serialize_attr(&attr, buffer, &bytes);
 * // Now, the buffer contains the serialized data, and bytes contains the size of the serialized data
 * @endcode
 */
void serialize_attr(iot_alarm_attr_load_t *attr, char *buffer, size_t *bytes);

/**
 * @brief Deserializes an attribute load from a buffer and populates an attribute structure.
 *
 * This function extracts various fields from a buffer, including device and attribute information, and deserializes them
 * into an `iot_alarm_attr_load_t` structure. It handles all necessary data type conversions and checks for buffer size
 * limitations during the deserialization process. The function ensures that all data fields are copied correctly and populates
 * the provided attribute structure with the extracted values.
 *
 * @param attr A pointer to a pointer of `iot_alarm_attr_load_t` where the deserialized attribute structure will be stored.
 *             If the provided pointer is `NULL`, no deserialization is performed.
 * @param buffer A pointer to the buffer containing the raw attribute data to be deserialized.
 * @param buffer_len The length of the buffer, which is used to validate that the buffer is large enough to hold the required data.
 *
 * @return void
 *
 * @details The function performs the following steps:
 * 1. Validates that the provided pointers (`attr` and `buffer`) are not `NULL`.
 * 2. Reads and deserializes fields from the buffer, including:
 *    - `ieee_addr`: IEEE address of the device.
 *    - `short_addr`: Short address of the device.
 *    - `device_id`: The device identifier.
 *    - `endpoint_id`: The endpoint identifier.
 *    - `cluster_id`: The cluster identifier.
 *    - `attr_id`: The attribute identifier.
 *    - `value_type`: The type of the attribute value.
 *    - `value`: The value of the attribute, if applicable.
 *    - `type_id`: The type identifier of the attribute.
 *    - `type`: The attribute's type.
 *    - `manuf`: The manufacturer of the device.
 *    - `name`: The name of the device.
 * 3. Checks if the buffer has enough data to deserialize each field and logs warnings if there is not enough data.
 * 4. Creates a new attribute structure using `create_attr` and populates it with the deserialized data.
 * 5. Destroys any existing attribute data using `destroy_attr` to avoid memory leaks.
 * 6. Populates the provided `attr` pointer with the newly created `iot_alarm_attr_load_t` structure containing the deserialized data.
 *
 * @note If any buffer size limitations are encountered during deserialization, the function logs a warning and skips deserialization for the affected fields.
 * 
 * Example Usage:
 * @code
 * uint8_t buffer[256]; // Example buffer filled with raw attribute data.
 * iot_alarm_attr_load_t *attr = NULL;
 * size_t buffer_len = sizeof(buffer);
 * deserialize_attr(&attr, buffer, buffer_len);
 * if (attr != NULL) {
 *     // Process the deserialized attribute data
 * } else {
 *     // Error in deserialization
 * }
 * @endcode
 */
void deserialize_attr(iot_alarm_attr_load_t **attr, uint8_t *buffer, size_t buffer_len);

/**
 * @brief Creates and initializes a new attribute structure.
 *
 * This function dynamically allocates memory for an `iot_alarm_attr_load_t` structure and initializes its fields
 * with the provided values. It copies the given manufacturer, name, type, and other attribute-specific information into the
 * structure. The function also handles various value types (e.g., 8-bit, 16-bit, 32-bit) by copying the provided value 
 * accordingly.
 *
 * @param manuf The manufacturer string. This will be copied to the `manuf` field of the attribute structure.
 * @param name The name string. This will be copied to the `name` field of the attribute structure.
 * @param type The type string. This will be copied to the `type` field of the attribute structure.
 * @param type_id The type ID of the attribute.
 * @param ieee_addr The IEEE address of the device, which will be copied into the `ieee_addr` field.
 * @param short_addr The short address of the device, which will be stored in the `short_addr` field.
 * @param device_id The device ID, which will be stored in the `device_id` field.
 * @param endpoint_id The endpoint ID, which will be stored in the `endpoint_id` field.
 * @param cluster_id The cluster ID for the attribute, which will be stored in the `cluster_id` field.
 * @param attr_id The attribute ID, which will be stored in the `attr_id` field.
 * @param value_type The type of the value (e.g., 8-bit, 16-bit, 32-bit). This will determine how the value is copied.
 * @param value The value of the attribute. This will be stored in the `value` field, with its type determined by `value_type`.
 *
 * @return A pointer to the dynamically allocated and initialized `iot_alarm_attr_load_t` structure, or `NULL` if memory 
 *         allocation fails.
 *
 * @details The function performs the following:
 * 1. Allocates memory for the `iot_alarm_attr_load_t` structure.
 * 2. Copies the `manuf`, `name`, and `type` strings into the structure fields (`manuf`, `name`, `type`), ensuring that the 
 *    strings are properly null-terminated (up to 50 characters).
 * 3. Copies the provided values into the relevant fields of the structure (`type_id`, `ieee_addr`, `short_addr`, etc.).
 * 4. Based on the `value_type`, the function copies the `value` into the `value` field using `memcpy` for the supported types.
 * 5. Returns the pointer to the created attribute structure.
 *
 * @note This function does not handle all possible value types; only types that are supported by the current implementation 
 *       (e.g., 8-bit, 16-bit, 32-bit) are processed. If an unsupported type is provided, the function will not set the value.
 *
 * Example Usage:
 * @code
 * const char *manuf = "Example Manufacturer";
 * const char *name = "Example Attribute";
 * const char *type = "Temperature";
 * uint32_t type_id = 1;
 * esp_zb_ieee_addr_t ieee_addr = {0x00, 0x0d, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x00}; // Example IEEE address
 * uint16_t short_addr = 0x1234;
 * uint8_t device_id = 0x01;
 * uint8_t endpoint_id = 0x01;
 * uint16_t cluster_id = 0x0000;
 * uint16_t attr_id = 0x0001;
 * esp_zb_zcl_attr_type_t value_type = ESP_ZB_ZCL_ATTR_TYPE_U32;
 * uint32_t value = 25;

 * iot_alarm_attr_load_t *attr = create_attr(manuf, name, type, type_id, ieee_addr, short_addr, device_id, 
 *                                           endpoint_id, cluster_id, attr_id, value_type, value);
 * if (attr != NULL) {
 *     // Attribute structure created successfully
 * } else {
 *     // Failed to allocate memory for attribute structure
 * }
 * @endcode
 */
iot_alarm_attr_load_t * create_attr(const char * manuf, const char * name, const char * type, uint32_t type_id, esp_zb_ieee_addr_t ieee_addr, uint16_t short_addr, uint8_t device_id, uint8_t endpoint_id, uint16_t cluster_id, uint16_t attr_id, esp_zb_zcl_attr_type_t value_type, uint32_t value);

/**
 * @brief Frees the memory allocated for an attribute structure.
 *
 * This function takes a pointer to a pointer to an `iot_alarm_attr_load_t` structure and frees the dynamically allocated
 * memory used by the attribute structure. After freeing the memory, it sets the pointer to `NULL` to avoid dangling
 * references.
 *
 * @param attr A pointer to a pointer to an `iot_alarm_attr_load_t` structure to be freed. This pointer should be 
 *             initialized to point to a dynamically allocated structure.
 *
 * @return None
 *
 * @details The function performs the following:
 * 1. Checks if the input pointer `*attr` is not `NULL`.
 * 2. If the pointer is valid (i.e., the structure is allocated), it calls `free()` to release the memory.
 * 3. It then sets the pointer `*attr` to `NULL` to ensure that the pointer is no longer pointing to the freed memory.
 *
 * @note After calling this function, the `*attr` pointer will be `NULL`, so it should not be dereferenced again.
 *
 * Example Usage:
 * @code
 * iot_alarm_attr_load_t *attr = create_attr("Example Manufacturer", "Temperature", "Temperature Sensor", 
 *                                           1, ieee_addr, 0x1234, 0x01, 0x01, 0x0000, 0x0001, 
 *                                           ESP_ZB_ZCL_ATTR_TYPE_U32, 25);
 * if (attr != NULL) {
 *     // Use the attribute...
 *     destroy_attr(&attr); // Free the memory after use
 * }
 * @endcode
 */
void destroy_attr(iot_alarm_attr_load_t **attr);

/**
 * @brief Compares two attribute structures for equality.
 *
 * This function compares two `iot_alarm_attr_load_t` attribute structures field by field to determine if they are equal.
 * The comparison is performed on all relevant fields, including string values and numerical attributes. The function
 * returns `true` if all fields match, and `false` otherwise.
 *
 * @param attr1 The first attribute structure to compare.
 * @param attr2 The second attribute structure to compare.
 *
 * @return `true` if both attribute structures are equal, `false` otherwise.
 *
 * @details The comparison includes the following fields:
 * 1. `manuf` - Manufacturer string (compared using `strcmp`).
 * 2. `name` - Name string (compared using `strcmp`).
 * 3. `type` - Type string (compared using `strcmp`).
 * 4. `type_id` - Numeric type ID (compared using `==`).
 * 5. `device_id` - Numeric device ID (compared using `==`).
 * 6. `endpoint_id` - Numeric endpoint ID (compared using `==`).
 * 7. `cluster_id` - Numeric cluster ID (compared using `==`).
 * 8. `attr_id` - Numeric attribute ID (compared using `==`).
 * 9. `value_type` - Numeric value type (compared using `==`).
 * 10. `value` - Numeric value (compared using `==`).
 * 11. `short_addr` - Numeric short address (compared using `==`).
 * 12. `ieee_addr` - IEEE address (compared using `memcmp`).
 *
 * The function performs a deep comparison of the attribute structures, including checking all the string values
 * and numeric values. If all fields match between the two structures, the function returns `true`. If any field
 * does not match, it returns `false`.
 *
 * Example Usage:
 * @code
 * iot_alarm_attr_load_t attr1 = create_attr("Manufacturer1", "Sensor1", "Temperature", 1, ieee_addr, 0x1234, 0x01, 0x01, 0x0000, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U32, 25);
 * iot_alarm_attr_load_t attr2 = create_attr("Manufacturer1", "Sensor1", "Temperature", 1, ieee_addr, 0x1234, 0x01, 0x01, 0x0000, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U32, 25);
 * 
 * if (compare_attr(attr1, attr2)) {
 *     // Attributes are equal
 * } else {
 *     // Attributes are not equal
 * }
 * 
 * destroy_attr(&attr1);
 * destroy_attr(&attr2);
 * @endcode
 */
bool compare_attr(iot_alarm_attr_load_t attr1, iot_alarm_attr_load_t attr2);

/**
 * @brief Copies the contents of one attribute structure to another.
 *
 * This function copies the values from a source `iot_alarm_attr_load_t` structure (`src`) to a destination structure (`dst`).
 * It performs a field-by-field copy for all relevant fields in the structure. If either the source or the destination
 * is `NULL`, no copy operation is performed.
 *
 * @param src The source attribute structure from which data will be copied.
 * @param dst The destination attribute structure to which data will be copied.
 *
 * @return None.
 *
 * @details The function performs a shallow copy of the data between the two attribute structures. Specifically, it copies:
 * 1. `manuf` - Manufacturer string (using `memcpy`).
 * 2. `name` - Name string (using `memcpy`).
 * 3. `type` - Type string (using `memcpy`).
 * 4. `type_id` - Numeric type ID (using `=`).
 * 5. `short_addr` - Numeric short address (using `=`).
 * 6. `ieee_addr` - IEEE address (using `memcpy`).
 * 7. `device_id` - Numeric device ID (using `=`).
 * 8. `endpoint_id` - Numeric endpoint ID (using `=`).
 * 9. `cluster_id` - Numeric cluster ID (using `=`).
 * 10. `attr_id` - Numeric attribute ID (using `=`).
 * 11. `value_type` - Numeric value type (using `=`).
 * 12. `value` - Numeric value (using `=`).
 *
 * If either the source or the destination is `NULL`, the function does nothing. If both are non-NULL, it copies the
 * entire structure from the source to the destination.
 *
 * Example Usage:
 * @code
 * iot_alarm_attr_load_t src_attr = create_attr("Manufacturer1", "Sensor1", "Temperature", 1, ieee_addr, 0x1234, 0x01, 0x01, 0x0000, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U32, 25);
 * iot_alarm_attr_load_t dst_attr;
 * 
 * copy_attr(&src_attr, &dst_attr);
 * 
 * // Now dst_attr is a copy of src_attr
 * destroy_attr(&src_attr);
 * destroy_attr(&dst_attr);
 * @endcode
 */
void copy_attr(iot_alarm_attr_load_t * src, iot_alarm_attr_load_t * dst);

/**
 * @brief Packs the attribute structure into a JSON string.
 *
 * This function takes an attribute structure (`iot_alarm_attr_load_t`) and converts its fields into a JSON string. 
 * It populates the JSON document with the relevant information from the structure, such as manufacturer, device ID, 
 * IEEE address, type, and value. The resulting JSON is serialized into the provided `jsonStr` string.
 * 
 * @param attr The attribute structure to be packed into JSON.
 * @param jsonStr The String object where the serialized JSON data will be stored.
 * 
 * @return `true` if the packing was successful and the JSON was serialized, `false` otherwise.
 *
 * @details This function uses the `ArduinoJson` library to create and serialize a JSON document. The following fields
 * from the `iot_alarm_attr_load_t` structure are serialized into the JSON:
 * 1. `short_addr`: The short address of the device.
 * 2. `ieee_addr`: The IEEE address, formatted as a string (e.g., "00:11:22:33:44:55:66:77").
 * 3. `device_id`: The device ID.
 * 4. `manufacturer`: The manufacturer name.
 * 5. `name`: The name of the device.
 * 6. `type`: The type of the device.
 * 7. `type_id`: The type ID.
 * 8. `endpoint_id`: The endpoint ID.
 * 9. `cluster_id`: The cluster ID.
 * 10. `attr_id`: The attribute ID.
 * 11. `value_type`: The type of the attribute value.
 * 12. `value`: The attribute value.
 * 13. `timestamp`: A timestamp for when the attribute was captured (using the global `g_vars_ptr->datetime`).
 * 
 * If the serialization fails, an error message is logged, and `false` is returned. Otherwise, `true` is returned.
 * 
 * Example usage:
 * @code
 * iot_alarm_attr_load_t attr = create_attr("Manufacturer", "Temperature Sensor", "Temperature", 1, ieee_addr, 0x1234, 1, 2, 0x0000, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U32, 25);
 * String jsonStr;
 * 
 * if (pack_attr(&attr, &jsonStr)) {
 *     Serial.println(jsonStr); // JSON packed successfully
 * } else {
 *     Serial.println("Failed to pack attribute!");
 * }
 * 
 * destroy_attr(&attr);
 * @endcode
 */
bool pack_attr(iot_alarm_attr_load_t * attr, String * jsonStr);

/**
 * @brief Unpacks a JSON string into an attribute structure.
 *
 * This function takes a JSON string and deserializes it into an attribute structure (`iot_alarm_attr_load_t`).
 * The JSON should contain fields representing various attributes of a device, such as IEEE address, short address,
 * device ID, endpoint ID, and more. The function validates the presence of necessary fields before populating the structure.
 *
 * @param attr A pointer to the attribute structure where the data will be unpacked.
 * @param jsonStr The JSON string containing the attribute data to be unpacked.
 * 
 * @return `true` if the unpacking was successful and the structure was populated with the parsed data, `false` otherwise.
 *
 * @details The function first checks the validity of the input parameters, then it parses the JSON string and validates
 * that the required fields are present. If the fields are present and valid, the function populates the structure's fields.
 * If any errors occur during parsing or field validation, the function returns `false` and logs an appropriate warning.
 * 
 * Example usage:
 * @code
 * String jsonStr = "{\"device\": {\"short\": 1234, \"ieee\": \"00:11:22:33:44:55:66:77\", \"id\": 1, \"manufacturer\": \"XYZ Corp\", \"name\": \"Sensor\", \"type\": \"Temperature\", \"type_id\": 1}, \"ep_id\": 1, \"cluster_id\": 0x0001, \"attr_id\": 0x0002, \"value_type\": 1, \"value\": 100}";
 * iot_alarm_attr_load_t attr;
 * 
 * if (unpack_attr(&attr, jsonStr)) {
 *     Serial.println("Attribute unpacked successfully.");
 * } else {
 *     Serial.println("Failed to unpack attribute.");
 * }
 * 
 * destroy_attr(&attr);
 * @endcode
 */
bool unpack_attr(iot_alarm_attr_load_t * attr, String jsonStr);

#endif
