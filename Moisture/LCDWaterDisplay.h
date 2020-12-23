/*
 * LCDDisplay.h
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#ifndef LCDWATERDISPLAY_H_
#define LCDWATERDISPLAY_H_

#include <LiquidCrystal.h>

/**
 * \brief This class handles the LCD display and buttons (1602 LCD Keypad shield).
 *
 * The predefined messages are encoded in this class and simply enabled from outside,
 * passing the required dynamic parameters.
 * It is also listening to the button events.
 */
class LCDWaterDisplay {
public:
	enum button {btnRIGHT, btnUP, btnDOWN, btnLEFT, btnSELECT, btnNONE}; /** enum for the pressed buttons */
	enum calibType {WATER, DRY};                                         /** enum for the predefined calibration messages */

	LCDWaterDisplay();
	virtual ~LCDWaterDisplay();

	void initCalibrationMode(calibType type);
	void initMonitorMode();
	void initShowMode();
	void initWatering(bool watering);
	void initRunning(bool running);

	void lcd_clear_line(uint8_t l);
	int read_LCD_buttons();

	void displayCalibMode(calibType type);
	void displayCalibValues(int raw, float average);
	void displayShowConstants(int water, int dry);
	void displayRunValues(int raw, float perc);


private:
	LiquidCrystal _lcd; /** Reference to the LCD driver */
};

#endif /* LCDWATERDISPLAY_H_ */
