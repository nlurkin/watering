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
#include "ValveController.h"
#include "Publication.h"
#include "Command.h"

class RemoteControl;

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
	static const short MAX_SENSORS = 5;
	AutomaticWater(uint8_t pump_pin);
	virtual ~AutomaticWater();

	void initSystem();
	void setPublicationServer(RemoteControl *server);

	void runCalibrationMode(LCDWaterDisplay::button button);
	void runShowMode(LCDWaterDisplay::button button);
	void runMonitorMode(LCDWaterDisplay::button button);

	bool addSensor(uint8_t pin, uint8_t powerPin);
	bool addValve(uint8_t pin, unsigned short circuit_index);
	bool addCircuit(uint8_t sensor_pin, uint8_t sensor_powerPin, uint8_t valve_pin);

	/**
	 * Set the tickInterval
	 * @param tickInterval: tick interval value
	 */
	void setTickInterval(unsigned int tickInterval) {
		_gTickInterval = tickInterval;
	}

	void tick();
	void loopActiveSensor(short delta);
	void setActiveSensor(unsigned short sensorID);

private:
	void tickSensors();
	void stopAllWatering();
	void setSensorsMeasureInterval(unsigned long int interval);
	bool monitorCircuits();
	bool isWatering();
	void checkCommands();
	void updatePublications();

	enum MainMode {MAIN_MODE_MONITOR, MAIN_MODE_CALIB, MAIN_MODE_SHOW};     /** enum to identify the main modes */
	enum SubMode  {MODE_MONITOR_IDLE, MODE_MONITOR_RUN,  //Sub modes for MONITOR
		           MODE_CALIB_WATER, MODE_CALIB_WATER_W, MODE_CALIB_DRY, MODE_CALIB_DRY_W, //Sub modes for CALIB
				   MODE_SHOW_CONST}; //Sub modes for SHOW                   /** enum to identify the sub modes */

	const int      LEVEL_WATER        = 600;                /** Value returned by the sensor below which watering is needed */
	const long int LONG_INTERVAL      = 3600000;            /** Long interval between measurements */
	const long int SHORT_INTERVAL     = 1000;               /** Long interval between measurements */
	const SubMode  defaultMonitorMode = MODE_MONITOR_RUN;   /** Default starting sub mode of the monitoring mode when switching to the monitor mode (Running) */

	MainMode       _gMainMode         = MAIN_MODE_MONITOR;  /** Main mode currently running (defaults to MONITOR) */
	SubMode        _gSubMode          = defaultMonitorMode; /** Sub mode currently running (defaults to defaultMonitorMode) */
	unsigned int   _gTickInterval;                          /** Tick interval to be set to all classes used */
	unsigned int   _currentCounter;                         /** Internal tick counter */
	unsigned short _nCircuits;                              /** Number of circuits (sensors + valve couples)*/
	unsigned short _currentSensor;                          /** Index of the currently active sensor. Active is defined as the one that is being displayed/calibrated */
	bool           _isWatering[MAX_SENSORS];                /** True if associated sensor is currently watering else false */

	MoistureSensor  *sensors[MAX_SENSORS]; /** Moisture sensor controller */
	ValveController *valves[MAX_SENSORS];  /** Valves controllers */
	PumpControl      pump1;      /** Pump controller */
	LCDWaterDisplay  lcdDisplay; /** LCD Display and buttons controller */

	RemoteControl     *_controlServer;
	Publication<bool> *_pub_pump;
	Publication<bool> *_pub_pump_enabled;
	Publication<bool> *_pub_pump_valves[MAX_SENSORS];
	Publication<int>  *_pub_sensors[MAX_SENSORS];
	Command<bool>     *_cmd_pump;
};

#endif /* AUTOMATICWATER_H_ */
