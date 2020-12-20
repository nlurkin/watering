/*
 * BMEMenu.h
 *
 *  Created on: 21 Jul 2019
 *      Author: nicol
 */

#ifndef MENUBME_H_
#define MENUBME_H_

#include "SubMenu.h"

/*
 *
 */
class MenuBME: public SubMenu {
public:
	MenuBME(LiquidCrystal& ref);
	virtual ~MenuBME();

	virtual void init();
	void set_values(float t, float p, float h, float a);

private:
	float _bme_temperature, _bme_pressure, _bme_humidity, _bme_altitude;
	LiquidLine   *_l_bme_header;
	LiquidScreen *_s_bme_temperature;
	LiquidLine   *_l_bme_temperature;
	LiquidScreen *_s_bme_pressure;
	LiquidLine   *_l_bme_pressure;
	LiquidScreen *_s_bme_humidity;
	LiquidLine   *_l_bme_humidity;
	LiquidScreen *_s_bme_altitude;
	LiquidLine   *_l_bme_altitude;
};

#endif /* MENUBME_H_ */
