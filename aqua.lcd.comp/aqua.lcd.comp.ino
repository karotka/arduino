#include <Wire.h>
#include <UTFT.h>
#include <ITDB02_Touch.h>
#include <UTFT_Buttons.h>
#include <EEPROM.h>
#include "utils.h"
#include <RTClib.h>

UTFT          myGLCD(ITDB32S, 38, 39, 40, 41);
ITDB02_Touch  myTouch(6, 5, 4, 3, 2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

int x, y;
int xW, xR, xG, xB = 38;
int xWC, xRC, xGC, xBC = 42;
//uint32_t targetTime = 0;                    // for next 1 second timeout
int but1, but2, but3, but4, but5, but6, but7, but8, but9, but10, but11, but12, but13, but14, pressedButton;

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

//==== Defining Variables
extern uint8_t GroteskBold24x48[];
extern uint8_t SmallFont[];
extern uint8_t BigFont[];

void setup() {

    Wire.begin();
    RTC.begin();

    if (!RTC.isrunning()) {
        Serial.println("RTC is NOT running!");
        // following line sets the RTC to the date & time this sketch was compiled
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }

    Serial.begin(9600);

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
    //targetTime = millis() + 1000;  // 1 Sec Update
}

void loop() {
    now = RTC.now();

    switch (page) {
    case PAGE_HOME:
        drawHomeScreen();

        if (myTouch.dataAvailable()) {
            myGLCD.clrScr();
            myTouch.read();
            debug();
            page = PAGE_SELECT;
            drawSelectScreen();
        }
        debug();

        break;

    case PAGE_SELECT:
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

            if (pressedButton == 5) {
                myGLCD.clrScr();
                drawHomeScreen();
                page = PAGE_HOME;
            }
            debug();
        }

        break;

    case PAGE_SET_LIGHT:

        if (myTouch.dataAvailable() == true) {
            pressedButton = myButtons.checkButtons();
            x = myButtons.Touch->getX();
            y = myButtons.Touch->getY();

            setLedColor();

            if (pressedButton == 0) {
                // set light to on
            }
            if (pressedButton == 1) {
                // set light to off
            }
            if (pressedButton == 2) {
                // set light to night
            }

            if (pressedButton == 3) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawHomeScreen();
                page = PAGE_HOME;
            }
            debug();
        }

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
        debug();
        break;

    case PAGE_SET_TIMER:
        break;

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

    but1 = myButtons.addButton(60,   52, 25,  30, "<");
    but2 = myButtons.addButton(90,   52, 25,  30, ">");
    but3 = myButtons.addButton(133,  52, 25,  30, "<");
    but4 = myButtons.addButton(163,  52, 25,  30, ">");
    but5 = myButtons.addButton(205,  52, 25,  30, "<");
    but6 = myButtons.addButton(235,  52, 25,  30, ">");

    but7  = myButtons.addButton(35,  139, 25,  30, "<");
    but8  = myButtons.addButton(65,  139, 25,  30, ">");
    but9  = myButtons.addButton(110, 139, 25,  30, "<");
    but10 = myButtons.addButton(140, 139, 25,  30, ">");
    but11 = myButtons.addButton(205, 139, 25,  30, "<");
    but12 = myButtons.addButton(235, 139, 25,  30, ">");

    but13 = myButtons.addButton(20,  190, 90,  30, "Save");
    but14 = myButtons.addButton(220, 190, 90,  30, "Home");

    myButtons.drawButtons();
}

void debug() {
    char str[62];

    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);

    //x = myTouch.getX();
    //y = myTouch.getY();
    sprintf (str, "BTN: %d", pressedButton);

    //sprintf (str, "xW: %d, xR: %d, xG: %d, xB: %d", xW, xR, xG, xB);
    //sprintf (str, "x: %d, y: %d, page: %d, btn: %d, W: %d, R: %d, G: %d, B: %d",
    //         x, y, page, pressedButton, xWC, xRC, xGC, xBC);
    Serial.println(str);

    myGLCD.print(str, CENTER, 228);
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
    but1 = myButtons.addButton(10,   90, 90,  30, "LIGHT");
    but2 = myButtons.addButton(115,  90, 90,  30, "TIME");
    but3 = myButtons.addButton(220,  90, 90,  30, "TIMER");

    but4 = myButtons.addButton(10,  140, 90,  30, "CO2");
    but5 = myButtons.addButton(115, 140, 90,  30, "???");
    but6 = myButtons.addButton(220, 140, 90,  30, "Home");

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

    but1 = myButtons.addButton(10,  197, 50,  30, "ON");
    but2 = myButtons.addButton(70,  197, 60,  30, "OFF");
    but3 = myButtons.addButton(139, 197, 81,  30, "NIGHT");
    but4 = myButtons.addButton(240, 197, 70,  30, "HOME");

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

void eepromReadRGB() {
    xWC = EEPROM.read(1);
    xRC = EEPROM.read(2);
    xGC = EEPROM.read(3);
    xBC = EEPROM.read(4);

    xW = map(xWC, 0, 255, 42, 304);
    xR = map(xRC, 0, 255, 42, 304);
    xG = map(xGC, 0, 255, 42, 304);
    xB = map(xBC, 0, 255, 42, 304);
}

void setPwm() {
    analogWrite(whiteLed, xWC);
    analogWrite(redLed,   xRC);
    analogWrite(greenLed, xGC);
    analogWrite(blueLed,  xBC);
}