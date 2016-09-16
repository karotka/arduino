#include "lightvalues.h"

//LigthValues_t::LigthValues_t() {}

LigthValues_t::LigthValues_t(uint8_t flag) : flag(flag) {
    //flag = flag;
}

void LigthValues_t::setCoolValue(int value) {
    coolValue = value;
    coolByte = map(value, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN, 0, 255);
    analogWrite(LED_COOL_WHITE, coolByte);
}

void LigthValues_t::setWarmValue(int value) {
    warmValue = value;
    warmByte = map(value, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN, 0, 255);
    analogWrite(LED_WHITE, warmByte);
}

void LigthValues_t::setYellowValue(int value) {
    yellowValue = value;
    yellowByte = map(value, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN, 0, 255);
    analogWrite(LED_YELLOW, yellowByte);
}

void LigthValues_t::setRedValue(int value) {
    redValue = value;
    redByte = map(value, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN, 0, 255);
    analogWrite(LED_RED, redByte);
}

void LigthValues_t::setGreenValue(int value) {
    greenValue = value;
    greenByte = map(value, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN, 0, 255);
    analogWrite(LED_GREEN, greenByte);
}

void LigthValues_t::setBlueValue(int value) {
    blueValue = value;
    blueByte = map(value, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN, 0, 255);
    analogWrite(LED_BLUE, blueByte);
}

void LigthValues_t::save(void) {
    if (flag == MODE_OFF) {
        EEPROM.write(64, coolByte);
        EEPROM.write(65, warmByte);
        EEPROM.write(66, yellowByte);
        EEPROM.write(67, redByte);
        EEPROM.write(68, greenByte);
        EEPROM.write(69, blueByte);
    }

    if (flag == MODE_DAY) {
        EEPROM.write(70, coolByte);
        EEPROM.write(71, warmByte);
        EEPROM.write(72, yellowByte);
        EEPROM.write(73, redByte);
        EEPROM.write(74, greenByte);
        EEPROM.write(75, blueByte);
    }

    if (flag == MODE_NIGHT) {
        EEPROM.write(76, coolByte);
        EEPROM.write(77, warmByte);
        EEPROM.write(78, yellowByte);
        EEPROM.write(79, redByte);
        EEPROM.write(80, greenByte);
        EEPROM.write(81, blueByte);
    }
}

void LigthValues_t::load(void) {
    if (flag == MODE_OFF) {
        coolByte   = EEPROM.read(64);
        warmByte   = EEPROM.read(65);
        yellowByte = EEPROM.read(66);
        redByte    = EEPROM.read(67);
        greenByte  = EEPROM.read(68);
        blueByte   = EEPROM.read(69);
    }

    if (flag == MODE_DAY) {
        coolByte   = EEPROM.read(70);
        warmByte   = EEPROM.read(71);
        yellowByte = EEPROM.read(72);
        redByte    = EEPROM.read(73);
        greenByte  = EEPROM.read(74);
        blueByte   = EEPROM.read(75);
    }

    if (flag == MODE_NIGHT) {
        coolByte   = EEPROM.read(76);
        warmByte   = EEPROM.read(77);
        yellowByte = EEPROM.read(78);
        redByte    = EEPROM.read(79);
        greenByte  = EEPROM.read(80);
        blueByte   = EEPROM.read(81);
    }

    coolValue   = map(coolByte,   0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    warmValue   = map(warmByte,   0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    yellowValue = map(yellowByte, 0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    redValue    = map(redByte,    0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    greenValue  = map(greenByte,  0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    blueValue   = map(blueByte,   0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
}