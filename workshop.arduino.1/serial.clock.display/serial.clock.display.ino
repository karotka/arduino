/**
 * Date and time functions using a DS3231 RTC connected
 * via I2C and Wire lib. Prepeare for Arduino workshop.
 */
#include <Wire.h>
#include "RTClib.h"
#include "U8glib.h"

// instance of RTC library
RTC_DS3231 rtc;

// instance of display u8g library
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

// buffer for sprintf
char buf[10];

void setup () {
    // we will need to write output into the console
    Serial.begin(9600);

    // wait for console opening
    delay(1000);

    // check if RTC working
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    /* RTC adjust if necessary format is
       year, month, day, hour, minutes, second */
    //rtc.adjust(DateTime(2017, 9, 1, 20, 16, 0));
}

// main loop
void loop () {
    // get actual time from rtc
    DateTime now = rtc.now();
       
    // picture loop
    u8g.firstPage();
    do {
        // draw buffer using u8g library
        // build time char
        u8g.setFont(u8g_font_osb21);
        sprintf(buf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        u8g.drawStr(5, 30, buf);

        u8g.setFont(u8g_font_unifont_0_8);
        sprintf(buf, "%02d.%02d.%04d", now.day(), now.month(), now.year());
        u8g.drawStr(20, 45, buf);

    } while( u8g.nextPage() );

    delay(200);
}