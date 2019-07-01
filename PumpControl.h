/*
 * PumpControl.h
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#ifndef PUMPCONTROL_H_
#define PUMPCONTROL_H_

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

	PumpControl(int pin);
	virtual ~PumpControl();

	void tick();
	void run(bool state);

	void setPin         (int pin         );
	void setDeadTime    (int deadTime    ) { _deadTime = deadTime;         }
	void setRunningTime (int runningTime ) { _runningTime = runningTime;   }
	void setTickInterval(int tickInterval) { _tickInterval = tickInterval; }

	int getPin()          const { return _pin;          }
	int getDeadTime()     const { return _deadTime;	    }
	int getRunningTime()  const { return _runningTime;  }
	int getTickInterval() const { return _tickInterval; }

private:
	int   _pin;            /** Digital pin on which the pump is connected */
	int   _deadTime;       /** Length of the dead time */
	int   _runningTime;    /** Length of the running time */
	int   _tickInterval;   /** Length of a tick */
	int   _currentCounter; /** Counter of ticks */
	state _on;             /** Current state of the pump */
};

#endif /* PUMPCONTROL_H_ */
