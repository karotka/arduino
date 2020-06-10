#include <DHT.h>

#define NUMSAMPLES 10

class Temperature {

public:
    float temperatureC;
    float humidity;
    int heatIndex;
    int pointer;
    float samples[NUMSAMPLES];
    DHT dht;

    Temperature(DHT &dht) : dht(dht), heatIndex(0), pointer(0)  {}

    void readTemperature() {

        float finalTemp;
        float tempC = dht.readTemperature(false);
        if (isnan(tempC)) {
            return;
        }

        humidity = dht.readHumidity();

        // shift array
        for (int i = 1; i < NUMSAMPLES; i++) {
            samples[i - 1] = samples[i];
            finalTemp += samples[i];
        }
        samples[NUMSAMPLES - 1] = tempC;
        pointer++;

        if (pointer > NUMSAMPLES) {
            pointer = NUMSAMPLES;
            heatIndex = dht.computeHeatIndex(temperatureC, humidity, false);
        }
        temperatureC = finalTemp / (NUMSAMPLES - 1);
        temperatureC = float(int(temperatureC * 10)) / 10.0;
    }

};
