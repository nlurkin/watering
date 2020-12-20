/*
 * BME280Reader.h
 *
 *  Created on: 20 Dec 2020
 *      Author: nlurkin
 */

#ifndef BME280READER_H_
#define BME280READER_H_

#include <Adafruit_BME280.h>

class MenuBME;

class BME280Reader {
public:
  BME280Reader();
  virtual ~BME280Reader();

  bool init(MenuBME *menu=nullptr);

  bool updateAll();

private:
  Adafruit_BME280      _bme;
  MenuBME             *_m_bme;
};

#endif /* BME280READER_H_ */
