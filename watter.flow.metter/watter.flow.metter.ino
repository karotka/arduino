#include <Wire.h>
#include "U8glib.h"
#include "RTClib.h"
#include "configvalues.h"
#include "logo.h"

#define DELAY 10
#define RETURN_TIMEOUT 10

unsigned int returnStatus = 0;
float frac;
int x = 0;
int y = 0;

int *minute;
int *hour;
int *st;

volatile unsigned int delayCount = 0;
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
    SET_T1_HOUR,
    SET_T1_MIN,
    SET_T1_ST,
    SET_T2_HOUR,
    SET_T2_MIN,
    SET_T2_ST,
    SET_T3_HOUR,
    SET_T3_MIN,
    SET_T3_ST,
    SET_T4_HOUR,
    SET_T4_MIN,
    SET_T4_ST,
    SET_T5_HOUR,
    SET_T5_MIN,
    SET_T5_ST,
    SET_T6_HOUR,
    SET_T6_MIN,
    SET_T6_ST,
    SET_T7_HOUR,
    SET_T7_MIN,
    SET_T7_ST,
    SET_T8_HOUR,
    SET_T8_MIN,
    SET_T8_ST,
    SET_T9_HOUR,
    SET_T9_MIN,
    SET_T9_ST,
    SET_END
};

enum {
    PAGE_FIRST = 0,
    PAGE_SECOND
};

const char *status[3] = {"OFF", "ON", "NO"};

byte setDateStatus = SET_NONE;
byte sensorInterrupt = 0;  // 0 = digital pin 2
byte page = PAGE_FIRST;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);
RTC_DS3231 rtc;
DateTime now;
ConfigValues_t configValues;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 9;
float flowRate;
float flowMilliLitres;
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
    //pulseCount++;

    if (setDateStatus != SET_NONE) {
        returnStatus++;
    }
    if (returnStatus > RETURN_TIMEOUT) {
        page = PAGE_FIRST;
        setDateStatus = SET_NONE;
        returnStatus = 0;
        x = 0;
        y = 0;
        configValues.save();
    }
}

/**
 * Check if actual time is bettween timer values
 * If yes, switch lights to right state
 **/
void checkTimer() {

    int minutes, nextMinutes, realMinute;
    realMinute = now.minute() + (now.hour() * 60);

    for(uint8_t i = 0; i < 9; i++) {
        uint8_t j = i + 1;
        if (i == 9) { j = 0; }

        minutes     = configValues.minutes[i] + (configValues.hours[i] * 60);
        nextMinutes = configValues.minutes[j] + (configValues.hours[j] * 60);

        if (minutes <= realMinute && realMinute < nextMinutes) {
            if (configValues.statuses[i] == 1) delayCount = DELAY;
            break;
        }
    }
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

    pulseCount      = 0;
    flowRate        = 0.0;
    flowMilliLitres = 0;
    oldTime         = 0;

    // Configured to trigger on a FALLING state change
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

    setTimer1();

    u8g.firstPage();
    do {
        u8g.drawXBMP(0, 0, u8g_logo_width, u8g_logo_height, u8g_logo_bits);
    } while( u8g.nextPage() );

    delay(2000);

    configValues.load();
}

void draw(void) {
    if((millis() - oldReadTime) > 500) {
        rtc.begin();
        now = rtc.now();
        oldReadTime = millis();
        ddot = !ddot;
        checkTimer();
    }

    u8g.setFont(u8g_font_unifont_0_8);
    //u8g.setFont(u8g_font_osb21);
    //u8g.setFont(u8g_font_04b_03br);

    if (page == PAGE_FIRST) {
        switch (setDateStatus) {
        case SET_A:
            u8g.drawFrame(32,  0, 68, 12);
            break;
        case SET_B:
            u8g.drawFrame(32, 13, 68, 12);
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

        dtostrf(configValues.totalLitresA, 7, 2, fl);
        sprintf(buf, "MeA: %s Ltr", fl);
        u8g.drawStr(0, 11, buf);

        dtostrf(configValues.totalLitresB, 7, 2, fl);
        sprintf(buf, "MeB: %s Ltr", fl);
        u8g.drawStr(0, 24, buf);

        dtostrf(configValues.totalLitres, 7, 2, fl);
        sprintf(buf, "Tot: %s Ltr", fl);
        u8g.drawStr(0, 37, buf);

        if (delayCount > 1) {
            sprintf(buf, "Rel: ON");
            digitalWrite(3, HIGH);
        } else {
            sprintf(buf, "Rel: OFF");
            digitalWrite(3, LOW);
        }
        u8g.drawStr(0, 50, buf);

        //dtostrf(frac, 3, 1, fl);
        //sprintf(buf, "F:%s/s", fl);
        //u8g.drawStr(70, 50, buf);

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

    } else if (page == PAGE_SECOND) {

        for (int i = 0; i < 9; i++) {
            sprintf(buf, "T%d-%02d:%02d %s", i + 1,
                    configValues.hours[i], configValues.minutes[i],
                    status[configValues.statuses[i]]);
            u8g.drawStr(0, x + 11 + (i * 13), buf);
        }

        if (y == 0) {
            u8g.drawFrame(23, 0, 18, 12);
        }
        if (y == 1) {
            u8g.drawFrame(47, 0, 18, 12);
        }
        if (y == 2) {
            u8g.drawFrame(71, 0, 27, 12);
        }

    }
    //sprintf(buf, "%d", x);
    //u8g.drawStr(70, 49, buf);
}

void setHour(int *hour) {
    if (digitalRead(9) == LOW) {
        *hour = *hour - 1;
        returnStatus = 0;
    }
    if (digitalRead(10) == LOW) {
        *hour = *hour + 1;
        returnStatus = 0;
    }
    if (*hour > 23) *hour = 0;
    if (*hour < 0) *hour = 23;
}

void setMinute(int *minute) {
    if (digitalRead(9) == LOW) {
        *minute = *minute - 1;
        returnStatus = 0;
    }
    if (digitalRead(10) == LOW) {
        *minute = *minute + 1;
        returnStatus = 0;
    }
    if (*minute > 59) *minute = 0;
    if (*minute < 0) *minute = 59;
}

void setStatus(int *st) {
    if (digitalRead(9) == LOW || digitalRead(10) == LOW) {
        *st = *st + 1;
        if (*st == 3) *st = 0;
        returnStatus = 0;
    }
}

void loop(void) {
    if (delayCount > 0) {
        delayCount--;
    }

    if((millis() - oldTime) > 1000) {    // Only process counters once per second

        detachInterrupt(sensorInterrupt);
        flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
        oldTime = millis();

        flowMilliLitres = flowRate / 60;
        configValues.totalLitres  += flowMilliLitres;
        configValues.totalLitresA += flowMilliLitres;
        configValues.totalLitresB += flowMilliLitres;
        frac = (flowRate - int(flowRate)) * 10;
        pulseCount = 0;

        attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    }

    // buttons
    if (digitalRead(8) == LOW) {
        returnStatus = 0;
        setDateStatus++;
        if (setDateStatus > SET_T1_HOUR) {
            y++;
            if (y == 3) {
                y = 0;
                x = x - 13;
            }
        }
        if (setDateStatus == SET_T1_HOUR) {
            page = PAGE_SECOND;
        }
        if (setDateStatus == SET_END) {
            setDateStatus = SET_NONE;
            page = PAGE_FIRST;
            x = 0;
        }
        delay(100);
    }

    switch (setDateStatus) {
    case SET_A:
        if (digitalRead(10) == LOW) {
            configValues.totalLitresA = 0;
        }
        break;

    case SET_B:
        if (digitalRead(10) == LOW) {
            configValues.totalLitresB = 0;
        }
        break;

    case SET_HOUR:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour() - 1, now.minute()));
            returnStatus = 0;
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour() + 1, now.minute()));
            returnStatus = 0;
        }
        break;

    case SET_MINUTE:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour(), now.minute() - 1));
            returnStatus = 0;
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day(),
                now.hour(), now.minute() + 1));
            returnStatus = 0;
        }
        break;

    case SET_DAY:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day() - 1,
                now.hour(), now.minute()));
            returnStatus = 0;
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month(), now.day() + 1,
                now.hour(), now.minute()));
            returnStatus = 0;
        }
        break;

    case SET_MONTH:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month() - 1, now.day(),
                now.hour(), now.minute()));
            returnStatus = 0;
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year(), now.month() + 1, now.day(),
                now.hour(), now.minute()));
            returnStatus = 0;
        }
        break;

    case SET_YEAR:
        if (digitalRead(9) == LOW) {
            rtc.adjust( DateTime(
                now.year() - 1, now.month(), now.day(),
                now.hour(), now.minute()));
            returnStatus = 0;
        }
        if (digitalRead(10) == LOW) {
            rtc.adjust( DateTime(
                now.year() + 1, now.month(), now.day(),
                now.hour(), now.minute()));
            returnStatus = 0;
        }
        break;

     case SET_T1_HOUR:
         hour = &configValues.hours[0];
         setHour(hour);
         break;
     case SET_T2_HOUR:
         hour = &configValues.hours[1];
         setHour(hour);
         break;
     case SET_T3_HOUR:
         hour = &configValues.hours[2];
         setHour(hour);
         break;
     case SET_T4_HOUR:
         hour = &configValues.hours[3];
         setHour(hour);
         break;
     case SET_T5_HOUR:
         hour = &configValues.hours[4];
         setHour(hour);
         break;
     case SET_T6_HOUR:
         hour = &configValues.hours[5];
         setHour(hour);
         break;
     case SET_T7_HOUR:
         hour = &configValues.hours[6];
         setHour(hour);
         break;
     case SET_T8_HOUR:
         hour = &configValues.hours[7];
         setHour(hour);
         break;
     case SET_T9_HOUR:
         hour = &configValues.hours[8];
         setHour(hour);
         break;

    case SET_T1_MIN:
        minute = &configValues.minutes[0];
        setMinute(minute);
        break;
    case SET_T2_MIN:
        minute = &configValues.minutes[1];
        setMinute(minute);
        break;
    case SET_T3_MIN:
        minute = &configValues.minutes[2];
        setMinute(minute);
        break;
    case SET_T4_MIN:
        minute = &configValues.minutes[3];
        setMinute(minute);
        break;
    case SET_T5_MIN:
        minute = &configValues.minutes[4];
        setMinute(minute);
        break;
    case SET_T6_MIN:
        minute = &configValues.minutes[5];
        setMinute(minute);
        break;
    case SET_T7_MIN:
        minute = &configValues.minutes[6];
        setMinute(minute);
        break;
    case SET_T8_MIN:
        minute = &configValues.minutes[7];
        setMinute(minute);
        break;
    case SET_T9_MIN:
        minute = &configValues.minutes[8];
        setMinute(minute);
        break;

    case SET_T1_ST:
        st = &configValues.statuses[0];
        setStatus(st);
        break;

    case SET_T2_ST:
        st = &configValues.statuses[1];
        setStatus(st);
        break;

    case SET_T3_ST:
        st = &configValues.statuses[2];
        setStatus(st);
        break;

    case SET_T4_ST:
        st = &configValues.statuses[3];
        setStatus(st);
        break;

    case SET_T5_ST:
        st = &configValues.statuses[4];
        setStatus(st);
        break;

    case SET_T6_ST:
        st = &configValues.statuses[5];
        setStatus(st);
        break;

    case SET_T7_ST:
        st = &configValues.statuses[6];
        setStatus(st);
        break;

    case SET_T8_ST:
        st = &configValues.statuses[7];
        setStatus(st);
        break;

    case SET_T9_ST:
        st = &configValues.statuses[8];
        setStatus(st);
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
