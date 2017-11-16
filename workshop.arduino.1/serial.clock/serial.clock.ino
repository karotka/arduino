/**
 * Date and time functions using a DS3231 RTC connected
 * via I2C and Wire lib. Prepeare for Arduino workshop.
 */
#include <Wire.h>
#include "RTClib.h"

// instance of RTC library
RTC_DS3231 rtc;

// define day of the week
char daysOfTheWeek[7][12] = {
    "Nedele", "Pondeli", "Utery", "Streda",
    "Ctvrtek", "Patek", "Sobota"};

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

    // print into the console
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(' ');
    Serial.print(now.day(), DEC);
    Serial.print('.');
    Serial.print(now.month(), DEC);
    Serial.print('.');
    Serial.print(now.year(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    delay(1000);
}