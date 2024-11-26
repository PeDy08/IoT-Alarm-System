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
#include "libAuth.h"
#include "mainAppDefinitions.h"

#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_ADDR 0x27

enum notificationScreenId {
    NOTIFICATION_AUTH_CONFIRM,
    NOTIFICATION_AUTH_ERROR,
    NOTIFICATION_AUTH_SET,
    NOTIFICATION_RFID_CONFIRM,
    NOTIFICATION_RFID_ERROR,
    NOTIFICATION_RFID_ADD,
    NOTIFICATION_RFID_DELETE,
    NOTIFICATION_ZIGBEE_OPEN,
    NOTIFICATION_ZIGBEE_CLOSE,
    NOTIFICATION_ZIGBEE_CLEAR,
    NOTIFICATION_ZIGBEE_RESET,
    NOTIFICATION_ZIGBEE_REPORT,
    NOTIFICATION_ZIGBEE_DEV_ANNCE,
    NOTIFICATION_ZIGBEE_DEV_LEAVE,
};

enum updateScreenParam {
    UPDATE_SELECTION,
    UPDATE_DATETIME,
    UPDATE_STATUS,
    UPDATE_PIN,
    UPDATE_ATTEMPTS,
    UPDATE_ALARM_STATUS,
    UPDATE_EVENTS,
    UPDATE_COUNTDOWN,
};

typedef struct {
    notificationScreenId id;
} notification_t;

void initEink();

void authScreenC(g_vars_t * g_vars);
void authScreenE(g_vars_t * g_vars);
void authScreenS(g_vars_t * g_vars);
void rfidScreenC(g_vars_t * g_vars, const char * uid);
void rfidScreenE(g_vars_t * g_vars, const char * uid);
void rfidScreenA(g_vars_t * g_vars, const char * uid);
void rfidScreenD(g_vars_t * g_vars, const char * uid);
void zigbeeScreenO(g_vars_t * g_vars, g_config_t * g_config, uint8_t duration);
void zigbeeScreenC(g_vars_t * g_vars, g_config_t * g_config);
void zigbeeScreenD(g_vars_t * g_vars, g_config_t * g_config);
void zigbeeScreenR(g_vars_t * g_vars, g_config_t * g_config);
void zigbeeScreenN(g_vars_t * g_vars, g_config_t * g_config, uint8_t device_count);

void updateScreen(g_vars_t * g_vars, g_config_t * g_config, int param);
void loadScreen(g_vars_t * g_vars, g_config_t * g_config, bool reboot = false);

void initScreenTemplate(const char * label);
void menuScreenTemplate(const char * label, int selection, bool test, const char * option1, const char * option2, const char * option3, const char * option4, const char * time, const char * date, int wifi, int gsm, int battery);
void authScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, String pin, int attempts, const char * time, const char * date, int wifi, int gsm, int battery);
void rfidScreenTemplate(const char * label, bool test, const char * instructions1, const char * instructions2, int attempts, const char * time, const char * date, int wifi, int gsm, int battery);
void alarmScreenTemplate(const char * label, bool test, const char * status, const char * data, String pin, int attempts, int data_load, const char * time, const char * date, int wifi, int gsm, int battery);
void notificationScreenTemplate(const char * label, const char * data);

#endif