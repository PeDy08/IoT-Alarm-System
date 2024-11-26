#include "libWiFi.h"

AsyncWebServer server(80);

extern g_config_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

void startWifiSetupMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PSWD);
    esplogI(TAG_LIB_WIFI, "(startWifiSetupMode)", "WiFi AP started! Connect to ESP using WiFi:\n - SSID: %s\n - Password: %s\n - IP: %s\n", WIFI_AP_SSID, WIFI_AP_PSWD, WiFi.softAPIP().toString().c_str());

    // webserver configuration
    server.on("/wifimanager", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SD, "/web/AP/wifimanager.html", "text/html");
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SD, "/web/AP/index.html", "text/html");
    });

    server.serveStatic("/", SD, "/web/AP");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        g_config_t c = *g_config_ptr; // so it updates only edited forms, empty forms will be saved from current configuration
        for (int i = 0; i < params; i++) {
            const AsyncWebParameter * p = request->getParam(i);
            if (p->isPost()) {
                if (p->name() == "ssid") {c.wifi_ssid = p->value().c_str();}
                if (p->name() == "pswd") {c.wifi_pswd = p->value().c_str();}
                if (p->name() == "ip") {c.wifi_ip = p->value().c_str();}
                if (p->name() == "gtw") {c.wifi_gtw = p->value().c_str();}
                if (p->name() == "sbnt") {c.wifi_sbnt = p->value().c_str();}

                if (p->name() == "mqtt_tls") {c.mqtt_tls = p->value().toInt()!=0;}
                if (p->name() == "mqtt_brkr") {c.mqtt_broker = p->value().c_str();}
                if (p->name() == "mqtt_port") {c.mqtt_port = p->value().toInt();}
                if (p->name() == "mqtt_id") {c.mqtt_id = p->value().c_str();}
                if (p->name() == "mqtt_tpc") {c.mqtt_topic = p->value().c_str();}
                if (p->name() == "mqtt_usrnm") {c.mqtt_username = p->value().c_str();}
                if (p->name() == "mqtt_pswd") {c.mqtt_password = p->value().c_str();}

                if (p->name() == "countdown") {c.alarm_countdown_s = p->value().toInt();}
                if (p->name() == "countdown_e") {c.alarm_e_countdown_s = p->value().toInt();}
                if (p->name() == "threshold_w") {c.alarm_w_threshold = p->value().toInt();}
                if (p->name() == "threshold_e") {c.alarm_e_threshold = p->value().toInt();}
                if (p->name() == "telephone") {c.alarm_telephone = p->value().c_str();}
            }
        }
        
        if (!saveConfig(&c)) {
            esplogE(TAG_SERVER, "(startWifiSetupMode)", "Failed to save configuration!");
            request->send(500, "text/plain", "Failed to save configuration!\n");
        } else {
            esplogI(TAG_SERVER, "(startWifiSetupMode)", "Configuration saved successfully!");
            request->send(200, "text/plain", "Configuration saved successfully!\nESP will now restart.");
        }

        displayRestart();
        delay(3000);
        ESP.restart();
    });

    server.begin();
}

void startWiFiServerMode(g_vars_t * g_vars, g_config_t * g_config) {
    WiFi.mode(WIFI_MODE_STA);
    g_vars->wifi_mode = WIFI_MODE_STA;

    // config wifi if possible
    if (g_config->wifi_ip != "\0" && g_config->wifi_gtw != "\0" && g_config->wifi_sbnt != "\0") {
        IPAddress localIP;
        IPAddress localGTW;
        IPAddress localSBNT;

        if (localIP.fromString(g_config->wifi_ip.c_str()) && localGTW.fromString(g_config->wifi_gtw.c_str()) && localSBNT.fromString(g_config->wifi_sbnt.c_str())) {
        if (!WiFi.config(localIP, localGTW, localSBNT)) {
            esplogE(TAG_RTOS_WIFI, "(startWiFiServerMode)", "Advanced WiFi configuration has failed! An unexpected error occured!");
        }
        } else {
        esplogW(TAG_RTOS_WIFI, "(startWiFiServerMode)", "Advanced WiFi configuration has been disabled! User configured advanced parameters in bad format!");
        }
    } else {
        esplogW(TAG_RTOS_WIFI, "(startWiFiServerMode)", "Advanced WiFi configuration has been disabled! User did not configured advanced parameters!");
    }

    // begin wifi conection
    WiFi.begin(g_config->wifi_ssid.c_str(), g_config->wifi_pswd.c_str());
    WiFi.setAutoReconnect(true);
    esplogI(TAG_RTOS_WIFI, "(startWiFiServerMode)", "Connecting to WiFi:\n - ssid: %s\n - password: %s", g_config->wifi_ssid.c_str(), g_config->wifi_pswd.c_str());

    // webserver configuration
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Server running!");
    });

    server.on("/download/log", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD.exists(LOG_FILE)) {
            request->send(SD, LOG_FILE, "text/plain");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    server.on("/download/password", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD.exists(LOCK_FILE)) {
            request->send(SD, LOCK_FILE, "text/plain");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    server.on("/download/rfid", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD.exists(CONFIG_FILE)) {
            request->send(SD, RFID_FILE, "text/plain");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    server.on("/download/config", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD.exists(CONFIG_FILE)) {
            request->send(SD, CONFIG_FILE, "application/json");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    server.on("/upload/config", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "JSON file received successfully!");
    }, nullptr, [g_config](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        esplogI(TAG_SERVER, "(startWiFiServerMode)", "Received configuration data size: %d bytes", len);
        File configFile = SD.open(CONFIG_UPLOAD_FILE, "w");
        if (!configFile) {
            esplogE(TAG_SERVER, "(startWiFiServerMode)", "Failed to open config file to write new configuration!");
        }

        // add check for successfull write!
        configFile.write(data, len);
        configFile.close();

        // add check for successfull save!
        if (!saveConfigFromJSON(g_config)) {
            return;
        }

        esplogI(TAG_SERVER, "(startWiFiServerMode)", "Configuration file saved to LittleFS!");

        // restart app, after reboot the configuration will be read by loadConfig command
        rebootESP();
    });

    server.begin();
}
