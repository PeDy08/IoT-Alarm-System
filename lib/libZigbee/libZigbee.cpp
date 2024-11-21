#include "libZigbee.h"

HardwareSerial SerialZigbee(2);

void updateSerialZigbee() {
    while (Serial.available()) {
        SerialZigbee.write(Serial.read());
    }

    while (SerialZigbee.available()) {
        Serial.write(SerialZigbee.read());
    }
}

/**
 * @brief Sends command to the zigbee module and waits for the expected response.
 * 
 * This function repeatedly sends command to the zigbee module and checks if the
 * expected response is received. If the response contains the expected string, it
 * returns true; otherwise, it will keep trying until a timeout occurs.
 * 
 * @param sendMsg The iot_alarm_message_t command to send to the zigbee module.
 * @param receivedMsg Pointer to iot_alarm_message_t that will store the received response.
 * @param timeout The time (in milliseconds) to wait for the response (default is 6000 ms).
 * @return true if the expected response was received, false if the timeout occurred.
 */
bool waitForCorrectResponseZigbee(iot_alarm_message_t * sendMsg, iot_alarm_message_t * receivedMsg, unsigned long timeout = 5000) {
    if (sendMsg == NULL || receivedMsg == NULL) {
        esplogW(" (libZigbee): Message to be sent or received is nullptr!\n");
        return false;
    }

    while (SerialZigbee.available()) {
        SerialZigbee.read();
    }

    unsigned long startTime = millis();
    int tx_bytes;
    int rx_bytes;
    while (millis() - startTime < timeout) {
        // send command
        tx_bytes = send_message(UART, tx_buffer, sendMsg);

        vTaskDelay(500 / portTICK_PERIOD_MS);

        // receive command
        rx_bytes = receive_message(UART, rx_buffer, &receivedMsg, RX_BUF_SIZE-1);

        // check if acknowledge is received
        if (((sendMsg->dir == IOT_ALARM_MSGDIR_COMMAND && receivedMsg->dir == IOT_ALARM_MSGDIR_COMMAND_ACK) ||
                (sendMsg->dir == IOT_ALARM_MSGDIR_NOTIFICATION && receivedMsg->dir == IOT_ALARM_MSGDIR_NOTIFICATION_ACK)) &&
                sendMsg->id == receivedMsg->id && receivedMsg->st == IOT_ALARM_MSGSTATUS_SUCCESS) {
            return true;
        }
    }

    esplogW(" (libZigbee): Zigbee module didnt responded in time!\n");
    return false;
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

    Serial.println("-------------------------------------------\nZIGBEE INITIALISATION:\n");

    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ECHO, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");
    int ret = true;

    // initialisation
    if (!ret || !waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed finding zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Zigbee module found!\n");
    }

    destroy_message(&msg);
    destroy_message(&ack);

    if (!ret) {
        Serial.println("\nINITIALISATION FAILED\n-------------------------------------------\n");
    } else {
        Serial.println("\nSUCCESSFULLY INITIALISED\n-------------------------------------------\n");
    }

    return ret;
}

bool zigbeeReset() {
    int ret = true;
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_CTL_RESTART, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    if (!waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed sending message to zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Reset command sent to zigbee module!\n");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeFactory() {
    int ret = true;
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_CTL_FACTORY, 1, "\0");
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    if (!waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed sending message to zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Factory reset command sent to zigbee module!\n");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeOpen(uint8_t duration = 180) {
    int ret = true;
    char load[12];
    sprintf(load, "%d", duration);
    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DEV_UNLOCK, 4, load);
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    if (!waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed sending message to zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Open zigbee network command sent to zigbee module!\n");
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
    if (!waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed sending message to zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Close zigbee network command sent to zigbee module!\n");
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
    if (!waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed sending message to zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Clear zigbee network command sent to zigbee module!\n");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeAttrRead(esp_zb_ieee_addr_t ieee_addr, uint16_t short_addr, uint8_t endpoint_id, uint16_t cluster_id, uint16_t attr_id, esp_zb_zcl_attr_type_t value_type, uint32_t value) {
    int ret = true;
    size_t length;
    char serialized_load[1024];

    iot_alarm_attr_load_t * attr = create_attr("\0", "\0", "\0", 0, ieee_addr, short_addr, 0, endpoint_id, cluster_id, attr_id, value_type, value);
    memset(serialized_load, 0, sizeof(serialized_load));
    serialize_attr(attr, serialized_load, &length);

    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DATA_READ, (uint32_t)length, serialized_load);
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    if (!waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed sending message to zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Read attribute command sent to zigbee module!\n");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    return ret;
}

bool zigbeeAttrWrite(esp_zb_ieee_addr_t ieee_addr, uint16_t short_addr, uint8_t endpoint_id, uint16_t cluster_id, uint16_t attr_id, esp_zb_zcl_attr_type_t value_type, uint32_t value) {
    int ret = true;
    size_t length;
    char serialized_load[1024];

    iot_alarm_attr_load_t * attr = create_attr("\0", "\0", "\0", 0, ieee_addr, short_addr, 0, endpoint_id, cluster_id, attr_id, value_type, value);
    memset(serialized_load, 0, sizeof(serialized_load));
    serialize_attr(attr, serialized_load, &length);

    iot_alarm_message_t * msg = create_message(IOT_ALARM_MSGDIR_COMMAND, IOT_ALARM_MSGSTATUS_SUCCESS, IOT_ALARM_MSGTYPE_ZB_DATA_WRITE, (uint32_t)length, serialized_load);
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");

    // sending the message
    if (!waitForCorrectResponseZigbee(msg, ack, 10000)) {
        esplogW(" (libZigbee): Failed sending message to zigbee module!\n");
        ret = false;
    } else {
        esplogI(" (libZigbee): Write attribute command sent to zigbee module!\n");
    }

    destroy_message(&msg);
    destroy_message(&ack);
    destroy_attr(&attr);
    return ret;
}


// *********************************************************************************************************************

const int RX_BUF_SIZE = 1024;
const int TX_BUF_SIZE = 1024;

uint8_t* tx_buffer = NULL;
uint8_t* rx_buffer = NULL;

void serialize_message(iot_alarm_message_t *msg, uint8_t *buffer, size_t *bytes) {
    
    if (msg == NULL) {
        esplogW(" (libZigbee): Error: Message is nullptr!\n");
        return;
    }
    
    if (buffer == NULL) {
        esplogW(" (libZigbee): Error: Buffer is nullptr!\n");
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

void deserialize_message(iot_alarm_message_t **msg, uint8_t *buffer) {

    if (msg == NULL) {
        esplogW(" (libZigbee): Error: Message is nullptr!\n");
        return;
    }

    if (*msg == NULL) {
        esplogW(" (libZigbee): Error: Message is nullptr!\n");
        return;
    }

    if (buffer == NULL) {
        esplogW(" (libZigbee): Error: Buffer is nullptr!\n");
        return;
    }

    int offset = 0;

    message_direction_t dir;
    message_status_t st;
    message_type_t id;
    uint32_t length;

    // Extract message direction
    memcpy(&dir, buffer + offset, sizeof(dir));
    offset += sizeof(dir);

    // Extract message status
    memcpy(&st, buffer + offset, sizeof(st));
    offset += sizeof(st);

    // Extract message ID
    memcpy(&id, buffer + offset, sizeof(id));
    offset += sizeof(id);

    // Extract length of load
    memcpy(&length, buffer + offset, sizeof(length));
    offset += sizeof(length);

    // Extract load itself
    char* load = (char *)malloc(length + 1);
    if (load == NULL) {
        esplogW(" (libZigbee): Failed to allocate memory for message load!\n");
        return;
    }

    if (length > 0) {
        memcpy(load, buffer + offset, length);
    }

    load[length] = '\0';

    destroy_message(msg);
    *msg = create_message(dir, st, id, length, load);

    free(load);
}

iot_alarm_message_t * create_message(message_direction_t dir, message_status_t st, message_type_t id, uint32_t length, const char* load) {

    iot_alarm_message_t *msg = (iot_alarm_message_t *)malloc(sizeof(iot_alarm_message_t));
    if (msg == NULL) {
        esplogW(" (libZigbee): Failed to allocate memory for message!\n");
        return NULL;
    }

    msg->dir = dir;
    msg->id = id;
    msg->st = st;
    msg->length = length;
    msg->load = (char *)malloc(length + 1);
    if (msg->load == NULL) {
        esplogW(" (libZigbee): Failed to allocate memory for message load!\n");
        free(msg);
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
        esplogI(" (libZigbee): Message (length: %d) was sent successfully!\n", tx_bytes);
    } else {
        esplogW(" (libZigbee): Failed to send message!\n");
    }

    return tx_bytes;
}

int receive_message(uart_port_t uart, uint8_t* rx_buffer, iot_alarm_message_t **msg, size_t max_len) {
    memset(rx_buffer, 0, max_len);
    int rx_bytes = read_uart(uart, rx_buffer, max_len);

    /* char hex_string[rx_bytes * 3 + 1];
    for (size_t i = 0; i < rx_bytes; i++) {
        snprintf(&hex_string[i * 3], 4, "%02X ", rx_buffer[i]);
    }
    esplogI("Buffer (length %d): %s\n", rx_bytes, hex_string); // */

    if (rx_bytes < max_len) {
        rx_buffer[rx_bytes] = 0;
    } else {
        rx_buffer[max_len - 1] = 0;
        esplogW(" (libZigbee): RX buffer overflow. Data may be truncated.\n");
    }
    
    if (rx_bytes > 0) {
        deserialize_message(msg, rx_buffer);
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

    iot_alarm_message_t msg = {
        .dir = dir,
        .st = IOT_ALARM_MSGSTATUS_SUCCESS,
        .id = id,
        .length = (uint16_t)length,
        .load = serialized_load,
    };

    // clear uart
    size_t available = 0;
    uint8_t temp_buffer[256];
    uart_get_buffered_data_len(uart, &available);
    while (available) {
        uart_read_bytes(uart, temp_buffer, available, pdMS_TO_TICKS(500));
        uart_get_buffered_data_len(uart, &available);
    }

    unsigned long startTime = esp_timer_get_time() / 1000;
    iot_alarm_message_t * ack = create_message(IOT_ALARM_MSGDIR_MAX, IOT_ALARM_MSGSTATUS_MAX, IOT_ALARM_MSGTYPE_MAX, 1, "\0");
    int tx_bytes = 0;
    int rx_bytes = 0;
    bool ret = false;

    // send message and wait for acknowledge
    while (esp_timer_get_time() / 1000 - startTime < 5000) {
        tx_bytes = send_message(uart, tx_buffer, &msg);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        rx_bytes = receive_message(uart, temp_buffer, &ack, 255);

        if (((msg.dir == IOT_ALARM_MSGDIR_COMMAND && ack->dir == IOT_ALARM_MSGDIR_COMMAND_ACK) ||
             (msg.dir == IOT_ALARM_MSGDIR_NOTIFICATION && ack->dir == IOT_ALARM_MSGDIR_NOTIFICATION_ACK)) &&
              msg.id == ack->id && ack->st == IOT_ALARM_MSGSTATUS_SUCCESS) {
            ret = true;
            break;
        }
    }

    destroy_message(&ack);

    if (!ret) {
        esplogW(" (libZigbee): Failed to get acknowlede for sending attribute data!");
    }

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
        esplogW(" (libZigbee): Error: Message is nullptr!\n");
        return;
    }
    
    if (buffer == NULL) {
        esplogW(" (libZigbee): Error: Buffer is nullptr!\n");
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

void deserialize_attr(iot_alarm_attr_load_t **attr, uint8_t *buffer) {
    if (attr == NULL) {
        esplogW(" (libZigbee): Error: Message is nullptr!\n");
        return;
    }

    if (*attr == NULL) {
        esplogW(" (libZigbee): Error: Message is nullptr!\n");
        return;
    }

    if (buffer == NULL) {
        esplogW(" (libZigbee): Error: Buffer is nullptr!\n");
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

    memcpy(&ieee_addr, buffer + offset, sizeof(ieee_addr));
    offset += sizeof(ieee_addr);
    memcpy(&short_addr, buffer + offset, sizeof(short_addr));
    offset += sizeof(short_addr);

    memcpy(&device_id, buffer + offset, sizeof(device_id));
    offset += sizeof(device_id);
    memcpy(&endpoint_id, buffer + offset, sizeof(endpoint_id));
    offset += sizeof(endpoint_id);
    memcpy(&cluster_id, buffer + offset, sizeof(cluster_id));
    offset += sizeof(cluster_id);
    memcpy(&attr_id, buffer + offset, sizeof(attr_id));
    offset += sizeof(attr_id);

    memcpy(&value_type, buffer + offset, sizeof(value_type));
    offset += sizeof(value_type);

    // debug prints
    // esplogI("DESERIALISE: manufacturer: %s\n", manuf);
    // esplogI("DESERIALISE: name: %s\n", name);
    // esplogI("DESERIALISE: type: %s\n", type);
    // esplogI("DESERIALISE: type_id: %lu\n", type_id);
    // esplogI("DESERIALISE: ieee_addr: %s\n", ieee_addr);
    // esplogI("DESERIALISE: short_addr: %d\n", short_addr);
    // esplogI("DESERIALISE: device_id: %d\n", device_id);
    // esplogI("DESERIALISE: endpoint_id: %d\n", endpoint_id);
    // esplogI("DESERIALISE: cluster_id: %d\n", cluster_id);
    // esplogI("DESERIALISE: attr_id: %d\n", attr_id);
    // esplogI("DESERIALISE: value_type: %d\n", value_type);

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
            memcpy(&value, buffer + offset, sizeof(value));
            offset += sizeof(value);
            // esplogI("DESERIALISE: value: %lu\n", value);
            break;

        default:
            break;
    }

    memcpy(&type_id, buffer + offset, sizeof(type_id));
    offset += sizeof(type_id);
    memcpy(&type, buffer + offset, sizeof(type));
    offset += sizeof(type);
    memcpy(&manuf, buffer + offset, sizeof(manuf));
    offset += sizeof(manuf);
    memcpy(&name, buffer + offset, sizeof(name));
    offset += sizeof(name);

    /* char hex_string[offset * 3 + 1];
    for (size_t i = 0; i < offset; i++) {
        snprintf(&hex_string[i * 3], 4, "%02X ", buffer[i]);
    }
    esplogI("Buffer (length %d): %s\n", offset, hex_string); // */

    destroy_attr(attr);
    *attr = create_attr(manuf, name, type, type_id, ieee_addr, short_addr, device_id, endpoint_id, cluster_id, attr_id, value_type, value);
}

iot_alarm_attr_load_t * create_attr(const char * manuf,const  char * name,const  char * type, uint32_t type_id, esp_zb_ieee_addr_t ieee_addr, uint16_t short_addr, uint8_t device_id, uint8_t endpoint_id, uint16_t cluster_id, uint16_t attr_id, esp_zb_zcl_attr_type_t value_type, uint32_t value) {

    iot_alarm_attr_load_t *attr = (iot_alarm_attr_load_t *)malloc(sizeof(iot_alarm_attr_load_t));
    if (attr == NULL) {
        esplogW(" (libZigbee): Failed to allocate memory for attr!\n");
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

// ----------------------------------------------------------------------------------------------------------------------------------------------------

/* void serialize_dev(iot_alarm_dev_load_t *dev, uint8_t *buffer, size_t *bytes) {
    if (dev == NULL) {
        esplogW(" (libZigbee): Error: Message is nullptr!\n");
        return;
    }

    if (buffer == NULL) {
        esplogW(" (libZigbee): Error: Buffer is nullptr!\n");
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