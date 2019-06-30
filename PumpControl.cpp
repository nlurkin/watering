/*
 * PumpControl.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: nicol
 */

#include "PumpControl.h"
#include <Arduino.h>

PumpControl::PumpControl(int pin) :
	_pin(pin),
	_deadTime(-1),
	_runningTime(-1),
	_tickInterval(-1),
	_currentCounter(0),
	_on(IDLE)
{
	pinMode(pin, OUTPUT);
	digitalWrite(_pin, HIGH);
}

PumpControl::~PumpControl() {
}

void PumpControl::tick() {
	++_currentCounter;
	int runningTimeInTick = _runningTime/_tickInterval;
	int deadTimeInTick = _deadTime/_tickInterval;
	if(_on==RUNNING){
		if(_currentCounter > runningTimeInTick){
			digitalWrite(_pin, HIGH);
			_on = DEAD;
			_currentCounter = 0;
		}
	}
	else if(_on==DEAD){
		if(_currentCounter > deadTimeInTick){
			_on = IDLE;
			_currentCounter = 0;
		}
	}
	else
		_currentCounter = 0;
}

void PumpControl::run(bool state) {
	if(state && _on==IDLE){
		digitalWrite(_pin, LOW);
		_on = RUNNING;
		_currentCounter = 0;
	}
	else if(!state && _on==RUNNING) {
		digitalWrite(_pin, HIGH);
		_on = DEAD;
		_currentCounter = 0;
	}
}

void PumpControl::setPin(int pin) {
	//Unset previous pin
	pinMode(_pin, INPUT);
	digitalWrite(_pin, LOW);

	//Set new pin
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
	_pin = pin;
}
