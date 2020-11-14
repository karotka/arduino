#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include <FS.h>
#include "EEPROM.h"
#include <RtcDS3231.h>
#include <Ticker.h>
#include <Arduino_JSON.h>

Config_t config;

ESP8266WebServer server(80);
RtcDS3231<TwoWire> rtc(Wire);
RtcDateTime ct;

volatile int timerCouterRtc = 0;
volatile int timerCouterDimmer = 0;
volatile bool switchProcess = false;

int finalPWMvalue;
Ticker timer0;

const int PWM_PIN = 13; // 12 D6, 13 D7, 14 D5, 16 D0

typedef struct {
    int hour;
    int minute;
    int value[4];
} period;

period timer[6] = {
    {7, 0,  {1023, 0, 0, 0}},
    {12, 0, {600, 0, 0, 0}},
    {13, 0, {1023, 0, 0, 0}},
    {18, 0, {500, 0, 0, 0}},
    {19, 0, {100, 0, 0, 0}},
    {20, 0, {0, 0, 0, 0}}
};

int pwm_values[17] = {0};

void anWrite(uint8_t pin, int value) {
    pwm_values[pin] = value;
    analogWrite(pin, 1023 - value);
}

void EEPROM_saveTimer() {
    int address = 18;
    Serial.println(address);
    EEPROM.put(address, timer);
    address += sizeof(timer);
    Serial.println(address);
    EEPROM.commit();
}

void EEPROM_loadTimer() {
    int address = 18;
    EEPROM.get(address, timer);
    address += sizeof(timer);
    Serial.println(address); //162
    EEPROM.commit();
}

void handleSaveTimer() {
    String data = server.arg("plain");
    JSONVar o = JSON.parse(data);

    if (o.hasOwnProperty("timer")) {
        for (int i = 0; i < o["timer"].length(); i++) {
            timer[i].hour = static_cast<int>(o["timer"][i]["th"]);
            timer[i].minute = static_cast<int>(o["timer"][i]["tm"]);

            for (int j = 0; j < o["timer"][i]["tv"].length(); j++) {
                timer[i].value[j] = static_cast<int>(o["timer"][i]["tv"][j]);
            }
        }
    }
    EEPROM_saveTimer();
    server.send(200, "text/plain", "OK");
}

void switchLight(uint8_t index) {

    finalPWMvalue = timer[index].value[0];

    if (pwm_values[PWM_PIN] == finalPWMvalue) {
        switchProcess = false;
        return;
    } else {
        switchProcess = true;
    }

    if (switchProcess) {
        // then --
        if (pwm_values[PWM_PIN] > finalPWMvalue) {
            anWrite(PWM_PIN, --pwm_values[PWM_PIN]);
        // then ++
        } else
        if (pwm_values[PWM_PIN] < finalPWMvalue) {
            anWrite(PWM_PIN, ++pwm_values[PWM_PIN]);
        }
    }
}

void checkTimer() {

    int minutes, nextMinutes, realMinute;
    realMinute = ct.Minute() + (ct.Hour() * 60);

    for(uint8_t i = 0; i < 6; i++) {
        uint8_t j = i + 1;
        if (i == 5) { j = 0; }

        minutes     = timer[i].minute + (timer[i].hour * 60);
        nextMinutes = timer[i].minute + (timer[j].hour * 60) + (i==5 ? 1400 : 0);

        if (minutes <= realMinute && realMinute < nextMinutes) {
            switchLight(i);
            break;
        }
    }
}

void wifiConnect() {

    // Connect to WiFi network
    //Serial.println("");
    //Serial.print("Connecting to: ");
    //Serial.print(config.ssid);

    WiFi.config(config.ip, config.gateway, config.subnet);
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    bool st = true;
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        //Serial.print(".");
        if (st) digitalWrite(LED_BUILTIN, HIGH);
        else digitalWrite(LED_BUILTIN, LOW);
        st = !st;
    }
    /*
    Serial.println("");
    Serial.print("WiFi connected: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP().toString());
    Serial.println("/");
    */
    digitalWrite(LED_BUILTIN, LOW);
}

void wifiAp() {
    IPAddress apIp(192, 168, 10, 1);
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

String getTime() {
    char str[3];

    String ret =
        "{\"time\":\"" + String(ct.Hour()) + ":";

    sprintf(str, "%02d", ct.Minute());
    ret +=
        String(str) + ":";

    sprintf(str, "%02d", ct.Second());
    ret +=
        String(str) + "\","
        "\"date\":\"" + String(ct.Day()) + "." +
        String(ct.Month()) + "." +
        String(ct.Year()) + "\"";

    return ret;
}

void handleTime() {
    String ret = getTime();
    ret += "}";

    server.setContentLength(ret.length());
    server.send(200, "text/json", ret);
}

void handleTimerData() {

    String ret = "{\"timer\":[";
    for (int i = 0; i < 6; i++) {
        ret +=
            "{\"th\":" + String(timer[i].hour) + ","
            "\"tm\":" + String(timer[i].minute) + ",\"tv\":[";

        for (int j = 0; j < 4; j++) {
            ret += String(timer[i].value[j]);
            if (j < 3) { ret += ","; }
        }
        ret += "]}";

        if (i < 5) { ret += ","; }
    }
    ret += "]}";

    server.setContentLength(ret.length());
    server.send(200, "text/json", ret);
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

void handleValue() {
    unsigned int channel1 = server.arg("v").toInt();

    anWrite(PWM_PIN, channel1);

    server.send(200, "text/plain", "OK");
}

void timer0run() {
    timerCouterRtc++;
    timerCouterDimmer++;
}

void setup() {

    Serial.begin(115200);
    delay(10);

    SPIFFS.begin();
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    if (config.apMode) {
        wifiAp();
    } else {
        wifiConnect();
    }

    // Start the server
    server.on("/", handleRoot);
    server.on("/timerData", handleTimerData);
    server.on("/time", handleTime);
    server.on("/styles.css", handleCss);
    server.on("/saveTimer", handleSaveTimer);
    server.on("/connect", handleConnect);
    server.on("/setup", handleSetup);
    server.on("/setupData", handleSetupData);
    server.on("/saveData", handleSaveData);
    server.on("/handleValue", handleValue);

    server.on("/apmode", handleAPmode);

    server.begin();
    Serial.println("Server started");

    Serial.println("Read eeprom: ");
    EEPROM.begin(EEPROM_SIZE);
    EEPROM_loadTimer();

    pinMode(13, OUTPUT); // D7
    pinMode(12, OUTPUT); // D6
    pinMode(14, OUTPUT); // D5
    pinMode(16, OUTPUT); // D0
    analogWriteFreq(5000);

    rtc.Begin();

    timer0.attach(0.1, timer0run);
    //RtcDateTime currentTime = RtcDateTime(20, 11, 6, 19, 40, 0);
    //rtc.SetDateTime(currentTime);
}

void loop() {
    server.handleClient();

    if (timerCouterRtc > 3) {
        timerCouterRtc = 0;
        ct = rtc.GetDateTime();
    }
    if (timerCouterDimmer > 1) {
        timerCouterDimmer = 0;
        checkTimer();
    }
}
