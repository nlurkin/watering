/*
 * LCDDisplay.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#include "LCDWaterDisplay.h"
#include <Arduino.h>

/**
 * Constructor. Initialises the LCD display driver.
 */
LCDWaterDisplay::LCDWaterDisplay() :
  _lcd(100),
  _m_welcome(_lcd.get_lcd_handle()),
  _m_monitor(_lcd.get_lcd_handle()),
  _m_show(_lcd.get_lcd_handle()),
  _m_calib(_lcd.get_lcd_handle())
{
  _lcd.add_menu(&_m_welcome);
  _lcd.add_menu(&_m_monitor);
  _lcd.add_menu(&_m_show);
  _lcd.add_menu(&_m_calib);
}

/**
 * Destructor.
 */
LCDWaterDisplay::~LCDWaterDisplay() {
}


bool LCDWaterDisplay::add_circuit() {
  return _m_show.add_screen() && _m_monitor.add_screen();
}
/**
 * Prepare the display for the calibration mode. This is the initial state where
 * the user is asked to prepare the calibration and press the Select button.
 *
 * ###################
 * #CALIB mode       #
 * #Put {mode} -> SEL#
 * ###################
 *
 * @param type: Specify the phase of the calibration mode (wet or dry).
 */
void LCDWaterDisplay::initCalibrationMode(MenuCalib::calibType type){
  _m_calib.set_values(type, true);
  // First line
  /*
  lcd_clear_line(0);
  _lcd.print(F("CALIB mode"));

  //Second line
  lcd_clear_line(1);
  if(type==MenuMonitor::WATER)
    _lcd.print(F("Put water -> SEL"));
  else if(type==MenuMonitor::DRY)
    _lcd.print(F("Put dry -> SEL"));
  */
}

void LCDWaterDisplay::resetCalibrationMode(){
  _m_calib.reset();
}

/**
 * Prepare the display for the monitor mode.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R:{val}  {fr}%@~#
 * ##################
 */
void LCDWaterDisplay::initMonitorMode(){
  // First line
  //lcd_clear_line(0);
//  _lcd.print(F("Moisture,SEL=Cal"));
  //_lcd_monit.change_menu(&_m_monitor);
  //_lcd_monit.force_refresh();
}

/**
 * Prepare the display for the Show mode.
 *
 * ##################
 * #Calib const     #
 * #W:{val} D:{val} #
 * ##################
 */
void LCDWaterDisplay::initShowMode(){
  // First line
//  lcd_clear_line(0);
//  _lcd.print(F("Calib const"));
}

/**
 * Clear the specified line and returns the cursor at the beginning of the line.
 * @param l: Display line to clear
 */
void LCDWaterDisplay::lcd_clear_line(uint8_t l){
//  _lcd.setCursor(0, l);
//  _lcd.print(F("                "));
//  _lcd.setCursor(0,l);
}

/**
 * Display the sensor values on the second line. To be used during calibration mode.
 *
 * ###################
 * #CALIB mode {type}#
 * #R:{val}  A:{val} #
 * ###################
 *
 * @param raw: Raw sensor value
 * @param average: Average sensor value
 */
void LCDWaterDisplay::displayCalibValues(int raw, float average) {
  _m_calib.set_values(raw, average);
//  lcd_clear_line(1);
//  _lcd.print(F("R:"));
//  _lcd.print(raw);
//  _lcd.setCursor(8,1);
//  _lcd.print(F("A:"));
//  _lcd.print(average);
}

void LCDWaterDisplay::displayCalibSensor(uint8_t sensor) {
  _m_calib.set_sensor(sensor);
}


/**
 * Display the calibration type at the end of the first line. To be used during calibration mode.
 *
 * ###################
 * #CALIB mode {type}#
 * #R:{val}  A:{val} #
 * ###################
 *
 * @param type: Enum corresponding to the type to display
 */
void LCDWaterDisplay::displayCalibMode(MenuCalib::calibType type) {
  _m_calib.set_values(type, false);
//  _lcd.setCursor(11,0);
//  if(type==MenuMonitor::WATER)
//    _lcd.print(F("water"));
//  else if(type==MenuMonitor::DRY)
//    _lcd.print(F("dry"));
}

/**
 * Display dynamic sensor values while in monitoring mode.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R: {val}  {fr}% #
 * ##################
 *
 * @param raw: Raw sensor value
 * @param perc: Percentage value according to the calibration wet and dry
 */
void LCDWaterDisplay::displayRunValues(int screen, int raw, float perc) {
  _m_monitor.set_mon_values(screen, raw, perc);
//  lcd_clear_line(1);
//  _lcd.print(F("R:"));
//  _lcd.print(raw);
//  _lcd.setCursor(8,1);
//  _lcd.print((int)perc);
//  _lcd.setCursor(11,1);
//  _lcd.print(F("%"));
}

/**
 * Display calibration constants on SHOW mode.
 *
 * ##################
 * #Calib const     #
 * #W:{val} D:{val} #
 * ##################
 *
 * @param water: Wet constant
 * @param dry: Dry constant
 */
void LCDWaterDisplay::displayShowConstants(int screen, int water, int dry) {
  _m_show.set_const_values(screen, water, dry);
//  lcd_clear_line(1);
//  _lcd.print(F("W:"));
//  _lcd.print(water);
//  _lcd.setCursor(8,1);
//  _lcd.print(F("D:"));
//  _lcd.print(dry);
}

/**
 * Switch on/off the watering symbol (~) in monitoring mode at the end of the second line.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R:{val}  {fr}%@~#
 * ##################
 *
 * @param watering: True to display, false to hide
 */
void LCDWaterDisplay::initWatering(bool watering) {
//  _lcd.setCursor(15,2);
//  if(watering)
//    _lcd.print(F("~"));
//  else
//    _lcd.print(F(" "));
}

/**
 * Switch on/off the running symbol (@) in monitoring mode at the one but last character of the second line.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R:{val}  {fr}%@~#
 * ##################
 *
 * @param running: True to display, false to hide
 */
void LCDWaterDisplay::initRunning(bool running) {
//  _lcd.setCursor(14,1);
//  if(running)
//    _lcd.print(F("@"));
//  else
//    _lcd.print(F(" "));
}

LCDButton::button LCDWaterDisplay::tick() {
  LCDButton::button btn = _lcd.tick();
  const SubMenu *menu = _lcd.get_current_menu();
  if(&_m_show==menu)
      _gMainMode = AW::MAIN_MODE_SHOW;
  else if(&_m_calib==menu)
    _gMainMode=AW::MAIN_MODE_CALIB;
  else
    _gMainMode = AW::MAIN_MODE_MONITOR;

  return btn;
}

void LCDWaterDisplay::disableMenuChange() {
  _lcd.EnableNavigation(false);
}

void LCDWaterDisplay::enableMenuChange() {
  _lcd.EnableNavigation(true);
}
