#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <IPAddress.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_ILI9486_STM32.h>

#define DEBUG
#define EEPROM_SIZE 1024

IPAddress wifiIp;
IPAddress wifiGateway;
IPAddress wifiSubnet;

#define WIFI_CONNECT_TIMEOUT 100 // 10s

#define DHTPIN 27
#define DHTTYPE DHT22

#define HYSTERESIS 0.2
#define BLACK_SCREEN_MS    6000   // black screen and sleep after x ms
#define DEEP_SLEEP_MS      3000   // not implemented
#define WAKEUP_TIMER_MS    2000   // wake up one per x ms
#define TEMP_READ_INTERVAL 150 // one per x ms

#define ADC_PIN0 33
#define PWM_FREQ 5000
#define PWM_CHA1 0
#define PWM_RES  12 // Resolution 8, 10, 12, 15 bit
#define PWM_PIN0 13 // LED PWM

#define RELAY_PIN0 2
#define RELAY_PIN1 4
enum {
    PAGE_HOME = 0, // default
    PAGE_SET_HEAT,
    PAGE_SET_TIME,
    PAGE_SETTING_WIFI,
    PAGE_SETTINGS,
    PAGE_DEBUG
};

// XPT2046 touch
#define T_CS   39
#define T_IRQ  36
const int coords[] =
    { 3820, 200, 3900, 160 }; // lanscape - left, right, top, bottom


#define TFT_CS   5
#define TFT_DC   0
#define TFT_RST  15
//#define TFT_MOSI 23
//#define TFT_CLK  18
//#define TFT_MISO 19
//#define TFT_LED  36
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//Adafruit_ILI9486_STM32 tft;

// Real color
#define WHITE ILI9341_WHITE
#define YELLOW ILI9341_YELLOW
#define GREEN ILI9341_GREEN
#define RED ILI9341_RED
#define BLUE ILI9341_BLUE
#define NAVY ILI9341_NAVY
#define DARKGREY ILI9341_DARKGREY
#define BLACK ILI9341_BLACK
#define BACK_COLOR BLACK


// Black and white
/*
#define WHITE 0xFFFF
#define YELLOW 0xD6BA
#define GREEN 0xA534
#define RED 0xCE59
#define BLUE 0x5AEB
#define NAVY 0x18E3
#define DARKGREY 0x5AEB
#define BLACK 0x0000
#define BACK_COLOR BLACK
*/

void removeSpaces(char *str) {
    int y = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ' ') continue;
        else {
            str[y++] = str[i];
        }
    }
    str[y] = '\0';
}

void trim(char * cpy) {
    int l = strlen(cpy);
    char *s = cpy;
    while(isspace(s[l - 1])) --l;
    while(* s && isspace(* s)) ++s, --l;
    strncpy(cpy, s, l);
    cpy[l] = '\0';
}

void mySleep(const int interval) {
    unsigned long currentMillis = millis();
    while(millis() < currentMillis + interval);
}
#endif
