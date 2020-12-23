/*
 * MenuCalib.cpp
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include "MenuCalib.h"
#include <Arduino.h>

MenuCalib::MenuCalib(LiquidCrystal &ref) :
  SubMenu(ref)
{
  strcpy(_dval, "dry");
  init();
}

MenuCalib::~MenuCalib() {
  delete _s_monitor_dry;
  delete _l_bme_header;
  delete _l_bme_dry;
}

void MenuCalib::init() {
  _l_bme_header      = new LiquidLine(0, 0, "CALIB mode");
  _l_bme_dry = new LiquidLine(0, 1, "Put ", _dval, " -> SEL");
  _s_monitor_dry = new LiquidScreen(*_l_bme_header, *_l_bme_dry);


  _menu.add_screen(*_s_monitor_dry);
//  _l_bme_dry->set_asProgmem(2);

  changed();
}

void MenuCalib::set_values(calibType type) {
  if(type==WATER)
    strcpy_P(_dval, PSTR("water"));
  else if(type==DRY)
    strcpy_P(_dval, PSTR("dry"));

  changed();
}

