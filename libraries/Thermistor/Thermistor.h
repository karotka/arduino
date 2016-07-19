// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000

// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25

// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 20

// The beta coefficient of the thermistor
#define BCOEFFICIENT 3950

// the value of the 'other' resistor
#define SERIESRESISTOR 99600

class Thermistor {

public:
    Thermistor(uint8_t pin);

    void begin();
    void readTemperature();
    float getCelsius();

protected:
    float samples[NUMSAMPLES];
    float steinhart;
    float celsius;
    float average;
    uint8_t pin;
};
