/*
 * PumpControl.h
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#ifndef PUMPCONTROL_H_
#define PUMPCONTROL_H_

#include <Arduino.h>

/**
 * \brief This is the class controlling the pump.
 *
 * It is simply running the pump for a defined amount of time.
 * A running period is always followed by a dead time during which
 * the pump is OFF and will not accept any order.
 * This is to prevent the pump from continuously running due to fault
 * with the sensor or empty water tank.
 */
class PumpControl {
public:
	enum state {IDLE, RUNNING, DEAD}; /** enum providing the list of possible states for the pump: IDLE and RUNNING are self explanatory, DEAD means dead time */

	PumpControl(uint8_t pin);
	virtual ~PumpControl();

	void tick();
	void run(bool state);

	void setPin         (uint8_t pin              );
	void setDeadTime    (unsigned int deadTime    ) { _deadTime = deadTime;         }
	void setRunningTime (unsigned int runningTime ) { _runningTime = runningTime;   }
	void setTickInterval(unsigned int tickInterval) { _tickInterval = tickInterval; }

	uint8_t getPin()               const { return _pin;          }
	unsigned int getDeadTime()     const { return _deadTime;	    }
	unsigned int getRunningTime()  const { return _runningTime;  }
	unsigned int getTickInterval() const { return _tickInterval; }

private:
	uint8_t   _pin;                 /** Digital pin on which the pump is connected */
	unsigned int   _deadTime;       /** Length of the dead time */
	unsigned int   _runningTime;    /** Length of the running time */
	unsigned int   _tickInterval;   /** Length of a tick */
	unsigned int   _currentCounter; /** Counter of ticks */
	state _on;                      /** Current state of the pump */
};

#endif /* PUMPCONTROL_H_ */
