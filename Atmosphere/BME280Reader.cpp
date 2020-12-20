/*
 * BME280Reader.cpp
 *
 *  Created on: 20 Dec 2020
 *      Author: nlurkin
 */

#include "BME280Reader.h"
#include "MenuBME.h"

BME280Reader::BME280Reader() :
  _m_bme(nullptr)
{
}

BME280Reader::~BME280Reader() {
}

bool BME280Reader::init(MenuBME *menu) {
  if(menu)
    _m_bme = menu;

  Serial.println(F("BME280 test"));
  if (!_bme.begin(0x76)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    return false;
  }
  Serial.println(F("Valid BME280 sensor found!"));

  return true;
}

bool BME280Reader::updateAll() {
  double temperature = _bme.readTemperature();
  double pressure = _bme.readPressure()/100;
  double humidity = _bme.readHumidity();
  double altitude = _bme.readAltitude(1019.66);

  Serial.print(F("Temperature = "));
  Serial.print(temperature);
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(pressure); //displaying the Pressure in hPa, you can change the unit
  Serial.println(" hPa");

  Serial.print(F("Humidity = "));
  Serial.print(humidity); //displaying the Pressure in hPa, you can change the unit
  Serial.println(" %");

  Serial.print(F("Approx altitude = "));
  Serial.print(altitude); //The "1019.66" is the pressure(hPa) at sea level in day in your region
  Serial.println(" m");                    //If you don't know it, modify it until you get your current altitude

  Serial.println();
  if(_m_bme)
    _m_bme->set_values(temperature, pressure, humidity, altitude);

  return true;
}
