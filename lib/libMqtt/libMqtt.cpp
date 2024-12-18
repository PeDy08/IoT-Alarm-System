#include "libMqtt.h"

WiFiClient mqttwificlient;
WiFiClientSecure mqttwificlientsecure;
PubSubClient mqtt;

extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;
extern QueueHandle_t mqttQueue;

void mqtt_callback(char* topic, byte* message, unsigned int length) {
    String mqtt_topic = String(topic);
    String mqtt_load;
    for (int i = 0; i < length; i++) {
        mqtt_load += (char)message[i];
    }

    String write_prefix = g_config_ptr->mqtt_topic + String("/write/in");
    String read_prefix  = g_config_ptr->mqtt_topic + String("/read/in");

    if (mqtt_topic.startsWith(write_prefix)) {
        esplogI(TAG_LIB_MQTT, "(mqtt_callback)", "MQTT write command received! (topic: %s, load: %s)", mqtt_topic.c_str(), mqtt_load.c_str());

        esp_zb_ieee_addr_t ieee_addr;
        iot_alarm_attr_load_t* attr = create_attr("\0", "\0", "\0", 0, ieee_addr, 0, 0, 0, 0, 0, ESP_ZB_ZCL_ATTR_TYPE_U8, 0);
        if (unpack_attr(attr, mqtt_load)) {
            esplogI(TAG_LIB_MQTT, "(mqtt_callback)", "MQTT message was unpacked successfully!");
            zigbeeAttrWrite(attr);
        } else {
            esplogW(TAG_LIB_MQTT, "(mqtt_callback)", "Failed to unpack MQTT message!");
        }
        destroy_attr(&attr);
    } else if (mqtt_topic.startsWith(read_prefix)) {
        esplogI(TAG_LIB_MQTT, "(mqtt_callback)", "MQTT read command received! (topic: %s, load: %s)", mqtt_topic.c_str(), mqtt_load.c_str());

        esp_zb_ieee_addr_t ieee_addr;
        iot_alarm_attr_load_t* attr = create_attr("\0", "\0", "\0", 0, ieee_addr, 0, 0, 0, 0, 0, ESP_ZB_ZCL_ATTR_TYPE_U8, 0);
        if (unpack_attr(attr, mqtt_load)) {
            esplogI(TAG_LIB_MQTT, "(mqtt_callback)", "MQTT message was unpacked successfully!");
            zigbeeAttrRead(attr);
        } else {
            esplogW(TAG_LIB_MQTT, "(mqtt_callback)", "Failed to unpack MQTT message!");
        }
        destroy_attr(&attr);
    } else {
        esplogW(TAG_LIB_MQTT, "(mqtt_callback)", "Undefined MQTT message was received! (topic: %s, load: %s)", mqtt_topic.c_str(), mqtt_load.c_str());
    }
}

bool mqtt_publish(String topic, String load) {
    bool ret = false;
    const int maxMessageSize = 200;

    if (mqtt.connected()) {
        esplogI(TAG_LIB_MQTT, "(mqtt_publish)", "Publishing: [%s] \n%s", topic.c_str(), load.c_str());
        if (load.length() > maxMessageSize) {
            int numChunks = load.length() / maxMessageSize + (load.length() % maxMessageSize != 0);

            if (mqtt.beginPublish(topic.c_str(), load.length(), false)) {
                for (int i = 0; i < numChunks; i++) {
                    int startIdx = i * maxMessageSize;
                    int endIdx = (i + 1) * maxMessageSize;
                    String chunk = load.substring(startIdx, endIdx);
                    
                    mqtt.print(chunk);
                    // esplogI(TAG_LIB_MQTT, "(mqtt_publish)", "Published chunk %d of %d", i + 1, numChunks);
                }

                if (mqtt.endPublish()) {
                    esplogI(TAG_LIB_MQTT, "(mqtt_publish)", "MQTT message (splitted) published successfully!");
                    ret = true;
                } else {
                    esplogW(TAG_LIB_MQTT, "(mqtt_publish)", "Error occured during publishing chunks!");
                    ret = false;
                }

            } else {
                esplogW(TAG_LIB_MQTT, "(mqtt_publish)", "Failed to begin publish for the whole message!");
                ret = false;
            }

        } else {
            if (mqtt.publish(topic.c_str(), load.c_str())) {
                esplogI(TAG_LIB_MQTT, "(mqtt_publish)", "MQTT message published successfully!");
                ret = true;
            } else {
                esplogW(TAG_LIB_MQTT, "(mqtt_publish)", "Failed to begin publish for whole message!");
                ret = false;
            }
        }

    } else {
        esplogW(TAG_LIB_MQTT, "(mqtt_publish)", "Tried to publish MQTT message, but client is not connected!");
        ret = false;
    }

    logMqttMessage(load);

    return ret;
}

bool logMqttMessage(String load) {
    time_t rawTime = g_vars_ptr->datetime;
    struct tm * timeInfo = localtime(&rawTime);

    if (rawTime <= 0) {
        esplogW(TAG_LIB_MQTT, "(logMqttMessage)", "MQTT logging failed! Datetime is incorrect!");
        return false;
    }

    char bufferFolder[11];
    char bufferFile[8];
    strftime(bufferFolder, sizeof(bufferFolder), "%Y-%m", timeInfo);
    strftime(bufferFile, sizeof(bufferFile), "%Y-%m-%d", timeInfo);
    String foldername = String(MQTT_LOG_FILES_PATH) + "/" + String(bufferFolder);
    String filename = foldername + "/" + String(bufferFile) + ".json";

    // Create the directory path for the month
    if (!SD.exists(foldername.c_str())) {
        if (!SD.mkdir(foldername.c_str())) {
            esplogW(TAG_LIB_MQTT, "(logMqttMessage)", "MQTT logging failed! Failed to create directory for log!");
            return false;
        } else {
            esplogI(TAG_LIB_MQTT, "(logMqttMessage)", "New folder for MQTT logging has been created successfully!");
        }
    }

    // Create or append to the daily log file
    File logFile;
    if (SD.exists(filename.c_str())) {
        logFile = SD.open(filename.c_str(), FILE_WRITE);
        if (!logFile) {
            esplogW(TAG_LIB_MQTT, "(logMqttMessage)", "MQTT logging failed! Failed to open log file for appending!");
            return false;
        }

        // Remove closing bracket, append the new message, and close JSON
        logFile.seek(logFile.size() - 1); // Go to the end
        logFile.printf(",\n%s\n]", load.c_str());
    } else {
        logFile = SD.open(filename.c_str(), FILE_WRITE);
        if (!logFile) {
            esplogW(TAG_LIB_MQTT, "(logMqttMessage)", "MQTT logging failed! Failed to create log file!");
            return false;
        } else {
            esplogI(TAG_LIB_MQTT, "(logMqttMessage)", "New file for MQTT logging has been created successfully! (%s)", filename.c_str());
        }

        // Write the new JSON array
        logFile.printf("[\n%s\n]", load.c_str());
    }

    logFile.close();
    esplogI(TAG_LIB_MQTT, "(logMqttMessage)", "MQTT message has been logged to SD card successfully! (%s)", filename.c_str());
    return true;
}

bool cleanOldLogs() {
    File logsDir = SD.open(MQTT_LOG_FILES_PATH);
    if (!logsDir || !logsDir.isDirectory()) {
        esplogW(TAG_LIB_MQTT, "(cleanOldLogs)", "Logs directory does not exist!");
        return false;
    }

    time_t rawTime = g_vars_ptr->datetime;
    struct tm * timeInfo = localtime(&rawTime);

    if (rawTime <= 0) {
        esplogW(TAG_LIB_MQTT, "(cleanOldLogs)", "Old logs cleaning failed! Datetime is incorrect!");
        return false;
    }

    timeInfo->tm_mon -= MQTT_LOG_KEEP_MONTHS;
    mktime(timeInfo);
    char cutoffMonth[8];
    strftime(cutoffMonth, sizeof(cutoffMonth), "%Y-%m", timeInfo);
    String cutoffStr = String(cutoffMonth);

    File monthDir;
    while ((monthDir = logsDir.openNextFile())) {
        if (!monthDir.isDirectory()) continue;

        String monthName = String(monthDir.name());
        if (monthName < cutoffStr) { // Compare lexicographically
            esplogI(TAG_LIB_MQTT, "(cleanOldLogs)", "Deleting old logs directory: %s\n", monthName);
            SD.rmdir(monthName);
        }
        monthDir.close();
    }
    logsDir.close();
    esplogI(TAG_LIB_MQTT, "(logMqttMessage)", "MQTT logging storage has been cleared successfully!");
    return true;
}
