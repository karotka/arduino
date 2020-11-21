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
Ticker timer0;

volatile unsigned int timerCouterRtc = 0;
volatile unsigned int timerCouterDimmer = 0;
volatile unsigned int status;
volatile bool switchProcess = false;

//const int PWM_PIN = 13; // 12 D6, 13 D7, 14 D5, 16 D0
const int pwmPins[4] = {13, 12, 14, 16};

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
    //Serial.println(address);
    EEPROM.put(address, timer);
    address += sizeof(timer);
    //Serial.println(address);
    EEPROM.commit();
}

void EEPROM_loadTimer() {
    int address = 18;
    EEPROM.get(address, timer);
    address += sizeof(timer);
    //Serial.println(address); //162
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

void switchLight(uint8_t index, uint8_t i) {

    int finalPWMvalue = timer[index].value[i];
    int pwmPin = pwmPins[i];

    if (pwm_values[pwmPin] == finalPWMvalue) {
        switchProcess = false;
        return;
    } else {
        switchProcess = true;
    }

    if (switchProcess) {
        // then --
        if (pwm_values[pwmPin] > finalPWMvalue) {
            anWrite(pwmPin, --pwm_values[pwmPin]);
        // then ++
        } else
        if (pwm_values[pwmPin] < finalPWMvalue) {
            anWrite(pwmPin, ++pwm_values[pwmPin]);
        }
    }
}

void switchLights(uint8_t index) {
    for (int i = 0; i < 4; i++) {
        switchLight(index, i);
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

        //Serial.print("minute:");Serial.println(minutes);
        //Serial.print("Next minute:");Serial.println(nextMinutes);
        //Serial.print("Real minute:");Serial.println(realMinute);
        if (minutes <= realMinute && realMinute < nextMinutes) {
            switchLights(i);
            break;
        }
    }
}

void wifiConnect() {

    config.load();

    // Connect to WiFi network
    //Serial.println("");
    //Serial.print("Connecting to: ");
    //Serial.print(config.ssid);
    //Serial.println(config.ssid);
    //Serial.println(config.ip);

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
    //Serial.println("");
    //Serial.print("WiFi connected: ");
    //Serial.print("http://");
    //Serial.print(WiFi.localIP().toString());
    //Serial.println("/");
    digitalWrite(LED_BUILTIN, LOW);
    anWrite(LED_BUILTIN, 100);
}

void wifiAp() {
    IPAddress apIp(192, 168, 10, 1);
    //WiFi.softAPConfig(IPAddress(10, 0, 0, 10),
    //                  IPAddress(10, 0, 0, 1),
    //                  IPAddress(255, 255, 255, 0));
    WiFi.softAP("ESP001");
    //WiFi.mode(WIFI_AP_STA);

    delay(500);
    //Serial.println("Setting AP");
    //Serial.println(WiFi.softAPIP());
    //Serial.print("Server MAC address: ");
    //Serial.println(WiFi.softAPmacAddress());
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

void handleTimeSet() {
    File dataFile = SPIFFS.open("/timeset.html", "r");
    server.streamFile(dataFile, "text/html");
    dataFile.close();
}

void handleTimeSave() {
    String date = server.arg("d");
    String time = server.arg("t");

    RtcDateTime currentTime =
        RtcDateTime(
                    date.substring(2, 4).toInt(),
                    date.substring(5, 7).toInt(),
                    date.substring(8, 10).toInt(),
                    time.substring(0, 2).toInt(),
                    time.substring(3, 5).toInt(), 0);
    rtc.SetDateTime(currentTime);

    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
}

void handleSetupData() {
    config.load();

    String ret =
        "{\"ip\" : \""      + config.ip.toString() + "\","
        "\"gateway\" : \""  + config.gateway.toString() + "\","
        "\"subnet\" : \""   + config.subnet.toString() + "\","
        "\"ssid\" : \""     + config.ssid + "\","
        "\"password\" : \"" + config.password + "\","
        "\"ds\" : "         + config.dimmerSpeed + "}";

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
    config.dimmerSpeed = server.arg("ds").toInt();

    config.save();

    server.sendHeader("Location", String("/setup"), true);
    server.send(302, "text/plain", "");
}

void handleValue() {
    unsigned int channel = server.arg("c").toInt();
    unsigned int value = server.arg("v").toInt();

    anWrite(channel, value);

    server.send(200, "text/plain", "OK");
}

void toggleAut() {
    status = server.arg("v").toInt();
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

    //if (config.apMode) {
    //    wifiAp();
    //} else {
        wifiConnect();
        //}

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
    server.on("/toggleAut", toggleAut);
    server.on("/timeset", handleTimeSet);
    server.on("/timesave", handleTimeSave);

    server.on("/apmode", handleAPmode);

    server.begin();
    //Serial.println("Server started");

    //Serial.println("Read eeprom: ");
    EEPROM.begin(EEPROM_SIZE);
    EEPROM_loadTimer();

    pinMode(13, OUTPUT); // D7
    pinMode(12, OUTPUT); // D6
    pinMode(14, OUTPUT); // D5
    pinMode(16, OUTPUT); // D0
    analogWriteFreq(200);

    rtc.Begin();

    timer0.attach(0.01, timer0run);
}

void loop() {
    server.handleClient();

    if (timerCouterRtc > 30) {
        timerCouterRtc = 0;
        ct = rtc.GetDateTime();
    }
    if (timerCouterDimmer > config.dimmerSpeed) {
        timerCouterDimmer = 0;
        checkTimer();
    }
}
