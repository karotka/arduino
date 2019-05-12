#include <SPI.h>
#include <Wire.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <XPT2046_Touchscreen.h>
#include "DHT.h"
#include "RTClib.h"
#include "heatvalues.h"
#include "configure.h"
#include "images.h"
#include "keyboard.h"

volatile int page;

XPT2046_Touchscreen ts(T_CS, T_IRQ);

DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;
Adafruit_GFX_Button buttons[15];
HeatValues heatValues;
HeatValues actualDayHeatValues;
WifiConfig wifiConfig;
Ticker timer0;
NewTime newTime;
DateTime now;

volatile unsigned int returnHomeCounter = 0;
volatile unsigned int temperatureCounter = 0;
volatile unsigned int readCounter = 0;
volatile unsigned int sleepCounter = 0;

boolean wake = false;
boolean holliday = false;
boolean manually = false;

float requiredTemp = 23.0;
float setHeatTemp = 20.0;

float temperatureC = 0;
float humidity = 0;
float heatIndex = 0;

int setHeatHour, setHeatMinute = 0;
int hourHeatValueIndex = 0;
int selectedButton = 0;
int heatRowBasePos = 100;

int textPosition = 0;
char *textBuffer;


void checkHeat() {
    now = rtc.now();
    temperatureC = dht.readTemperature();

    int pressedTime, nextPressedTime, actualTime;
    float pressedTemp;
    actualTime = now.minute() + (now.hour() * 60);

    int dow = now.dayOfTheWeek() - 1 == 0 ? 7 : now.dayOfTheWeek() - 1;
    actualDayHeatValues.flag = dow;
    actualDayHeatValues.load();

#ifdef DEBUG
    printf("ActualT:%d, DOW:%d \n", actualTime, dow );
#endif

    for(uint8_t i = 0; i < 6; i++) {
        uint8_t j;
        i == 5 ? j = 0 : j = i + 1;

        pressedTime = actualDayHeatValues.minute[i] + (actualDayHeatValues.hour[i] * 60);
        nextPressedTime = actualDayHeatValues.minute[j] +
            (actualDayHeatValues.hour[j] * 60) + (j == 0 ? 1400 : 0);
        pressedTemp = actualDayHeatValues.temperature[i];

        if (pressedTime <= actualTime &&
            actualTime < nextPressedTime) {
            requiredTemp = pressedTemp;
            if (pressedTemp > temperatureC) {
#ifdef DEBUG
                printf("PressedTime:%d NextPressedTime:%d PressedTemp:%f \n",
                    pressedTime, nextPressedTime, pressedTemp);
#endif
                digitalWrite(2, HIGH);
            }
            break;
        } else {
            digitalWrite(2, LOW);
        }
    }
}

void deepSleep() {
    printf("I'm going to deep sleep ... \n");
    esp_sleep_enable_timer_wakeup(10000 * DEEP_SLEEP_MS);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);
    mySleep(1000);
    esp_deep_sleep_start();
}

void wakeupReason() {
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

    switch(wakeupReason) {
    case ESP_SLEEP_WAKEUP_EXT0:
        page = PAGE_HOME;
        drawHome();
        printf("Wakeup caused by external signal using RTC_IO \n");
    break;
    case ESP_SLEEP_WAKEUP_TIMER:
        ledcWrite(PWM_CHA1, 0);
        printf("Wakeup caused by timer \n");
        checkHeat();
        deepSleep();
    break;
    }
}

void dimmDisplay() {
    int v0 = analogRead(ADC_PIN0);
    ledcWrite(PWM_CHA1, 4096 - v0);
#ifdef DEBUG
    printf("Temp: %.2f, PWM dimmer: %d \n", temperatureC, v0);
#endif
}

boolean isTouched(uint16_t *x, uint16_t *y) {
    boolean touched = ts.touched();
    if (touched) {
        TS_Point p = ts.getPoint();
#ifdef DEBUG
        //printf("Raw> X:%d, Y:%d, Z:%d Ln:%d\n", p.x, p.y, p.z, __LINE__);
#endif
        if (p.y > coords[0] || p.y < coords[1]) return false;
        //|| p.y > coords[1] ||
        //    p.x < coords[3] || p.x > coords[2]) {
        //     return false;
        //}
        *x = map(p.y, coords[0], coords[1], 0, tft.width());
        *y = map(p.x, coords[3], coords[2], 0, tft.height());

#ifdef DEBUG
        tft.drawPixel(*x, *y, WHITE);
        printf("Real> X:%d, Y:%d, Z:%d Ln:%d\n", *x, *y, p.z, __LINE__);
#endif

        if (p.z > 100 && p.z < 2500) {
            if (sleepCounter > BLACK_SCREEN_MS) {
                tft.sleep(false);
                dimmDisplay();
                mySleep(1000);
                sleepCounter = 0;
                return false;
            }
            sleepCounter = 0;
            return true;
        } else return false;
    }
}

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
        //delay(250);
        if (status) {
            tft.drawBitmap(138, 165, 35, 35, wifi, BLUE);
        } else {
            tft.drawBitmap(138, 165, 35, 35, wifi, NAVY);
        }
        status = !status;
    }
    tft.drawBitmap(138, 165, 35, 35, wifi, BLUE);
}

void drawBorder() {
    tft.drawLine(0, 3, 240, 3,     DARKGREY);
    tft.drawLine(0, 3, 0, 320,     DARKGREY);
    tft.drawLine(0, 319,  240, 319,DARKGREY);
    tft.drawLine(239, 319, 239, 3, DARKGREY); // DARKGREY
}

void drawSettingScreen() {
    tft.fillScreen(BACK_COLOR); //Set Background Color with BLACK

    drawBorder();

    tft.setTextColor (YELLOW, BLACK);

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
        tft.drawLine(6, i, 232, i, DARKGREY);
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
                          YELLOW, BLUE, BLACK, (char*)"CON", 2);
    buttons[8].drawButton();

    buttons[9].initButton(&tft, 34, 183, 60, 27,
                          YELLOW, BLUE, BLACK, (char*)"BACK", 2);
    buttons[9].drawButton();

    buttons[10].initButton(&tft, 207, 183, 60, 27,
                          YELLOW, RED, WHITE, (char*)"SAVE", 2);
    buttons[10].drawButton();

    MakeKB_Button(KB);
}

void drawSetHeatScreenBtns(Adafruit_GFX_Button buttons[]) {
    buttons[11].initButton(&tft, 24, 37, 30, 43,
                          WHITE, BLACK, WHITE, (char*)"-", 3);
    buttons[11].drawButton();

    buttons[12].initButton(&tft, 165, 37, 30, 43,
                          WHITE, BLACK, WHITE, (char*)"+", 3);
    buttons[12].drawButton();

    buttons[13].initButton(&tft, 24, 85, 30, 43,
                          WHITE, BLACK, WHITE, (char*)"-", 3);
    buttons[13].drawButton();

    buttons[14].initButton(&tft, 165, 85, 30, 43,
                          WHITE, BLACK, WHITE, (char*)"+", 3);
    buttons[14].drawButton();
}

void drawSetHeatScreen() {

    tft.fillScreen(BACK_COLOR); //Set Background Color with BLACK

    drawBorder();
    drawSetHeatScreenBtns(buttons);

    tft.setTextColor (YELLOW, BLACK);

    tft.setCursor (6, 0);
    tft.setTextSize (1);
    tft.print("HEAT TIMER SETTING");

    buttons[0].initButton(&tft, 214, 25, 44, 37,
                          YELLOW, BLACK, RED, (char*)"Mo", 2);
    buttons[0].drawButton();

    buttons[1].initButton(&tft, 214, 63, 44, 37,
                          YELLOW, BLACK, RED, (char*)"Tu", 2);
    buttons[1].drawButton();

    buttons[2].initButton(&tft, 214, 101, 44, 37,
                          YELLOW, BLACK, RED, (char*)"We", 2);
    buttons[2].drawButton();

    buttons[3].initButton(&tft, 214, 139, 44, 37,
                          YELLOW, BLACK, RED, (char*)"Th", 2);
    buttons[3].drawButton();

    buttons[4].initButton(&tft, 214, 177, 44, 37,
                          YELLOW, BLACK, RED, (char*)"Fr", 2);
    buttons[4].drawButton();

    buttons[5].initButton(&tft, 214, 215, 44, 37,
                          YELLOW, BLACK, RED, (char*)"Sa", 2);
    buttons[5].drawButton();

    buttons[6].initButton(&tft, 214, 253, 44, 37,
                          YELLOW, BLACK, RED, (char*)"Su", 2);
    buttons[6].drawButton();

    buttons[7].initButton(&tft, 33, 297, 55, 37,
                          YELLOW, BLACK, RED, (char*)"Wod", 2);
    //buttons[7].drawButton();

    buttons[8].initButton(&tft, 90, 297, 55, 37,
                          YELLOW, BLACK, RED, (char*)"WeD", 2);
    //buttons[8].drawButton();

    buttons[9].initButton(&tft, 34, 297, 60, 37,
                          YELLOW, BLUE, BLACK, (char*)"HOME", 2);
    buttons[9].drawButton();

    buttons[10].initButton(&tft, 207, 297, 60, 37,
                          YELLOW, RED, WHITE, (char*)"SAVE", 2);
    buttons[10].drawButton();

    buttons[hourHeatValueIndex].drawButton(true);
    heatValues.flag = hourHeatValueIndex + 1;
    heatValues.load();
    drawHeatValues();
}

void drawSetDateAndTime() {
    char str[11];

    tft.setTextColor(WHITE, BLACK);

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
    tft.fillScreen(BACK_COLOR); //Set Background Color with BLACK

    drawBorder();

    tft.setTextColor(YELLOW, BLACK);

    tft.setCursor (6, 0);
    tft.setTextSize (1);
    tft.print(" TIME SETTING ");

    drawSetDateAndTime();

    buttons[0].initButton(&tft, 75, 45, 50, 37,
                          YELLOW, BLACK, WHITE, (char*)"+", 2);
    buttons[0].drawButton();

    buttons[1].initButton(&tft, 150, 45, 50, 37,
                          YELLOW, BLACK, WHITE, (char*)"+", 2);
    buttons[1].drawButton();


    buttons[4].initButton(&tft, 55, 130, 55, 37,
                          YELLOW, BLACK, WHITE, (char*)"+", 2);
    buttons[4].drawButton();

    buttons[5].initButton(&tft, 118, 130, 55, 37,
                          YELLOW, BLACK, WHITE, (char*)"+", 2);
    buttons[5].drawButton();

    buttons[6].initButton(&tft, 181, 130, 55, 37,
                          YELLOW, BLACK, WHITE, (char*)"+", 2);
    buttons[6].drawButton();


    buttons[10].initButton(&tft, 207, 297, 60, 37,
                           YELLOW, RED, WHITE, (char*)"SAVE", 2);
    buttons[10].drawButton();

    buttons[11].initButton(&tft, 34, 297, 60, 37,
                          YELLOW, BLUE, BLACK, (char*)"HOME", 2);
    buttons[11].drawButton();
}

void drawHome() {

    tft.fillScreen(BACK_COLOR); //Set Background Color with BLACK

    //Design Interface (lines)
    drawBorder();

    tft.drawLine(148, 3,   148, 80,  DARKGREY);
    tft.drawLine(158, 152, 158, 220, DARKGREY);

    tft.drawLine(1, 80, 240, 80,   DARKGREY);
    tft.drawLine(1, 152, 240, 152, DARKGREY);
    tft.drawLine(1, 220, 240, 220, DARKGREY);

    tft.setTextColor (YELLOW, BLACK);
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

void myTimer0() {
    returnHomeCounter++;
    temperatureCounter++;
    readCounter++;

    if (sleepCounter < BLACK_SCREEN_MS + 2) {
        sleepCounter++;
    }

    // return home
    if (returnHomeCounter > 1000) {
        returnHomeCounter = 0;
        if (page != PAGE_HOME) {
            returnHome();
        }
    }
}

void setup() {
#ifdef DEBUG
    //Serial.begin(115200);
#endif
    Serial.begin(115200);

    dht.begin();

    tft.begin();
    tft.fillScreen(BACK_COLOR);
    tft.setRotation(2);

    ts.begin();  // Must be done before setting rotation
    rtc.begin();

    ledcSetup(PWM_CHA1, PWM_FREQ, PWM_RES);
    ledcAttachPin(PWM_PIN0, 0);

    pinMode(2, OUTPUT);

    wakeupReason();

    timer0.attach(0.01, myTimer0);

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
    char strTime[11];

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
    tft.setTextColor(WHITE, BLACK);
    tft.setCursor (8, heatRowPos);

    sprintf(str, "%d. %02d:%02d %.1f", index + 1,
                heatValues.hour[index],
                heatValues.minute[index],
                heatValues.temperature[index]);
    tft.print(str);
}

void drawHeatValues() {
    int heatRowPos = heatRowBasePos;

    char str[16];
    tft.setTextSize (2);
    tft.setTextColor(WHITE, BLACK);
    for (int i = 0; i < 6; i++) {
        updateHeatRow(i);
    }
}

bool blink = false;;

void printBuffer() {
    tft.setTextColor(WHITE, BLACK);
    tft.fillRect(5, 18 + 32 * wifiConfig.line, 230, 20, BLACK);
    tft.setTextSize(2);
    tft.setCursor(6, 22 + 32 * wifiConfig.line);
    tft.print(textBuffer);
}

void loop() {

    uint16_t touchX, touchY = 0;
    char str[16];
    bool r = false;

    if (temperatureCounter > TEMP_READ_INTERVAL) {
        temperatureCounter = 0;

        temperatureC = dht.readTemperature();
        humidity = dht.readHumidity();
        heatIndex = dht.computeHeatIndex(temperatureC, humidity, false);

        if (!manually) {
            checkHeat();
        }

        if (sleepCounter > BLACK_SCREEN_MS) {
            ledcWrite(PWM_CHA1, 0);
            //tft.sleep(true);
            deepSleep();
        } else {
            dimmDisplay();
        }

#ifdef DEBUG
        printf("Sleep conter: %d \n", sleepCounter);
#endif

    }

    switch (page) {
    case PAGE_SET_HEAT:
    {
        if (readCounter < 8) break;
        readCounter = 0;

        if (isTouched(&touchX, &touchY)) {
            returnHomeCounter = 0;

            // save and home
            if (buttons[9].contains(touchX, touchY)) {
                returnHome();
                break;
            } else
            if (buttons[10].contains(touchX, touchY)) {
                heatValues.save();
                returnHome();
                break;
            } else
            if (buttons[11].contains(touchX, touchY)) {
                heatValues.minute[hourHeatValueIndex] -= 10;
                if (heatValues.minute[hourHeatValueIndex] < 0) {
                    heatValues.minute[hourHeatValueIndex] = 50;
                    heatValues.hour[hourHeatValueIndex] -= 1;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[12].contains(touchX, touchY)) {
                heatValues.minute[hourHeatValueIndex] += 10;
                if (heatValues.minute[hourHeatValueIndex] > 59) {
                    heatValues.minute[hourHeatValueIndex] = 0;
                    heatValues.hour[hourHeatValueIndex] += 1;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[13].contains(touchX, touchY)) {
                heatValues.temperature[hourHeatValueIndex] -= 0.2;
                if (heatValues.temperature[hourHeatValueIndex] < 10) {
                    heatValues.temperature[hourHeatValueIndex] = 35;
                }
                updateHeatRow(hourHeatValueIndex);
            } else
            if (buttons[14].contains(touchX, touchY)) {
                heatValues.temperature[hourHeatValueIndex] += 0.2;
                if (heatValues.temperature[hourHeatValueIndex] > 35) {
                    heatValues.temperature[hourHeatValueIndex] = 10;
                }
                updateHeatRow(hourHeatValueIndex);
            }

            // Mo to Su
            for (int i = 0; i < 7; i++) {
                if (buttons[i].contains(touchX, touchY)) {
                    for (int j = 0; j < 7; j++) {
                        buttons[j].drawButton();
                    }
                    heatValues.save();
                    buttons[i].drawButton(true);
                    heatValues.flag = i + 1;
                    heatValues.load();
                    drawHeatValues();
                    break;
                }
            }

            // for all buttons
            int pos = 141;
            for (int i = 0; i < 6; i++) {
                // if one was touched
                if (touchX > 6 && touchX < 180 && touchY < pos - 1 && touchY > 125) {
                    // delete all previous
                    int pos1 = 141;
                    for (int j = 0; j < 7; j++) {
                        tft.drawLine(8, pos1 - 25, 170, pos1 - 25, BLACK);
                        pos1 = pos1 + 25;
                    }
                    // and draw new line
                    hourHeatValueIndex = i;
                    tft.drawLine(8, pos, 170, pos, YELLOW);
                    // do not continue
                    break;
                }
                pos = pos + 25;
            }

        } // end isTouching()

        tft.setTextColor(WHITE, BLACK);
        tft.setTextSize(3);
        tft.setCursor (45, 30);
        sprintf(str, "%02d:%02d",
                heatValues.hour[hourHeatValueIndex],
                heatValues.minute[hourHeatValueIndex]);
        tft.print(str);

        tft.setCursor (55, 75);
        sprintf(str, "%.1f",
                heatValues.temperature[hourHeatValueIndex]);
        tft.print(str);
    }
    break;

    default:

        char strTime[11];

        if (isTouched(&touchX, &touchY)) {

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
            if (touchX > 104 && touchX < 140 && touchY > 280 && touchY < 315) {
                manually = !manually;
            } else
            if (touchX > 200 && touchX < 235 && touchY > 280 && touchY < 315) {
                page = PAGE_SETTING;
                drawSettingScreen();
                break;
            }
        }

        tft.setTextColor(RED, BLACK);
        tft.setTextSize(5);
        tft.setCursor(15, 24);
        tft.print(temperatureC, 1);

        tft.setTextColor(GREEN, BLACK);
        tft.setTextSize(4);
        tft.setCursor(160, 27);
        tft.print((int)humidity);
        tft.print("%");

        tft.setTextSize(5);
        tft.setTextColor(YELLOW, BLACK);
        tft.setCursor(7, 170);
        tft.print(" --.-");

        tft.setTextSize(4);
        tft.setTextColor(GREEN, BLACK);
        tft.setCursor(175, 175);
        tft.print((int)heatIndex);

        drawTime();

        if (manually) {
            if (requiredTemp > temperatureC + HYSTERESIS) {
                digitalWrite(2, HIGH);
                tft.drawBitmap(10, 280, 35, 35, flame, RED);
            } else
            if (manually && requiredTemp <= temperatureC) {
                digitalWrite(2, LOW);
                tft.drawBitmap(10, 280, 35, 35, flame, BLUE);
            }
        } else {
            if (digitalRead(2) == HIGH) {
                tft.drawBitmap(10, 280, 35, 35, flame, RED);
            } else {
                tft.drawBitmap(10, 280, 35, 35, flame, BLUE);
            }
        }

        if (holliday) {
            tft.drawBitmap(57, 280, 35, 35, plane, RED);
        } else {
            tft.drawBitmap(57, 280, 35, 35, plane, BLUE);
        }

        if (manually) {
            tft.drawBitmap(104, 280, 35, 35, manual, BLUE);
        } else {
            tft.drawBitmap(104, 280, 35, 35, automatic, RED);
        }

        if (WiFi.status() == WL_CONNECTED) {
            tft.drawBitmap(151, 280, 35, 35, wifi, RED);
        } else {
            tft.drawBitmap(151, 280, 35, 35, wifi, DARKGREY);
        }
        tft.drawBitmap(200, 280, 35, 35, settings, BLUE);

        tft.setCursor(60, 100);
        tft.setTextSize(5);
        sprintf(strTime, "%.1f", requiredTemp);
        tft.print(strTime);

        tft.setTextColor(WHITE, BLACK);
        tft.setTextSize(6);
        if (manually) {
            tft.setCursor(8, 98);
            tft.print("-");
            tft.setCursor(190, 98);
            tft.print("+");
        } else {
            tft.setCursor(190, 98);
            tft.print(" ");
            tft.drawBitmap(8, 98, 35, 35, lock, RED);
        }
    break;

    case PAGE_SET_TIME:

        if (isTouched(&touchX, &touchY)) {
            returnHomeCounter = 0;

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

        if (readCounter > 17) {
            readCounter = 0;

            if (isTouched(&touchX, &touchY)) {
                returnHomeCounter = 0;
#ifdef DEBUG
                tft.drawPixel(touchX, touchY, WHITE);
#endif
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

                int ch;
                boolean pressed = getKeyPress(&ch, touchX, touchY);
                #ifdef DEBUG
                if (pressed) printf("Key pressed: <%c>\n", (char)ch);
                #endif
                if (ch == 0x12) { // if enter was pressed
                    wifiConfig.text[wifiConfig.line] = textBuffer;
                    wifiConfig.line++;
                    textPosition = 0;

                    if (wifiConfig.line > 4) {
                        wifiConfig.line = 0;
                    }
                    textBuffer = wifiConfig.text[wifiConfig.line];
                    break;
                } else if (ch == 0x8) { // if backspace was pressed
                    textBuffer[textPosition - 1] = ' ';
                    textPosition--;
                    removeSpaces(textBuffer);
                } else if (ch == 0xe) { // if > was pressed
                    textPosition++;
                } else if (ch == 0xf) { // if < was pressed
                    textPosition--;
                } else if (ch == 0x20) { // if space was pressed
                    textBuffer[textPosition + 1] = ' ';
                    textPosition++;
                    removeSpaces(textBuffer);
                } else if (ch > 0x20 && ch < 0x7f) {
                    textBuffer[textPosition++] = ch;
                }
                if (textPosition == wifiConfig.arrLength[wifiConfig.line]) {
                    textPosition = 0;
                } else
                if (textPosition < 0) {
                    textPosition = 0;
                }
                printBuffer();
            }

            if (blink) {
                tft.drawLine(6 + (12 * textPosition), 6 + 32 * (wifiConfig.line + 1),
                            18 + (12 * textPosition), 6 + 32 * (wifiConfig.line + 1), WHITE);
            } else {
                tft.drawLine(6, 6 + 32 * (wifiConfig.line + 1),
                             232, 6 + 32 * (wifiConfig.line + 1), DARKGREY);
            }
            blink = !blink;

        }
        break;
    }



}
