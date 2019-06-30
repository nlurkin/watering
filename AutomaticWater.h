/*
 * AutomaticWater.h
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#ifndef AUTOMATICWATER_H_
#define AUTOMATICWATER_H_

#include "MoistureSensor.h"
#include "PumpControl.h"
#include "LCDWaterDisplay.h"

/**
 * \brief This is the main class of the program. It runs the state machine determining the actions to take.
 *
 * It uses the LCDWaterDisplay to retrieve the button that was clicked and decide to move in
 * the state machine depending on it.
 * It uses the MoistureSensor output  to decide, within the main state machine states, to move between the
 * substates.
 * It uses the PumpControl class to control the water pump depending on the state.
 */
class AutomaticWater {
public:
	AutomaticWater();
	virtual ~AutomaticWater();

	void initSystem();

	void runCalibrationMode(LCDWaterDisplay::button button);
	void runShowMode(LCDWaterDisplay::button button);
	void runMonitorMode(LCDWaterDisplay::button button);

	/**
	 * Set the tickInterval
	 * @param tickInterval: tick interval value
	 */
	void setTickInterval(int tickInterval) {
		_gTickInterval = tickInterval;
	}

	void tick();

private:
	enum MainMode {MAIN_MODE_MONITOR, MAIN_MODE_CALIB, MAIN_MODE_SHOW};     /** enum to identify the main modes */
	enum SubMode  {MODE_MONITOR_IDLE, MODE_MONITOR_RUN,  //Sub modes for MONITOR
		           MODE_CALIB_WATER, MODE_CALIB_WATER_W, MODE_CALIB_DRY, MODE_CALIB_DRY_W, //Sub modes for CALIB
				   MODE_SHOW_CONST}; //Sub modes for SHOW                                   /** enum to identify the sub modes */

	MainMode checkMainModeChange(LCDWaterDisplay::button button);

	const int      PIN_VALVE1         = 32;                 /** Digital PIN number ?? Not used ?? */
	const int      LEVEL_WATER        = 600;                /** Value returned by the sensor below which watering is needed */
	const int      LONG_INTERVAL      = 3600000;            /** Long interval between measurements */
	const int      SHORT_INTERVAL     = 1000;               /** Long interval between measurements */
	const SubMode  defaultMonitorMode = MODE_MONITOR_RUN;   /** Default starting sub mode of the monitoring mode when switching to the monitor mode (Running) */

	MainMode       _gMainMode         = MAIN_MODE_MONITOR;  /** Main mode currently running (defaults to MONITOR) */
	SubMode        _gSubMode          = defaultMonitorMode; /** Sub mode currently running (defaults to defaultMonitorMode) */
	int            _gTickInterval;                          /** Tick interval to be set to all classes used */
	int            _currentCounter;                         /** Internal tick counter */

	MoistureSensor  sensor1;    /** Moisture sensor controller */
	PumpControl     pump1;      /** Pump controller */
	LCDWaterDisplay lcdDisplay; /** LCD Display and buttons controller */
};

#endif /* AUTOMATICWATER_H_ */
