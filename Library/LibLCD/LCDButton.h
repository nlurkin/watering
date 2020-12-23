/*
 * Button.h
 *
 *  Created on: 21 Jul 2019
 *      Author: nicol
 */

#ifndef LIBLCD_LCDBUTTON_H_
#define LIBLCD_LCDBUTTON_H_


#include <Arduino.h>

/**
 * \brief This class is used to handle the buttons from the LCD display.
 *
 * It makes sure that the button are handled correctly, avoiding returning
 * the pressed button every time requested if it has not been released
 * in between.
 */
class LCDButton {
public:
	enum button {btnRIGHT, btnUP, btnDOWN, btnLEFT, btnSELECT, btnNONE}; /** enum for the pressed buttons */

	LCDButton(uint8_t pin, uint16_t debounceDelay = 50);

	button read_LCD_buttons();
	button check_LCD_push();

private:
	const uint8_t _pin;         /** Pin on which to read the button states */
	button _state;              /** Last reported state of the button */
	button _lastState;          /** Last recorded state of the button */
	uint16_t _debounceDelay;    /** Delay before the button is considered pressed */
	uint32_t _lastDebounceTime; /** Last change of recorded state */
};

#endif /* LIBLCD_LCDBUTTON_H_ */

