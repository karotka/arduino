#ifndef CONFIGURE_H
#define CONFIGURE_H

// Define the pin which is used as DataIn on the LED matrix
#define MATRIX_PIN 7

// define some color codes
#define WHITE   0XFFFF
#define GREEN   0x07E0
#define RED     0xF800
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define BLUE    0x001F
#define AQUA    0x07FF
#define TEAL    0x0410
#define NAVY    0x0010
#define FUCHSIA 0xF81F
#define PURPLE  0x8010
#define OLIVE   0x8400
#define LIME    0x07E0

enum {
    SCORE_NONE = 0,
    SCORE_MINUS,
    SCORE_END
};

enum {
    MODE_SCORE = 0,
    MODE_TIME,
    MODE_NONE
};

enum {
    SETUP_MODE = 0,
    SETUP_BRIG,
    SETUP_COLOR,
    SETUP_VOLTAGE,
    SETUP_NONE
};

const char *modes[4] = {"SETUP", "BRIGH", "COLOR", "V" };
const unsigned int colors[14] = {
    WHITE,
    GREEN,
    RED,
    CYAN,
    MAGENTA,
    YELLOW,
    BLUE,
    AQUA,
    TEAL,
    NAVY,
    FUCHSIA,
    PURPLE,
    OLIVE,
    LIME
};

inline unsigned int debounce(volatile uint8_t *port, uint8_t pin) {
    if (!(*port & (1 << pin))) {
        _delay_ms(50);
        return HIGH;
    }
    return LOW;
}

inline unsigned int notBounce(volatile uint8_t *port, uint8_t pin) {
    if ((*port & (1 << pin))) {
        _delay_ms(50);
        return HIGH;
    }
    return LOW;
}

class Score_t {

 public:
    int first;
    int second;

    Score_t () {
        first = 0;
        second = 0;
    }
};

#endif
