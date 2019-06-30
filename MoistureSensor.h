/*
 * MoistureSensor.h
 *
 *  Created on: 4 Nov 2017
 *      Author: nicol
 */

#ifndef MOISTURESENSOR_H_
#define MOISTURESENSOR_H_

class MoistureSensor {
public:
	MoistureSensor(int pin, int powerPin);
	virtual ~MoistureSensor();

	void tick();
	float computeAverage();
	void resetCalibration();

	bool readCalibrationValue ();
	bool readCalibrationWater ();
	bool readCalibrationDry   ();

	void setMeasureInterval(long int measureInterval) { _measureInterval = measureInterval; }
	void setPin            (int pin            ) { _pin = pin;                         }
	void setPowerPin       (int pin            );
	void setTickInterval   (int tickInterval   ) { _tickInterval = tickInterval;       }

	int getRawMoisture()     const { return _lastValue; }
	int getMeasureInterval() const { return _measureInterval; }
	int getPin()             const { return _pin;             }
	int getPowerPin()        const { return _powerPin;        }
	int getTickInterval()    const { return _tickInterval;    }
	int getDryValue()        const { return _DRY_VALUE;       }
	int getWaterValue()      const { return _WATER_VALUE;     }
	int getPercentageMoisture() const;

private:
	int _pin;
	int _powerPin;

	long int _measureInterval; //in milliseconds
	int _tickInterval; //in milliseconds
	int _currentCounter;

	int _lastValue;

	int _valArray[30];
	int _currentValueIndex;
	int _WATER_VALUE;
	int _DRY_VALUE;
};

#endif /* MOISTURESENSOR_H_ */
