#include <Thermistor.h>

Thermistor t0(A0, 0.5);
Thermistor t1(A1, 0.3);
float    te0, te1;

void setup(void) {
    Serial.begin(9600);
    t0.begin();
    t1.begin();
}

void myRound(float * value) {
    *value = *value * 10.0f;
    *value = *value > 0.0f ? floor(*value + 0.5f) : ceil(*value - 0.5f);
    *value = *value / 10.0f;
}

void loop(void) {

    t0.readTemperature();
    te0 = t0.getCelsius();
    myRound(&te0);

    Serial.print("Temp0: ");
    Serial.print(te0);
    Serial.print(" *C ADC0: ");
    Serial.print(t0.getAdc());
    Serial.print(" ");

    t1.readTemperature();
    te1 = t1.getCelsius();
    myRound(&te1);

    Serial.print("Temp1: ");
    Serial.print(te1);
    Serial.print(" *C ADC1: ");
    Serial.println(t1.getAdc());

    delay(1000);
}