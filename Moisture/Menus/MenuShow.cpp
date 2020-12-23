/*
 * MenuShow.cpp
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include "MenuShow.h"

MenuShow::MenuShow(LiquidCrystal &ref) :
  SubMenu(ref),
  _n_screens(0),
  _screen_id(0),
  _l_const_header(0, 0, "Calib consts  #", _screen_id)
{
  for(uint8_t i=0; i<AW::MAX_SENSORS; ++i){
    _v_const_water[i] = 0;
    _v_const_dry[i] = 0;
    _l_const_vals[i] = nullptr;
    _s_const[i] = nullptr;
  }
}

MenuShow::~MenuShow() {
  for(uint8_t i=0; i<_n_screens; ++i){
    delete _s_const[i];
    delete _l_const_vals[i];
  }
}

void MenuShow::init() {
}

bool MenuShow::add_screen() {
  if(_n_screens>=AW::MAX_SENSORS)
    return false;

  _l_const_vals[_n_screens] = new LiquidLine(0, 1, "W: ", _v_const_water[_n_screens], " D:", _v_const_dry[_n_screens]);
  _s_const[_n_screens] = new LiquidScreen(_l_const_header, *_l_const_vals[_n_screens]);

  _menu.add_screen(*_s_const[_n_screens]);
  ++_n_screens;

  changed();
  return true;
}

bool MenuShow::screen_changed() {
  LiquidScreen *screen = _menu.get_currentScreen();
  for(uint8_t i=0; i<_n_screens; ++i){
    if(screen == _s_const[i]){
      _screen_id = i;
      changed();
      return true;
    }
  }
  return false;
}

void MenuShow::set_const_values(int screen, int water, int dry) {
  if(screen>=_n_screens)
    return;
  _v_const_water[screen] = water;
  _v_const_dry[screen] = dry;

  if(screen==_screen_id)
    changed();
}
