#include "config.h"
#include <Wire.h>
#include <RTClib.h>
#include "util.h"

#include "lightvalues.h"
#include <UTFT.h>
#include <UTFT_Buttons.h>
#include <ITDB02_Touch.h>
#include <Thermistor.h>
#include <RTClib.h>
#include <EEPROM.h>
#include "utils.h"

RTC_DS1307 RTC;
DateTime now;
NewTime newTime;

bool RECVCOMPL = false;
char serialRxBuffer[10];
int serialRxBufferCounter = 0;

LigthValues_t offValues(MODE_OFF);
LigthValues_t day1Values(MODE_DAY1);
LigthValues_t day2Values(MODE_DAY2);
LigthValues_t night1Values(MODE_NIGHT1);
LigthValues_t night2Values(MODE_NIGHT2);
LigthValues_t *actualLightValues;

// temperature of board
Thermistor t0(A0, 0, 100000, 3950);
// main temp
Thermistor t1(A1, 0, 10000,  3380);
// other
Thermistor t2(A2, 0, 10000,  3380);

float te0, te1, te2;

uint8_t lightStates[10];
uint8_t hourStates[10];
uint8_t minuteStates[10];

uint8_t co2states[6];
uint8_t co2hour[6];
uint8_t co2minute[6];

uint8_t feedhour[4];
uint8_t feedminute[4];

uint8_t temperatureSenzorStatus[4];
int feedDone = 0;
int lastRun = 0;

int page = PAGE_HOME;

volatile uint8_t switchMode;

volatile int timerCounter1 = 0;
volatile int timerCounter2 = 0;
volatile int temperatureReadTimeCounter;
volatile int timerCounterFeed = 500;

volatile uint8_t actualCo2state;

volatile uint8_t i2cReadTimeCounter;

int dimmingSpeed = DIMMING_SPEED_FAST;

void pinInit(void) {
    DDRK = 0xff;
    PORTK = 0x0;

    pinMode(FEED_PIN_SW, INPUT);
    pinMode(FEED_PIN_POWER, OUTPUT);
}

void timer2set(void) {
    TIMSK2 |= (1 << TOIE2);  // Enable Overflow Interrupt Enable
    TCNT2 = 0;               // Initialize Counter
}

const char* strDayOfTheWeek(int num) {
    switch(num) {
    case 1:
        return "Monday";
    break;
    case 2:
        return "Tuesday";
    break;
    case 3:
        return "Wednesday";
    break;
    case 4:
        return "Thursday";
    break;
    case 5:
        return "Friday";
    break;
    case 6:
        return "Saturday";
    break;
    case 0:
        return "Sunday";
    break;
    default:
        return 0;
    }
}

void serialEvent1() {
    while (Serial1.available()) {
        char ch = (char)Serial1.read();
        //Serial.println(ch);
        if (ch == '\n') {
            RECVCOMPL = true;
        } else {
            serialRxBuffer[serialRxBufferCounter] = ch;
            serialRxBufferCounter++;
        }
    }
}

void serialInterface() {

    char ret[128];
    char *spl[3];
    int c = split(serialRxBuffer, '\t', spl, sizeof(spl));
    int command = atoi(spl[0]);

    char *value  = spl[1];
    bool resp = true;

    switch (command) {
    case PAGE_HOME:
        page = PAGE_HOME;
        resp = false;
        break;

    case PAGE_TIME:
        newTime.day    = now.day();
        newTime.month  = now.month();
        newTime.year   = now.year();
        newTime.hour   = now.hour();
        newTime.minute = now.minute();
        newTime.second = 0;

        page = PAGE_TIME;
        resp = false;
        break;

    case HOLO:
        sprintf(
            ret, "%s-%02d-%02d %04d-%02d:%02d:%02d\t23.6-20.2-33.4\t%s\t%s %s-%02d-%02d-%02d-%02d-%02d-%02d\t%s\n",
            strDayOfTheWeek(now.dayOfTheWeek()), now.day(), now.month(), now.year(),
            now.hour(), now.minute(), now.second(), "OFF", "Automat", "OFF",
            10, 20, 30, 40, 50, 60, "7:00-19:00");
        break;
    case TILO:
        sprintf(ret, "%02d-%02d-%04d-%02d-%02d-%02d-n\n",
                newTime.day, newTime.month, newTime.year,
                newTime.hour, newTime.minute, newTime.second);
        break;
    case COLO:
        sprintf(ret, "%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\n",
                co2hour[0], co2minute[0], co2hour[1], co2minute[1],
                co2hour[2], co2minute[2], co2hour[3], co2minute[3],
                co2states[0], co2states[1], co2states[2], co2states[3]);
        break;
    case FDLO:
        sprintf(ret, "%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\n",
                7, 0, 10, 30, 13, 0, 15, 0);
        break;
    case LILO:
        sprintf(ret, "%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                10, 20, 30, 40, 50, 60, 0, 1, 0, 0, 0, 0);
        break;
    case TRLO:
        sprintf(ret,
                "%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                7, 0, 10, 30, 13, 0, 15, 0, 16, 50, 17, 45, 19, 50, 21, 20,
                1, 0, 1, 0, 1, 0, 1, 0);
        break;
    case TISA: {
        if (strcmp(value, "YU") == 0) {
            newTime.year++;
            if (newTime.year > 2050) newTime.year = 2018;
        } else
        if (strcmp(value, "YD") == 0) {
            newTime.year--;
            if (newTime.year < 2018) newTime.year = 2050;
        } else
        if (strcmp(value, "MU") == 0) {
            newTime.month++;
            if (newTime.month > 12) newTime.month = 1;
        } else
        if (strcmp(value, "MD") == 0) {
            newTime.month--;
            if (newTime.month < 1) newTime.month = 12;
        } else
        if (strcmp(value, "DU") == 0) {
            newTime.day++;
            if (newTime.day > 31) newTime.day = 1;
        } else
        if (strcmp(value, "DD") == 0) {
            newTime.day--;
            if (newTime.day < 1) newTime.day = 31;
        } else
        if (strcmp(value, "HU") == 0) {
            newTime.hour++;
            if (newTime.hour > 23) newTime.hour = 0;
        } else
        if (strcmp(value, "HD") == 0) {
            newTime.hour--;
            if (newTime.hour == 255) newTime.hour = 23;
        } else
        if (strcmp(value, "IU") == 0) {
            newTime.minute++;
            if (newTime.minute > 59) newTime.minute = 0;
        } else
        if (strcmp(value, "ID") == 0) {
            newTime.minute--;
            if (newTime.minute == 255) newTime.minute = 59;
        }

        char saved = 'n';
        if (strcmp(value, "ok") == 0) {
            RTC.adjust(newTime);
            saved = 'y';
        }

        sprintf(ret, "%02d-%02d-%04d-%02d-%02d-%02d-%c\n",
                newTime.day, newTime.month, newTime.year,
                newTime.hour, newTime.minute, newTime.second, saved);
        break;
    }
    case COSA:
        if (strcmp(value, "h1u") == 0) {
            co2hour[0]++;
            if (co2hour[0] > 23) co2hour[0] = 0;
            sprintf(ret, "h1-%02d\n", co2hour[0]);
        } else
        if (strcmp(value, "h1d") == 0) {
            co2hour[0]--;
            if (co2hour[0] == 255) co2hour[0] = 23;
            sprintf(ret, "h1-%02d\n", co2hour[0]);
        } else
        if (strcmp(value, "m1u") == 0) {
            co2minute[0]++;
            if (co2minute[0] > 59) co2minute[0] = 0;
            sprintf(ret, "m1-%02d\n", co2minute[0]);
        } else
        if (strcmp(value, "m1d") == 0) {
            co2minute[0]--;
            if (co2minute[0] == 255) co2minute[0] = 59;
            sprintf(ret, "m1-%02d\n", co2minute[0]);
        } else
        if (strcmp(value, "s0") == 0) {
            if (co2states[0] == 0) co2states[0] = 1;
            else
            if (co2states[0] == 1) co2states[0] = 0;
            if (co2states[0] > 1) co2states[0] = 0;
            sprintf(ret, "s0-%d\n", co2states[0]);
        } else

        if (strcmp(value, "h2u") == 0) {
            co2hour[1]++;
            if (co2hour[1] > 23) co2hour[1] = 0;
            sprintf(ret, "h2-%02d\n", co2hour[1]);
        } else
        if (strcmp(value, "h2d") == 0) {
            co2hour[1]--;
            if (co2hour[1] == 255) co2hour[1] = 23;
            sprintf(ret, "h2-%02d\n", co2hour[1]);
        } else
        if (strcmp(value, "m2u") == 0) {
            co2minute[1]++;
            if (co2minute[1] > 59) co2minute[1] = 0;
            sprintf(ret, "m2-%02d\n", co2minute[1]);
        } else
        if (strcmp(value, "m2d") == 0) {
            co2minute[1]--;
            if (co2minute[1] == 255) co2minute[1] = 59;
            sprintf(ret, "m2-%02d\n", co2minute[1]);
        } else
        if (strcmp(value, "s1") == 0) {
            if (co2states[1] == 0) co2states[1] = 1;
            else
            if (co2states[1] == 1) co2states[1] = 0;
            if (co2states[1] > 1) co2states[1] = 0;
            sprintf(ret, "s1-%d\n", co2states[1]);
        } else


        if (strcmp(value, "ok") == 0) {
            EEPROM.write(31,  co2states[0]);
            EEPROM.write(32,  co2states[1]);
            EEPROM.write(33,  co2states[2]);
            EEPROM.write(34,  co2states[3]);
            co2states[4] = co2states[3];
            co2states[5] = co2states[3];

            EEPROM.write(35, co2hour[0]);
            EEPROM.write(36, co2hour[1]);
            EEPROM.write(37, co2hour[2]);
            EEPROM.write(38, co2hour[3]);

            EEPROM.write(39, co2minute[0]);
            EEPROM.write(40, co2minute[1]);
            EEPROM.write(41, co2minute[2]);
            EEPROM.write(42, co2minute[3]);
            sprintf(ret, "y\n");
        }

        break;
    }

    if (resp) {
        Serial1.print(ret);
        //Serial.println(ret);
    }
    RECVCOMPL = false;
    serialRxBufferCounter = 0;
    //Serial1.flush(); // wait for a serial string to be finished sending

    //Serial.print("buffer:");
    //Serial.println(serialRxBuffer);
    //Serial.print("command:");
    //Serial.println(command);
    //Serial.print("value:");
    //Serial.println(value);
    //Serial.print("page:");
    //Serial.println(page);

}

void loop() {
    now = RTC.now();

    if (RECVCOMPL) {
        serialInterface();
    }

    if (temperatureReadTimeCounter > 430) {
        t0.readTemperature();
        te0 = t0.getCelsius();
        myRound(&te0);

        t1.readTemperature();
        te1 = t1.getCelsius();
        myRound(&te1);

        if (temperatureSenzorStatus[2] == 1) {
            te2 = t2.getCelsius();
            myRound(&te2);
            t2.readTemperature();
        }
        temperatureReadTimeCounter = 0;
    }

}


void switchLight(int i) {
    if (actualLightValues->flag != lightStates[i]) {

        if (lightStates[i] == MODE_OFF) {
            actualLightValues = &offValues;
        } else
        if (lightStates[i] == MODE_DAY1) {
            actualLightValues = &day1Values;
        } else
        if (lightStates[i] == MODE_DAY2) {
            actualLightValues = &day2Values;
        } else
        if (lightStates[i] == MODE_NIGHT1) {
            actualLightValues = &night1Values;
        } else
        if (lightStates[i] == MODE_NIGHT2) {
            actualLightValues = &night2Values;
        }
    }
}

/**
 * Check if actual time is bettween timer values
 * If yes, switch lights to right state
 **/
void checkTimer() {

    int minutes, nextMinutes, realMinute;
    realMinute = now.minute() + (now.hour() * 60);

    for(uint8_t i = 0; i < 10; i++) {
        uint8_t j = i + 1;
        if (i == 9) { j = 0; }

        minutes     = minuteStates[i] + (hourStates[i] * 60);
        nextMinutes = minuteStates[j] + (hourStates[j] * 60);

        if (minutes <= realMinute && realMinute < nextMinutes) {
            switchLight(i);
            break;
        }
    }
}

void switchCo2(int i) {
    if (actualCo2state != co2states[i]) {
        actualCo2state = co2states[i];
        if (actualCo2state == CO2_OFF) {
            digitalWrite(RELE_PIN, HIGH);
        }
        if (actualCo2state == CO2_ON) {
            digitalWrite(RELE_PIN, LOW);
        }
    }
}

/**
 * Check if actual time is bettween timer values
 * If yes, switch lights to the right state
 **/
void checkCo2() {

    int minutes, nextMinutes, realMinute;
    realMinute = now.minute() + (now.hour() * 60);

    for(uint8_t i = 0; i < 6; i++) {
        uint8_t j = i + 1;
        if (i == 5) { j = 0; }

        minutes     = co2minute[i] + (co2hour[i] * 60);
        nextMinutes = co2minute[j] + (co2hour[j] * 60);

        if (minutes <= realMinute && realMinute < nextMinutes) {
            switchCo2(i);
            break;
        }
    }
}

void checkFeed() {

    int minutes, realMinute;
    realMinute = now.minute() + (now.hour() * 60);

    if (lastRun < realMinute) {
        feedDone = 0;
    }

    for(uint8_t i = 0; i < 4; i++) {

        minutes = feedminute[i] + (feedhour[i] * 60);

        // check time
        if (minutes == realMinute) {
            // if interval is done, run feed and
            // set done for this period of time
            if(feedDone == 0) {
                lastRun = realMinute;
                feedDone = 1;
                timerCounterFeed = 500;
            }
            break;
        }
    }
}

/**
 * Feed is runnig when timerCounterFeed > 0
 */
void processFeed() {
    if (timerCounterFeed) {
        timerCounterFeed--;
        digitalWrite(FEED_PIN_POWER, HIGH); // working
    } else {
        if (digitalRead(FEED_PIN_SW)) {
            digitalWrite(FEED_PIN_POWER, LOW);  // endstop
        } else {
            digitalWrite(FEED_PIN_POWER, HIGH); // working
        }
    }
}

ISR(TIMER2_OVF_vect) {
    timerCounter1++;
    timerCounter2++;
    i2cReadTimeCounter++;
    temperatureReadTimeCounter++;

    if (switchMode == MODE_AUTO) {
        checkTimer();
        checkCo2();
        checkFeed();
        dimmingSpeed = DIMMING_SPEED_SLOW;
    } else {
        dimmingSpeed = DIMMING_SPEED_FAST;
    }

    processFeed();

    if (timerCounter1 > dimmingSpeed) {

        if (OCR1A > actualLightValues->coolByte) {
            analogWrite(LED_COOL_WHITE, --OCR1A);
        }
        if (OCR1B > actualLightValues->warmByte) {
            analogWrite(LED_WHITE, --OCR1B);
        }
        if (OCR0A > actualLightValues->yellowByte) {
            analogWrite(LED_YELLOW, --OCR0A);
        }
        if (OCR2A > actualLightValues->redByte) {
            analogWrite(LED_RED, --OCR2A);
        }
        if (OCR2B > actualLightValues->greenByte) {
            analogWrite(LED_GREEN, --OCR2B);
        }
        if (OCR4C > actualLightValues->blueByte) {
            analogWrite(LED_BLUE, --OCR4C);
        }

        if (OCR1A < actualLightValues->coolByte) {
            analogWrite(LED_COOL_WHITE, ++OCR1A);
        }
        if (OCR1B < actualLightValues->warmByte) {
            analogWrite(LED_WHITE, ++OCR1B);
        }
        if (OCR0A < actualLightValues->yellowByte) {
            analogWrite(LED_YELLOW, ++OCR0A);
        }
        if (OCR2A < actualLightValues->redByte) {
            analogWrite(LED_RED, ++OCR2A);
        }
        if (OCR2B < actualLightValues->greenByte) {
            analogWrite(LED_GREEN, ++OCR2B);
        }
        if (OCR4C < actualLightValues->blueByte) {
            analogWrite(LED_BLUE, ++OCR4C);
        }
        timerCounter1 = 0;
    }

    // return to homepage
    if (timerCounter2 > RETURN_DELAY) {
        timerCounter2 = 0;
    }
}

void eepromRead() {

    lightStates[0] = EEPROM.read(6);
    lightStates[1] = EEPROM.read(7);
    lightStates[2] = EEPROM.read(8);
    lightStates[3] = EEPROM.read(9);
    lightStates[4] = EEPROM.read(10);
    lightStates[5] = EEPROM.read(11);
    lightStates[6] = EEPROM.read(12);
    lightStates[7] = EEPROM.read(13);
    lightStates[8] = lightStates[7];
    lightStates[9] = lightStates[7];

    hourStates[0] = EEPROM.read(14);
    hourStates[1] = EEPROM.read(15);
    hourStates[2] = EEPROM.read(16);
    hourStates[3] = EEPROM.read(17);
    hourStates[4] = EEPROM.read(18);
    hourStates[5] = EEPROM.read(19);
    hourStates[6] = EEPROM.read(20);
    hourStates[7] = EEPROM.read(21);
    hourStates[8] = 23;
    hourStates[9] = 0;

    minuteStates[0] = EEPROM.read(22);
    minuteStates[1] = EEPROM.read(23);
    minuteStates[2] = EEPROM.read(24);
    minuteStates[3] = EEPROM.read(25);
    minuteStates[4] = EEPROM.read(26);
    minuteStates[5] = EEPROM.read(27);
    minuteStates[6] = EEPROM.read(28);
    minuteStates[7] = EEPROM.read(29);
    minuteStates[8] = 59;
    minuteStates[9] = 0;

    switchMode = EEPROM.read(30);

    co2states[0] = EEPROM.read(31);
    co2states[1] = EEPROM.read(32);
    co2states[2] = EEPROM.read(33);
    co2states[3] = EEPROM.read(34);
    co2states[4] = co2states[3];
    co2states[5] = co2states[3];

    co2hour[0] = EEPROM.read(35);
    co2hour[1] = EEPROM.read(36);
    co2hour[2] = EEPROM.read(37);
    co2hour[3] = EEPROM.read(38);
    co2hour[4] = 23;
    co2hour[5] = 0;

    co2minute[0] = EEPROM.read(39);
    co2minute[1] = EEPROM.read(40);
    co2minute[2] = EEPROM.read(41);
    co2minute[3] = EEPROM.read(42);
    co2minute[4] = 59;
    co2minute[5] = 0;

    temperatureSenzorStatus[0] = 1;
    temperatureSenzorStatus[1] = 1;
    temperatureSenzorStatus[2] = EEPROM.read(44);
    temperatureSenzorStatus[3] = EEPROM.read(45);

    // from 64 to 93 LigthValues_t

    feedhour[0] = EEPROM.read(94);
    feedhour[1] = EEPROM.read(95);
    feedhour[2] = EEPROM.read(96);
    feedhour[3] = EEPROM.read(97);
    //feedhour[4] = 23;
    //feedhour[5] = 0;

    feedminute[0] = EEPROM.read(98);
    feedminute[1] = EEPROM.read(99);
    feedminute[2] = EEPROM.read(100);
    feedminute[3] = EEPROM.read(101);
    //feedminute[4] = 59;
    //feedminute[5] = 0;
}

void setup() {
    pinMode(RELE_PIN, OUTPUT);

    Serial.begin(115200);  while (!Serial);
    Serial1.begin(115200); while (!Serial1);

    delay(10);
    Serial.flush(); // wait for a serial string to be finished sending
    Serial1.flush(); // wait for a serial string to be finished sending


    RTC.begin();

    if (!RTC.isrunning()) {
        //Serial.println("RTC is NOT running!");
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }

    t0.begin();
    t1.begin();
    t2.begin();

    eepromRead();

    offValues.load();
    day1Values.load();
    day2Values.load();
    night1Values.load();
    night2Values.load();
    actualLightValues = &day1Values;

    pinInit();
    timer2set();
}
