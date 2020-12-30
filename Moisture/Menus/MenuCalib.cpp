/*
 * MenuCalib.cpp
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include "MenuCalib.h"
#include <Arduino.h>

MenuCalib::MenuCalib(LiquidCrystal &ref) :
  SubMenu(ref),
  _val(0),
  _val_avg(0),
  _sensor(0),
  _mode_letter(' ')
{
  strcpy_P(_dval, PSTR("Sel. sens.->SEL"));
  init();
}

MenuCalib::~MenuCalib() {
  delete _s_monitor_wait;
  delete _l_bme_header;
  delete _l_bme_dry;
}

void MenuCalib::init() {
  _l_bme_header      = new LiquidLine(0, 0, "CALIB mode ", _mode_letter, "  #", _sensor);
  _l_bme_dry = new LiquidLine(0, 1, _dval);
  _l_bme_run = new LiquidLine(0, 1, "R:", _val, " A:", _val_avg);
  _s_monitor_wait = new LiquidScreen(*_l_bme_header, *_l_bme_dry);
  _s_monitor_run = new LiquidScreen(*_l_bme_header, *_l_bme_run);

  _s_monitor_run->hide(true);
  _menu.add_screen(*_s_monitor_wait);
  _menu.add_screen(*_s_monitor_run);
//  _l_bme_dry->set_asProgmem(2);

  changed();
}

void MenuCalib::set_values(calibType type, bool sel) {
  if(sel) {
    _mode_letter = ' ';
    _s_monitor_wait->hide(false);
    _s_monitor_run->hide(true);
    if(type==WATER)
      strcpy_P(_dval, PSTR("Put water -> SEL"));
    else if(type==DRY)
      strcpy_P(_dval, PSTR("Put dry   -> SEL"));
    _menu.change_screen(_s_monitor_wait);
  }
  else {
    _s_monitor_wait->hide(true);
    _s_monitor_run->hide(false);
    if(type==WATER)
      _mode_letter = 'W';
    else if(type==DRY)
      _mode_letter = 'D';

    _val = 0;
    _val_avg = 0.;

    _menu.change_screen(_s_monitor_run);
  }

  changed();
}

void MenuCalib::set_values(int val, float avg) {
  _val = val;
  _val_avg = avg;

  changed();
}

void MenuCalib::set_sensor(uint8_t sensor) {
  _sensor = sensor;

  changed();
}

void MenuCalib::reset() {
  _mode_letter = ' ';
  strcpy_P(_dval, PSTR("Sel. sens.->SEL"));
  _menu.change_screen(_s_monitor_wait);

  changed();
}
