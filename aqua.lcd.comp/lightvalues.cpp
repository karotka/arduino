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

void LigthValues_t::setCoolByte(uint8_t value) {
    coolByte = value;
    coolValue = map(value, 0, 255, X_TOUCH_AREA_MIN, X_TOUCH_AREA_MAX);
    analogWrite(LED_COOL_WHITE, value);
}

void LigthValues_t::setWarmValue(int value) {
    warmValue = value;
    warmByte = map(value, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN, 0, 255);
    analogWrite(LED_WHITE, warmByte);
}

void LigthValues_t::setWarmByte(uint8_t value) {
    warmByte = value;
    warmValue = map(value, 0, 255, X_TOUCH_AREA_MIN, X_TOUCH_AREA_MAX);
    analogWrite(LED_COOL_WHITE, value);
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
    if (flag == TM_OFF) {
        EEPROM.write(64, coolByte);
        EEPROM.write(65, warmByte);
        EEPROM.write(66, yellowByte);
        EEPROM.write(67, redByte);
        EEPROM.write(68, greenByte);
        EEPROM.write(69, blueByte);
    }

    if (flag == TM_DAY1) {
        EEPROM.write(70, coolByte);
        EEPROM.write(71, warmByte);
        EEPROM.write(72, yellowByte);
        EEPROM.write(73, redByte);
        EEPROM.write(74, greenByte);
        EEPROM.write(75, blueByte);
    }

    if (flag == TM_DAY2) {
        EEPROM.write(76, coolByte);
        EEPROM.write(77, warmByte);
        EEPROM.write(78, yellowByte);
        EEPROM.write(79, redByte);
        EEPROM.write(80, greenByte);
        EEPROM.write(81, blueByte);
    }

    if (flag == TM_NIGHT1) {
        EEPROM.write(82, coolByte);
        EEPROM.write(83, warmByte);
        EEPROM.write(84, yellowByte);
        EEPROM.write(85, redByte);
        EEPROM.write(86, greenByte);
        EEPROM.write(87, blueByte);
    }

    if (flag == TM_NIGHT2) {
        EEPROM.write(88, coolByte);
        EEPROM.write(89, warmByte);
        EEPROM.write(90, yellowByte);
        EEPROM.write(91, redByte);
        EEPROM.write(92, greenByte);
        EEPROM.write(93, blueByte);
    }
}

void LigthValues_t::load(void) {
    if (flag == TM_OFF) {
        coolByte   = EEPROM.read(64);
        warmByte   = EEPROM.read(65);
        yellowByte = EEPROM.read(66);
        redByte    = EEPROM.read(67);
        greenByte  = EEPROM.read(68);
        blueByte   = EEPROM.read(69);
    }

    if (flag == TM_DAY1) {
        coolByte   = EEPROM.read(70);
        warmByte   = EEPROM.read(71);
        yellowByte = EEPROM.read(72);
        redByte    = EEPROM.read(73);
        greenByte  = EEPROM.read(74);
        blueByte   = EEPROM.read(75);
    }

    if (flag == TM_DAY2) {
        coolByte   = EEPROM.read(76);
        warmByte   = EEPROM.read(77);
        yellowByte = EEPROM.read(78);
        redByte    = EEPROM.read(79);
        greenByte  = EEPROM.read(80);
        blueByte   = EEPROM.read(81);
    }

    if (flag == TM_NIGHT1) {
        coolByte   = EEPROM.read(82);
        warmByte   = EEPROM.read(83);
        yellowByte = EEPROM.read(84);
        redByte    = EEPROM.read(85);
        greenByte  = EEPROM.read(86);
        blueByte   = EEPROM.read(87);
    }

    if (flag == TM_NIGHT2) {
        coolByte   = EEPROM.read(88);
        warmByte   = EEPROM.read(89);
        yellowByte = EEPROM.read(90);
        redByte    = EEPROM.read(91);
        greenByte  = EEPROM.read(92);
        blueByte   = EEPROM.read(93);
    }

    coolValue   = map(coolByte,   0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    warmValue   = map(warmByte,   0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    yellowValue = map(yellowByte, 0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    redValue    = map(redByte,    0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    greenValue  = map(greenByte,  0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
    blueValue   = map(blueByte,   0, 255, X_TOUCH_AREA_MAX, X_TOUCH_AREA_MIN);
}
