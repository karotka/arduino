#include <UTFT.h>
#include <ITDB02_Touch.h>
#include <UTFT_Buttons.h>

#define DEBUG               0

#define RETURN_DELAY        5000 // 11s
#define RELE_PIN            14
#define LED_WHITE           12
#define LED_COOL_WHITE      11
#define LED_RED             10
#define LED_GREEN           9
#define LED_BLUE            8

#define COLOR_TIME 255, 60, 60

UTFT          myGLCD(ITDB32S, 38, 39, 40, 41);
ITDB02_Touch  myTouch(6, 5, 4, 3, 2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);


const char* co2Str[2]  = {"OFF", "ON"};
const char* dateStr[4] = {"OFF", "DAY", "NIGHT", "NONE"};

enum {
    MODE_OFF = 0,
    MODE_DAY,
    MODE_NIGHT,
    MODE_NONE
};

enum {
    PAGE_HOME = 0,
    PAGE_SELECT,
    PAGE_SET_LIGHT,
    PAGE_SET_TIMER,
    PAGE_SET_TIME,
    PAGE_SET_CO2,
    PAGE_TEMP,
    PAGE_DEBUG,
    PAGE_RETURN_MOME
};

enum {
    MODE_AUTO = 0,
    MODE_MANUAL
};

enum {
    CO2_OFF = 0,
    CO2_ON,
    CO2_NONE
};
