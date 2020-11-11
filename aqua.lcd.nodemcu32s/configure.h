#ifndef CONFIGURE_H
#define CONFIGURE_H

#define EEPROM_SIZE 1024

#define PR(x) Serial.print(x)
#define PRN(x) Serial.print(x)
#define PRBN(x) Serial.print(x, BIN); Serial.println("")

const char* ssid = "KWIFI";
const char* password =  "Heslicko12";

#define PWM_FREQ 5000
#define PWM_RESOLUTION 10

#define MODE_0 0
#define MODE_1 1
#define MODE_2 2
#define MODE_3 3
#define MODE_4 4

#define TFT_DARKORANGE  0xCC80
#define TFT_DARKBLUE 0x0073
#define TFT_SKY 0x34D9
#define TFT_DARKSKY 0x2BF4
#define TFT_MYGRAY 0x7BEF
#define TFT_MYDARKGRAY 0x630C



int hours[9] = {0, 3, 6, 9};
int minutes[9] = {1, 4, 7, 10};
int buttons[4] = {2, 5, 8, 11};

const char* onOffStr[2]  = {"OFF", "ON"};

bool inArray(int array[], int element, size_t size) {
    for (int i = 0; i < size; i++) {
        if (array[i] == element) {
            return true;
        }
    }
    return false;
}

#endif
