#ifndef VALUES_H
#define VALUES_H

#include <EEPROM.h>
#include <stdint.h>

#define MAX_SLIDERS 6


class Values_t {

public:
    Values_t(uint8_t modeId);

    void save();
    void load();

    uint16_t sliders[MAX_SLIDERS];
    uint8_t modeId;

};

#endif
