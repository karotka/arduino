/**
 * simple 7 segment cabin clock for Ice Hockey team Hvezda Praha
 * It show time, date and temperarue
 */
#include "configure.h"
#include <Wire.h>
#include <RTClib.h>
#include <DHTlib.h>

RTC_DS1307 RTC;
DateTime now;
NewTime  newTime;
dht DHT;

volatile char digitsc[4];
volatile int position = 0;
volatile int timerCounterSec = 0;
volatile unsigned int timerCounterShow = 0;
volatile int timerCounterBlink = 0;
volatile int showState = SHOW_TIME;
volatile int set = SET_NONE;
volatile unsigned int dot = 0b0000;
volatile unsigned int showDelay = SHOW_TIME_TIME;

unsigned int i = 0;
char str[4];

void pinInit(void) {
    // Port B as Output
    DDRB = 0xff;
    PORTB = 0xff;

    // Port D
    SEVEN_SEGMENT_DDR=0xff;

    // Turn off all segments
    SEVEN_SEGMENT_PORT=0x00;

    PORTB &= ~(1 << PB0);
    PORTB &= ~(1 << PB1);
    PORTB &= ~(1 << PB2);
    PORTB &= ~(1 << PB3);
}

void timer2set(void) {
    TIMSK2 |= (1 << TOIE2);  // Enable Overflow Interrupt Enable
    TCNT2 = 0;               // Initialize Counter

    TCCR2B =  0b0000010;
}

void printChr(char c[]) {
    for(i = 0; i < 4; i++) {
        digitsc[i] = c[i];
    }
}

/**
 * This function writes a char given by n to the display
 * the decimal point is displayed if dp=1
 */
void sevenSegmentChar(char ch, uint8_t dp) {
    switch (ch) {
    case ' ':
        SEVEN_SEGMENT_PORT=0b00000000;
        break;
    case '0':
        SEVEN_SEGMENT_PORT=0b00111111;
        break;
    case 'O':
        SEVEN_SEGMENT_PORT=0b00111111;
        break;
    case '1':
        SEVEN_SEGMENT_PORT=0b00000110;
        break;
    case '2':
        SEVEN_SEGMENT_PORT=0b01011011;
        break;
    case '3':
        SEVEN_SEGMENT_PORT=0b01001111;
        break;
    case '4':
        SEVEN_SEGMENT_PORT=0b01100110;
        break;
    case '5':
        SEVEN_SEGMENT_PORT=0b01101101;
        break;
    case '6':
        SEVEN_SEGMENT_PORT=0b01111101;
        break;
    case '7':
        SEVEN_SEGMENT_PORT=0b00000111;
        break;
    case '8':
        SEVEN_SEGMENT_PORT=0b01111111;
        break;
    case '9':
        SEVEN_SEGMENT_PORT=0b01101111;
        break;
    case 'C':
        SEVEN_SEGMENT_PORT=0b00111001;
        break;
    case '-':
        SEVEN_SEGMENT_PORT=0b01000000;
        break;
    }

    if(dp) {
        // If decimal point should be displayed
        // Make 0th bit Low
        SEVEN_SEGMENT_PORT |= 0b10000000;
    }
}

unsigned int pos = 0;
unsigned int dimmer = 7;

ISR(TIMER2_OVF_vect) {

    if (((pos + 1) % 8) == 0) {

        switch (set) {

        case SET_HOUR:
            showState = SHOW_TIME;
            if (position == 0 || position == 1) {
                if (timerCounterBlink < 1200) {
                    PORTB |= (1 << position);
                }
            }
            break;

        case SET_MINUTE:
            showState = SHOW_TIME;
            if (position == 2 || position == 3) {
                if (timerCounterBlink < 1200) {
                    PORTB |= (1 << position);
                }
            }
            break;

        case SET_DAY:
            showState = SHOW_DATE;
            if (position == 0 || position == 1) {
                if (timerCounterBlink < 1200) {
                    PORTB |= (1 << position);
                }
            }
            break;

        case SET_MONTH:
            showState = SHOW_DATE;
            if (position == 2 || position == 3) {
                if (timerCounterBlink < 1200) {
                    PORTB |= (1 << position);
                }
            }
            break;

        default:
            PORTB |= (1 << position);
            break;
        }

        if (showState == SHOW_TIME) {
            if (timerCounterSec < 2456) {
                dot = 0b0010;
            } else {
                dot = 0b0000;
            }
        }

        if (CHECK_BIT(dot, position)) {
            sevenSegmentChar(digitsc[position], 1);
        } else {
            sevenSegmentChar(digitsc[position], 0);
        }
        position++;
    }

    pos++;
    if ((pos % 8) == dimmer) {
        PORTB &= ~(1 << PB0);
        PORTB &= ~(1 << PB1);
        PORTB &= ~(1 << PB2);
        PORTB &= ~(1 << PB3);
    }

    if (position == 4) {
        position = 0;
    }

    if (showState == SHOW_TIME) {
        timerCounterSec++;
        if (timerCounterSec == 4912) {
            timerCounterSec = 0;
        }
    }

    timerCounterShow++;
    timerCounterBlink++;
    if (set == SET_NONE && timerCounterShow > showDelay) {
        showState++;
        if (showState > SHOW_TEMP) {
            showState = SHOW_TIME;
        }
        timerCounterShow = 0;
    }

    if (timerCounterBlink == 2400) {
        timerCounterBlink = 0;
    }
}

void setup() {
    RTC.begin();
    if (!RTC.isrunning()) {
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }
    pinInit();
    timer2set();
}

void setNewTime() {
    newTime.hour   = now.hour();
    newTime.minute = now.minute();
    newTime.day    = now.day();
    newTime.month  = now.month();
}

int v;
int chk;
int temp;

void loop() {

    if (analogRead(2) < 200) {
        delay(50);
        if (set == SET_NONE) {
            set = SET_HOUR;
        } else {
            set++;
        }
        if (set == SET_NONE) {
            timerCounterShow = 0;
            showState = SHOW_TIME;
        }
    }

    switch (set) {
    case SET_HOUR:
        if (analogRead(1) < 200) {
            delay(50);
            setNewTime();
            newTime.hour++;
            if (newTime.hour > 23) newTime.hour = 0;
            RTC.adjust(newTime);
        }
        break;

    case SET_MINUTE:
        if (analogRead(1) < 200) {
            delay(50);
            setNewTime();
            newTime.minute++;
            if (newTime.minute > 59) newTime.minute = 0;
            RTC.adjust(newTime);
        }
        break;

    case SET_DAY:
        if (analogRead(1) < 200) {
            delay(50);
            setNewTime();
            newTime.day++;
            if (newTime.day > 31) newTime.day = 0;
            RTC.adjust(newTime);
        }
        break;

    case SET_MONTH:
        if (analogRead(1) < 200) {
            delay(50);
            setNewTime();
            newTime.month++;
            if (newTime.month > 12) newTime.month = 1;
            RTC.adjust(newTime);
        }
        break;
    }

    v = analogRead(3);

    //showState = SHOW_LIGHT;
    switch (showState) {

    case SHOW_TIME:
        showDelay = SHOW_TIME_TIME;
        now = RTC.now();
        if (now.hour() < 10) {
            sprintf (str, "% 2d%02d", now.hour(), now.minute());
        } else {
            sprintf (str, "%02d%02d", now.hour(), now.minute());
        }
        printChr(str);
        break;

    case SHOW_DATE:
        dot = 0b1010;
        showDelay = SHOW_DATE_TIME;
        now = RTC.now();
        if (now.day() < 10) {
            sprintf (str, "% 2d%02d", now.day(), now.month());
        } else {
            sprintf (str, "%02d%02d", now.day(), now.month());
        }
        printChr(str);
        break;

    case SHOW_TEMP:
        dot = 0b0010;
        chk = DHT.read22(DHT22_PIN);
        temp = (int)((DHT.temperature - TEMP_CORRECTION) * 10);
        if (chk == DHTLIB_OK) {
            sprintf (str, "%03dC", temp);
        } else {
            sprintf (str, "%s", "-- C");
        }
        showDelay = SHOW_TEMP_TIME;
        timerCounterSec = 0;
        printChr(str);
        delay(500);
        break;

//    case SHOW_LIGHT:
//        dot = 0b0000;
//        sprintf (str, "%04dC", v);
//        printChr(str);
//        delay(500);
//        break;
//
    }

    if (v <= 920) {
        dimmer = 7;
    } else
    if (v > 920 && v <= 940) {
        dimmer = 6;
    } else
    if (v > 940 && v <= 950) {
        dimmer = 5;
    } else
    if (v > 950 && v <= 970) {
        dimmer = 4;
    } else
    if (v > 970 && v <= 990) {
        dimmer = 3;
    } else
    if (v > 990 && v <= 1010) {
        dimmer = 2;
    } else
    if (v > 1010) {
        dimmer = 1;
    }

    delay(100);
}
