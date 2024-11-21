#include "utils.h"

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
    esplogW("Rebooting...\n");
    delay(2000);
    ESP.restart();
}

bool checkLogFileSize() {
    File logFile = SD.open(LOG_FILE, FILE_READ);
    if (!logFile) {
        esplogW("[fs]: Failed to open log file for size check!\n");
        return false;
    }

    size_t fileSize = logFile.size();
    logFile.close();
    
    if (fileSize >= LOG_FILE_MAX_SIZE) {
        esplogI("[fs]: Log file max size was reached! Cleaning logs!\n");
        SD.rename(LOG_FILE, LOG_FILE_OLD);
        esplogI("[fs]: New log file was created!\n");
    }

    return true;
}

bool esplogI(const char* format, ...) {
    char buf[1024];
    unsigned long timestamp = millis();
    va_list args;

    va_start(args, format);
    vsniprintf(buf, sizeof(buf), format, args);
    va_end(args);

    Serial.print("\033[1;39m");
    Serial.printf("[%lu]%s", timestamp, buf);
    Serial.print("\033[1;39m");

    File logFile = SD.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
        Serial.print("\033[1;31m");
        Serial.printf(" -> failed to log info to file\n");
        Serial.print("\033[1;39m");
        return false;
    }

    logFile.printf("[%lu]%s", timestamp, buf);
    logFile.close();
    return true;
}

bool esplogW(const char* format, ...) {
    char buf[512];
    unsigned long timestamp = millis();
    va_list args;

    va_start(args, format);
    vsniprintf(buf, sizeof(buf), format, args);
    va_end(args);

    Serial.print("\033[1;33m");
    Serial.printf("[%lu]%s", timestamp, buf);
    Serial.print("\033[1;39m");

    File logFile = SD.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
        Serial.print("\033[1;31m");
        Serial.printf(" -> failed to log info to file\n");
        Serial.print("\033[1;39m");
        return false;
    }

    logFile.printf("[%lu]%s", timestamp, buf);
    logFile.close();
    return true;
}

bool esplogE(const char* format, ...) {
    char buf[512];
    unsigned long timestamp = millis();
    va_list args;

    va_start(args, format);
    vsniprintf(buf, sizeof(buf), format, args);
    va_end(args);

    Serial.print("\033[1;31m");
    Serial.printf("[%lu]%s", timestamp, buf);
    Serial.print("\033[1;39m");

    File logFile = SD.open(LOG_FILE, FILE_APPEND);
    if (!logFile) {
        Serial.print("\033[1;31m");
        Serial.printf(" -> failed to log info to file\n");
        Serial.print("\033[1;39m");
        return false;
    }

    logFile.printf("[%lu]%s", timestamp, buf);
    logFile.close();

    rebootESP();
    return true;
}