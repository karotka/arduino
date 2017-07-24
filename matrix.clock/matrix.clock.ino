// Include libraries for the 8x32 LED matrix
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <gamma.h>
#include <Adafruit_NeoPixel.h>

// Define the pin which is used as DataIn on the LED matrix
#define MATRIX_PIN 6


Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8,  MATRIX_PIN,
  NEO_MATRIX_BOTTOM  + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// define some color codes
#define white 0xFFFF
#define green 0x07E0
#define red 0xF800
#define cyan 0x07FF
#define magenta 0xF81F
#define yellow 0xFFE0
#define blue 0x001F

int value;

void setup() {
    // initialize the LED matrix
    matrix.begin();

    // set the brightness; max = 255
    matrix.setBrightness(60);

    // Display "compu" on the LED matrix to indicate the computation of e
    matrix.setTextWrap(false);

    Serial.begin(9600);

    // display some advertisment
    matrix.setTextWrap(false);

    matrix.setTextColor(red);

    pinMode(A1, INPUT);

    rtc.begin();
}


byte dot;
int dimmer;

void loop() {
    String chars = "";

    DateTime now = rtc.now();
    if (now.hour() < 10) {
        chars += " ";
    }
    chars += now.hour();
    if (dot) {
        chars += " ";
        dot = 0;
    } else {
        chars += ":";
        dot = 1;

    }
    if (now.minute() < 10) {
        chars += 0;
    }
    chars += now.minute();

    value = analogRead(A1);

    if (value <= 600) {
        dimmer = 100;
    } else
    if (value > 600 && value <= 650) {
        dimmer = 80;
    } else
    if (value > 700 && value <= 750) {
        dimmer = 60;
    } else
    if (value > 750 && value <= 800) {
        dimmer = 50;
    } else
    if (value > 800 && value <= 850) {
        dimmer = 40;
    } else
    if (value > 850 && value <= 950) {
        dimmer = 30;
    } else
    if (value > 950 && value <= 1000) {
        dimmer = 20;
    } else
    if (value >= 1000) {
        dimmer = 5;
    }

    matrix.fillScreen(0);
    matrix.setBrightness(dimmer);

    matrix.setCursor(1, 0);
    //matrix.print(value);
    matrix.print(chars);
    matrix.show();


    delay(500);

}
