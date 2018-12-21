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

volatile int page;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ MOSI, /*irq=*/ 16);
DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;
Adafruit_GFX_Button buttons[15];
HeatValues actualHeatValues;
Temperature temperature(dht);
Ticker timer0;

long previousMillis = 0;
bool holliday = false;
bool manually = false;

float requiredTemp = 23.0;
float setHeatTemp = 20.0;
int setHeatHour, setHeatMinute = 0;
int hourHeatValueIndex = 0;
int selectedButton = 0;
int heatRowBasePos = 100;
unsigned int returnHomeCounter0 = 0;
unsigned int timerCounter0 = 0;

void wifiConnect() {

    // Connect to WiFi network
    Serial.println("");
    Serial.print("Connecting to: ");
    Serial.print(WIFI_SSID);
    wifiIp.fromString(WIFIIP);
    wifiGateway.fromString(WIFIGATEWAY);
    wifiSubnet.fromString(WIFISUBNET);

    WiFi.hostname(DEVICENAME);
    WiFi.config(wifiIp, wifiGateway, wifiSubnet);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    //ESPserial.print("WiFi connected: ");
    //ESPserial.print("http://");
    Serial.print(WiFi.localIP().toString());
    //ESPserial.println("/");
}

void dbglog(int x, int y, int z) {
#ifdef DEBUG==1
            tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor (120, 0);
    tft.setTextSize (1);
    tft.print("x:");
    tft.print(x);
    tft.print(" y:");
    tft.print(y);
    tft.print(" z:");
    tft.print(z);
#endif
}

void drawBorder() {
    tft.writeLine(1, 3, 3, 3,       ILI9341_WHITE);
    tft.writeLine(120, 3, 240, 3,   ILI9341_WHITE);
    tft.writeLine(1, 3, 1, 320,     ILI9341_WHITE);
    tft.writeLine(1, 319,  240, 319,ILI9341_WHITE);
    tft.writeLine(239, 319, 239, 3, ILI9341_WHITE); // DARKGREY
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

    buttons[9].initButton(&tft, 147, 297, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "All", 2);
    //buttons[9].drawButton();

    buttons[10].initButton(&tft, 207, 297, 60, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_RED, ILI9341_WHITE, "SAVE", 2);
    buttons[10].drawButton();

}

void drawSetTimeScreen() {
    tft.fillScreen(ILI9341_BLACK); //Set Background Color with BLACK

    drawBorder();

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
    buttons[7].drawButton();

    buttons[8].initButton(&tft, 90, 297, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "WeD", 2);
    buttons[8].drawButton();

    buttons[9].initButton(&tft, 147, 297, 55, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_BLACK, ILI9341_RED, "All", 2);
    buttons[9].drawButton();

    buttons[10].initButton(&tft, 207, 297, 60, 37,
                          ILI9341_LIGHTYELLOW, ILI9341_RED, ILI9341_WHITE, "SAVE", 2);
    buttons[10].drawButton();
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
    actualHeatValues.save();
    drawHome();
}

void timer() {
    returnHomeCounter0++;
    // return home
    if (returnHomeCounter0 > 100) {
        returnHomeCounter0 = 0;
        if (page != PAGE_HOME) {
            returnHome();
        }
    }
    // reset home read interval
    if (timerCounter0 > HOMEREADINTERVAL) {
        timerCounter0 = 0;
    }
}

void setup() {
    Serial.begin(115200);
    wifiConnect();
    delay(1000);

    //pinMode(1, FUNCTION_3);
    //pinMode(3, FUNCTION_3);

    pinMode(10, OUTPUT);
    pinMode(1, OUTPUT);
    digitalWrite(1, LOW);

    SPI.setFrequency(ESP_SPI_FREQ);

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);
    tft.setRotation(2);

    touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
    touch.setCalibration(192, 1792, 1856, 228);

    rtc.begin();
    dht.begin();

    timer0.attach(0.1, timer);

    //drawSetHeatScreen();
    //page = PAGE_SET_HEAT;
    drawHome();
    page = PAGE_HOME;
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

void drawTime() {
    DateTime now = rtc.now();
    char strTime[9];

    tft.setTextSize(2);
    tft.setCursor(10, 230);
    sprintf(strTime, "%s", strDayOfTheWeek(now.dayOfTheWeek()));
    tft.print(strTime);

    tft.setCursor(125, 230);
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

void loop() {
    //analogWrite(10, 512);
    uint16_t x, y;
    unsigned long currentMillis = millis();
    char str[16];

    switch (page) {
    case PAGE_SET_HEAT:
    {
        if (touch.isTouching()) {
            touch.getPosition(x, y);
            returnHomeCounter0 = 0;

            // save and home
            if (buttons[10].contains(x, y)) {
                returnHome();
                break;
            } else
            if (buttons[11].contains(x, y)) {
                actualHeatValues.minute[hourHeatValueIndex] -= 10;
                if (actualHeatValues.minute[hourHeatValueIndex] < 0) {
                    actualHeatValues.minute[hourHeatValueIndex] = 50;
                    actualHeatValues.hour[hourHeatValueIndex] -= 1;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[12].contains(x, y)) {
                actualHeatValues.minute[hourHeatValueIndex] += 10;
                if (actualHeatValues.minute[hourHeatValueIndex] > 59) {
                    actualHeatValues.minute[hourHeatValueIndex] = 0;
                    actualHeatValues.hour[hourHeatValueIndex] += 1;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[13].contains(x, y)) {
                actualHeatValues.temperature[hourHeatValueIndex] -= 0.2;
                if (actualHeatValues.temperature[hourHeatValueIndex] < 10) {
                    actualHeatValues.temperature[hourHeatValueIndex] = 35;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[14].contains(x, y)) {
                actualHeatValues.temperature[hourHeatValueIndex] += 0.2;
                if (actualHeatValues.temperature[hourHeatValueIndex] > 35) {
                    actualHeatValues.temperature[hourHeatValueIndex] = 10;
                }
                updateHeatRow(hourHeatValueIndex);
            }

            // Mo to Su
            for (int i = 0; i < 7; i++) {
                if (buttons[i].contains(x, y)) {
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
                if (x > 6 && x < 180 && y < pos - 1 && y > 125) {
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

            dbglog(hourHeatValueIndex, actualHeatValues.flag, 1);
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

        timerCounter0++;
        if (timerCounter0 > HOMEREADINTERVAL) {
            temperature.readTemperature();

            char strTime[11];

            if (touch.isTouching()) {
                touch.getPosition(x, y);
                if (manually) {
                    if (x > 8 && y > 80 && x < 48 && y < 170) {
                        requiredTemp -= 0.2;
                    } else
                    if (x > 190 && y > 80 && x < 230 && y < 170) {
                        requiredTemp += 0.2;
                    }
                }
                if (x > 60 && x < 170 && y > 80 && y < 170) {
                    page = PAGE_SET_HEAT;
                    drawSetHeatScreen();
                    break;
                } else
                if (x > 90 && x < 160 && y > 240) {
                    manually = !manually;
                }

            } else {


                if (holliday) {
                    tft.drawBitmap(55, 280, 35, 35, plane, ILI9341_RED);
                } else {
                    tft.drawBitmap(55, 280, 35, 35, plane, ILI9341_NAVY);
                }

                tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
                tft.setTextSize(5);
                tft.setCursor(13, 24);
                tft.print(temperature.temperatureC, 1);

                tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
                tft.setTextSize(4);
                tft.setCursor(165, 25);
                tft.print(temperature.humidity, 1);

                tft.setTextSize(5);
                tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
                tft.setCursor(7, 170);
                tft.print(" --.-");

                tft.setTextSize(4);
                tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
                tft.setCursor(175, 175);
                tft.print(temperature.heatIndex, 1);

                drawTime();

                if (manually) {
                    tft.drawBitmap(100, 280, 35, 35, automatic, ILI9341_NAVY);
                } else {
                    tft.drawBitmap(100, 280, 35, 35, automatic, ILI9341_RED);
                }
                if (manually) {
                    tft.drawBitmap(155, 280, 35, 35, manual, ILI9341_RED);
                    //requiredTemp =
                } else {
                    tft.drawBitmap(155, 280, 35, 35, manual, ILI9341_NAVY);
                }


                if (requiredTemp > temperature.temperatureC + HYSTERESIS) {
                    digitalWrite(1, HIGH);
                    tft.drawBitmap(10, 280, 35, 35, fire, ILI9341_RED);
                } else
                if (requiredTemp <= temperature.temperatureC) {
                    digitalWrite(1, LOW);
                    tft.drawBitmap(10, 280, 35, 35, fire, ILI9341_NAVY);
                }
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
        if (currentMillis - previousMillis > HOMEREADINTERVAL) {
            previousMillis = currentMillis;
        }
        break;

    }

}
