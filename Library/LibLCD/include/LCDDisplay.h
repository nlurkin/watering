/*
 * LCDDisplay.h
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef LIBLCD_LCDDISPLAY_H_
#define LIBLCD_LCDDISPLAY_H_

#include <LiquidCrystal.h>

#include "LCDButton.h"
#include "SubMenu.h"

/**
 * \brief This class is used to handle the LCD display and buttons (1602 LCD Keypad shield).
 *
 * It uses the LiquidMenu to display a dynamic list of menus and navigate through
 * them by using the Up/Down/Left/Right buttons. Up will go back one menu, up to the
 * welcome screen. Down will go down the menus. Left and right will cycle
 * through the difference screens of each menu.
 * It has a minimal update rate, updating the menu only when a button is pressed,
 * or when one of the screens of the current menu is updated.
 */
class LCDDisplay {
public:
  static const uint8_t MAX_MENUS=5;

  LCDDisplay(unsigned int tickInterval, unsigned int updateInterval=1000);
  LCDDisplay(LiquidCrystal &lcd, unsigned int tickInterval, unsigned int updateInterval=1000);
  virtual ~LCDDisplay();

  LCDButton::button tick();
  bool add_menu(SubMenu *menu);
  LiquidCrystal& get_lcd_handle();
  const SubMenu * get_current_menu();
  uint8_t get_current_menu_id();

  bool change_menu(SubMenu *menu);
  void force_refresh();
  void EnableNavigation(bool en) { _NavEnabled = !en; }

private:
  bool check_btn_changes(LCDButton::button btn);
  bool check_updates();

  bool    _NavEnabled;
  uint8_t _nMenus;              /** Number of menus */
  uint8_t _currentMenu;         /** Index of the currently displayed menu */
  unsigned int _tickInterval;   /** Length of a tick*/
  unsigned int _updateInterval; /** Length between consecutive update of the screen*/
  unsigned int _currentCounter; /** Counter of ticks */
  LiquidCrystal _lcd;           /** Reference to the LCD driver */
  LCDButton     _btn;           /** Reference to the button reader */
  LiquidSystem  _menu;          /** Reference to the LiquidMenu instance */

  SubMenu* _subMenus[MAX_MENUS]; /** Array of SubMenus */
};

#endif /* LIBLCD_LCDDISPLAY_H_ */
