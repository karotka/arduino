#include <Wire.h>
#include "U8glib.h"

#define VREF 5.0

char buf[15];
char fl[11];

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

void setup(void) {
    u8g.setColorIndex(1); // pixel on
    u8g.setHiColorByRGB(255,255,255);
    u8g.setFont(u8g_font_unifont_0_8);
}

void loop(void) {

    int sensorValue = analogRead(A0);
    float voltage = sensorValue * (VREF / 1023.0);

    // picture loop
    u8g.firstPage();
    do {

        dtostrf(voltage, 4, 2, fl);
        sprintf(buf, "Sensor A0: %s", voltage);
        u8g.drawStr(0, 11, buf);

    } while( u8g.nextPage() );
}
