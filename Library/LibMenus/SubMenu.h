/*
 * SubMenu.h
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef LIBMENUS_SUBMENU_H_
#define LIBMENUS_SUBMENU_H_

#include <LiquidMenu.h>

/**
 * SubMenu abstract class. All SubMenus used in LCDDisplay must derive from it.
 */
class SubMenu {
public:
  SubMenu(LiquidCrystal& ref);
  virtual ~SubMenu();

  virtual void init() = 0; /** Pure virtual init method to be used and called by the constructor */
  bool has_changed();
  LiquidMenu& get_menu_handle();

protected:
  void changed();

  LiquidMenu _menu;  /** Reference to the LiquidMenu instance to be passed to the LiquidSystem */
private:
  bool _has_changed; /** Tells whether the content of the menu has changed */
};

#endif /* LIBMENUS_SUBMENU_H_ */
