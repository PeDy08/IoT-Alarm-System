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
    message_direction_t dir;
    message_status_t st;
    message_type_t id;
    uint32_t length;
    char* load;
} iot_alarm_message_t;

typedef uint8_t esp_zb_64bit_addr_t[8];
typedef esp_zb_64bit_addr_t esp_zb_ieee_addr_t;

typedef struct {
    uint16_t cluster_id;
    uint8_t endpoint_id;
} user_ctx_t;

typedef struct {
    uint16_t cluster_id; // Cluster ID
} cluster_info_t;

typedef struct {
    uint8_t endpoint_id;          // Endpoint ID
    uint16_t app_profile_id;      // Application Profile ID
    uint16_t app_device_id;       // Application Device ID
    uint8_t app_device_version;    // Application Device Version
    uint8_t input_cluster_count;   // Number of input clusters
    uint8_t output_cluster_count;  // Number of output clusters
    cluster_info_t *input_clusters;  // Pointer to dynamically allocated input clusters
    cluster_info_t *output_clusters; // Pointer to dynamically allocated output clusters
} endpoint_info_t;

typedef struct {
    bool usefull;
    char manuf[50];
    char name[50];
    char type[50];
    uint32_t type_id;
    esp_zb_ieee_addr_t ieee_addr;
    uint16_t short_addr;
    uint8_t endpoint_count;        // Number of active endpoints
    endpoint_info_t *endpoints;    // Pointer to dynamically allocated endpoints
} device_info_t;

typedef struct {
    char manuf[50];
    char name[50];
    char type[50];
    uint32_t type_id;
    esp_zb_ieee_addr_t ieee_addr;
    uint16_t short_addr;    
    uint8_t device_id;
    uint8_t endpoint_id;
    uint16_t cluster_id;
    uint16_t attr_id;
    esp_zb_zcl_attr_type_t value_type;
    uint32_t value;
} iot_alarm_attr_load_t;

/* typedef struct {
    uint8_t device_id;
    uint8_t devices_len;
    device_info_t device;
} iot_alarm_dev_load_t; */

/**
 * @brief Updates the communication between the Arduino and the zigbee module.
 * 
 * This function checks if there is any data available on the Serial port and sends it
 * to the zigbee module. Similarly, it reads any available data from the zigbee module and 
 * sends it to the Serial port for debugging purposes.
 */
void updateSerialZigbee();

/**
 * @brief Initializes the zigbee module communication.
 * 
 * TODO description
 * 
 * @return true if the zigbee module was successfully initialized, false otherwise.
 */
bool initSerialZigbee();

bool zigbeeReset();
bool zigbeeFactory();
bool zigbeeCount();
bool zigbeeOpen(uint8_t duration = 180);
bool zigbeeClose();
bool zigbeeClear();
bool zigbeeAttrRead(iot_alarm_attr_load_t * attr);
bool zigbeeAttrWrite(iot_alarm_attr_load_t * attr);
bool zigbeeAttrReadWriteHandler(iot_alarm_attr_load_t * attr);
bool zigbeeAttrReportHandler(iot_alarm_attr_load_t * attr);

// *********************************************************************************************************************

void serialize_message(iot_alarm_message_t *msg, uint8_t *buffer, size_t *bytes);
void deserialize_message(iot_alarm_message_t **msg, uint8_t *buffer, size_t buffer_len);

iot_alarm_message_t * create_message(message_direction_t dir, message_status_t st, message_type_t id, uint32_t length, const char* load);
void destroy_message(iot_alarm_message_t **msg);

// *********************************************************************************************************************

int send_message(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_message_t *msg);
int receive_message(uart_port_t uart, uint8_t* rx_buffer, iot_alarm_message_t **msg, size_t max_len);

int send_attr(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_attr_load_t * load, message_type_t id);
// int send_dev(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_dev_load_t * load, message_type_t id);

void serialize_attr(iot_alarm_attr_load_t *attr, char *buffer, size_t *bytes);
void deserialize_attr(iot_alarm_attr_load_t **attr, uint8_t *buffer, size_t buffer_len);
iot_alarm_attr_load_t * create_attr(const char * manuf, const char * name, const char * type, uint32_t type_id, esp_zb_ieee_addr_t ieee_addr, uint16_t short_addr, uint8_t device_id, uint8_t endpoint_id, uint16_t cluster_id, uint16_t attr_id, esp_zb_zcl_attr_type_t value_type, uint32_t value);
void destroy_attr(iot_alarm_attr_load_t **attr);
bool compare_attr(iot_alarm_attr_load_t attr1, iot_alarm_attr_load_t attr2);
void copy_attr(iot_alarm_attr_load_t * src, iot_alarm_attr_load_t * dst);
bool pack_attr(iot_alarm_attr_load_t * attr, String * jsonStr);
bool unpack_attr(iot_alarm_attr_load_t * attr, String jsonStr);

#endif
