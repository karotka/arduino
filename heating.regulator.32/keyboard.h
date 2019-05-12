//This example implements a simple sliding On/Off button. The example
// demonstrates drawing and touch operations.
//
//Thanks to Adafruit forums member Asteroid for the original sketch!
//
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>

// This is calibration data for the raw touch data to the screen coordinates

#define isWithin(x, a, b) ((x>=a)&&(x<=b))

#define Height 199

#define TextColor WHITE // white
#define TextBackColor BACK_COLOR // red
#define TFTBackground BACK_COLOR

const char KB[3][13] PROGMEM = {
    {0, 13, 10, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
    {1, 12, 9, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'},
    {3, 10, 7, 'Z', 'X', 'C', 'V', 'B', 'N', 'M'},
};

const char KB_NumKeys[3][13] PROGMEM = {
    {0, 13, 10, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {0, 13, 10, '-', '/', ':', ';', '(', ')', '$', '&', '@', '"'},
    {5, 8, 5, '.', '\,', '?', '!', '\''}
};

const char KB_SymKeys[3][13] PROGMEM = {
    {0, 13, 10, '[', ']', '{', '}', '#', '%', '^', '*', '+', '='},
    {4, 9, 6, '_', '\\', '|', '~', '<', '>'}, //4
    {5, 8, 5, '.', '\,', '?', '!', '\''}
};

const char textLimit = 20;
static bool shift = false, special = false;

byte TouchButton(int x, int y, int w, int h, uint16_t touchX, uint16_t touchY) {
    return (isWithin(touchX, x, x + w) & isWithin(touchY, y, y + h));
}

void drawButton(int x, int y, int w, int h) {
    // white
    tft.drawRect(x, y, w, h, YELLOW);// outter button color
}

void MakeKB_Button(const char type[][13]) {
    tft.setTextSize(2);
    tft.setTextColor(TextColor, TextBackColor);
    for (int y = 0; y < 3; y++) {
        int ShiftRight = 10 * pgm_read_byte(&(type[y][0]));
        for (int x = 3; x < 13; x++) {
            if (x >= pgm_read_byte(&(type[y][1]))) break;

            drawButton(6 + (23 * (x - 3)) + ShiftRight, Height + (30 * y), 24, 29); // this will draw the button on the screen by so many pixels
            tft.setCursor(12 + (23 * (x - 3)) + ShiftRight, Height + 8 + (30 * y));
            tft.print(char(pgm_read_byte(&(type[y][x]))));
        }
    }
    //ShiftKey
    drawButton(5, Height + 60, 30, 29);
    tft.setCursor(15, Height + 70);
    tft.print(F("^"));

    //Special Characters
    drawButton(5, Height + 90, 30, 29);
    tft.setCursor(9, Height + 98);
    tft.print(F("Sh"));

    //BackSpace
    drawButton(205, Height + 60, 30, 29);
    tft.setCursor(209, Height + 68);
    tft.print(F("Bs"));

    //Enter
    drawButton(190, Height + 90, 45, 29);
    tft.setCursor(195, Height + 98);
    tft.print(F("Enr"));

    // <
    drawButton(37, Height + 90, 32, 29);
    tft.setCursor(45, Height + 98);
    tft.print(F("<"));

    //Spacebar
    drawButton(65, Height + 90, 90, 29);
    tft.setCursor(80, Height + 99);
    tft.print(F("Space"));

    // >
    drawButton(156, Height + 90, 32, 29);
    tft.setCursor(163, Height + 98);
    tft.print(F(">"));
}

boolean getKeyPress(int *ch, uint16_t touchX, uint16_t touchY) {
    static bool back = false, lastSp = false, lastSh = false;

    // ShiftKey
    if (TouchButton(5, Height + 60, 30, 29, touchX, touchY)) {
        shift = !shift;
    }

    // Special Characters
    if (TouchButton(5, Height + 90, 30, 29, touchX, touchY)) {
        special = !special;
    }

    if (special != lastSp || shift != lastSh) {
        if (special) {
            if (shift) {
                tft.fillRect(4, Height, 233, 110, BACK_COLOR);
                MakeKB_Button(KB_SymKeys);
            } else {
                tft.fillRect(4, Height, 233, 110, BACK_COLOR);
                MakeKB_Button(KB_NumKeys);
            }
        } else {
            tft.fillRect(4, Height, 233, 110, BACK_COLOR);
            MakeKB_Button(KB);
            tft.setTextColor(TextColor, TextBackColor);
        }

        if (special)
            tft.setTextColor(RED, TextBackColor);
        else
            tft.setTextColor(TextColor, TextBackColor);

        tft.setCursor(9, Height + 98);
        tft.print(F("SP"));

        if (shift)
            tft.setTextColor(RED, TextBackColor);
        else
            tft.setTextColor(TextColor, TextBackColor);

        tft.setCursor(15, Height + 70);
        tft.print('^');

        lastSh = shift;
        lastSp = special;
        lastSh = shift;
    }

    for (int y = 0; y < 3; y++) {
        int ShiftRight;
        if (special) {
            if (shift)
                ShiftRight = 10 * pgm_read_byte(&(KB_SymKeys[y][0]));
            else
                ShiftRight = 10 * pgm_read_byte(&(KB_NumKeys[y][0]));
        } else
            ShiftRight = 10 * pgm_read_byte(&(KB[y][0]));

        for (int x = 3; x < 13; x++) {
            if (x >=  (special ? (shift ? pgm_read_byte(&(KB_SymKeys[y][1])) : pgm_read_byte(&(KB_NumKeys[y][1]))) : pgm_read_byte(&(KB[y][1])) )) break;

            if (TouchButton(10 + (23 * (x - 3)) + ShiftRight,
                            Height + 4 + (30 * y), 15, 20, touchX, touchY)) { // this will draw the button on the screen by so many pixels

                //tft.drawRect(10 + (23 * (x - 3)) + ShiftRight, Height + 4 + (30 * y), 15, 20, ILI9341_RED);
                if (special) {
                    if (shift) {
                        *ch = (int)pgm_read_byte(&(KB_SymKeys[y][x]));
                        return true;
                    } else {
                        *ch = (int)pgm_read_byte(&(KB_NumKeys[y][x]));
                        return true;
                    }
                } else {
                    *ch = (int)(pgm_read_byte(&(KB[y][x])) + (shift ? 0 : ('a' - 'A')));
                    return true;
                }
            }
        }
    }

    // shif in <
    if (TouchButton(37, Height + 90, 30, 25, touchX, touchY)) {
        *ch = 0xf;
        return true;
    } else

    // Spacebar
    if (TouchButton(65, Height + 90, 90, 25, touchX, touchY)) {
        *ch = 0x20;
        return true;
    } else

    // shif out >
    if (TouchButton(156, Height + 90, 30, 25, touchX, touchY)) {
        *ch = 0xe;
        return true;
    } else

    // BackSpace
    if (TouchButton(200, Height + 60, 30, 25, touchX, touchY)) {
        *ch = 0x8;
        return true;
    } else

    // Enter
    if (TouchButton(190, Height + 90, 30, 25, touchX, touchY)) {
        *ch = 0x12;
        return true;
    }
    return false;;
}
