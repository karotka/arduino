#include <Wire.h>
#include "U8glib.h"
#include "RTClib.h"

#define DELAY 10

volatile unsigned int delayCount;
volatile byte pulseCount;
enum {
    SET_NONE = 0,
    SET_A,
    SET_B,
    SET_HOUR,
    SET_MINUTE,
    SET_DAY,
    SET_MONTH,
    SET_YEAR,
    SET_END
};

byte setDateStatus = SET_NONE;
byte sensorInterrupt = 0;  // 0 = digital pin 2

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);
RTC_DS3231 rtc;
DateTime now;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long totalMilliLitresA;
unsigned long totalMilliLitresB;
unsigned long oldReadTime;
unsigned long oldTime;

bool ddot;

char buf[20];
char fl[9];

void setTimer1();

/*
 * Insterrupt Service Routine
 */
void pulseCounter() {
    pulseCount++;
    delayCount = DELAY;
}

ISR(TIMER1_COMPA_vect) {  // timer compare interrupt service routine
    // Increment the pulse counter
    pulseCount++;
    delayCount = DELAY;
}

void setup(void) {
    rtc.begin();

    u8g.setColorIndex(1); // pixel on
    u8g.setHiColorByRGB(255,255,255);

    pinMode(2, INPUT); // flow meter senzor
    pinMode(8, INPUT); // select button
    pinMode(9, INPUT); // decrease button
    pinMode(10,INPUT); // increase button

    pinMode(3, OUTPUT);

    digitalWrite(2, HIGH);

    pulseCount         = 0;
    flowRate           = 0.0;
    flowMilliLitres    = 0;
    totalMilliLitres   = 0;
    totalMilliLitresA  = 0;
    totalMilliLitresB  = 0;
    oldTime            = 0;

    // Configured to trigger on a FALLING state change
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

    setTimer1();
}

void myRound(float * value) {
    *value = *value * 10.0f;
    *value = *value > 0.0f ? floor(*value + 0.5f) : ceil(*value - 0.5f);
    *value = *value / 10.0f;
}

void draw(void) {
    if((millis() - oldReadTime) > 500) {
        rtc.begin();
        now = rtc.now();
        oldReadTime = millis();
        ddot = !ddot;
    }

    u8g.setFont(u8g_font_unifont_0_8);
    //u8g.setFont(u8g_font_osb21);
    //u8g.setFont(u8g_font_04b_03br);

    switch (setDateStatus) {
    case SET_A:
        u8g.drawFrame(38,  0, 65, 12);
        break;
    case SET_B:
        u8g.drawFrame(38, 13, 65, 12);
        break;
    case SET_HOUR:
        u8g.drawHLine( 0, 52, 15);
        break;
    case SET_MINUTE:
        u8g.drawHLine(25, 52, 15);
        break;
    case SET_DAY:
        u8g.drawHLine(49, 52, 15);
        break;
    case SET_MONTH:
        u8g.drawHLine(73, 52, 15);
        break;
    case SET_YEAR:
        u8g.drawHLine(97, 52, 30);
    }

    dtostrf(totalMilliLitresA, 7, 1, fl);
    sprintf(buf, "A  : %s L", fl);
    u8g.drawStr(0, 11, buf);

    dtostrf(totalMilliLitresB, 7, 1, fl);
    sprintf(buf, "B  : %s L", fl);
    u8g.drawStr(0, 24, buf);

    dtostrf(totalMilliLitres, 7, 1, fl);
    sprintf(buf, "Tot: %s L", fl);
    u8g.drawStr(0, 37, buf);

    if (delayCount < 1) {
        sprintf(buf, "UV : ON");
    } else {
        sprintf(buf, "UV : OFF");
    }
    u8g.drawStr(0, 50, buf);

    //sprintf(buf, "%d-%d-%d", digitalRead(8), digitalRead(9), digitalRead(10) );
    //u8g.drawStr(70, 49, buf);

    if (ddot) {
        sprintf(buf, "%02d:%02d %02d.%02d.%04d", now.hour(), now.minute(),
            /*now.second(),*/ now.day(), now.month(), now.year());
    } else {
        sprintf(buf, "%02d %02d %02d.%02d.%04d", now.hour(), now.minute(),
            /*now.second(),*/ now.day(), now.month(), now.year());
    }
    u8g.drawStr(0, 64, buf);
}

void loop(void) {
    delayCount--;

    if((millis() - oldTime) > 1000) {    // Only process counters once per second

        detachInterrupt(sensorInterrupt);
        flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
        oldTime = millis();

        flowMilliLitres = (flowRate / 60) * 1000;
        totalMilliLitres  += flowMilliLitres;
        totalMilliLitresA += flowMilliLitres;
        totalMilliLitresB += flowMilliLitres;
        //unsigned int frac;
        //frac = (flowRate - int(flowRate)) * 10;
        pulseCount = 0;

        attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    }

    if (delayCount < 1) {
        digitalWrite(3, HIGH);
    } else {
        digitalWrite(3, LOW);
    }

    // buttons
    if (digitalRead(8) == LOW) {
        setDateStatus++;
        if (setDateStatus == SET_END) {
            setDateStatus = SET_NONE;
        }
        delay(100);
    }

    switch (setDateStatus) {
    case SET_A:
        if (digitalRead(10) == LOW) {
            totalMilliLitresA = 0;
        }
        break;

    case SET_B:
        if (digitalRead(10) == LOW) {
            totalMilliLitresB = 0;
        }
        break;

    case SET_HOUR:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour() - 1, now.minute()));
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour() + 1, now.minute()));
        }
        break;

    case SET_MINUTE:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour(), now.minute() - 1));
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour(), now.minute() + 1));
        }
        break;

    case SET_DAY:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day() - 1,
                now.hour(), now.minute()));
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day() + 1,
                now.hour(), now.minute()));
        }
        break;

    case SET_MONTH:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month() - 1, now.day(),
                now.hour(), now.minute()));
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month() + 1, now.day(),
                now.hour(), now.minute()));
        }
        break;

    case SET_YEAR:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year() - 1, now.month(), now.day(),
                now.hour(), now.minute()));
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year() + 1, now.month(), now.day(),
                now.hour(), now.minute()));
        }
        break;
    }

    // picture loop
    u8g.firstPage();
    do {
        draw();
    } while( u8g.nextPage() );
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
