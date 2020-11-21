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
    uint16_t dimmerSpeed;

    Config_t() :
        ip(192, 168, 0, 8),
        gateway(192, 168, 0, 1),
        subnet(255, 255, 255, 0),
        dimmerSpeed(5) {
    }

    void load() {
        apMode = EEPROM.read(0);
        EEPROM.get(8, dimmerSpeed);

        int addr = 200;

        uint8_t first = EEPROM.read(addr);   addr += sizeof(first);
        uint8_t second = EEPROM.read(addr);  addr += sizeof(second);
        uint8_t third = EEPROM.read(addr);   addr += sizeof(third);
        uint8_t fourtht = EEPROM.read(addr); addr += sizeof(fourtht);

        //EEPROM.get(addr, ssid); addr += 20;
        //EEPROM.get(addr, password); addr += 20;
        //Serial.print("Load ssid:");
        //Serial.println(ssid);

        //Serial.print("Load pass:");
        //Serial.println(password);

        IPAddress ip(first, second, third, fourtht);

        EEPROM.commit();
    }

    void save() {
        EEPROM.put(8, dimmerSpeed);

        int addr = 200;
        EEPROM.put(addr, ip[0]); addr += sizeof(ip[0]);
        EEPROM.put(addr, ip[1]); addr += sizeof(ip[1]);
        EEPROM.put(addr, ip[2]); addr += sizeof(ip[2]);
        EEPROM.put(addr, ip[3]); addr += sizeof(ip[3]);

        //Serial.print("Save Address 4: ");
        //Serial.println(ip[3]);

        EEPROM.put(addr, gateway[0]); addr += sizeof(gateway[0]);
        EEPROM.put(addr, gateway[1]); addr += sizeof(gateway[1]);
        EEPROM.put(addr, gateway[2]); addr += sizeof(gateway[2]);
        EEPROM.put(addr, gateway[3]); addr += sizeof(gateway[3]);

        EEPROM.put(addr, subnet[0]); addr += sizeof(subnet[0]);
        EEPROM.put(addr, subnet[1]); addr += sizeof(subnet[1]);
        EEPROM.put(addr, subnet[2]); addr += sizeof(subnet[2]);
        EEPROM.put(addr, subnet[3]); addr += sizeof(subnet[3]);

        EEPROM.put(addr, ssid);     addr += 20;
        //EEPROM.get(addr, ssid);
        //Serial.print("SSID: ");
        //Serial.println(ssid);

        EEPROM.put(addr, password); addr += 20;
        //EEPROM.get(addr, password);
        //Serial.print("PASS: ");
        //Serial.println(password);

        EEPROM.commit();
    }

};

/*
void readEeprom() {
}
*/
#endif
