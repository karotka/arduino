#ifndef CONFIGVALUES_H
#define CONFIGVALUES_H

#include "Arduino.h"

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
