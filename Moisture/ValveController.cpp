/*
 * ValveController.cpp
 *
 *  Created on: 6 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ValveController.h"

/**
 * Constructor. Initialise the valve in the closed state.
 *
 * @param pin: Digital pin from which the valve is controlled
 */
ValveController::ValveController(uint8_t pin) :
	_pin(pin),
	_open(false),
	_enable(true)
{
	// Initialise the pin. Must be in output
	pinMode(pin, OUTPUT);
	// And must be HIGH (=OFF)
	digitalWrite(_pin, HIGH);
}

/**
 * Destructor
 */
ValveController::~ValveController() {
}

/**
 * Request to open/close the valve.
 *
 * @param state: True for opening the valve, false to close it
 */
void ValveController::open(bool state) {
	if(!_enable) // If disabled, do not allow to open the valve
		state = false;

	if(state && !_open){
		// If request to open and valve is closed, open it
		digitalWrite(_pin, LOW);
		_open = true;
	}
	else if(!state && _open) {
		// If request to close and valve is open, close it
		digitalWrite(_pin, HIGH);
		_open = false;
	}
}

/**
 * Toggle the valve state
 */
void ValveController::toggle() {
	open(!_open);
}
