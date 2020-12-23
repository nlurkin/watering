/*
 * MenuMonitor.cpp
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include "MenuMonitor.h"
#include <Arduino.h>

MenuMonitor::MenuMonitor(LiquidCrystal &ref) :
  SubMenu(ref),
  _n_screens(0),
  _screen_id(0),
  _l_monitor_header(0, 0, "Moisture      #", _screen_id)
{
  for(uint8_t i=0; i<AW::MAX_SENSORS; ++i){
    _v_monit_raw[i] = 0;
    _v_monit_perc[i] = 0;
    _l_monitor_vals[i] = nullptr;
    _s_monitor[i] = nullptr;
  }
}

MenuMonitor::~MenuMonitor() {
  for(uint8_t i=0; i<_n_screens; ++i){
    delete _s_monitor[i];
    delete _l_monitor_vals[i];
  }
}

void MenuMonitor::init() {
}

bool MenuMonitor::add_screen() {
  if(_n_screens>=AW::MAX_SENSORS)
    return false;

  _l_monitor_vals[_n_screens] = new LiquidLine(0, 1, "R: ", _v_monit_raw[_n_screens], " %", _v_monit_perc[_n_screens]);
  _s_monitor[_n_screens] = new LiquidScreen(_l_monitor_header, *_l_monitor_vals[_n_screens]);

  _menu.add_screen(*_s_monitor[_n_screens]);
  ++_n_screens;

  changed();
  return true;
}

bool MenuMonitor::screen_changed() {
  LiquidScreen *screen = _menu.get_currentScreen();
  for(uint8_t i=0; i<_n_screens; ++i){
    if(screen == _s_monitor[i]){
      _screen_id = i;
      changed();
      return true;
    }
  }
  return false;
}

void MenuMonitor::set_mon_values(int screen, int raw, float perc) {
  if(screen>=_n_screens)
    return;

  _v_monit_raw[screen] = raw;
  _v_monit_perc[screen] = perc;

  if(screen==_screen_id)
    changed();
}
