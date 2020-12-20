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

	void Enable()  { _enable = true;  }
	void Disable() { _enable = false; open(false); }
	void open(bool state);
	void toggle();

	bool isOpen()    const { return _open;   }
	bool isEnabled() const { return _enable; }

private:
	uint8_t  _pin; /** Digital pin on which the pump is connected */
	bool _open;    /** Current state of the pump */
	bool _enable;  /** Enable/disable valve */
};

#endif /* VALVECONTROLLER_H_ */
