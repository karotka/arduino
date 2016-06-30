#include <UTFT.h>
#include <ITDB02_Touch.h>
#include <UTFT_Buttons.h>
#include "utils.h"

UTFT          myGLCD(ITDB32S,38,39,40,41);
ITDB02_Touch  myTouch(6,5,4,3,2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

int x, y;
int xW, xR, xG, xB = 38;
int xWC, xRC, xGC, xBC = 42;
uint32_t targetTime = 0;                    // for next 1 second timeout
int but1, but2, but3, but4, but5, but6, but7, pressedButton;

uint8_t hh = conv2d(__TIME__),
        mm = conv2d(__TIME__+3), ss = conv2d(__TIME__+6);  // Get H, M, S from compile time

volatile unsigned int page;
const int whiteLed = 11, redLed = 10, greenLed = 9, blueLed  = 8;

char strTime[9];

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
    Serial.begin(9600);

    // Initial setup
    myGLCD.InitLCD(LANDSCAPE);
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0);

    myTouch.InitTouch(LANDSCAPE);
    myTouch.setPrecision(PREC_MEDIUM);

    myButtons.setTextFont(BigFont);

    page = PAGE_HOME;

    drawHomeScreen();
    targetTime = millis() + 1000;  // 1 Sec Update
}

void loop() {

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
        drawTime();

        if (myTouch.dataAvailable() == true) {
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                hh--;
                if (hh < 0) {
                    hh = 0;
                }
            }
            if (pressedButton == 1) {
                hh++;
                if (hh > 23) {
                    hh = 0;
                }
            }
            if (pressedButton == 2) {
                mm--;
                if (mm < 0) {
                    mm = 0;
                }
            }
            if (pressedButton == 3) {
                mm++;
                if (mm > 59) {
                    mm = 0;
                }
            }
            if (pressedButton == 4) {
                ss--;
                if (ss < 0) {
                    ss = 0;
                }
            }
            if (pressedButton == 5) {
                ss++;
                if (ss > 59) {
                    ss = 0;
                }
            }

            if (pressedButton == 6) {
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

    if (targetTime < millis()) {
        targetTime = millis() + 1000;
        ss++;              // Advance second
        if (ss == 60) {
            ss = 0;
            mm++;            // Advance minute
            if(mm > 59) {
                mm = 0;
                hh++;          // Advance hour
                if (hh > 23) {
                    hh = 0;
                }
            }
        }
    }
}

void drawTimeControl() {
    sprintf (strTime, "%02d:%02d:%02d", hh, mm, ss);
    myGLCD.setColor(200, 200, 200);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 20);

    but1 = myButtons.addButton(60,   90, 25,  30, "<");
    but2 = myButtons.addButton(90,   90, 25,  30, ">");
    but3 = myButtons.addButton(133,  90, 25,  30, "<");
    but4 = myButtons.addButton(163,  90, 25,  30, ">");
    but5 = myButtons.addButton(205,  90, 25,  30, "<");
    but6 = myButtons.addButton(235,  90, 25,  30, ">");

    but7 = myButtons.addButton(220, 140, 90,  30, "Home");

    myButtons.drawButtons();
}

void debug() {
    char str[62];

    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);

    //x = myTouch.getX();
    //y = myTouch.getY();

    sprintf (str, "x: %d, y: %d, page: %d, btn: %d, W: %d, R: %d, G: %d, B: %d",
             x, y, page, pressedButton, xWC, xRC, xGC, xBC);
    Serial.println(str);

    myGLCD.print(str, CENTER, 228);
}


void drawHomeScreen() {
    sprintf (strTime, "%02d:%02d:%02d", hh, mm, ss);

    myGLCD.setColor(162, 0, 0);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 80);
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

    but1 = myButtons.addButton(10,  197, 50,  30, "ON");
    but2 = myButtons.addButton(70,  197, 60,  30, "OFF");
    but3 = myButtons.addButton(139, 197, 80,  30, "NIGHT");
    but4 = myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();
}

void drawTime() {
    sprintf (strTime, "%02d:%02d:%02d", hh, mm, ss);
    myGLCD.setColor(200, 200, 200);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 20);
}

void setLedColor() {
    // Area of the Main color slider
    if( (y >= 42) && (y <= 58)) {
        xW = x; // Stores the X value where the screen has been pressed in to variable xR
        if (xW <= 42) { // Confines the area of the slider to be above 38 pixels
            xW = 42;
        }
        if (xW >= 304){ /// Confines the area of the slider to be under 310 pixels
            xW = 304;
        }
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
    }

    xWC = map(xW, 42, 304, 0, 255);
    xRC = map(xR, 42, 304, 0, 255);
    xGC = map(xG, 42, 304, 0, 255);
    xBC = map(xB, 42, 304, 0, 255);

    // Sends PWM signal to the pins of the led
    analogWrite(whiteLed, xWC);
    analogWrite(redLed,   xRC);
    analogWrite(greenLed, xGC);
    analogWrite(blueLed,  xBC);

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
