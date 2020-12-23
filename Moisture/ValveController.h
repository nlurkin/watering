/*
 * ValveController.h
 *
 *  Created on: 6 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef VALVECONTROLLER_H_
#define VALVECONTROLLER_H_

#include <Arduino.h>

/**
 * \brief This class controls an on/off valve. The valve can be either open or closed.
 * It is controlled by a single digital pin.
 */
class ValveController {
public:
	ValveController(uint8_t pin);
	virtual ~ValveController();

	void open(bool state);
	void toggle();

private:
	uint8_t  _pin; /** Digital pin on which the pump is connected */
	bool _open;    /** Current state of the pump */
};

#endif /* VALVECONTROLLER_H_ */
