#include "configvalues.h"
#include <EEPROM.h>

ConfigValues_t::ConfigValues_t() {
};

void ConfigValues_t::save() {
    int address = 0;

    EEPROM.write(address     , hours[0]);
    EEPROM.write(address + 1 , minutes[0]);
    EEPROM.write(address + 2 , statuses[0]);

    EEPROM.write(address + 3 , hours[1]);
    EEPROM.write(address + 4 , minutes[1]);
    EEPROM.write(address + 5 , statuses[1]);

    EEPROM.write(address + 6 , hours[2]);
    EEPROM.write(address + 7 , minutes[2]);
    EEPROM.write(address + 8 , statuses[2]);

    EEPROM.write(address + 9 , hours[3]);
    EEPROM.write(address + 10, minutes[3]);
    EEPROM.write(address + 11, statuses[3]);

    EEPROM.write(address + 12, hours[4]);
    EEPROM.write(address + 13, minutes[4]);
    EEPROM.write(address + 14, statuses[4]);

    EEPROM.write(address + 15, hours[5]);
    EEPROM.write(address + 16, minutes[5]);
    EEPROM.write(address + 17, statuses[5]);

    EEPROM.write(address + 18, hours[6]);
    EEPROM.write(address + 19, minutes[6]);
    EEPROM.write(address + 20, statuses[6]);

    EEPROM.write(address + 21, hours[7]);
    EEPROM.write(address + 22, minutes[7]);
    EEPROM.write(address + 23, statuses[7]);

    EEPROM.write(address + 24, hours[8]);
    EEPROM.write(address + 25, minutes[8]);
    EEPROM.write(address + 26, statuses[8]);

    address = address + 32;

    EEPROM.put(address, totalLitresA);
    address += sizeof(float);

    EEPROM.put(address, totalLitresB);
    address += sizeof(float);

    EEPROM.put(address, totalLitres);
};

void ConfigValues_t::load() {
    int address = 0;

    hours[0]   = EEPROM.read(address);
    minutes[0] = EEPROM.read(address + 1);
    statuses[0]= EEPROM.read(address + 2);

    hours[1]   = EEPROM.read(address + 3);
    minutes[1] = EEPROM.read(address + 4);
    statuses[1]= EEPROM.read(address + 5);

    hours[2]   = EEPROM.read(address + 6);
    minutes[2] = EEPROM.read(address + 7);
    statuses[2]= EEPROM.read(address + 8);

    hours[3]   = EEPROM.read(address + 9);
    minutes[3] = EEPROM.read(address + 10);
    statuses[3]= EEPROM.read(address + 11);

    hours[4]   = EEPROM.read(address + 12);
    minutes[4] = EEPROM.read(address + 13);
    statuses[4]= EEPROM.read(address + 14);

    hours[5]   = EEPROM.read(address + 15);
    minutes[5] = EEPROM.read(address + 16);
    statuses[5]= EEPROM.read(address + 17);

    hours[6]   = EEPROM.read(address + 18);
    minutes[6] = EEPROM.read(address + 19);
    statuses[6]= EEPROM.read(address + 20);

    hours[7]   = EEPROM.read(address + 21);
    minutes[7] = EEPROM.read(address + 22);
    statuses[7]= EEPROM.read(address + 23);

    hours[8]   = EEPROM.read(address + 24);
    minutes[8] = EEPROM.read(address + 25);
    statuses[8]= EEPROM.read(address + 26);

    address = address + 32;

    EEPROM.get(address, totalLitresA);
    address += sizeof(float);

    EEPROM.get(address, totalLitresB);
    address += sizeof(float);

    EEPROM.get(address, totalLitres);

    //totalLitresA = 0.0;
    //totalLitresB = 0.0;
    //totalLitres  = 0.0;
    //statuses[0] = 2;
    //statuses[1] = 2;
    //statuses[2] = 2;
    //statuses[3] = 2;
    //statuses[4] = 2;
    //statuses[5] = 2;
    //statuses[6] = 2;
    //statuses[7] = 2;
    //statuses[8] = 2;

};
