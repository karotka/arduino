#include "configure.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RtcDS3231.h>
#include "lightvalues.h"
#include <Buttons.h>


//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "us.pool.ntp.org", 2 * 3600);
// RTC Libraries
RtcDS3231<TwoWire> Rtc(Wire);
TFT_eSPI tft = TFT_eSPI(420, 380);

Buttons btns(&tft);
button_t pressetButton;
LightValues_t lightValues;

enum page {
    HOME_PAGE = 0,
    LIGHT_PAGE,
    CO2_PAGE
};

char str[40];
uint16_t x, y, xa, ya;
unsigned int printedDay;


hw_timer_t * timer0 = NULL;
volatile int page;
volatile int returnHomeCounter;
volatile int returnHomeNow = false;;


void RTC_Update() {
    //timeClient.update();
    //unsigned long epochTime = timeClient.getEpochTime()-946684800UL;
    //Rtc.SetDateTime(epochTime);
}

bool RTC_Valid(){
    bool boolCheck = true;
    if (!Rtc.IsDateTimeValid()){
        Serial.println("RTC lost confidence");
        boolCheck = false;
        RTC_Update();
    }

    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now.");
        Rtc.SetIsRunning(true);
        boolCheck = false;
        RTC_Update();
    }
}


void touchResponse() {
    returnHomeCounter = 0;

    // write touch debug
    tft.setTextSize(2);
    tft.setCursor(10, 300, 1);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    sprintf (str, "X: %3d, Y: %3d, X0: %5u, Y0: %5u", x, y, xa, ya);
    tft.println(str);
}

void drawHome() {
    tft.fillScreen(TFT_BLACK);
    printedDay = 0;

    tft.fillRect(0, 0, 480, 78, TFT_VIOLET);

    tft.fillRect(0, 80, 258, 78, TFT_BLUE);
    tft.fillRect(260, 80, 240, 78, TFT_ORANGE);

    tft.fillRect(0, 160, 480, 78, TFT_DARKGREEN);

    tft.fillRect(0, 240, 238, 78, TFT_CYAN);
    tft.fillRect(241, 240, 238, 78, TFT_DARKGREY);

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(90, 165, 4);
    tft.println("Ch1");
    tft.setCursor(90, 190, 4);
    tft.println("Ch2");
    tft.setCursor(90, 215, 4);
    tft.println("Ch3");

    tft.setCursor(260, 165, 4);
    tft.println("Ch4");
    tft.setCursor(260, 190, 4);
    tft.println("Ch5");
    tft.setCursor(260, 215, 4);
    tft.println("Ch6");
}

void drawHeader(const uint16_t bgLight, uint16_t bgDark, const char* label) {

    tft.fillScreen(bgLight);
    tft.fillRect(0, 0, 40, 40, bgDark);
    tft.fillRect(180, 0, 300, 40, bgDark);

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, bgLight);

    tft.setCursor(60, 12, 4);

    tft.println(label);

    // close
    tft.drawLine(5, 5, 35, 35, TFT_WHITE);
    tft.drawLine(35, 5, 5, 35, TFT_WHITE);
    // save
    tft.drawLine(440, 20, 457, 35, TFT_WHITE);
    tft.drawLine(457, 35, 475, 5, TFT_WHITE);

}

void drawLight() {
    page = LIGHT_PAGE;
    lightValues.load(0);

    drawHeader(TFT_GREEN, TFT_DARKGREEN, "LIGHTS");

    btns.deleteAllButtons();

    int space = 55;
    for (int i = 1; i < 7; i++) {
        sprintf (str, "Channel %d", i);
        btns.addSlider(15, space, 300, 14,
                       lightValues.getSlider(i - 1).pixels, str, TFT_WHITE);
        space += 35;
    }

    btns.addButton(10,  270, 70, 45, "AUT", TFT_DARKGREEN);
    btns.addButton(100, 270, 55, 45, "M1", TFT_DARKGREEN);
    btns.addButton(160, 270, 55, 45, "M2", TFT_DARKGREEN);
    btns.addButton(220, 270, 55, 45, "M3", TFT_DARKGREEN);
    btns.addButton(280, 270, 55, 45, "M4", TFT_DARKGREEN);
    btns.addButton(340, 270, 55, 45, "M5", TFT_DARKGREEN);

    btns.draw();

}


void drawCo2() {
    page = CO2_PAGE;

    drawHeader(TFT_ORANGE, TFT_DARKORANGE, "CO2");

    btns.deleteAllButtons();

    btns.addSelect(40,  85, 50, 50, "0", TFT_DARKORANGE);
    btns.addSelect(110, 85, 50, 50, "1", TFT_DARKORANGE);
    btns.addButton(180, 85, 50, 50, "2", TFT_DARKORANGE);

    btns.addSelect(280, 85, 50, 50, "3", TFT_DARKORANGE);
    btns.addSelect(350, 85, 50, 50, "4", TFT_DARKORANGE);
    btns.addButton(420, 85, 50, 50, "5", TFT_DARKORANGE);

    btns.addSelect(40,  210, 50, 50, "6", TFT_DARKORANGE);
    btns.addSelect(110, 210, 50, 50, "7", TFT_DARKORANGE);
    btns.addButton(180, 210, 50, 50, "8", TFT_DARKORANGE);

    btns.addSelect(280, 210, 50, 50, "9", TFT_DARKORANGE);
    btns.addSelect(350, 210, 50, 50, "10", TFT_DARKORANGE);
    btns.addButton(420, 210, 50, 50, "11", TFT_DARKORANGE);

    btns.draw();
}

void returnHome() {
    page = HOME_PAGE;
    drawHome();
}

void save() {
    switch (page) {
    case LIGHT_PAGE:
        lightValues.save(0);
        break;
    }
    page = HOME_PAGE;
    drawHome();
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt) {
    char datestring[20];
    snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.println(datestring);
}

void IRAM_ATTR onTimer0() {

    if (page != HOME_PAGE) {
        returnHomeCounter++;
        if (returnHomeCounter > 10) {
            returnHomeCounter = 0;
            returnHomeNow = true;
        }
    }
}

void setup(void) {
    Serial.begin(115200);
    Serial.println("\n\nStarting...");

    EEPROM.begin(EEPROM_SIZE);
    //WiFi.begin(ssid, password);
    //while (WiFi.status() != WL_CONNECTED) {
    //    delay(500);
    //    Serial.println("Connecting to WiFi..");
    //}

    //timeClient.begin();
    //delay(2000);
    //timeClient.update();
    Rtc.Begin();
    //RTC_Update();

    tft.init();

    tft.setRotation(1);
    tft.setTextFont(4);

    timer0 = timerBegin(0, 80, true);
    timerAttachInterrupt(timer0, &onTimer0, true);
    timerAlarmWrite(timer0, 1000000, true);
    timerAlarmEnable(timer0);

    returnHome();
}

void drawTime() {
    RtcDateTime dt = Rtc.GetDateTime();
    char strTime[20];
    sprintf (strTime, "%02u %04u", dt.Month(), dt.Year());
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(1);
    tft.setCursor(22, 125, 4);
    tft.println(strTime);

    sprintf (strTime, "%02u:%02u:%02u", dt.Hour(), dt.Minute(), dt.Second());
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(1);
    tft.setCursor(20, 95, 4);
    tft.println(strTime);

    if (dt.Day() != printedDay) {
        sprintf (strTime, "%02u th", dt.Day());
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.setTextSize(2);
        tft.setCursor(140, 100, 4);
        tft.println(strTime);
        printedDay = dt.Day();
    }
}

void drawTemperature() {
    RtcTemperature dt = Rtc.GetTemperature();
    char str[20];
    sprintf (str, "Board temp. %.1f C", dt.AsFloatDegC());
    tft.setTextColor(TFT_WHITE, TFT_VIOLET);
    tft.setTextSize(1);
    tft.setCursor(120, 48, 4);
    tft.println(str);
}



void loop() {


    if (returnHomeNow) {
        returnHomeNow = false;
        returnHome();
    }

    switch (page) {

    case HOME_PAGE:
        {
            drawTime();
            drawTemperature();

            if (tft.getTouch(&x, &y)) {
                touchResponse();
                if (y > 160 && y < 239) { // lights
                    drawLight();
                } else
                if (x > 260 && y > 80 && y < 158) { // co2
                    drawCo2();
                }
            }
        }
        break;
    case CO2_PAGE:
        if (tft.getTouch(&x, &y)) {

            touchResponse();
            uint8_t id = btns.checkButtons(x, y);
            pressetButton = btns.getButton(id);

            PR("ButtonID: ");
            PRN(id);
            if (x < 40 && y < 40) {
                PRN("HOME");
                returnHome();
            } else
            if (x > 440 && y < 40) {
                PRN("SAVE");
                save();
            } else
            if (id == 0) {
                if (pressetButton.flags & BUTTON_TOUCH_UP) {
                    PRN(" -> Touch UP ");
                    btns.relableButton(id, "", TFT_DARKORANGE);
                } else
                if (pressetButton.flags & BUTTON_TOUCH_DOWN) {
                    PRN(" -> Touch DOWN ");
                    btns.relableButton(id, "", TFT_DARKORANGE);
                }
            }

        }
        break;
    case LIGHT_PAGE:
        if (tft.getTouch(&x, &y)) {
            tft.getTouchRaw(&xa, &ya);
            touchResponse();
            uint8_t id = btns.checkButtons(x, y);
            pressetButton = btns.getButton(id);

            if (x < 40 && y < 40) {
                returnHome();
            } else
            if (x > 440 && y < 40) {
                save();
            }
            if (pressetButton.flags & BUTTON_TOUCH_SLIDE) {
                PR("B.ID:"); PR(id);
                PR(" B.VAL:"); PRN(pressetButton.value);
                lightValues.sliders[id].pixels = pressetButton.value;
            }
        }
        break;

    default:
        break;

    }

    delay(100);
}
