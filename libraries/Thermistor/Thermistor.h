// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25

// the value of the 'other' resistor
#define SERIESRESISTOR 10000

// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10

class Thermistor {

public:
    Thermistor(uint8_t pin, float correction,
               uint32_t thermistornominal, int bcoefficient);

    void begin();
    void readTemperature();
    float getCelsius();
    int getAdc() { return adc; };
    bool isEnabled() { return adc == 1023 ? false : true; };

protected:
    uint8_t pin;
    float samples[NUMSAMPLES];
    float steinhart;
    float celsius;
    float average;
    int adc;
    float correction;
    uint32_t thermistornominal;
    int bcoefficient;
    uint8_t pointer;
};
