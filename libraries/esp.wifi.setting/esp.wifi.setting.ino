#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "config.h"
#include <debugutil.h>

ESP8266WebServer server(80);
ConfigWifi_t config;

void wifiAp() {
    WiFi.disconnect();
    for (int i = 0; i < 4; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }

    String hostname("ESP-temp-" + String(ESP.getChipId()));
    WiFi.softAP(hostname);
    IPAddress IP = WiFi.softAPIP();
    SLOGF("Switch into AP mode: %s %s",
          hostname.c_str(), IP.toString().c_str());
}

void wifiConnect() {

    if (config.dhcp) {
        SLOG("WiFi in DHCP mode");
    } else {
        IPAddress ip;
        ip.fromString(config.ip);
        IPAddress gw;
        gw.fromString(config.gateway);
        IPAddress sub;
        sub.fromString(config.subnet);

        WiFi.config(ip, gw, sub);
        SLOGF("WiFi in static IP mode: %s", ip.toString().c_str());
    }
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    pinMode(LED_BUILTIN, OUTPUT);
    analogWrite(LED_BUILTIN, 1000);

    int retryCount = 0;
    bool st = true;
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        if (st) digitalWrite(LED_BUILTIN, HIGH);
        else analogWrite(LED_BUILTIN, 1000);
        st = !st;
        retryCount++;
        if (retryCount > 20) {
            wifiAp();
            break;
        }
    }
    SLOGF("WiFi connected: http://%s/", WiFi.localIP().toString().c_str());

    analogWrite(LED_BUILTIN, 1000);
}

void reconnectWifi() {
    WiFi.disconnect();
    wifiConnect();
}

void handleAPmode() {
    wifiAp();
    delay(500);
    server.send(200, "text/json",
                String("Use this IP: ") + WiFi.softAPIP().toString());
}

void handleNetworkCss() {
    File dataFile = SPIFFS.open("/nstyle.css", "r");
    server.streamFile(dataFile, "text/css");
    dataFile.close();
}

void handleNetworkSetup() {
    File dataFile = SPIFFS.open("/network_setup.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleNetworkData() {
    config.load();

    String ret =
        "{\"ip\" : \""      + config.ip + "\","
        "\"gateway\" : \""  + config.gateway + "\","
        "\"subnet\" : \""   + config.subnet + "\","
        "\"ssid\" : \""     + config.ssid + "\","
        "\"password\" : \"" + config.password + "\","
        "\"dhcp\" : \""     + config.dhcp + "\","
        "\"localIp\" : \""  + WiFi.localIP().toString() + "\"}";

    server.setContentLength(ret.length());
    server.send(200, "text/json", ret);
}

void handleSaveNetworkData() {

    config.ssid = server.arg("ssid");
    config.ssidSize = config.ssid.length();

    config.password = server.arg("password");
    config.passwordSize = config.password.length();

    config.ip = server.arg("ip");
    config.ipSize = config.ip.length();

    config.gateway = server.arg("gateway");
    config.gatewaySize = config.gateway.length();

    config.subnet = server.arg("subnet");
    config.subnetSize = config.subnet.length();

    config.dhcp = server.arg("dhcp").equals("1") ? 1 : 0;

    config.save();

    server.sendHeader("Location", String("/networkSetup"), true);
    server.send(302, "text/plain", "");
}

void setup() {
    Serial.begin(115200);
    delay(10);

    EEPROM.begin(EEPROM_SIZE);

    config.load();

    SPIFFS.begin();

    wifiConnect();

    // Start the server
    server.on("/nstyle.css",      handleNetworkCss);
    server.on("/networkSetup",    handleNetworkSetup);
    server.on("/networkData",     handleNetworkData);
    server.on("/saveNetworkData", handleSaveNetworkData);
    server.on("/apmode",          handleAPmode);
    server.on("/connect",         reconnectWifi);

    server.begin();
}

void loop() {
    server.handleClient();
}
