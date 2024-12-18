#include "libJson.h"

void setDefaultConfig(g_config_t *g_config) {
    esplogI(TAG_LIB_JSON, "(setDefaultConfig)", "Reseting configuration data to defaults.");
    g_config->wifi_ssid = "";
    g_config->wifi_pswd = "";
    g_config->wifi_ip = "";
    g_config->wifi_gtw = "";
    g_config->wifi_sbnt = "";

    g_config->mqtt_tls = 1;
    g_config->mqtt_broker = "";
    g_config->mqtt_port = 1883;
    g_config->mqtt_id = "IoT_Alarm";
    g_config->mqtt_topic = "IoT_Alarm";
    g_config->mqtt_username = "";
    g_config->mqtt_password = "";
    g_config->mqtt_cert = "";

    g_config->alarm_countdown_s = 120;
    g_config->alarm_e_countdown_s = 120;
    g_config->alarm_w_threshold = 5;
    g_config->alarm_e_threshold = 7;

    g_config->alarm_telephone = "";
}

void setInvalidonfig(g_config_t *g_config) {
    g_config->wifi_ssid = "INVALID";
    g_config->wifi_pswd = "INVALID";
    g_config->wifi_ip = "INVALID";
    g_config->wifi_gtw = "INVALID";
    g_config->wifi_sbnt = "INVALID";

    g_config->mqtt_tls = -1;
    g_config->mqtt_broker = "INVALID";
    g_config->mqtt_port = -1;
    g_config->mqtt_id = "INVALID";
    g_config->mqtt_topic = "INVALID";
    g_config->mqtt_username = "INVALID";
    g_config->mqtt_password = "INVALID";
    g_config->mqtt_cert = "INVALID";

    g_config->alarm_countdown_s = -1;
    g_config->alarm_e_countdown_s = -1;
    g_config->alarm_w_threshold = -1;
    g_config->alarm_e_threshold = -1;
    g_config->alarm_telephone = "INVALID";
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
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = g_config->wifi_ssid;
    wifi["password"] = g_config->wifi_pswd;
    wifi["ip"] = g_config->wifi_ip;
    wifi["gateway"] = g_config->wifi_gtw;
    wifi["subnet"] = g_config->wifi_sbnt;

    JsonObject mqtt = doc["mqtt"].to<JsonObject>();
    mqtt["mqtt_tls"] = g_config->mqtt_tls;
    mqtt["mqtt_broker"] = g_config->mqtt_broker;
    mqtt["mqtt_port"] = g_config->mqtt_port;
    mqtt["mqtt_id"] = g_config->mqtt_id;
    mqtt["mqtt_topic"] = g_config->mqtt_topic;
    mqtt["mqtt_username"] = g_config->mqtt_username;
    mqtt["mqtt_password"] = g_config->mqtt_password;
    mqtt["mqtt_cert"] = g_config->mqtt_cert;

    JsonObject alarm = doc["alarm"].to<JsonObject>();
    alarm["alarm_countdown"] = g_config->alarm_countdown_s;
    alarm["alarm_countdown_e"] = g_config->alarm_e_countdown_s;
    alarm["alarm_threshold_w"] = g_config->alarm_w_threshold;
    alarm["alarm_threshold_e"] = g_config->alarm_e_threshold;
    alarm["alarm_telephone"] = g_config->alarm_telephone;

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

bool rewriteConfig(g_config_t *src, g_config_t *dst) {
    esplogI(TAG_LIB_JSON, "(rewriteConfig)", "Rewriting configuration data...");

    if (!loadConfig(dst, CONFIG_FILE)) {
        esplogW(TAG_LIB_JSON, "(rewriteConfig)", "Failed to load destination configuration data!");
        return false;
    }
    esplogI(TAG_LIB_JSON, "(rewriteConfig)", "Successfully loaded destination configuration data!");

    if (src->wifi_ssid != "INVALID") {
        dst->wifi_ssid = src->wifi_ssid;
    }
    if (src->wifi_pswd != "INVALID") {
        dst->wifi_pswd = src->wifi_pswd;
    }
    if (src->wifi_ip != "INVALID") {
        dst->wifi_ip = src->wifi_ip;
    }
    if (src->wifi_gtw != "INVALID") {
        dst->wifi_gtw = src->wifi_gtw;
    }
    if (src->wifi_sbnt != "INVALID") {
        dst->wifi_sbnt = src->wifi_sbnt;
    }

    if (src->mqtt_tls != -1) {
        dst->mqtt_tls = src->mqtt_tls;
    }
    if (src->mqtt_broker != "INVALID") {
        dst->mqtt_broker = src->mqtt_broker;
    }
    if (src->mqtt_port != -1) {
        dst->mqtt_port = src->mqtt_port;
    }
    if (src->mqtt_id != "INVALID") {
        dst->mqtt_id = src->mqtt_id;
    }
    if (src->mqtt_topic != "INVALID") {
        dst->mqtt_topic = src->mqtt_topic;
    }
    if (src->mqtt_username != "INVALID") {
        dst->mqtt_username = src->mqtt_username;
    }
    if (src->mqtt_password != "INVALID") {
        dst->mqtt_password = src->mqtt_password;
    }
    if (src->mqtt_cert != "INVALID") {
        dst->mqtt_cert = src->mqtt_cert;
    }

    if (src->alarm_countdown_s != -1) {
        dst->alarm_countdown_s = src->alarm_countdown_s;
    }
    if (src->alarm_e_countdown_s != -1) {
        dst->alarm_e_countdown_s = src->alarm_e_countdown_s;
    }
     if (src->alarm_w_threshold != -1) {
        dst->alarm_w_threshold = src->alarm_w_threshold;
    }
    if (src->alarm_e_threshold != -1) {
        dst->alarm_e_threshold = src->alarm_e_threshold;
    }
    if (src->alarm_telephone != "INVALID") {
        dst->alarm_telephone = src->alarm_telephone;
    }

    esplogI(TAG_LIB_JSON, "(rewriteConfig)", "Successfully saved configuration data from received file!");
    return true;
}

bool loadConfig(g_config_t * g_config, const char* filepath) {
    esplogI(TAG_LIB_JSON, "(loadConfig)", "Loading configuration data from config file...");

    if (strcmp(filepath, CONFIG_FILE) == 0) {
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
            return loadConfig(g_config, CONFIG_FILE);
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
    if (!doc["wifi"]["ssid"].is<String>() ||
        !doc["wifi"]["password"].is<String>() ||
        !doc["wifi"]["ip"].is<String>() ||
        !doc["wifi"]["gateway"].is<String>() ||
        !doc["wifi"]["subnet"].is<String>() ||
        !doc["mqtt"]["mqtt_tls"].is<int>() ||
        !doc["mqtt"]["mqtt_broker"].is<String>() ||
        !doc["mqtt"]["mqtt_port"].is<int>() ||
        !doc["mqtt"]["mqtt_id"].is<String>() ||
        !doc["mqtt"]["mqtt_topic"].is<String>() ||
        !doc["mqtt"]["mqtt_username"].is<String>() ||
        !doc["mqtt"]["mqtt_password"].is<String>() ||
        !doc["mqtt"]["mqtt_cert"].is<String>() ||
        !doc["alarm"]["alarm_countdown"].is<int>() ||
        !doc["alarm"]["alarm_countdown_e"].is<int>() ||
        !doc["alarm"]["alarm_threshold_w"].is<int>() ||
        !doc["alarm"]["alarm_threshold_e"].is<int>() ||
        !doc["alarm"]["alarm_telephone"].is<String>()) {
            
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
    g_config->wifi_ssid = doc["wifi"]["ssid"].as<String>();
    g_config->wifi_pswd = doc["wifi"]["password"].as<String>();
    g_config->wifi_ip = doc["wifi"]["ip"].as<String>();
    g_config->wifi_gtw = doc["wifi"]["gateway"].as<String>();
    g_config->wifi_sbnt = doc["wifi"]["subnet"].as<String>();

    g_config->mqtt_tls = doc["mqtt"]["mqtt_tls"].as<bool>();
    g_config->mqtt_broker = doc["mqtt"]["mqtt_broker"].as<String>();
    g_config->mqtt_port = doc["mqtt"]["mqtt_port"].as<int>();
    g_config->mqtt_id = doc["mqtt"]["mqtt_id"].as<String>();
    g_config->mqtt_topic = doc["mqtt"]["mqtt_topic"].as<String>();
    g_config->mqtt_username = doc["mqtt"]["mqtt_username"].as<String>();
    g_config->mqtt_password = doc["mqtt"]["mqtt_password"].as<String>();
    g_config->mqtt_cert = doc["mqtt"]["mqtt_cert"].as<String>();

    g_config->alarm_countdown_s = doc["alarm"]["alarm_countdown"].as<int>();
    g_config->alarm_e_countdown_s = doc["alarm"]["alarm_countdown_e"].as<int>();
    g_config->alarm_w_threshold = doc["alarm"]["alarm_threshold_w"].as<int>();
    g_config->alarm_e_threshold = doc["alarm"]["alarm_threshold_e"].as<int>();
    g_config->alarm_telephone = doc["alarm"]["alarm_telephone"].as<String>();

    doc.clear();
    configFile.close();

    esplogI(TAG_LIB_JSON, "(loadConfig)", "Successully loaded.");
    return true;
}
