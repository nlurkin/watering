/*
 * BMEMenu.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: nicol
 */

#include "MenuBME.h"

MenuBME::MenuBME(LiquidCrystal &ref) :
  SubMenu(ref)
{
  init();
}

MenuBME::~MenuBME() {
  delete _s_bme_temperature;
  delete _l_bme_temperature;
  delete _s_bme_pressure;
  delete _l_bme_pressure;
  delete _s_bme_humidity;
  delete _l_bme_humidity;
  delete _s_bme_altitude;
  delete _l_bme_altitude;
}

void MenuBME::init() {
  _l_bme_header      = new LiquidLine(0, 0, "BME Measurement");
  _l_bme_temperature = new LiquidLine(0, 1, "T: ", _bme_temperature, "*C");
  _s_bme_temperature = new LiquidScreen(*_l_bme_header, *_l_bme_temperature);
  _l_bme_pressure    = new LiquidLine(0, 1, "P: ", _bme_pressure, "hPa");
  _s_bme_pressure    = new LiquidScreen(*_l_bme_header, *_l_bme_pressure);
  _l_bme_humidity    = new LiquidLine(0, 1, "H: ", _bme_humidity, "%");
  _s_bme_humidity    = new LiquidScreen(*_l_bme_header, *_l_bme_humidity);
  _l_bme_altitude    = new LiquidLine(0, 1, "A: ", _bme_altitude, "m");
  _s_bme_altitude    = new LiquidScreen(*_l_bme_header, *_l_bme_altitude);

  _menu.add_screen(*_s_bme_temperature);
  _menu.add_screen(*_s_bme_pressure);
  _menu.add_screen(*_s_bme_humidity);
  _menu.add_screen(*_s_bme_altitude);

  changed();
}

void MenuBME::set_values(float t, float p, float h, float a) {
  _bme_temperature = t;
  _bme_pressure = p;
  _bme_humidity = h;
  _bme_altitude = a;

  changed();
}

