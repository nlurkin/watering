/*
 * AutomaticWater.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#include "AutomaticWater.h"
#include <Arduino.h>

/**
 * Constructor
 */
AutomaticWater::AutomaticWater() :
	_gMainMode(MAIN_MODE_MONITOR),
	_gSubMode(defaultMonitorMode),
	_gTickInterval(500),
	_nSensors(0),
	_currentSensor(0),
	pump1(30),
	_currentCounter(0)
{
	for(unsigned short i=0; i<MAX_SENSORS; ++i) sensors[i] = nullptr;
}

/**
 * Destructor.
 *
 * Make sure we delete all the sensor pointers
 */
AutomaticWater::~AutomaticWater() {
	for(unsigned short i=0; i<_nSensors; ++i){
		delete sensors[i];
		sensors[i] = nullptr;
	}
}

/**
 * Initialise the system.
 * Initialise all the subsystems
 * Set the tick interval on all the subsystems (moisture sensor and pump)
 */
void AutomaticWater::initSystem(){
	//Setup pump
	pump1.run(false);
	pump1.setTickInterval(_gTickInterval);
	pump1.setRunningTime(10000);
	pump1.setDeadTime(4000);

	//Setup lcd
	lcdDisplay.initMonitorMode();
}

/**
 * Method called at tick when in Calibration mode.
 * It will evaluate the current sub state and pressed button
 * and execute actions accordingly.
 *
 * The calibration mode is used to calibrate the extremes of the sensor
 * (most humid and most dry). It is activated by pressing the Select button.
 *
 * It will ask the user to put the sensor in the most humid medium (humid
 * soil is actually more moist that tap water - up to the user to determine
 * what he wants for moist).
 * Then it will ask the user to put the sensor in the dry place (air).
 * Finally it will return to MONITOR mode.
 *
 *  The calibrated sensor is the active one and it is not possible to change
 *  during the procedure.
 *
 * @param button: Eventual button pressed on the LCD display
 */
void AutomaticWater::runCalibrationMode(LCDWaterDisplay::button button){
	// If the sub mode is not one of the CALIB sub modes, we are just entering CALIB
	// from another mode. The calibrated sensor is the active one.
	// It is not possible to change the active sensor during the procedure.
	if(    _gSubMode!=MODE_CALIB_WATER && _gSubMode!=MODE_CALIB_WATER_W
		&& _gSubMode!=MODE_CALIB_DRY   && _gSubMode!=MODE_CALIB_DRY_W){
		// In this case, initialise the CALIB mode to the first sub mode (WATER wait)
		// Also set the measuring interval of the sensor to fast and set the display accordingly.
		lcdDisplay.initCalibrationMode(LCDWaterDisplay::WATER);
		_gSubMode = MODE_CALIB_WATER_W;
		sensors[_currentSensor]->setMeasureInterval(_gTickInterval);
	}


	bool calibOver;
	//Choose actions depending on submode
	switch(_gSubMode) {
	case MODE_CALIB_WATER_W:  // Mode WATER wait
		// Wait until the user says that the sensor is correctly in the moist medium (SELECT button)
		if(button==LCDWaterDisplay::btnSELECT){
			//Move to the next mode (WATER), set the display accordingly and reset the sensor
			lcdDisplay.displayCalibMode(LCDWaterDisplay::WATER);
			_gSubMode = MODE_CALIB_WATER;
			sensors[_currentSensor]->resetCalibration();
		}
		break;
	case MODE_CALIB_WATER: // Mode WATER
		// Read moisture level while in moist medium and display the current read and average
		// on the screen.
		calibOver = sensors[_currentSensor]->readCalibrationWater();
		lcdDisplay.displayCalibValues(sensors[_currentSensor]->getRawMoisture(), sensors[_currentSensor]->computeAverage());
		// It is finished when the sensor says so (it decides when enough value have been read)
		if(calibOver){
			// If this is finished, move to the next mode (DRY wait) and set the display accordingly
			lcdDisplay.initCalibrationMode(LCDWaterDisplay::DRY);
			_gSubMode = MODE_CALIB_DRY_W;
		}
		break;
	case MODE_CALIB_DRY_W: // Mode DRY wait
		// Wait until the user says that the sensor is correctly in the dry medium (SELECT button)
		if(button==LCDWaterDisplay::btnSELECT){
			//Move to the next mode (DRY), set the display accordingly and reset the sensor
			lcdDisplay.displayCalibMode(LCDWaterDisplay::DRY);
			_gSubMode = MODE_CALIB_DRY;
			sensors[_currentSensor]->resetCalibration();
		}
		break;
	case MODE_CALIB_DRY: // Mode DRY
		// Read moisture level while in moist medium and display the current read and average
		// on the screen.
		calibOver = sensors[_currentSensor]->readCalibrationDry();
		lcdDisplay.displayCalibValues(sensors[_currentSensor]->getRawMoisture(), sensors[_currentSensor]->computeAverage());
		// It is finished when the sensor says so (it decides when enough value have been read)
		if(calibOver){
			// If this is finished, return to the main MONITOR mode and reset the monitoring
			// interval of the sensor to slow
			sensors[_currentSensor]->setMeasureInterval(LONG_INTERVAL);
			_gMainMode = MAIN_MODE_MONITOR;
		}
		break;
	default:
		break;
	}
}

/**
 * Method called at tick when in Show mode.
 * It will  evaluate the current sub state and pressed button
 * and execute actions accordingly.
 *
 * The show mode displays the calibration constants for the active sensor. It is
 * (de)activated by pressing the Left button.
 *
 * @param button: Eventual button pressed on the LCD display
 */
void AutomaticWater::runShowMode(LCDWaterDisplay::button button){
	//If sub mode is not one of the SHOW sub modes, we are just entering
	//SHOW mode from another mode
	if(_gSubMode!=MODE_SHOW_CONST){
		//Set the display and the sub mode properly
		lcdDisplay.initShowMode();
		_gSubMode = MODE_SHOW_CONST;
	}

	switch(_gSubMode) {
	case MODE_SHOW_CONST: // Mode SHOW
		//Display the calibration constants on the LCD
		lcdDisplay.displayShowConstants(sensors[_currentSensor]->getWaterValue(), sensors[_currentSensor]->getDryValue());
		if(button==LCDWaterDisplay::btnLEFT){ // If left button pressed again, go back to MONITOR mode
			_gMainMode = MAIN_MODE_MONITOR;
		}
		else if(button==LCDWaterDisplay::btnUP){ // If up button pressed, change active sensor to the next one
			loopActiveSensor(1);
		}
		else if(button==LCDWaterDisplay::btnDOWN){ // If down button pressed, change active sensor to the previous one
			loopActiveSensor(-1);
		}
		break;
	default:
		break;
	}
}

/**
 * Method called at tick when in Show mode.
 * It will evaluate the current sub state and pressed button
 * and execute actions accordingly.
 *
 * The MONITOR mode is the main running mode of this program.
 * It can either be IDLE (just computing and reporting moisture level),
 * or RUNNING (checking moisture level and watering when needed)
 *
 * @param button: Eventual button pressed on the LCD display
 */
void AutomaticWater::runMonitorMode(LCDWaterDisplay::button button){
	// If current sub mode is not one of the MONITOR mode, we are coming to MONITOR
	// mode from another mode
	if( _gSubMode!=MODE_MONITOR_IDLE &&
		_gSubMode!=MODE_MONITOR_RUN){
		// Set sub mode as the default monitor mode and set the display accordingly
		lcdDisplay.initMonitorMode();
		_gSubMode = defaultMonitorMode;
	}

	// Display the moisture level and the percentage
	if(_nSensors>0)
		lcdDisplay.displayRunValues(sensors[_currentSensor]->getRawMoisture(), sensors[_currentSensor]->getPercentageMoisture());

	//We can change active sensor in all cases
	if(button==LCDWaterDisplay::btnUP){ // If up button pressed, change active sensor to the next one
		loopActiveSensor(1);
	}
	else if(button==LCDWaterDisplay::btnDOWN){ // If down button pressed, change active sensor to the previous one
		loopActiveSensor(-1);
	}

	switch(_gSubMode){
	case MODE_MONITOR_IDLE: // Mode IDLE
		// In this case we only monitor the moisture level without taking any action
		// LCD does not display the running symbol
		lcdDisplay.initRunning(false);

		// Check what are the possible button pressed
		if(button==LCDWaterDisplay::btnSELECT) // If Select was pressed, move to CALIB mode
			_gMainMode = MAIN_MODE_CALIB;
		else if(button==LCDWaterDisplay::btnLEFT) // If Left was pressed, move to SHOW mode
			_gMainMode = MAIN_MODE_SHOW;
		else if(button==LCDWaterDisplay::btnRIGHT) { // If Right was pressed, move to RUNNING sub mode
			_gSubMode = MODE_MONITOR_RUN;
		}
		break;
	case MODE_MONITOR_RUN: // Mode IDLE
		// In this mode we monitor the moisture level and start watering if it drops too low
		// LCD does display the running symbol
		lcdDisplay.initRunning(true);

		if(button==LCDWaterDisplay::btnRIGHT){ // If Right button was pressed, move to IDLE sub mode
			// Ensure the pump is not running and reset measurement interval to LONG_INTERVAL
			_gSubMode = MODE_MONITOR_IDLE;
			pump1.run(false);
			setSensorsMeasureInterval(LONG_INTERVAL);
		}
		else{
			bool needWater = monitorCircuits();
			if(needWater && !_isWatering){
				//Need water
				// Enable the pump
				pump1.run(true);
				_isWatering = true;
			}
			else if(!needWater && _isWatering){
				//Does not need water
				// Disable the pump
				pump1.run(false);
				_isWatering = false;
			}
			// Display or not the watering symbol.
			lcdDisplay.initWatering(_isWatering);
		}
		break;
	default:
		break;
	}
}

/**
 * Add a new sensor. Only AutomaticWater::MAX_SENSORS sensors are allowed.
 * So the method does nothing if that number has been reached. The sensor
 * is initialised with the AutomaticWater::LONG_INTERVAL measuring interval.
 *
 * @param pin: Analog pin on which to read the sensor raw values
 * @param powerPin: Digital pin used for powering the sensor
 * @return True of the sensor was added, else false.
 */
bool AutomaticWater::addSensor(unsigned short pin, unsigned short powerPin) {
	// Not more than MAX_SENSORS
	if(_nSensors>=MAX_SENSORS) return false;
	sensors[_nSensors] = new MoistureSensor(pin, powerPin);

	//Setup sensor 1, reading every hour
	sensors[_nSensors]->setMeasureInterval(LONG_INTERVAL);
	sensors[_nSensors]->setTickInterval(_gTickInterval);
	++_nSensors;

	return true;
}

/**
 * This is the tick method called at every clock tick from the main
 * It will read the button information from the LCD and forward it to the
 * main mode methods. It actually runs every 10 clock ticks, or when a button
 * is pressed.
 * It also call the tick methods of the sensor and the pump.
 */
void AutomaticWater::tick() {
	//Reads the button info from the LCD
	LCDWaterDisplay::button button = lcdDisplay.read_LCD_buttons();

	// Pass to the rest of the function only every 10 ticks, or when a button is pressed
	if(++_currentCounter<10 && button==LCDWaterDisplay::btnNONE) return;
	_currentCounter = 0;

	// Tick the sub classes
	tickSensors();
	pump1.tick();

	// Run the method associated to the current main mode
	switch(_gMainMode) {
	case MAIN_MODE_MONITOR: //MONITORING MODE
		runMonitorMode(button);
		break;
	case MAIN_MODE_CALIB: //CALIBRATION MODE
		runCalibrationMode(button);
		break;
	case MAIN_MODE_SHOW: //SHOW MODE
		runShowMode(button);
		break;
	default:
		break;
	}
}

/**
 * Loop over the sensors to change the active one.
 *
 * @param delta: Only 2 values allowed: +1 and -1. If +1, the sensor after the current active
 * one becomes the active sensor. If -1, the sensor before the current active one becomes the
 * active sensor.
 */
void AutomaticWater::loopActiveSensor(short delta) {
	// Only delta of +1 or -1 are allowed, just for simplicity.
	// Also no sense to loop if there are no sensors.
	if(abs(delta)!=1 || _nSensors==0) return;

	if(delta<0 && _currentSensor==0) _currentSensor = _nSensors-1;
	else if(delta>0 && _currentSensor==_nSensors-1) _currentSensor = 0;
	else _currentSensor += delta;
}

/**
 * Call the tick method of all the existing sensors.
 */
void AutomaticWater::tickSensors() {
	for(unsigned short i=0; i<_nSensors; ++i) sensors[i]->tick();
}

/**
 * Set the current active sensor. The method does nothing if sensorID is not
 * the ID of an existing sensor.
 *
 * @param sensorID: ID of the sensor to set active
 */
void AutomaticWater::setActiveSensor(unsigned short sensorID) {
	if(sensorID<_nSensors) _currentSensor = sensorID;
}

/**
 * Set the measure interval on all the existing sensors
 *
 * @param interval: Measure interval to set
 */
void AutomaticWater::setSensorsMeasureInterval(unsigned long int interval) {
	for(unsigned short i=0; i<_nSensors; ++i) sensors[i]->setMeasureInterval(interval);
}

bool AutomaticWater::monitorCircuits() {
	bool needWater = false;
	for (unsigned short i = 0; i < _nSensors; ++i) {
		if (sensors[i]->getRawMoisture() > LEVEL_WATER) {
			// Need water
			needWater = true;
			// Shorten the measurement interval to SHORT_INTERVAL for that sensor.
			sensors[i]->setMeasureInterval(SHORT_INTERVAL);
		} else {
			// Does not need water
			// Reset the measurement interval to LONG_INTERVAL for that sensor.
			sensors[i]->setMeasureInterval(LONG_INTERVAL);
		}
	}
	return needWater;
}
