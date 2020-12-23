/*
 * MenuCalib.h
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef MENUCALIB_H_
#define MENUCALIB_H_

#include "SubMenu.h"

/*
 *
 */
class MenuCalib: public SubMenu {
public:
  enum calibType {WATER, DRY};            /** enum for the predefined calibration messages */

  MenuCalib(LiquidCrystal& ref);
  virtual ~MenuCalib();

  virtual void init();
  void reset();
  void set_values(calibType type, bool sel);
  void set_values(int val, float avg);
  void set_sensor(uint8_t sensor);

private:
  int _val, _val_avg;
  int _sensor;
  char _dval[17];
  char _mode_letter;
  LiquidScreen *_s_monitor_wait;
  LiquidScreen *_s_monitor_run;
  LiquidLine   *_l_bme_header;
  LiquidLine   *_l_bme_dry;
  LiquidLine   *_l_bme_run;
};

#endif /* MENUCALIB_H_ */
