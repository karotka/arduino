// Include libraries for the 8x32 LED matrix
#include "configure.h"
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <gamma.h>
#include <Dimmer.h>
#include "names.cz.h"

// Define the pin which is used as DataIn on the LED matrix
#define MATRIX_PIN  6
#define MATRIX_COLS 32
#define MATRIX_ROWS 8

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_COLS, MATRIX_ROWS,  MATRIX_PIN,
  NEO_MATRIX_BOTTOM  + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

RTC_DS3231 rtc;
Dimmer d(A1);
DateTime now;

volatile int dateSteps;
volatile int show = SHOW_TIME;
volatile int switchTimer = 20;
volatile byte dot;

String chars = "";

int x = 1;
unsigned long oldTime;


void setTimer1();

void setup() {
    // initialize the LED matrix
    matrix.begin();
    // set the brightness; max = 255
    matrix.setBrightness(10);
    matrix.setTextWrap(false);
    matrix.setTextColor(blue);

    // Serial.begin(9600);
    d.begin();

    rtc.begin();
    //rtc.adjust(DateTime(2017, 7, 28, 22, 26, 0));

    oldTime = 0;
    setTimer1();
}

void loop() {
    if((millis() - oldTime) > 2000) {
        now = rtc.now();
        matrix.setBrightness(d.dimm(dimtable, NUMDSTEPS));
        oldTime = millis();
    }

    switch (show) {
    case SHOW_TIME:
        matrix.setCursor(1, 0);
        matrix.fillScreen(0);
        matrix.print(chars);
        matrix.show();
        break;

    case SHOW_DATE:
        matrix.fillScreen(0);
        if (x < -10) {
            matrix.setCursor(x + 10, 0);
        } else {
            matrix.setCursor(1, 0);
        }
        if (x > -dateSteps) {
            x--;
        }
        matrix.print(chars);
        matrix.show();
        delay(100);
        break;
    }
}

void setTimer1() {
    // initialize timer1
    noInterrupts();           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    OCR1A = 31250;            // compare match register 16MHz/256/2Hz
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
    interrupts();             // enable all interrupts
}

ISR(TIMER1_COMPA_vect) {          // timer compare interrupt service routine
    switchTimer++;
    if (switchTimer > 20) {
        switchTimer = 0;
        if (show == SHOW_TIME) {
            show = SHOW_DATE;
            x = 1;
        } else {
            show = SHOW_TIME;
        }
    }

    if (dot) {
        dot = 0;
    } else {
        dot = 1;
    }
    chars = "";
    switch (show) {
    case SHOW_TIME:
        if (now.hour() < 10) {
            chars += " ";
        }
        chars += now.hour();
        if (dot) {
            chars += " ";
        } else {
            chars += ":";
        }
        if (now.minute() < 10) {
            chars += 0;
        }
        chars += now.minute();
        break;

    case SHOW_DATE:
        chars += daysOfTheWeek[now.dayOfTheWeek()];
        chars += " ";
        chars += now.day();
        chars += ".";
        chars += now.month();
        chars += ". ";
        //dayOfTheYear = now.dayOfTheYear();
        //strcpy_P(buffer, (char*)pgm_read_word(&(names[2])));
        //chars += buffer;
        dateSteps = (chars.length() * 6) - MATRIX_COLS + 4;
        break;
    }
}
