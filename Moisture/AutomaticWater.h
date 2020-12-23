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
#include "AutomaticWaterConfig.h"

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
  AutomaticWater(uint8_t pump_pin);
  virtual ~AutomaticWater();

  void initSystem();
  void setPublicationServer(RemoteControl *server);

  void runCalibrationMode(LCDButton::button button);
  void runShowMode(LCDButton::button button);
  void runMonitorMode(LCDButton::button button);

  bool addSensor(uint8_t pin, uint8_t powerPin);
  bool addValve(uint8_t pin, uint8_t circuit_index);
  bool addCircuit(uint8_t sensor_pin, uint8_t sensor_powerPin, uint8_t valve_pin);

  /**
   * Set the tickInterval
   * @param tickInterval: tick interval value
   */
  void setTickInterval(unsigned int tickInterval) {
    _gTickInterval = tickInterval;
  }

  void tick();
  void loopActiveSensor(int8_t delta);
  void setActiveSensor(uint8_t sensorID);

private:
  void tickSensors();
  void stopAllWatering();
  void setSensorsMeasureInterval(unsigned long int interval);
  bool monitorCircuits();
  bool isWatering();
  void checkCommands();
  void updatePublications();

  enum SubMode  {MODE_MONITOR_IDLE, MODE_MONITOR_RUN,  //Sub modes for MONITOR
                 MODE_CALIB_WATER, MODE_CALIB_WATER_W, MODE_CALIB_DRY, MODE_CALIB_DRY_W}; //Sub modes for CALIB /** enum to identify the sub modes */

  static constexpr int      LEVEL_WATER        = 600;                /** Value returned by the sensor below which watering is needed */
  static constexpr long int LONG_INTERVAL      = 3600000;            /** Long interval between measurements */
  static constexpr long int SHORT_INTERVAL     = 1000;               /** Long interval between measurements */
  static constexpr SubMode  defaultMonitorMode = MODE_MONITOR_RUN;   /** Default starting sub mode of the monitoring mode when switching to the monitor mode (Running) */

  SubMode        _gSubMode          = defaultMonitorMode; /** Sub mode currently running (defaults to defaultMonitorMode) */
  unsigned int   _gTickInterval;                          /** Tick interval to be set to all classes used */
  unsigned int   _currentCounter;                         /** Internal tick counter */
  uint8_t        _nCircuits;                              /** Number of circuits (sensors + valve couples)*/
  uint8_t        _currentSensor;                          /** Index of the currently active sensor. Active is defined as the one that is being displayed/calibrated */
  bool           _isWatering[AW::MAX_SENSORS];                /** True if associated sensor is currently watering else false */

  MoistureSensor  *sensors[AW::MAX_SENSORS]; /** Moisture sensor controller */
  ValveController *valves[AW::MAX_SENSORS];  /** Valves controllers */
  PumpControl      pump1;      /** Pump controller */
  LCDWaterDisplay  lcdDisplay; /** LCD Display and buttons controller */

  RemoteControl     *_controlServer;
  Publication<bool> *_pub_pump;
  Publication<bool> *_pub_pump_enabled;
  Publication<bool> *_pub_pump_valves[AW::MAX_SENSORS];
  Publication<int>  *_pub_sensors[AW::MAX_SENSORS];
  Command<bool>     *_cmd_pump;
};

#endif /* AUTOMATICWATER_H_ */
