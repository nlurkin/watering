/*
 * OregonSetup.h
 *
 *  Created on: 17 Jan 2021
 *      Author: Nicolas Lurkin
 */

#ifndef OREGONSETUP_H_
#define OREGONSETUP_H_

#include "Publication.h"

class ControlServer;
class OregonReader;

class OregonSetup {
public:
  OregonSetup();
  virtual ~OregonSetup();

  bool init(uint8_t pin);
  void setPublicationServer(ControlServer *server);

  void updateAll();

private:
  ControlServer       *_controlServer;
  OregonReader        *_oregon;
  Publication<double> *_pub_temperature;
  Publication<int>    *_pub_humidity;
};

#endif /* OREGONSETUP_H_ */
