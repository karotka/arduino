// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000

// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25

// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10

// The beta coefficient of the thermistor
#define BCOEFFICIENT 3380

// the value of the 'other' resistor
#define SERIESRESISTOR 10000

class Thermistor {

public:
    Thermistor(uint8_t pin, float correction = 0);

    void begin();
    void readTemperature();
    float getCelsius();
    int getAdc() { return adc; };

protected:
    uint8_t pin;
    float correction;
    float samples[NUMSAMPLES];
    float steinhart;
    float celsius;
    float average;
    int adc;
    uint8_t pointer;
};
