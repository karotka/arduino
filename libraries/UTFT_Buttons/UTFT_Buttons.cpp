/*
  UTFT_Buttons.cpp - Add-on Library for UTFT: Buttons
  Copyright (C)2016 Rinky-Dink Electronics, Henning Karlsen. All right reserved

  This library adds simple but easy to use buttons to extend the use
  of the UTFT and URTouch libraries.

  You can find the latest version of the library at
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the
  examples and tools supplied with the library.
*/

#include "UTFT_Buttons.h"
#include <UTFT.h>
#include <ITDB02_Touch.h>

UTFT_Buttons::UTFT_Buttons(UTFT *ptrUTFT, ITDB02_Touch *ptrURTouch) {
    _UTFT = ptrUTFT;
	Touch = ptrURTouch;
	deleteAllButtons();
	_color_text				= VGA_WHITE;
	_color_text_inactive	= VGA_GRAY;
	_color_background		= VGA_BLUE;
	_color_border			= VGA_WHITE;
	_color_hilite			= VGA_RED;
	_font_text				= NULL;
	_font_symbol			= NULL;
}

int UTFT_Buttons::addButton(uint16_t x, uint16_t y, uint16_t width,
                            uint16_t height, const char *label, uint16_t flags) {
	int btcnt = 0;

	while (((buttons[btcnt].flags & BUTTON_UNUSED) == 0) and (btcnt<MAX_BUTTONS))
		btcnt++;

	if (btcnt == MAX_BUTTONS)
		return -1;
	else {
		buttons[btcnt].pos_x  = x;
		buttons[btcnt].pos_y  = y;
		buttons[btcnt].width  = width;
		buttons[btcnt].height = height;
		buttons[btcnt].flags  = flags;
		buttons[btcnt].label  = label;
		buttons[btcnt].data   = NULL;
		return btcnt;
	}
}

int UTFT_Buttons::addButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                            bitmapdatatype data, uint16_t flags)
{
	int btcnt = 0;

	while (((buttons[btcnt].flags & BUTTON_UNUSED) == 0) and (btcnt<MAX_BUTTONS))
		btcnt++;

	if (btcnt == MAX_BUTTONS)
		return -1;
	else
	{
		buttons[btcnt].pos_x  = x;
		buttons[btcnt].pos_y  = y;
		buttons[btcnt].width  = width;
		buttons[btcnt].height = height;
		buttons[btcnt].flags  = flags | BUTTON_BITMAP;
		buttons[btcnt].label  = NULL;
		buttons[btcnt].data   = data;
		return btcnt;
	}
}


void UTFT_Buttons::drawButtons() {
	for (int i=0;i<MAX_BUTTONS;i++)
	{
		if ((buttons[i].flags & BUTTON_UNUSED) == 0)
			drawButton(i);
	}
}

void UTFT_Buttons::drawButton(int buttonID) {
	int		text_x, text_y;
	uint8_t	*_font_current = _UTFT->getFont();;
	word	_current_color = _UTFT->getColor();
	word	_current_back  = _UTFT->getBackColor();

	if (buttons[buttonID].flags & BUTTON_BITMAP)
	{
		_UTFT->drawBitmap(buttons[buttonID].pos_x, buttons[buttonID].pos_y, buttons[buttonID].width, buttons[buttonID].height, buttons[buttonID].data);
		if (!(buttons[buttonID].flags & BUTTON_NO_BORDER))
		{
			if ((buttons[buttonID].flags & BUTTON_DISABLED))
				_UTFT->setColor(_color_text_inactive);
			else
				_UTFT->setColor(_color_border);
			_UTFT->drawRect(buttons[buttonID].pos_x, buttons[buttonID].pos_y, buttons[buttonID].pos_x+buttons[buttonID].width, buttons[buttonID].pos_y+buttons[buttonID].height);
		}
	}
	else
	{
		_UTFT->setColor(_color_background);
		_UTFT->fillRoundRect(buttons[buttonID].pos_x, buttons[buttonID].pos_y, buttons[buttonID].pos_x+buttons[buttonID].width, buttons[buttonID].pos_y+buttons[buttonID].height);
		_UTFT->setColor(_color_border);
		_UTFT->drawRoundRect(buttons[buttonID].pos_x, buttons[buttonID].pos_y, buttons[buttonID].pos_x+buttons[buttonID].width, buttons[buttonID].pos_y+buttons[buttonID].height);
		if (buttons[buttonID].flags & BUTTON_DISABLED)
			_UTFT->setColor(_color_text_inactive);
		else
			_UTFT->setColor(_color_text);
		if (buttons[buttonID].flags & BUTTON_SYMBOL)
		{
			_UTFT->setFont(_font_symbol);
			text_x = (buttons[buttonID].width/2) - (_UTFT->getFontXsize()/2) + buttons[buttonID].pos_x;
			text_y = (buttons[buttonID].height/2) - (_UTFT->getFontYsize()/2) + buttons[buttonID].pos_y;
		}
		else
		{
			_UTFT->setFont(_font_text);
			text_x = ((buttons[buttonID].width/2) - ((strlen(buttons[buttonID].label) * _UTFT->getFontXsize())/2)) + buttons[buttonID].pos_x;
			text_y = (buttons[buttonID].height/2) - (_UTFT->getFontYsize()/2) + buttons[buttonID].pos_y;
		}
		_UTFT->setBackColor(_color_background);
		_UTFT->print(buttons[buttonID].label, text_x, text_y);
		if ((buttons[buttonID].flags & BUTTON_SYMBOL) and (buttons[buttonID].flags & BUTTON_SYMBOL_REP_3X))
		{
			_UTFT->print(buttons[buttonID].label, text_x-_UTFT->getFontXsize(), text_y);
			_UTFT->print(buttons[buttonID].label, text_x+_UTFT->getFontXsize(), text_y);
		}
	}
	_UTFT->setFont(_font_current);
	_UTFT->setColor(_current_color);
	_UTFT->setBackColor(_current_back);
}

void UTFT_Buttons::enableButton(int buttonID, boolean redraw) {
	if (!(buttons[buttonID].flags & BUTTON_UNUSED))
	{
		buttons[buttonID].flags = buttons[buttonID].flags & ~BUTTON_DISABLED;
		if (redraw)
			drawButton(buttonID);
	}
}

void UTFT_Buttons::disableButton(int buttonID, boolean redraw) {
	if (!(buttons[buttonID].flags & BUTTON_UNUSED))
	{
		buttons[buttonID].flags = buttons[buttonID].flags | BUTTON_DISABLED;
		if (redraw)
			drawButton(buttonID);
	}
}

void UTFT_Buttons::relabelButton(int buttonID, const char *label, boolean redraw) {
	if (!(buttons[buttonID].flags & BUTTON_UNUSED))
	{
		buttons[buttonID].label = label;
		if (redraw)
			drawButton(buttonID);
	}
}

boolean UTFT_Buttons::buttonEnabled(int buttonID) {
	return !(buttons[buttonID].flags & BUTTON_DISABLED);
}

void UTFT_Buttons::deleteButton(int buttonID) {
	if (!(buttons[buttonID].flags & BUTTON_UNUSED))
		buttons[buttonID].flags = BUTTON_UNUSED;
}

void UTFT_Buttons::deleteAllButtons() {
	for (int i=0;i<MAX_BUTTONS;i++)
	{
		buttons[i].pos_x=0;
		buttons[i].pos_y=0;
		buttons[i].width=0;
		buttons[i].height=0;
		buttons[i].flags=BUTTON_UNUSED;
		buttons[i].label="";
	}
}

int UTFT_Buttons::checkButtons() {

    //if (_Touch->dataAvailable() == true) {
		Touch->read();
		int		result = -1;
		int		touch_x = Touch->getX();
		int		touch_y = Touch->getY();
		word	_current_color = _UTFT->getColor();

		for (int i=0;i<MAX_BUTTONS;i++) {
			if (((buttons[i].flags & BUTTON_UNUSED) == 0) and ((buttons[i].flags & BUTTON_DISABLED) == 0) and (result == -1))
			{
				if ((touch_x >= buttons[i].pos_x) and (touch_x <= (buttons[i].pos_x + buttons[i].width)) and (touch_y >= buttons[i].pos_y) and (touch_y <= (buttons[i].pos_y + buttons[i].height)))
					result = i;
			}
		}
		if (result != -1)
		{
			if (!(buttons[result].flags & BUTTON_NO_BORDER))
			{
				_UTFT->setColor(_color_hilite);
				if (buttons[result].flags & BUTTON_BITMAP)
					_UTFT->drawRect(buttons[result].pos_x, buttons[result].pos_y, buttons[result].pos_x+buttons[result].width, buttons[result].pos_y+buttons[result].height);
				else
					_UTFT->drawRoundRect(buttons[result].pos_x, buttons[result].pos_y, buttons[result].pos_x+buttons[result].width, buttons[result].pos_y+buttons[result].height);
			}
		}

		while (Touch->dataAvailable() == true) {};

		if (result != -1)
		{
			if (!(buttons[result].flags & BUTTON_NO_BORDER))
			{
				_UTFT->setColor(_color_border);
				if (buttons[result].flags & BUTTON_BITMAP)
					_UTFT->drawRect(buttons[result].pos_x, buttons[result].pos_y, buttons[result].pos_x+buttons[result].width, buttons[result].pos_y+buttons[result].height);
				else
					_UTFT->drawRoundRect(buttons[result].pos_x, buttons[result].pos_y, buttons[result].pos_x+buttons[result].width, buttons[result].pos_y+buttons[result].height);
			}
		}

		_UTFT->setColor(_current_color);
		return result;
        //}
        //else
		//return -1;
}

void UTFT_Buttons::setTextFont(uint8_t* font) {
	_font_text = font;
}

void UTFT_Buttons::setSymbolFont(uint8_t* font) {
	_font_symbol = font;
}

void UTFT_Buttons::setButtonColors(word atxt, word iatxt, word brd, word brdhi, word back) {
	_color_text				= atxt;
	_color_text_inactive	= iatxt;
	_color_background		= back;
	_color_border			= brd;
	_color_hilite			= brdhi;
}
