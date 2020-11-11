// slider
// |-b.x Label |---- b.x + 150 ->| width        b.x + 150 + b.width ->|-|
// | Channel x                   |--------------|---=-----------------|-|
//                     length = x - b.x + 150 ->

#include "Buttons.h"

Buttons::Buttons(TFT_eSPI *tft) {
    _tft = tft;
    btncnt = 0;
	//deleteAllButtons();
}

int Buttons::addSlider(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       uint16_t value, const char *label, uint16_t color, uint16_t textColor) {

    int _btncnt = btncnt;

    buttons[btncnt].x = x;
    buttons[btncnt].y = y;
    buttons[btncnt].value = value;
    buttons[btncnt].width = width;
    buttons[btncnt].height = height;
    buttons[btncnt].color = color;
    buttons[btncnt].textColor = textColor;
    buttons[btncnt].label = strdup(label);
    buttons[btncnt].flags = BUTTON_TYPE_SLIDER;

    btncnt++;
    return _btncnt;
}

int Buttons::addSelect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       int label, uint16_t color, uint16_t textColor) {
    char str[4];
    sprintf(str, format, label);

    addSelect(x, y, width, height, str, color, textColor);
}

int Buttons::addSelect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       const char *label, uint16_t color, uint16_t textColor) {

    int _btncnt = btncnt;

    buttons[btncnt].x = x;
    buttons[btncnt].y = y;
    buttons[btncnt].width = width;
    buttons[btncnt].height = height;
    buttons[btncnt].flags = BUTTON_TYPE_SELECT;
    buttons[btncnt].color = color;
    buttons[btncnt].textColor = textColor;
    buttons[btncnt].label = strdup(label);

    btncnt++;
    return _btncnt;
}

int Buttons::addButton(uint16_t x, uint16_t y, uint16_t width,
                       uint16_t height, int label,
                       uint16_t color, uint16_t textColor) {
    char str[4];
    sprintf(str, format, label);

    addButton(x, y, width, height, str, color, textColor);
}

int Buttons::addButton(uint16_t x, uint16_t y, uint16_t width,
                       uint16_t height, const char *label,
                       uint16_t color, uint16_t textColor) {
    int _btncnt = btncnt;

    buttons[btncnt].x = x;
    buttons[btncnt].y = y;
    buttons[btncnt].width = width;
    buttons[btncnt].height = height;
    buttons[btncnt].color = color;
    buttons[btncnt].textColor = textColor;
    buttons[btncnt].label = label;
    buttons[btncnt].flags = BUTTON_TYPE_BUTTON;

    btncnt++;
    return _btncnt;
}

int Buttons::addCheckbox(uint16_t x, uint16_t y, const char *label,
                        uint16_t color, uint16_t textColor) {
    int _btncnt = btncnt;

    buttons[btncnt].x = x;
    buttons[btncnt].y = y;
    buttons[btncnt].width = 30;
    buttons[btncnt].height = 30;
    buttons[btncnt].color = color;
    buttons[btncnt].textColor = textColor;
    buttons[btncnt].label = label;
    buttons[btncnt].flags = BUTTON_TYPE_CHECKBOX;

    btncnt++;
    return _btncnt;
}

void Buttons::draw() {
	for (int i = 0; i < MAX_BUTTONS; i++) {
        if (buttons[i].flags & BUTTON_TYPE_SELECT) {
            drawSelect(i);
        } else
        if (buttons[i].flags & BUTTON_TYPE_BUTTON) {
            drawButton(i);
        } else
        if (buttons[i].flags & BUTTON_TYPE_SLIDER) {
            drawSlider(i);
        } else
        if (buttons[i].flags & BUTTON_TYPE_CHECKBOX) {
            drawCheckbox(i);
        }
	}
}

void Buttons::drawButtonLabel(int id) {
    _tft->setTextSize(1);

    int xPos = (buttons[id].width - _tft->textWidth(buttons[id].label, 4)) / 2;
    int yPos = (buttons[id].height + 2 - _tft->fontHeight(4)) / 2;

    _tft->setCursor(buttons[id].x + xPos, buttons[id].y + yPos, 4);
    _tft->setTextColor(buttons[id].textColor, buttons[id].color);
    _tft->println(buttons[id].label);
}

void Buttons::drawSelect(int id) {

    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                   buttons[id].height, buttons[id].color);

    _tft->drawLine(buttons[id].x, buttons[id].y - 3,
                   buttons[id].x + buttons[id].width / 2,
                   buttons[id].y - buttons[id].height / 2, TFT_WHITE);
    _tft->drawLine(buttons[id].x + buttons[id].width / 2,
                   buttons[id].y - buttons[id].height / 2,
                   buttons[id].x + buttons[id].width,
                   buttons[id].y - 3,
                   TFT_WHITE);

    _tft->drawLine(buttons[id].x, buttons[id].y + buttons[id].height + 3,
                   buttons[id].x + buttons[id].width / 2,
                   buttons[id].y + 3 + buttons[id].height + buttons[id].height / 2,
                   TFT_WHITE);
    _tft->drawLine(buttons[id].x + buttons[id].width / 2,
                   buttons[id].y + 3 + buttons[id].height + buttons[id].height / 2,
                   buttons[id].x + buttons[id].width,
                   buttons[id].y + buttons[id].height + 3,
                   TFT_WHITE);

    drawButtonLabel(id);
}

void Buttons::drawButton(int id) {

    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                   buttons[id].height, buttons[id].color);

    drawButtonLabel(id);
}

void Buttons::drawSlider(int id) {
    int slStartX =  buttons[id].x + 150;

    _tft->setTextColor(buttons[id].textColor, buttons[id].color);
    _tft->drawString(buttons[id].label, buttons[id].x, buttons[id].y);

    _tft->drawRect(slStartX, buttons[id].y + 4, buttons[id].width,
                   buttons[id].height, TFT_WHITE);
    _tft->fillRect(slStartX + 1, buttons[id].y + 5,
                   buttons[id].value, buttons[id].height - 2, TFT_LIGHTGREY);

    _tft->fillRect(slStartX + buttons[id].value, buttons[id].y + 5,
                   buttons[id].width - buttons[id].value - 1,
                   buttons[id].height - 2, TFT_BLACK);
}

void Buttons::drawCheckbox(int id) {
    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                       buttons[id].height, buttons[id].color);

    if (buttons[id].flags & BUTTON_CHECKBOX_CHECKED) {
        _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                       buttons[id].height, buttons[id].color);

        _tft->drawLine(buttons[id].x, buttons[id].y + (buttons[id].height / 2),
                       buttons[id].x + (buttons[id].height / 2),
                       buttons[id].y + buttons[id].height - 1, buttons[id].textColor);

        _tft->drawLine(buttons[id].x, buttons[id].y + (buttons[id].height / 2) - 1,
                       buttons[id].x + (buttons[id].height / 2),
                       buttons[id].y + buttons[id].height - 2, buttons[id].textColor);

        _tft->drawLine(buttons[id].x + (buttons[id].width / 2),
                       buttons[id].y + buttons[id].height - 1,
                       buttons[id].x + buttons[id].width - 1,
                       buttons[id].y,
                       buttons[id].textColor);
        _tft->drawLine(buttons[id].x + (buttons[id].width / 2) + 1,
                       buttons[id].y + buttons[id].height - 1,
                       buttons[id].x + buttons[id].width - 1,
                       buttons[id].y + 1,
                       buttons[id].textColor);

        /*
        _tft->drawLine(buttons[id].x + (buttons[id].width / 2),
                      buttons[id].y + buttons[id].height - 1,
                      buttons[id].x + buttons[id].width,
                      buttons[id].y - 1,
                      buttons[id].textColor);
        */

    } else {
        _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                       buttons[id].height, buttons[id].color);
    }
    _tft->setTextSize(1);
    _tft->setCursor(buttons[id].x + 35,
                    (buttons[id].y + 2 + ((buttons[id].height - _tft->fontHeight(4)) / 2)), 4);
    _tft->setTextColor(buttons[id].textColor);
    _tft->println(buttons[id].label);
}

void Buttons::relableButton(int id, const char *label) {
    buttons[id].label = label;

    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                   buttons[id].height, buttons[id].color);

    drawButtonLabel(id);
}

void Buttons::relableButton(int id, int label) {

    char str[4];
    sprintf(str, format, label);

    buttons[id].label = str;

    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                   buttons[id].height, buttons[id].color);

    drawButtonLabel(id);
}

bool Buttons::buttonEnabled(int id) {
	return true;
}

void Buttons::deleteButton(int id) {
	buttons[id].x = 0;
	buttons[id].y = 0;
	buttons[id].width = 0;
	buttons[id].height = 0;
	buttons[id].flags = 0;
	buttons[id].label = "";
}

void Buttons::deleteAllButtons() {
    btncnt = 0;
	for (int i = 0; i < MAX_BUTTONS; i++) {
		buttons[i].x = 0;
		buttons[i].y = 0;
		buttons[i].width = 0;
		buttons[i].height = 0;
		buttons[i].flags = 0;
		buttons[i].label = "";
	}
}

int Buttons::checkButtons(uint16_t x, uint16_t y) {
    PR("Touch x:");PR(x); PR(" y:"); PRN(y);

	for (int i = 0; i < MAX_BUTTONS; i++) {

        if (buttons[i].flags & BUTTON_DISABLED) {
            continue;
        }

        buttons[i].flags &= ~BUTTON_TOUCH_UP;
        buttons[i].flags &= ~BUTTON_TOUCH_DOWN;
        buttons[i].flags &= ~BUTTON_TOUCH_SLIDE;

        if (buttons[i].flags & BUTTON_TYPE_SELECT) {

            if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width &&
                y >= buttons[i].y - buttons[i].height / 2 &&
                y <= buttons[i].y + buttons[i].height / 2) {

                //PRN("UP");
                //PR("x>="); PR(buttons[i].x); PR("x<="); PRN(buttons[i].x + buttons[i].width);
                //PR("y>="); PR(buttons[i].y - buttons[i].height / 2); PR("y<="); PRN(buttons[i].y + buttons[i].height / 2);

                buttons[i].flags |= BUTTON_TOUCH_UP;

                return i;
            } else

            if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width &&
                y >= buttons[i].y + buttons[i].height / 2 &&
                y <= buttons[i].y + buttons[i].height + buttons[i].height / 2) {

                //PRN("DOWN");
                //PR("x>="); PR(buttons[i].x); PR("x<="); PRN(buttons[i].x + buttons[i].width);
                //PR("y>="); PR(buttons[i].y + buttons[i].height / 2); PR("y<="); PRN(buttons[i].y + buttons[i].height + buttons[i].height / 2);

                buttons[i].flags |= BUTTON_TOUCH_DOWN;

                return i;
            }

        } else

        if (buttons[i].flags & BUTTON_TYPE_BUTTON) {

            if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width &&
                y >= buttons[i].y && y <= buttons[i].y + buttons[i].height) {

                PRN("TOUCH");

                buttons[i].flags |= BUTTON_TOUCH_DOWN;

                return i;
            }

        } else

        if(buttons[i].flags & BUTTON_TYPE_SLIDER) {
                                                         ;
            if (x >= buttons[i].x + 150 && x <= buttons[i].x + 150 + buttons[i].width &&
                y >= buttons[i].y &&
                y <= buttons[i].y + buttons[i].height * 2) {

                PRN("SLIDE");
                PR("x:"); PR(x); PR(" y:"); PRN(y);

                //buttons[i].value =  x - buttons[i].x + 150;

                buttons[i].flags |= BUTTON_TOUCH_SLIDE;

                buttons[i].value = x - (buttons[i].x + 150);
                _tft->fillRect(buttons[i].x + 151, buttons[i].y + 5,
                               buttons[i].width - 2, buttons[i].height - 2, TFT_BLACK);
                _tft->fillRect(buttons[i].x + 151, buttons[i].y + 5,
                               buttons[i].value, buttons[i].height - 2, TFT_LIGHTGREY);
                return i;
            }
        } else
        if (buttons[i].flags & BUTTON_TYPE_CHECKBOX) {

            if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width &&
                y >= buttons[i].y && y <= buttons[i].y + buttons[i].height) {


                //buttons[i].flags |= BUTTON_TOUCH_DOWN;
                buttons[i].flags ^= BUTTON_CHECKBOX_CHECKED;
                PR("TOUCH"); PRBN(buttons[i].flags);

                drawCheckbox(i);

                return i;
            }
        }
    }
    return -1;
}

void Buttons::enable(int id) {
    buttons[id].flags &= ~BUTTON_DISABLED;
    relableButton(id, buttons[id].label);
}

void Buttons::disable(int id) {
    buttons[id].flags |= BUTTON_DISABLED;
    relableButton(id, buttons[id].label);
}

button_t Buttons::getButton(int id) {
    return buttons[id];
}

void Buttons::setColor(int id, uint16_t color) {
    buttons[id].color = color;
}

void Buttons::setSlider(int id, uint16_t value) {

    buttons[id].value = value;
    _tft->fillRect(buttons[id].x + 151, buttons[id].y + 5,
                   buttons[id].width - 2, buttons[id].height - 2, TFT_BLACK);
    _tft->fillRect(buttons[id].x + 151, buttons[id].y + 5,
                   buttons[id].value, buttons[id].height - 2, TFT_LIGHTGREY);

}
