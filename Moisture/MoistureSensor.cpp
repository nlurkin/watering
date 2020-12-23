/*
 * MoistureSensor.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#include "MoistureSensor.h"

/**
 * Constructor. It initialises the internal values of the moisture sensor.
 *
 * @param pin: Analog pin on which moisture values are read.
 * @param powerPin: Digital pin used to power the sensor.
 */
MoistureSensor::MoistureSensor(uint8_t pin, uint8_t powerPin) :
  _pin(pin),
  _powerPin(powerPin),
  _measureInterval(-1),
  _tickInterval(-1),
  _currentCounter(10000),
  _lastValue(0),
  _currentValueIndex(0),
  _WATER_VALUE(353),     // Calibration value obtained after tests of this particular sensor
  _DRY_VALUE(1022)       // Calibration value obtained after tests of this particular sensor
{
  // Make sure the sensor is not powered
  pinMode(_powerPin, OUTPUT);
  digitalWrite(_powerPin, LOW);
}

/**
 * Destructor
 */
MoistureSensor::~MoistureSensor() {
}

/**
 * This is the tick method called at every clock tick from the main
 * It powers the sensor and reads a value when the measurement time has come.
 */
void MoistureSensor::tick(){
  // Increment the tick counter and check whether it is time for a read
  if( (++_currentCounter) < (_measureInterval/_tickInterval) ) return;

  _currentCounter = 0;

  // Power up the sensor
  digitalWrite(_powerPin, HIGH);
  // Wait a bit to let the sensor stabilise then read raw value
  delay(10);
  _lastValue = analogRead(_pin);
  delay(10);
  // Poser down the sensor
  digitalWrite(_powerPin, LOW);
}

/**
 * Compute and returns the percentage.
 *
 * @return Moisture percentage based on the calibration values
 */
int MoistureSensor::getPercentageMoisture() const{
  int maxValue = _DRY_VALUE-_WATER_VALUE;
  if(maxValue==0) return -1; // No valid calibration

  // DRY actually returns a higher raw value than moist.
  // So we need first to invert the slope of the function
  int inverted = _DRY_VALUE-_lastValue;

  float perc = inverted/(float)maxValue;
  return perc*100;
}

/**
 * Compute value average from value buffer.
 *
 * @return Average of the values acquired since last reset of the buffer
 */
float MoistureSensor::computeAverage(){
  float total = 0;
  // Sum all the values acquired since the index
  for(int i=0; i<_currentValueIndex; ++i) total += +_valArray[i];

  return total/(float)(_currentValueIndex); // Average
}

/**
 * Read a new raw value for calibration and add it to the buffer.
 *
 * @return True if the buffer is full
 */
bool MoistureSensor::readCalibrationValue() {
  // Increment the current index and fill it with the last value read
  _valArray[_currentValueIndex++] = _lastValue;

  return _currentValueIndex>=30; // The size of the buffer is 30. Check if it is full
}

/**
 * Read a new raw value for WATER calibration. If the buffer
 * is full, compute the average and use it as the calibration
 * value.
 *
 * @return True if the calibration is finished
 */
bool MoistureSensor::readCalibrationWater() {
  bool calibOver = readCalibrationValue();
  if(calibOver) // Buffer is full
    _WATER_VALUE = computeAverage();
  return calibOver;
}

/**
 * Read a new raw value for DRY calibration. If the buffer
 * is full, compute the average and use it as the calibration
 * value.
 *
 * @return True if the calibration is finished
 */
bool MoistureSensor::readCalibrationDry() {
  bool calibOver = readCalibrationValue();
  if(calibOver) // Buffer is full
    _DRY_VALUE = computeAverage();
  return calibOver;
}

/**
 * Reset the calibration buffer
 */
void MoistureSensor::resetCalibration() {
  _currentValueIndex = 0;
}

/**
 * Change the power pin
 *
 * @param pin: Digital pin used for powering the sensor
 */
void MoistureSensor::setPowerPin(uint8_t pin) {
  // Reset the previous pin to input
  pinMode(_powerPin, INPUT);

  // Change the pin and make sure it is not powering the sensor
  _powerPin = pin;
  pinMode(_powerPin, OUTPUT);
  digitalWrite(_powerPin, LOW);
}
