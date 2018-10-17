#include "config.h"
#include <Wire.h>
#include <RTClib.h>
#include "util.h"

#include "lightvalues.h"
#include <Thermistor.h>
#include <RTClib.h>
#include <EEPROM.h>
#include "utils.h"

const char* timerStr[5] = {"D1", "D2", "N1", "N2", "OFF"};

RTC_DS1307 RTC;
DateTime now;
NewTime newTime;

bool RECVCOMPL = false;
char serialRxBuffer[10];
int serialRxBufferCounter = 0;

LigthValues_t offValues(TM_OFF);
LigthValues_t day1Values(TM_DAY1);
LigthValues_t day2Values(TM_DAY2);
LigthValues_t night1Values(TM_NIGHT1);
LigthValues_t night2Values(TM_NIGHT2);
LigthValues_t *actualLightValues;

// temperature of board
Thermistor t0(A0, 0, 100000, 3950);
// main temp
Thermistor t1(A1, 0, 10000,  3380);
// other
Thermistor t2(A2, 0, 10000,  3380);

float te0, te1, te2;

uint8_t timerstates[10];
uint8_t timerhour[10];
uint8_t timerminute[10];

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

/**
 * Input reqest is 3 command separate by \t ends by \n
 * first   - command, needs to be int in char format
 * second  - value char*
 * third   - none for, to be sure
 */
void serialInterface() {

    char ret[128];
    char *spl[3];
    int c = split(serialRxBuffer, '\t', spl, sizeof(spl));
    int command = atoi(spl[0]);

    char *value  = spl[1];
    bool needResp = true;

    switch (command) {
    case PAGE_HOME:
        page = PAGE_HOME;
        needResp = false;
        break;

    case PAGE_TIME:
        newTime.day    = now.day();
        newTime.month  = now.month();
        newTime.year   = now.year();
        newTime.hour   = now.hour();
        newTime.minute = now.minute();
        newTime.second = 0;

        page = PAGE_TIME;
        needResp = false;
        break;

    case HOLO:
        char strTemp[15];
        //sprintf(strTemp, "%2d.%1d:%2d.%1d:%2d.%1d", (int)te0%100, (int)(te0*10)%10, (int)te1%100, (int)(te1*10)%10, (int)te2%100, (int)(te2*10)%10);
        sprintf(strTemp, "22.9:25.8:34.8");
        sprintf(
            ret, "%s-%02d-%02d %04d-%02d:%02d:%02d\t%s\t%s\t%s %s-%02d-%02d-%02d-%02d-%02d-%02d\t%02d:%02d\t%02d:%02d\t%02d:%02d\t%02d:%02d\n",
            strDayOfTheWeek(now.dayOfTheWeek()), now.day(), now.month(), now.year(),
            now.hour(), now.minute(), now.second(), strTemp, "OFF", "Automat", "OFF",
            10, 20, 30, 40, 50, 60,
            feedhour[0], feedminute[0], feedhour[1], feedminute[1],
            feedhour[2], feedminute[2], feedhour[3], feedminute[3]);
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
                feedhour[0], feedminute[0], feedhour[1], feedminute[1],
                feedhour[2], feedminute[2], feedhour[3], feedminute[3]);
        break;
    case LILO:
        sprintf(ret, "%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                actualLightValues->coolValue, actualLightValues->warmValue,
                actualLightValues->yellowValue, actualLightValues->redValue,
                actualLightValues->greenValue, actualLightValues->blueValue,
                0, 1, 0, 0, 0, 0);
        break;
    case TRLO:
        sprintf(ret,
                "%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%02d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
                timerhour[0], timerminute[0], timerhour[1], timerminute[1],
                timerhour[2], timerminute[2], timerhour[3], timerminute[3],
                timerhour[4], timerminute[4], timerhour[5], timerminute[5],
                timerhour[6], timerminute[6], timerhour[7], timerminute[7],
                timerStr[timerstates[0]], timerStr[timerstates[1]],
                timerStr[timerstates[2]], timerStr[timerstates[3]],
                timerStr[timerstates[4]], timerStr[timerstates[5]],
                timerStr[timerstates[6]], timerStr[timerstates[7]]);
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

        if (strcmp(value, "h3u") == 0) {
            co2hour[2]++;
            if (co2hour[2] > 23) co2hour[2] = 0;
            sprintf(ret, "h3-%02d\n", co2hour[2]);
        } else
        if (strcmp(value, "h3d") == 0) {
            co2hour[2]--;
            if (co2hour[2] == 255) co2hour[2] = 23;
            sprintf(ret, "h3-%02d\n", co2hour[2]);
        } else
        if (strcmp(value, "m3u") == 0) {
            co2minute[2]++;
            if (co2minute[2] > 59) co2minute[2] = 0;
            sprintf(ret, "m3-%02d\n", co2minute[2]);
        } else
        if (strcmp(value, "m3d") == 0) {
            co2minute[2]--;
            if (co2minute[2] == 255) co2minute[2] = 59;
            sprintf(ret, "m3-%02d\n", co2minute[2]);
        } else
        if (strcmp(value, "s2") == 0) {
            if (co2states[2] == 0) co2states[2] = 1;
            else
            if (co2states[2] == 1) co2states[2] = 0;
            if (co2states[2] > 1) co2states[2] = 0;
            sprintf(ret, "s2-%d\n", co2states[2]);
        } else

        if (strcmp(value, "h4u") == 0) {
            co2hour[3]++;
            if (co2hour[3] > 23) co2hour[3] = 0;
            sprintf(ret, "h4-%02d\n", co2hour[3]);
        } else
        if (strcmp(value, "h4d") == 0) {
            co2hour[3]--;
            if (co2hour[3] == 255) co2hour[3] = 23;
            sprintf(ret, "h4-%02d\n", co2hour[3]);
        } else
        if (strcmp(value, "m4u") == 0) {
            co2minute[3]++;
            if (co2minute[3] > 59) co2minute[3] = 0;
            sprintf(ret, "m4-%02d\n", co2minute[3]);
        } else
        if (strcmp(value, "m4d") == 0) {
            co2minute[3]--;
            if (co2minute[3] == 255) co2minute[3] = 59;
            sprintf(ret, "m4-%02d\n", co2minute[3]);
        } else
        if (strcmp(value, "s3") == 0) {
            if (co2states[3] == 0) co2states[3] = 1;
            else
            if (co2states[3] == 1) co2states[3] = 0;
            if (co2states[3] > 1) co2states[3] = 0;
            sprintf(ret, "s3-%d\n", co2states[3]);
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

    case FDSA:
        if (strcmp(value, "h1u") == 0) {
            feedhour[0]++;
            if (feedhour[0] > 23) feedhour[0] = 0;
            sprintf(ret, "h1-%02d\n", feedhour[0]);
        } else
        if (strcmp(value, "h1d") == 0) {
            feedhour[0]--;
            if (feedhour[0] == 255) feedhour[0] = 23;
            sprintf(ret, "h1-%02d\n", feedhour[0]);
        } else
        if (strcmp(value, "m1u") == 0) {
            feedminute[0]++;
            if (feedminute[0] > 59) feedminute[0] = 0;
            sprintf(ret, "m1-%02d\n", feedminute[0]);
        } else
        if (strcmp(value, "m1d") == 0) {
            feedminute[0]--;
            if (feedminute[0] == 255) feedminute[0] = 59;
            sprintf(ret, "m1-%02d\n", feedminute[0]);
        } else

        if (strcmp(value, "h2u") == 0) {
            feedhour[1]++;
            if (feedhour[1] > 23) feedhour[1] = 0;
            sprintf(ret, "h2-%02d\n", feedhour[1]);
        } else
        if (strcmp(value, "h2d") == 0) {
            feedhour[1]--;
            if (feedhour[1] == 255) feedhour[1] = 23;
            sprintf(ret, "h2-%02d\n", feedhour[1]);
        } else
        if (strcmp(value, "m2u") == 0) {
            feedminute[1]++;
            if (feedminute[1] > 59) feedminute[1] = 0;
            sprintf(ret, "m2-%02d\n", feedminute[1]);
        } else
        if (strcmp(value, "m2d") == 0) {
            feedminute[1]--;
            if (feedminute[1] == 255) feedminute[1] = 59;
            sprintf(ret, "m2-%02d\n", feedminute[1]);
        } else

        if (strcmp(value, "h3u") == 0) {
            feedhour[2]++;
            if (feedhour[2] > 23) feedhour[2] = 0;
            sprintf(ret, "h3-%02d\n", feedhour[2]);
        } else
        if (strcmp(value, "h3d") == 0) {
            feedhour[2]--;
            if (feedhour[2] == 255) feedhour[2] = 23;
            sprintf(ret, "h3-%02d\n", feedhour[2]);
        } else
        if (strcmp(value, "m3u") == 0) {
            feedminute[2]++;
            if (feedminute[2] > 59) feedminute[2] = 0;
            sprintf(ret, "m3-%02d\n", feedminute[2]);
        } else
        if (strcmp(value, "m3d") == 0) {
            feedminute[2]--;
            if (feedminute[2] == 255) feedminute[2] = 59;
            sprintf(ret, "m3-%02d\n", feedminute[2]);
        } else

        if (strcmp(value, "h4u") == 0) {
            feedhour[3]++;
            if (feedhour[3] > 23) feedhour[3] = 0;
            sprintf(ret, "h4-%02d\n", feedhour[3]);
        } else
        if (strcmp(value, "h4d") == 0) {
            feedhour[3]--;
            if (feedhour[3] == 255) feedhour[3] = 23;
            sprintf(ret, "h4-%02d\n", feedhour[3]);
        } else
        if (strcmp(value, "m4u") == 0) {
            feedminute[3]++;
            if (feedminute[3] > 59) feedminute[3] = 0;
            sprintf(ret, "m4-%02d\n", feedminute[3]);
        } else
        if (strcmp(value, "m4d") == 0) {
            feedminute[3]--;
            if (feedminute[3] == 255) feedminute[3] = 59;
            sprintf(ret, "m4-%02d\n", feedminute[3]);
        } else

        if (strcmp(value, "ok") == 0) {
            EEPROM.write(94, feedhour[0]);
            EEPROM.write(95, feedhour[1]);
            EEPROM.write(96, feedhour[2]);
            EEPROM.write(97, feedhour[3]);

            EEPROM.write(98, feedminute[0]);
            EEPROM.write(99, feedminute[1]);
            EEPROM.write(100, feedminute[2]);
            EEPROM.write(101, feedminute[3]);
            sprintf(ret, "y\n");
        }
        break;

    case TRSA:
        if (strcmp(value, "h1u") == 0) {
            timerhour[0]++;
            if (timerhour[0] > 23) timerhour[0] = 0;
            sprintf(ret, "h1-%02d\n", timerhour[0]);
        } else
        if (strcmp(value, "h1d") == 0) {
            timerhour[0]--;
            if (timerhour[0] == 255) timerhour[0] = 23;
            sprintf(ret, "h1-%02d\n", timerhour[0]);
        } else
        if (strcmp(value, "m1u") == 0) {
            timerminute[0]++;
            if (timerminute[0] > 59) timerminute[0] = 0;
            sprintf(ret, "m1-%02d\n", timerminute[0]);
        } else
        if (strcmp(value, "m1d") == 0) {
            timerminute[0]--;
            if (timerminute[0] == 255) timerminute[0] = 59;
            sprintf(ret, "m1-%02d\n", timerminute[0]);
        } else
        if (strcmp(value, "s0") == 0) {
            timerstates[0]++;
            if (timerstates[0] == TM_END) timerstates[0] = TM_DAY1;
            sprintf(ret, "s0-%s\n", timerStr[timerstates[0]]);
        } else

        if (strcmp(value, "h2u") == 0) {
            timerhour[1]++;
            if (timerhour[1] > 23) timerhour[1] = 0;
            sprintf(ret, "h2-%02d\n", timerhour[1]);
        } else
        if (strcmp(value, "h2d") == 0) {
            timerhour[1]--;
            if (timerhour[1] == 255) timerhour[1] = 23;
            sprintf(ret, "h2-%02d\n", timerhour[1]);
        } else
        if (strcmp(value, "m2u") == 0) {
            timerminute[1]++;
            if (timerminute[1] > 59) timerminute[1] = 0;
            sprintf(ret, "m2-%02d\n", timerminute[1]);
        } else
        if (strcmp(value, "m2d") == 0) {
            timerminute[1]--;
            if (timerminute[1] == 255) timerminute[1] = 59;
            sprintf(ret, "m2-%02d\n", timerminute[1]);
        } else
        if (strcmp(value, "s1") == 0) {
            timerstates[1]++;
            if (timerstates[1] == TM_END) timerstates[1] = TM_DAY1;
            sprintf(ret, "s1-%s\n", timerStr[timerstates[1]]);
        } else

        if (strcmp(value, "h3u") == 0) {
            timerhour[2]++;
            if (timerhour[2] > 23) timerhour[2] = 0;
            sprintf(ret, "h3-%02d\n", timerhour[2]);
        } else
        if (strcmp(value, "h3d") == 0) {
            timerhour[2]--;
            if (timerhour[2] == 255) timerhour[2] = 23;
            sprintf(ret, "h3-%02d\n", timerhour[2]);
        } else
        if (strcmp(value, "m3u") == 0) {
            timerminute[2]++;
            if (timerminute[2] > 59) timerminute[2] = 0;
            sprintf(ret, "m3-%02d\n", timerminute[2]);
        } else
        if (strcmp(value, "m3d") == 0) {
            timerminute[2]--;
            if (timerminute[2] == 255) timerminute[2] = 59;
            sprintf(ret, "m3-%02d\n", timerminute[2]);
        } else
        if (strcmp(value, "s2") == 0) {
            timerstates[2]++;
            if (timerstates[2] == TM_END) timerstates[2] = TM_DAY1;
            sprintf(ret, "s2-%s\n", timerStr[timerstates[2]]);
        } else

        if (strcmp(value, "h4u") == 0) {
            timerhour[3]++;
            if (timerhour[3] > 23) timerhour[3] = 0;
            sprintf(ret, "h4-%02d\n", timerhour[3]);
        } else
        if (strcmp(value, "h4d") == 0) {
            timerhour[3]--;
            if (timerhour[3] == 255) timerhour[3] = 23;
            sprintf(ret, "h4-%02d\n", timerhour[3]);
        } else
        if (strcmp(value, "m4u") == 0) {
            timerminute[3]++;
            if (timerminute[3] > 59) timerminute[3] = 0;
            sprintf(ret, "m4-%02d\n", timerminute[3]);
        } else
        if (strcmp(value, "m4d") == 0) {
            timerminute[3]--;
            if (timerminute[3] == 255) timerminute[3] = 59;
            sprintf(ret, "m4-%02d\n", timerminute[3]);
        } else
        if (strcmp(value, "s3") == 0) {
            timerstates[3]++;
            if (timerstates[3] == TM_END) timerstates[3] = TM_DAY1;
            sprintf(ret, "s3-%s\n", timerStr[timerstates[3]]);
        } else

        if (strcmp(value, "h5u") == 0) {
            timerhour[4]++;
            if (timerhour[4] > 23) timerhour[4] = 0;
            sprintf(ret, "h5-%02d\n", timerhour[4]);
        } else
        if (strcmp(value, "h4d") == 0) {
            timerhour[4]--;
            if (timerhour[4] == 255) timerhour[4] = 23;
            sprintf(ret, "h5-%02d\n", timerhour[4]);
        } else
        if (strcmp(value, "m5u") == 0) {
            timerminute[4]++;
            if (timerminute[4] > 59) timerminute[4] = 0;
            sprintf(ret, "m5-%02d\n", timerminute[4]);
        } else
        if (strcmp(value, "m5d") == 0) {
            timerminute[4]--;
            if (timerminute[4] == 255) timerminute[4] = 59;
            sprintf(ret, "m5-%02d\n", timerminute[4]);
        } else
        if (strcmp(value, "s4") == 0) {
            timerstates[4]++;
            if (timerstates[4] == TM_END) timerstates[4] = TM_DAY1;
            sprintf(ret, "s4-%s\n", timerStr[timerstates[4]]);
        } else

        if (strcmp(value, "h6u") == 0) {
            timerhour[5]++;
            if (timerhour[5] > 23) timerhour[5] = 0;
            sprintf(ret, "h6-%02d\n", timerhour[5]);
        } else
        if (strcmp(value, "h6d") == 0) {
            timerhour[5]--;
            if (timerhour[5] == 255) timerhour[5] = 23;
            sprintf(ret, "h6-%02d\n", timerhour[5]);
        } else
        if (strcmp(value, "m6u") == 0) {
            timerminute[5]++;
            if (timerminute[5] > 59) timerminute[5] = 0;
            sprintf(ret, "m6-%02d\n", timerminute[5]);
        } else
        if (strcmp(value, "m6d") == 0) {
            timerminute[5]--;
            if (timerminute[5] == 255) timerminute[5] = 59;
            sprintf(ret, "m6-%02d\n", timerminute[5]);
        } else
        if (strcmp(value, "s5") == 0) {
            timerstates[5]++;
            if (timerstates[5] == TM_END) timerstates[5] = TM_DAY1;
            sprintf(ret, "s5-%s\n", timerStr[timerstates[5]]);
        } else

        if (strcmp(value, "h7u") == 0) {
            timerhour[6]++;
            if (timerhour[6] > 23) timerhour[6] = 0;
            sprintf(ret, "h7-%02d\n", timerhour[6]);
        } else
        if (strcmp(value, "h7d") == 0) {
            timerhour[6]--;
            if (timerhour[6] == 255) timerhour[6] = 23;
            sprintf(ret, "h7-%02d\n", timerhour[6]);
        } else
        if (strcmp(value, "m7u") == 0) {
            timerminute[6]++;
            if (timerminute[6] > 59) timerminute[6] = 0;
            sprintf(ret, "m7-%02d\n", timerminute[6]);
        } else
        if (strcmp(value, "m7d") == 0) {
            timerminute[6]--;
            if (timerminute[6] == 255) timerminute[6] = 59;
            sprintf(ret, "m7-%02d\n", timerminute[6]);
        } else
        if (strcmp(value, "s6") == 0) {
            timerstates[6]++;
            if (timerstates[6] == TM_END) timerstates[6] = TM_DAY1;
            sprintf(ret, "s6-%s\n", timerStr[timerstates[6]]);
        } else

        if (strcmp(value, "h8u") == 0) {
            timerhour[7]++;
            if (timerhour[7] > 23) timerhour[7] = 0;
            sprintf(ret, "h8-%02d\n", timerhour[7]);
        } else
        if (strcmp(value, "h8d") == 0) {
            timerhour[7]--;
            if (timerhour[7] == 255) timerhour[7] = 23;
            sprintf(ret, "h8-%02d\n", timerhour[7]);
        } else
        if (strcmp(value, "m8u") == 0) {
            timerminute[7]++;
            if (timerminute[7] > 59) timerminute[7] = 0;
            sprintf(ret, "m8-%02d\n", timerminute[7]);
        } else
        if (strcmp(value, "m8d") == 0) {
            timerminute[7]--;
            if (timerminute[7] == 255) timerminute[7] = 59;
            sprintf(ret, "m8-%02d\n", timerminute[7]);
        } else
        if (strcmp(value, "s7") == 0) {
            timerstates[7]++;
            if (timerstates[7] == TM_END) timerstates[7] = TM_DAY1;
            sprintf(ret, "s7-%s\n", timerStr[timerstates[7]]);
        } else

        if (strcmp(value, "ok") == 0) {

            EEPROM.write(6,  timerstates[0]);
            EEPROM.write(7,  timerstates[1]);
            EEPROM.write(8,  timerstates[2]);
            EEPROM.write(9,  timerstates[3]);
            EEPROM.write(10, timerstates[4]);
            EEPROM.write(11, timerstates[5]);
            EEPROM.write(12, timerstates[6]);
            EEPROM.write(13, timerstates[7]);
            timerstates[8] = timerstates[7];
            timerstates[9] = timerstates[7];

            EEPROM.write(14, timerhour[0]);
            EEPROM.write(15, timerhour[1]);
            EEPROM.write(16, timerhour[2]);
            EEPROM.write(17, timerhour[3]);
            EEPROM.write(18, timerhour[4]);
            EEPROM.write(19, timerhour[5]);
            EEPROM.write(20, timerhour[6]);
            EEPROM.write(21, timerhour[7]);

            EEPROM.write(22, timerminute[0]);
            EEPROM.write(23, timerminute[1]);
            EEPROM.write(24, timerminute[2]);
            EEPROM.write(25, timerminute[3]);
            EEPROM.write(26, timerminute[4]);
            EEPROM.write(27, timerminute[5]);
            EEPROM.write(28, timerminute[6]);
            EEPROM.write(29, timerminute[7]);
            sprintf(ret, "y\n");
        }
        break;


    case LISE:
        if (strcmp(value, "b0") == 0) { // set light to day 1
            switchMode = TM_DAY1;
            actualLightValues->save();
            actualLightValues = &day1Values;
        } else
        if (strcmp(value, "b1") == 0) {
            switchMode = TM_DAY2;
            actualLightValues->save();
            actualLightValues = &day2Values;
        } else
        if (strcmp(value, "b2") == 0) {
            switchMode = TM_NIGHT1;
            actualLightValues->save();
            actualLightValues = &night1Values;
        } else
        if (strcmp(value, "b3") == 0) {
            switchMode = TM_NIGHT1;
            actualLightValues->save();
            actualLightValues = &night2Values;
        } else
        if (strcmp(value, "b4") == 0) {
            switchMode = TM_NIGHT2;
            actualLightValues->save();
            actualLightValues = &offValues;
        } else
        if (strcmp(value, "b5") == 0) { // set light mode to AUTO
            switchMode = TM_AUTO;
            actualLightValues->save();
            EEPROM.write(30, switchMode);
        }
        sprintf(ret, "y\n");
        break;

    }

    if (needResp) {
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

    // read time only 3 times per second
    if (i2cReadTimeCounter > 50) {
        now = RTC.now();
        i2cReadTimeCounter = 0;
    }

    // if serial receive command was received
    // call serialInterface, parse and return data back
    if (RECVCOMPL) {
        serialInterface();
    }

    // read temperature sensors aproximatelly
    // one times per one second
    if (temperatureReadTimeCounter > 430) {
        if (temperatureSenzorStatus[0] == 1) {
            t0.readTemperature();
            te0 = t0.getCelsius();
            myRound(&te0);
        }
        if (temperatureSenzorStatus[1] == 1) {
            t1.readTemperature();
            te1 = t1.getCelsius();
            myRound(&te1);
        }
        if (temperatureSenzorStatus[2] == 1) {
            t2.readTemperature();
            te2 = t2.getCelsius();
            myRound(&te2);
        }
        temperatureReadTimeCounter = 0;
    }
}

void switchLight(int i) {
    if (actualLightValues->flag != timerstates[i]) {

        if (timerstates[i] == TM_OFF) {
            actualLightValues = &offValues;
        } else
        if (timerstates[i] == TM_DAY1) {
            actualLightValues = &day1Values;
        } else
        if (timerstates[i] == TM_DAY2) {
            actualLightValues = &day2Values;
        } else
        if (timerstates[i] == TM_NIGHT1) {
            actualLightValues = &night1Values;
        } else
        if (timerstates[i] == TM_NIGHT2) {
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

        minutes     = timerminute[i] + (timerhour[i] * 60);
        nextMinutes = timerminute[j] + (timerhour[j] * 60);

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

    if (switchMode == TM_AUTO) {
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

    timerstates[0] = EEPROM.read(6);
    timerstates[1] = EEPROM.read(7);
    timerstates[2] = EEPROM.read(8);
    timerstates[3] = EEPROM.read(9);
    timerstates[4] = EEPROM.read(10);
    timerstates[5] = EEPROM.read(11);
    timerstates[6] = EEPROM.read(12);
    timerstates[7] = EEPROM.read(13);
    timerstates[8] = timerstates[7];
    timerstates[9] = timerstates[7];

    timerhour[0] = EEPROM.read(14);
    timerhour[1] = EEPROM.read(15);
    timerhour[2] = EEPROM.read(16);
    timerhour[3] = EEPROM.read(17);
    timerhour[4] = EEPROM.read(18);
    timerhour[5] = EEPROM.read(19);
    timerhour[6] = EEPROM.read(20);
    timerhour[7] = EEPROM.read(21);
    timerhour[8] = 23;
    timerhour[9] = 0;

    timerminute[0] = EEPROM.read(22);
    timerminute[1] = EEPROM.read(23);
    timerminute[2] = EEPROM.read(24);
    timerminute[3] = EEPROM.read(25);
    timerminute[4] = EEPROM.read(26);
    timerminute[5] = EEPROM.read(27);
    timerminute[6] = EEPROM.read(28);
    timerminute[7] = EEPROM.read(29);
    timerminute[8] = 59;
    timerminute[9] = 0;

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
