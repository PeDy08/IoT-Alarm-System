#include "libZigbee.h"

HardwareSerial SerialZigbee(2);

extern TaskHandle_t handleTaskZigbee;
extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

void updateSerialZigbee() {
    while (Serial.available()) {
        SerialZigbee.write(Serial.read());
    }

    while (SerialZigbee.available()) {
        Serial.write(SerialZigbee.read());
    }
}

/**
 * @brief Sends a message to the Zigbee module and waits for the correct acknowledgment response.
 *
 * This function sends a message over the Zigbee UART interface and waits for a corresponding acknowledgment
 * within a specified timeout period. It temporarily suspends the Zigbee task to ensure exclusive access to
 * the UART interface. The function checks if the received message matches the expected response type and ID.
 *
 * @param[in] sendMsg Pointer to the message to be sent.
 * @param[out] receivedMsg Double pointer to store the received acknowledgment message.
 * @param[in] timeout Optional timeout in milliseconds to wait for a response (default is 5000 ms).
 * @return The number of bytes sent on success, or -1 if the operation times out or fails.
 *
 * @note The function suspends `handleTaskZigbee` during the operation to avoid UART conflicts.
 *
 * @warning If `sendMsg` or `receivedMsg` is `NULL`, the function logs a warning and returns `false`.
 *
 * @details The function operates as follows:
 *  1. Clears any pending data in the Zigbee UART buffer.
 *  2. Suspends `handleTaskZigbee` if it's not the current task.
 *  3. Sends the message using `send_message()`.
 *  4. Waits for an acknowledgment response by calling `receive_message()`.
 *  5. Verifies the response direction, ID, and status.
 *  6. Resumes `handleTaskZigbee` after completion.
 *
 * Example Usage:
 * @code
 * iot_alarm_message_t sendMsg = {...};
 * iot_alarm_message_t *receivedMsg = NULL;
 * int result = waitForCorrectResponseZigbee(&sendMsg, &receivedMsg);
 * if (result > 0) {
 *     // Success handling
 * } else {
 *     // Error handling
 * }
 * @endcode
 */
int waitForCorrectResponseZigbee(iot_alarm_message_t * sendMsg, iot_alarm_message_t ** receivedMsg, unsigned long timeout = 5000) {
    if (sendMsg == NULL || receivedMsg == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(waitForCorrectResponseZigbee)", "Message to be sent or received is nullptr!");
        return false;
    }

    while (SerialZigbee.available()) {
        SerialZigbee.read();
    }


    if (xTaskGetCurrentTaskHandle() != handleTaskZigbee && handleTaskZigbee != NULL) {
        vTaskSuspend(handleTaskZigbee);
    }

    unsigned long startTime = millis();
    int tx_bytes = 0;
    int rx_bytes;
    bool ret = false;
    while (millis() - startTime < timeout) {
        // send command
        tx_bytes = send_message(UART, tx_buffer, sendMsg);

        vTaskDelay(550 / portTICK_PERIOD_MS);

        // receive command
        rx_bytes = receive_message(UART, rx_buffer, receivedMsg, RX_BUF_SIZE-1);

        // check if acknowledge is received
        if (((sendMsg->dir == IOT_ALARM_MSGDIR_COMMAND && (*receivedMsg)->dir == IOT_ALARM_MSGDIR_COMMAND_ACK) ||
                (sendMsg->dir == IOT_ALARM_MSGDIR_NOTIFICATION && (*receivedMsg)->dir == IOT_ALARM_MSGDIR_NOTIFICATION_ACK)) &&
                sendMsg->id == (*receivedMsg)->id && (*receivedMsg)->st == IOT_ALARM_MSGSTATUS_SUCCESS) {
            ret = true;
            break;
        }
    }

    if (xTaskGetCurrentTaskHandle() != handleTaskZigbee && handleTaskZigbee != NULL) {
        vTaskResume(handleTaskZigbee);
    }

    if (ret == false) {
        esplogW(TAG_LIB_ZIGBEE, "(waitForCorrectResponseZigbee)", "Zigbee module didnt responded in time!");
        tx_bytes = -1;
    }
    
    return tx_bytes;
}

bool initSerialZigbee() {
    SerialZigbee.begin(ZIGBEE_BAUDRATE, SERIAL_8N1, ZIGBEE_RX_PIN, ZIGBEE_TX_PIN);
    SerialZigbee.setTimeout(ZIGBEE_TIMEOUT);

    #ifdef ZIGBEE_EN_PIN
    pinMode(ZIGBEE_EN_PIN, OUTPUT);
    digitalWrite(ZIGBEE_EN_PIN, LOW);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    digitalWrite(ZIGBEE_EN_PIN, HIGH);
    #endif

    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ECHO, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");
    int ret = true;

    esplogI(TAG_LIB_ZIGBEE, "(initSerialZigbee)", "ZIGBEE INITIALISATION!");

    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (!ret || tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(initSerialZigbee)", "Failed finding zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(initSerialZigbee)", "Zigbee module found!");
    }

    destroy_message(&msg);
    destroy_message(&ack);

    return ret;
}

bool zigbeeReset() {
    int ret = true;
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_CTL_RESTART, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeReset)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeReset)", "Reset command sent to zigbee module!");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeFactory() {
    int ret = true;
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_CTL_FACTORY, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeFactory)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeFactory)", "Factory reset command sent to zigbee module!");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}


bool zigbeeCount() {
    int ret = true;
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_DEV_COUNT, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeCount)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeCount)", "Devices count command sent to zigbee module!");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeOpen(uint8_t duration) {
    int ret = true;
    char load[12];
    sprintf(load, "%d", duration);
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DEV_UNLOCK, 4, load);
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeOpen)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeOpen)", "Open zigbee network command sent to zigbee module!");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeClose() {
    int ret = true;
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DEV_LOCK, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeClose)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeClose)", "Close zigbee network command sent to zigbee module!");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeClear() {
    int ret = true;
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DEV_CLEAR, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeClear)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeClear)", "Clear zigbee network command sent to zigbee module!");
        displayNotification(NOTIFICATION_ZIGBEE_NET_CLEAR);
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeAttrRead(iot_alarm_attr_load_t * attr) {
    int ret = true;
    size_t length;

    char serialized_load[1024];
    memset(serialized_load, 0, sizeof(serialized_load));
    serialize_attr(attr, serialized_load, &length);

    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DATA_READ, (uint32_t)length, serialized_load);
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeAttrRead)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeAttrRead)", "Read attribute command sent to zigbee module!");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeAttrWrite(iot_alarm_attr_load_t * attr) {
    int ret = true;
    size_t length;
    
    char serialized_load[1024];
    memset(serialized_load, 0, sizeof(serialized_load));
    serialize_attr(attr, serialized_load, &length);

    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DATA_WRITE, (uint32_t)length, serialized_load);
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    int tx_bytes = waitForCorrectResponseZigbee(msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(zigbeeAttrWrite)", "Failed sending message to zigbee module!");
        ret = false;
    } else {
        esplogI(TAG_LIB_ZIGBEE, "(zigbeeAttrWrite)", "Write attribute command sent to zigbee module!");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    destroy_attr(&attr);
    return ret;
}

// TODO
bool zigbeeAttrReadWriteHandler(iot_alarm_attr_load_t * attr) {return true;}

bool zigbeeAttrReportHandler(iot_alarm_attr_load_t * attr) {
    bool ret = false;

    if (attr != NULL) {
        // esplogI(TAG_LIB_DEBUG, "(zigbeeAttrReportHandler)", "Device data: %s - %s [%s (%lu)]", attr->manuf, attr->name, attr->type, attr->type_id);
        // esplogI(TAG_LIB_DEBUG, "(zigbeeAttrReportHandler)", "Attribute data: short: %04hx/%d, cluster: %04hx, attribute: %04hx, type: %d, value: %lu",
        //     attr->short_addr, attr->endpoint_id, attr->cluster_id, attr->attr_id, attr->value_type, attr->value);

        // local application handeling
        switch (attr->type_id) {
            // handle IAS zone reports, occupacy reports
            case 0x0500000DU:
            case 0x05000015U:
            case 0x0500002DU:
            case 0x05000225U:
                if (attr->attr_id == 0x0002 && attr->value == 1) {
                    esplogW(TAG_RTOS_ZIGBEE, "(zigbeeAttrReportHandler)", "Alarm event triggered! [ZONESTATUS = 1 at 0x%04hx/%d]", attr->short_addr, attr->endpoint_id);
                    displayNotification(NOTIFICATION_ZIGBEE_ATTR_REPORT);
                    if (g_vars_ptr->state == STATE_ALARM_OK || g_vars_ptr->state == STATE_ALARM_W) {
                        g_vars_ptr->alarm.alarm_events++;
                    }
                }
                break;

            case 0x04060000U:
            case 0x04060001U:
            case 0x04060002U:
                if (attr->attr_id == 0x0000 && attr->value == 1) {
                    esplogW(TAG_RTOS_ZIGBEE, "(zigbeeAttrReportHandler)", "Alarm event triggered! [OCCUPANCY = 1 at 0x%04hx/%d]", attr->short_addr, attr->endpoint_id);
                    displayNotification(NOTIFICATION_ZIGBEE_ATTR_REPORT);
                    if (g_vars_ptr->state == STATE_ALARM_OK || g_vars_ptr->state == STATE_ALARM_W) {
                        g_vars_ptr->alarm.alarm_events++;
                    }
                }
                break;

            // handle fire sensor reports
            case 0x05000028U:
            case 0x0500002BU:
                if (attr->attr_id == 0x0002) {
                    if (attr->value == 1) {esplogW(TAG_RTOS_ZIGBEE, "(zigbeeAttrReportHandler)", "Fire alarm triggered! [ZONESTATUS = 1 at 0x%04hx/%d]", attr->short_addr, attr->endpoint_id);}
                    g_vars_ptr->alarm.alarm_fire = attr->value > 0;
                }
                break;

            // handle water sensor reports
            case 0x0500002AU:
                if (attr->attr_id == 0x0002) {
                    if (attr->value == 1) {esplogW(TAG_RTOS_ZIGBEE, "(zigbeeAttrReportHandler)", "Water-leakage alarm triggered! [ZONESTATUS = 1 at 0x%04hx/%d]", attr->short_addr, attr->endpoint_id);}
                    g_vars_ptr->alarm.alarm_water = attr->value > 0;
                }
                break;

            default:
                break;
        }
        ret = true;
    }

    return ret;
}


// *********************************************************************************************************************

const int RX_BUF_SIZE = 1024;
const int TX_BUF_SIZE = 1024;

uint8_t* tx_buffer = NULL;
uint8_t* rx_buffer = NULL;

void serialize_message(iot_alarm_message_t *msg, uint8_t *buffer, size_t *bytes) {
    
    if (msg == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(serialize_message)", "Error: Message is nullptr!");
        return;
    }
    
    if (buffer == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(serialize_message)", "Error: Buffer is nullptr!");
        return;
    }

    int offset = 0;

    // Copy the struct fields into the buffer
    memcpy(buffer + offset, &msg->dir, sizeof(msg->dir));
    offset += sizeof(msg->dir);

    memcpy(buffer + offset, &msg->st, sizeof(msg->st));
    offset += sizeof(msg->st);

    memcpy(buffer + offset, &msg->id, sizeof(msg->id));
    offset += sizeof(msg->id);

    memcpy(buffer + offset, &msg->length, sizeof(msg->length));
    offset += sizeof(msg->length);

    // If there is a load, copy it (i want to copy the "\0" too if there is no load)
    if (msg->length > 0) {
        memcpy(buffer + offset, msg->load, msg->length);
        offset += msg->length;
    }

    buffer[offset] = '\0';
    *bytes = offset;
}

void deserialize_message(iot_alarm_message_t **msg, uint8_t *buffer, size_t buffer_len) {

    if (msg == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "Error: Message is nullptr!");
        return;
    }

    if (*msg == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "Error: Message is nullptr!");
        return;
    }

    if (buffer == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "Error: Buffer is nullptr!");
        return;
    }

    int offset = 0;

    message_direction_t dir;
    message_status_t st;
    message_type_t id;
    uint32_t length;

    // Extract message direction
    if (offset + sizeof(dir) <= buffer_len) {
        memcpy(&dir, buffer + offset, sizeof(dir));
        offset += sizeof(dir);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "The buffer length is too small to deserialise all data!");
    }

    // Extract message status
    if (offset + sizeof(st) <= buffer_len) {
        memcpy(&st, buffer + offset, sizeof(st));
        offset += sizeof(st);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "The buffer length is too small to deserialise all data!");
    }

    // Extract message ID
    if (offset + sizeof(id) <= buffer_len) {
        memcpy(&id, buffer + offset, sizeof(id));
        offset += sizeof(id);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "The buffer length is too small to deserialise all data!");
    }

    // Extract length of load
    if (offset + sizeof(length) <= buffer_len) {
        memcpy(&length, buffer + offset, sizeof(length));
        offset += sizeof(length);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "The buffer length is too small to deserialise all data!");
    }

    // Extract load itself
    char* load = (char *)malloc(length + 1);
    if (load == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "Failed to allocate memory for message load!");
        return;
    }

    if (length > 0 && offset + length <= buffer_len) {
        memcpy(load, buffer + offset, length);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_message)", "The buffer length is too small to deserialise all data!");
    }

    load[length] = '\0';

    destroy_message(msg);
    *msg = (iot_alarm_message_t *)malloc(sizeof(iot_alarm_message_t));
    if (msg == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(create_message)", "Failed to allocate memory for message!");
        free(load);
        load = NULL;
        return;
    }

    (*msg)->dir = dir;
    (*msg)->id = id;
    (*msg)->st = st;
    (*msg)->length = length;
    (*msg)->load = load;
}

iot_alarm_message_t * create_message(message_direction_t dir, message_status_t st, message_type_t id, uint32_t length, const char* load) {

    iot_alarm_message_t *msg = (iot_alarm_message_t *)malloc(sizeof(iot_alarm_message_t));
    if (msg == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(create_message)", "Failed to allocate memory for message!");
        return NULL;
    }

    msg->dir = dir;
    msg->id = id;
    msg->st = st;
    msg->length = length;
    msg->load = (char *)malloc(length + 1);
    if (msg->load == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(create_message)", "Failed to allocate memory for message load!");
        free(msg);
        msg = NULL;
        return NULL;
    }
    memcpy(msg->load, load, length);
    msg->load[length] = '\0';

    return msg;
}

void destroy_message(iot_alarm_message_t **msg) {
    if (*msg != NULL) {
        if ((*msg)->load != NULL) {
            free((*msg)->load);
            (*msg)->load = NULL;
        }

        free(*msg);
        *msg = NULL;
    }
}

// *********************************************************************************************************************

int read_uart(uart_port_t uart, uint8_t* rx_buffer, size_t max_len) {
    size_t available = 0;
    size_t read = 0;
    int bytes_read = 0;

    vTaskDelay(100 / portTICK_PERIOD_MS);
    uart_get_buffered_data_len(uart, &available);
    while (available > 0 && bytes_read < max_len) {
        bytes_read = uart_read_bytes(uart, rx_buffer+bytes_read, available, pdMS_TO_TICKS(500));
        if (bytes_read > 0) {
            read += bytes_read;
        }

        uart_get_buffered_data_len(uart, &available);
    }

    return read;
}

int send_message(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_message_t *msg) {
    size_t length;

    serialize_message(msg, tx_buffer, &length);
    int tx_bytes = uart_write_bytes(uart, (const char *)tx_buffer, length);
    if (tx_bytes == length) {
        // esplogI(TAG_LIB_ZIGBEE, "(send_message)", "Message (length: %d) was sent successfully!", tx_bytes);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(send_message)", "Failed to send message!");
    }

    return tx_bytes;
}

int receive_message(uart_port_t uart, uint8_t* rx_buffer, iot_alarm_message_t **msg, size_t max_len) {
    if (msg == NULL || *msg == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(receive_message)", "Error: Message is nullptr!");
    }

    if (rx_buffer == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(receive_message)", "Error: Buffer is nullptr!");
    }

    memset(rx_buffer, 0, max_len);
    int rx_bytes = read_uart(uart, rx_buffer, max_len);

    /* char hex_string[rx_bytes * 3 + 1];
    for (size_t i = 0; i < rx_bytes; i++) {
        snprintf(&hex_string[i * 3], 4, "%02X ", rx_buffer[i]);
    }
    esplogI(TAG_LIB_DEBUG, "(receive_message)", "Buffer (length %d): %s", rx_bytes, hex_string); // */

    if (rx_bytes < max_len) {
        rx_buffer[rx_bytes] = 0;
    } else {
        rx_buffer[max_len - 1] = 0;
        esplogW(TAG_LIB_ZIGBEE, "(receive_message)", "RX buffer overflow. Data may be truncated.");
    }
    
    if (rx_bytes > 0) {
        deserialize_message(msg, rx_buffer, rx_bytes);
    }

    return rx_bytes;
}

int send_attr(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_attr_load_t * load, message_type_t id) {
    message_direction_t dir;
    size_t length;
    char serialized_load[1000];
    serialize_attr(load, serialized_load, &length);

    switch (id) {
        case IOT_ALARM_MSGTYPE_ZB_DATA_READ:
        case IOT_ALARM_MSGTYPE_ZB_DATA_WRITE:
        case IOT_ALARM_MSGTYPE_ZB_DATA_REPORT:
            dir = IOT_ALARM_MSGDIR_NOTIFICATION;
            break;
        
        default:
            dir = IOT_ALARM_MSGDIR_MAX;
            break;
    }

    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");
    iot_alarm_message_t msg = {
        .dir = dir,
        .st = IOT_ALARM_MSGSTATUS_SUCCESS,
        .id = id,
        .length = (uint16_t)length,
        .load = serialized_load,
    };

    int tx_bytes = waitForCorrectResponseZigbee(&msg, &ack, 10000);
    if (tx_bytes <= 0) {
        esplogW(TAG_LIB_ZIGBEE, "(send_attr)", "Failed to get acknowlede for sending attribute data!");
    }

    destroy_message(&ack);

    return tx_bytes;
}

/* int send_dev(uart_port_t uart, uint8_t* tx_buffer, iot_alarm_dev_load_t * load, message_type_t id) {
    // convert iot_alarm_dev_load_t to char*
    // compute the length of char* load
    // from load structure & id compute the dir

    iot_alarm_message_t msg = {
        .dir = ,
        .id = id,
        .st = IOT_ALARM_MSGSTATUS_SUCCESS,
        .length = ,
        .load = ,
    };

    int tx_bytes = send_message(uart, tx_buffer, &msg);
    return tx_bytes;
} */

// ----------------------------------------------------------------------------------------------------------------------------------------------------

void serialize_attr(iot_alarm_attr_load_t *attr, char *buffer, size_t *bytes) {
    if (attr == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(serialize_attr)", "Error: Message is nullptr!");
        return;
    }
    
    if (buffer == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(serialize_attr)", "Error: Buffer is nullptr!");
        return;
    }

    int offset = 0;

    memcpy(buffer + offset, &attr->ieee_addr, sizeof(attr->ieee_addr));
    offset += sizeof(attr->ieee_addr);
    memcpy(buffer + offset, &attr->short_addr, sizeof(attr->short_addr));
    offset += sizeof(attr->short_addr);

    memcpy(buffer + offset, &attr->device_id, sizeof(attr->device_id));
    offset += sizeof(attr->device_id);
    memcpy(buffer + offset, &attr->endpoint_id, sizeof(attr->endpoint_id));
    offset += sizeof(attr->endpoint_id);
    memcpy(buffer + offset, &attr->cluster_id, sizeof(attr->cluster_id));
    offset += sizeof(attr->cluster_id);
    memcpy(buffer + offset, &attr->attr_id, sizeof(attr->attr_id));
    offset += sizeof(attr->attr_id);

    memcpy(buffer + offset, &attr->value_type, sizeof(attr->value_type));
    offset += sizeof(attr->value_type);

    uint16_t length;
    switch (attr->value_type) {
        case ESP_ZB_ZCL_ATTR_TYPE_8BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:
        case ESP_ZB_ZCL_ATTR_TYPE_U8:

        case ESP_ZB_ZCL_ATTR_TYPE_16BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:
        case ESP_ZB_ZCL_ATTR_TYPE_U16:

        case ESP_ZB_ZCL_ATTR_TYPE_32BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_U32:
            memcpy(buffer + offset, &attr->value, sizeof(attr->value));
            offset += sizeof(attr->value);
            break;

        default:
            break;
    }

    memcpy(buffer + offset, &attr->type_id, sizeof(attr->type_id));
    offset += sizeof(attr->type_id);
    memcpy(buffer + offset, &attr->type, sizeof(attr->type));
    offset += sizeof(attr->type);
    buffer[offset-1] = '\0';
    memcpy(buffer + offset, &attr->manuf, sizeof(attr->manuf));
    offset += sizeof(attr->manuf);
    buffer[offset-1] = '\0';
    memcpy(buffer + offset, &attr->name, sizeof(attr->name));
    offset += sizeof(attr->name);
    buffer[offset-1] = '\0';

    buffer[offset] = '\0';
    *bytes = offset;
}

void deserialize_attr(iot_alarm_attr_load_t **attr, uint8_t *buffer, size_t buffer_len) {
    if (attr == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "Error: Message is nullptr!");
        return;
    }

    if (*attr == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "Error: Message is nullptr!");
        return;
    }

    if (buffer == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "Error: Buffer is nullptr!");
        return;
    }

    int offset = 0;

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

    if (offset + sizeof(ieee_addr) <= buffer_len) {
        memcpy(&ieee_addr, buffer + offset, sizeof(ieee_addr));
        offset += sizeof(ieee_addr);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    if (offset + sizeof(short_addr) <= buffer_len) {
        memcpy(&short_addr, buffer + offset, sizeof(short_addr));
        offset += sizeof(short_addr);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    if (offset + sizeof(device_id) <= buffer_len) {
        memcpy(&device_id, buffer + offset, sizeof(device_id));
        offset += sizeof(device_id);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    if (offset + sizeof(endpoint_id) <= buffer_len) {
        memcpy(&endpoint_id, buffer + offset, sizeof(endpoint_id));
        offset += sizeof(endpoint_id);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    if (offset + sizeof(cluster_id) <= buffer_len) {
        memcpy(&cluster_id, buffer + offset, sizeof(cluster_id));
        offset += sizeof(cluster_id);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    if (offset + sizeof(attr_id) <= buffer_len) {
        memcpy(&attr_id, buffer + offset, sizeof(attr_id));
        offset += sizeof(attr_id);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    if (offset + sizeof(value_type) <= buffer_len) {
        memcpy(&value_type, buffer + offset, sizeof(value_type));
        offset += sizeof(value_type);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    switch (value_type) {
        case ESP_ZB_ZCL_ATTR_TYPE_8BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:
        case ESP_ZB_ZCL_ATTR_TYPE_U8:

        case ESP_ZB_ZCL_ATTR_TYPE_16BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:
        case ESP_ZB_ZCL_ATTR_TYPE_U16:

        case ESP_ZB_ZCL_ATTR_TYPE_32BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_U32:
            if (offset + sizeof(value) <= buffer_len) {
                memcpy(&value, buffer + offset, sizeof(value));
                offset += sizeof(value);
            } else {
                esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
            }
            break;

        default:
            break;
    }

    if (offset + sizeof(type_id) <= buffer_len) {
        memcpy(&type_id, buffer + offset, sizeof(type_id));
        offset += sizeof(type_id);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    if (offset + sizeof(type) <= buffer_len) {
        memcpy(&type, buffer + offset, sizeof(type));
        offset += sizeof(type);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }
    
    if (offset + sizeof(manuf) <= buffer_len) {
        memcpy(&manuf, buffer + offset, sizeof(manuf));
        offset += sizeof(manuf);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }
    
    if (offset + sizeof(name) <= buffer_len) {
        memcpy(&name, buffer + offset, sizeof(name));
        offset += sizeof(name);
    } else {
        esplogW(TAG_LIB_ZIGBEE, "(deserialize_attr)", "The buffer length is too small to deserialise all data!");
    }

    /* char hex_string[offset * 3 + 1];
    for (size_t i = 0; i < offset; i++) {
        snprintf(&hex_string[i * 3], 4, "%02X ", buffer[i]);
    }
    esplogI(TAG_LIB_DEBUG, "(deserialize_attr)", "Buffer (length %d): %s", offset, hex_string); // */

    destroy_attr(attr);
    *attr = create_attr(manuf, name, type, type_id, ieee_addr, short_addr, device_id, endpoint_id, cluster_id, attr_id, value_type, value);
}

iot_alarm_attr_load_t * create_attr(const char * manuf,const  char * name,const  char * type, uint32_t type_id, esp_zb_ieee_addr_t ieee_addr, uint16_t short_addr, uint8_t device_id, uint8_t endpoint_id, uint16_t cluster_id, uint16_t attr_id, esp_zb_zcl_attr_type_t value_type, uint32_t value) {

    iot_alarm_attr_load_t *attr = (iot_alarm_attr_load_t *)malloc(sizeof(iot_alarm_attr_load_t));
    if (attr == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(create_attr)", "Failed to allocate memory for attr!");
        return NULL;
    }

    if (manuf != NULL) {
        snprintf(attr->manuf, sizeof(attr->manuf), "%s", manuf);
        attr->manuf[50] = '\0';
    }

    if (name != NULL) {
        snprintf(attr->name, sizeof(attr->name), "%s", name);
        attr->name[50] = '\0';
    }

    if (type != NULL) {
        snprintf(attr->type, sizeof(attr->type), "%s", type);
        attr->type[50] = '\0';
    }

    attr->type_id = type_id;
    memcpy(attr->ieee_addr, ieee_addr, sizeof(attr->ieee_addr));
    attr->short_addr = short_addr;
    attr->device_id = device_id;
    attr->endpoint_id = endpoint_id;
    attr->cluster_id = cluster_id;
    attr->attr_id = attr_id;
    attr->value_type = value_type;

    uint16_t length;
    switch (value_type) {
        case ESP_ZB_ZCL_ATTR_TYPE_8BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_8BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM:
        case ESP_ZB_ZCL_ATTR_TYPE_U8:

        case ESP_ZB_ZCL_ATTR_TYPE_16BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_16BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_16BIT_ENUM:
        case ESP_ZB_ZCL_ATTR_TYPE_U16:

        case ESP_ZB_ZCL_ATTR_TYPE_32BIT:
        case ESP_ZB_ZCL_ATTR_TYPE_32BITMAP:
        case ESP_ZB_ZCL_ATTR_TYPE_U32:
            memcpy(&attr->value, &value, sizeof(attr->value));
            break;

        default:
            break;
    }

    return attr;
}

void destroy_attr(iot_alarm_attr_load_t **attr) {
    if (*attr != NULL) {
        free(*attr);
        *attr = NULL;
    }
}

/* bool compare_attr(iot_alarm_attr_load_t attr1, iot_alarm_attr_load_t attr2) {
    return (attr1.manuf == attr2.manuf && attr1.name == attr2.name && attr1.type == attr2.type && attr1.type_id == attr2.type_id && 
        attr1.device_id == attr2.device_id && attr1.endpoint_id == attr2.endpoint_id && attr1.cluster_id == attr2.cluster_id && attr1.attr_id == attr2.attr_id && 
        attr1.value_type == attr2.value_type && attr1.value == attr2.value && attr1.short_addr == attr2.short_addr && attr1.ieee_addr == attr2.ieee_addr);
} */

bool compare_attr(iot_alarm_attr_load_t attr1, iot_alarm_attr_load_t attr2) {
    return (strcmp(attr1.manuf, attr2.manuf) == 0 &&   // Compare manuf strings
            strcmp(attr1.name, attr2.name) == 0 &&     // Compare name strings
            strcmp(attr1.type, attr2.type) == 0 &&     // Compare type strings
            attr1.type_id == attr2.type_id &&
            attr1.device_id == attr2.device_id &&
            attr1.endpoint_id == attr2.endpoint_id &&
            attr1.cluster_id == attr2.cluster_id &&
            attr1.attr_id == attr2.attr_id &&
            attr1.value_type == attr2.value_type &&
            attr1.value == attr2.value &&
            attr1.short_addr == attr2.short_addr &&
            memcmp(attr1.ieee_addr, attr2.ieee_addr, sizeof(attr1.ieee_addr)) == 0);  // Compare ieee_addr
}

void copy_attr(iot_alarm_attr_load_t * src, iot_alarm_attr_load_t * dst) {
    if (src != NULL && dst != NULL) {
        memcpy(dst->manuf, src->manuf, sizeof(dst->manuf));
        memcpy(dst->name, src->name, sizeof(dst->name));
        memcpy(dst->type, src->type, sizeof(dst->type));
        dst->type_id = src->type_id;
        dst->short_addr = src->short_addr;
        memcpy(dst->ieee_addr, src->ieee_addr, sizeof(dst->ieee_addr));
        dst->device_id = src->device_id;
        dst->endpoint_id = src->endpoint_id;
        dst->cluster_id = src->cluster_id;
        dst->attr_id = src->attr_id;
        dst->value_type = src->value_type;
        dst->value = src->value;
    }
}

bool pack_attr(iot_alarm_attr_load_t * attr, String * jsonStr) {
    if (attr == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(pack_attr)", "Error: Attr struct is nullptr!");
        return false;
    }

    /* esplogI(TAG_LIB_ZIGBEE, NULL, "Attr packing: short: %04hx, ieee: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X, dev_id: %d, ep_id: %d, cluster_id: %04hx, attr_id: %04hx, value: %lu",
                  attr->short_addr,
                  attr->ieee_addr[7], attr->ieee_addr[6], attr->ieee_addr[5], attr->ieee_addr[4],
                  attr->ieee_addr[3], attr->ieee_addr[2], attr->ieee_addr[1], attr->ieee_addr[0],
                  attr->device_id, attr->endpoint_id, attr->cluster_id, attr->attr_id, attr->value); */

    // Create a JSON document
    JsonDocument doc;
    JsonObject device = doc["device"].to<JsonObject>();

    // Populate the JSON document
    device["short"] = attr->short_addr;
    char ieee_str[41]; // For converting ieee_addr to string
    sprintf(ieee_str, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
            attr->ieee_addr[7], attr->ieee_addr[6], attr->ieee_addr[5], attr->ieee_addr[4],
            attr->ieee_addr[3], attr->ieee_addr[2], attr->ieee_addr[1], attr->ieee_addr[0]);
    device["ieee"] = ieee_str;
    device["id"] = attr->device_id;
    device["manufacturer"] = attr->manuf;
    device["name"] = attr->name;
    device["type"] = attr->type;
    device["type_id"] = attr->type_id;

    doc["ep_id"] = attr->endpoint_id;
    doc["cluster_id"] = attr->cluster_id;
    doc["attr_id"] = attr->attr_id;
    doc["value_type"] = attr->value_type;
    doc["value"] = attr->value;
    doc["timestamp"] = g_vars_ptr->datetime;

    // Serialize JSON to String
    if (serializeJson(doc, *jsonStr) == 0) {
        esplogE(TAG_LIB_ZIGBEE, "(pack_attr)", "Failed serialise data!");
        doc.clear();
        return false;
    }

    doc.clear();
    // esplogI(TAG_LIB_ZIGBEE, "(pack_attr)", "Successully created attr json. %s", (*jsonStr).c_str());
    return true;
}

bool unpack_attr(iot_alarm_attr_load_t * attr, String jsonStr) {
    if (attr == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(unpack_attr)", "Error: Attr struct is nullptr!");
        return false;
    }

    // Parse the JSON string
    JsonDocument doc; // Adjust size based on expected JSON complexity
    DeserializationError error = deserializeJson(doc, jsonStr);

    // Check for errors
    if (error) {
        esplogW(TAG_LIB_ZIGBEE, "(unpack_attr)", "Failed to parse JSON string! Error: %s", error.c_str());
        doc.clear();
        return false;
    }

    if (!doc["device"].is<JsonObject>()) {
        esplogW(TAG_LIB_ZIGBEE, "(unpack_attr)", "MQTT message is missing device field! Ignoring...");
        doc.clear();
        return false;
    }

    JsonObject device = doc["device"];

    // validate the JSON data (checking for required fields)
    if (!doc["device"]["ieee"].is<const char *>() ||
        !doc["ep_id"].is<uint8_t>() ||
        !doc["cluster_id"].is<uint16_t>() ||
        !doc["attr_id"].is<uint16_t>() ||
        !doc["value_type"].is<uint8_t>() ||
        !doc["value"].is<uint32_t>()) {
            
        esplogW(TAG_LIB_ZIGBEE, "(unpack_attr)", "MQTT message is missing some required fields! Ignoring...");
        device.clear();
        doc.clear();
        return false;
    }

    // Parse JSON into the structure
    if (doc["device"]["short"].is<uint16_t>()) {
        attr->short_addr = device["short"];
    } else {
        attr->short_addr = 0;
    }
    const char *ieee = device["ieee"];
    sscanf(ieee, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
           &attr->ieee_addr[7], &attr->ieee_addr[6], &attr->ieee_addr[5], &attr->ieee_addr[4],
           &attr->ieee_addr[3], &attr->ieee_addr[2], &attr->ieee_addr[1], &attr->ieee_addr[0]);

    attr->endpoint_id = doc["ep_id"];
    attr->cluster_id = doc["cluster_id"];
    attr->attr_id = doc["attr_id"];
    attr->value_type = doc["value_type"];
    attr->value = doc["value"];

    memset(attr->manuf, 0, sizeof(attr->manuf));
    memset(attr->name, 0, sizeof(attr->manuf));
    memset(attr->type, 0, sizeof(attr->manuf));
    attr->type_id = 0;
    attr->device_id = 0;

    doc.clear();
    return true;
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------

/* void serialize_dev(iot_alarm_dev_load_t *dev, uint8_t *buffer, size_t *bytes) {
    if (dev == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(serialize_dev)", "Error: Message is nullptr!");
        return;
    }

    if (buffer == NULL) {
        esplogW(TAG_LIB_ZIGBEE, "(serialize_dev)", "Error: Buffer is nullptr!");
        return;
    }

    int offset = 0;

    memcpy(buffer + offset, &dev->device_id, sizeof(dev->device_id));
    offset += sizeof(dev->device_id);
    memcpy(buffer + offset, &dev->devices_len, sizeof(dev->devices_len));
    offset += sizeof(dev->devices_len);

    // serialization of pointer wont work, have to serialize each endpoint separately and in each endpoint each clusterlist too
    memcpy(buffer + offset, &dev->device, sizeof(dev->device));
    offset += sizeof(dev->device);
}

void serialize_ep() {

}

void serialize_cluster() {

} */