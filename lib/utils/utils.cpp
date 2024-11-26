#include "utils.h"

const char *TAG_SETUP               = "\033[1;32mSETUP       ";
const char *TAG_RTOS_MAIN           = "\033[1;32mMAIN        ";
const char *TAG_RTOS_ALARM          = "\033[38;5;202mALARM       ";
const char *TAG_RTOS_KEYPAD         = "\033[38;5;189mKEYPAD      ";
const char *TAG_RTOS_WIFI           = "\033[38;5;225mWIFI        ";
const char *TAG_RTOS_DATETIME       = "\033[38;5;117mDATETIME    ";
const char *TAG_RTOS_RFID           = "\033[38;5;184mRFID        ";
const char *TAG_RTOS_GSM            = "\033[38;5;51mGSM         ";
const char *TAG_RTOS_ZIGBEE         = "\033[38;5;51mZIGBEE      ";
const char *TAG_RTOS_MQTT           = "\033[38;5;51mMQTT        ";

const char *TAG_SERVER              = "\033[38;5;208mSERVER      ";

const char *TAG_LIB_AUTH            = "\033[38;5;250m LIB-AUTH   ";
const char *TAG_LIB_DISPLAY         = "\033[38;5;250m LIB-DISPLAY";
const char *TAG_LIB_GSM             = "\033[38;5;250m LIB-GSM    ";
const char *TAG_LIB_JSON            = "\033[38;5;250m LIB-JSON   ";
const char *TAG_LIB_KEYPAD          = "\033[38;5;250m LIB-KEYPAD ";
const char *TAG_LIB_MQTT            = "\033[38;5;250m LIB-MQTT   ";
const char *TAG_LIB_WIFI            = "\033[38;5;250m LIB-WIFI   ";
const char *TAG_LIB_ZIGBEE          = "\033[38;5;250m LIB-ZIGBEE ";
const char *TAG_LIB_UTILS           = "\033[38;5;250m LIB-UTILS  ";

void cropSelection(int * selection, int selection_max) {
    if (selection_max == 0) {
        *selection = 0;
        return;
    }

    if (*selection >= selection_max) {
        *selection = selection_max-1;
    }

    if (*selection < 0) {
        *selection = 0;
    }
}

void cycleSelection(int * selection, int selection_max) {
    if (*selection >= selection_max) {
        *selection = 0;
    }

    if (*selection < 0) {
        *selection = selection_max-1;
    }
}

void rebootESP() {
    esplogW(TAG_LIB_UTILS, NULL, "Rebooting...");
    delay(2000);
    ESP.restart();
}

bool checkLogFileSize() {
    File logFile = SD.open(LOG_FILE, FILE_READ);
    if (!logFile) {
        esplogW(TAG_LIB_UTILS, NULL, "Failed to open log file for size check!");
        return false;
    }

    size_t fileSize = logFile.size();
    logFile.close();
    
    if (fileSize >= LOG_FILE_MAX_SIZE) {
        esplogI(TAG_LIB_UTILS, NULL, "Log file max size was reached! Cleaning logs!");
        SD.rename(LOG_FILE, LOG_FILE_OLD);
        esplogI(TAG_LIB_UTILS, NULL, "New log file was created!");
    }

    return true;
}

bool esplogI(const char* tag, const char* fx, const char* format, ...) {
    String log;
    unsigned long timestamp = millis();
    va_list args;

    // Format the log message
    char* formattedMessage;
    va_start(args, format);
    if (vasprintf(&formattedMessage, format, args) < 0) {
        va_end(args);
        return false; // Failed to format the message
    }
    va_end(args);

    if (tag != NULL && tag[0] != '\0' && fx != NULL && fx[0] != '\0') {
        Serial.printf("\033[1;32mI [%lu]\033[1;39m %s: %s \033[1;90m(fx: %s)\033[0m\n", timestamp, tag, formattedMessage, fx);
    } else if (tag != NULL && tag[0] != '\0') {
        Serial.printf("\033[1;32mI [%lu]\033[1;39m %s: %s\033[0m\n", timestamp, tag, formattedMessage);
    } else {
        Serial.printf("\033[1;32mI [%lu]\033[1;39m %s\033[0m\n", timestamp, formattedMessage);
    }

    File logFile = SD.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
        Serial.print("\033[1;31m");
        Serial.printf(" -> failed to log info to file");
        Serial.print("\033[1;39m\n");
        free(formattedMessage);
        return false;
    }

    if (tag != NULL && tag[0] != '\0' && fx != NULL && fx[0] != '\0') {
        logFile.printf("\033[1;32mI [%lu]\033[1;39m %s: %s \033[1;90m(fx: %s)\033[0m\n", timestamp, tag, formattedMessage, fx);
    } else if (tag != NULL && tag[0] != '\0') {
        logFile.printf("\033[1;32mI [%lu]\033[1;39m %s: %s\033[0m\n", timestamp, tag, formattedMessage);
    } else {
        logFile.printf("\033[1;32mI [%lu]\033[1;39m %s\033[0m\n", timestamp, formattedMessage);
    }

    free(formattedMessage);
    logFile.close();
    return true;
}

bool esplogW(const char* tag, const char* fx, const char* format, ...) {
    String log;
    unsigned long timestamp = millis();
    va_list args;

    // Format the log message
    char* formattedMessage;
    va_start(args, format);
    if (vasprintf(&formattedMessage, format, args) < 0) {
        va_end(args);
        return false; // Failed to format the message
    }
    va_end(args);

    if (tag != NULL && tag[0] != '\0' && fx != NULL && fx[0] != '\0') {
        Serial.printf("\033[1;33mW [%lu]\033[1;39m %s: %s \033[1;90m(fx: %s)\033[0m\n", timestamp, tag, formattedMessage, fx);
    } else if (tag != NULL && tag[0] != '\0') {
        Serial.printf("\033[1;33mW [%lu]\033[1;39m %s: %s\033[0m\n", timestamp, tag, formattedMessage);
    } else {
        Serial.printf("\033[1;33mW [%lu]\033[1;39m %s\033[0m\n", timestamp, formattedMessage);
    }

    File logFile = SD.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
        Serial.print("\033[1;31m");
        Serial.printf(" -> failed to log info to file");
        Serial.print("\033[1;39m\n");
        free(formattedMessage);
        return false;
    }

    if (tag != NULL && tag[0] != '\0' && fx != NULL && fx[0] != '\0') {
        logFile.printf("\033[1;33mW\033[1;39m [%lu] %s: %s (fx: %s)\033[0m\n", timestamp, tag, formattedMessage, fx);
    } else if (tag != NULL && tag[0] != '\0') {
        logFile.printf("\033[1;33mW\033[1;39m [%lu] %s: %s\033[0m\n", timestamp, tag, formattedMessage);
    } else {
        logFile.printf("\033[1;33mW\033[1;39m [%lu] %s\033[0m\n", timestamp, formattedMessage);
    }

    free(formattedMessage);
    logFile.close();
    return true;
}

bool esplogE(const char* tag, const char* fx, const char* format, ...) {
    String log;
    unsigned long timestamp = millis();
    va_list args;

    // Format the log message
    char* formattedMessage;
    va_start(args, format);
    if (vasprintf(&formattedMessage, format, args) < 0) {
        va_end(args);
        return false; // Failed to format the message
    }
    va_end(args);

    if (tag != NULL && tag[0] != '\0' && fx != NULL && fx[0] != '\0') {
        Serial.printf("\033[1;31mE\033[1;39m [%lu] %s: %s (fx: %s)\033[0m\n", timestamp, tag, formattedMessage, fx);
    } else if (tag != NULL && tag[0] != '\0') {
        Serial.printf("\033[1;31mE\033[1;39m [%lu] %s: %s\033[0m\n", timestamp, tag, formattedMessage);
    } else {
        Serial.printf("\033[1;31mE\033[1;39m [%lu] %s\033[0m\n", timestamp, formattedMessage);
    }

    File logFile = SD.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
        Serial.print("\033[1;31m");
        Serial.printf(" -> failed to log info to file");
        Serial.print("\033[1;39m\n");
        free(formattedMessage);
        return false;
    }

    if (tag != NULL && tag[0] != '\0' && fx != NULL && fx[0] != '\0') {
        logFile.printf("\033[1;31mE\033[1;39m [%lu] %s %s (fx: %s)\033[0m\n", tag, formattedMessage, fx);
    } else if (tag != NULL && tag[0] != '\0') {
        logFile.printf("\033[1;31mE\033[1;39m [%lu] %s %s\033[0m\n", tag, formattedMessage);
    } else {
        logFile.printf("\033[1;31mE\033[1;39m [%lu] %s\033[0m\n", formattedMessage);
    }

    free(formattedMessage);
    logFile.close();
    rebootESP();
    return true;
}