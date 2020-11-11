#ifndef Buttons_h
#define Buttons_h

#define BUTTONS_VERSION	1

#define PR(x) Serial.print(x)
#define PRN(x) Serial.println(x)
#define PRBN(x) Serial.print(x, BIN); Serial.println("")

#if defined(__AVR__)
	#include "Arduino.h"
#elif defined(__PIC32MX__)
	#include "WProgram.h"
#elif defined(__arm__)
	#include "Arduino.h"
#endif

#include <TFT_eSPI.h>
#include <stdint.h>

#define MAX_BUTTONS	12	// Maximum number of buttons available at one time
#define BUTTON_DISABLED          0b0000000000000001
#define BUTTON_TYPE_BUTTON       0b0000000000000010
#define BUTTON_TYPE_SELECT       0b0000000000000100
#define BUTTON_TYPE_SLIDER       0b0000000000001000
#define BUTTON_TYPE_CHECKBOX     0b0000000000010000
#define BUTTON_TOUCH_UP          0b0000000000100000
#define BUTTON_TOUCH_DOWN        0b0000000001000000
#define BUTTON_TOUCH_SLIDE       0b0000000010000000
#define BUTTON_CHECKBOX_CHECKED  0b0000000100000000

typedef struct {
    uint16_t    x, y, width, height, value;
    const char  *label;
    uint16_t    color;
    uint16_t    textColor;
    uint16_t    flags;
} button_t;

class Buttons {
	public:
		Buttons(TFT_eSPI *tft);

		int		addSelect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          const char *label,
                          uint16_t color = TFT_BLUE,
                          uint16_t textColor = TFT_WHITE);
		int		addSelect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          int label,
                          uint16_t color = TFT_BLUE,
                          uint16_t textColor = TFT_WHITE);
		int		addButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          const char *label,
                          uint16_t color = TFT_BLUE,
                          uint16_t textColor = TFT_WHITE);
		int		addButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          int label,
                          uint16_t color = TFT_BLUE,
                          uint16_t textColor = TFT_WHITE);
		int		addSlider(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          uint16_t value, const char *label,
                          uint16_t color = TFT_BLUE,
                          uint16_t textColor = TFT_WHITE);
        int     addCheckbox(uint16_t x, uint16_t y, const char *label,
                            uint16_t color, uint16_t textColor = TFT_WHITE);
		void	draw();
		void	drawButton(int id);
        void    drawSlider(int id);
        void    drawSelect(int id);
        void    drawCheckbox(int id);

        void    drawButtonLabel(int id);
        void	relableButton(int id, const char *label);
        void	relableButton(int id, int label);
        void    setSlider(int id, uint16_t value);
		bool	buttonEnabled(int id);
		void	deleteButton(int id);
		void	deleteAllButtons();
		int     checkButtons(uint16_t x, uint16_t y);

        void    enable(int id);
        void    disable(int id);
        void    setColor(int id, uint16_t color);

		button_t getButton(int id);

	protected:
        TFT_eSPI    *_tft;
		button_t	buttons[MAX_BUTTONS];
        int btncnt;
        const char* format = "%02d";;
};

#endif
