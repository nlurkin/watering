/*
 * AutomaticWater.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#include "AutomaticWater.h"
#include <Arduino.h>
#include "ControlServer.h"

/**
 * Constructor
 *
 * @param pump_pin: Digital pin controlling the pump
 */
AutomaticWater::AutomaticWater(uint8_t pump_pin) :
  _gSubMode(defaultMonitorMode),
  _gTickInterval(500),
  _currentCounter(0),
  _nCircuits(0),
  _currentSensor(0),
  pump1(pump_pin),
  _controlServer(nullptr),
  _pub_pump(nullptr),
  _pub_pump_enabled(nullptr),
  _pub_pump_valves{nullptr},
  _pub_sensors{nullptr},
  _cmd_pump{nullptr}
{
  for(unsigned short i=0; i<AW::MAX_SENSORS; ++i) {
    sensors[i] = nullptr;
    valves[i] = nullptr;
    _isWatering[i] = false;
  }
}

/**
 * Destructor.
 *
 * Make sure we delete all the sensor and valve pointers
 */
AutomaticWater::~AutomaticWater() {
  for(unsigned short i=0; i<_nCircuits; ++i){
    delete sensors[i];
    sensors[i] = nullptr;
    if(valves[i]) delete valves[i];
    valves[i] = nullptr;
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
  lcdDisplay.initRunning(true);
}

void AutomaticWater::loaded() {
  lcdDisplay.loaded();
}

/**
 * Sets the publication server if it is to be used
 * @param server: PubServer instance
 */
void AutomaticWater::setPublicationServer(ControlServer *server){
  _controlServer = server;
  if(server==nullptr)
    return;
  // Create publications and add them
  _pub_pump = new Publication<bool>("pump1_state");
  _pub_pump->updateValue(false);
  _controlServer->addPublication(_pub_pump);
  _cmd_pump = new Command<bool>("pump1_inhibit");
  _controlServer->addCommand(_cmd_pump);
  _pub_pump_enabled = new Publication<bool>("pump1_inhibit");
  _pub_pump_enabled->updateValue(pump1.isEnabled());
  _controlServer->addPublication(_pub_pump_enabled);
  for (unsigned short i = 0; i < _nCircuits; ++i) {
    char name[15];
    sprintf(name, "valve%d_state", i);
    _pub_pump_valves[i] = new Publication<bool>(name);
    _pub_pump_valves[i]->updateValue(false);
    _controlServer->addPublication(_pub_pump_valves[i]);
    sprintf(name, "sensor%d", i);
    _pub_sensors[i] = new Publication<int>(name);
    _pub_sensors[i]->updateValue(-1);
    _controlServer->addPublication(_pub_sensors[i]);
  }

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
void AutomaticWater::runCalibrationMode(LCDButton::button button){
  // If the sub mode is not one of the CALIB sub modes, we are just entering CALIB
  // from another mode. The calibrated sensor is the active one.
  // It is not possible to change the active sensor during the procedure.
  if(_gSubMode!=MODE_CALIB_INIT && _gSubMode!=MODE_CALIB_WATER && _gSubMode!=MODE_CALIB_WATER_W
    && _gSubMode!=MODE_CALIB_DRY   && _gSubMode!=MODE_CALIB_DRY_W){
    // In this case, initialise the CALIB mode to the first sub mode (WATER wait)
    // Also set the measuring interval of the sensor to fast and set the display accordingly.
    _gSubMode = MODE_CALIB_INIT;
    lcdDisplay.resetCalibrationMode();
  }


  bool calibOver;
  //Choose actions depending on submode
  switch(_gSubMode) {
  case MODE_CALIB_INIT: // Mode init (sensor not selected yet)
    if(button==LCDButton::btnRIGHT)
      loopActiveSensor(1);
    else if(button==LCDButton::btnLEFT)
      loopActiveSensor(-1);
    if(button==LCDButton::btnSELECT){
      lcdDisplay.disableMenuChange();
      lcdDisplay.initCalibrationMode(MenuCalib::WATER);
      sensors[_currentSensor]->setMeasureInterval(_gTickInterval);
      _gSubMode = MODE_CALIB_WATER_W;
    }
    lcdDisplay.displayCalibSensor(_currentSensor);
    break;
  case MODE_CALIB_WATER_W:  // Mode WATER wait
    // Wait until the user says that the sensor is correctly in the moist medium (SELECT button)
    if(button==LCDButton::btnSELECT){
      //Move to the next mode (WATER), set the display accordingly and reset the sensor
      lcdDisplay.displayCalibMode(MenuCalib::WATER);
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
      lcdDisplay.initCalibrationMode(MenuCalib::DRY);
      _gSubMode = MODE_CALIB_DRY_W;
    }
    break;
  case MODE_CALIB_DRY_W: // Mode DRY wait
    // Wait until the user says that the sensor is correctly in the dry medium (SELECT button)
    if(button==LCDButton::btnSELECT){
      //Move to the next mode (DRY), set the display accordingly and reset the sensor
      lcdDisplay.displayCalibMode(MenuCalib::DRY);
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
      _gSubMode = MODE_CALIB_INIT;
      lcdDisplay.resetCalibrationMode();
      lcdDisplay.enableMenuChange();
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
void AutomaticWater::runShowMode(LCDButton::button /*button*/){
  //Update the calibration constants in the LCD Show Menu
  for(uint8_t i=0; i<_nCircuits; ++i)
    lcdDisplay.displayShowConstants(i, sensors[i]->getWaterValue(), sensors[i]->getDryValue());
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
void AutomaticWater::runMonitorMode(LCDButton::button button){
  // If current sub mode is not one of the MONITOR mode, we are coming to MONITOR
  // mode from another mode
  if( _gSubMode!=MODE_MONITOR_IDLE &&
    _gSubMode!=MODE_MONITOR_RUN){
    // Set sub mode as the default monitor mode and set the display accordingly
    // LCD does not display the running symbol
    lcdDisplay.initRunning(true);
    _gSubMode = defaultMonitorMode;
  }

  // Display the moisture level and the percentage
  for(uint8_t i=0; i<_nCircuits; ++i)
    lcdDisplay.displayRunValues(i, sensors[i]->getRawMoisture(), sensors[i]->getPercentageMoisture());

  //We can change active sensor in all cases
  //if(button==LCDWaterDisplay::btnUP){ // If up button pressed, change active sensor to the next one
  //  loopActiveSensor(1);
  //}
  //else if(button==LCDWaterDisplay::btnDOWN){ // If down button pressed, change active sensor to the previous one
  //  loopActiveSensor(-1);
  //}

  checkCommands();
  updatePublications();
  switch(_gSubMode){
  case MODE_MONITOR_IDLE: // Mode IDLE
    // In this case we only monitor the moisture level without taking any action
    // Check what are the possible button pressed
    if(button==LCDButton::btnSELECT) { // If Select was pressed, move to CALIB mode
      _gSubMode = MODE_MONITOR_RUN;
      // LCD does display the running symbol
      lcdDisplay.initRunning(true);
    }
      //_gMainMode = MAIN_MODE_CALIB;
    //else if(button==LCDWaterDisplay::btnLEFT) // If Left was pressed, move to SHOW mode
    //  _gMainMode = MAIN_MODE_SHOW;
    //else if(button==LCDWaterDisplay::btnRIGHT) { // If Right was pressed, move to RUNNING sub mode
    //}
    break;
  case MODE_MONITOR_RUN: // Mode IDLE
    // In this mode we monitor the moisture level and start watering if it drops too low
    if(button==LCDButton::btnSELECT){ // If Select button was pressed, move to IDLE sub mode
      // Ensure the pump is not running, reset measurement interval to LONG_INTERVAL
      // and make sure all valves are closed
      lcdDisplay.initRunning(false);
      lcdDisplay.initWatering(false);
      _gSubMode = MODE_MONITOR_IDLE;
      pump1.run(false);
      setSensorsMeasureInterval(LONG_INTERVAL);
      stopAllWatering();
    }
    else{
      bool currentIsWatering = ( pump1.getStatus() == PumpControl::RUNNING);
      bool needWater = monitorCircuits();
      if(needWater && !currentIsWatering){
        // Need watering state changed
        // Enable the pump
        pump1.run(true);
      }
      else if(!needWater && currentIsWatering){
        // Does not need watering state changed
        // Disable the pump
        pump1.run(false);
      }
      // Display or not the watering symbol.
      lcdDisplay.initWatering(needWater);
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
bool AutomaticWater::addSensor(uint8_t pin, uint8_t powerPin) {
  // Not more than MAX_SENSORS
  if(_nCircuits>=AW::MAX_SENSORS) return false;
  sensors[_nCircuits] = new MoistureSensor(pin, powerPin);

  //Setup sensor 1, reading every hour
  sensors[_nCircuits]->setMeasureInterval(LONG_INTERVAL);
  sensors[_nCircuits]->setTickInterval(_gTickInterval);
  ++_nCircuits;

  lcdDisplay.add_circuit();

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
  LCDButton::button button = lcdDisplay.tick();

  // Pass to the rest of the function only every 10 ticks, or when a button is pressed
  if(++_currentCounter<10 && button==LCDButton::btnNONE) return;
  _currentCounter = 0;

  // Tick the sub classes
  tickSensors();
  pump1.tick();

  AW::MainMode mode = lcdDisplay.getMainMode();

  // Run the method associated to the current main mode
  switch(mode) {
  case AW::MAIN_MODE_MONITOR: //MONITORING MODE
    runMonitorMode(button);
    break;
  case AW::MAIN_MODE_CALIB: //CALIBRATION MODE
    runCalibrationMode(button);
    break;
  case AW::MAIN_MODE_SHOW: //SHOW MODE
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
void AutomaticWater::loopActiveSensor(int8_t delta) {
  // Only delta of +1 or -1 are allowed, just for simplicity.
  // Also no sense to loop if there are no sensors.
  if(abs(delta)!=1 || _nCircuits==0) return;

  if(delta<0 && _currentSensor==0) _currentSensor = _nCircuits-1;
  else if(delta>0 && _currentSensor==_nCircuits-1) _currentSensor = 0;
  else _currentSensor += delta;
}

/**
 * Call the tick method of all the existing sensors.
 */
void AutomaticWater::tickSensors() {
  for(unsigned short i=0; i<_nCircuits; ++i) sensors[i]->tick();
}

/**
 * Set the current active sensor. The method does nothing if sensorID is not
 * the ID of an existing sensor.
 *
 * @param sensorID: ID of the sensor to set active
 */
void AutomaticWater::setActiveSensor(uint8_t sensorID) {
  if(sensorID<_nCircuits) _currentSensor = sensorID;
}

/**
 * Set the measure interval on all the existing sensors
 *
 * @param interval: Measure interval to set
 */
void AutomaticWater::setSensorsMeasureInterval(unsigned long int interval) {
  for(unsigned short i=0; i<_nCircuits; ++i) sensors[i]->setMeasureInterval(interval);
}

/**
 * Add a new valve to the designated circuit. The circuit must exist (a sensor with that
 * index must exist) and a valve cannot be already associated to that circuit.
 *
 * @param pin: Digital control pin of the valve
 * @param circuit_index: Index of the circuit to which the valve should be added
 * @return True of the valve has been successfully added, else false.
 */
bool AutomaticWater::addValve(uint8_t pin, uint8_t circuit_index) {
  if(circuit_index>=_nCircuits) return false; // Not allowed to have a valve without sensor
  if(valves[circuit_index]!=nullptr) return false; // Already one valve assigned to that circuit

  valves[circuit_index] = new ValveController(pin);

  return true;
}

/**
 * Add a sensor and an associated valve on the specified pins. It uses successively
 * addSensor and addValve (check documentation).
 *
 * @param sensor_pin: Analog pin on which to read the sensor values
 * @param sensor_powerPin: Digital pin used to provide power to the sensor
 * @param valve_pin: Digital control pin of the valve
 * @return True if both the sensor and the valve have been added successfully, else false.
 */
bool AutomaticWater::addCircuit(uint8_t sensor_pin, uint8_t sensor_powerPin, uint8_t valve_pin) {
  if(addSensor(sensor_pin, sensor_powerPin)) // Add the sensor, continue only if successful
    return addValve(valve_pin, _nCircuits-1); // If the sensor has been added successfully, add a valve to the last circuit.

  return false;
}

/**
 * Monitor every water circuit (sensor + valve pair, even if a valve is not present).
 * Change the measuring interval of each sensor needed water to the SHORT_INTERVAL, and
 * to the LONG_INTERVAL for those not needing it.
 * It also opens the associated valve if it exists.
 *
 * @return True if any sensor needs watering. False if none of them needs it.
 */
bool AutomaticWater::monitorCircuits() {
  for (unsigned short i = 0; i < _nCircuits; ++i) {
    if (sensors[i]->getRawMoisture() > LEVEL_WATER && !_isWatering[i]) {
      // Need water
      _isWatering[i] = true;
      // Shorten the measurement interval to SHORT_INTERVAL for that sensor.
      sensors[i]->setMeasureInterval(SHORT_INTERVAL);
      // And open the valve if exists
      if(valves[i])
        valves[i]->open(true);
    } else if (sensors[i]->getRawMoisture() < LEVEL_WATER && _isWatering[i]){
      // Does not need water
      _isWatering[i] = false;
      // Reset the measurement interval to LONG_INTERVAL for that sensor.
      sensors[i]->setMeasureInterval(LONG_INTERVAL);
      if(valves[i])
        valves[i]->open(false);
    }
  }
  return isWatering();
}

/**
 * Make sure everything stops watering properly. Close all the valves and
 * set the _isWatering variable to false.
 */
void AutomaticWater::stopAllWatering() {
  for(unsigned short i=0; i<_nCircuits; ++i) {
    _isWatering[i] = false;
    if(valves[i]) valves[i]->open(false);
  }
}

/**
 * Checks if any of the circuit is currently watering
 *
 * @return True if any circuit is watering. False if none of them is.
 */
bool AutomaticWater::isWatering() {
  for(unsigned short i=0; i<_nCircuits; ++i) {
    if(_isWatering[i]) return true;
  }

  return false;
}

void AutomaticWater::updatePublications() {
  if(!_controlServer)
    return;

  _pub_pump->updateValue(pump1.getStatus()==PumpControl::RUNNING);
  _pub_pump_enabled->updateValue(pump1.isEnabled());
  for(unsigned short i=0; i<_nCircuits; ++i) {
    _pub_pump_valves[i]->updateValue(valves[i]->isOpen());
    _pub_sensors[i]->updateValue(sensors[i]->getPercentageMoisture());
  }
}


void AutomaticWater::checkCommands() {
  if(!_controlServer)
    return;

  if(_cmd_pump->isUpdated()){
    bool pump_enabled = _cmd_pump->getValue();
    if(pump1.isEnabled()==pump_enabled) return; // Nothing to do
    if(pump_enabled) pump1.Enable();
    else pump1.Disable();
}
}
