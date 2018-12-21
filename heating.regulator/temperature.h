#include <DHT.h>

#define NUMSAMPLES 10

class Temperature {

public:
    float temperatureC;
    int humidity;
    int heatIndex;
    int pointer;
    float samples[NUMSAMPLES];
    DHT dht;

    Temperature(DHT &dht) : dht(dht), heatIndex(0)  {
        int pointer = 0;
    }

    void readTemperature() {

        float finalTemp;
        float tempC = dht.readTemperature();
        if (isnan(tempC)) {
            return;
        }

        float tempF = dht.readTemperature(true);
        if (isnan(tempF)) {
            return;
        }
        // shift array
        for (int i = 1; i < NUMSAMPLES; i++) {
            samples[i - 1] = samples[i];
            finalTemp += samples[i];
        }
        samples[NUMSAMPLES - 1] = tempC;
        pointer++;

        if (pointer > NUMSAMPLES) {
            pointer = NUMSAMPLES;
            heatIndex = dht.computeHeatIndex(temperatureC, tempF);
        }
        temperatureC = finalTemp / (NUMSAMPLES - 1);
        temperatureC = float(int(temperatureC * 10)) / 10.0 ;
        humidity = dht.readHumidity();
    }

};
