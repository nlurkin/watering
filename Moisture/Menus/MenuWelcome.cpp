/*
 * MenuWelcome.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "MenuWelcome.h"

MenuWelcome::MenuWelcome(LiquidCrystal& ref) :
  SubMenu(ref)
{
  init();
}

MenuWelcome::~MenuWelcome() {
  delete _s_welcome;
  delete _l_welcome_top;
  delete _l_welcome_bottom;
}

void MenuWelcome::init() {
  static const char title[]        PROGMEM = "LiquidMenu";
  static const char project_name[] PROGMEM = "Moisture";
  _l_welcome_top    = new LiquidLine(1, 0, title, LIQUIDMENU_VERSION);
  _l_welcome_bottom = new LiquidLine(0, 1, project_name);
  _s_welcome        = new LiquidScreen(*_l_welcome_top, *_l_welcome_bottom);

  _l_welcome_top->set_asProgmem(1);
  _l_welcome_bottom->set_asProgmem(1);
  _menu.add_screen(*_s_welcome);
  changed();
}
