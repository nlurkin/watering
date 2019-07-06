/*
 * MoistureSensor.h
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#ifndef MOISTURESENSOR_H_
#define MOISTURESENSOR_H_

/**
 * \brief This class handles the readings from the moisture sensor.
 *
 * It reads the values from the sensor at the required interval. It contains 2 calibration
 * values (DRY and WATER) which give the range for the moisture percentage.
 */
class MoistureSensor {
public:
	MoistureSensor(int pin, int powerPin);
	virtual ~MoistureSensor();

	void tick();
	float computeAverage();
	void resetCalibration();

	bool readCalibrationValue();
	bool readCalibrationWater();
	bool readCalibrationDry();

	void setMeasureInterval(long int measureInterval) { _measureInterval = measureInterval; }
	void setTickInterval   (int tickInterval   )      { _tickInterval = tickInterval;       }
	void setPin            (int pin            )      { _pin = pin;                         }
	void setPowerPin       (int pin            );

	int getRawMoisture()        const { return _lastValue;       }
	int getMeasureInterval()    const { return _measureInterval; }
	int getPin()                const { return _pin;             }
	int getPowerPin()           const { return _powerPin;        }
	int getTickInterval()       const { return _tickInterval;    }
	int getDryValue()           const { return _DRY_VALUE;       }
	int getWaterValue()         const { return _WATER_VALUE;     }
	int getPercentageMoisture() const;

private:
	int _pin;                  /** Analog pin on which the raw values are read */
	int _powerPin;             /** Digital pin used to power the sensor */

	long int _measureInterval; /** Interval between 2 consecutive measurements [ms] */
	int _tickInterval;         /** Length of a tick [ms] */
	int _currentCounter;       /** Internal tick counter */

	int _lastValue;            /** Last raw value read */

	int _valArray[30];         /** Buffer of values used for averaging during calibration */
	int _currentValueIndex;    /** Index of the current value in the _valArray buffer */
	int _WATER_VALUE;          /** Calibration value for maximum moisture */
	int _DRY_VALUE;            /** Calibration value for minimum moisture */
};

#endif /* MOISTURESENSOR_H_ */
