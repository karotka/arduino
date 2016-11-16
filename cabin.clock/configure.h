#ifndef CONFIGURE_H
#define CONFIGURE_H

#define SEVEN_SEGMENT_DDR DDRD
#define SEVEN_SEGMENT_PORT PORTD

enum {
    SHOW_TIME = 0,
    SHOW_DATE,
    SHOW_TEMP
    //    SHOW_LIGHT
};

enum {
    SET_HOUR = 0,
    SET_MINUTE,
    SET_DAY,
    SET_MONTH,
    SET_NONE
};

#define SHOW_TIME_TIME 64000
#define SHOW_DATE_TIME 24000
#define SHOW_TEMP_TIME 24000

#define DHT22_PIN 13
#define TEMP_CORRECTION 0.7

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

#endif
