#include <Thermistor.h>

Thermistor d(A0);

void setup(void) {
    Serial.begin(9600);
    t0.begin();
}

void loop(void) {
    t0.read();

    Serial.print("Value ");
    Serial.print(t0.getValue());
    Serial.println("");

    delay(1000);
}