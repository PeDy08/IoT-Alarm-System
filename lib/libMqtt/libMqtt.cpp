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

/* bool mqtt_publish_queue(String topic, String load) {
    mqtt_message_t * msg = (mqtt_message_t *)malloc(sizeof(mqtt_message_t));
    if (msg == NULL) {
        esplogW(TAG_LIB_MQTT, "(mqtt_publish_queue)", "Failed to allocate memory for mqtt message!");
        return false;
    }

    msg->topic = (char *)malloc(topic.length() + 1);
    if (msg->topic == NULL) {
        esplogW(TAG_LIB_MQTT, "(mqtt_publish_queue)", "Failed to allocate memory for topic!");
        free(msg);
        return false;
    }
    strcpy(msg->topic, topic.c_str());

    msg->load = (char *)malloc(load.length() + 1);
    if (msg->load == NULL) {
        esplogW(TAG_LIB_MQTT, "(mqtt_publish_queue)", "Failed to allocate memory for load!");
        free(msg->topic);
        free(msg);
        return false;
    }
    strcpy(msg->load, load.c_str());

    if (xQueueSend(mqttQueue, &msg, pdMS_TO_TICKS(10)) != pdPASS) {
        esplogW(TAG_LIB_MQTT, "(mqtt_publish_queue)", "Failed to send message to publish queue!");
        free(msg->topic);
        free(msg->load);
        free(msg);
        return false;
    } else {
        esplogI(TAG_LIB_MQTT, "(mqtt_publish_queue)", "MQTT message has enqueued! [%s]: %s", msg->topic, msg->load);
    }

    return true;
} */

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

    return ret;
}

