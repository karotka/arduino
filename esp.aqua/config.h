#ifndef CONFIG_H
#define CONFIG_H

#include "EEPROM.h"

#define EEPROM_SIZE 512


class Config_t {

public:
    String ssid = "KWIFI";
    String password = "Heslicko12";
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    uint8_t apMode;

    Config_t() :
        ip(192, 168, 0, 10),
        gateway(192, 168, 0, 1),
        subnet(255, 255, 255, 0) {
        load();
    }

    void load() {
        apMode = EEPROM.read(0);

        int addr = 1;

        uint8_t first = EEPROM.read(addr); addr = 0 + sizeof(first);
        uint8_t second = EEPROM.read(addr); addr = addr + sizeof(second);
        uint8_t third = EEPROM.read(addr); addr = addr + sizeof(third);
        uint8_t fourtht = EEPROM.read(addr); addr = addr + sizeof(fourtht);
        IPAddress ip(first, second, third, fourtht);

        EEPROM.commit();
    }

    void save() {
        int addr = 1;
        //addr += EEPROM.put(addr, myInt);
        EEPROM.end();
    }

};

/*
void readEeprom() {
}
*/
#endif
