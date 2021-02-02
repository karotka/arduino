/**
 *   DebugUtils.h - Simple Serial debugging utilities
 */

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <stdarg.h>

#ifdef DEBUG

    #define SLOG(str) \
        Serial.print(str);              \
        Serial.print(" ");

    #define SLOGLN(str)                       \
        Serial.print(str);              \
        Serial.print(" <");    \
        Serial.print(millis());     \
        Serial.print(":");    \
        Serial.print(__PRETTY_FUNCTION__); \
        Serial.print(':');  \
        Serial.print(__LINE__); \
        Serial.println('>');

    #define SLOGF(format, ...)  \
        Serial.printf(format, __VA_ARGS__);             \
        Serial.print(" <");    \
        Serial.print(millis());     \
        Serial.print(":");    \
        Serial.print(__PRETTY_FUNCTION__); \
        Serial.print(':');  \
        Serial.print(__LINE__); \
        Serial.println('>');

#else
    #define SLOG(str)
    #define SLOFLN(str)
    #define SLOGF(format, ...)
#endif

#endif
