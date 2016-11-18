// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 15

class Dimmer {

public:
    Dimmer(uint8_t pin);

    void begin();
    void read();
    float getValue();
    int getAdc() { return adc; };

protected:
    uint8_t pin;
    float samples[NUMSAMPLES];
    float average;
    int adc;
    uint8_t pointer;
};
