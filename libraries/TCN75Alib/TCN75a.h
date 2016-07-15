#include <Wire.h>

#define TCN75A_ADDRESS       0x48

class TCN75A {
public:
    TCN75A();

    void begin();
    float readTemperature();
};
