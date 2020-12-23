/*
 * LCDDisplay.h
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#ifndef LCDWATERDISPLAY_H_
#define LCDWATERDISPLAY_H_

#include "LCDDisplay.h"
#include "AutomaticWaterConfig.h"
#include "Menus/MenuWelcome.h"
#include "Menus/MenuMonitor.h"
#include "Menus/MenuCalib.h"
#include "Menus/MenuShow.h"

/**
 * \brief This class handles the LCD display and buttons (1602 LCD Keypad shield).
 *
 * The predefined messages are encoded in this class and simply enabled from outside,
 * passing the required dynamic parameters.
 * It is also listening to the button events.
 */
class LCDWaterDisplay {
public:
  LCDWaterDisplay();
  virtual ~LCDWaterDisplay();

  void initCalibrationMode(MenuCalib::calibType type);
  void initMonitorMode();
  void initShowMode();
  void initWatering(bool watering);
  void initRunning(bool running);

  void lcd_clear_line(uint8_t l);

  void displayCalibMode(MenuCalib::calibType type);
  void displayCalibValues(int raw, float average);
  void displayShowConstants(int screen, int water, int dry);
  void displayRunValues(int screen, int raw, float perc);

  bool add_circuit();

  LCDButton::button tick();

private:
  LCDDisplay _lcd; /** Reference to the LCD display*/
  MenuWelcome _m_welcome;
  MenuMonitor _m_monitor;
  MenuShow    _m_show;
  MenuCalib   _m_calib;
};

#endif /* LCDWATERDISPLAY_H_ */
