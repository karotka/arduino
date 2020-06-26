
#include "Buttons.h"

Buttons::Buttons(TFT_eSPI *tft) {
    _tft = tft;
    btncnt = 0;
	//deleteAllButtons();
}

int Buttons::addSlider(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       uint16_t value, const char *label, uint16_t color) {

    int _btncnt = btncnt;

    buttons[btncnt].x = x;
    buttons[btncnt].y = y;
    buttons[btncnt].value = value;
    buttons[btncnt].width = width;
    buttons[btncnt].height = height;
    buttons[btncnt].flags = BUTTON_TYPE_SLIDER;
    buttons[btncnt].color = color;
    buttons[btncnt].label = strdup(label);;
    btncnt++;
    return _btncnt;
}

int Buttons::addSelect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       const char *label, uint16_t color) {

    int _btncnt = btncnt;

    buttons[btncnt].x = x;
    buttons[btncnt].y = y;
    buttons[btncnt].width = width;
    buttons[btncnt].height = height;
    buttons[btncnt].flags = BUTTON_TYPE_SELECT;
    buttons[btncnt].color = color;
    buttons[btncnt].label = strdup(label);

    btncnt++;
    return _btncnt;
}

int Buttons::addButton(uint16_t x, uint16_t y, uint16_t width,
                       uint16_t height, const char *label,
                       uint16_t color) {
    int _btncnt = btncnt;

    buttons[btncnt].x = x;
    buttons[btncnt].y = y;
    buttons[btncnt].width = width;
    buttons[btncnt].height = height;
    buttons[btncnt].color = color;
    buttons[btncnt].label = label;
    buttons[btncnt].flags = BUTTON_TYPE_BUTTON;

    btncnt++;
    return _btncnt;
}

void Buttons::draw() {
	for (int i = 0; i < MAX_BUTTONS; i++) {
        if (buttons[i].flags & BUTTON_TYPE_SELECT) {
            //PRN("Draw select");
            drawSelect(i);
        } else
        if (buttons[i].flags & BUTTON_TYPE_BUTTON) {
            //PRN("Draw button");
            drawButton(i);
        } else
        if (buttons[i].flags & BUTTON_TYPE_SLIDER) {
            //PRN("Draw button");
            drawSlider(i);
        }
	}
}

void Buttons::drawButtonLabel(int id, const char *label, uint16_t bgColor) {
    //PR("ID:"); PR(id); PR(" X:"); PR(buttons[id].x + 10); PR(" Y:"); PRN(buttons[id].y + 12);
    _tft->setTextSize(1);
    _tft->setCursor(buttons[id].x + 10, buttons[id].y + 12, 4);
    _tft->setTextColor(TFT_WHITE, bgColor);
    _tft->println(label);
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

    _tft->setTextSize(1);
    _tft->setCursor(buttons[id].x + 10, buttons[id].y + 12, 4);
    _tft->setTextColor(TFT_WHITE, buttons[id].color);
    _tft->println(buttons[id].label);
}

void Buttons::drawButton(int id) {

    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                   buttons[id].height, buttons[id].color);

    drawButtonLabel(id, buttons[id].label, buttons[id].color);
}

void Buttons::drawSlider(int id) {
    int slStartX =  buttons[id].x + 150;

    _tft->drawString(buttons[id].label, buttons[id].x, buttons[id].y);

    _tft->drawRect(slStartX, buttons[id].y + 4, buttons[id].width,
                   buttons[id].height, TFT_WHITE);
    _tft->fillRect(slStartX + 1, buttons[id].y + 5, buttons[id].value,
                   buttons[id].height - 2, TFT_LIGHTGREY);
}

void Buttons::relableButton(int id, const char *label, uint16_t bgColor) {
    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                   buttons[id].height, bgColor);

    drawButtonLabel(id, label, bgColor);
}

void Buttons::relableButton(int id, int label, uint16_t bgColor) {

    char str[4];
    sprintf(str, "%2d", label);
    _tft->fillRect(buttons[id].x, buttons[id].y, buttons[id].width,
                   buttons[id].height, bgColor);

    drawButtonLabel(id, str, bgColor);
}

bool Buttons::buttonEnabled(int id) {
	return true;
}

void Buttons::deleteButton(int id) {
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

        buttons[i].flags &= ~BUTTON_TOUCH_UP;
        buttons[i].flags &= ~BUTTON_TOUCH_DOWN;
        buttons[i].flags &= ~BUTTON_TOUCH_SLIDE;

        if (buttons[i].flags & BUTTON_TYPE_SELECT) {

            if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width &&
                y >= buttons[i].y - buttons[i].height / 2 &&
                y <= buttons[i].y + buttons[i].height / 2) {

                PRN("UP");
                //PR("x>="); PR(buttons[i].x); PR("x<="); PRN(buttons[i].x + buttons[i].width);
                //PR("y>="); PR(buttons[i].y - buttons[i].height / 2); PR("y<="); PRN(buttons[i].y + buttons[i].height / 2);

                buttons[i].flags |= BUTTON_TOUCH_UP;

                return i;
            } else

            if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width &&
                y >= buttons[i].y + buttons[i].height / 2 &&
                y <= buttons[i].y + buttons[i].height + buttons[i].height / 2) {

                PRN("DOWN");
                //PR("x>="); PR(buttons[i].x); PR("x<="); PRN(buttons[i].x + buttons[i].width);
                //PR("y>="); PR(buttons[i].y + buttons[i].height / 2); PR("y<="); PRN(buttons[i].y + buttons[i].height + buttons[i].height / 2);

                buttons[i].flags |= BUTTON_TOUCH_DOWN;

                return i;
            }

        } else

        if (buttons[i].flags & BUTTON_TYPE_BUTTON) {
        } else

        if(buttons[i].flags & BUTTON_TYPE_SLIDER) {

                                                         ;
            if (x >= buttons[i].x + 150 && x <= buttons[i].x + 150 + buttons[i].width &&
                y >= buttons[i].y &&
                y <= buttons[i].y + buttons[i].height * 2) {

                PRN("SLIDE");
                PR("x:"); PR(x); PR(" y:"); PRN(y);

                buttons[i].value =  x - buttons[i].x + 150;

                buttons[i].flags |= BUTTON_TOUCH_SLIDE;

                _tft->fillRect(buttons[i].x + 151, buttons[i].y + 5,
                               buttons[i].width - 2, buttons[i].height - 2, TFT_BLACK);
                _tft->fillRect(buttons[i].x + 151, buttons[i].y + 5,
                               x - buttons[i].width + 118 + buttons[i].x, buttons[i].height - 2, TFT_LIGHTGREY);

                return i;
            }
        }
    }

    return -1;
}

button_t Buttons::getButton(int id) {
    return buttons[id];
}
