#ifndef LIGHTVALUES_H
#define LIGHTVALUES_H

#include <EEPROM.h>
#include <stdint.h>
#include "configure.h"

typedef struct {
    uint16_t    pixels;
    uint16_t    bytes;
} Values_t ;


class LightValues_t {

public:

    void save(int modeId) {
        EEPROM.begin(EEPROM_SIZE);

        if (modeId == 0) {
            int address = 0;
            for (int i = 0; i < 6; i++) {
                EEPROM.put(address, sliders[i].pixels);
                address += sizeof(uint16_t);
            }
        }
        EEPROM.commit();
    }

    void load(int modeId) {
        EEPROM.begin(EEPROM_SIZE);

        if (modeId == 0) {
            int address = 0;
            for (int i = 0; i < 6; i++) {
                EEPROM.get(address, sliders[i].pixels);
                address += sizeof(uint16_t);
            }
        }
        EEPROM.commit();
    }

    Values_t getSlider(int id) {
        return sliders[id];
    }

    Values_t sliders[MAX_SLIDERS];

};

#endif
