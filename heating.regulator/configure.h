#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <IPAddress.h>
#include <Adafruit_ILI9341esp.h>
#include <stdarg.h>

#define DEBUG    1

#define LOG(...) {\
    tft.setCursor(90, 0);\
    tft.setTextSize(1);\
    char printf_buf[60]; \
    sprintf(printf_buf, (const char *)__VA_ARGS__);\
    tft.print(printf_buf);\
  }

IPAddress wifiIp;
IPAddress wifiGateway;
IPAddress wifiSubnet;

/*#define DEVICENAME  "heatregulator"
#define WIFIIP       "172.16.0.190"
#define WIFIGATEWAY  "172.16.0.1"
#define WIFISUBNET   "172.16.255.255"

const char* WIFI_SSID   = "KWIFI";
const char* WIFI_PASSWD = "tajneheslo";
*/

#define TFT_DC 2
#define TFT_CS 15
#define DHTPIN 0
#define DHTTYPE DHT22
#define HYSTERESIS 0.2
#define HOME_READ_INTERVAL 3
#define TEMP_READ_INTERVAL 10

enum {
    PAGE_HOME = 0,
    PAGE_SET_HEAT,
    PAGE_SET_TIME,
    PAGE_SETTING,
    PAGE_DEBUG
};

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void dbglog(int x, int y, int z, bool c) {
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
    tft.print(" c:");
    tft.print(c);
#endif
}

#endif

void removeSpaces(char* s) {
    char* cpy = s;  // an alias to iterate through s without moving s
    char* temp = s;

    while (*cpy)  {
        if (*cpy != ' ')
            *temp++ = *cpy;
        cpy++;
    }
    *temp = 0;
}
