#include "libJson.h"

void setDefaultConfig(g_config_t *g_config) {
    esplogI(TAG_LIB_JSON, "(setDefaultConfig)", "Reseting configuration data to defaults.");
    g_config->wifi_ssid = "\0";
    g_config->wifi_pswd = "\0";
    g_config->wifi_ip = "\0";
    g_config->wifi_gtw = "\0";
    g_config->wifi_sbnt = "\0";

    g_config->mqtt_tls = true;
    g_config->mqtt_broker = "\0";
    g_config->mqtt_port = 1883;
    g_config->mqtt_id = "IoT_Alarm";
    g_config->mqtt_topic = "IoT_Alarm";
    g_config->mqtt_username = "\0";
    g_config->mqtt_password = "\0";
    g_config->mqtt_cert = "\0";

    g_config->alarm_countdown_s = 120;
    g_config->alarm_e_countdown_s = 120;
    g_config->alarm_w_threshold = 5;
    g_config->alarm_e_threshold = 7;

    g_config->alarm_telephone = "\0";
}

bool saveConfig(g_config_t *g_config) {
    esplogI(TAG_LIB_JSON, "(saveConfig)", "Saving configuration data to config file...");
    if (SD.exists(CONFIG_FILE)) {
        esplogW(TAG_LIB_JSON, "(saveConfig)", "Config file found, rewriting!");
        if (!SD.remove(CONFIG_FILE)) {
            esplogE(TAG_LIB_JSON, "(saveConfig)", "Failed to rewrite existing file: %s!", CONFIG_FILE);
            return false;
        }
    }

    File configFile = SD.open(CONFIG_FILE, "w");
    if (!configFile) {
        esplogE(TAG_LIB_JSON, "(saveConfig)", "Failed to open config file: %s when writing! Unexpected error!", CONFIG_FILE);
        return false;
    }

    JsonDocument doc;
    doc["ssid"] = g_config->wifi_ssid;
    doc["password"] = g_config->wifi_pswd;
    doc["ip"] = g_config->wifi_ip;
    doc["gateway"] = g_config->wifi_gtw;
    doc["subnet"] = g_config->wifi_sbnt;

    doc["mqtt_tls"] = g_config->mqtt_tls;
    doc["mqtt_broker"] = g_config->mqtt_broker;
    doc["mqtt_port"] = g_config->mqtt_port;
    doc["mqtt_id"] = g_config->mqtt_id;
    doc["mqtt_topic"] = g_config->mqtt_topic;
    doc["mqtt_username"] = g_config->mqtt_username;
    doc["mqtt_password"] = g_config->mqtt_password;
    doc["mqtt_cert"] = g_config->mqtt_cert;

    doc["alarm_countdown"] = g_config->alarm_countdown_s;
    doc["alarm_countdown_e"] = g_config->alarm_e_countdown_s;
    doc["alarm_threshold_w"] = g_config->alarm_w_threshold;
    doc["alarm_threshold_e"] = g_config->alarm_e_threshold;

    doc["alarm_telephone"] = g_config->alarm_telephone;

    if (serializeJson(doc, configFile) == 0) {
        esplogE(TAG_LIB_JSON, "(saveConfig)", "Failed serialise data!");
        doc.clear();
        configFile.close();
        return false;
    }

    doc.clear();
    configFile.close();
    esplogI(TAG_LIB_JSON, "(saveConfig)", "Successully saved.");
    return true;
}

bool saveConfigFromJSON(g_config_t *g_config) {
    esplogI(TAG_LIB_JSON, "(saveConfigFromJSON)", "Saving configuration data from upload config file to config file...");

    if (!loadConfig(g_config, CONFIG_UPLOAD_FILE)) {
        esplogW(TAG_LIB_JSON, "(saveConfigFromJSON)", "Failed to load configuration data from received file!");
        return false;
    }
    esplogI(TAG_LIB_JSON, "(saveConfigFromJSON)", "Successfully loaded configuration data from received file!");

    if (!saveConfig(g_config)) {
        esplogW(TAG_LIB_JSON, "(saveConfigFromJSON)", "Failed to save configuration data from received file!");
        return false;
    }
    esplogI(TAG_LIB_JSON, "(saveConfigFromJSON)", "Successfully saved configuration data from received file!");

    return true;
}

bool loadConfig(g_config_t * g_config, const char* filepath) {
    esplogI(TAG_LIB_JSON, "(loadConfig)", "Loading configuration data from config file...");

    if (filepath == CONFIG_FILE) {
        if (!SD.exists(CONFIG_FILE)) {
            esplogI(TAG_LIB_JSON, "(loadConfig)", "Config file not found, creating new file! New file: %s", CONFIG_FILE);
            setDefaultConfig(g_config);
            if (!saveConfig(g_config)) {
                esplogE(TAG_LIB_JSON, "(loadConfig)", "Failed to save configuration file!");
                return false;
            }
            return true;
        }
    } else {
        if (!SD.exists(filepath)) {
            esplogW(TAG_LIB_JSON, "(loadConfig)", "File: '%s' not found, stopping the loading process!", filepath);
            setDefaultConfig(g_config);
            if (!saveConfig(g_config)) {
                esplogE(TAG_LIB_JSON, "(loadConfig)", "Failed to save configuration file!");
                return false;
            }
            return true;
        }
    }

    File configFile = SD.open(filepath, "r");
    if (!configFile) {
        esplogW(TAG_LIB_JSON, "(loadConfig)", "Failed to open config file: %s! Removing this file and reseting the configuration.", filepath);
        configFile.close();
        SD.remove(filepath);
        setDefaultConfig(g_config);
        if (!saveConfig(g_config)) {
            esplogE(TAG_LIB_JSON, "(loadConfig)", "Failed to save configuration file!");
            return false;
        }
        return true;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        esplogW(TAG_LIB_JSON, "(loadConfig)", "Failed to parse config file %s! Re-creating this file. Error: %s", filepath, error.c_str());
        configFile.close();
        SD.remove(filepath);
        setDefaultConfig(g_config);
        if (!saveConfig(g_config)) {
            esplogE(TAG_LIB_JSON, "(loadConfig)", "Failed to save configuration file!");
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
        !doc["mqtt_tls"].is<bool>() ||
        !doc["mqtt_broker"].is<String>() ||
        !doc["mqtt_port"].is<uint16_t>() ||
        !doc["mqtt_id"].is<String>() ||
        !doc["mqtt_topic"].is<String>() ||
        !doc["mqtt_username"].is<String>() ||
        !doc["mqtt_password"].is<String>() ||
        !doc["mqtt_cert"].is<String>() ||
        !doc["alarm_countdown"].is<int>() ||
        !doc["alarm_countdown_e"].is<int>() ||
        !doc["alarm_threshold_w"].is<int>() ||
        !doc["alarm_threshold_e"].is<int>() ||
        !doc["alarm_telephone"].is<String>()) {
            
        esplogW(TAG_LIB_JSON, "(loadConfig)", "Uploaded config file is missing some required fields! Reseting the configuration!");
        configFile.close();
        SD.remove(filepath);
        setDefaultConfig(g_config);
        if (!saveConfig(g_config)) {
            esplogE(TAG_LIB_JSON, "(loadConfig)", "Failed to save configuration file!");
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

    g_config->mqtt_tls = doc["mqtt_tls"].as<bool>();
    g_config->mqtt_broker = doc["mqtt_broker"].as<String>();
    g_config->mqtt_port = doc["mqtt_port"].as<uint16_t>();
    g_config->mqtt_id = doc["mqtt_id"].as<String>();
    g_config->mqtt_topic = doc["mqtt_topic"].as<String>();
    g_config->mqtt_username = doc["mqtt_username"].as<String>();
    g_config->mqtt_password = doc["mqtt_password"].as<String>();
    g_config->mqtt_cert = doc["mqtt_cert"].as<String>();

    g_config->alarm_countdown_s = doc["alarm_countdown"].as<int>();
    g_config->alarm_e_countdown_s = doc["alarm_countdown_e"].as<int>();
    g_config->alarm_w_threshold = doc["alarm_threshold_w"].as<int>();
    g_config->alarm_e_threshold = doc["alarm_threshold_e"].as<int>();
    g_config->alarm_telephone = doc["alarm_telephone"].as<String>();

    doc.clear();
    configFile.close();

    esplogI(TAG_LIB_JSON, "(loadConfig)", "Successully loaded.");
    return true;
}
