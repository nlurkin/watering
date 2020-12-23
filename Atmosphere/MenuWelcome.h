/*
 * MenuWelcome.h
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef MENUWELCOME_H_
#define MENUWELCOME_H_

#include "LibMenus/SubMenu.h"

class MenuWelcome: public SubMenu {
public:
	MenuWelcome(LiquidCrystal& ref, const char name[16]);
	virtual ~MenuWelcome();
	virtual void init();

private:
	const char _project_name[16];     /** Project name to be displayed on the welcome screen */
	LiquidScreen *_s_welcome;
	LiquidLine   *_l_welcome_top;
	LiquidLine   *_l_welcome_bottom;
};

#endif /* MENUWELCOME_H_ */
