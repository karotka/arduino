#include "SPI.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "config.h"
#include <FS.h>
#include "EEPROM.h"
#include "util.h"

Config_t config;
ESP8266WebServer server(80);
SoftwareSerial ESPserial(D7, D8); // RX | TX

int getlen(const char* buffer) {
    int len = 0;
    char c = buffer[len];
    while(c != '\0'){
        ++len;
        c = buffer[len];
    }
    return len;
}

int split(char* buffer, const char delimiter, char** strs, int n) {

    //obtain len
    int len = getlen(buffer);
    int split = 0;
    strs[split] = buffer;

    for(int i = 0; i < len; ++i){
        if (buffer[i] == delimiter){
            buffer[i] = '\0';
            if( i + 1 != len){
                ++split;
                strs[split] = &buffer[i+1];
                if(split == n - 1)
                    break;
            }
        }
    }
    return split + 1;
}

inline int SERIALREAD(char* serialRxBuffer) {

    unsigned long time = millis();
    int avail;
    int serialRxBufferCounter = 0;
    while(true) {
        avail = Serial.available();
        if (avail) break;
        if (millis() - time > SERIAL_READ_TIMEOUT) return serialRxBufferCounter;
    }

    do {
        char ch = Serial.read();
        ESPserial.print("ch:");
        ESPserial.println(ch);

        if (ch == '\n') {
            break;
        } else {
            serialRxBuffer[serialRxBufferCounter] = ch;
            serialRxBufferCounter++;
        }

    } while (Serial.available());

    return serialRxBufferCounter;
}

void wifiConnect() {

    // Connect to WiFi network
    ESPserial.println("");
    ESPserial.print("Connecting to: ");
    ESPserial.print(config.ssid);

    WiFi.config(config.ip, config.gateway, config.subnet);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        ESPserial.print(".");
    }
    ESPserial.println("");
    ESPserial.print("WiFi connected: ");
    ESPserial.print("http://");
    ESPserial.print(WiFi.localIP().toString());
    ESPserial.println("/");
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

void handleSelect() {
    File dataFile = SPIFFS.open("/select.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleLight() {
    File dataFile = SPIFFS.open("/light.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleTime() {
    File dataFile = SPIFFS.open("/time.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleTimer() {
    File dataFile = SPIFFS.open("/timer.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleTemp() {
    File dataFile = SPIFFS.open("/temp.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleCo2() {
    File dataFile = SPIFFS.open("/co2.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleFeed() {
    File dataFile = SPIFFS.open("/feed.html", "r");
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

String responseHolo(char* data) {
    char *spl[2];
    int c = split(data, '\t', spl, sizeof(spl));
    return String(
        "[{\"cl\":\"time\",\"v\":\"" + String(spl[1]) + "\"}," +
        "{\"cl\":\"date\",\"v\":\"" + String(spl[0]) + "\"}," +
        "{\"cl\":\"temp1\",\"v\":\"23.0 C\"}," +
        "{\"cl\":\"mode\",\"v\":\"MODE: AUTO (OFF)\"}," +
        "{\"cl\":\"co2\",\"v\":\"CO2: OFF &nbsp; TB: 26.8C\"}," +
        "{\"cl\":\"temp2\",\"v\":\"T1: 0.0C T2: 0.0C\"}]"
    );
}

String responseTmlo(char* data) {
    char *spl[2];
    int c = split(data, '\t', spl, sizeof(spl));
    return String(
        "[{\"id\":\"time\",\"v\":\"" + String(spl[1]) + "\"}," +
        "{\"id\":\"date\",\"v\":\"" + String(spl[0]) + "\"}]"
    );
}

String responseTilo(char* data) {
    char *spl[16];
    int c = split(data, '\t', spl, sizeof(spl));
    return String(
        "[{\"id\":\"t1\",\"h\":\"" + String(spl[0]) + "\"},"
        "{\"id\":\"t2\",\"h\":\"" + String(spl[1]) + "\"},"
        "{\"id\":\"t3\",\"h\":\"" + String(spl[2]) + "\"},"
        "{\"id\":\"t4\",\"h\":\"" + String(spl[3]) + "\"},"
        "{\"id\":\"t5\",\"h\":\"" + String(spl[4]) + "\"},"
        "{\"id\":\"t6\",\"h\":\"" + String(spl[5]) + "\"},"
        "{\"id\":\"t7\",\"h\":\"" + String(spl[6]) + "\"},"
        "{\"id\":\"t8\",\"h\":\"" + String(spl[7]) + "\"},"

        "{\"id\":\"T1MO\",\"v\":\"" + String(spl[8]) + "\"},"
        "{\"id\":\"T2MO\",\"v\":\"" + String(spl[9]) + "\"},"
        "{\"id\":\"T3MO\",\"v\":\"" + String(spl[10]) + "\"},"
        "{\"id\":\"T4MO\",\"v\":\"" + String(spl[11]) + "\"},"
        "{\"id\":\"T5MO\",\"v\":\"" + String(spl[12]) + "\"},"
        "{\"id\":\"T6MO\",\"v\":\"" + String(spl[13]) + "\"},"
        "{\"id\":\"T7MO\",\"v\":\"" + String(spl[14]) + "\"},"
        "{\"id\":\"T8MO\",\"v\":\"" + String(spl[15]) + "\"}]"
    );
}

String responseLilo(char* data) {
    char *spl[12];
    //ESPserial.print("data:");
    //ESPserial.println(data);
    int c = split(data, '\t', spl, sizeof(spl));
    return String(
        "[{\"id\":\"cool\",\"v\":\""+ String(spl[0]) +"\"},"
        "{\"id\":\"warm\",\"v\":\""+ String(spl[1]) +"\"},"
        "{\"id\":\"yellow\",\"v\":\""+ String(spl[2]) +"\"},"
        "{\"id\":\"red\",\"v\":\""+ String(spl[3]) +"\"},"
        "{\"id\":\"green\",\"v\":\""+ String(spl[4]) +"\"},"
        "{\"id\":\"blue\",\"v\":\""+ String(spl[5]) +"\"},"

        "{\"id\":\"B0\",\"v\":\""+ String(spl[6]) +"\"},"
        "{\"id\":\"B1\",\"v\":\""+ String(spl[7]) +"\"},"
        "{\"id\":\"B2\",\"v\":\""+ String(spl[8]) +"\"},"
        "{\"id\":\"B3\",\"v\":\""+ String(spl[9]) +"\"},"
        "{\"id\":\"B4\",\"v\":\""+ String(spl[10]) +"\"},"
        "{\"id\":\"B5\",\"v\":\""+ String(spl[11]) +"\"}]"
        );
}

String responseLcw(char* data) {
    char *spl[2];
    int c = split(data, '\t', spl, sizeof(spl));
    return String(
        "{\"id\":\"" + String(spl[1]) +
        "\",\"value\":\"" + String(spl[0]) + "\"}"
    );
}

String responseLight(char* data) {
    char *spl[2];
    int c = split(data, '\t', spl, sizeof(spl));
    return String(
        "{\"id\":\"" + String(spl[1]) +
        "\",\"value\":\"" + String(spl[0]) + "\"}"
    );
}

String responseTis(char* data) {
    char *spl[2];
    int c = split(data, '\t', spl, sizeof(spl));
    return String(
        "{\"id\":\"" + String(spl[0]) +
        "\",\"v\":\"" + String(spl[1]) + "\"}"
    );
}

void handleRead() {
    String params = server.arg("p");
    String value = server.arg("v");

    Serial.print(params + '\t' + value + '\n');
    //Serial.flush(); // wait for a serial string to be finished sending

    char data[80];
    if (!SERIALREAD(data)) {
        server.setContentLength(2);
        server.send(204, "application/json", "{}");
    }

    ESPserial.print("params:");
    ESPserial.println(params);

    String ret;
    if (params == "HOLO") {
        ret = responseHolo(data);
    } else
    if (params == "TMLO") {
        ret = responseTmlo(data);
    } else
    if (params == "LILO") {
        ret = responseLilo(data);
    } else
    if (params == "TILO") {
        ret = responseTilo(data);
    } else
    if (params == "LCW") {
        ret = responseLight(data);
    } else
    if (params == "LWW") {
        ret = responseLight(data);
    } else
    if (params == "LYE") {
        ret = responseLight(data);
    } else
    if (params == "LRE") {
        ret = responseLight(data);
    } else
    if (params == "LGR") {
        ret = responseLight(data);
    } else
    if (params == "LBL") {
        ret = responseLight(data);
    }
    if (params == "TIS") {
        ret = responseTis(data);
    }

    server.setContentLength(ret.length());
    server.send(200, "application/json", ret);
}

void setup() {
    Serial.begin(115200);
    ESPserial.begin(115200);

    //while (!Serial);
    delay(10);
    //Serial.write('\n');

    SPIFFS.begin();

    if (config.apMode) {
        wifiAp();
    } else {
        wifiConnect();
    }

    // Start the server
    server.on("/", handleRoot);
    server.on("/timer.html", handleTimer);
    server.on("/time.html", handleTime);
    server.on("/light.html", handleLight);
    server.on("/temp.html", handleTemp);
    server.on("/co2.html", handleCo2);
    server.on("/feed.html", handleFeed);

    server.on("/select.html", handleSelect);
    server.on("/styles.css", handleCss);
    server.on("/connect", handleConnect);
    server.on("/setup", handleSetup);
    server.on("/setupData", handleSetupData);
    server.on("/saveData", handleSaveData);
    server.on("/apmode", handleAPmode);

    server.on("/read", handleRead);

    server.begin();
    ESPserial.println("Server started");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    server.handleClient();
    //ESPserial.println("Ahoj");
    //delay(1000);
}
