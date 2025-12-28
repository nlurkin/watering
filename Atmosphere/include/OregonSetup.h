/*
 * OregonSetup.h
 *
 *  Created on: 17 Jan 2021
 *      Author: Nicolas Lurkin
 */

#ifndef OREGONSETUP_H_
#define OREGONSETUP_H_

#include "Publication.h"
#include "OregonDecoder.h"

class ControlServer;
class OregonReader;

class OregonSetup {
public:
  static constexpr uint8_t MAX_SENSORS = 5;
  OregonSetup();
  virtual ~OregonSetup();

  bool init(uint8_t pin);
  void setPublicationServer(ControlServer *server);

  void updateAll();

private:
  int8_t isSensorKnown(uint8_t id);
  int8_t addSensor(uint8_t id, OregonDecoder &sensor, uint8_t sight_index);
  int8_t addSighting(uint8_t id);

  uint8_t              _nsensors;
  uint8_t              _sensors_sightings[MAX_SENSORS];
  uint8_t              _sensors_list[MAX_SENSORS];
  ControlServer       *_controlServer;
  OregonReader        *_oregon;
  Publication<double> *_pub_temperature[MAX_SENSORS];
  Publication<int>    *_pub_humidity[MAX_SENSORS];
};

#endif /* OREGONSETUP_H_ */
