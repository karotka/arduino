#include "SPI.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include <FS.h>
#include "EEPROM.h"

Config_t config;
ESP8266WebServer server(80);

void wifiConnect() {

    // Connect to WiFi network
    Serial.println("");
    Serial.print("Connecting to: ");
    Serial.print(config.ssid);

    WiFi.config(config.ip, config.gateway, config.subnet);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP().toString());
    Serial.println("/");
}

void wifiAp() {
    IPAddress apIp(10, 0, 0, 1);
    //WiFi.softAPConfig(IPAddress(10, 0, 0, 10),
    //                  IPAddress(10, 0, 0, 1),
    //                  IPAddress(255, 255, 255, 0));
    WiFi.softAP("ESP001");
    //WiFi.mode(WIFI_AP_STA);

    delay(500);
    Serial.println("Setting AP");
    Serial.println(WiFi.softAPIP());
    Serial.print("Server MAC address: ");
    Serial.println(WiFi.softAPmacAddress());
}

void handleConnect() {
    wifiConnect();

    delay(500);

    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
}

void handleRoot() {
    File dataFile = SPIFFS.open("/index.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleAPmode() {
    wifiAp();
    delay(500);
    server.send(200, "text/json",
                String("Use this IP: ") + WiFi.softAPIP().toString());
}

void handleCss() {
    File dataFile = SPIFFS.open("/styles.css", "r");
    server.streamFile(dataFile, "text/css");
    dataFile.close();
}

void handleSetup() {
    File dataFile = SPIFFS.open("/setup.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleSetupData() {
    String ret =
        "{\"ip\" : \""      + config.ip.toString() + "\","
        "\"gateway\" : \""  + config.gateway.toString() + "\","
        "\"subnet\" : \""   + config.subnet.toString() + "\","
        "\"ssid\" : \""     + config.ssid + "\","
        "\"password\" : \"" + config.password + "\"}";

    server.setContentLength(ret.length());
    server.send(200, "text/json", ret);
}

void handleSaveData() {
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;

    ip.fromString(server.arg("ip"));
    gateway.fromString(server.arg("gateway"));
    subnet.fromString(server.arg("subnet"));

    config.ip = ip;
    config.gateway = gateway;
    config.subnet = subnet;

    config.ssid = server.arg("ssid");
    config.password = server.arg("password");

    delay(500);

    server.sendHeader("Location", String("/setup"), true);
    server.send(302, "text/plain", "");

}

void digitalPotWrite(int value) {
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(CS, LOW);
    SPI.transfer(POT_ADDRESS);
    SPI.transfer(value);
    digitalWrite(CS, HIGH);
    delay(10);
    digitalWrite(LED_BUILTIN, HIGH);
}

void handleVal() {
    String v = server.arg("v");
    unsigned int val = map(v.toInt(), 0, 255, 0, 100);
    digitalPotWrite(val);

    server.send(200, "text/html", "OK");
}

void spiBegin() {
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
    SPI.begin();
}

void setup() {
    Serial.begin(115200);
    delay(10);

    SPIFFS.begin();

    if (config.apMode) {
        wifiAp();
    } else {
        wifiConnect();
    }

    // Start the server
    server.on("/", handleRoot);
    server.on("/styles.css", handleCss);
    server.on("/val", handleVal);
    server.on("/connect", handleConnect);
    server.on("/setup", handleSetup);
    server.on("/setupData", handleSetupData);
    server.on("/saveData", handleSaveData);

    server.on("/apmode", handleAPmode);

    server.begin();
    Serial.println("Server started");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    spiBegin();
}

void loop() {
    server.handleClient();
}
