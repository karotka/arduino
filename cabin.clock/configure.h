#ifndef CONFIGURE_H
#define CONFIGURE_H

enum {
    SHOW_TIME = 0,
    SHOW_DATE,
    SHOW_TEMP
};

enum {
    SET_HOUR = 0,
    SET_MINUTE,
    SET_DAY,
    SET_MONTH,
    SET_NONE
};

#define SHOW_TIME_TIME 7000
#define SHOW_DATE_TIME 2000
#define SHOW_TEMP_TIME 2000

#define DHT22_PIN 13

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))


#endif
