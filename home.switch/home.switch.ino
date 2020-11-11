/**
 *
 * https://www.onetransistor.eu/2019/04/https-server-on-esp8266-nodemcu.html
 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <SPI.h>
#include <UTFT-ESP.h>
#include <XPT2046_Touchscreen.h>
#include <Ticker.h>
#include "images.h"


#define LED 16
#define ANALOG_PIN 3
#define TOUCH_IRQ 0

extern uint8_t BigFont[];//SmallFont[];

Ticker timer0;
ESP8266WebServer server(80);

volatile byte lightState = 0;
volatile byte counterLed = 0;

const char *ssid = "KWIFI";
const char *password = "Heslicko12";

char str[40];

// Modify the line below to match your display and wiring:
UTFT lcd (ILI9341_S5P, 15, 255, 2);

XPT2046_Touchscreen touch(1, 0);
//BearSSL::ESP8266WebServerSecure server(443);
ESP8266WebServer serverHTTP(80);

void displayOff() {
    digitalWrite(LED, LOW);
}

void displayOn() {
    digitalWrite(LED, HIGH);
    counterLed = 0;
}

/*
void wifiConnect() {

    // Connect to WiFi network
    WiFi.config("192.168.0.156", "192.168.0.1", "255.255.255.0");
    WiFi.mode(WIFI_STA);
    WiFi.begin("KWIFI", "Heslicko12");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

String handleResponse() {
    return String(
        "{\"st\":\"OK\"}"
    );
}

void handleSet() {
    String params = server.arg("p");
    handleResponse();
    server.setContentLength(ret.length());
    server.send(200, "application/json", ret);
}
*/
void timerLed() {
    counterLed++;
    if (counterLed > 5) {
        displayOff();
        counterLed = 0;
    }
}


void drawBulb() {
    if (lightState == 0) {
        lcd.drawBitmap(55, 85, 128, 128, light, 1, 0x528A);
        analogWrite(ANALOG_PIN, 0);

        lcd.setColor (0, 0, 0);
        lcd.fillRect(0, 280, 240, 320);
    } else {
        lcd.drawBitmap(55, 85, 128, 128, light, 1, 0xFFFF);
        analogWrite(ANALOG_PIN, 1023);
    }
}

void drawHome() {
    lcd.clrScr();
    lcd.setBackColor(0, 0, 0);
    lcd.setColor (80, 80, 80);
    lcd.drawLine(0, 279, 240, 279);
    drawBulb();
}


uint16_t getX(TS_Point p) {
    return map(p.y, 240, 3823, 240, 0);
}

uint16_t getY(TS_Point p) {
    return map(p.x, 345, 3870, 320, 0);
}

void setup() {
    Serial.begin(115200);

    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    // Setup the LCD
    lcd.InitLCD (0);
    lcd.setFont(BigFont);

    touch.begin();  // Must be done before setting rotation
    touch.setRotation(1);
    drawHome();

    //SPIFFS.begin();
    //wifiConnect();

    //server.on("/read.php", handleSet);
    //server.begin();

    timer0.attach(1, timerLed);
}


uint16_t x, y, lastX = 0;
void loop () {
    /*
    if (touch.touched()) {
        TS_Point p = touch.getPoint();
        x = getX(p);
        y = getY(p);
        displayOn();

        if (y < 250) {
            lightState = !lightState;
            drawBulb();
        } else {

            if (lightState == 1 && x > 20 && x <= 195) {
                lcd.setColor (0, 0, 0);
                lcd.drawHLine(0, 300, lastX - 5);
                lcd.drawCircle(lastX, 300, 7);

                lcd.setColor (255, 255, 255);
                lcd.drawCircle(x, 300, 7);
                lcd.drawHLine(0, 300, x - 5);

                lcd.setBackColor(0, 0, 0);
                lcd.setColor (255, 255, 255);
                lcd.printNumI(map(x, 20, 195, 0, 100), 200, 290);

                lastX = x;
                analogWrite(3, map(x, 20, 195, 0, 1023) );
            }
        }
        //sprintf (str, "X:%d Y:%d.", x, y);
        //lcd.setColor (255, 255, 255);
        //lcd.print(str, LEFT, 10);
        //Serial.print(", x = ");Serial.print(p.x);Serial.print(", y = ");Serial.println(p.y);
    }
    */
}
