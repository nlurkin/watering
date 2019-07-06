/*
 * MoistureSensor.h
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#ifndef MOISTURESENSOR_H_
#define MOISTURESENSOR_H_

#include <Arduino.h>

/**
 * \brief This class handles the readings from the moisture sensor.
 *
 * It reads the values from the sensor at the required interval. It contains 2 calibration
 * values (DRY and WATER) which give the range for the moisture percentage.
 */
class MoistureSensor {
public:
	MoistureSensor(uint8_t pin, uint8_t powerPin);
	virtual ~MoistureSensor();

	void tick();
	float computeAverage();
	void resetCalibration();

	bool readCalibrationValue();
	bool readCalibrationWater();
	bool readCalibrationDry();

	void setMeasureInterval(unsigned long measureInterval) { _measureInterval = measureInterval; }
	void setTickInterval   (unsigned int tickInterval    ) { _tickInterval = tickInterval;       }
	void setPin            (uint8_t pin                  ) { _pin = pin;                         }
	void setPowerPin       (uint8_t pin                  );

	int getRawMoisture()               const { return _lastValue;       }
	unsigned long getMeasureInterval() const { return _measureInterval; }
	uint8_t getPin()                   const { return _pin;             }
	uint8_t getPowerPin()              const { return _powerPin;        }
	unsigned int getTickInterval()     const { return _tickInterval;    }
	int getDryValue()                  const { return _DRY_VALUE;       }
	int getWaterValue()                const { return _WATER_VALUE;     }
	int getPercentageMoisture()        const;

private:
	uint8_t _pin;      /** Analog pin on which the raw values are read */
	uint8_t _powerPin; /** Digital pin used to power the sensor */

	unsigned long _measureInterval; /** Interval between 2 consecutive measurements [ms] */
	unsigned int  _tickInterval;    /** Length of a tick [ms] */
	unsigned int  _currentCounter;  /** Internal tick counter */

	unsigned short _currentValueIndex; /** Index of the current value in the _valArray buffer */
	int _lastValue;                    /** Last raw value read */
	int _valArray[30];                 /** Buffer of values used for averaging during calibration */
	int _WATER_VALUE;                  /** Calibration value for maximum moisture */
	int _DRY_VALUE;                    /** Calibration value for minimum moisture */
};

#endif /* MOISTURESENSOR_H_ */
