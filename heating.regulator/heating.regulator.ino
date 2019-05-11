#include <SPI.h>
#include <Wire.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
#include <DHT.h>
#include "RTClib.h"
#include "heatvalues.h"
#include "configure.h"
#include "images.h"
#include "temperature.h"
#include "keyboard.h"

volatile int page;

XPT2046 touch(/*cs=*/ MOSI, /*irq=*/ 16);
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
Adafruit_GFX_Button buttons[15];
HeatValues actualHeatValues;
WifiConfig wifiConfig;
Temperature temperature(dht);
Ticker timer0;
NewTime newTime;
DateTime now;

volatile unsigned int returnHomeCounter = 0;
volatile unsigned int temperatureCounter = 0;
volatile unsigned int homeReadIntervalCounter = 0;

bool holliday = false;
bool manually = false;

float requiredTemp = 23.0;
float setHeatTemp = 20.0;

int setHeatHour, setHeatMinute = 0;
int hourHeatValueIndex = 0;
int selectedButton = 0;
int heatRowBasePos = 100;

int textPosition = 0;
char *textBuffer;


void wifiConnect() {
    WiFi.disconnect();

    // Connect to WiFi network
    //Serial.println("");
    //Serial.print("Connecting to: ");
    //Serial.print(WIFI_SSID);
    wifiIp.fromString(wifiConfig.wifiIp);
    wifiGateway.fromString(wifiConfig.wifiGateway);
    wifiSubnet.fromString(wifiConfig.wifiSubnet);

    //WiFi.hostname(DEVICENAME);
    WiFi.config(wifiIp, wifiGateway, wifiSubnet);
    WiFi.mode(WIFI_STA);

    WiFi.begin(wifiConfig.wifiEssid, wifiConfig.wifiPass);

    bool status = false;
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        if (status) {
            tft.drawBitmap(138, 165, 35, 35, wifi, ILI9341_BLUE);
        } else {
            tft.drawBitmap(138, 165, 35, 35, wifi, ILI9341_NAVY);
        }
        status = !status;
        //Serial.print(".");
    }
    tft.drawBitmap(138, 165, 35, 35, wifi, ILI9341_BLUE);

    //Serial.println("");
    //ESPserial.print("WiFi connected: ");
    //ESPserial.print("http://");
    //Serial.print(WiFi.localIP().toString());
    //ESPserial.println("/");
}

void drawBorder() {
    tft.writeLine(0, 3, 240, 3,     ILI9341_WHITE);
    tft.writeLine(0, 3, 0, 320,     ILI9341_WHITE);
    tft.writeLine(0, 319,  240, 319,ILI9341_WHITE);
    tft.writeLine(239, 319, 239, 3, ILI9341_WHITE); // DARKGREY
}

void drawSettingScreen() {
    tft.fillScreen(ILI9341_BLACK); //Set Background Color with BLACK

    drawBorder();

    tft.setTextColor (ILI9341_LIGHTYELLOW, ILI9341_BLACK);

    tft.setCursor (6, 0);
    tft.setTextSize (1);
    tft.print(" WIFI SETTING ");

    tft.setCursor (6, 10);
    tft.print("SSID");

    tft.setCursor (6, 42);
    tft.print("PASSWORD");

    tft.setCursor (6, 74);
    tft.print("IP");

    tft.setCursor (6, 106);
    tft.print("GATEWAY");

    tft.setCursor (6, 138);
    tft.print("SUBNET");

    for (int i = 38; i < 167; i = i + 32) {
        tft.writeLine(6, i, 232, i, ILI9341_WHITE);
    }

    tft.setTextSize (2);

    tft.setCursor (6, 22);
    tft.print(wifiConfig.wifiEssid);

    tft.setCursor (6, 54);
    tft.print(wifiConfig.wifiPass);

    tft.setCursor (6, 86);
    tft.print(wifiConfig.wifiIp);

    tft.setCursor (6, 118);
    tft.print(wifiConfig.wifiGateway);

    tft.setCursor (6, 150);
    tft.print(wifiConfig.wifiSubnet);

    wifiConfig.line = 0;
    textPosition = 0;
    textBuffer = wifiConfig.text[0];

    buttons[8].initButton(&tft, 100, 183, 60, 27,
                          ILI9341_LIGHTYELLOW, ILI9341_BLUE, ILI9341_BLACK, "CON", 2);
    buttons[8].drawButton();

    buttons[9].initButton(&tft, 34, 183, 60, 27,
                          ILI9341_LIGHTYELLOW, ILI9341_BLUE, ILI9341_BLACK, "BACK", 2);
    buttons[9].drawButton();

    buttons[10].initButton(&tft, 207, 183, 60, 27,
                          ILI9341_LIGHTYELLOW, ILI9341_RED, ILI9341_WHITE, "SAVE", 2);
    buttons[10].drawButton();

    MakeKB_Button(KB);
}

void drawSetHeatScreenBtns(Adafruit_GFX_Button buttons[]) {
    buttons[11].initButton(&tft, 24, 37, 30, 43,
                          ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, "-", 3);
    buttons[11].drawButton();

    buttons[12].initButton(&tft, 165, 37, 30, 43,
                          ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, "+", 3);
    buttons[12].drawButton();

    buttons[13].initButton(&tft, 24, 85, 30, 43,
                          ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, "-", 3);
    buttons[13].drawButton();

    buttons[14].initButton(&tft, 165, 85, 30, 43,
                          ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, "+", 3);
    buttons[14].drawButton();
}

void drawSetHeatScreen() {
    tft.fillScreen(ILI9341_BLACK); //Set Background Color with BLACK

    drawBorder();
    drawSetHeatScreenBtns(buttons);

    tft.setTextColor (ILI9341_LIGHTYELLOW, ILI9341_BLACK);

    tft.setCursor (6, 0);
    tft.setTextSize (1);
    tft.print("HEAT TIMER SETTING");

    buttons[0].initButton(&tft, 214, 25, 44, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "Mo", 2);
    buttons[0].drawButton();

    buttons[1].initButton(&tft, 214, 63, 44, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "Tu", 2);
    buttons[1].drawButton();

    buttons[2].initButton(&tft, 214, 101, 44, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "We", 2);
    buttons[2].drawButton();

    buttons[3].initButton(&tft, 214, 139, 44, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "Th", 2);
    buttons[3].drawButton();

    buttons[4].initButton(&tft, 214, 177, 44, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "Fr", 2);
    buttons[4].drawButton();

    buttons[5].initButton(&tft, 214, 215, 44, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "Sa", 2);
    buttons[5].drawButton();

    buttons[6].initButton(&tft, 214, 253, 44, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "Su", 2);
    buttons[6].drawButton();

    buttons[7].initButton(&tft, 33, 297, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "Wod", 2);
    //buttons[7].drawButton();

    buttons[8].initButton(&tft, 90, 297, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "WeD", 2);
    //buttons[8].drawButton();

    buttons[9].initButton(&tft, 34, 297, 60, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLUE, ILI9341_BLACK, "HOME", 2);
    buttons[9].drawButton();

    buttons[10].initButton(&tft, 207, 297, 60, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_RED, ILI9341_WHITE, "SAVE", 2);
    buttons[10].drawButton();
}

void drawSetDateAndTime() {
    char str[11];

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

    tft.setTextSize (3);
    tft.setCursor (60, 72);
    sprintf(str, "%02d", newTime.hour);
    tft.print(str);
    tft.setCursor (100, 72);
    tft.print(":");
    sprintf(str, "%02d", newTime.minute);
    tft.setCursor (133, 72);
    tft.print(str);

    sprintf(str, "%02d.%02d.%04d", newTime.day, newTime.month, newTime.year),
    tft.setTextSize (3);
    tft.setCursor (40, 170);
    tft.print(str);
}

void drawSetTimeScreen() {
    tft.fillScreen(ILI9341_BLACK); //Set Background Color with BLACK

    drawBorder();

    tft.setTextColor(ILI9341_LIGHTYELLOW, ILI9341_BLACK);

    tft.setCursor (6, 0);
    tft.setTextSize (1);
    tft.print(" TIME SETTING ");

    drawSetDateAndTime();

    buttons[0].initButton(&tft, 75, 45, 50, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_WHITE, "+", 2);
    buttons[0].drawButton();

    buttons[1].initButton(&tft, 150, 45, 50, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_WHITE, "+", 2);
    buttons[1].drawButton();


    buttons[4].initButton(&tft, 55, 130, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_WHITE, "+", 2);
    buttons[4].drawButton();

    buttons[5].initButton(&tft, 118, 130, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_WHITE, "+", 2);
    buttons[5].drawButton();

    buttons[6].initButton(&tft, 181, 130, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_WHITE, "+", 2);
    buttons[6].drawButton();


    buttons[10].initButton(&tft, 207, 297, 60, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_RED, ILI9341_WHITE, "SAVE", 2);
    buttons[10].drawButton();

    buttons[11].initButton(&tft, 34, 297, 60, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLUE, ILI9341_BLACK, "HOME", 2);
    buttons[11].drawButton();
}

void drawHome() {

    tft.fillScreen(ILI9341_BLACK); //Set Background Color with BLACK

    //Design Interface (lines)
    drawBorder();

    tft.writeLine(148, 3,   148, 80,  ILI9341_WHITE);
    tft.writeLine(158, 152, 158, 220, ILI9341_WHITE);

    tft.writeLine(1, 80, 240, 80,   ILI9341_WHITE);
    tft.writeLine(1, 152, 240, 152, ILI9341_WHITE);
    tft.writeLine(1, 220, 240, 220, ILI9341_WHITE);

    tft.setTextColor (ILI9341_LIGHTYELLOW, ILI9341_BLACK);
    tft.setTextSize (1);

    tft.setCursor(6, 0);
    tft.print(" INSIDE TEMPERATURE ");

    tft.setCursor (152, 0);
    tft.print (" HUMIDITY ");

    tft.setCursor(6, 76);
    tft.print(" REQUIRED TEMPERATURE ");

    tft.setCursor (6, 150);
    tft.print (" OUTSIDE TEMPERATURE ");

    tft.setCursor (165, 150);//heat index fahrenheit
    tft.print (" HEAT INDEX ");
}

void returnHome() {
    page = PAGE_HOME;
    drawHome();
}

void timer() {
    returnHomeCounter++;
    temperatureCounter++;
    homeReadIntervalCounter++;

    // return home
    if (returnHomeCounter > 100) {
        returnHomeCounter = 0;
        if (page != PAGE_HOME) {
            //returnHome();
        }
    }
}

void setup() {
    //Serial.begin(115200);

    //delay(1000);

    pinMode(1, FUNCTION_3);
    //pinMode(3, FUNCTION_3);

    pinMode(10, OUTPUT);
    pinMode(1, OUTPUT);
    digitalWrite(1, LOW);

    SPI.setFrequency(ESP_SPI_FREQ);

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);
    tft.setRotation(2);

    touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
    //touch.setCalibration(192, 1792, 1856, 228);
    //touch.setCalibration(209, 1759, 1800, 273);
    touch.setCalibration(193, 1800, 1898, 224);

    //touch.setCalibration(209, 1759, 1775, 273);

    rtc.begin();
    dht.begin();

    timer0.attach(0.1, timer);

    //page = PAGE_SET_TIME;
    //drawSetTimeScreen();
    //drawSetHeatScreen();
    //page = PAGE_SET_HEAT;
    drawHome();
    page = PAGE_HOME;
    //page = PAGE_SETTING;
    //drawSettingScreen();
}

const char* strDayOfTheWeek(int num) {
    switch(num) {
    case 2:
        return "Monday";
    break;
    case 3:
        return "Tuesday";
    break;
    case 4:
        return "Wednesday";
    break;
    case 5:
        return "Thursday";
    break;
    case 6:
        return "Friday";
    break;
    case 7:
        return "Saturday";
    break;
    case 1:
        return "Sunday";
    break;
    default:
        return 0;
    }
}

void drawTime() {
    now = rtc.now();
    char strTime[9];

    tft.setTextSize(1);
    tft.setCursor(10, 253);
    tft.print("IP:");
    tft.print(WiFi.localIP());

    tft.setTextSize(2);

    tft.setCursor(10, 230);
    sprintf(strTime, "%s", strDayOfTheWeek(now.dayOfTheWeek()));
    tft.print(strTime);

    tft.setCursor(138, 230);
    sprintf(strTime, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    tft.print(strTime);

    tft.setCursor(115, 253);
    sprintf(strTime, "%02d.%02d.%04d", now.day(), now.month(), now.year());
    tft.print(strTime);
    tft.setFont();
}

void updateHeatRow(int index) {
    unsigned int heatRowPos = heatRowBasePos + (25 * (index + 1));
    char str[16];

    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor (8, heatRowPos);
    sprintf(str, "%d. %02d:%02d %.1f", index + 1,
                actualHeatValues.hour[index],
                actualHeatValues.minute[index],
                actualHeatValues.temperature[index]);
    tft.print(str);
}

void drawHeatValues() {
    int heatRowPos = heatRowBasePos;

    char str[16];
    tft.setTextSize (2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    for (int i = 0; i < 6; i++) {
        updateHeatRow(i);
    }

}

bool blink = false;;

void printBuffer() {
                    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
                    tft.setTextSize(2);
                    tft.setCursor(6, 22 + 32 * wifiConfig.line);
                    tft.print(textBuffer);
}


void loop() {
    //analogWrite(10, 512);
    uint16_t touchX = 0, touchY = 0;
    char str[16];
    bool r = false;

    if (temperatureCounter > TEMP_READ_INTERVAL) {
        temperature.readTemperature();
        temperatureCounter = 0;
    }

    switch (page) {
    case PAGE_SET_HEAT:
    {

        if (touch.isTouching()) {
            touch.getPosition(touchX, touchY);
            returnHomeCounter = 0;

            // save and home
            if (buttons[9].contains(touchX, touchY)) {
                returnHome();
                break;
            } else
            if (buttons[10].contains(touchX, touchY)) {
                actualHeatValues.save();
                returnHome();
                break;
            } else
            if (buttons[11].contains(touchX, touchY)) {
                actualHeatValues.minute[hourHeatValueIndex] -= 10;
                if (actualHeatValues.minute[hourHeatValueIndex] < 0) {
                    actualHeatValues.minute[hourHeatValueIndex] = 50;
                    actualHeatValues.hour[hourHeatValueIndex] -= 1;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[12].contains(touchX, touchY)) {
                actualHeatValues.minute[hourHeatValueIndex] += 10;
                if (actualHeatValues.minute[hourHeatValueIndex] > 59) {
                    actualHeatValues.minute[hourHeatValueIndex] = 0;
                    actualHeatValues.hour[hourHeatValueIndex] += 1;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[13].contains(touchX, touchY)) {
                actualHeatValues.temperature[hourHeatValueIndex] -= 0.2;
                if (actualHeatValues.temperature[hourHeatValueIndex] < 10) {
                    actualHeatValues.temperature[hourHeatValueIndex] = 35;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[14].contains(touchX, touchY)) {
                actualHeatValues.temperature[hourHeatValueIndex] += 0.2;
                if (actualHeatValues.temperature[hourHeatValueIndex] > 35) {
                    actualHeatValues.temperature[hourHeatValueIndex] = 10;
                }
                updateHeatRow(hourHeatValueIndex);
            }

            // Mo to Su
            for (int i = 0; i < 7; i++) {
                if (buttons[i].contains(touchX, touchY)) {
                    for (int j = 0; j < 7; j++) {
                        buttons[j].drawButton();
                    }
                    actualHeatValues.save();
                    buttons[i].drawButton(true);
                    actualHeatValues.flag = i + 1;
                    actualHeatValues.load();
                    drawHeatValues();
                    break;
                }
            }

            // for all buttons
            int pos = 141;
            for (int i = 0; i < 7; i++) {
                // if one was touched
                if (touchX > 6 && touchX < 180 && touchY < pos - 1 && touchY > 125) {
                    // delete all previous
                    int pos1 = 141;
                    for (int j = 0; j < 7; j++) {
                        tft.drawLine(8, pos1 - 25, 170, pos1 - 25, ILI9341_BLACK);
                        pos1 = pos1 + 25;
                    }
                    // and draw new line
                    hourHeatValueIndex = i;
                    tft.drawLine(8, pos, 170, pos, ILI9341_WHITE);
                    // do not continue
                    break;
                }
                pos = pos + 25;
            }

        } // end isTouching()

        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        tft.setTextSize(3);
        tft.setCursor (45, 30);
        sprintf(str, "%02d:%02d",
                actualHeatValues.hour[hourHeatValueIndex],
                actualHeatValues.minute[hourHeatValueIndex]);
        tft.print(str);

        tft.setCursor (55, 75);
        sprintf(str, "%.1f",
                actualHeatValues.temperature[hourHeatValueIndex]);
        tft.print(str);
    }
    break;

    case PAGE_HOME:

        if (homeReadIntervalCounter > HOME_READ_INTERVAL) {
            homeReadIntervalCounter = 0;

            char strTime[11];

            if (touch.isTouching()) {
                touch.getPosition(touchX, touchY);

                if (manually) {
                    if (touchX > 8 && touchY > 80 && touchX < 48 && touchY < 170) {
                        requiredTemp -= HYSTERESIS;
                    } else
                    if (touchX > 190 && touchY > 80 && touchX < 230 && touchY < 170) {
                        requiredTemp += HYSTERESIS;
                    }
                }
                if (touchX > 60 && touchX < 170 && touchY > 80 && touchY < 170) {
                    page = PAGE_SET_HEAT;
                    drawSetHeatScreen();
                    break;
                } else
                if (touchX > 115 && touchX < 240 && touchY > 220 && touchY < 275) {
                    newTime.day    = now.day();
                    newTime.month  = now.month();
                    newTime.year   = now.year();
                    newTime.hour   = now.hour();
                    newTime.minute = now.minute();
                    newTime.second = 0;

                    page = PAGE_SET_TIME;
                    drawSetTimeScreen();
                    break;
                } else
                if (touchX > 100 && touchX < 135 && touchY > 280 && touchY < 315) {
                    manually = !manually;
                } else
                if (touchX > 200 && touchX < 235 && touchY > 280 && touchY < 315) {
                    page = PAGE_SETTING;
                    drawSettingScreen();
                    break;
                }

            } else {

                tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
                tft.setTextSize(5);
                tft.setCursor(15, 24);
                tft.print(temperature.temperatureC, 1);

                tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
                tft.setTextSize(4);
                tft.setCursor(160, 27);
                tft.print(int(temperature.humidity), 1);
                tft.print("%");

                tft.setTextSize(5);
                tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
                tft.setCursor(7, 170);
                tft.print(" --.-");

                tft.setTextSize(4);
                tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
                tft.setCursor(175, 175);
                tft.print(temperature.heatIndex, 1);

                drawTime();

                if (requiredTemp > temperature.temperatureC + HYSTERESIS) {
                    digitalWrite(1, HIGH);
                    tft.drawBitmap(10, 280, 35, 35, flame, ILI9341_RED);
                } else
                if (requiredTemp <= temperature.temperatureC) {
                    digitalWrite(1, LOW);
                    tft.drawBitmap(10, 280, 35, 35, flame, ILI9341_BLUE);
                }

                if (holliday) {
                    tft.drawBitmap(55, 280, 35, 35, plane, ILI9341_RED);
                } else {
                    tft.drawBitmap(55, 280, 35, 35, plane, ILI9341_BLUE);
                }

                if (manually) {
                    tft.drawBitmap(100, 280, 35, 35, manual, ILI9341_BLUE);
                } else {
                    //requiredTemp =
                    tft.drawBitmap(100, 280, 35, 35, automatic, ILI9341_RED);
                }

                if (WiFi.status() == WL_CONNECTED) {
                    tft.drawBitmap(145, 280, 35, 35, wifi, ILI9341_BLUE);
                }
                tft.drawBitmap(200, 280, 35, 35, settings, ILI9341_BLUE);
            }

            tft.setCursor(60, 100);
            tft.setTextSize(5);
            sprintf(strTime, "%.1f", requiredTemp);
            tft.print(strTime);

            tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
            tft.setTextSize(6);
            if (manually) {
                tft.setCursor(8, 98);
                tft.print("-");
                tft.setCursor(190, 98);
                tft.print("+");
            } else {
                tft.setCursor(8, 98);
                tft.print(" ");
                tft.setCursor(190, 98);
                tft.print(" ");
            }
        }

        break;

    case PAGE_SET_TIME:

        if (touch.isTouching()) {
            touch.getPosition(touchX, touchY);

            // save and home
            if (buttons[0].contains(touchX, touchY)) {
                newTime.hour > 22 ? newTime.hour = 0 : newTime.hour++;
            } else
            if (buttons[1].contains(touchX, touchY)) {
                newTime.minute > 58 ? newTime.minute = 0 : newTime.minute++;
            } else
            if (buttons[4].contains(touchX, touchY)) {
                newTime.day > 30 ? newTime.day = 1 : newTime.day++;
            } else
            if (buttons[5].contains(touchX, touchY)) {
                newTime.month > 11 ? newTime.month = 1 : newTime.month++;
            } else
            if (buttons[6].contains(touchX, touchY)) {
                newTime.year > 2040 ? newTime.year = 2018 : newTime.year++;
            }
            drawSetDateAndTime();

            if (buttons[10].contains(touchX, touchY)) {
                rtc.adjust(newTime);
                returnHome();
            } else
            if (buttons[11].contains(touchX, touchY)) {
                returnHome();
            }
            delay(75);
        }

        break;
    case PAGE_SETTING:
        if (homeReadIntervalCounter > 1) {
            homeReadIntervalCounter = 0;

            if (touch.isTouching()) {
                touch.getPosition(touchX, touchY);
                //touch.getRaw(touchX, touchY);

                //LOG("%03d, %03d", touchX, touchY);

                if (buttons[9].contains(touchX, touchY)) {
                    returnHome();
                    break;
                } else
                if (buttons[10].contains(touchX, touchY)) {
                    returnHome();
                    break;
                } else
                if (buttons[8].contains(touchX, touchY)) {
                    wifiConnect();
                    break;
                }

                char ch;
                r = getKeyPress(ch, touchX, touchY);
                if (r) { // if char was pressed
                    textBuffer[textPosition++] = ch;
                    //tft.writeLine(6, 38, 232, 38, ILI9341_WHITE);
                    printBuffer();
                    if (textPosition == wifiConfig.arrLength[wifiConfig.line]) {
                        textPosition = 0;
                    }
                    break;
                } else {
                    // backspace was pressed
                    if (ch == 0x8 && textPosition > 0) {
                        textBuffer[textPosition - 1] = ' ';
                        textPosition--;
                        //removeSpaces(textBuffer);
                        printBuffer();
                        break;
                    } else

                    if (ch == 0x12) { // enter was pressed
                        wifiConfig.text[wifiConfig.line] = textBuffer;
                        wifiConfig.line++;
                        textPosition = 0;

                        if (wifiConfig.line > 4) {
                            wifiConfig.line = 0;
                        }
                        textBuffer = wifiConfig.text[wifiConfig.line];
                        break;
                    }
                }
            }

            if (blink) {
                tft.writeLine(6 + (12 * textPosition), 6 + 32 * (wifiConfig.line + 1),
                              18 + (12 * textPosition), 6 + 32 * (wifiConfig.line + 1), ILI9341_WHITE);
            } else {
                tft.writeLine(6 + (12 * textPosition), 6 + 32 * (wifiConfig.line + 1),
                              18 + (12 * textPosition), 6 + 32 * (wifiConfig.line + 1), ILI9341_BLACK);
            }
            blink = !blink;
        }

        break;
    }

    dbglog(touchX, touchY, wifiConfig.line, r);

}
