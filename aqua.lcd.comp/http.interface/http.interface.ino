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
char serialRxBuffer[256];
int command;

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

int SERIALREAD() {
    for(int i = 0; i < sizeof(serialRxBuffer); ++i) {
        serialRxBuffer[i] = (char)0;
    }

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
        //ESPserial.print("ch:");
        //ESPserial.println(ch);

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
    char buf[5];
    sprintf(buf, "%d\t\n", PAGE_HOME);
    Serial.print(buf);

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
    char buf[5];
    sprintf(buf, "%d\t\n", PAGE_TIME);
    Serial.print(buf);

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

String responseHolo() {
    char *spl[8];
    //ESPserial.print("data:");
    //ESPserial.println();
    int c = split(serialRxBuffer, '\t', spl, sizeof(spl));
    return String(
        "[{\"id\":\"dt\",\"v\":\""   + String(spl[0]) + "\"}," +
        "{\"id\":\"temp\",\"v\":\""  + String(spl[1]) + "\"}," +
        "{\"id\":\"co2\",\"v\":\""   + String(spl[2]) + "\"}," +
        "{\"id\":\"light\",\"v\":\"" + String(spl[3]) + "\"}," +
        "{\"id\":\"feed\",\"v\":[\""  + String(spl[4]) + "\", \""  + String(spl[5]) + "\", \""  + String(spl[6]) + "\", \""  + String(spl[7]) + "\"]}," +
        "{\"id\":\"ser\",\"v\":\"neco\"}," +
        "{\"p\":\"index\"}]"
    );
}

String responseTilo() {
    return String(
        "{\"dt\":\"" + String(serialRxBuffer) + "\"}"
    );
}

String responseColo() {
    char *spl[12];
    int c = split(serialRxBuffer, '\t', spl, sizeof(spl));
    return String(
       "[{\"id\":1, \"h\":\""+ String(spl[0]) +"\",\"m\":\"" + String(spl[1]) + "\"}," +
       "{\"id\":2, \"h\":\""+ String(spl[2]) +"\",\"m\":\"" + String(spl[3]) + "\"}," +
       "{\"id\":3, \"h\":\""+ String(spl[4]) +"\",\"m\":\"" + String(spl[5]) + "\"}," +
       "{\"id\":4, \"h\":\""+ String(spl[6]) +"\",\"m\":\"" + String(spl[7]) + "\"}," +
       "{\"s\":[\"" + String(spl[8]) + "\",\""+ String(spl[9]) + "\",\"" +
                     String(spl[10]) + "\",\"" + String(spl[11]) + "\"]}]"
    );
}

String responseFdlo() {
    char *spl[8];
    int c = split(serialRxBuffer, '\t', spl, sizeof(spl));
    return String(
       "[{\"id\":1, \"h\":\""+ String(spl[0]) +"\",\"m\":\"" + String(spl[1]) + "\"}," +
       "{\"id\":2, \"h\":\""+  String(spl[2]) +"\",\"m\":\"" + String(spl[3]) + "\"}," +
       "{\"id\":3, \"h\":\""+  String(spl[4]) +"\",\"m\":\"" + String(spl[5]) + "\"}," +
       "{\"id\":4, \"h\":\""+  String(spl[6]) +"\",\"m\":\"" + String(spl[7]) + "\"}]"
    );
}

String responseLilo() {
    char *spl[12];
    int c = split(serialRxBuffer, '\t', spl, sizeof(spl));
    return String(
        "[{\"id\":\"cool\",\"v\":\""  + String(spl[0])  +"\"}," +
        "{\"id\":\"warm\",\"v\":\""   + String(spl[1])  +"\"}," +
        "{\"id\":\"yellow\",\"v\":\"" + String(spl[2])  +"\"}," +
        "{\"id\":\"red\",\"v\":\""    + String(spl[3])  +"\"}," +
        "{\"id\":\"green\",\"v\":\""  + String(spl[4])  +"\"}," +
        "{\"id\":\"blue\",\"v\":\""   + String(spl[5])  +"\"}," +
        "{\"id\":\"ch0\",\"v\":\""     + String(spl[6])  +"\"}," +
        "{\"id\":\"ch1\",\"v\":\""     + String(spl[7])  +"\"}," +
        "{\"id\":\"ch2\",\"v\":\""     + String(spl[8])  +"\"}," +
        "{\"id\":\"ch3\",\"v\":\""     + String(spl[9])  +"\"}," +
        "{\"id\":\"ch4\",\"v\":\""     + String(spl[10]) +"\"}," +
        "{\"id\":\"ch5\",\"v\":\""     + String(spl[11]) +"\"}]"
    );
}

String responseTrlo() {
    char *spl[24];
    //ESPserial.print("data:");
    //ESPserial.println(serialRxBuffer);
    int c = split(serialRxBuffer, '\t', spl, sizeof(spl));
    return String(
        "[{\"id\":1, \"h\":\""+ String(spl[0]) +"\",\"m\":\"" + String(spl[1]) + "\"}," +
        "{\"id\":2, \"h\":\""+ String(spl[2])  +"\",\"m\":\"" + String(spl[3]) + "\"}," +
        "{\"id\":3, \"h\":\""+ String(spl[4])  +"\",\"m\":\"" + String(spl[5]) + "\"}," +
        "{\"id\":4, \"h\":\""+ String(spl[6])  +"\",\"m\":\"" + String(spl[7]) + "\"}," +
        "{\"id\":5, \"h\":\""+ String(spl[8])  +"\",\"m\":\"" + String(spl[9]) + "\"}," +
        "{\"id\":6, \"h\":\""+ String(spl[10]) +"\",\"m\":\"" + String(spl[11]) + "\"}," +
        "{\"id\":7, \"h\":\""+ String(spl[12]) +"\",\"m\":\"" + String(spl[13]) + "\"}," +
        "{\"id\":8, \"h\":\""+ String(spl[14]) +"\",\"m\":\"" + String(spl[15]) + "\"}," +
        "{\"s\":[\"" +
        String(spl[16]) + "\",\"" + String(spl[17]) + "\",\"" +
        String(spl[18]) + "\",\"" + String(spl[19]) + "\",\"" +
        String(spl[20]) + "\",\"" + String(spl[21]) + "\",\"" +
        String(spl[22]) + "\",\"" + String(spl[23]) + "\"]}]"
    );
}

String responseTisa() {
    return String(
        "{\"dt\":\"" + String(serialRxBuffer) + "\"}"
    );
}

String responseCosa() {
    return String(
        "{\"dt\":\"" + String(serialRxBuffer) + "\"}"
    );
}

String responseFdsa() {
    return String(
        "{\"dt\":\"" + String(serialRxBuffer) + "\"}"
    );
}

String responseTrsa() {
    return String(
        "{\"dt\":\"" + String(serialRxBuffer) + "\"}"
    );
}

String responseLise() {
    return String(
        "{\"dt\":\"" + String(serialRxBuffer) + "\"}"
    );
}

void handleRead() {
    String params = server.arg("p");
    String value = server.arg("v");

    command = find(const_cast<char*>(params.c_str()));
    char buf[8];
    sprintf(buf, "%d\t%s\t\n", command, value.c_str());
    Serial.print(buf);
    Serial.flush(); // wait for a serial string to be finished sending

    if (command > 2) {
        if (!SERIALREAD()) {
            server.setContentLength(2);
            server.send(204, "application/json", "{}");
        }
    }

    String ret;
    switch (command) {
    case HOLO:
        ret = responseHolo();
        break;
    case TILO:
        ret = responseTilo();
        break;
    case COLO:
        ret = responseColo();
        break;
    case FDLO:
        ret = responseFdlo();
        break;
    case LILO:
        ret = responseLilo();
        break;
    case TRLO:
        ret = responseTrlo();
        break;

    case TISA:
        ret = responseTisa();
        break;
    case COSA:
        ret = responseCosa();
        break;
    case FDSA:
        ret = responseFdsa();
        break;
    case TRSA:
        ret = responseFdsa();
        break;
    case LISE:
        ret = responseLise();
        break;
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

    server.on("/read.php", handleRead);

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
