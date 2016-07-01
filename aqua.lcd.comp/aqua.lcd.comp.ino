#include <Wire.h>
#include <UTFT.h>
#include <ITDB02_Touch.h>
#include <UTFT_Buttons.h>
#include <EEPROM.h>
#include "utils.h"
#include <RTClib.h>

#define DEBUG 1

UTFT          myGLCD(ITDB32S, 38, 39, 40, 41);
ITDB02_Touch  myTouch(6, 5, 4, 3, 2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

//==== Defining Variables
int x, y;
int xW, xR, xG, xB = 38;
int xWC, xRC, xGC, xBC = 42;
int pressedButton;
uint32_t targetTime = 0;

int lightStatus[6];
int hourStatus[6];
int minuteStatus[6];

volatile unsigned int page;
const int whiteLed = 11, redLed = 10, greenLed = 9, blueLed  = 8;

RTC_DS1307 RTC;
DateTime now;
NewTime  newTime;

char strTime[9];
char strDate[10];

enum {
    PAGE_HOME = 0,
    PAGE_SELECT,
    PAGE_SET_LIGHT,
    PAGE_SET_TIME,
    PAGE_SET_TIMER
};

extern uint8_t GroteskBold24x48[];
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

void setup() {

    Wire.begin();
    RTC.begin();

    if (!RTC.isrunning()) {
        //Serial.println("RTC is NOT running!");
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }

    //Serial.begin(9600);

    eepromReadRGB();

    // Initial setup
    myGLCD.InitLCD(LANDSCAPE);
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0);

    myTouch.InitTouch(LANDSCAPE);
    myTouch.setPrecision(PREC_MEDIUM);

    myButtons.setTextFont(BigFont);

    page = PAGE_HOME;

    setPwm();

    drawHomeScreen();
}

void loop() {

    // automatick return to Homepage
    if (millis() > targetTime && page != PAGE_HOME) {
        myButtons.deleteAllButtons();
        myGLCD.clrScr();
        drawHomeScreen();
        page = PAGE_HOME;
    }

    switch (page) {

    case PAGE_SELECT:
        now = RTC.now();
        drawTime();

        if (myTouch.dataAvailable() == true) {
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawLightControl();
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
                page = PAGE_SET_TIMER;
                myGLCD.clrScr();
                drawTimerScreen();
            }

            if (pressedButton == 5) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                page = PAGE_HOME;
            }
        }
#ifdef DEBUG
        debug();
#endif
        break;

    case PAGE_SET_LIGHT:

        if (myTouch.dataAvailable() == true) {
            pressedButton = myButtons.checkButtons();
            x = myButtons.Touch->getX();
            y = myButtons.Touch->getY();

            setLedColor();

            if (pressedButton == 0) {
                // set light to on
                analogWrite(whiteLed, 255);
                analogWrite(redLed,   0);
                analogWrite(greenLed, 0);
                analogWrite(blueLed,  0);
            }
            if (pressedButton == 1) {
                // set light to off
                analogWrite(whiteLed, 0);
                analogWrite(redLed,   0);
                analogWrite(greenLed, 0);
                analogWrite(blueLed,  0);
            }
            if (pressedButton == 2) {
                // set light to night
                analogWrite(whiteLed, 0);
                analogWrite(redLed,   xRC);
                analogWrite(greenLed, xGC);
                analogWrite(blueLed,  xBC);
            }

            if (pressedButton == 3) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                page = PAGE_HOME;
            }
        }
#ifdef DEBUG
        debug();
#endif
        break;

    case PAGE_SET_TIME:

        if (myTouch.dataAvailable() == true) {
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
                page = PAGE_HOME;
            }
        }
#ifdef DEBUG
        debug();
#endif
        break;

    case PAGE_SET_TIMER:

        if (myTouch.dataAvailable() == true) {
            pressedButton = myButtons.checkButtons();

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
                EEPROM.write(5,  lightStatus[0]);
                EEPROM.write(6,  lightStatus[1]);
                EEPROM.write(7,  lightStatus[2]);
                EEPROM.write(8,  lightStatus[3]);
                EEPROM.write(9,  lightStatus[4]);
                EEPROM.write(10, lightStatus[5]);

                EEPROM.write(11, hourStatus[0]);
                EEPROM.write(12, hourStatus[1]);
                EEPROM.write(13, hourStatus[2]);
                EEPROM.write(14, hourStatus[3]);
                EEPROM.write(15, hourStatus[4]);
                EEPROM.write(16, hourStatus[5]);

                EEPROM.write(17, minuteStatus[0]);
                EEPROM.write(18, minuteStatus[1]);
                EEPROM.write(19, minuteStatus[2]);
                EEPROM.write(20, minuteStatus[3]);
                EEPROM.write(21, minuteStatus[4]);
                EEPROM.write(22, minuteStatus[5]);
            }

            // home button
            if (pressedButton == 31) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                page = PAGE_HOME;
            }
        }
#ifdef DEBUG
        debug();
#endif
        break;

    case PAGE_HOME:
        now = RTC.now();
        drawHomeScreen();

        if (myTouch.dataAvailable() == true) {
            myGLCD.clrScr();
            myTouch.read();
            page = PAGE_SELECT;
            drawSelectScreen();
            targetTime = millis() + 20000; // 20 Sec
        }

#ifdef DEBUG
        debug();
#endif
        break;

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

    myButtons.addButton(10,  192, 90,  30, "Save");
    myButtons.addButton(220, 192, 90,  30, "Home");

    myButtons.drawButtons();
}

void drawHomeScreen() {
    sprintf (strTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    myGLCD.setColor(162, 0, 0);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 40);

    sprintf (strDate, "%02d.%02d.%02d", now.day(), now.month(), now.year());
    myGLCD.setColor(150, 150, 150);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, CENTER, 95);
}

void drawSelectScreen() {
    myButtons.addButton(10,   90, 90,  30, "LIGHT");
    myButtons.addButton(115,  90, 90,  30, "TIME");
    myButtons.addButton(220,  90, 90,  30, "TIMER");

    myButtons.addButton(10,  140, 90,  30, "CO2");
    myButtons.addButton(115, 140, 90,  30, "???");
    myButtons.addButton(220, 140, 90,  30, "Home");

    myButtons.drawButtons();
}

void drawLightControl() {
    myGLCD.setFont(BigFont);
    myGLCD.print("Light controler", CENTER, 10);
    myGLCD.print("M:", 10,  44);
    myGLCD.print("R:", 10,  87);
    myGLCD.print("G:", 10, 115);
    myGLCD.print("B:", 10, 145);
    myGLCD.setColor(255, 0, 0);
    myGLCD.drawLine(0, 30, 319, 30);

    myGLCD.setColor(255, 255, 255);

    myGLCD.drawRect(40,  45, 310,  58); // Main  slider
    myGLCD.drawRect(40,  88, 310, 101); // Red   slider
    myGLCD.drawRect(40, 116, 310, 129); // Green slider
    myGLCD.drawRect(40, 146, 310, 159); // Blue  slider

    drawSliders();

    myButtons.addButton(10,  197, 50,  30, "ON");
    myButtons.addButton(70,  197, 60,  30, "OFF");
    myButtons.addButton(139, 197, 81,  30, "NIGHT");
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
    int oldXRC = xR;
    int oldXGC = xG;
    int oldXBC = xB;

    // Area of the Main color slider
    if( (y >= 42) && (y <= 58)) {
        xW = x; // Stores the X value where the screen has been pressed in to variable xR
        if (xW <= 42) { // Confines the area of the slider to be above 38 pixels
            xW = 42;
        }
        if (xW >= 304){ /// Confines the area of the slider to be under 310 pixels
            xW = 304;
        }
        xWC = map(xW, 42, 304, 0, 255);
        analogWrite(whiteLed, xWC);
    }

    // Area of the Red color slider
    if( (y >= 86) && (y <= 101)) {
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
    if( (y >= 114) && (y <= 129)) {
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
    if( (y >= 144) && (y <= 159)) {
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

    if (xW != oldXWC) {
        EEPROM.write(1, xWC);
    }
    if (xR != oldXRC) {
        EEPROM.write(2, xRC);
    }
    if (xG != oldXGC) {
        EEPROM.write(3, xGC);
    }
    if (xB != oldXBC) {
        EEPROM.write(4, xBC);
    }
}

void drawSliders() {
    // Draws the positioners White
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xW, 46, (xW + 4), 57); // Positioner

    myGLCD.setColor(xWC, xWC, xWC);
    myGLCD.fillRect(41, 46, (xW - 1),  57); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xW + 5), 46, 308, 57); // second rect

    // Draws the positioners Red
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xR, 89, (xR + 4), 100); // Positioner

    myGLCD.setColor(xRC, 0, 0);
    myGLCD.fillRect(41, 89, (xR - 1),  100); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xR + 5), 89, 308, 100); // second rect

    // Draws the positioners Green
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xG, 117, (xG + 4), 128); // Positioner

    myGLCD.setColor(0, xGC, 0);
    myGLCD.fillRect(41, 117, (xG - 1),  128); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xG + 5), 117, 308, 128); // second rect

    // Draws the positioners Blue
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(xB, 147, (xB + 4), 158); // Positioner

    myGLCD.setColor(0, 0, xBC);
    myGLCD.fillRect(41, 147, (xB - 1),  158); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((xB + 5), 147, 308, 158); // second rect
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

    myButtons.addButton(10,  192, 90,  30, "Save");
    myButtons.addButton(220, 192, 90,  30, "Home");

    setLightButton(4,  0);
    setLightButton(9,  1);
    setLightButton(14, 2);
    setLightButton(19, 3);
    setLightButton(24, 4);
    setLightButton(29, 5);

    myButtons.drawButtons();
}

void eepromReadRGB() {
    xWC = EEPROM.read(1);
    xRC = EEPROM.read(2);
    xGC = EEPROM.read(3);
    xBC = EEPROM.read(4);

    xW = map(xWC, 0, 255, 42, 304);
    xR = map(xRC, 0, 255, 42, 304);
    xG = map(xGC, 0, 255, 42, 304);
    xB = map(xBC, 0, 255, 42, 304);

    lightStatus[0] = EEPROM.read(5);
    lightStatus[1] = EEPROM.read(6);
    lightStatus[2] = EEPROM.read(7);
    lightStatus[3] = EEPROM.read(8);
    lightStatus[4] = EEPROM.read(9);
    lightStatus[5] = EEPROM.read(10);

    hourStatus[0] = EEPROM.read(11);
    hourStatus[1] = EEPROM.read(12);
    hourStatus[2] = EEPROM.read(13);
    hourStatus[3] = EEPROM.read(14);
    hourStatus[4] = EEPROM.read(15);
    hourStatus[5] = EEPROM.read(16);

    minuteStatus[0] = EEPROM.read(17);
    minuteStatus[1] = EEPROM.read(18);
    minuteStatus[2] = EEPROM.read(19);
    minuteStatus[3] = EEPROM.read(20);
    minuteStatus[4] = EEPROM.read(21);
    minuteStatus[5] = EEPROM.read(22);
}

void setPwm() {
    analogWrite(whiteLed, xWC);
    analogWrite(redLed,   xRC);
    analogWrite(greenLed, xGC);
    analogWrite(blueLed,  xBC);
}

#ifdef DEBUG
void debug() {
    char str[62];

    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);

    sprintf (str, "BTN: %d, page: %d, %d %d %d %d %d %d", pressedButton, page,
             lightStatus[0], lightStatus[1],lightStatus[2],
             lightStatus[3], lightStatus[4], lightStatus[5]);

    //sprintf (str, "xW: %d, xR: %d, xG: %d, xB: %d", xW, xR, xG, xB);
    //sprintf (str, "x: %d, y: %d, page: %d, btn: %d, W: %d, R: %d, G: %d, B: %d",
    //         x, y, page, pressedButton, xWC, xRC, xGC, xBC);
    Serial.println(str);

    myGLCD.print(str, CENTER, 228);
}
#endif
