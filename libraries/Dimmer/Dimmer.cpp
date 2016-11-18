#include <Arduino.h>
#include "Dimmer.h"

Dimmer::Dimmer(uint8_t pin)
    : pin(pin), pointer(0)  {
}

void Dimmer::begin() {
}

float Dimmer::getValue() {
    if (pointer != NUMSAMPLES) {
        return 0;
    }
    // average all the samples out
    average = 0;
    for (uint8_t i = 0; i < NUMSAMPLES; i++) {
        average += samples[i];
    }
    return average /= NUMSAMPLES;
}

void Dimmer::read() {

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
