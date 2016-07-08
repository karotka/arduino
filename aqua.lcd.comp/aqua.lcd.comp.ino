#include <Wire.h>
#include <UTFT.h>
#include <ITDB02_Touch.h>
#include <UTFT_Buttons.h>
#include <EEPROM.h>
#include "utils.h"
#include <RTClib.h>

#define DEBUG 0
#define RETURN_DELAY 20000

UTFT          myGLCD(ITDB32S, 38, 39, 40, 41);
ITDB02_Touch  myTouch(6, 5, 4, 3, 2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

//==== Defining Variables
int x, y;

int xC, xW, xR, xG, xB = 38;
int xCC, xWC, xRC, xGC, xBC = 42;

uint8_t lightStatus[8];
uint8_t hourStatus[8];
uint8_t minuteStatus[8];

const char * dateStr[3] = {"OFF", "DAY", "NIGHT"};
const uint8_t whiteLed = 12, coolWhiteLed = 11, redLed = 10, greenLed = 9, blueLed  = 8;

volatile uint32_t targetTime = 0;
volatile uint8_t actualLightStatus;
volatile uint8_t page;
volatile uint8_t switchMode;
volatile uint8_t pressedButton;

RTC_DS1307 RTC;
DateTime now;
NewTime  newTime;

char strTemp[4];
char strTime[9];
char strDate[10];
char str[19];

enum {
    PAGE_HOME = 0,
    PAGE_SELECT,
    PAGE_SET_LIGHT,
    PAGE_SET_TIMER,
    PAGE_SET_TIME,
    PAGE_SET_CO2,
    PAGE_TEMP,
    PAGE_DEBUG
};

enum {
    MODE_AUTO = 0,
    MODE_MANUAL
};

extern uint8_t GroteskBold24x48[];
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

volatile int dateNow = 0;
volatile float tempNow = 24.5;
volatile uint8_t lightStatusNow = 3;

void setup() {

    Wire.begin();
    RTC.begin();

    if (!RTC.isrunning()) {
        //Serial.println("RTC is NOT running!");
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }

    Serial.begin(9600);
    Serial1.begin(9600);

    eepromRead();

    // Initial setup
    myGLCD.InitLCD(LANDSCAPE);
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0);

    myTouch.InitTouch(LANDSCAPE);
    myTouch.setPrecision(PREC_MEDIUM);

    myButtons.setTextFont(BigFont);

    page = PAGE_HOME;

    pwmInit();

    setPwm();
    pinInit();
    //timer2set();
    //drawHomeScreen();
}

void pinInit(void) {
    DDRK = 0xff;
    PORTK = 0x0;
}

void timer2set(void) {
    TIMSK2 |= (1 << TOIE2);  // Enable Overflow Interrupt Enable
    TCNT2 = 0;               // Initialize Counter
}

void loop() {

    if (switchMode == MODE_AUTO) {
        switchLight();
    }

    // automatick return to Homepage
    if (millis() > targetTime && page != PAGE_HOME) {
        myButtons.deleteAllButtons();
        myGLCD.clrScr();
        drawHomeScreen();
        lightStatusNow = 3;
        page = PAGE_HOME;
    }

    switch (page) {

    case PAGE_SELECT:
        now = RTC.now();
        drawTime();

        if (myTouch.dataAvailable() == true) {
            targetTime = millis() + RETURN_DELAY; // 20 Sec
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawLightControl();
                setMode();
                page = PAGE_SET_LIGHT;
            }

            if (pressedButton == 1) {
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

            if (pressedButton == 2) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawTimerScreen();
                page = PAGE_SET_TIMER;
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
                lightStatusNow = 3;
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
            targetTime = millis() + RETURN_DELAY; // 20 Sec
            pressedButton = myButtons.checkButtons();
            if (pressedButton == 0) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatusNow = 3;
                page = PAGE_HOME;
            }
        }
        break;
#endif

    case PAGE_SET_LIGHT:

        if (myTouch.dataAvailable() == true) {
            targetTime = millis() + RETURN_DELAY; // 20 Sec
            pressedButton = myButtons.checkButtons();

            x = myButtons.Touch->getX();
            y = myButtons.Touch->getY();

            setLedColor();

            if (pressedButton == 0) {
                // set light to day
                setDay();
            }
            if (pressedButton == 1) {
                // set light to off
                setOff();
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
                lightStatusNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_SET_TIME:

        if (myTouch.dataAvailable() == true) {
            targetTime = millis() + RETURN_DELAY; // 20 Sec
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
                lightStatusNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_TEMP:

        if (myTouch.dataAvailable() == true) {
            targetTime = millis() + RETURN_DELAY; // 20 Sec
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatusNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_SET_CO2:
        if (myTouch.dataAvailable() == true) {
            targetTime = millis() + RETURN_DELAY; // 20 Sec
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 1) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatusNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_SET_TIMER:

        if (myTouch.dataAvailable() == true) {
            pressedButton = myButtons.checkButtons();
            targetTime = millis() + RETURN_DELAY; // 20 Sec

            // first timer
            if (pressedButton == 0) {
                hourStatus[0]--;
                if (hourStatus[0] < 0) {
                    hourStatus[0] = 23;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 1) {
                hourStatus[0]++;
                if (hourStatus[0] > 23) {
                    hourStatus[0] = 0;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 2) {
                minuteStatus[0]--;
                if (minuteStatus[0] < 0) {
                    minuteStatus[0] = 59;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 3) {
                minuteStatus[0]++;
                if (minuteStatus[0] > 59) {
                    minuteStatus[0] = 0;
                }
                setTimerTime(0, 10, 8);
            }
            if (pressedButton == 4) {
                setLightType(4, 0);
            }

            // second time
            if (pressedButton == 5) {
                hourStatus[1]--;
                if (hourStatus[1] < 0) {
                    hourStatus[1] = 23;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 6) {
                hourStatus[1]++;
                if (hourStatus[1] > 23) {
                    hourStatus[1] = 0;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 7) {
                minuteStatus[1]--;
                if (minuteStatus[1] < 0) {
                    minuteStatus[1] = 59;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 8) {
                minuteStatus[1]++;
                if (minuteStatus[1] > 59) {
                    minuteStatus[1] = 0;
                }
                setTimerTime(1, 10, 64);
            }
            if (pressedButton == 9) {
                setLightType(9, 1);
            }

            // third time
            if (pressedButton == 10) {
                hourStatus[2]--;
                if (hourStatus[2] < 0) {
                    hourStatus[2] = 23;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 11) {
                hourStatus[2]++;
                if (hourStatus[2] > 23) {
                    hourStatus[2] = 0;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 12) {
                minuteStatus[2]--;
                if (minuteStatus[2] < 0) {
                    minuteStatus[2] = 59;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 13) {
                minuteStatus[2]++;
                if (minuteStatus[2] > 59) {
                    minuteStatus[2] = 0;
                }
                setTimerTime(2, 10, 122);
            }
            if (pressedButton == 14) {
                setLightType(14, 2);
            }

            if (pressedButton == 15) {
                hourStatus[3]--;
                if (hourStatus[3] < 0) {
                    hourStatus[3] = 23;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 16) {
                hourStatus[3]++;
                if (hourStatus[3] > 23) {
                    hourStatus[3] = 0;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 17) {
                minuteStatus[3]--;
                if (minuteStatus[3] < 0) {
                    minuteStatus[3] = 59;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 18) {
                minuteStatus[3]++;
                if (minuteStatus[3] > 59) {
                    minuteStatus[3] = 0;
                }
                setTimerTime(3, 176, 8);
            }
            if (pressedButton == 19) {
                setLightType(19, 3);
            }

            if (pressedButton == 20) {
                hourStatus[4]--;
                if (hourStatus[4] < 0) {
                    hourStatus[4] = 23;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 21) {
                hourStatus[4]++;
                if (hourStatus[4] > 23) {
                    hourStatus[4] = 0;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 22) {
                minuteStatus[4]--;
                if (minuteStatus[4] < 0) {
                    minuteStatus[4] = 59;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 23) {
                minuteStatus[4]++;
                if (minuteStatus[4] > 59) {
                    minuteStatus[4] = 0;
                }
                setTimerTime(4, 176, 64);
            }
            if (pressedButton == 24) {
                setLightType(24, 4);
            }

            if (pressedButton == 25) {
                hourStatus[5]--;
                if (hourStatus[5] < 0) {
                    hourStatus[5] = 23;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 26) {
                hourStatus[5]++;
                if (hourStatus[5] > 23) {
                    hourStatus[5] = 0;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 27) {
                minuteStatus[5]--;
                if (minuteStatus[5] < 0) {
                    minuteStatus[5] = 59;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 28) {
                minuteStatus[5]++;
                if (minuteStatus[5] > 59) {
                    minuteStatus[5] = 0;
                }
                setTimerTime(5, 176, 120);
            }
            if (pressedButton == 29) {
                setLightType(29, 5);
            }

            // save values
            if (pressedButton == 30) {
                EEPROM.write(6,  lightStatus[0]);
                EEPROM.write(7,  lightStatus[1]);
                EEPROM.write(8,  lightStatus[2]);
                EEPROM.write(9,  lightStatus[3]);
                EEPROM.write(10, lightStatus[4]);
                EEPROM.write(11, lightStatus[5]);
                lightStatus[6] = lightStatus[5];
                lightStatus[7] = lightStatus[5];

                EEPROM.write(12, hourStatus[0]);
                EEPROM.write(13, hourStatus[1]);
                EEPROM.write(14, hourStatus[2]);
                EEPROM.write(15, hourStatus[3]);
                EEPROM.write(16, hourStatus[4]);
                EEPROM.write(17, hourStatus[5]);

                EEPROM.write(18, minuteStatus[0]);
                EEPROM.write(19, minuteStatus[1]);
                EEPROM.write(20, minuteStatus[2]);
                EEPROM.write(21, minuteStatus[3]);
                EEPROM.write(22, minuteStatus[4]);
                EEPROM.write(23, minuteStatus[5]);
            }

            // home button
            if (pressedButton == 31) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                lightStatusNow = 3;
                page = PAGE_HOME;
            }
        }
        break;

    case PAGE_HOME:
        PORTK ^= (1 << PH7);
        now = RTC.now();
        drawHomeScreen();

        if (myTouch.dataAvailable() == true) {
            myGLCD.clrScr();
            myTouch.read();
            page = PAGE_SELECT;
            drawSelectScreen();
            targetTime = millis() + RETURN_DELAY; // 20 Sec
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
        int y1 = starty - tempData[i] * 10;
        int x2 = startx + (j * 4);
        int y2 = starty - tempData[j] * 10;

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

    myButtons.addButton(240, 197, 70,  30, "HOME");
    myButtons.drawButtons();
}

void drawCo2Screen() {

    myButtons.addButton(10,  197, 90,  30, "SAVE");
    myButtons.addButton(240, 197, 70,  30, "HOME");
    myButtons.drawButtons();
}

void setOff() {
    analogWrite(coolWhiteLed, 0);
    analogWrite(whiteLed, 0);
    analogWrite(redLed,   0);
    analogWrite(greenLed, 0);
    analogWrite(blueLed,  0);

}

void setDay() {
    analogWrite(coolWhiteLed, xCC);
    analogWrite(whiteLed, xWC);
    analogWrite(redLed,   0);
    analogWrite(greenLed, 0);
    analogWrite(blueLed,  0);
}

void setNight() {
    analogWrite(coolWhiteLed, 0);
    analogWrite(whiteLed, 0);
    analogWrite(redLed,   xRC);
    analogWrite(greenLed, xGC);
    analogWrite(blueLed,  xBC);
}

void setMode() {
    if (switchMode == MODE_AUTO) {
        myButtons.relabelButton(3, "AUT", true);
        myButtons.disableButton(0);
        myButtons.disableButton(1);
        myButtons.disableButton(2);
    }
    if (switchMode == MODE_MANUAL) {
        myButtons.relabelButton(3, "MAN", true);
        myButtons.enableButton(0);
        myButtons.enableButton(1);
        myButtons.enableButton(2);
    }
}

void setTimerTime(int id, int x, int y) {
    sprintf (strDate, "%02dh:%02dm.", hourStatus[id], minuteStatus[id]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, x, y);
}

void setLightType(int btn, int id) {
    lightStatus[id]++;
    if (lightStatus[id] > 2) {
        lightStatus[id] = 0;
    }
    setLightButton(btn, id);
}

void setLightButton(int btn, int id) {
    if (lightStatus[id] == 0) {
        myButtons.relabelButton(btn, "OF", true);
    } else
    if (lightStatus[id] == 1) {
        myButtons.relabelButton(btn, "DA", true);
    } else
    if (lightStatus[id] == 2) {
        myButtons.relabelButton(btn, "NI", true);
    } else {
        myButtons.relabelButton(btn, "NO", true);
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

    myButtons.addButton(10,  197, 90,  30, "SAVE");
    myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();
}

void drawHomeScreen() {
    sprintf (strTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    myGLCD.setColor(162, 0, 0);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 30);

    int d = now.day() + now.month() + now.year();
    if (dateNow != d) {
        sprintf (strDate, "%02d.%02d.%02d", now.day(), now.month(), now.year());
        myGLCD.setColor(150, 150, 150);
        myGLCD.setFont(BigFont);
        myGLCD.print(strDate, CENTER, 85);
    }

    int temp = 24.5;
    if (tempNow != temp) {
        dtostrf(tempNow, 4, 1, strTemp);
        myGLCD.setColor(7, 56, 212);
        myGLCD.setFont(GroteskBold24x48);
        myGLCD.print(strTemp, 100, 125);
        myGLCD.print("C", 205, 125);
    }

    if (actualLightStatus != lightStatusNow) {
        myGLCD.print("                   ", CENTER, 185);
        if (switchMode == MODE_MANUAL) {
            sprintf (str, "MODE:MANUAL (%s)", dateStr[actualLightStatus]);
        } else {
            sprintf (str, "MODE:AUTO (%s)", dateStr[actualLightStatus]);
        }
        myGLCD.setColor(90, 90, 90);
        myGLCD.setFont(BigFont);
        myGLCD.print(str, CENTER, 185);
        lightStatusNow = actualLightStatus;
    }
}

void drawSelectScreen() {
    myButtons.addButton(10,   90, 90,  30, "LIGHT");
    myButtons.addButton(115,  90, 90,  30, "TIME");
    myButtons.addButton(220,  90, 90,  30, "TIMER");

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

    drawSliders();

    myButtons.addButton(2,   197, 45,  30, "ON");
    myButtons.addButton(52,  197, 60,  30, "OFF");
    myButtons.addButton(117, 197, 42,  30, "NI");
    myButtons.addButton(163, 197, 50,  30, "AUT");
    myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();
}

void drawTime() {
    sprintf (strTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    myGLCD.setColor(200, 200, 200);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 20);
}

void setLedColor() {
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
        analogWrite(coolWhiteLed, xCC);
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
        analogWrite(whiteLed, xWC);
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
        xRC = map(xR, 42, 304, 0, 255);
        analogWrite(redLed,   xRC);
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
        xGC = map(xG, 42, 304, 0, 255);
        analogWrite(greenLed, xGC);
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
        xBC = map(xB, 42, 304, 0, 255);
        analogWrite(blueLed,  xBC);
    }

    drawSliders();

    if (xC != oldXWC) {
        EEPROM.write(1, xCC);
    }
    if (xW != oldXWC) {
        EEPROM.write(2, xWC);
    }
    if (xR != oldXRC) {
        EEPROM.write(3, xRC);
    }
    if (xG != oldXGC) {
        EEPROM.write(4, xGC);
    }
    if (xB != oldXBC) {
        EEPROM.write(5, xBC);
    }
}

void drawSliders() {
    // Draws the positioners Cool White
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xC, 42, (xC + 4), 53); // Positioner

    myGLCD.setColor(255 - xCC, 255 - xCC, 255 - xCC);
    myGLCD.fillRect(41, 42, (xC - 1),  53); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xC + 5), 42, 308, 53); // second rect

    // Draws the positioners Warm White
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xW, 72, (xW + 4), 83); // Positioner

    myGLCD.setColor(255 - xWC, 255 - xWC, 255 - xWC);
    myGLCD.fillRect(41, 72, (xW - 1),  83); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xW + 5), 72, 308, 83); // second rect

    // Draws the positioners Red
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xR, 107, (xR + 4), 118); // Positioner

    myGLCD.setColor(xRC, 0, 0);
    myGLCD.fillRect(41, 107, (xR - 1),  118); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xR + 5), 107, 308, 118); // second rect

    // Draws the positioners Green
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xG, 137, (xG + 4), 148); // Positioner

    myGLCD.setColor(0, xGC, 0);
    myGLCD.fillRect(41, 137, (xG - 1),  148); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xG + 5), 137, 308, 148); // second rect

    // Draws the positioners Blue
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xB, 167, (xB + 4), 178); // Positioner

    myGLCD.setColor(0, 0, xBC);
    myGLCD.fillRect(41, 167, (xB - 1),  178); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xB + 5), 167, 308, 178); // second rect
}

void drawTimerScreen() {

    sprintf (strDate, "%02dh:%02dm.", hourStatus[0], minuteStatus[0]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 8);

    myButtons.addButton(2,   28, 22,  30, "<");
    myButtons.addButton(28,  28, 22,  30, ">");
    myButtons.addButton(62,  28, 22,  30, "<");
    myButtons.addButton(88,  28, 22,  30, ">");
    myButtons.addButton(116, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStatus[1], minuteStatus[1]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 64);

    myButtons.addButton(2,   85, 22,  30, "<");
    myButtons.addButton(28,  85, 22,  30, ">");
    myButtons.addButton(62,  85, 22,  30, "<");
    myButtons.addButton(88,  85, 22,  30, ">");
    myButtons.addButton(116, 85, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStatus[2], minuteStatus[2]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 122);

    myButtons.addButton(2,   142, 22,  30, "<");
    myButtons.addButton(28,  142, 22,  30, ">");
    myButtons.addButton(62,  142, 22,  30, "<");
    myButtons.addButton(88,  142, 22,  30, ">");
    myButtons.addButton(116, 142, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStatus[3], minuteStatus[3]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 8);

    myButtons.addButton(166, 28, 22,  30, "<");
    myButtons.addButton(192, 28, 22,  30, ">");
    myButtons.addButton(226, 28, 22,  30, "<");
    myButtons.addButton(252, 28, 22,  30, ">");
    myButtons.addButton(280, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStatus[4], minuteStatus[4]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 64);

    myButtons.addButton(166, 85, 22,  30, "<");
    myButtons.addButton(192, 85, 22,  30, ">");
    myButtons.addButton(226, 85, 22,  30, "<");
    myButtons.addButton(252, 85, 22,  30, ">");
    myButtons.addButton(280, 85, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStatus[5], minuteStatus[5]);
    myGLCD.setColor(225, 225, 225);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 120);

    myButtons.addButton(166, 142, 22,  30, "<");
    myButtons.addButton(192, 142, 22,  30, ">");
    myButtons.addButton(226, 142, 22,  30, "<");
    myButtons.addButton(252, 142, 22,  30, ">");
    myButtons.addButton(280, 142, 34,  30, "");

    myButtons.addButton(10,  197, 90,  30, "SAVE");
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

    xC = map(xCC, 0, 255, 42, 304);
    xW = map(xWC, 0, 255, 42, 304);
    xR = map(xRC, 0, 255, 42, 304);
    xG = map(xGC, 0, 255, 42, 304);
    xB = map(xBC, 0, 255, 42, 304);

    lightStatus[0] = EEPROM.read(6);
    lightStatus[1] = EEPROM.read(7);
    lightStatus[2] = EEPROM.read(8);
    lightStatus[3] = EEPROM.read(9);
    lightStatus[4] = EEPROM.read(10);
    lightStatus[5] = EEPROM.read(11);
    lightStatus[6] = lightStatus[5];
    lightStatus[7] = lightStatus[5];

    hourStatus[0] = EEPROM.read(12);
    hourStatus[1] = EEPROM.read(13);
    hourStatus[2] = EEPROM.read(14);
    hourStatus[3] = EEPROM.read(15);
    hourStatus[4] = EEPROM.read(16);
    hourStatus[5] = EEPROM.read(17);
    hourStatus[6] = 23;
    hourStatus[7] = 0;

    minuteStatus[0] = EEPROM.read(18);
    minuteStatus[1] = EEPROM.read(19);
    minuteStatus[2] = EEPROM.read(20);
    minuteStatus[3] = EEPROM.read(21);
    minuteStatus[4] = EEPROM.read(22);
    minuteStatus[5] = EEPROM.read(23);
    minuteStatus[6] = 59;
    minuteStatus[7] = 0;

    switchMode = EEPROM.read(24);
}

void switchLight() {
    int minutes, nextMinutes, realMinute;
    realMinute = now.minute() + (now.hour() * 60);

    //myGLCD.setColor(225, 225, 225);
    //myGLCD.setFont(SmallFont);

    for(uint8_t i = 0; i < 8; i++) {
        uint8_t j = i + 1;
        if (i == 7) { j = 0; }

        minutes     = minuteStatus[i] + (hourStatus[i] * 60);
        nextMinutes = minuteStatus[j] + (hourStatus[j] * 60);

        if (minutes <= realMinute && realMinute < nextMinutes) {
            //myGLCD.printNumI(minutes, 0, 10);
            //myGLCD.printNumI(nextMinutes, 30, 10);
            //myGLCD.printNumI(realMinutes, 60, 10);
            //myGLCD.printNumI(i, 90, 10);
            setLight(i);
            break;
        }
    }
}

void setLight(int i) {
    //myGLCD.printNumI(i, 0, 60);
    //myGLCD.printNumI(lightStatus[i], 30, 60);

    actualLightStatus = lightStatus[i];
    if (actualLightStatus == 0) {
        // set off
        setOff();
        //myGLCD.print("OFF", 0, 70);
    }
    if (actualLightStatus == 1) {
        // set day
        setDay();
        //myGLCD.print("DAY", 0, 70);
    }
    if (actualLightStatus == 2) {
        // set night
        setNight();
        //myGLCD.print("NIGHT", 0, 70);
    }
}

void setPwm() {
    analogWrite(coolWhiteLed, xCC);
    analogWrite(whiteLed, xWC);
    analogWrite(redLed,   xRC);
    analogWrite(greenLed, xGC);
    analogWrite(blueLed,  xBC);
}

void pwmInit(void) {
    // invert PWM
    //TCCR1A |=  (1 << COM1A1) | (1 << COM1A0);
    //TCCR1B |=  (1 << COM1B1) | (1 << COM1B0);
    //TCCR1B = (1 << CS11);   // clock source = CLK / 64, start PWM
}

#if DEBUG == 1
void debug() {
    char str[62];

    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);

    //sprintf (str, "BTN:%02d, PG:%d, R:%03d G:%03d B:%03d W:%03d",
    //         pressedButton, page, xRC, xGC, xBC, xWC);
    sprintf (str, "BTN:%02d, PG:%d",
             pressedButton, page);

    //sprintf (str, "BTN: %d, page: %d, %d %d %d %d %d %d", pressedButton, page,
    //         lightStatus[0], lightStatus[1],lightStatus[2],
    //         lightStatus[3], lightStatus[4], lightStatus[5]);

    //sprintf (str, "xW: %d, xR: %d, xG: %d, xB: %d", xW, xR, xG, xB);
    //sprintf (str, "x: %d, y: %d, page: %d, btn: %d, W: %d, R: %d, G: %d, B: %d",
    //         x, y, page, pressedButton, xWC, xRC, xGC, xBC);
    //Serial.println(str);

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
}
#endif
