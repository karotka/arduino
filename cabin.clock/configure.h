#ifndef CONFIGURE_H
#define CONFIGURE_H

#define SEVEN_SEGMENT_DDR DDRD
#define SEVEN_SEGMENT_PORT PORTD

enum {
    SHOW_TIME = 0,
    SHOW_DATE,
    SHOW_TEMP,
    SHOW_TIMER
    //SHOW_LIGHT
};

enum {
    TIMER_20 = 0,
    TIMER_10,
    TIMER_05
};
unsigned int timerValues [3] = {1200, 600, 300};

enum {
    TIMER_SET = 0,
    TIMER_RUN,
    TIMER_LASTMIN,
    TIMER_FINISHED
};

unsigned int timerRun = TIMER_SET;

enum {
    SET_HOUR = 0,
    SET_MINUTE,
    SET_DAY,
    SET_MONTH,
    SET_NONE
};

#define SHOW_TIME_TIME 65000
#define SHOW_DATE_TIME 20000
#define SHOW_TEMP_TIME 20000

#define DHT22_PIN 13
#define TEMP_CORRECTION 0.7

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

#endif
