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
  char buffer[20], buffer2[5];
  static const char header[] PROGMEM = "BME Measurement ";
  static const char T[] PROGMEM = "T: ";
  static const char TUnit[] PROGMEM = "*C";
  static const char P[] PROGMEM = "P: ";
  static const char PUnit[] PROGMEM = "hPa";
  static const char H[] PROGMEM = "H: ";
  static const char HUnit[] PROGMEM = "%";
  static const char A[] PROGMEM = "A: ";
  static const char AUnit[] PROGMEM = "m";

  _l_bme_header      = new LiquidLine(0, 0, header);
  _l_bme_temperature = new LiquidLine(0, 1, T, _bme_temperature, TUnit);
  _s_bme_temperature = new LiquidScreen(*_l_bme_header, *_l_bme_temperature);
  strcpy_P(buffer, PSTR("P: "));
  strcpy_P(buffer2, PSTR("hPa"));
  _l_bme_pressure    = new LiquidLine(0, 1, P, _bme_pressure, PUnit);
  _s_bme_pressure    = new LiquidScreen(*_l_bme_header, *_l_bme_pressure);
  strcpy_P(buffer, PSTR("H: "));
  strcpy_P(buffer2, PSTR("%"));
  _l_bme_humidity    = new LiquidLine(0, 1, H, _bme_humidity, HUnit);
  _s_bme_humidity    = new LiquidScreen(*_l_bme_header, *_l_bme_humidity);
  strcpy_P(buffer, PSTR("A: "));
  strcpy_P(buffer2, PSTR("m"));
  _l_bme_altitude    = new LiquidLine(0, 1, A, _bme_altitude, AUnit);
  _s_bme_altitude    = new LiquidScreen(*_l_bme_header, *_l_bme_altitude);

  _l_bme_header->set_asProgmem(1);
  _l_bme_temperature->set_asProgmem(1);
  _l_bme_temperature->set_asProgmem(3);
  _l_bme_pressure->set_asProgmem(1);
  _l_bme_pressure->set_asProgmem(3);
  _l_bme_humidity->set_asProgmem(1);
  _l_bme_humidity->set_asProgmem(3);
  _l_bme_altitude->set_asProgmem(1);
  _l_bme_altitude->set_asProgmem(3);
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

