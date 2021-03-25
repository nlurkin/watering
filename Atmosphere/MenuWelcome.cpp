/*
 * MenuWelcome.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "MenuWelcome.h"

MenuWelcome::MenuWelcome(LiquidCrystal& ref) :
  SubMenu(ref),
  _sym_connected("  o"),
  _project_name{"Atmosphere"}
{
  init();
}

MenuWelcome::~MenuWelcome() {
  delete _s_welcome;
  delete _l_welcome_top;
  delete _l_welcome_bottom;
}

void MenuWelcome::init() {
  _l_welcome_top    = new LiquidLine(1, 0, "LiquidMenu ", LIQUIDMENU_VERSION);
  _l_welcome_bottom = new LiquidLine(0, 1, _project_name, _sym_connected);
  _s_welcome        = new LiquidScreen(*_l_welcome_top, *_l_welcome_bottom);

  _menu.add_screen(*_s_welcome);
  changed();
}

void MenuWelcome::connected(bool val) {
  if(val)
    strcpy_P(_sym_connected, PSTR("  +"));
  else
    strcpy_P(_sym_connected, PSTR("  -"));
}

void MenuWelcome::communication(bool val) {
  if(val)
    strcpy_P(_sym_connected, PSTR("  -"));
  else
    strcpy_P(_sym_connected, PSTR("  o"));
}
