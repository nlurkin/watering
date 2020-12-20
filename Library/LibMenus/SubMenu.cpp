/*
 * SubMenu.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "SubMenu.h"

/**
 * Constructor
 * @param ref: Reference to the LiquidCrystal object on which to display the menu
 */
SubMenu::SubMenu(LiquidCrystal& ref) :
	_has_changed(true), _menu(ref)
{
}

/**
 * Destructor
 */
SubMenu::~SubMenu() {
}

/**
 * Checks whether the menu content has been modified, and reset the state to false
 * @return True if it has been modified, else false
 */
bool SubMenu::has_changed() {
	bool changed = _has_changed;
	_has_changed = false;
	return changed;
}

/**
 * Notifies that the content of the menu has been modified
 */
void SubMenu::changed() {
	_has_changed = true;
}

/**
 * @return Reference to the internal LiquidMenu
 */
LiquidMenu& SubMenu::get_menu_handle() {
	return _menu;
}
