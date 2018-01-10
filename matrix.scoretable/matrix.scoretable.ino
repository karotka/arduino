// Include libraries for the LED matrix
#include "configure.h"
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(30, 8,  MATRIX_PIN,
  NEO_MATRIX_BOTTOM  + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

Score_t score;
char str[6];

int mode = MODE_SCORE;
int set = SCORE_NONE;
int setModes = SETUP_MODE;
int brighness = 100;
int colorIndex = 2;

void setup() {
    pinMode(4,  INPUT);
    pinMode(5,  INPUT);
    pinMode(6,  INPUT);
    pinMode(7,  OUTPUT);
    pinMode(8,  INPUT);
    pinMode(9,  INPUT);
    pinMode(10, INPUT);

    // initialize the LED matrix
    matrix.begin();

    // set the brightness; max = 255
    brighness = EEPROM.read(0);
    colorIndex = EEPROM.read(1);
    matrix.setBrightness(brighness);

    matrix.setTextWrap(false);
    matrix.setTextSize(1);
    int counter = 0;

    while (counter < 15 || setModes == SETUP_VOLTAGE) {
        matrix.fillScreen(0);
        if (setModes == SETUP_VOLTAGE) {
            sprintf(str, "V%.2f",  analogRead(A0) * (4.9 / 1023.0));
        } else {
            sprintf(str, "%s", modes[setModes]);
        }

        if (digitalRead(8) == LOW) {
            counter = 0;
            if (setModes == SETUP_BRIG) {
                brighness++;
                if (brighness > 20) {
                    brighness = 1;
                }
            }
            if (setModes == SETUP_COLOR) {
                colorIndex++;
                if (colorIndex > 14) {
                    colorIndex = 1;
                }
            }
        }

        if (digitalRead(9) == LOW) {
            counter = 0;
            if (setModes == SETUP_BRIG) {
                brighness--;
                if (brighness == 0) {
                    brighness = 20;
                }
            }
            if (setModes == SETUP_COLOR) {
                colorIndex--;
                if (colorIndex == -1) {
                    colorIndex = 0;
                }
            }
        }

        matrix.setTextColor(colors[colorIndex]);
        int b = map(brighness, 0, 20, 0, 255);
        matrix.setBrightness(b);
        matrix.setCursor(0, 0);
        matrix.print(str);
        matrix.show();

        if (digitalRead(10) == LOW) {
            counter = 0;
            setModes++;
            if (setModes == SETUP_NONE) {
                EEPROM.write(0, brighness);
                EEPROM.write(1, colorIndex);
                delay(1000);
                break;
            }
        }

        counter++;
        delay(300);
    }
}


void handleScore() {

    if (score.first < 10) {
        sprintf(str, "% 2d:%d", score.first, score.second);
    } else {
        sprintf(str, "%2d:%d", score.first, score.second);
    }

    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print(str);

    if (set == SCORE_MINUS) {
        matrix.drawLine(0, 7, 29, 7, BLUE);
    }
    matrix.show();

    if (digitalRead(10) == LOW) {
        if (set == SCORE_MINUS) {
            score.first--;
        } else
        if (set == SCORE_NONE) {
            score.first++;
        }
    }

    if (digitalRead(9) == LOW) {
        set++;
        if (set == SCORE_END) {
            set = SCORE_NONE;
        }
    }

    if (digitalRead(8) == LOW) {
        if (set == SCORE_MINUS) {
            score.second--;
        } else
        if (set == SCORE_NONE) {
            score.second++;
        }
    }

    if (debounce(&PIND, PD3) == HIGH) {
        //if (digitalRead(3) == HIGH) {
        score.first--;
    }
    if (debounce(&PIND, PD4) == HIGH) {
        //if (digitalRead(4) == HIGH) {
        score.second--;
    }
    if (debounce(&PIND, PD5) == HIGH) {
        //if (digitalRead(5) == HIGH) {
        score.first++;
    }
    if (debounce(&PIND, PD6) == HIGH) {
        //if (digitalRead(6) == HIGH) {
        score.second++;
    }

    if (score.second > 99) {
        score.second = 0;
    }
    if (score.second < 0) {
        score.second = 0;
    }
}

void loop() {
    switch (mode) {

    case MODE_SCORE:
        handleScore();
        break;
    case MODE_TIME:
        //handleTime();
        break;
    }
    delay(500);
}
