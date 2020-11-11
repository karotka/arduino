#include "configure.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <RtcDS3231.h>
#include "values.h"
#include <Buttons.h>
#include "keyboard.h"


//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "cz.pool.ntp.org", 2 * 3600);
// RTC Libraries
RtcDS3231<TwoWire> Rtc(Wire);
TFT_eSPI tft = TFT_eSPI(420, 380);

Buttons btns(&tft);
button_t pressetButton;

Values_t *actualMode;
Values_t mode0(MODE_0);
Values_t mode1(MODE_1);
Values_t mode2(MODE_2);
Values_t mode3(MODE_3);
Values_t mode4(MODE_4);

bool automat;

enum page {
    HOME_PAGE = 0,
    LIGHT_PAGE,
    CO2_PAGE,
    FEED_PAGE,
    TIME_PAGE,
    TIMER_PAGE,
    SETTING_IP,
    SETTING_WIFI
};

char str[40];
uint16_t x, y;
unsigned int printedDay;

hw_timer_t * timer0 = NULL;
volatile int page;
volatile int returnHomeCounter;
volatile int returnHomeNow = false;;

AsyncWebServer server(80);

void IRAM_ATTR onTimer0() {

    if (page != HOME_PAGE) {
        returnHomeCounter++;
        if (returnHomeCounter > 10) {
            returnHomeCounter = 0;
            returnHomeNow = true;
        }
    }

    if (page == LIGHT_PAGE) {
        for(int i = 0; i < 6; i++) {
            int m = map(actualMode->sliders[i], 0, 300, 0, 1023);
            ledcWrite(i, m);
        }
    }
}

int co2Values[12];
int feedValues[12];

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

void co2Save() {
    int address = 61;
    EEPROM.put(address, co2Values);
    EEPROM.commit();

    address += sizeof(co2Values);
}

void feedSave() {
    int address = 110;
    EEPROM.put(address, feedValues);
    EEPROM.commit();

    address += sizeof(feedValues);
}

void co2Load() {
    EEPROM.get(61, co2Values);
    EEPROM.commit();
}

void feedLoad() {
    EEPROM.get(110, feedValues);
    EEPROM.commit();
}

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
    sprintf (str, "X: %3d, Y: %3d", x, y);
    tft.println(str);
}

void drawHome() {
    tft.fillScreen(TFT_BLACK);
    printedDay = 0;

    tft.fillRect(0, 0, 480, 78, TFT_VIOLET);

    tft.fillRect(0, 80, 300, 79, TFT_DARKBLUE);
    tft.fillRect(302, 80, 178, 78, TFT_ORANGE);

    tft.fillRect(0, 160, 480, 78, TFT_DARKGREEN);

    tft.fillRect(0, 240, 238, 78, TFT_SKY);
    tft.fillRect(241, 240, 238, 78, TFT_DARKGREY);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);

    tft.setCursor(90, 167, 1);
    tft.println("Ch1");
    tft.setCursor(90, 192, 1);
    tft.println("Ch2");
    tft.setCursor(90, 217, 1);
    tft.println("Ch3");

    tft.setCursor(270, 167, 1);
    tft.println("Ch4");
    tft.setCursor(270, 192, 1);
    tft.println("Ch5");
    tft.setCursor(270, 217, 1);
    tft.println("Ch6");
}

void drawHeader(const uint16_t bgLight, uint16_t bgDark, const char* label[], size_t size) {

    tft.fillScreen(bgLight);
    tft.setTextColor(TFT_WHITE, bgLight);
    tft.fillRect(0, 0, 40, 40, bgDark);

    tft.setTextSize(1);

    int space = 40;
    for (int i = 0; i < size; i++) {
        space = space + 15;
        int textWidth = tft.textWidth(label[i], 4);
        tft.setCursor(space, 12, 4);
        tft.println(label[i]);
        space = space + 15 + textWidth;
        tft.drawLine(space, 40, space, 0, bgDark);
    }
    tft.fillRect(space, 0, 480 - space, 40, bgDark);

    // close
    tft.drawLine(5, 5, 35, 35, TFT_WHITE);
    tft.drawLine(35, 5, 5, 35, TFT_WHITE);
    // save
    tft.drawLine(440, 20, 457, 35, TFT_WHITE);
    tft.drawLine(457, 35, 475, 5, TFT_WHITE);
}

void relableLightButtonsByMode() {

    PR("AUTOMAT:"); PRN(automat);
    if (automat) {
        btns.setColor(0,  TFT_BLACK);
        btns.relableButton(0,  "AUT");
        for (int i = 7; i < 12; i++) {
            btns.disable(i);
        }
    } else {
        btns.setColor(0,  TFT_DARKGREEN);
        btns.relableButton(0,  "AUT");
        for (int i = 7; i < 12; i++) {
            btns.enable(i);
        }
        btns.setColor(7,  TFT_DARKGREEN);
        btns.setColor(8,  TFT_DARKGREEN);
        btns.setColor(9,  TFT_DARKGREEN);
        btns.setColor(10,  TFT_DARKGREEN);
        btns.setColor(11,  TFT_DARKGREEN);
        btns.relableButton(7,  "M1");
        btns.relableButton(8,  "M2");
        btns.relableButton(9,  "M3");
        btns.relableButton(10, "M4");
        btns.relableButton(11, "M5");

        switch (actualMode->modeId) {
        case MODE_0:
            btns.setColor(7,  TFT_BLACK);
            btns.relableButton(7,  "M1");
            break;
        case MODE_1:
            btns.setColor(8,  TFT_BLACK);
            btns.relableButton(8,  "M2");
            break;
        case MODE_2:
            btns.setColor(9,  TFT_BLACK);
            btns.relableButton(9,  "M3");
            break;
        case MODE_3:
            btns.setColor(10,  TFT_BLACK);
            btns.relableButton(10, "M4");
            break;
        case MODE_4:
            btns.setColor(11,  TFT_BLACK);
            btns.relableButton(11, "M5");
            break;
        default:
            break;

        }

    }

    for (int i = 1; i < 7; i++) {
        btns.setSlider(i, actualMode->sliders[i - 1]);
    }
}

void drawLight() {
    page = LIGHT_PAGE;

    static const char* names[] = {"LIGHTS"};
    drawHeader(TFT_GREEN, TFT_DARKGREEN, names, 1);

    btns.deleteAllButtons();

    // button 0
    btns.addButton(10,  270, 70, 45, "AUT", TFT_DARKGREEN);

    // buttons 1 - 6
    int space = 55;
    for (int i = 0; i < 6; i++) {
        sprintf (str, "Channel %d", i + 1);
        btns.addSlider(15, space, 300, 14,
                       actualMode->sliders[i], str, TFT_GREEN, TFT_WHITE);
        space += 35;
    }

    // buttons 7 - 11
    btns.addButton(100, 270, 55, 45, "M1", TFT_DARKGREEN);
    btns.addButton(160, 270, 55, 45, "M2", TFT_DARKGREEN);
    btns.addButton(220, 270, 55, 45, "M3", TFT_DARKGREEN);
    btns.addButton(280, 270, 55, 45, "M4", TFT_DARKGREEN);
    btns.addButton(340, 270, 55, 45, "M5", TFT_DARKGREEN);

    btns.draw();

    relableLightButtonsByMode();
}

void drawCo2() {
    page = CO2_PAGE;

    static const char* names[] = {"CO2"};
    drawHeader(TFT_ORANGE, TFT_DARKORANGE, names, 1);

    btns.deleteAllButtons();

    btns.addSelect(40,  85, 50, 50, co2Values[0], TFT_DARKORANGE);
    btns.addSelect(110, 85, 50, 50, co2Values[1], TFT_DARKORANGE);
    btns.addButton(180, 85, 50, 50, onOffStr[co2Values[2]], TFT_DARKORANGE);

    btns.addSelect(280, 85, 50, 50, co2Values[3], TFT_DARKORANGE);
    btns.addSelect(350, 85, 50, 50, co2Values[4], TFT_DARKORANGE);
    btns.addButton(420, 85, 50, 50, onOffStr[co2Values[5]], TFT_DARKORANGE);

    btns.addSelect(40,  210, 50, 50, co2Values[6], TFT_DARKORANGE);
    btns.addSelect(110, 210, 50, 50, co2Values[7], TFT_DARKORANGE);
    btns.addButton(180, 210, 50, 50, onOffStr[co2Values[8]], TFT_DARKORANGE);

    btns.addSelect(280, 210, 50, 50, co2Values[9], TFT_DARKORANGE);
    btns.addSelect(350, 210, 50, 50, co2Values[10], TFT_DARKORANGE);
    btns.addButton(420, 210, 50, 50, onOffStr[co2Values[11]], TFT_DARKORANGE);

    btns.draw();
}

void drawFeed() {
    page = FEED_PAGE;

    static const char* names[] = {"FEED"};
    drawHeader(TFT_SKY, TFT_DARKSKY, names, 1);

    btns.deleteAllButtons();

    btns.addSelect(40,  85, 50, 50, feedValues[0], TFT_DARKSKY);
    btns.addSelect(110, 85, 50, 50, feedValues[1], TFT_DARKSKY);
    btns.addButton(180, 85, 50, 50, onOffStr[feedValues[2]], TFT_DARKSKY);

    btns.addSelect(280, 85, 50, 50, feedValues[3], TFT_DARKSKY);
    btns.addSelect(350, 85, 50, 50, feedValues[4], TFT_DARKSKY);
    btns.addButton(420, 85, 50, 50, onOffStr[feedValues[5]], TFT_DARKSKY);

    btns.addSelect(40,  210, 50, 50, feedValues[6], TFT_DARKSKY);
    btns.addSelect(110, 210, 50, 50, feedValues[7], TFT_DARKSKY);
    btns.addButton(180, 210, 50, 50, onOffStr[feedValues[8]], TFT_DARKSKY);

    btns.addSelect(280, 210, 50, 50, feedValues[9], TFT_DARKSKY);
    btns.addSelect(350, 210, 50, 50, feedValues[10], TFT_DARKSKY);
    btns.addButton(420, 210, 50, 50, onOffStr[feedValues[11]], TFT_DARKSKY);

    btns.draw();
}

void drawSettingIp() {
    page = SETTING_IP;

    static const char* names[] = {"WIFI", "IP"};
    drawHeader(TFT_MYGRAY, TFT_MYDARKGRAY, names, 2);

    btns.deleteAllButtons();

    btns.addSelect(100,  70, 50, 30, 192, TFT_MYDARKGRAY);
    btns.addSelect(170, 70, 50, 30, 168, TFT_MYDARKGRAY);
    btns.addSelect(240, 70, 50, 30, 1, TFT_MYDARKGRAY);
    btns.addSelect(310, 70, 50, 30, 10, TFT_MYDARKGRAY);

    btns.addSelect(100,  160, 50, 30, 255, TFT_MYDARKGRAY);
    btns.addSelect(170, 160, 50, 30, 255, TFT_MYDARKGRAY);
    btns.addSelect(240, 160, 50, 30, 255, TFT_MYDARKGRAY);
    btns.addSelect(310, 160, 50, 30, 0, TFT_MYDARKGRAY);

    btns.addSelect(100,  250, 50, 30, 255, TFT_MYDARKGRAY);
    btns.addSelect(170, 250, 50, 30, 255, TFT_MYDARKGRAY);
    btns.addSelect(240, 250, 50, 30, 255, TFT_MYDARKGRAY);
    btns.addSelect(310, 250, 50, 30, 0, TFT_MYDARKGRAY);

    btns.draw();
}

void drawSettingWifi() {
    page = SETTING_WIFI;

    static const char* names[] = {"WIFI", "IP" };
    drawHeader(TFT_MYGRAY, TFT_MYDARKGRAY, names, 2);

    btns.deleteAllButtons();

    btns.addCheckbox(310, 250, "Test", TFT_MYDARKGRAY);

    btns.draw();
}

void returnHome() {
    page = HOME_PAGE;
    drawHome();
}

void save() {
    switch (page) {
    case LIGHT_PAGE:
        mode0.save();
        mode1.save();
        mode2.save();
        mode3.save();
        mode4.save();
        break;
    case CO2_PAGE:
        co2Save();
        break;
    case FEED_PAGE:
        feedSave();
        break;
    }

    page = HOME_PAGE;
    drawHome();
}

void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "message received");
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

    ledcSetup(0, PWM_RESOLUTION, PWM_RESOLUTION);
    ledcSetup(1, PWM_RESOLUTION, PWM_RESOLUTION);
    ledcSetup(2, PWM_RESOLUTION, PWM_RESOLUTION);
    ledcSetup(3, PWM_RESOLUTION, PWM_RESOLUTION);
    ledcSetup(4, PWM_RESOLUTION, PWM_RESOLUTION);
    ledcSetup(5, PWM_RESOLUTION, PWM_RESOLUTION);

    ledcAttachPin(19, 0);
    ledcAttachPin(18, 1);
    ledcAttachPin(5, 2);
    ledcAttachPin(17, 3);
    ledcAttachPin(16, 4);
    ledcAttachPin(4, 5);

    mode0.load();
    mode1.load();
    mode2.load();
    mode3.load();
    mode4.load();
    actualMode = &mode0;

    co2Load();
    feedLoad();

    automat = true;

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleRoot(request);
    });

    returnHome();
}

void drawTime() {
    RtcDateTime dt = Rtc.GetDateTime();
    char strTime[20];
    sprintf (strTime, "%02u %04u", dt.Month(), dt.Year());
    tft.setTextColor(TFT_WHITE, TFT_DARKBLUE);
    tft.setTextSize(1);
    tft.setCursor(67, 125, 4);
    tft.println(strTime);

    sprintf (strTime, "%02u:%02u:%02u", dt.Hour(), dt.Minute(), dt.Second());
    tft.setTextColor(TFT_WHITE, TFT_DARKBLUE);
    tft.setTextSize(1);
    tft.setCursor(65, 95, 4);
    tft.println(strTime);

    if (dt.Day() != printedDay) {
        sprintf (strTime, "%02u th", dt.Day());
        tft.setTextColor(TFT_WHITE, TFT_DARKBLUE);
        tft.setTextSize(2);
        tft.setCursor(180, 100, 4);
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
    tft.setCursor(126, 52, 4);
    tft.println(str);
}

bool checkHomeSave(uint16_t x, uint16_t y) {
    if (x < 40 && y < 40) {
        returnHome();
        return true;
    } else
    if (x > 440 && y < 40) {
        save();
        return true;
    }
    return false;
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
                } else
                if (x > 0 && x < 238 && y > 240) { // feed
                    drawFeed();
                } else
                if (x > 240 && y > 240) { // Setting
                    drawSettingWifi();
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

            if (checkHomeSave(x, y)) {
                break;
            } else
            if (inArray(hours, id, 4)) {
                if (pressetButton.flags & BUTTON_TOUCH_UP) {
                    co2Values[id]++;
                    if (co2Values[id] > 23) {
                        co2Values[id] = 0;
                    }
                    btns.relableButton(id, co2Values[id]);
                } else
                if(pressetButton.flags & BUTTON_TOUCH_DOWN) {
                    --co2Values[id];
                    if (co2Values[id] < 0) {
                        co2Values[id] = 23;
                    }
                    btns.relableButton(id, co2Values[id]);
                }
            } else
            if (inArray(minutes, id, 4)) {
                if (pressetButton.flags & BUTTON_TOUCH_UP) {
                    co2Values[id]++;
                    if (co2Values[id] > 59) {
                        co2Values[id] = 0;
                    }
                    btns.relableButton(id, co2Values[id]);
                } else
                if(pressetButton.flags & BUTTON_TOUCH_DOWN) {
                    --co2Values[id];
                    if (co2Values[id] < 0) {
                        co2Values[id] = 59;
                    }
                    btns.relableButton(id, co2Values[id]);
                }
            } else
            if (inArray(buttons, id, 4)) {
                if(pressetButton.flags & BUTTON_TOUCH_DOWN) {
                    co2Values[id] = !co2Values[id];
                    btns.relableButton(id, onOffStr[co2Values[id]]);
                }
            }
        }
        break;

    case LIGHT_PAGE:
        if (tft.getTouch(&x, &y)) {
            touchResponse();
            uint8_t id = btns.checkButtons(x, y);
            pressetButton = btns.getButton(id);

            if (checkHomeSave(x, y)) {
                break;
            } else
            if (pressetButton.flags & BUTTON_TOUCH_SLIDE) {
                PR("B.ID:"); PR(id);
                actualMode->sliders[id - 1] = pressetButton.value;
            } else
            if (pressetButton.flags & BUTTON_TYPE_BUTTON) {
                PR("B.ID:"); PR(id);
                if (id == 0)
                    automat = !automat;
                if (id == 7)
                    actualMode = &mode0;
                if (id == 8)
                    actualMode = &mode1;
                if (id == 9)
                    actualMode = &mode2;
                if (id == 10)
                    actualMode = &mode3;
                if (id == 11)
                    actualMode = &mode4;
                relableLightButtonsByMode();
            }
        }
        break;

    case SETTING_WIFI:
        if (tft.getTouch(&x, &y)) {

            touchResponse();
            uint8_t id = btns.checkButtons(x, y);
            pressetButton = btns.getButton(id);

            if (checkHomeSave(x, y)) {
                break;
            } else
            if (x > 101 && x < 180 && y < 40) {
                drawSettingIp();
            }
        }
        break;

    case SETTING_IP:
        if (tft.getTouch(&x, &y)) {

            touchResponse();
            uint8_t id = btns.checkButtons(x, y);
            pressetButton = btns.getButton(id);

            if (checkHomeSave(x, y)) {
                break;
            } else
            if (x > 41 && x < 100 && y < 40 ) {
                drawSettingWifi();
            }
        }
        break;

    case FEED_PAGE:
        if (tft.getTouch(&x, &y)) {

            touchResponse();
            uint8_t id = btns.checkButtons(x, y);
            pressetButton = btns.getButton(id);

            PR("ButtonID: ");
            PRN(id);

            if (checkHomeSave(x, y)) {
                break;
            } else
            if (inArray(hours, id, 4)) {
                if (pressetButton.flags & BUTTON_TOUCH_UP) {
                    feedValues[id]++;
                    if (feedValues[id] > 23) {
                        feedValues[id] = 0;
                    }
                    btns.relableButton(id, feedValues[id]);
                } else
                if(pressetButton.flags & BUTTON_TOUCH_DOWN) {
                    --feedValues[id];
                    if (feedValues[id] < 0) {
                        feedValues[id] = 23;
                    }
                    btns.relableButton(id, feedValues[id]);
                }
            } else
            if (inArray(minutes, id, 4)) {
                if (pressetButton.flags & BUTTON_TOUCH_UP) {
                    feedValues[id]++;
                    if (feedValues[id] > 59) {
                        feedValues[id] = 0;
                    }
                    btns.relableButton(id, feedValues[id]);
                } else
                if(pressetButton.flags & BUTTON_TOUCH_DOWN) {
                    --feedValues[id];
                    if (feedValues[id] < 0) {
                        feedValues[id] = 59;
                    }
                    btns.relableButton(id, feedValues[id]);
                }
            } else
            if (inArray(buttons, id, 4)) {
                if(pressetButton.flags & BUTTON_TOUCH_DOWN) {
                    feedValues[id] = !feedValues[id];
                    btns.relableButton(id, onOffStr[feedValues[id]]);
                }
            }
        }
        break;
    }

    delay(50);
}
