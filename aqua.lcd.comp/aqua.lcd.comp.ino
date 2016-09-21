/**
 * Graphical interface for Arduino Aquarium Computer
 * http://www.sainsmart.com/sainsmart-mega2560-board-3-5-tft-lcd-module-display-shield-kit-for-atmel-atmega-avr-16au-atmega8u2.html
 * 3.2" 320x240 pixel SSD1289
 *
 */
#include "configure.h"
#include "lightvalues.h"
#include <Wire.h>
#include <UTFT.h>
#include <UTFT_Buttons.h>
#include <ITDB02_Touch.h>
#include <Thermistor.h>
#include <RTClib.h>
#include <EEPROM.h>
#include "utils.h"

RTC_DS1307 RTC;
DateTime now;
NewTime  newTime;

LigthValues_t offValues(MODE_OFF);
LigthValues_t dayValues(MODE_DAY);
LigthValues_t nightValues(MODE_NIGHT);
LigthValues_t *actualLightValues;

// temperature of board
Thermistor t0(A0, 0, 100000, 3950);
// main temp.
Thermistor t1(A1, 0, 10000,  3380);
// other
Thermistor t2(A2, 0, 10000,  3380);
Thermistor t3(A3, 0, 10000,  3380);

float te0, te1, te2, te3;

volatile int x, y;

uint8_t lightStates[8];
uint8_t hourStates[8];
uint8_t minuteStates[8];

uint8_t co2states[6];
uint8_t co2hour[6];
uint8_t co2minute[6];

uint8_t temperatureSenzorStatus[4];

volatile uint8_t page;
volatile uint8_t actualCo2state;
volatile uint8_t switchMode;
volatile uint8_t pressedButton;
volatile uint8_t i2cReadTimeCounter;
volatile uint8_t showTemp;

volatile int timerCounter1 = 0;
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

int dimmingSpeed = 30;

void pinInit(void) {
    DDRK = 0xff;
    PORTK = 0x0;
}

void timer2set(void) {
    TIMSK2 |= (1 << TOIE2);  // Enable Overflow Interrupt Enable
    TCNT2 = 0;               // Initialize Counter
}

void switchLight(int i) {
    if (actualLightValues->flag != lightStates[i]) {

        if (lightStates[i] == MODE_OFF) {
            actualLightValues = &offValues;
        }
        if (lightStates[i] == MODE_DAY) {
            actualLightValues = &dayValues;
        }
        if (lightStates[i] == MODE_NIGHT) {
            actualLightValues = &nightValues;
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


ISR(TIMER2_OVF_vect) {
    timerCounter1++;
    timerCounter2++;
    i2cReadTimeCounter++;
    temperatureReadTimeCounter++;

    if (switchMode == MODE_AUTO) {
        checkTimer();
        checkCo2();
    }

    //switchMode == MODE_AUTO &&
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
        if (page != PAGE_HOME) {
            page = PAGE_RETURN_MOME;
        }

        showTemp++;
        if (showTemp == TEMP_NONE) {
            showTemp = TEMP_0;
        }
    }
}

void setTempButton(uint8_t id) {
    sprintf (str, "TE%01d", id);

    if (temperatureSenzorStatus[id] > 1) {
        temperatureSenzorStatus[id] = 0;
    }
    if (temperatureSenzorStatus[id] == 0) {
        myButtons.relabelButton(id - 2, str, true, VGA_GRAY);
    } else {
        myButtons.relabelButton(id - 2, str, true);
    }
}

void drawTempScreen() {
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawHLine(0, 180, 319);
    myGLCD.drawLine(20, 205, 20, 0);
    myGLCD.setFont(SmallFont);

    myButtons.addButton(2,   197, 50,  30, "", 0, VGA_GRAY);
    myButtons.addButton(55,  197, 50,  30, "", 0, VGA_GRAY);
    //myButtons.addButton(108, 197, 50,  30, "", 0, VGA_GRAY);

    myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();

    for (uint8_t i = 2; i < 4; i++) {
        setTempButton(i);
    }
}

void setCo2Button(int btn, int id) {
    if (co2states[id] == CO2_OFF) {
        myButtons.relabelButton(btn, "OF", true);
    } else
    if (co2states[id] == CO2_ON) {
        myButtons.relabelButton(btn, "ON", true);
    }
}

void drawCo2Screen() {
    sprintf (strDate, "%02dh:%02dm.", co2hour[0], co2minute[0]);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 8);

    myButtons.addButton(2,   28, 22,  30, "<");
    myButtons.addButton(28,  28, 22,  30, ">");
    myButtons.addButton(62,  28, 22,  30, "<");
    myButtons.addButton(88,  28, 22,  30, ">");
    myButtons.addButton(116, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", co2hour[1], co2minute[1]);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 64);

    myButtons.addButton(2,   85, 22,  30, "<");
    myButtons.addButton(28,  85, 22,  30, ">");
    myButtons.addButton(62,  85, 22,  30, "<");
    myButtons.addButton(88,  85, 22,  30, ">");
    myButtons.addButton(116, 85, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", co2hour[2], co2minute[2]);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 8);

    myButtons.addButton(166, 28, 22,  30, "<");
    myButtons.addButton(192, 28, 22,  30, ">");
    myButtons.addButton(226, 28, 22,  30, "<");
    myButtons.addButton(252, 28, 22,  30, ">");
    myButtons.addButton(280, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", co2hour[3], co2minute[3]);
    myGLCD.setColor(VGA_WHITE);
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

void analogSwitch() {
    analogWrite(LED_COOL_WHITE, actualLightValues->coolByte);
    analogWrite(LED_WHITE,      actualLightValues->warmByte);
    analogWrite(LED_YELLOW,     actualLightValues->yellowByte);
    analogWrite(LED_RED,        actualLightValues->redByte);
    analogWrite(LED_GREEN,      actualLightValues->greenByte);
    analogWrite(LED_BLUE,       actualLightValues->blueByte);
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
        myButtons.relabelButton(0, "DAY", true, actualLightValues->flag == MODE_DAY ? VGA_GREEN : VGA_BLUE);

        myButtons.enableButton(1, true);
        myButtons.relabelButton(1, "OFF", true, actualLightValues->flag == MODE_OFF ? VGA_GREEN : VGA_BLUE);

        myButtons.enableButton(2, true);
        myButtons.relabelButton(2, "NI", true, actualLightValues->flag == MODE_NIGHT ? VGA_GREEN : VGA_BLUE);
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

void setLightType(int btn, int id) {
    lightStates[id]++;
    if (lightStates[id] > 2) {
        lightStates[id] = 0;
    }
    setLightButton(btn, id);
}

void drawNewTime() {
    myGLCD.setColor(255, 255, 255);
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
    myGLCD.setColor(COLOR_TIME);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 15);

    sumDateNow = now.day() + now.month() + now.year();
    if (dateNow != sumDateNow) {
        sprintf (strDate, "%02d.%02d.%02d", now.day(), now.month(), now.year());
        myGLCD.setColor(225, 225, 225);
        myGLCD.setFont(BigFont);
        myGLCD.print(strDate, CENTER, 65);
    }

    dtostrf(te1, 2, 1, strTemp);
    myGLCD.setColor(VGA_AQUA);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTemp, 100, 95);
    myGLCD.print("C", 205, 95);

    myGLCD.setColor(210, 210, 210);
    myGLCD.setFont(BigFont);

    myGLCD.print("MODE:", 25, 150);
    if (switchMode == MODE_MANUAL) {
        sprintf (str, "MANUAL (%s)", dateStr[actualLightValues->flag]);
    } else {
        sprintf (str, "AUTO (%s)", dateStr[actualLightValues->flag]);
    }
    myGLCD.print(str, 105, 150);

    if (actualCo2state == CO2_OFF) {
        sprintf (str, "CO2:%s", co2Str[CO2_OFF]);
    } else {
        sprintf (str, "CO2:%s", co2Str[CO2_ON]);
    }

    if (t0.isEnabled()) {
        dtostrf (te0, 2, 1, strTemp);
        sprintf (str, "%s TB:%sC ", str, strTemp);
    } else {
        sprintf (str, "%s TB:--   ", str);
    }
    myGLCD.print(str, CENTER, 170);

    if (t2.isEnabled()) {
        dtostrf (te2, 2, 1, strTemp);
        sprintf (str, "T1:%sC", strTemp);
    } else {
        sprintf (str, "T1:--   ");
    }
    myGLCD.print(str, 30, 190);

    if (t3.isEnabled()) {
        dtostrf (te3, 2, 1, strTemp);
        sprintf (str, "T2:%sC ", strTemp);
    } else {
        sprintf (str, "T2:--   ");
    }
    myGLCD.print(str, 175, 190);
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

void drawSlidersCC() {
    // Draws the positioners Cool White
    int s = 37;
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(actualLightValues->coolValue, s, (actualLightValues->coolValue + 4), s + 11); // Positioner

    myGLCD.setColor(255, 255, 255);
    myGLCD.fillRect(41, s, (actualLightValues->coolValue - 1),  s + 11); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((actualLightValues->coolValue + 5), s, 308, s + 11); // second rect
}

void drawSlidersWC() {
    // Draws the positioners Warm White
    int s = 64;
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(actualLightValues->warmValue, s, (actualLightValues->warmValue + 4), s + 11); // Positioner

    myGLCD.setColor(255, 253, 201);
    myGLCD.fillRect(41, s, (actualLightValues->warmValue - 1),  s + 11); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((actualLightValues->warmValue + 5), s, 308, s + 11); // second rect
}

void drawSlidersYC() {
    // Draws the positioners Yellow
    int s = 91;
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(actualLightValues->yellowValue, s, (actualLightValues->yellowValue + 4), s + 11); // Positioner

    myGLCD.setColor(255, 246, 0);
    myGLCD.fillRect(41, s, (actualLightValues->yellowValue - 1),  s + 11); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((actualLightValues->yellowValue + 5), s, 308, s + 11); // second rect
}

void drawSlidersRC() {
    // Draws the positioners Red
    int s = 118;
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(actualLightValues->redValue, s, (actualLightValues->redValue + 4), s + 11); // Positioner

    myGLCD.setColor(255, 0, 0);
    myGLCD.fillRect(41, s, (actualLightValues->redValue - 1),  s + 11); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((actualLightValues->redValue + 5), s, 308, s + 11); // second rect
}

void drawSlidersGC() {
    // Draws the positioners Green
    int s = 145;
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(actualLightValues->greenValue, s, (actualLightValues->greenValue + 4), s + 11); // Positioner

    myGLCD.setColor(0, 255, 0);
    myGLCD.fillRect(41, s, (actualLightValues->greenValue - 1),  s + 11); // first rect

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((actualLightValues->greenValue + 5), s, 308, s + 11); // second rect
}

void drawSlidersBC() {
    // Draws the positioners Blue
    int s = 172;
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(actualLightValues->blueValue, s, (actualLightValues->blueValue + 4), s + 11); // Positioner

    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRect(41, s, (actualLightValues->blueValue - 1),  s + 11); // first

    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect((actualLightValues->blueValue + 5), s, 308, s + 11); // second rect
}

void redrawSliders() {
    drawSlidersCC();
    drawSlidersWC();
    drawSlidersYC();
    drawSlidersRC();
    drawSlidersGC();
    drawSlidersBC();
}

void drawLightControl() {
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setFont(BigFont);
    myGLCD.print("Light controler", CENTER, 6);
    myGLCD.print("C:", 10,  34);
    myGLCD.print("W:", 10,  61);
    myGLCD.print("Y:", 10,  88);
    myGLCD.print("R:", 10, 115);
    myGLCD.print("G:", 10, 142);
    myGLCD.print("B:", 10, 169);
    myGLCD.setColor(255, 0, 0);
    myGLCD.drawLine(0, 25, 319, 25);

    myGLCD.setColor(VGA_WHITE);
    myGLCD.drawRect(40,  36, 310,  49); // Cool   slider
    myGLCD.drawRect(40,  63, 310,  76); // Warm   slider
    myGLCD.drawRect(40,  90, 310, 103); // Yellow slider
    myGLCD.drawRect(40, 117, 310, 130); // Red   slider
    myGLCD.drawRect(40, 144, 310, 157); // Green slider
    myGLCD.drawRect(40, 171, 310, 184); // Blue  slider

    redrawSliders();

    myButtons.addButton(2,   197, 50,  30, "DAY");
    myButtons.addButton(55,  197, 60,  30, "OFF");
    myButtons.addButton(120, 197, 42,  30, "NI");
    myButtons.addButton(169, 197, 50,  30, "AUT");
    myButtons.addButton(240, 197, 70,  30, "HOME");

    myButtons.drawButtons();
}

void drawTime() {
    sprintf (strTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    myGLCD.setColor(COLOR_TIME);
    myGLCD.setFont(GroteskBold24x48);
    myGLCD.print(strTime, CENTER, 20);
}

void checkTouchLedArea() {
    if (x > X_TOUCH_AREA_MAX) {
        x = X_TOUCH_AREA_MAX;
    }
    if (x < X_TOUCH_AREA_MIN) {
        x = X_TOUCH_AREA_MIN;
    }

    // Area of the Cool color slider
    if( (y >= 36) && (y <= 48)) {
        actualLightValues->setCoolValue(x);
        drawSlidersCC();
    } else

    // Area of the Warm color slider
    if( (y >= 63) && (y <= 75)) {
        actualLightValues->setWarmValue(x);
        drawSlidersWC();
    } else

    // Area of the Yellow color slider
    if( (y >= 90) && (y <= 102)) {
        actualLightValues->setYellowValue(x);
        drawSlidersYC();
    } else

    // Area of the Red color slider
    if( (y >= 117) && (y <= 129)) {
        actualLightValues->setRedValue(x);
        drawSlidersRC();
    } else

    // Area of the Green color slider
    if( (y >= 144) && (y <= 156)) {
        actualLightValues->setGreenValue(x);
        drawSlidersGC();
    } else

    // Area of the Blue color slider
    if( (y >= 171) && (y <= 183)) {
        actualLightValues->setBlueValue(x);
        drawSlidersBC();
    }
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
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 64);

    myButtons.addButton(2,   85, 22,  30, "<");
    myButtons.addButton(28,  85, 22,  30, ">");
    myButtons.addButton(62,  85, 22,  30, "<");
    myButtons.addButton(88,  85, 22,  30, ">");
    myButtons.addButton(116, 85, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[2], minuteStates[2]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 10, 122);

    myButtons.addButton(2,   142, 22,  30, "<");
    myButtons.addButton(28,  142, 22,  30, ">");
    myButtons.addButton(62,  142, 22,  30, "<");
    myButtons.addButton(88,  142, 22,  30, ">");
    myButtons.addButton(116, 142, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[3], minuteStates[3]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 8);

    myButtons.addButton(166, 28, 22,  30, "<");
    myButtons.addButton(192, 28, 22,  30, ">");
    myButtons.addButton(226, 28, 22,  30, "<");
    myButtons.addButton(252, 28, 22,  30, ">");
    myButtons.addButton(280, 28, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[4], minuteStates[4]);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(BigFont);
    myGLCD.print(strDate, 176, 64);

    myButtons.addButton(166, 85, 22,  30, "<");
    myButtons.addButton(192, 85, 22,  30, ">");
    myButtons.addButton(226, 85, 22,  30, "<");
    myButtons.addButton(252, 85, 22,  30, ">");
    myButtons.addButton(280, 85, 34,  30, "");

    sprintf (strDate, "%02dh:%02dm.", hourStates[5], minuteStates[5]);
    myGLCD.setColor(255, 255, 255);
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

    temperatureSenzorStatus[0] = 1;//EEPROM.read(42);
    temperatureSenzorStatus[1] = 1;//EEPROM.read(43);
    temperatureSenzorStatus[2] = EEPROM.read(44);
    temperatureSenzorStatus[3] = EEPROM.read(45);

    // from 64 to 90 LigthValues_t
}

void defaultLights() {
    analogWrite(LED_COOL_WHITE, actualLightValues->coolByte);
    analogWrite(LED_WHITE,      actualLightValues->warmByte);
    analogWrite(LED_YELLOW,     actualLightValues->yellowByte);
    analogWrite(LED_RED,        actualLightValues->redByte);
    analogWrite(LED_GREEN,      actualLightValues->greenByte);
    analogWrite(LED_BLUE,       actualLightValues->blueByte);
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

    sprintf (str, "Y:%03d W:%03d C:%03d R:%03d G:%03d B:%03d",
             OCR0A, OCR1B, OCR1A, OCR2A, OCR2B, OCR4C);
    myGLCD.print(str, CENTER, 215);

    sprintf (str, "Y:%03d W:%03d C:%03d R:%03d G:%03d B:%03d",
             actualLightValues->yellowByte, actualLightValues->warmByte,
             actualLightValues->coolByte, actualLightValues->redByte,
             actualLightValues->greenByte, actualLightValues->blueByte);
    myGLCD.print(str, CENTER, 228);

    sprintf (str, "A0:%04d A1:%04d A2:%04d A3:%04d",
             t0.getAdc(), t1.getAdc(), t2.getAdc(), t3.getAdc());
    myGLCD.print(str, CENTER, 4);

    //sprintf (str, "A1:%d A2:%d A3:%d", t1.isEnabled(), t2.isEnabled(), t3.isEnabled());
    //myGLCD.print(str, CENTER, 228);

    //sprintf (str, "B: %d NO: %d ADC: %d",
    //         t1.getBcoefficient(), t1.getThermistornominal(), t1.getAdc()
    //         );
    //myGLCD.print(str, CENTER, 228);
}

void drawRegDebug() {
    //sprintf(str, "TCCR0A:" BYTE_TO_BINARY_PATTERN " TCCR0B:" BYTE_TO_BINARY_PATTERN,
    //    BYTE_TO_BINARY(TCCR0A), BYTE_TO_BINARY(TCCR0B));
    //myGLCD.setColor(255, 255, 255);
    //myGLCD.setFont(SmallFont);
    //myGLCD.print(str, LEFT, 10);
    //
    //sprintf(str, "TCCR1A:" BYTE_TO_BINARY_PATTERN " TCCR1B:" BYTE_TO_BINARY_PATTERN,
    //    BYTE_TO_BINARY(TCCR1A), BYTE_TO_BINARY(TCCR1B));
    //myGLCD.setColor(255, 255, 255);
    //myGLCD.setFont(SmallFont);
    //myGLCD.print(str, LEFT, 25);
    //
    //sprintf(str, "TCCR2A:" BYTE_TO_BINARY_PATTERN " TCCR0B:" BYTE_TO_BINARY_PATTERN,
    //    BYTE_TO_BINARY(TCCR2A), BYTE_TO_BINARY(TCCR2B));
    //myGLCD.setColor(255, 255, 255);
    //myGLCD.setFont(SmallFont);
    //myGLCD.print(str, LEFT, 40);
    //
    //sprintf(str, "TCCR3A:" BYTE_TO_BINARY_PATTERN " TCCR3B:" BYTE_TO_BINARY_PATTERN,
    //    BYTE_TO_BINARY(TCCR3A), BYTE_TO_BINARY(TCCR3B));
    //myGLCD.setColor(255, 255, 255);
    //myGLCD.setFont(SmallFont);
    //myGLCD.print(str, LEFT, 55);
    //
    //sprintf(str, "TCCR4A:" BYTE_TO_BINARY_PATTERN " TCCR4B:" BYTE_TO_BINARY_PATTERN,
    //    BYTE_TO_BINARY(TCCR4A), BYTE_TO_BINARY(TCCR4B));
    //myGLCD.setColor(255, 255, 255);
    //myGLCD.setFont(SmallFont);
    //myGLCD.print(str, LEFT, 70);
    //
    //sprintf(str, "TCCR5A:" BYTE_TO_BINARY_PATTERN " TCCR5B:" BYTE_TO_BINARY_PATTERN,
    //    BYTE_TO_BINARY(TCCR5A), BYTE_TO_BINARY(TCCR5B));
    //myGLCD.setColor(255, 255, 255);
    //myGLCD.setFont(SmallFont);
    //myGLCD.print(str, LEFT, 85);

    myGLCD.setColor(255, 255, 255);
    myGLCD.setFont(SmallFont);

    myGLCD.print("OFF", LEFT, 85);
    sprintf(str, "C:%03d  W:%03d  Y:%03d  R:%03d  G:%03d  B:%03d",
            EEPROM.read(64), EEPROM.read(65), EEPROM.read(66),
            EEPROM.read(67), EEPROM.read(68), EEPROM.read(69));
    myGLCD.print(str, LEFT, 100);

    myGLCD.print("DAY", LEFT, 115);
    sprintf(str, "C:%03d  W:%03d  Y:%03d  R:%03d  G:%03d  B:%03d",
            EEPROM.read(70), EEPROM.read(71), EEPROM.read(72),
            EEPROM.read(73), EEPROM.read(74), EEPROM.read(75));
    myGLCD.print(str, LEFT, 130);

    myGLCD.print("NIGHT", LEFT, 145);
    sprintf(str, "C:%03d  W:%03d  Y:%03d  R:%03d  G:%03d  B:%03d",
            EEPROM.read(76), EEPROM.read(77), EEPROM.read(78),
            EEPROM.read(79), EEPROM.read(80), EEPROM.read(81));
    myGLCD.print(str, LEFT, 160);
}
#endif

void setup() {
    pinMode(RELE_PIN, OUTPUT);

    //Serial.begin(9600);
    RTC.begin();

    if (!RTC.isrunning()) {
        //Serial.println("RTC is NOT running!");
        RTC.adjust(DateTime(__DATE__, __TIME__));
    }

    t0.begin();
    t1.begin();
    t2.begin();
    t3.begin();

    eepromRead();

    offValues.load();
    dayValues.load();
    nightValues.load();
    actualLightValues = &dayValues;

    // Initial setup
    myGLCD.InitLCD(LANDSCAPE);
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0);
    myButtons.setTextFont(BigFont);

    myTouch.InitTouch(LANDSCAPE);
    myTouch.setPrecision(PREC_LOW);

    pinInit();
    timer2set();

    page = PAGE_HOME;
}

void returnHome() {
    myButtons.deleteAllButtons();
    myGLCD.clrScr();
    drawHomeScreen();
    page = PAGE_HOME;
}

void loop() {
    //PORTK &= ~(1 << PK7);
    //PORTK |= (1 << PK7);

    // read temperature each one second
    if (temperatureReadTimeCounter > 430) {
        t0.readTemperature();
        te0 = t0.getCelsius();
        myRound(&te0);

        te1 = t1.getCelsius();
        myRound(&te1);
        t1.readTemperature();

        if (temperatureSenzorStatus[2] == 1) {
            te2 = t2.getCelsius();
            myRound(&te2);
            t2.readTemperature();
        }
        if (temperatureSenzorStatus[3] == 1) {
            te3 = t3.getCelsius();
            myRound(&te3);
            t3.readTemperature();
        }
        temperatureReadTimeCounter = 0;
    }

    switch (page) {

    case PAGE_RETURN_MOME:
        returnHome();
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
            } else

            if (pressedButton == 1) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawTimerScreen();
                page = PAGE_SET_TIMER;
            } else

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
            } else

            if (pressedButton == 3) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawCo2Screen();
                page = PAGE_SET_CO2;
            } else

            if (pressedButton == 4) {
                myButtons.deleteAllButtons();
                myGLCD.clrScr();
                drawTempScreen();
                page = PAGE_TEMP;
            } else

            if (pressedButton == 5) {
                returnHome();
            }
#if DEBUG == 1
            else
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
                returnHome();
            } else
            if (pressedButton == 1) {
                returnHome();
            }
        }
        break;
#endif

    case PAGE_SET_LIGHT:
        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;

            pressedButton = myButtons.checkButtons();

            x = myButtons.Touch->getX();
            y = myButtons.Touch->getY();

            if (
                switchMode != MODE_AUTO &&
                y >= Y_TOUCH_AREA_MIN &&
                y <= Y_TOUCH_AREA_MAX) {
                checkTouchLedArea();
            } else

            if (pressedButton == 0) {
                // set light to day
                dimmingSpeed = 30;
                actualLightValues->save();
                actualLightValues = &dayValues;
                redrawSliders();
                //analogSwitch();
                setMode();
            } else

            if (pressedButton == 1) {
                // set light to off
                dimmingSpeed = 30;
                actualLightValues->save();
                actualLightValues = &offValues;
                redrawSliders();
                //analogSwitch();
                setMode();
            } else

            if (pressedButton == 2) {
                // set light to NIGHT
                dimmingSpeed = 30;
                actualLightValues->save();
                actualLightValues = &nightValues;
                redrawSliders();
                //analogSwitch();
                setMode();
            } else

            if (pressedButton == 3) {
                // set light mode to AUTO
                dimmingSpeed = 500;
                actualLightValues->save();
                switchMode++;
                if (switchMode == 2) {
                    switchMode = MODE_AUTO;
                }
                setMode();
                EEPROM.write(24, switchMode);
            } else

            if (pressedButton == 4) {
                returnHome();
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
            } else
            if (pressedButton == 1) {
                newTime.hour++;
                if (newTime.hour > 23) newTime.hour = 0;
                drawNewTime();
            } else
            if (pressedButton == 2) {
                newTime.minute--;
                if (newTime.minute == 255) newTime.minute = 59;
                drawNewTime();
            } else
            if (pressedButton == 3) {
                newTime.minute++;
                if (newTime.minute > 59) newTime.minute = 0;
                drawNewTime();
            } else
            if (pressedButton == 4) {
                newTime.second--;
                if (newTime.second == 255) newTime.second = 59;
                drawNewTime();
            } else
            if (pressedButton == 5) {
                newTime.second++;
                if (newTime.second > 59) newTime.second = 0;
                drawNewTime();
            } else
            if (pressedButton == 6) {
                newTime.day--;
                if (newTime.day < 1) newTime.day = 31;
                drawNewTime();
            }
            if (pressedButton == 7) {
                newTime.day++;
                if (newTime.day > 31) newTime.day = 1;
                drawNewTime();
            } else
            if (pressedButton == 8) {
                newTime.month--;
                if (newTime.month < 1) newTime.month = 12;
                drawNewTime();
            } else
            if (pressedButton == 9) {
                newTime.month++;
                if (newTime.month > 12) newTime.month = 1;
                drawNewTime();
            } else
            if (pressedButton == 10) {
                newTime.year--;
                if (newTime.year < 2000) newTime.year = 2050;
                drawNewTime();
            } else
            if (pressedButton == 11) {
                newTime.year++;
                if (newTime.year > 2050) newTime.year = 2000;
                drawNewTime();
            } else

            // save
            if (pressedButton == 12) {
                RTC.adjust(newTime);
            } else

            // home
            if (pressedButton == 13) {
                returnHome();
            }
        }
        break;

    case PAGE_TEMP:
        if (myTouch.dataAvailable() == true) {
            timerCounter2 = 0;
            pressedButton = myButtons.checkButtons();

            if (pressedButton == 0) {
                temperatureSenzorStatus[2]++;
                setTempButton(2);
                EEPROM.write(44, temperatureSenzorStatus[2]);
            } else
            if (pressedButton == 1) {
                temperatureSenzorStatus[3]++;
                setTempButton(3);
                EEPROM.write(45, temperatureSenzorStatus[3]);
            } else

            if (pressedButton == 2) {
                returnHome();
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
            } else
            if (pressedButton == 1) {
                co2hour[0]++;
                if (co2hour[0] > 23) {
                    co2hour[0] = 0;
                }
                setCo2TimerTime(0, 10, 8);
            } else
            if (pressedButton == 2) {
                co2minute[0]--;
                if (co2minute[0] == 255) {
                    co2minute[0] = 59;
                }
                setCo2TimerTime(0, 10, 8);
            } else
            if (pressedButton == 3) {
                co2minute[0]++;
                if (co2minute[0] > 59) {
                    co2minute[0] = 0;
                }
                setCo2TimerTime(0, 10, 8);
            } else
            if (pressedButton == 4) {
                setCo2Type(4, 0);
            } else

            if (pressedButton == 5) {
                co2hour[1]--;
                if (co2hour[1] == 255) {
                    co2hour[1] = 23;
                }
                setCo2TimerTime(1, 10, 64);
            } else
            if (pressedButton == 6) {
                co2hour[1]++;
                if (co2hour[1] > 23) {
                    co2hour[1] = 0;
                }
                setCo2TimerTime(1, 10, 64);
            } else
            if (pressedButton == 7) {
                co2minute[1]--;
                if (co2minute[1] == 255) {
                    co2minute[1] = 59;
                }
                setCo2TimerTime(1, 10, 64);
            } else
            if (pressedButton == 8) {
                co2minute[1]++;
                if (co2minute[1] > 59) {
                    co2minute[1] = 0;
                }
                setCo2TimerTime(1, 10, 64);
            } else
            if (pressedButton == 9) {
                setCo2Type(9, 1);
            } else

            // second column
            if (pressedButton == 10) {
                co2hour[2]--;
                if (co2hour[2] == 255) {
                    co2hour[2] = 23;
                }
                setCo2TimerTime(2, 176, 8);
            } else
            if (pressedButton == 11) {
                co2hour[2]++;
                if (co2hour[2] > 23) {
                    co2hour[2] = 0;
                }
                setCo2TimerTime(2, 176, 8);
            } else
            if (pressedButton == 12) {
                co2minute[2]--;
                if (co2minute[2] == 255) {
                    co2minute[2] = 59;
                }
                setCo2TimerTime(2, 176, 8);
            } else
            if (pressedButton == 13) {
                co2minute[2]++;
                if (co2minute[2] > 59) {
                    co2minute[2] = 0;
                }
                setCo2TimerTime(2, 176, 8);
            } else
            if (pressedButton == 14) {
                setCo2Type(14, 2);
            } else

            if (pressedButton == 15) {
                co2hour[3]--;
                if (co2hour[3] == 255) {
                    co2hour[3] = 23;
                }
                setCo2TimerTime(3, 176, 64);
            } else
            if (pressedButton == 16) {
                co2hour[3]++;
                if (co2hour[3] > 23) {
                    co2hour[3] = 0;
                }
                setCo2TimerTime(3, 176, 64);
            } else
            if (pressedButton == 17) {
                co2minute[3]--;
                if (co2minute[3] == 255) {
                    co2minute[3] = 59;
                }
                setCo2TimerTime(3, 176, 64);
            } else
            if (pressedButton == 18) {
                co2minute[3]++;
                if (co2minute[3] > 59) {
                    co2minute[3] = 0;
                }
                setCo2TimerTime(3, 176, 64);
            } else
            if (pressedButton == 19) {
                setCo2Type(19, 3);
            } else

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
            } else

            // home button
            if (pressedButton == 21) {
                returnHome();
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
            } else
            if (pressedButton == 1) {
                hourStates[0]++;
                if (hourStates[0] > 23) {
                    hourStates[0] = 0;
                }
                setTimerTime(0, 10, 8);
            } else
            if (pressedButton == 2) {
                minuteStates[0]--;
                if (minuteStates[0] == 255) {
                    minuteStates[0] = 59;
                }
                setTimerTime(0, 10, 8);
            } else
            if (pressedButton == 3) {
                minuteStates[0]++;
                if (minuteStates[0] > 59) {
                    minuteStates[0] = 0;
                }
                setTimerTime(0, 10, 8);
            } else
            if (pressedButton == 4) {
                setLightType(4, 0);
            } else

            // second time
            if (pressedButton == 5) {
                hourStates[1]--;
                if (hourStates[1] == 255) {
                    hourStates[1] = 23;
                }
                setTimerTime(1, 10, 64);
            } else
            if (pressedButton == 6) {
                hourStates[1]++;
                if (hourStates[1] > 23) {
                    hourStates[1] = 0;
                }
                setTimerTime(1, 10, 64);
            } else
            if (pressedButton == 7) {
                minuteStates[1]--;
                if (minuteStates[1] == 255) {
                    minuteStates[1] = 59;
                }
                setTimerTime(1, 10, 64);
            } else
            if (pressedButton == 8) {
                minuteStates[1]++;
                if (minuteStates[1] > 59) {
                    minuteStates[1] = 0;
                }
                setTimerTime(1, 10, 64);
            } else
            if (pressedButton == 9) {
                setLightType(9, 1);
            } else

            // third time
            if (pressedButton == 10) {
                hourStates[2]--;
                if (hourStates[2] == 255) {
                    hourStates[2] = 23;
                }
                setTimerTime(2, 10, 122);
            } else
            if (pressedButton == 11) {
                hourStates[2]++;
                if (hourStates[2] > 23) {
                    hourStates[2] = 0;
                }
                setTimerTime(2, 10, 122);
            } else
            if (pressedButton == 12) {
                minuteStates[2]--;
                if (minuteStates[2] == 255) {
                    minuteStates[2] = 59;
                }
                setTimerTime(2, 10, 122);
            } else
            if (pressedButton == 13) {
                minuteStates[2]++;
                if (minuteStates[2] > 59) {
                    minuteStates[2] = 0;
                }
                setTimerTime(2, 10, 122);
            } else
            if (pressedButton == 14) {
                setLightType(14, 2);
            } else

            if (pressedButton == 15) {
                hourStates[3]--;
                if (hourStates[3] == 255) {
                    hourStates[3] = 23;
                }
                setTimerTime(3, 176, 8);
            } else
            if (pressedButton == 16) {
                hourStates[3]++;
                if (hourStates[3] > 23) {
                    hourStates[3] = 0;
                }
                setTimerTime(3, 176, 8);
            } else
            if (pressedButton == 17) {
                minuteStates[3]--;
                if (minuteStates[3] == 255) {
                    minuteStates[3] = 59;
                }
                setTimerTime(3, 176, 8);
            } else
            if (pressedButton == 18) {
                minuteStates[3]++;
                if (minuteStates[3] > 59) {
                    minuteStates[3] = 0;
                }
                setTimerTime(3, 176, 8);
            } else
            if (pressedButton == 19) {
                setLightType(19, 3);
            } else

            if (pressedButton == 20) {
                hourStates[4]--;
                if (hourStates[4] == 255) {
                    hourStates[4] = 23;
                }
                setTimerTime(4, 176, 64);
            } else
            if (pressedButton == 21) {
                hourStates[4]++;
                if (hourStates[4] > 23) {
                    hourStates[4] = 0;
                }
                setTimerTime(4, 176, 64);
            } else
            if (pressedButton == 22) {
                minuteStates[4]--;
                if (minuteStates[4] == 255) {
                    minuteStates[4] = 59;
                }
                setTimerTime(4, 176, 64);
            } else
            if (pressedButton == 23) {
                minuteStates[4]++;
                if (minuteStates[4] > 59) {
                    minuteStates[4] = 0;
                }
                setTimerTime(4, 176, 64);
            } else
            if (pressedButton == 24) {
                setLightType(24, 4);
            } else

            if (pressedButton == 25) {
                hourStates[5]--;
                if (hourStates[5] == 255) {
                    hourStates[5] = 23;
                }
                setTimerTime(5, 176, 120);
            } else
            if (pressedButton == 26) {
                hourStates[5]++;
                if (hourStates[5] > 23) {
                    hourStates[5] = 0;
                }
                setTimerTime(5, 176, 120);
            } else
            if (pressedButton == 27) {
                minuteStates[5]--;
                if (minuteStates[5] == 255) {
                    minuteStates[5] = 59;
                }
                setTimerTime(5, 176, 120);
            } else
            if (pressedButton == 28) {
                minuteStates[5]++;
                if (minuteStates[5] > 59) {
                    minuteStates[5] = 0;
                }
                setTimerTime(5, 176, 120);
            } else
            if (pressedButton == 29) {
                setLightType(29, 5);
            } else

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
            } else

            // home button
            if (pressedButton == 31) {
                returnHome();
            }
        }
        break;
    }
#if DEBUG == 1
    debug();
#endif
}
