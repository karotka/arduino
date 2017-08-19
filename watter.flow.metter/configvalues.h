#ifndef CONFIGVALUES_H
#define CONFIGVALUES_H

#include "Arduino.h"

#define DELAY 20
#define RETURN_TIMEOUT 13
enum {
    SET_NONE = 0,
    SET_A,
    SET_B,
    SET_HOUR,
    SET_MINUTE,
    SET_DAY,
    SET_MONTH,
    SET_YEAR,
    SET_T1_HOUR,
    SET_T1_MIN,
    SET_T1_ST,
    SET_T2_HOUR,
    SET_T2_MIN,
    SET_T2_ST,
    SET_T3_HOUR,
    SET_T3_MIN,
    SET_T3_ST,
    SET_T4_HOUR,
    SET_T4_MIN,
    SET_T4_ST,
    SET_T5_HOUR,
    SET_T5_MIN,
    SET_T5_ST,
    SET_T6_HOUR,
    SET_T6_MIN,
    SET_T6_ST,
    SET_T7_HOUR,
    SET_T7_MIN,
    SET_T7_ST,
    SET_T8_HOUR,
    SET_T8_MIN,
    SET_T8_ST,
    SET_T9_HOUR,
    SET_T9_MIN,
    SET_T9_ST,
    SET_END
};
enum {
    PAGE_FIRST = 0,
    PAGE_SECOND
};

class ConfigValues_t {

 public:

    ConfigValues_t();

    int minutes[9];
    int hours[9];
    int statuses[9];

    float totalLitres;
    float totalLitresA;
    float totalLitresB;

    void save();
    void load();
};

#endif
