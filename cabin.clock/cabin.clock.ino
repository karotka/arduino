/**
 * Graphical interface for Arduino Aquarium Computer
 *
 */
#include "configure.h"
#include <Wire.h>
#include <RTClib.h>
#include <DHTlib.h>

#define SEVEN_SEGMENT_DDR DDRD
#define SEVEN_SEGMENT_PORT PORTD

RTC_DS1307 RTC;
DateTime now;
NewTime  newTime;
dht DHT;

volatile char digitsc[4];
volatile int timerCounter0 = 0;
volatile int timerCounterSec = 0;
volatile int timerCounterShow = 0;
volatile int showState = SHOW_TIME;
bool showDot = false;
unsigned int dot = 0b0000;

int showLimit = SHOW_TIME_TIME;

unsigned int i = 0;
char str[4];

void pinInit(void) {
    DDRB = 0xff;
    PORTB = 0xff;

    // Port D
    SEVEN_SEGMENT_DDR=0xff;

    // Turn off all segments
    SEVEN_SEGMENT_PORT=0xff;
}

void timer2set(void) {
    TIMSK2 |= (1 << TOIE2);  // Enable Overflow Interrupt Enable
    TCNT2 = 0;               // Initialize Counter
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

ISR(TIMER2_OVF_vect) {
    PORTB &= ~(1 << PB0);
    PORTB &= ~(1 << PB1);
    PORTB &= ~(1 << PB2);
    PORTB &= ~(1 << PB3);
    PORTB |= (1 << timerCounter0);

    if (showState == SHOW_TIME) {
        if (timerCounterSec < 307) {
            dot = 0b0010;
        } else {
            dot = 0b0000;
        }
    }

    if (CHECK_BIT(dot, timerCounter0)) {
        sevenSegmentChar(digitsc[timerCounter0], 1);
    } else {
        sevenSegmentChar(digitsc[timerCounter0], 0);
    }

    // reset timers if necessary
    if (showState == SHOW_TIME) {
        timerCounterSec++;
    }
    timerCounter0++;
    timerCounterShow++;

    if (timerCounterSec == 614) {
        timerCounterSec = 0;
    }
    if (timerCounter0 == 4) {
        timerCounter0 = 0;
    }

    if (timerCounterShow > showLimit) {
        showState++;
        if (showState > SHOW_TEMP) {
            showState = SHOW_TIME;
        }
        timerCounterShow = 0;
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

void loop() {

    switch (showState) {

    case SHOW_TIME:
        showLimit = SHOW_TIME_TIME;
        now = RTC.now();
        if (now.hour() < 10) {
            sprintf (str, "% 2d%02d", now.hour(), now.minute());
        } else {
            sprintf (str, "%02d%02d", now.hour(), now.minute());
        }
        printChr(str);
        break;

    case SHOW_DATE:
        dot = 0b0101;
        showLimit = SHOW_DATE_TIME;
        now = RTC.now();
        if (now.hour() < 10) {
            sprintf (str, "% 2d%02d", now.day(), now.month());
        } else {
            sprintf (str, "%02d%02d", now.day(), now.month());
        }
        printChr(str);
        break;

    case SHOW_TEMP:
        dot = 0b0100;
        int chk = DHT.read22(DHT22_PIN);
        int temp = (int)(DHT.temperature * 10);
        if (chk == DHTLIB_OK) {
            sprintf (str, "%03dC", temp);
        } else {
            sprintf (str, "%s", "-- C");
        }
        showLimit = SHOW_TEMP_TIME;
        timerCounterSec = 0;
        printChr(str);
        delay(1000);
        break;
    }

    delay(250);
}
