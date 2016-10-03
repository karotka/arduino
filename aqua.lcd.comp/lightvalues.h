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
    MODE_DAY1,
    MODE_DAY2,
    MODE_NIGHT1,
    MODE_NIGHT2
};

#define LED_YELLOW          13  // OC0A
#define LED_WHITE           12  // OC1B
#define LED_COOL_WHITE      11  // OC1A
#define LED_RED             10  // OC2A
#define LED_GREEN           9   // OC2B
#define LED_BLUE            8   // OC4C


class LigthValues_t {

public:
    //    LigthValues_t();
    LigthValues_t(uint8_t flag);

    void setCoolValue(int value);
    void setCoolByte(uint8_t value);

    void setWarmValue(int value);
    void setWarmByte(uint8_t value);

    void setYellowValue(int value);

    void setRedValue(int value);

    void setGreenValue(int value);

    void setBlueValue(int value);

    void save(void);
    void load(void);

    int coolValue;
    uint8_t coolByte;

    int warmValue;
    uint8_t warmByte;

    int yellowValue;
    uint8_t yellowByte;

    int redValue;
    uint8_t redByte;

    int greenValue;
    uint8_t greenByte;

    int blueValue;
    uint8_t blueByte;

    uint8_t flag;
};

#endif
