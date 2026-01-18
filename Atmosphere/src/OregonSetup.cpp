/*
 * OregonSetup.cpp
 *
 *  Created on: 17 Jan 2021
 *      Author: Nicolas Lurkin
 */

#include "OregonSetup.h"
#include "ControlServer.h"
#include "OregonReader.h"

OregonSetup::OregonSetup() :
  _nsensors(0),
  _sensors_list{0},
  _controlServer(nullptr),
  _oregon(new OregonReader),
  _pub_temperature{nullptr},
  _pub_humidity{nullptr}
{
}

OregonSetup::~OregonSetup() {
}

void OregonSetup::setPublicationServer(ControlServer *server){
  _controlServer = server;
  if(server==nullptr)
    return;
}

bool OregonSetup::init(uint8_t pin) {
  OregonReader::setup(pin);

  return true;
}

void OregonSetup::updateAll() {
  uint8_t trials=30;
  while(_oregon->loop() && 0<trials--) {}

  if(!_oregon->available())
    return;

  OregonDecoder sensor = _oregon->get_data();
  if(!sensor.checksum()){
    // Serial.println(F("OregonSetup: invalid checksum"));
    return; // At least some data are invalid. Skip this one
  }

  uint8_t rolling_id = sensor.get_rolling_code();
  int8_t index = isSensorKnown(rolling_id);

  if(index<0 && _nsensors==MAX_SENSORS) // Nothing to do, we have no more space for a new sensor
    return;

  // In principle we have here either a known sensor, or space to add more
  if(index==-99) { // First sighting
    addSighting(rolling_id);
    return; // Wait for at least another sighting to confirm its existence
  }
  else if(index<0) { // Second sighting. Add it to the registered sensors
    index = addSensor(rolling_id, sensor, -index);
    if(index==-1) // There has been a problem
      return;
  }

  // Here we should definitely have an already registered sensor
  //sensor.print();

  if(!_controlServer)
    return;

  if(sensor.has_temperature())
    _pub_temperature[index]->updateValue(sensor.get_temperature());
  if(sensor.has_humidity())
    _pub_humidity[index]->updateValue(sensor.get_relative_humidity());
}

int8_t OregonSetup::isSensorKnown(uint8_t id) {
  for(uint8_t i=0; i<MAX_SENSORS; ++i){
    if(_sensors_list[i]==id) // Look for registered sensors
      return i;
    if(_sensors_sightings[i]==id) // Look for sighted but unregistered sensors
      return -i-1; // Add offset of 1, because 0==-0
  }

  return -99;
}

int8_t OregonSetup::addSighting(uint8_t id) {
  for(uint8_t i=0; i<MAX_SENSORS; ++i){
    if(_sensors_sightings[i]==0){
      _sensors_sightings[i] = id; // Record it and return index in array
      return i;
    }
  }
  return -1; // Could not be added
}

int8_t OregonSetup::addSensor(uint8_t id, OregonDecoder &sensor, uint8_t sight_index) {
  --sight_index; // Remove the offset of 1 previously added
  if(sight_index>=MAX_SENSORS) // Not supposed to happen
    return -1;
  if(_nsensors>=MAX_SENSORS) // No more space for a sensor
    return -1;

  // Remove it from sightings
  _sensors_sightings[sight_index] = 0;
  // Add it to registered sensors and then increment number of registered sensors
  uint8_t index = _nsensors++;
  _sensors_list[index] = id;

  // Add new publications
  // Create publications and add them
  char buffer[30];
  uint16_t sensor_id = sensor.get_sensor_id();

  if(!_controlServer)
    return index;

  if(sensor.has_temperature()) {
    sprintf(buffer, "%04X_%02X_temp", sensor_id, id);
    _pub_temperature[index] = new Publication<double>(buffer);
    _controlServer->addPublication(_pub_temperature[index]);
  }
  if(sensor.has_humidity()) {
    sprintf(buffer, "%04X_%02X_hum", sensor_id, id);
    _pub_humidity[index]    = new Publication<int>(buffer);
    _controlServer->addPublication(_pub_humidity[index]);
  }

  //Advertise new publications
  _controlServer->advertise();

  return index; // Return index of the sensor
}
