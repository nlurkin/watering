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
  void set_values(calibType type);

private:
  char _dval[16];
  LiquidScreen *_s_monitor_dry;
  LiquidLine   *_l_bme_header;
  LiquidLine   *_l_bme_dry;
};

#endif /* MENUCALIB_H_ */
