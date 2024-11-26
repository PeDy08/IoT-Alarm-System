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

#include "utils.h"
#include "libZigbee.h"
#include "mainAppDefinitions.h"

extern WiFiClient mqttwificlient;
extern WiFiClientSecure mqttwificlientsecure;
extern PubSubClient mqtt;

/* typedef struct {
    char* topic;
    char* load;
} mqtt_message_t; */

void mqtt_callback(char* topic, byte* message, unsigned int length);
// bool mqtt_publish_queue(String topic, String load);
bool mqtt_publish(String topic, String load);

#endif