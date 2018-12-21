#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <IPAddress.h>

#define DEBUG    1

IPAddress wifiIp;
IPAddress wifiGateway;
IPAddress wifiSubnet;

#define DEVICENAME  "heatregulator"
#define WIFIIP       "172.16.0.190"
#define WIFIGATEWAY  "172.16.0.1"
#define WIFISUBNET   "172.16.255.255"

const char* WIFI_SSID   = "KWIFI";
const char* WIFI_PASSWD = "tajneheslo";

#define TFT_DC 2
#define TFT_CS 15
#define DHTPIN 0
#define DHTTYPE DHT22
#define HYSTERESIS 0.2
#define HOMEREADINTERVAL 25

enum {
    PAGE_HOME = 0,
    PAGE_SET_HEAT,
    PAGE_SET_TIME,
    PAGE_DEBUG
};

#endif
