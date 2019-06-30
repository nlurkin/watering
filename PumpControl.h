/*
 * PumpControl.h
 *
 *  Created on: 4 Nov 2017
 *      Author: nicol
 */

#ifndef PUMPCONTROL_H_
#define PUMPCONTROL_H_

class PumpControl {
public:
	enum state {IDLE, RUNNING, DEAD};

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
	int _pin;
	int _deadTime;
	int _runningTime;
	int _tickInterval;
	int _currentCounter;
	state _on;
};

#endif /* PUMPCONTROL_H_ */
