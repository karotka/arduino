#ifndef LIGHTVALUES_H
#define LIGHTVALUES_H

#include <EEPROM.h>
#include "Arduino.h"


#define X_TOUCH_AREA_MIN 42
#define X_TOUCH_AREA_MAX 304

#define Y_TOUCH_AREA_MIN 36
#define Y_TOUCH_AREA_MAX 183

enum {
    MODE_OFF = 0,
    MODE_DAY,
    MODE_NIGHT,
    MODE_NONE
};

#define LED_YELLOW          13
#define LED_WHITE           12
#define LED_COOL_WHITE      11
#define LED_RED             10
#define LED_GREEN           9
#define LED_BLUE            8


class LigthValues_t {

public:
    //    LigthValues_t();
    LigthValues_t(uint8_t flag);

    void setCoolValue(int value);
    void setWarmValue(int value);
    void setYellowValue(int value);
    void setRedValue(int value);
    void setGreenValue(int value);
    void setBlueValue(int value);
    void save(void);
    void load(void);

    int coolValue;
    int coolByte;

    int warmValue;
    int warmByte;

    int yellowValue;
    int yellowByte;

    int redValue;
    int redByte;

    int greenValue;
    int greenByte;

    int blueValue;
    int blueByte;

    uint8_t flag;
};

#endif
