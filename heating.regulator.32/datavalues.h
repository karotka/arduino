#ifndef DAYVALUES_H
#define DAYVALUES_H

#include <EEPROM.h>
#include "Arduino.h"
#include "configure.h"

class WifiConfig {

public:
    char wifiEssid[20]   = "                   ";  // 20 bytes
    char wifiPass[20]    = "                   ";  // 20 bytes
    char wifiIp[16]      = "               ";      // 16 bytes
    char wifiGateway[16] = "               ";      // 16 bytes
    char wifiSubnet[16]  = "               ";      // 16 bytes
    int line = 0;

    char *text[5] = {wifiEssid, wifiPass, wifiIp, wifiGateway, wifiSubnet};
    int arrLength[5] = {20, 20, 16, 16, 16};

    void load(void) {
        EEPROM.readBytes(504, &wifiEssid, 16);   // 20 bytes
        EEPROM.readBytes(524, &wifiPass, 16);    // 20 bytes
        EEPROM.readBytes(544, &wifiIp, 20);      // 16 bytes
        EEPROM.readBytes(570, &wifiGateway, 20); // 16 bytes
        EEPROM.readBytes(586, &wifiSubnet, 20);  // 16 bytes
        EEPROM.commit();
        printf("Load->ESS:%s, Pass:%s, IP:%s, GW:%s, SN:%s.\n",
               wifiEssid, wifiPass, wifiIp, wifiGateway, wifiSubnet);
    }

    void save(void) {
        printf("Save->ESS:%s, Pass:%s, IP:%s, GW:%s, SN:%s.\n",
               wifiEssid, wifiPass, wifiIp, wifiGateway, wifiSubnet);
        EEPROM.writeBytes(504, wifiEssid, 20);   // 80 bytes
        EEPROM.writeBytes(524, wifiPass, 20);    // 80 bytes
        EEPROM.writeBytes(544, wifiIp, 16);      // 64 bytes
        EEPROM.writeBytes(570, wifiGateway, 16); // 64 bytes
        EEPROM.writeBytes(586, wifiSubnet, 16);  // 64 bytes
        EEPROM.commit();
    }
};

class HeatValues {

public:
    int hour[6];           // 24 bytes
    int minute[6];         // 24 bytes
    float temperature[6];  // 24 bytes = 72 bytes
    int flag;
    boolean heatAuto;

    void save(void) {
        //EEPROM.begin(1024);
        if (flag == 1) {
            int addr = 0;
            EEPROM.put(addr, hour);
            addr += sizeof(hour);
            EEPROM.put(addr, minute);
            addr += sizeof(hour);
            EEPROM.put(addr, temperature);
        } else
        if (flag == 2) {
            int addr = 72;
            EEPROM.put(addr, hour);
            addr += sizeof(hour);
            EEPROM.put(addr, minute);
            addr += sizeof(hour);
            EEPROM.put(addr, temperature);
        } else
        if (flag == 3) {
            int addr = 144;
            EEPROM.put(addr, hour);
            addr += sizeof(hour);
            EEPROM.put(addr, minute);
            addr += sizeof(hour);
            EEPROM.put(addr, temperature);
        } else
        if (flag == 4) {
            int addr = 216;
            EEPROM.put(addr, hour);
            addr += sizeof(hour);
            EEPROM.put(addr, minute);
            addr += sizeof(hour);
            EEPROM.put(addr, temperature);
        } else
        if (flag == 5) {
            int addr = 288;
            EEPROM.put(addr, hour);
            addr += sizeof(hour);
            EEPROM.put(addr, minute);
            addr += sizeof(hour);
            EEPROM.put(addr, temperature);
        } else
        if (flag == 6) {
            int addr = 360;
            EEPROM.put(addr, hour);
            addr += sizeof(hour);
            EEPROM.put(addr, minute);
            addr += sizeof(minute);
            EEPROM.put(addr, temperature);
        } else
        if (flag == 7) {
            int addr = 432;
            EEPROM.put(addr, hour);
            addr += sizeof(hour);
            EEPROM.put(addr, minute);
            addr += sizeof(minute);
            EEPROM.put(addr, temperature);
        }
        EEPROM.commit();
    }

    void load(void) {
        //EEPROM.begin(1024);
        if (flag == 1) {
            int addr = 0;
            EEPROM.get(addr, hour);
            addr += sizeof(hour);
            EEPROM.get(addr, minute);
            addr += sizeof(minute);
            EEPROM.get(addr, temperature);
            checkNan(temperature);
        } else
        if (flag == 2) {
            int addr = 72;
            EEPROM.get(addr, hour);
            addr += sizeof(hour);
            EEPROM.get(addr, minute);
            addr += sizeof(minute);
            EEPROM.get(addr, temperature);
            checkNan(temperature);
        } else
        if (flag == 3) {
            int addr = 144;
            EEPROM.get(addr, hour);
            addr += sizeof(hour);
            EEPROM.get(addr, minute);
            addr += sizeof(minute);
            EEPROM.get(addr, temperature);
            checkNan(temperature);
        } else
        if (flag == 4) {
            int addr = 216;
            EEPROM.get(addr, hour);
            addr += sizeof(hour);
            EEPROM.get(addr, minute);
            addr += sizeof(minute);
            EEPROM.get(addr, temperature);
            checkNan(temperature);
        } else
        if (flag == 5) {
            int addr = 288;
            EEPROM.get(addr, hour);
            addr += sizeof(hour);
            EEPROM.get(addr, minute);
            addr += sizeof(minute);
            EEPROM.get(addr, temperature);
            checkNan(temperature);
        } else
        if (flag == 6) {
            int addr = 360;
            EEPROM.get(addr, hour);
            addr += sizeof(hour);
            EEPROM.get(addr, minute);
            addr += sizeof(minute);
            EEPROM.get(addr, temperature);
            checkNan(temperature);
        } else
        if (flag == 7) {
            int addr = 432;
            EEPROM.get(addr, hour);
            addr += sizeof(hour);
            EEPROM.get(addr, minute);
            addr += sizeof(minute);
            EEPROM.get(addr, temperature);
            checkNan(temperature);
        }
        EEPROM.commit();
    }

private:
    void checkNan(float arr[]) {
        for (int i = 0; i < 7; i++) {
            if (isnan(arr[i])) {
                arr[i] = 20;
            }
        }
    }
    void checkMinus(int arr[]) {
        for (int i = 0; i < 7; i++) {
            if (arr[i] < 0) {
                arr[i] = 0;
            }
        }
    }
};

#endif
