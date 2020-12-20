/*
 * BME280Reader.h
 *
 *  Created on: 20 Dec 2020
 *      Author: nlurkin
 */

#ifndef BME280READER_H_
#define BME280READER_H_

#include <Adafruit_BME280.h>
#include "Publication.h"

class MenuBME;
class ControlServer;

class BME280Reader {
public:
  BME280Reader();
  virtual ~BME280Reader();

  bool init(MenuBME *menu=nullptr);
  void setPublicationServer(ControlServer *server);

  bool updateAll();

private:
  Adafruit_BME280      _bme;
  MenuBME             *_m_bme;
  ControlServer       *_controlServer;
  Publication<double> *_pub_temperature;
  Publication<double> *_pub_pressure;
  Publication<double> *_pub_humidity;
};

#endif /* BME280READER_H_ */
