/*
 * MenuMonitor.h
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef MENUMONITOR_H_
#define MENUMONITOR_H_

#include "SubMenu.h"
#include "../AutomaticWaterConfig.h"

class MenuMonitor: public SubMenu {
public:
  MenuMonitor(LiquidCrystal& ref);
  virtual ~MenuMonitor();

  virtual void init();
  virtual bool screen_changed();
  bool add_screen();
  void set_mon_values(int screen, int raw, float perc);

private:
  int _n_screens;
  int _screen_id;
  int _v_monit_raw[AW::MAX_SENSORS];
  int _v_monit_perc[AW::MAX_SENSORS];
  LiquidScreen *_s_monitor[AW::MAX_SENSORS];
  LiquidLine   *_l_monitor_vals[AW::MAX_SENSORS];
  LiquidLine   _l_monitor_header;
};

#endif /* MENUMONITOR_H_ */
