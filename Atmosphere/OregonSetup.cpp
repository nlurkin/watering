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
  _controlServer(nullptr),
  _oregon(new OregonReader),
  _pub_temperature(nullptr),
  _pub_humidity(nullptr)
{

}

OregonSetup::~OregonSetup() {
}

void OregonSetup::setPublicationServer(ControlServer *server){
  _controlServer = server;
  if(server==nullptr)
    return;
  // Create publications and add them
  _pub_temperature = new Publication<double>("oregon_temperature");
  _controlServer->addPublication(_pub_temperature);
  _pub_humidity    = new Publication<int>("oregon_humidity");
  _controlServer->addPublication(_pub_humidity);
}

bool OregonSetup::init(uint8_t pin) {
  OregonReader::setup(pin);

  return true;
}

void OregonSetup::updateAll() {
  while(_oregon->loop()) {}

  if(!_oregon->available())
    return;

  OregonDecoder sensor = _oregon->get_data();
  //sensor.print();

  _pub_temperature->updateValue(sensor.get_temperature());
  _pub_humidity->updateValue(sensor.get_relative_humidity());
}
