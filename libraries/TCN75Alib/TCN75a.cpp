#include <Wire.h>
#include "TCN75A.h"

TCN75A::TCN75A() {
  //Set initial values for private vars
}

void TCN75A::begin(void) {
    Wire.begin();

    // Start I2C transmission
    Wire.beginTransmission(TCN75A_ADDRESS);
    // Select configuration register
    Wire.write(0x01);
    // 12-bit ADC resolution
    Wire.write(0x60);
    // Stop I2C transmission
    Wire.endTransmission();
}

float TCN75A::readTemperature() {
    unsigned int data[2];

    Wire.beginTransmission(TCN75A_ADDRESS);
    Wire.write(0x00);
    Wire.endTransmission();

    // Request 2 bytes of data
    Wire.requestFrom(TCN75A_ADDRESS, 2);

    // Read 2 bytes of data
    // temp msb, temp lsb
    if (Wire.available() == 2) {
        data[0] = Wire.read();
        data[1] = Wire.read();
    }

    // Convert the data to 12-bits
    int temp = (((data[0] * 256) + (data[1] & 0xF0)) / 16);
    if(temp > 2047) {
        temp -= 4096;
    }

    return temp * 0.0625;
}
