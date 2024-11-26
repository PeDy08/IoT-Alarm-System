/**
 * @file libDisplay.h
 * @brief Contains functions and definitions for controlling display.
 * 
 * Contains functions and definitions for controlling display.
 */

#ifndef LIBDISPLAY_H_DEFINITION
#define LIBDISPLAY_H_DEFINITION

#include <Arduino.h>
#include <WiFi.h>

#include "utils.h"
#include "mainAppDefinitions.h"

#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_ADDR 0x27

enum notificationScreenId {
    NOTIFICATION_NONE,
    NOTIFICATION_AUTH_CHECK_SUCCESS,
    NOTIFICATION_AUTH_CHECK_ERROR,
    NOTIFICATION_AUTH_SET_SUCCESS,
    NOTIFICATION_AUTH_SET_ERROR,
    NOTIFICATION_RFID_CHECK_SUCCESS,
    NOTIFICATION_RFID_CHECK_ERROR,
    NOTIFICATION_RFID_ADD_SUCCESS,
    NOTIFICATION_RFID_ADD_ERROR,
    NOTIFICATION_RFID_DEL_SUCCESS,
    NOTIFICATION_RFID_DEL_ERROR,
    NOTIFICATION_ZIGBEE_NET_OPEN,
    NOTIFICATION_ZIGBEE_NET_CLOSE,
    NOTIFICATION_ZIGBEE_NET_CLEAR,
    NOTIFICATION_ZIGBEE_NET_RESET,
    NOTIFICATION_ZIGBEE_ATTR_REPORT,
    NOTIFICATION_ZIGBEE_DEV_ANNCE,
    NOTIFICATION_ZIGBEE_DEV_LEAVE,
    NOTIFICATION_ZIGBEE_DEV_COUNT,
    NOTIFICATION_MAX,
};

enum updateScreenParam {
    UPDATE_NONE,
    UPDATE_SELECTION,
    UPDATE_DATETIME,
    UPDATE_STATUS,
    UPDATE_PIN,
    UPDATE_ATTEMPTS,
    UPDATE_ALARM_STATUS,
    UPDATE_EVENTS,
    UPDATE_COUNTDOWN,
    UPDATE_MAX,
};

typedef struct {
    notificationScreenId id;
    int param;
    int duration;
} notification_t;

// display initialisation function
void initEink();

// show on display restarting message
void displayRestart();

// show on display actual content of the running application
void displayLoad();

// show on display pop-up notification
void displayNotificationHandler(notificationScreenId notification, int param = 0);

// enqueue display pop-up notification
void displayNotification(notificationScreenId notification, int param = 0, int duration = 0);

// function for custom notification on display
void notificationScreenTemplate(const char * label, const char * data);

#endif