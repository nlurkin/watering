/*
 * MoistureSensor.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: nicol
 */

#include "MoistureSensor.h"
#include <Arduino.h>

MoistureSensor::MoistureSensor(int pin, int powerPin) :
	_pin(pin),
	_powerPin(powerPin),
	_measureInterval(-1),
	_tickInterval(-1),
	_currentCounter(10000),
	_lastValue(0),
	_currentValueIndex(0),
	_WATER_VALUE(353),
	_DRY_VALUE(1022)
{
	pinMode(_powerPin, OUTPUT);
	digitalWrite(_powerPin, LOW);
}

MoistureSensor::~MoistureSensor() {
}

void MoistureSensor::tick(){
	if( (++_currentCounter) < (_measureInterval/_tickInterval) ) return;

	_currentCounter = 0;

	digitalWrite(_powerPin, HIGH);
	delay(10);
	_lastValue = analogRead(_pin);
	delay(10);
	digitalWrite(_powerPin, LOW);
}

int MoistureSensor::getPercentageMoisture() const{
	int maxValue = _DRY_VALUE-_WATER_VALUE;
	if(maxValue==0) return -1;
	int inverted = _DRY_VALUE-_lastValue;
	float perc = inverted/(float)maxValue;
	return perc*100;
}

float MoistureSensor::computeAverage(){
	float total = 0;
	for(int i=0; i<_currentValueIndex; ++i) total += +_valArray[i];
	return total/(float)(_currentValueIndex);
}

bool MoistureSensor::readCalibrationValue() {
	_valArray[_currentValueIndex++] = _lastValue;

	return _currentValueIndex>=30;
}

bool MoistureSensor::readCalibrationWater() {
	bool calibOver = readCalibrationValue();
	if(calibOver)
		_WATER_VALUE = computeAverage();
	return calibOver;
}

bool MoistureSensor::readCalibrationDry() {
	bool calibOver = readCalibrationValue();
	if(calibOver)
		_DRY_VALUE = computeAverage();
	return calibOver;
}

void MoistureSensor::resetCalibration() {
	_currentValueIndex = 0;
}

void MoistureSensor::setPowerPin(int pin) {
	pinMode(_powerPin, INPUT);

	_powerPin = pin;
	pinMode(_powerPin, OUTPUT);
	digitalWrite(_powerPin, LOW);
}
