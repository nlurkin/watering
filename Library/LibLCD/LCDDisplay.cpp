/*
 * LCDDisplay.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "LCDDisplay.h"

/**
 * Constructor. Initialises the LCD display driver and the LiquidMenu.
 *
 * @param tickInterval: Interval between consecutive calls to the tick method
 * @param updateInterval: Interval between consecutive refresh of the screen (default 1000 [ms])
 */
LCDDisplay::LCDDisplay(unsigned int tickInterval, unsigned int updateInterval) :
  _NavEnabled(true), _nMenus(0), _currentMenu(0), _tickInterval(tickInterval), _updateInterval(updateInterval), _currentCounter(0),
  _lcd(8, 9, 4, 5, 6, 7), _btn(0)
{
  _lcd.begin(12, 2);
  //_menu.update(); // Too early, no menu defined yet
}

/**
 * Constructor. LiquidMenu and uses the provided LCD display driver.
 *
 * @param lcd: LCD Driver to user
 * @param tickInterval: Interval between consecutive calls to the tick method
 * @param updateInterval: Interval between consecutive refresh of the screen (default 1000 [ms])
 */
LCDDisplay::LCDDisplay(LiquidCrystal &lcd, unsigned int tickInterval, unsigned int updateInterval) :
  _NavEnabled(true), _nMenus(0), _currentMenu(0), _tickInterval(tickInterval), _updateInterval(updateInterval), _currentCounter(0),
  _lcd(lcd), _btn(0)
{
}

/**
 * Destructor
 */
LCDDisplay::~LCDDisplay() {
}

/**
 * In case one of the navigation button was pressed, perform
 * the required action (change screen or change menu)
 *
 * @param btn: Button status from the LCD
 *
 * @return True if a button requiring menu change was pressed, else false
 */
bool LCDDisplay::check_btn_changes(LCDButton::button btn) {
  if(!_NavEnabled)
    return false;
  if (btn == LCDButton::btnRIGHT) { // Cycle to the next screen
    _menu.next_screen();
    if(_subMenus[_currentMenu]->screen_changed())
      _menu.update();
  }
  else if (btn == LCDButton::btnLEFT) { // Cycle to the previous screen
    _menu.previous_screen();
    if(_subMenus[_currentMenu]->screen_changed())
      _menu.update();
  }
  else if (btn == LCDButton::btnUP && _currentMenu > 0) { // Need to possibly switch one menu top
    uint8_t newMenu = _currentMenu - 1;
    _menu.change_menu(_subMenus[newMenu]->get_menu_handle());
    _currentMenu = newMenu;
  } else if (btn == LCDButton::btnDOWN && _currentMenu < _nMenus-1) { // Need to possibly switch one menu down
    uint8_t newMenu = _currentMenu + 1;
    _menu.change_menu(_subMenus[newMenu]->get_menu_handle());
    _currentMenu = newMenu;
  }
  else
    return false; // Default case, no (interesting) button was pressed
  return true;
}

/**
 * Add a new menu in the hierarchy.
 *
 * @param menu: pointer to the menu to add
 *
 * @return True if successful, else false
 */
bool LCDDisplay::add_menu(SubMenu *menu) {
  if(_nMenus<MAX_MENUS-1){ // Do it only if we did not yet reach limit
    _subMenus[_nMenus] = menu;
    _menu.add_menu(menu->get_menu_handle());
    ++_nMenus;
    _menu.update(); // Force refresh of the screen
    return true;
  }
  return false;
}

void LCDDisplay::force_refresh() {
  _menu.update();
}

/**
 * @return handle to the LCD driver as it needs to be passed to the SubMenus on creation
 */
LiquidCrystal& LCDDisplay::get_lcd_handle() {
  return _lcd;
}

const SubMenu *LCDDisplay::get_current_menu() {
  if(_currentMenu>=_nMenus) return nullptr;

  return _subMenus[_currentMenu];
}

uint8_t LCDDisplay::get_current_menu_id() {
  return _currentMenu;
}

bool LCDDisplay::change_menu(SubMenu *menu) {
  for(uint8_t iMenu=0; iMenu<_nMenus; ++iMenu){
    if(_subMenus[iMenu] == menu){
      _menu.change_menu(menu->get_menu_handle());
      _currentMenu = iMenu;
      return true;
    }
  }
  return false;
}

/**
 * Checks if any of the SubMenu state has changed and requires
 * an refresh of the screen.
 * @return True if the screen must be refreshed, else false
 */
bool LCDDisplay::check_updates() {
  return _subMenus[_currentMenu]->has_changed();
}

/**
 * Tick method called at every clock tick.
 * It checks if any navigation or screen refresh
 * needs to be done.
 * @return LCD button status
 */
LCDButton::button LCDDisplay::tick() {
  //Get the button state from the LCD
  LCDButton::button btn = _btn.check_LCD_push();
  bool needs_update = check_btn_changes(btn);

  // Increment the internal tick counter
  ++_currentCounter;
  unsigned int updateTimeInTick = _updateInterval/_tickInterval;
  // Possibly needs refresh
  if(_currentCounter > updateTimeInTick){
    _currentCounter = 0;
    if(needs_update || check_updates()) // Yes it does
      _menu.update();
  }
  return btn;
}

