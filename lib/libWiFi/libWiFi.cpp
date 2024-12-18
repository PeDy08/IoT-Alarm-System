#include "libWiFi.h"

AsyncWebServer server(80);

extern g_vars_t * g_vars_ptr;
extern g_config_t * g_config_ptr;

const char* http_username = "admin";
const char* http_password = "8888";

void startWifiSetupMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PSWD);
    esplogI(TAG_LIB_WIFI, "(startWifiSetupMode)", "WiFi AP started! Connect to ESP using WiFi:\n - SSID: %s\n - Password: %s\n - IP: %s\n", WIFI_AP_SSID, WIFI_AP_PSWD, WiFi.softAPIP().toString().c_str());

    char buffer[80];
    sprintf(buffer, "SSID: %s, PASSWORD: %s, IP: %s!", WIFI_AP_SSID, WIFI_AP_PSWD, WiFi.softAPIP().toString().c_str());
    notificationScreenTemplate("WiFi AP running", buffer);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SD, "/web/AP/index.html", "text/html");
    });

    server.serveStatic("/", SD, "/web/AP");

    // webserver configuration
    server.on("/wifimanager", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SD, "/web/AP/wifimanager.html", "text/html");
    });

    server.on("/wifimanager", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        g_config_t c;
        setInvalidonfig(&c);

        for (int i = 0; i < params; i++) {
            const AsyncWebParameter * p = request->getParam(i);
            if (p->isPost()) {
                if (p->name() == "ssid" && p->value().length() > 0) {c.wifi_ssid = p->value();}
                if (p->name() == "pswd") {c.wifi_pswd = p->value();}
                if (p->name() == "ip" && p->value().length() > 8 && p->value().length() < 16) {c.wifi_ip = p->value();}
                if (p->name() == "gtw" && p->value().length() > 8 && p->value().length() < 16) {c.wifi_gtw = p->value();}
                if (p->name() == "sbnt" && p->value().length() > 8 && p->value().length() < 16) {c.wifi_sbnt = p->value();}
            }
        }

        esplogI(TAG_SERVER, "(startWifiSetupMode)", "Received configuration:\n - ssid: %s\n - password: %s\n - ip: %s\n - gateway: %s\n - subnet: %s\n",
            c.wifi_ssid.c_str(), c.wifi_pswd.c_str(), c.wifi_ip.c_str(), c.wifi_gtw.c_str(), c.wifi_sbnt.c_str());

        if (!rewriteConfig(&c, g_config_ptr)) {
            esplogW(TAG_SERVER, "(startWifiSetupMode)", "Failed to rewrite configuration!");
            request->send(500, "text/plain", "Failed to rewrite configuration!\n");
        }

        esplogI(TAG_SERVER, "(startWifiSetupMode)", "Configuration after rewrite:\n - ssid: %s\n - password: %s\n - ip: %s\n - gateway: %s\n - subnet: %s\n",
            g_config_ptr->wifi_ssid.c_str(), g_config_ptr->wifi_pswd.c_str(), g_config_ptr->wifi_ip.c_str(), g_config_ptr->wifi_gtw.c_str(), g_config_ptr->wifi_sbnt.c_str());

        if (!saveConfig(g_config_ptr)) {
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

void startWiFiServerMode() {
    WiFi.mode(WIFI_MODE_STA);
    g_vars_ptr->wifi_mode = WIFI_MODE_STA;

    // config wifi if possible
    if (g_config_ptr->wifi_ip != "\0" && g_config_ptr->wifi_gtw != "\0" && g_config_ptr->wifi_sbnt != "\0") {
        IPAddress localIP;
        IPAddress localGTW;
        IPAddress localSBNT;

        if (localIP.fromString(g_config_ptr->wifi_ip.c_str()) && localGTW.fromString(g_config_ptr->wifi_gtw.c_str()) && localSBNT.fromString(g_config_ptr->wifi_sbnt.c_str())) {
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
    WiFi.begin(g_config_ptr->wifi_ssid.c_str(), g_config_ptr->wifi_pswd.c_str());
    WiFi.setAutoReconnect(true);
    esplogI(TAG_RTOS_WIFI, "(startWiFiServerMode)", "Connecting to WiFi:\n - ssid: %s\n - password: %s", g_config_ptr->wifi_ssid.c_str(), g_config_ptr->wifi_pswd.c_str());

    // ----------------------------------------------------- BASIC ------------------------------------------------------

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        request->send(SD, "/web/STA/index.html", "text/html");
    });

    server.serveStatic("/", SD, "/web/STA");

    server.on("/login", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        request->redirect("/");
    });

    server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
        // request->send(400);
    });

    server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        request->send(SD, "/web/STA/setup.html", "text/html");
    });

    server.on("/setup", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        int params = request->params();
        g_config_t c;
        setInvalidonfig(&c);
        for (int i = 0; i < params; i++) {
            const AsyncWebParameter * p = request->getParam(i);
            if (p->isPost()) {
                if (p->name() == "mqtt_tls" && p->value().length() > 0) {c.mqtt_tls = p->value().toInt()!=0;}
                if (p->name() == "mqtt_brkr" && p->value().length() > 0) {c.mqtt_broker = p->value();}
                if (p->name() == "mqtt_port" && p->value().length() > 0) {c.mqtt_port = p->value().toInt();}
                if (p->name() == "mqtt_id" && p->value().length() > 0) {c.mqtt_id = p->value();}
                if (p->name() == "mqtt_tpc" && p->value().length() > 0) {c.mqtt_topic = p->value();}
                if (p->name() == "mqtt_usrnm" && p->value().length() > 0) {c.mqtt_username = p->value();}
                if (p->name() == "mqtt_pswd") {c.mqtt_password = p->value();}

                if (p->name() == "countdown" && p->value().length() > 0) {c.alarm_countdown_s = p->value().toInt();}
                if (p->name() == "countdown_e" && p->value().length() > 0) {c.alarm_e_countdown_s = p->value().toInt();}
                if (p->name() == "threshold_w" && p->value().length() > 0) {c.alarm_w_threshold = p->value().toInt();}
                if (p->name() == "threshold_e" && p->value().length() > 0) {c.alarm_e_threshold = p->value().toInt();}
                if (p->name() == "telephone" && p->value().length() > 0) {c.alarm_telephone = p->value();}
            }
        }

        esplogI(TAG_SERVER, "(startWiFiServerMode)", "Received configuration:\n -> mqtt:\n   - tls: %d\n   - broker: %s\n   - port: %d\n   - id: %s\n   - topic: %s\n   - username: %s\n   - password: %s\n -> alarm:\n   - cnt (c): %d\n   - cnt (e): %d\n,   - thr (w): %d\n   - thr (e): %d\n   - tel: %s\n",
            c.mqtt_tls, c.mqtt_broker.c_str(), c.mqtt_port, c.mqtt_id.c_str(), c.mqtt_topic.c_str(), c.mqtt_username.c_str(), c.mqtt_password.c_str(),
            c.alarm_countdown_s, c.alarm_e_countdown_s, c.alarm_w_threshold, c.alarm_e_threshold, c.alarm_telephone.c_str());

        if (!rewriteConfig(&c, g_config_ptr)) {
            esplogW(TAG_SERVER, "(startWiFiServerMode)", "Failed to rewrite configuration!");
            request->send(500, "text/plain", "Failed to rewrite configuration!\n");
        }

        esplogI(TAG_SERVER, "(startWiFiServerMode)", "Received configuration:\n -> mqtt:\n   - tls: %d\n   - broker: %s\n   - port: %d\n   - id: %s\n   - topic: %s\n   - username: %s\n   - password: %s\n -> alarm:\n   - cnt (c): %d\n   - cnt (e): %d\n,   - thr (w): %d\n   - thr (e): %d\n   - tel: %s\n",
            g_config_ptr->mqtt_tls, c.mqtt_broker.c_str(), g_config_ptr->mqtt_port, g_config_ptr->mqtt_id.c_str(), g_config_ptr->mqtt_topic.c_str(), g_config_ptr->mqtt_username.c_str(), g_config_ptr->mqtt_password.c_str(),
            g_config_ptr->alarm_countdown_s, g_config_ptr->alarm_e_countdown_s, g_config_ptr->alarm_w_threshold, g_config_ptr->alarm_e_threshold, g_config_ptr->alarm_telephone.c_str());

        if (!saveConfig(g_config_ptr)) {
            esplogE(TAG_SERVER, "(startWiFiServerMode)", "Failed to save configuration!");
            request->send(500, "text/plain", "Failed to save configuration!\n");
        } else {
            esplogI(TAG_SERVER, "(startWiFiServerMode)", "Configuration saved successfully!");
            request->send(200, "text/plain", "Configuration saved successfully!\nESP will now restart.");
        }

        displayRestart();
        delay(3000);
        ESP.restart();
    });

    // server.onNotFound();

    // ---------------------------------------------------- DOWNOLAD ----------------------------------------------------

    server.on("/download/log", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        if (SD.exists(LOG_FILE)) {
            request->send(SD, LOG_FILE, "text/plain");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    server.on("/download/password", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        if (SD.exists(LOCK_FILE)) {
            request->send(SD, LOCK_FILE, "text/plain");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    server.on("/download/rfid", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        if (SD.exists(CONFIG_FILE)) {
            request->send(SD, RFID_FILE, "text/plain");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    server.on("/download/config", HTTP_GET, [](AsyncWebServerRequest *request){
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        if (SD.exists(CONFIG_FILE)) {
            request->send(SD, CONFIG_FILE, "application/json");
        } else {
            request->send(200, "text/plain", "File not found!");
        }
    });

    // ----------------------------------------------------- UPOLAD -----------------------------------------------------

    g_config_t * config = g_config_ptr;

    server.on("/upload/config", HTTP_POST, [](AsyncWebServerRequest *request){
        if (!request->authenticate(http_username, http_password)) {
            return request->requestAuthentication();
        }
        request->send(200, "text/plain", "JSON file received successfully!");
    }, nullptr, [config](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        esplogI(TAG_SERVER, "(startWiFiServerMode)", "Received configuration data size: %d bytes", len);
        File configFile = SD.open(CONFIG_UPLOAD_FILE, "w");
        if (!configFile) {
            esplogE(TAG_SERVER, "(startWiFiServerMode)", "Failed to open config file to write new configuration!");
        }

        // add check for successfull write!
        configFile.write(data, len);
        configFile.close();

        // add check for successfull save!
        if (!saveConfigFromJSON(config)) {
            return;
        }

        esplogI(TAG_SERVER, "(startWiFiServerMode)", "Configuration file saved to LittleFS!");

        // restart app, after reboot the configuration will be read by loadConfig command
        rebootESP();
    });

    server.begin();
}
