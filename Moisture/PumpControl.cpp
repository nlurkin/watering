/*
 * PumpControl.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#include "PumpControl.h"

/**
 * Constructor. Initialise the pin in output mode and value HIGH (corresponding to
 * the pump being OFF).
 *
 * @param pin: Digital pin on which the pump is connected
 */
PumpControl::PumpControl(uint8_t pin) :
	_pin(pin),
	_deadTime(-1),
	_runningTime(-1),
	_tickInterval(-1),
	_currentCounter(0),
	_on(IDLE)
{
	// Initialise the pin. Must be in output
	pinMode(pin, OUTPUT);
	// And must be HIGH (=OFF)
	digitalWrite(_pin, HIGH);
}

/**
 * Destructor
 */
PumpControl::~PumpControl() {
}

/**
 * Tick method being called by the parent class at every clock tick.
 *
 * It increments the internal tick counter and if the pump is either
 * RUNNING or DEAD, checks if a state change is required according to
 * the time length it is supposed to stay in that state.
 */
void PumpControl::tick() {
	// Increment the internal tick counter
	++_currentCounter;

	if(_on==RUNNING){
		// If the pump is currently running, compute the amount of time it is
		// supposed to be running, in unit of tick.
		int runningTimeInTick = _runningTime/_tickInterval;
		if(_currentCounter > runningTimeInTick){
			// We have been running long enough, stop the pump
			// Move to DEAD state and reset the tick counter.
			digitalWrite(_pin, HIGH);
			_on = DEAD;
			_currentCounter = 0;
		}
	}
	else if(_on==DEAD){
		// If the pump is currently in DEAD state, compute the amount of time
		// it is supposed to stay in that state, in unit of tick.
		int deadTimeInTick = _deadTime/_tickInterval;
		if(_currentCounter > deadTimeInTick){
			// We have been dead for long enough, move back to IDLE state
			// and reset the tick counter
			_on = IDLE;
			_currentCounter = 0;
		}
	}
	else
		// Else, we are IDLE, nothing to do. Just keep the counter to 0
		_currentCounter = 0;
}

/**
 * Ask the pump to run or to stop.
 * @param state: True will start the pump running, false will make it stop
 */
void PumpControl::run(bool state) {
	if(state && _on==IDLE){
		// We ask it to run, and it is currently IDLE
		// Enable to pump, move to RUNNING state and reset the
		// tick counter.
		digitalWrite(_pin, LOW);
		_on = RUNNING;
		_currentCounter = 0;
	}
	else if(!state && _on==RUNNING) {
		// We ask it to stop, and it is currently RUNNING
		// Disable the pump, move to DEAD state and reset the
		// tick counter
		digitalWrite(_pin, HIGH);
		_on = DEAD;
		_currentCounter = 0;
	}
	// In all other cases, there is nothing to do as we
	// are already in the requested state (or in DEAD time in which
	// case we do not obey any order).
}

/**
 * Change the digital pin on which the pump is connected
 * @param pin: Digital pin on which the pump is connected
 */
void PumpControl::setPin(uint8_t pin) {
	//Unset previous pin
	pinMode(_pin, INPUT);
	digitalWrite(_pin, LOW);

	//Set new pin
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
	_pin = pin;
}
