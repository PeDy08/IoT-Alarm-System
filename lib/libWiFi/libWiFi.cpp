#include "libWiFi.h"

AsyncWebServer server(80);

void startWifiSetupMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PSWD);
    esplogI("[wifi]: WiFi AP started! Connect to ESP using WiFi:\n - SSID: %s\n - Password: %s\n - IP: %s\n", WIFI_AP_SSID, WIFI_AP_PSWD, WiFi.softAPIP().toString().c_str());

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
        g_config_t c;
        for (int i = 0; i < params; i++) {
            const AsyncWebParameter * p = request->getParam(i);
            if (p->isPost()) {
                if (p->name() == "ssid") {c.wifi_ssid = p->value().c_str();}
                if (p->name() == "pswd") {c.wifi_pswd = p->value().c_str();}
                if (p->name() == "ip") {c.wifi_ip = p->value().c_str();}
                if (p->name() == "gtw") {c.wifi_gtw = p->value().c_str();}
                if (p->name() == "sbnt") {c.wifi_sbnt = p->value().c_str();}
                if (p->name() == "countdown") {c.alarm_countdown_s = p->value().toInt();}
            }
        }
        
        if (!saveConfig(&c)) {
            esplogE("[server]: Failed to save configuration!\n");
            request->send(500, "text/plain", "Failed to save configuration!\n");
        } else {
            esplogI("[server]: Configuration saved successfully!\n");
            request->send(200, "text/plain", "Configuration saved successfully!\nESP will now restart.");
        }

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
            esplogE("[wifi]: Advanced WiFi configuration has failed! An unexpected error occured!\n");
        }
        } else {
        esplogW("[wifi]: Advanced WiFi configuration has been disabled! User configured advanced parameters in bad format!\n");
        }
    } else {
        esplogW("[wifi]: Advanced WiFi configuration has been disabled! User did not configured advanced parameters!\n");
    }

    // begin wifi conection
    WiFi.begin(g_config->wifi_ssid.c_str(), g_config->wifi_pswd.c_str());
    WiFi.setAutoReconnect(true);
    esplogI("[wifi]: Connecting to WiFi:\n - ssid: %s\n - password: %s\n", g_config->wifi_ssid.c_str(), g_config->wifi_pswd.c_str());

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
        esplogI("[server]: Received configuration data size: %d bytes\n", len);
        File configFile = SD.open(CONFIG_UPLOAD_FILE, "w");
        if (!configFile) {
            esplogE("[server]: Failed to open config file to write new configuration!\n");
        }

        // add check for successfull write!
        configFile.write(data, len);
        configFile.close();

        // add check for successfull save!
        if (!saveConfigFromJSON(g_config)) {
            return;
        }

        esplogI("[server]: Configuration file saved to LittleFS!\n");

        // restart app, after reboot the configuration will be read by loadConfig command
        rebootESP();
    });

    server.begin();
}
