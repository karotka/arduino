#include <Arduino.h>
#include "Thermistor.h"

Thermistor::Thermistor(uint8_t pin, float correction)
    : pin(pin), correction(correction), celsius(0), pointer(0)  {
}

void Thermistor::begin() {
    analogReference(EXTERNAL);
}

float Thermistor::getCelsius() {
    if (pointer != NUMSAMPLES) {
        return 0;
    }
    // average all the samples out
    average = 0;
    for (uint8_t i = 0; i < NUMSAMPLES; i++) {
        average += samples[i];
    }
    average /= NUMSAMPLES;

    // convert the value to resistance
    average = 1023 / average - 1;
    average = SERIESRESISTOR / average;

    steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert to C
    steinhart += correction;

    return steinhart;
}

void Thermistor::readTemperature() {

    // shift array
    for (uint8_t i = 1; i < NUMSAMPLES; i++) {
        samples[i - 1] = samples[i];
    }
    adc = analogRead(pin);
    samples[NUMSAMPLES - 1] = adc;
    pointer++;

    if (pointer > NUMSAMPLES) {
        pointer = NUMSAMPLES;
    }
}
