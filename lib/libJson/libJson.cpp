#include "libJson.h"

void setDefaultConfig(g_config_t *g_config) {
    esplogI(" (libJson): Reseting configuration data to defaults.\n");
    g_config->wifi_ssid = "\0";
    g_config->wifi_pswd = "\0";
    g_config->wifi_ip = "\0";
    g_config->wifi_gtw = "\0";
    g_config->wifi_sbnt = "\0";
    g_config->alarm_countdown_s = 120;
}

bool saveConfig(g_config_t *g_config) {
    esplogI(" (libJson): Saving configuration data to config file...\n");
    if (SD.exists(CONFIG_FILE)) {
        esplogW(" (libJson): Config file found, rewriting!\n");
        if (!SD.remove(CONFIG_FILE)) {
            esplogE(" (libJson): Failed to rewrite existing file: %s!\n", CONFIG_FILE);
            return false;
        }
    }

    File configFile = SD.open(CONFIG_FILE, "w");
    if (!configFile) {
        esplogE(" (libJson): Failed to open config file: %s when writing! Unexpected error!\n", CONFIG_FILE);
        return false;
    }

    JsonDocument doc;
    doc["ssid"] = g_config->wifi_ssid;
    doc["password"] = g_config->wifi_pswd;
    doc["ip"] = g_config->wifi_ip;
    doc["gateway"] = g_config->wifi_gtw;
    doc["subnet"] = g_config->wifi_sbnt;
    doc["alarm_countdown"] = g_config->alarm_countdown_s;

    if (serializeJson(doc, configFile) == 0) {
        esplogE(" (libJson): Failed serialise data to file!\n");
        doc.clear();
        configFile.close();
        return false;
    }

    doc.clear();
    configFile.close();
    esplogI(" (libJson): Successully saved.\n");
    return true;
}

bool saveConfigFromJSON(g_config_t *g_config) {
    esplogI(" (libJson): Saving configuration data from upload config file to config file...\n");

    if (!loadConfig(g_config, CONFIG_UPLOAD_FILE)) {
        esplogW(" (libJson): Failed to load configuration data from received file!\n");
        return false;
    }
    esplogI(" (libJson): Successfully loaded configuration data from received file!\n");

    if (!saveConfig(g_config)) {
        esplogW(" (libJson): Failed to save configuration data from received file!\n");
        return false;
    }
    esplogI(" (libJson): Successfully saved configuration data from received file!\n");

    return true;
}

bool loadConfig(g_config_t * g_config, const char* filepath) {
    esplogI(" (libJson): Loading configuration data from config file...\n");

    if (filepath == CONFIG_FILE) {
        if (!SD.exists(CONFIG_FILE)) {
            esplogI(" (libJson): Config file not found, creating new file! New file: %s\n", CONFIG_FILE);
            setDefaultConfig(g_config);
            if (!saveConfig(g_config)) {
                esplogE(" (libJson): Failed to save configuration file!\n");
                return false;
            }
            return true;
        }
    } else {
        if (!SD.exists(filepath)) {
            esplogW(" (libJson): File: '%s' not found, stopping the loading process!\n", filepath);
            setDefaultConfig(g_config);
            if (!saveConfig(g_config)) {
                esplogE(" (libJson): Failed to save configuration file!\n");
                return false;
            }
            return true;
        }
    }

    File configFile = SD.open(filepath, "r");
    if (!configFile) {
        esplogW(" (libJson): Failed to open config file: %s! Removing this file and reseting the configuration.\n", filepath);
        configFile.close();
        SD.remove(filepath);
        setDefaultConfig(g_config);
        if (!saveConfig(g_config)) {
            esplogE(" (libJson): Failed to save configuration file!\n");
            return false;
        }
        return true;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        esplogW(" (libJson): Failed to parse config file %s! Re-creating this file. Error: %s\n", filepath, error.c_str());
        configFile.close();
        SD.remove(filepath);
        setDefaultConfig(g_config);
        if (!saveConfig(g_config)) {
            esplogE(" (libJson): Failed to save configuration file!\n");
            return false;
        }
        return true;
    }

    // validate the JSON data (checking for required fields)
    if (!doc["ssid"].is<String>() ||
        !doc["password"].is<String>() ||
        !doc["ip"].is<String>() ||
        !doc["gateway"].is<String>() ||
        !doc["subnet"].is<String>() ||
        !doc["alarm_countdown"].is<int>()) {
            
        esplogW(" (libJson): Uploaded config file is missing some required fields! Reseting the configuration!\n");
        configFile.close();
        SD.remove(filepath);
        setDefaultConfig(g_config);
        if (!saveConfig(g_config)) {
            esplogE(" (libJson): Failed to save configuration file!\n");
            return false;
        }
        return true;
    }

    // set configuration
    g_config->wifi_ssid = doc["ssid"].as<String>();
    g_config->wifi_pswd = doc["password"].as<String>();
    g_config->wifi_ip = doc["ip"].as<String>();
    g_config->wifi_gtw = doc["gateway"].as<String>();
    g_config->wifi_sbnt = doc["subnet"].as<String>();
    g_config->alarm_countdown_s = doc["alarm_countdown"].as<int>();
    

    doc.clear();
    configFile.close();

    esplogI(" (libJson): Successully loaded.\n");
    return true;
}