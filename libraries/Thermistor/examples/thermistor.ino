#include <Thermistor.h>

Thermistor t0(A0);

void setup(void) {
    Serial.begin(9600);
    t0.begin();
}

void loop(void) {
    t0.readTemperature();

    Serial.print("Temperature ");
    Serial.print(t0.getCelsius());
    Serial.println(" *C");

    delay(1000);
}