#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <RTClib.h>
#include <UTFT.h>
#include <ITDB02_Touch.h>
#include <UTFT_Buttons.h>

#define DEBUG               1

#define RETURN_DELAY        5000 // 11s
#define RELE_PIN            14
#define COLOR_TIME          255, 60, 60
#define DIMMING_SPEED_FAST  10
#define DIMMING_SPEED_SLOW  1000

#define FEED_PIN_SW      18
#define FEED_PIN_POWER   19

UTFT          myGLCD(ITDB32S, 38, 39, 40, 41);
ITDB02_Touch  myTouch(6, 5, 4, 3, 2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

enum {
    PAGE_HOME = 0,
    PAGE_TIME,
    PAGE_CO2,
    HOLO,
    TILO,
    COLO,
    FDLO,
    LILO,
    TRLO,
    TISA,
    COSA
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


#endif //_CONFIG_H_
