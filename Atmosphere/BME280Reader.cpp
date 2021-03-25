/*
 * BME280Reader.cpp
 *
 *  Created on: 20 Dec 2020
 *      Author: nlurkin
 */

#include "BME280Reader.h"
#include "MenuBME.h"
#include "ControlServer.h"

BME280Reader::BME280Reader() :
  _m_bme(nullptr),
  _controlServer(nullptr),
  _pub_temperature(nullptr),
  _pub_pressure(nullptr),
  _pub_humidity(nullptr)
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

void BME280Reader::setPublicationServer(ControlServer *server){
  _controlServer = server;
  if(server==nullptr)
    return;
  // Create publications and add them
  char temp[20], press[20], hum[20];
  strcpy_P(temp, PSTR("bme1_temperature"));
  strcpy_P(press, PSTR("bme1_pressure"));
  strcpy_P(hum, PSTR("bme1_humidity"));
  _pub_temperature = new Publication<double>(temp);
  _pub_temperature->updateValue(_bme.readTemperature());
  _controlServer->addPublication(_pub_temperature);
  _pub_pressure    = new Publication<double>(press);
  _pub_pressure->updateValue(_bme.readPressure());
  _controlServer->addPublication(_pub_pressure);
  _pub_humidity    = new Publication<double>(hum);
  _pub_humidity->updateValue(_bme.readHumidity());
  _controlServer->addPublication(_pub_humidity);
}

void BME280Reader::updateAll() {
  double temperature = _bme.readTemperature();
  double pressure = _bme.readPressure()/100;
  double humidity = _bme.readHumidity();
  double altitude = _bme.readAltitude(1019.66);

  //Serial.print(F("Temperature = "));
  //Serial.print(temperature);
  //Serial.println(F(" *C"));
  //
  //Serial.print(F("Pressure = "));
  //Serial.print(pressure); //displaying the Pressure in hPa, you can change the unit
  //Serial.println(F(" hPa"));
  //
  //Serial.print(F("Humidity = "));
  //Serial.print(humidity); //displaying the Pressure in hPa, you can change the unit
  //Serial.println(F(" %"));
  //
  //Serial.print(F("Approx altitude = "));
  //Serial.print(altitude); //The "1019.66" is the pressure(hPa) at sea level in day in your region
  //Serial.println(F(" m"));                    //If you don't know it, modify it until you get your current altitude
  //
  //Serial.println();
  if(_m_bme)
    _m_bme->set_values(temperature, pressure, humidity, altitude);

  _pub_temperature->updateValue(temperature);
  _pub_pressure->updateValue(pressure);
  _pub_humidity->updateValue(humidity);
}
