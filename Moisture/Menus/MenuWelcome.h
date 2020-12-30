/*
 * MenuWelcome.h
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef MENUWELCOME_H_
#define MENUWELCOME_H_

#include "SubMenu.h"

class MenuWelcome: public SubMenu {
public:
  MenuWelcome(LiquidCrystal& ref);
  virtual ~MenuWelcome();
  virtual void init();

  void clear_init();

private:
  char          _init_text[5];
  LiquidScreen *_s_welcome;
  LiquidLine   *_l_welcome_top;
  LiquidLine   *_l_welcome_bottom;
};

#endif /* MENUWELCOME_H_ */
