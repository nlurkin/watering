/*
 * AutomaticWater.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: nicol
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
	sensor1(10, 40),
	pump1(30),
	_currentCounter(0)
{
}

/**
 * Destructor
 */
AutomaticWater::~AutomaticWater() {
}

/**
 * Initialise the system.
 * Initialise all the subsystems
 * Set the tick interval on all the subsystems (moisture sensor and pump)
 */
void AutomaticWater::initSystem(){
   //Setup sensor 1, reading every hour
   sensor1.setMeasureInterval(3600000);
   sensor1.setTickInterval(_gTickInterval);

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
 * It will  evaluate the current sub state and pressed button
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
 * @param button: Eventual button pressed on the LCD display
 */
void AutomaticWater::runCalibrationMode(LCDWaterDisplay::button button){
	// If the sub mode is not one of the CALIB sub modes, we are just entering CALIB
	// from another mode
	if(    _gSubMode!=MODE_CALIB_WATER && _gSubMode!=MODE_CALIB_WATER_W
		&& _gSubMode!=MODE_CALIB_DRY   && _gSubMode!=MODE_CALIB_DRY_W){
		// In this case, initialise the CALIB mode to the first sub mode (WATER wait)
		// Also set the measuring interval of the sensor to fast and set the display accordingly.
		lcdDisplay.initCalibrationMode(LCDWaterDisplay::WATER);
		_gSubMode = MODE_CALIB_WATER_W;
		sensor1.setMeasureInterval(_gTickInterval);
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
			sensor1.resetCalibration();
		}
		break;
	case MODE_CALIB_WATER: // Mode WATER
		// Read moisture level while in moist medium and display the current read and average
		// on the screen.
		calibOver = sensor1.readCalibrationWater();
		lcdDisplay.displayCalibValues(sensor1.getRawMoisture(), sensor1.computeAverage());
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
			sensor1.resetCalibration();
		}
		break;
	case MODE_CALIB_DRY: // Mode DRY
		// Read moisture level while in moist medium and display the current read and average
		// on the screen.
		calibOver = sensor1.readCalibrationDry();
		lcdDisplay.displayCalibValues(sensor1.getRawMoisture(), sensor1.computeAverage());
		// It is finished when the sensor says so (it decides when enough value have been read)
		if(calibOver){
			// If this is finished, return to the main MONITOR mode and reset the monitoring
			// interval of the sensor to slow
			sensor1.setMeasureInterval(3600000);
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
 * The show mode displays the calibration constants. It is
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
	case MODE_SHOW_CONST:
		//Display the calibration constants on the LCD
		lcdDisplay.displayShowConstants(sensor1.getWaterValue(), sensor1.getDryValue());
		if(button==LCDWaterDisplay::btnLEFT){ // If left button pressed again, go back to MONITOR mode
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
 * The MONITOR mode is the main running mode of this program.
 * It can either be IDLE (just computing and reporting moisture level),
 * or RUNNING (checking moisture level and watering when needed)
 *
 * @param button: Eventual button pressed on the LCD display
 */
void AutomaticWater::runMonitorMode(LCDWaterDisplay::button button){
	if( _gSubMode!=MODE_MONITOR_IDLE &&
		_gSubMode!=MODE_MONITOR_RUN){
		lcdDisplay.initMonitorMode();
		_gSubMode = defaultMonitorMode;
	}

	lcdDisplay.displayRunValues(sensor1.getRawMoisture(), sensor1.getPercentageMoisture());

	switch(_gSubMode){
	case MODE_MONITOR_IDLE:
		lcdDisplay.initRunning(false);
		if(button==LCDWaterDisplay::btnSELECT)
			_gMainMode = MAIN_MODE_CALIB;
		else if(button==LCDWaterDisplay::btnLEFT){
			_gMainMode = MAIN_MODE_SHOW;
		}
		else if(button==LCDWaterDisplay::btnRIGHT)
			_gSubMode = MODE_MONITOR_RUN;
		break;
	case MODE_MONITOR_RUN:
		lcdDisplay.initRunning(true);

		if(button==LCDWaterDisplay::btnRIGHT){
			_gSubMode = MODE_MONITOR_IDLE;
			pump1.run(false);
		}

		if(sensor1.getRawMoisture()>LEVEL_WATER){
			//Need water
			lcdDisplay.initWatering(true);
			sensor1.setMeasureInterval(1000);
			pump1.run(true);
		}
		else{
			//Does not need water
			lcdDisplay.initWatering(false);
			sensor1.setMeasureInterval(3600000);
			pump1.run(false);
		}
		break;
	default:
		break;
	}
}

void AutomaticWater::runIdleMode(LCDWaterDisplay::button button) {
}

void AutomaticWater::tick() {
	LCDWaterDisplay::button button = lcdDisplay.read_LCD_buttons();

	if(++_currentCounter<10 && button==LCDWaterDisplay::btnNONE) return;
	_currentCounter = 0;
	sensor1.tick();
	pump1.tick();

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
	case MAIN_MODE_IDLE: //IDLE MODE
		runIdleMode(button);
		break;
	default:
		break;
	}
}

AutomaticWater::MainMode AutomaticWater::checkMainModeChange(LCDWaterDisplay::button button) {

}
