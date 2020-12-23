/*
 * MenuShow.h
 *
 *  Created on: 23 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef MENUS_MENUSHOW_H_
#define MENUS_MENUSHOW_H_

#include <SubMenu.h>
#include "../AutomaticWaterConfig.h"

class MenuShow: public SubMenu {
public:
  MenuShow(LiquidCrystal &ref);
  virtual ~MenuShow();

  virtual void init();
  virtual bool screen_changed();
  bool add_screen();
  void set_const_values(int screen, int water, int dry);

private:
  int _n_screens;
  int _screen_id;
  int _v_const_water[AW::MAX_SENSORS];
  int _v_const_dry[AW::MAX_SENSORS];
  LiquidScreen *_s_const[AW::MAX_SENSORS];
  LiquidLine   *_l_const_vals[AW::MAX_SENSORS];
  LiquidLine   _l_const_header;
};

#endif /* MENUS_MENUSHOW_H_ */
