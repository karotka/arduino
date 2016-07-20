/**
 * Graphical interface for Arduino Aquarium Computer
 * http://www.sainsmart.com/sainsmart-mega2560-board-3-5-tft-lcd-module-display-shield-kit-for-atmel-atmega-avr-16au-atmega8u2.html
 * 3.2" 320x240 pixel SSD1289
 *
 */

#include "configure.h"
#include <Thermistor.h>
#include <Wire.h>
#include <UTFT_Buttons.h>
#include <EEPROM.h>
#include "utils.h"
#include <RTClib.h>
#include "TCN75a.h"

RTC_DS1307 RTC;
DateTime now;
NewTime  newTime;
TCN75A temperature;
Thermistor t0(A0);
Thermistor t1(A1);
Thermistor t2(A2);
Thermistor t3(A3);

volatile int x, y;
volatile int xC, xW, xR, xG, xB = 38;
volatile int xCC, xWC, xRC, xGC, xBC = 42;
volatile int actualWW, actualCW, targetWW, targetCW;

uint8_t lightStates[8];
uint8_t hourStates[8];
uint8_t minuteStates[8];

uint8_t co2states[6];
uint8_t co2hour[6];
uint8_t co2minute[6];

uint8_t temperatureSenzorStatus[4];

volatile uint8_t page;
volatile uint8_t actualCo2state;
volatile uint8_t lightStatesNow;
volatile uint8_t actualLightState;
volatile uint8_t switchMode;
volatile uint8_t pressedButton;
volatile uint8_t flashWrote;
volatile uint8_t i2cReadTimeCounter;

volatile int timerCounter2 = 0;
volatile int temperatureReadTimeCounter;

volatile int sumDateNow = 0;
volatile int dateNow = 0;

char strTemp[4];
char strTime[9];
char strDate[10];
char str[19];

extern uint8_t GroteskBold24x48[];
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

//HTU21D temperature;
float temperatureMin[60]  = {0};
float temperatureHour[60] = {0};
float tempDataTable[75]   = {0};

volatile uint8_t tempPointer = 0;
volatile uint8_t newTemperature;
volatile float   tempSum = 0;
volatile float   actualTemp = 0;
volatile int     tempMinPointer = 0;


void setup() {
    actualLightState = MODE_NONE;
    lightStatesNow = MODE_NONE;
    flashWrote = false;

    pinMode(RELE_PIN, OUTPUT);

    Wire.begin();
    RTC.begin();

    if (!RTC.isrunning()) {
        Serial.println("RTC is NOT running!");
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }

    temperature.begin();
    t0.begin();
    t1.begin();
    t2.begin();
    t3.begin();

    eepromRead();

    // Initial setup
    myGLCD.InitLCD(LANDSCAPE);
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0);

    myTouch.InitTouch(LANDSCAPE);
    myTouch.setPrecision(PREC_LOW);

    myButtons.setTextFont(BigFont);

    page = PAGE_HOME;

    pinInit();
    timer2set();

    defaultLights();

    myButtons.setButtonColors(VGA_WHITE, VGA_WHITE, VGA_WHITE,
                              VGA_RED, VGA_BLUE, VGA_GRAY);
}

void pinInit(void) {
    DDRK = 0xff;
    PORTK = 0x0;
}

void timer2set(void) {
    TIMSK2 |= (1 << TOIE2);  // Enable Overflow Interrupt Enable
    TCNT2 = 0;               // Initialize Counter
}

ISR(TIMER2_OVF_vect) {
    //cli();

    timerCounter2++;
    i2cReadTimeCounter++;
    temperatureReadTimeCounter++;

    if (switchMode == MODE_AUTO && !(timerCounter2 % 200)) {
        checkTimer();
        checkCo2();

        // dimming process
        if (actualCW > targetCW) {
            analogWrite(LED_COOL_WHITE, actualCW--);
        }
        if (actualCW < targetCW) {
            analogWrite(LED_COOL_WHITE, ++actualCW);
        }
        if (actualWW > targetWW) {
            analogWrite(LED_WHITE, actualWW--);
        }
        if (actualWW < targetWW) {
            analogWrite(LED_WHITE, ++actualWW);
        }
    }

    // return to homepage
    if (timerCounter2 > RETURN_DELAY) {
        timerCounter2 = 0;

        if (page != PAGE_HOME) {
            page = PAGE_RETURN_MOME;
        }
    }

    /*
     * temperature average per minute
     * each minute add one new value
     */
    if (newTemperature) {
        newTemperature = false;
        tempPointer++;
        temperatureMin[tempPointer] = actualTemp;

        /*
         * if number of values is 60 make average and
         * write it into temperatureHour
         */
        if (tempPointer == 60) {
            tempPointer = 0;

            // count avr min temperature
            tempSum = 0;
            for (uint8_t i = 0; i < 60; i++) {
                tempSum += temperatureMin[i];
            }

            temperatureHour[tempMinPointer] = tempSum / 60;
            tempMinPointer++;

            /*
             * if number of values is 60 make average and
             * write it into tempDataTable on the end of array
             */
            if (tempMinPointer == 60) {
                // reset tempMinutePointer
                tempMinPointer = 0;

                // shift values in array
                for (uint8_t i = 1; i < 75; i++) {
                    tempDataTable[i - 1] = tempDataTable[i];
                }

                tempSum = 0;
                // count new last value
                for (uint8_t i = 0; i < 60; i++) {
                    tempSum += temperatureHour[i];
                }

                // write last value into the end of the array
                tempDataTable[74] = tempSum / 60;
            }
        }
    }

    if (now.hour() == 0 && flashWrote == false) {
        // write array into EEEPROM
        EEPROM_writeAnything(128, tempDataTable);
        flashWrote = true;
    }
    if (now.hour() == 2) {
        flashWrote = false;
    }

}

void loop() {
    //PORTK &= ~(1 << PK7);
    //PORTK |= (1 << PK7);

    // read temperature each one second
    if (temperatureReadTimeCounter > 430) {
        if (temperatureSenzorStatus[0] == 1) {
            t0.readTemperature();
        }
        if (temperatureSenzorStatus[1] == 1) {
            t1.readTemperature();
        }
        if (temperatureSenzorStatus[2] == 1) {
            t2.readTemperature();
        }
        if (temperatureSenzorStatus[3] == 1) {
            t3.readTemperature();
        }
        newTemperature = true;
        actualTemp = temperature.readTemperature();
        temperatureReadTimeCounter = 0;
    }

    switch (page) {

    case PAGE_RETURN_MOME:
        myButtons.deleteAllButtons();
        myGLCD.clrScr();
        lightStatesNow = 3;
        page = PAGE_HOME;
        break;

    case PAGE_HOME:
        if (i2cReadTimeCounter > 50) {
            now = RTC.now();
            drawHomeScreen();
            i2cReadTimeCounter = 0;
        }

        if (myTouch.dataAvailable() == true) {
            myGLCD.clrScr();
            page = PAGE_SELECT;
            drawSelectScreen();
            timerCounter2 = 0;
        }
        break;

    case PAGE_SELECT:
        if (i2cReadTimeCounter > 50) {
            i2cReadTimeCounter = 0;
            now = RTC.now();
            drawTime();
        }

        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawLightControl();
                setMode();
                page = PAGE_SET_LIGHT;
            }

            if (pressedButton == 1) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawTimerScreen();
                page = PAGE_SET_TIMER;
            }

            if (pressedButton == 2) {
                newTime.day    = now.day();
                newTime.month  = now.month();
                newTime.year   = now.year();
                newTime.hour   = now.hour();
                newTime.minute = now.minute();
                newTime.second = now.second();

                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawTimeControl();
                page = PAGE_SET_TIME;
            }

            if (pressedButton == 3) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawCo2Screen();
                page = PAGE_SET_CO2;
            }

            if (pressedButton == 4) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawTempScreen();
                page = PAGE_TEMP;
            }

            if (pressedButton == 5) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatesNow = 3;
                page = PAGE_HOME;
            }
#if DEBUG == 1
            if (pressedButton == 6) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawRegDebug();
                page = PAGE_DEBUG;
            }
#endif
        }
#if DEBUG == 1
        debug();
#endif
        break;

#if DEBUG == 1
    case PAGE_DEBUG:
        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;

            pressedButton = myButtons.checkButtons();
            if (pressedButton == 0) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatesNow = 3;
                page = PAGE_HOME;
            }
        }
        break;
#endif

    case PAGE_SET_LIGHT:
        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;
            actualLightState = MODE_NONE;

            pressedButton = myButtons.checkButtons();

            x = myButtons.Touch->getX();
            y = myButtons.Touch->getY();

            drawTouchLedArea();

            if (pressedButton == 0) {
                // set light to day
                lightsDay();
            }
            if (pressedButton == 1) {
                // set light to off
                lightsOff();
            }
            if (pressedButton == 2) {
                // set light to night
                setNight();
            }
            if (pressedButton == 3) {
                // set light mode
                switchMode++;
                if (switchMode == 2) {
                    switchMode = MODE_AUTO;
                }
                setMode();
                EEPROM.write(24, switchMode);
            }

            if (pressedButton == 4) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatesNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_SET_TIME:

        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                newTime.hour--;
                if (newTime.hour == 255) newTime.hour = 23;
                drawNewTime();
            }
            if (pressedButton == 1) {
                newTime.hour++;
                if (newTime.hour > 23) newTime.hour = 0;
                drawNewTime();
            }
            if (pressedButton == 2) {
                newTime.minute--;
                if (newTime.minute == 255) newTime.minute = 59;
                drawNewTime();
            }
            if (pressedButton == 3) {
                newTime.minute++;
                if (newTime.minute > 59) newTime.minute = 0;
                drawNewTime();
            }
            if (pressedButton == 4) {
                newTime.second--;
                if (newTime.second == 255) newTime.second = 59;
                drawNewTime();
            }
            if (pressedButton == 5) {
                newTime.second++;
                if (newTime.second > 59) newTime.second = 0;
                drawNewTime();
            }
            if (pressedButton == 6) {
                newTime.day--;
                if (newTime.day < 1) newTime.day = 31;
                drawNewTime();
            }
            if (pressedButton == 7) {
                newTime.day++;
                if (newTime.day > 31) newTime.day = 1;
                drawNewTime();
            }
            if (pressedButton == 8) {
                newTime.month--;
                if (newTime.month < 1) newTime.month = 12;
                drawNewTime();
            }
            if (pressedButton == 9) {
                newTime.month++;
                if (newTime.month > 12) newTime.month = 1;
                drawNewTime();
            }
            if (pressedButton == 10) {
                newTime.year--;
                if (newTime.year < 2000) newTime.year = 2050;
                drawNewTime();
            }
            if (pressedButton == 11) {
                newTime.year++;
                if (newTime.year > 2050) newTime.year = 2000;
                drawNewTime();
            }

            // save
            if (pressedButton == 12) {
                RTC.adjust(newTime);
            }

            // home
            if (pressedButton == 13) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatesNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_TEMP:
        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                temperatureSenzorStatus[0]++;
                setTempButton(0);
                EEPROM.write(42, temperatureSenzorStatus[0]);
            }
            if (pressedButton == 1) {
                temperatureSenzorStatus[1]++;
                setTempButton(1);
                EEPROM.write(43, temperatureSenzorStatus[1]);
            }
            if (pressedButton == 2) {
                temperatureSenzorStatus[2]++;
                setTempButton(2);
                EEPROM.write(44, temperatureSenzorStatus[2]);
            }
            if (pressedButton == 3) {
                temperatureSenzorStatus[3]++;
                setTempButton(3);
                EEPROM.write(45, temperatureSenzorStatus[3]);
            }

            if (pressedButton == 4) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatesNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_SET_CO2:
        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;
            pressedButton = myButtons.checkButtons();

            // first timer
            if (pressedButton == 0) {
                co2hour[0]--;
                if (co2hour[0] == 255) {
                    co2hour[0] = 23;
                }
                setCo2TimerTime(0, 10, 8);
            }
            if (pressedButton == 1) {
                co2hour[0]++;
                if (co2hour[0] > 23) {
                    co2hour[0] = 0;
                }
                setCo2TimerTime(0, 10, 8);
            }
            if (pressedButton == 2) {
                co2minute[0]--;
                if (co2minute[0] == 255) {
                    co2minute[0] = 59;
                }
                setCo2TimerTime(0, 10, 8);
            }
            if (pressedButton == 3) {
                co2minute[0]++;
                if (co2minute[0] > 59) {
                    co2minute[0] = 0;
                }
                setCo2TimerTime(0, 10, 8);
            }
            if (pressedButton == 4) {
                setCo2Type(4, 0);
            }

            if (pressedButton == 5) {
                co2hour[1]--;
                if (co2hour[1] == 255) {
                    co2hour[1] = 23;
                }
                setCo2TimerTime(1, 10, 64);
            }
            if (pressedButton == 6) {
                co2hour[1]++;
                if (co2hour[1] > 23) {
                    co2hour[1] = 0;
                }
                setCo2TimerTime(1, 10, 64);
            }
            if (pressedButton == 7) {
                co2minute[1]--;
                if (co2minute[1] == 255) {
                    co2minute[1] = 59;
                }
                setCo2TimerTime(1, 10, 64);
            }
            if (pressedButton == 8) {
                co2minute[1]++;
                if (co2minute[1] > 59) {
                    co2minute[1] = 0;
                }
                setCo2TimerTime(1, 10, 64);
            }
            if (pressedButton == 9) {
                setCo2Type(9, 1);
            }

            // second column
            if (pressedButton == 10) {
                co2hour[2]--;
                if (co2hour[2] == 255) {
                    co2hour[2] = 23;
                }
                setCo2TimerTime(2, 176, 8);
            }
            if (pressedButton == 11) {
                co2hour[2]++;
                if (co2hour[2] > 23) {
                    co2hour[2] = 0;
                }
                setCo2TimerTime(2, 176, 8);
            }
            if (pressedButton == 12) {
                co2minute[2]--;
                if (co2minute[2] == 255) {
                    co2minute[2] = 59;
                }
                setCo2TimerTime(2, 176, 8);
            }
            if (pressedButton == 13) {
                co2minute[2]++;
                if (co2minute[2] > 59) {
                    co2minute[2] = 0;
                }
                setCo2TimerTime(2, 176, 8);
            }
            if (pressedButton == 14) {
                setCo2Type(14, 2);
            }

            if (pressedButton == 15) {
                co2hour[3]--;
                if (co2hour[3] == 255) {
                    co2hour[3] = 23;
                }
                setCo2TimerTime(3, 176, 64);
            }
            if (pressedButton == 16) {
                co2hour[3]++;
                if (co2hour[3] > 23) {
                    co2hour[3] = 0;
                }
                setCo2TimerTime(3, 176, 64);
            }
            if (pressedButton == 17) {
                co2minute[3]--;
                if (co2minute[3] == 255) {
                    co2minute[3] = 59;
                }
                setCo2TimerTime(3, 176, 64);
            }
            if (pressedButton == 18) {
                co2minute[3]++;
                if (co2minute[3] > 59) {
                    co2minute[3] = 0;
                }
                setCo2TimerTime(3, 176, 64);
            }
            if (pressedButton == 19) {
                setCo2Type(19, 3);
            }

            // save values
            if (pressedButton == 20) {
                EEPROM.write(25,  co2states[0]);
                EEPROM.write(26,  co2states[1]);
                EEPROM.write(27,  co2states[2]);
                EEPROM.write(28,  co2states[3]);
                co2states[4] = co2states[3];
                co2states[5] = co2states[3];

                EEPROM.write(29, co2hour[0]);
                EEPROM.write(30, co2hour[1]);
                EEPROM.write(31, co2hour[2]);
                EEPROM.write(32, co2hour[3]);

                EEPROM.write(33, co2minute[0]);
                EEPROM.write(34, co2minute[1]);
                EEPROM.write(35, co2minute[2]);
                EEPROM.write(36, co2minute[3]);
            }

            // home button
            if (pressedButton == 21) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatesNow = 3;
                page = PAGE_HOME;
            }

        }
        break;

    case PAGE_SET_TIMER:
        if (myTouch.dataAvailable() == true) {
            pressedButton = myButtons.checkButtons();
            timerCounter2 = 0;

            // first timer
            if (pressedButton == 0) {
                hourStates[0]--;
                if (hourStates[0] == 255) {
                    hourStates[0] = 23;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 1) {
                hourStates[0]++;
                if (hourStates[0] > 23) {
                    hourStates[0] = 0;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 2) {
                minuteStates[0]--;
                if (minuteStates[0] == 255) {
                    minuteStates[0] = 59;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 3) {
                minuteStates[0]++;
                if (minuteStates[0] > 59) {
                    minuteStates[0] = 0;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 4) {
                setLightType(4, 0);
            }

            // second time
            if (pressedButton == 5) {
                hourStates[1]--;
                if (hourStates[1] == 255) {
                    hourStates[1] = 23;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 6) {
                hourStates[1]++;
                if (hourStates[1] > 23) {
                    hourStates[1] = 0;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 7) {
                minuteStates[1]--;
                if (minuteStates[1] == 255) {
                    minuteStates[1] = 59;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 8) {
                minuteStates[1]++;
                if (minuteStates[1] > 59) {
                    minuteStates[1] = 0;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 9) {
                setLightType(9, 1);
            }

            // third time
            if (pressedButton == 10) {
                hourStates[2]--;
                if (hourStates[2] == 255) {
                    hourStates[2] = 23;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 11) {
                hourStates[2]++;
                if (hourStates[2] > 23) {
                    hourStates[2] = 0;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 12) {
                minuteStates[2]--;
                if (minuteStates[2] == 255) {
                    minuteStates[2] = 59;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 13) {
                minuteStates[2]++;
                if (minuteStates[2] > 59) {
                    minuteStates[2] = 0;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 14) {
                setLightType(14, 2);
            }

            if (pressedButton == 15) {
                hourStates[3]--;
                if (hourStates[3] == 255) {
                    hourStates[3] = 23;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 16) {
                hourStates[3]++;
                if (hourStates[3] > 23) {
                    hourStates[3] = 0;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 17) {
                minuteStates[3]--;
                if (minuteStates[3] == 255) {
                    minuteStates[3] = 59;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 18) {
                minuteStates[3]++;
                if (minuteStates[3] > 59) {
                    minuteStates[3] = 0;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 19) {
                setLightType(19, 3);
            }

            if (pressedButton == 20) {
                hourStates[4]--;
                if (hourStates[4] == 255) {
                    hourStates[4] = 23;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 21) {
                hourStates[4]++;
                if (hourStates[4] > 23) {
                    hourStates[4] = 0;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 22) {
                minuteStates[4]--;
                if (minuteStates[4] == 255) {
                    minuteStates[4] = 59;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 23) {
                minuteStates[4]++;
                if (minuteStates[4] > 59) {
                    minuteStates[4] = 0;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 24) {
                setLightType(24, 4);
            }

            if (pressedButton == 25) {
                hourStates[5]--;
                if (hourStates[5] == 255) {
                    hourStates[5] = 23;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 26) {
                hourStates[5]++;
                if (hourStates[5] > 23) {
                    hourStates[5] = 0;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 27) {
                minuteStates[5]--;
                if (minuteStates[5] == 255) {
                    minuteStates[5] = 59;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 28) {
                minuteStates[5]++;
                if (minuteStates[5] > 59) {
                    minuteStates[5] = 0;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 29) {
                setLightType(29, 5);
            }

            // save values
            if (pressedButton == 30) {
                EEPROM.write(6,  lightStates[0]);
                EEPROM.write(7,  lightStates[1]);
                EEPROM.write(8,  lightStates[2]);
                EEPROM.write(9,  lightStates[3]);
                EEPROM.write(10, lightStates[4]);
                EEPROM.write(11, lightStates[5]);
                lightStates[6] = lightStates[5];
                lightStates[7] = lightStates[5];

                EEPROM.write(12, hourStates[0]);
                EEPROM.write(13, hourStates[1]);
                EEPROM.write(14, hourStates[2]);
                EEPROM.write(15, hourStates[3]);
                EEPROM.write(16, hourStates[4]);
                EEPROM.write(17, hourStates[5]);

                EEPROM.write(18, minuteStates[0]);
                EEPROM.write(19, minuteStates[1]);
                EEPROM.write(20, minuteStates[2]);
                EEPROM.write(21, minuteStates[3]);
                EEPROM.write(22, minuteStates[4]);
                EEPROM.write(23, minuteStates[5]);
            }

            // home button
            if (pressedButton == 31) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatesNow = 3;
                page = PAGE_HOME;
            }
        }
        break;
    }

#if DEBUG == 1
    debug();
#endif
}

void drawTempScreen() {
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawHLine(0, 180, 319);
    myGLCD.drawLine(20, 205, 20, 0);
    myGLCD.setFont(SmallFont);

    int startx = 20;
    int starty = 300;

    // print vertical line and numbers
    myGLCD.setColor(0, 255, 255);
    myGLCD.print("C", 5, 4);
    for (int i = 140; i < starty; i = i + 20) {
        myGLCD.setColor(255, 255, 255);
        myGLCD.printNumI(i / 10, 2, starty - i);
        myGLCD.setColor(80, 80, 80);
        myGLCD.drawLine(20, starty -i, 319, starty -i);
    }
    myGLCD.drawLine(20, 1, 319, 1);

    // print chart
    myGLCD.setColor(0, 255, 255);
    myGLCD.print("hours", 25, 182);
    for (int i = 0; i < 75; i++) {
        int j = i + 1;
        int x1 = startx + (i * 4);
        int y1 = starty - tempDataTable[i] * 10;
        int x2 = startx + (j * 4);
        int y2 = starty - tempDataTable[j] * 10;

        if (i == 24) {
            myGLCD.setColor(255, 255, 255);
            myGLCD.printNumI(-48, x1 - 13, 182);
            myGLCD.setColor(80, 80, 80);
            myGLCD.drawLine(x1 + 1, 179, x1 + 1, 0);
        }
        if (i == 48) {
            myGLCD.setColor(255, 255, 255);
            myGLCD.printNumI(-24, x1 - 13, 182);
            myGLCD.setColor(80, 80, 80);
            myGLCD.drawLine(x1 + 1, 179, x1 + 1, 0);
        }
        if (i == 74) {
            myGLCD.setColor(255, 255, 255);
            myGLCD.printNumI(0, x1 - 5, 182);
            myGLCD.setColor(80, 80, 80);
            myGLCD.drawLine(x1, 179, x1, 0);
        }

        if (j < 75) {
            // print chart line
            myGLCD.setColor(255, 0, 0);
            myGLCD.drawLine(x1, y1, x2, y2);
        }
    }

    myButtons.addButton(2,   197, 50,  30, "", 0, VGA_GRAY);
    myButtons.addButton(55,  197, 50,  30, "", 0, VGA_GRAY);
    myButtons.addButton(108, 197, 50,  30, "", 0, VGA_GRAY);
    myButtons.addButton(161, 197, 50,  30, "", 0, VGA_GRAY);

    myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();

    for (uint8_t i = 0; i < 4; i++) {
        setTempButton(i);
    }
}

void setTempButton(uint8_t id) {
    sprintf (str, "TE%01d", id + 1);

    if (temperatureSenzorStatus[id] > 1) {
        temperatureSenzorStatus[id] = 0;
    }
    if (temperatureSenzorStatus[id] == 0) {
        myButtons.relabelButton(id, str, true, VGA_GRAY);
    } else {
        myButtons.relabelButton(id, str, true, VGA_GREEN);
    }
}

void drawCo2Screen() {
    sprintf (strDate, "%02dh:%02dm.", co2hour[0], co2minute[0]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 8);

    myButtons.addButton(2,   28, 22,  30, "<");
    myButtons.addButton(28,  28, 22,  30, ">");
    myButtons.addButton(62,  28, 22,  30, "<");
    myButtons.addButton(88,  28, 22,  30, ">");
    myButtons.addButton(116, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", co2hour[1], co2minute[1]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 64);

    myButtons.addButton(2,   85, 22,  30, "<");
    myButtons.addButton(28,  85, 22,  30, ">");
    myButtons.addButton(62,  85, 22,  30, "<");
    myButtons.addButton(88,  85, 22,  30, ">");
    myButtons.addButton(116, 85, 34,  30, "");


    sprintf (strDate, "%02dh:%02dm.", co2hour[2], co2minute[2]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 8);

    myButtons.addButton(166, 28, 22,  30, "<");
    myButtons.addButton(192, 28, 22,  30, ">");
    myButtons.addButton(226, 28, 22,  30, "<");
    myButtons.addButton(252, 28, 22,  30, ">");
    myButtons.addButton(280, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", co2hour[3], co2minute[3]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 64);

    myButtons.addButton(166, 85, 22,  30, "<");
    myButtons.addButton(192, 85, 22,  30, ">");
    myButtons.addButton(226, 85, 22,  30, "<");
    myButtons.addButton(252, 85, 22,  30, ">");
    myButtons.addButton(280, 85, 34,  30, "");

    myButtons.addButton(10,  197, 90,  30, "SAVE", 0, VGA_RED);
    myButtons.addButton(240, 197, 70,  30, "HOME");

    setCo2Button(4,  0);
    setCo2Button(9,  1);
    setCo2Button(14, 2);
    setCo2Button(19, 3);

    myButtons.drawButtons();
}

void lightsOff() {
    analogWrite(LED_COOL_WHITE, 255);
    analogWrite(LED_WHITE, 255);
    analogWrite(LED_RED,   255);
    analogWrite(LED_GREEN, 255);
    analogWrite(LED_BLUE,  255);
    actualLightState = MODE_OFF;
}

void lightsDay() {
    analogWrite(LED_COOL_WHITE, xCC);
    analogWrite(LED_WHITE, xWC);
    analogWrite(LED_RED,   255);
    analogWrite(LED_GREEN, 255);
    analogWrite(LED_BLUE,  255);
    actualLightState = MODE_DAY;
}

void setNight() {
    analogWrite(LED_COOL_WHITE, 255);
    analogWrite(LED_WHITE, 255);
    analogWrite(LED_RED,   xRC);
    analogWrite(LED_GREEN, xGC);
    analogWrite(LED_BLUE,  xBC);
    actualLightState = MODE_NIGHT;
}

void setMode() {
    if (switchMode == MODE_AUTO) {
        myButtons.relabelButton(3, "AUT", true);
        myButtons.disableButton(0, true);
        myButtons.disableButton(1, true);
        myButtons.disableButton(2, true);
    }
    if (switchMode == MODE_MANUAL) {
        myButtons.relabelButton(3, "MAN", true);
        myButtons.enableButton(0, true);
        myButtons.enableButton(1, true);
        myButtons.enableButton(2, true);
    }
}

void setTimerTime(int id, int x, int y) {
    sprintf (strDate, "%02dh:%02dm.", hourStates[id], minuteStates[id]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, x, y);
}

void setCo2TimerTime(int id, int x, int y) {
    sprintf (strDate, "%02dh:%02dm.", co2hour[id], co2minute[id]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, x, y);
}

void setCo2Type(int btn, int id) {
    co2states[id]++;
    if (co2states[id] > 1) {
        co2states[id] = 0;
    }
    setCo2Button(btn, id);
}

void setCo2Button(int btn, int id) {
    if (co2states[id] == CO2_OFF) {
        myButtons.relabelButton(btn, "OF", true);
    } else
    if (co2states[id] == CO2_ON) {
        myButtons.relabelButton(btn, "ON", true);
    }
}

void setLightType(int btn, int id) {
    lightStates[id]++;
    if (lightStates[id] > 2) {
        lightStates[id] = 0;
    }
    setLightButton(btn, id);
}

void setLightButton(int btn, int id) {
    if (lightStates[id] == MODE_OFF) {
        myButtons.relabelButton(btn, "OF", true);
    } else
    if (lightStates[id] == MODE_DAY) {
        myButtons.relabelButton(btn, "DA", true);
    } else
    if (lightStates[id] == MODE_NIGHT) {
        myButtons.relabelButton(btn, "NI", true);
    } else {
        myButtons.relabelButton(btn, "Un", true);
    }
}

void drawNewTime() {
    myGLCD.setColor(200, 200, 200);
    myGLCD.setFont(GroteskBold24x48);

    sprintf (strTime, "%02d:%02d:%02d", newTime.hour, newTime.minute, newTime.second);
    myGLCD.print(strTime, CENTER, 3);

    sprintf (strDate, "%02d.%02d.%02d", newTime.day, newTime.month, newTime.year);
    myGLCD.print(strDate, CENTER, 90);
}

void drawTimeControl() {
    drawNewTime();

    myButtons.addButton(60,   52, 25,  30, "<");
    myButtons.addButton(90,   52, 25,  30, ">");
    myButtons.addButton(133,  52, 25,  30, "<");
    myButtons.addButton(163,  52, 25,  30, ">");
    myButtons.addButton(205,  52, 25,  30, "<");
    myButtons.addButton(235,  52, 25,  30, ">");

    myButtons.addButton(35,  139, 25,  30, "<");
    myButtons.addButton(65,  139, 25,  30, ">");
    myButtons.addButton(110, 139, 25,  30, "<");
    myButtons.addButton(140, 139, 25,  30, ">");
    myButtons.addButton(205, 139, 25,  30, "<");
    myButtons.addButton(235, 139, 25,  30, ">");

    myButtons.addButton(10,  197, 90,  30, "SAVE", 0, VGA_RED);
    myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();
}

void drawHomeScreen() {
    sprintf (strTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    myGLCD.setColor(162, 0, 0);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 27);

    sumDateNow = now.day() + now.month() + now.year();
    if (dateNow != sumDateNow) {
        sprintf (strDate, "%02d.%02d.%02d", now.day(), now.month(), now.year());
        myGLCD.setColor(150, 150, 150);
        myGLCD.setFont(BigFont);
        myGLCD.print(strDate, CENTER, 80);
    }

    dtostrf(actualTemp, 4, 1, strTemp);
    myGLCD.setColor(7, 56, 212);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTemp, 100, 112);
    myGLCD.print("C", 205, 112);

    myGLCD.setColor(90, 90, 90);
    myGLCD.setFont(BigFont);
    if (actualLightState != lightStatesNow) {
        myGLCD.print("                   ", CENTER, 185);
        if (switchMode == MODE_MANUAL) {
            sprintf (str, "MODE:MANUAL (%s)", dateStr[actualLightState]);
        } else {
            sprintf (str, "MODE:AUTO (%s)", dateStr[actualLightState]);
        }
        myGLCD.print(str, CENTER, 165);
        lightStatesNow = actualLightState;
    }

    myGLCD.print("    ", 250, 185);
    if (actualCo2state == CO2_OFF) {
        sprintf (str, "CO2:%s", co2Str[CO2_OFF]);
    } else {
        sprintf (str, "CO2:%s", co2Str[CO2_ON]);
    }
    myGLCD.print(str, CENTER, 185);

    if (actualCW > targetCW || actualCW < targetCW) {
        myGLCD.setColor(255, 255, 255);
        myGLCD.fillRect(30, 215, 285 - actualCW, 212);
        myGLCD.setColor(0, 0, 0);
        myGLCD.fillRect(284 - actualCW, 215, 285, 212);
    }
    if (actualWW > targetWW || actualWW < targetWW) {
        myGLCD.setColor(255, 223, 143);
        myGLCD.fillRect(30, 222, 285 - actualWW, 219);
        myGLCD.setColor(0, 0, 0);
        myGLCD.fillRect(284 - actualWW, 222, 285, 219);
    }
}

void drawSelectScreen() {
    myButtons.addButton(10,   90, 90,  30, "LIGHT");
    myButtons.addButton(115,  90, 90,  30, "TIMER");
    myButtons.addButton(220,  90, 90,  30, "TIME");

    myButtons.addButton(10,  140, 90,  30, "CO2");
    myButtons.addButton(115, 140, 90,  30, "TEMP");
    myButtons.addButton(220, 140, 90,  30, "HOME");

#if DEBUG == 1
    myButtons.addButton(115, 190, 90,  30, "DEBUG");
#endif

    myButtons.drawButtons();
}

void drawLightControl() {
    myGLCD.setFont(BigFont);
    myGLCD.print("Light controler", CENTER, 10);
    myGLCD.print("CW", 2,  40);
    myGLCD.print("WW", 2,  70);
    myGLCD.print("R:", 10, 106);
    myGLCD.print("G:", 10, 136);
    myGLCD.print("B:", 10, 166);
    myGLCD.setColor(255, 0, 0);
    myGLCD.drawLine(0, 30, 319, 30);

    myGLCD.setColor(255, 255, 255);

    myGLCD.drawRect(40,  41, 310,  54); // CW  slider
    myGLCD.drawRect(40,  71, 310,  84); // WW  slider
    myGLCD.drawRect(40, 106, 310, 121); // Red   slider
    myGLCD.drawRect(40, 136, 310, 149); // Green slider
    myGLCD.drawRect(40, 166, 310, 179); // Blue  slider

    drawSlidersCC();
    drawSlidersWC();
    drawSlidersRC();
    drawSlidersGC();
    drawSlidersBC();

    myButtons.addButton(2,   197, 50,  30, "DAY");
    myButtons.addButton(55,  197, 60,  30, "OFF");
    myButtons.addButton(120, 197, 42,  30, "NI");
    myButtons.addButton(169, 197, 50,  30, "AUT");
    myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();
}

void drawTime() {
    sprintf (strTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    myGLCD.setColor(200, 200, 200);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 20);
}

void drawTouchLedArea() {
    int oldXWC = xW;
    int oldXCC = xC;
    int oldXRC = xR;
    int oldXGC = xG;
    int oldXBC = xB;

    // Area of the Main color slider
    if( (y >= 38) && (y <= 54)) {
        xC = x; // Stores the X value where the screen has been pressed in to variable xR
        if (xC <= 42) { // Confines the area of the slider to be above 38 pixels
            xC = 42;
        }
        if (xC >= 304) { /// Confines the area of the slider to be under 310 pixels
            xC = 304;
        }
        xCC = map(xC, 304, 42, 0, 255);
        analogWrite(LED_COOL_WHITE, xCC);
    }

    // Area of the Main color slider
    if( (y >= 68) && (y <= 84)) {
        xW = x; // Stores the X value where the screen has been pressed in to variable xR
        if (xW <= 42) { // Confines the area of the slider to be above 38 pixels
            xW = 42;
        }
        if (xW >= 304) { /// Confines the area of the slider to be under 310 pixels
            xW = 304;
        }
        xWC = map(xW, 304, 42, 0, 255);
        analogWrite(LED_WHITE, xWC);
    }

    // Area of the Red color slider
    if( (y >= 104) && (y <= 119)) {
        xR = x;
        if (xR <= 42) {
            xR = 42;
        }
        if (xR >= 304) {
            xR = 304;
        }
        xRC = map(xR, 304, 42, 0, 255);
        analogWrite(LED_RED,   xRC);
    }

    // Area of the Green color slider
    if( (y >= 134) && (y <= 149)) {
        xG = x;
        if (xG <= 42) {
            xG = 42;
        }
        if (xG >= 304) {
            xG = 304;
        }
        xGC = map(xG, 304, 42, 0, 255);
        analogWrite(LED_GREEN, xGC);
    }

    // Area of the Blue color slider
    if( (y >= 164) && (y <= 179)) {
        xB = x;
        if (xB <= 42) {
            xB = 42;
        }
        if (xB >= 304) {
            xB = 304;
        }
        xBC = map(xB, 304, 42, 0, 255);
        analogWrite(LED_BLUE,  xBC);
    }

    if (xC != oldXWC) {
        drawSlidersCC();
        EEPROM.write(1, xCC);
    }
    if (xW != oldXWC) {
        drawSlidersWC();
        EEPROM.write(2, xWC);
    }
    if (xR != oldXRC) {
        drawSlidersRC();
        EEPROM.write(3, xRC);
    }
    if (xG != oldXGC) {
        drawSlidersGC();
        EEPROM.write(4, xGC);
    }
    if (xB != oldXBC) {
        drawSlidersBC();
        EEPROM.write(5, xBC);
    }
}

void drawSlidersCC() {
    // Draws the positioners Cool White
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xC, 42, (xC + 4), 53); // Positioner

    myGLCD.setColor(255, 255, 255);
    myGLCD.fillRect(41, 42, (xC - 1),  53); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xC + 5), 42, 308, 53); // second rect
}

void drawSlidersWC() {
    // Draws the positioners Warm White
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xW, 72, (xW + 4), 83); // Positioner

    myGLCD.setColor(255, 223, 143);
    myGLCD.fillRect(41, 72, (xW - 1),  83); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xW + 5), 72, 308, 83); // second rect
}

void drawSlidersRC() {
    // Draws the positioners Red
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xR, 107, (xR + 4), 118); // Positioner

    myGLCD.setColor(255, 0, 0);
    myGLCD.fillRect(41, 107, (xR - 1),  118); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xR + 5), 107, 308, 118); // second rect
}

void drawSlidersGC() {
    // Draws the positioners Green
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xG, 137, (xG + 4), 148); // Positioner

    myGLCD.setColor(0, 255, 0);
    myGLCD.fillRect(41, 137, (xG - 1),  148); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xG + 5), 137, 308, 148); // second rect
}

void drawSlidersBC() {
    // Draws the positioners Blue
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xB, 167, (xB + 4), 178); // Positioner

    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(41, 167, (xB - 1),  178); // first

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xB + 5), 167, 308, 178); // second rect
}

void drawTimerScreen() {

    sprintf (strDate, "%02dh:%02dm.", hourStates[0], minuteStates[0]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 8);

    myButtons.addButton(2,   28, 22,  30, "<");
    myButtons.addButton(28,  28, 22,  30, ">");
    myButtons.addButton(62,  28, 22,  30, "<");
    myButtons.addButton(88,  28, 22,  30, ">");
    myButtons.addButton(116, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[1], minuteStates[1]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 64);

    myButtons.addButton(2,   85, 22,  30, "<");
    myButtons.addButton(28,  85, 22,  30, ">");
    myButtons.addButton(62,  85, 22,  30, "<");
    myButtons.addButton(88,  85, 22,  30, ">");
    myButtons.addButton(116, 85, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[2], minuteStates[2]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 122);

    myButtons.addButton(2,   142, 22,  30, "<");
    myButtons.addButton(28,  142, 22,  30, ">");
    myButtons.addButton(62,  142, 22,  30, "<");
    myButtons.addButton(88,  142, 22,  30, ">");
    myButtons.addButton(116, 142, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[3], minuteStates[3]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 8);

    myButtons.addButton(166, 28, 22,  30, "<");
    myButtons.addButton(192, 28, 22,  30, ">");
    myButtons.addButton(226, 28, 22,  30, "<");
    myButtons.addButton(252, 28, 22,  30, ">");
    myButtons.addButton(280, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[4], minuteStates[4]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 64);

    myButtons.addButton(166, 85, 22,  30, "<");
    myButtons.addButton(192, 85, 22,  30, ">");
    myButtons.addButton(226, 85, 22,  30, "<");
    myButtons.addButton(252, 85, 22,  30, ">");
    myButtons.addButton(280, 85, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[5], minuteStates[5]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 120);

    myButtons.addButton(166, 142, 22,  30, "<");
    myButtons.addButton(192, 142, 22,  30, ">");
    myButtons.addButton(226, 142, 22,  30, "<");
    myButtons.addButton(252, 142, 22,  30, ">");
    myButtons.addButton(280, 142, 34,  30, "");

    myButtons.addButton(10,  197, 90,  30, "SAVE", 0, VGA_RED);
    myButtons.addButton(240, 197, 70,  30, "HOME");

    setLightButton(4,  0);
    setLightButton(9,  1);
    setLightButton(14, 2);
    setLightButton(19, 3);
    setLightButton(24, 4);
    setLightButton(29, 5);

    myButtons.drawButtons();
}

void eepromRead() {
    xCC = EEPROM.read(1);
    xWC = EEPROM.read(2);
    xRC = EEPROM.read(3);
    xGC = EEPROM.read(4);
    xBC = EEPROM.read(5);

    xC = map(xCC, 0, 255, 304, 42);
    xW = map(xWC, 0, 255, 304, 42);
    xR = map(xRC, 0, 255, 304, 42);
    xG = map(xGC, 0, 255, 304, 42);
    xB = map(xBC, 0, 255, 304, 42);

    lightStates[0] = EEPROM.read(6);
    lightStates[1] = EEPROM.read(7);
    lightStates[2] = EEPROM.read(8);
    lightStates[3] = EEPROM.read(9);
    lightStates[4] = EEPROM.read(10);
    lightStates[5] = EEPROM.read(11);
    lightStates[6] = lightStates[5];
    lightStates[7] = lightStates[5];

    hourStates[0] = EEPROM.read(12);
    hourStates[1] = EEPROM.read(13);
    hourStates[2] = EEPROM.read(14);
    hourStates[3] = EEPROM.read(15);
    hourStates[4] = EEPROM.read(16);
    hourStates[5] = EEPROM.read(17);
    hourStates[6] = 23;
    hourStates[7] = 0;

    minuteStates[0] = EEPROM.read(18);
    minuteStates[1] = EEPROM.read(19);
    minuteStates[2] = EEPROM.read(20);
    minuteStates[3] = EEPROM.read(21);
    minuteStates[4] = EEPROM.read(22);
    minuteStates[5] = EEPROM.read(23);
    minuteStates[6] = 59;
    minuteStates[7] = 0;

    switchMode = EEPROM.read(24);

    co2states[0] = EEPROM.read(25);
    co2states[1] = EEPROM.read(26);
    co2states[2] = EEPROM.read(27);
    co2states[3] = EEPROM.read(28);
    co2states[4] = co2states[3];
    co2states[5] = co2states[3];

    co2hour[0] = EEPROM.read(29);
    co2hour[1] = EEPROM.read(30);
    co2hour[2] = EEPROM.read(31);
    co2hour[3] = EEPROM.read(32);
    co2hour[4] = 23;
    co2hour[5] = 0;

    co2minute[0] = EEPROM.read(33);
    co2minute[1] = EEPROM.read(34);
    co2minute[2] = EEPROM.read(35);
    co2minute[3] = EEPROM.read(36);
    co2minute[4] = 59;
    co2minute[5] = 0;

    temperatureSenzorStatus[0] = EEPROM.read(42);
    temperatureSenzorStatus[1] = EEPROM.read(43);
    temperatureSenzorStatus[2] = EEPROM.read(44);
    temperatureSenzorStatus[3] = EEPROM.read(45);

    EEPROM_readAnything(128, tempDataTable);
}

void defaultLights() {
    analogWrite(LED_COOL_WHITE, xCC);
    analogWrite(LED_WHITE, xWC);
    analogWrite(LED_RED, xRC);
    analogWrite(LED_GREEN, xGC);
    analogWrite(LED_BLUE, xBC);
}

/**
 * Check if actual time is bettween timer values
 * If yes, switch lights to right state
 **/
void checkTimer() {

    int minutes, nextMinutes, realMinute;
    realMinute = now.minute() + (now.hour() * 60);

    for(uint8_t i = 0; i < 8; i++) {
        uint8_t j = i + 1;
        if (i == 7) { j = 0; }

        minutes     = minuteStates[i] + (hourStates[i] * 60);
        nextMinutes = minuteStates[j] + (hourStates[j] * 60);

        if (minutes <= realMinute && realMinute < nextMinutes) {
            switchLight(i);
            break;
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

void switchLight(int i) {
    if (actualLightState != lightStates[i]) {
        actualLightState = lightStates[i];
        if (actualLightState == MODE_OFF) {
            // set off
            actualWW = OCR1B;
            actualCW = OCR1A;
            targetWW = 255;
            targetCW = 255;
            analogWrite(LED_RED,   255);
            analogWrite(LED_GREEN, 255);
            analogWrite(LED_BLUE,  255);
        }
        if (actualLightState == MODE_DAY) {
            // set day
            actualWW = OCR1B;
            actualCW = OCR1A;
            targetWW = xWC;
            targetCW = xCC;
            analogWrite(LED_RED,   255);
            analogWrite(LED_GREEN, 255);
            analogWrite(LED_BLUE,  255);
        }
        if (actualLightState == MODE_NIGHT) {
            // set night
            actualWW = OCR1B;
            actualCW = OCR1A;
            targetWW = 255;
            targetCW = 255;
            analogWrite(LED_RED,   xRC);
            analogWrite(LED_GREEN, xGC);
            analogWrite(LED_BLUE,  xBC);
        }
    }
}

#if DEBUG == 1
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
    (byte & 0x80 ? '1' : '0'), \
    (byte & 0x40 ? '1' : '0'), \
    (byte & 0x20 ? '1' : '0'), \
    (byte & 0x10 ? '1' : '0'), \
    (byte & 0x08 ? '1' : '0'), \
    (byte & 0x04 ? '1' : '0'), \
    (byte & 0x02 ? '1' : '0'), \
    (byte & 0x01 ? '1' : '0')

void debug() {
    char str[62];

    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);

    //sprintf (str, "BTN:%02d, PG:%d, R:%03d G:%03d B:%03d W:%03d",
    //         pressedButton, page, xRC, xGC, xBC, xWC);

    //sprintf (str, "Actual:%03d Now:%03d",
    //             actualLightState, lightStatesNow);
    //myGLCD.print(str, CENTER, 10);
    //
    //sprintf (str, "Slider R:%03d G:%03d B:%03d WW:%03d CW:%03d",
    //         xRC, xGC, xBC, xWC, xCC);
    //myGLCD.print(str, CENTER, 215);
    //
    //sprintf (str, "Ww a:%03d-t:%03d, Co: a%03d-t:%03d SM:%d",
    //         actualWW, targetWW, actualCW, targetCW, switchMode);

    //sprintf (str, "Temp avg:%3d-t:%03d, Co: a%03d-t:%03d SM:%d",
    //         actualWW, targetWW, actualCW, targetCW, switchMode);

    //sprintf (str, "NeT:%01d Tp:%02d Tmp:%02d TS:%04d 74:%03d",
    //         newTemperature, tempPointer, tempMinPointer, (int)tempSum, (int)tempDataTable[74]);

    //sprintf (str, "BTN: %d, page: %d, %d %d %d %d %d %d", pressedButton, page,
    //         lightStates[0], lightStates[1],lightStates[2],
    //         lightStates[3], lightStates[4], lightStates[5]);

    //sprintf (str, "xW: %d, xR: %d, xG: %d, xB: %d", xW, xR, xG, xB);
    //sprintf (str, "x: %d, y: %d, page: %d, btn: %d, W: %d, R: %d, G: %d, B: %d",
    //         x, y, page, pressedButton, xWC, xRC, xGC, xBC);

    float te0, te1, te2, te3;

    te0 = t0.getCelsius();
    myRound(&te0);

    te1 = t1.getCelsius();
    myRound(&te1);

    te2 = t2.getCelsius();
    myRound(&te2);

    te3 = t3.getCelsius();
    myRound(&te3);

    sprintf (str, "T0:%03d T1:%03d T2:%03d T3:%03d",
             (int)(te0*10), (int)(te1*10), (int)(te2*10), (int)(te3*10));

    //dtostrf(t, 2, 1, str);
    myGLCD.print(str, CENTER, 228);
}

void drawRegDebug() {
    sprintf(str, "TCCR0A:" BYTE_TO_BINARY_PATTERN " TCCR0B:" BYTE_TO_BINARY_PATTERN,
        BYTE_TO_BINARY(TCCR0A), BYTE_TO_BINARY(TCCR0B));
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);
    myGLCD.print(str, LEFT, 10);

    sprintf(str, "TCCR1A:" BYTE_TO_BINARY_PATTERN " TCCR1B:" BYTE_TO_BINARY_PATTERN,
        BYTE_TO_BINARY(TCCR1A), BYTE_TO_BINARY(TCCR1B));
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);
    myGLCD.print(str, LEFT, 25);

    sprintf(str, "TCCR2A:" BYTE_TO_BINARY_PATTERN " TCCR0B:" BYTE_TO_BINARY_PATTERN,
        BYTE_TO_BINARY(TCCR2A), BYTE_TO_BINARY(TCCR2B));
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);
    myGLCD.print(str, LEFT, 40);

    sprintf(str, "TCCR3A:" BYTE_TO_BINARY_PATTERN " TCCR3B:" BYTE_TO_BINARY_PATTERN,
        BYTE_TO_BINARY(TCCR3A), BYTE_TO_BINARY(TCCR3B));
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);
    myGLCD.print(str, LEFT, 55);

    sprintf(str, "TCCR4A:" BYTE_TO_BINARY_PATTERN " TCCR4B:" BYTE_TO_BINARY_PATTERN,
        BYTE_TO_BINARY(TCCR4A), BYTE_TO_BINARY(TCCR4B));
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);
    myGLCD.print(str, LEFT, 70);

    sprintf(str, "TCCR5A:" BYTE_TO_BINARY_PATTERN " TCCR5B:" BYTE_TO_BINARY_PATTERN,
        BYTE_TO_BINARY(TCCR5A), BYTE_TO_BINARY(TCCR5B));
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);
    myGLCD.print(str, LEFT, 85);

    int posy = 105;
    int posx = 0;
    for (int i = 1; i < 61; i++) {

        if (i % 7 == 0) {
            posy += 15;
            posx = 0;
        }

        dtostrf(temperatureHour[i - 1], 4, 1, str);
        myGLCD.print(str, posx, posy);
        posx += 35;

    }
}
#endif
