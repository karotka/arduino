#include <TFT_eSPI.h>
#include <Buttons.h>

TFT_eSPI tft = TFT_eSPI(420, 380);
Buttons btns(&tft);

uint16_t x, y;

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nStarting...");

    tft.init();


    tft.setRotation(1);
    tft.setTextFont(4);

    tft.fillScreen(TFT_BLACK);

    tft.drawLine(0, 100, 100, 100, TFT_WHITE);

    btns.addSelect(100, 100, 50, 50, "La", TFT_DARKORANGE);
    btns.drawButtons();
}

void loop() {
    if (tft.getTouch(&x, &y)) {

        button_t btn = btns.checkButtons(x, y);

        Serial.print("ID:");
        Serial.println(btn.id);

        if (btn.flags & BUTTON_TOUCH_UP) {
            Serial.println("UP");
        } else
        if (btn.flags & BUTTON_TOUCH_DOWN) {
            Serial.println("DOWN");
        }

    }
    delay(250);
}


