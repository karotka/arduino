#ifndef CONFIGURE_H
#define CONFIGURE_H

#define EEPROM_SIZE 1024
#define MAX_SLIDERS 6

#define PR(x) Serial.print(x)
#define PRN(x) Serial.print(x)
#define PRBN(x) Serial.print(x, BIN); Serial.println("")

const char* ssid = "KWIFI";
const char* password =  "Heslicko12";


#endif
