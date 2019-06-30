/*
 * LCDDisplay.h
 *
 *  Created on: 4 Nov 2017
 *      Author: nicol
 */

#ifndef LCDWATERDISPLAY_H_
#define LCDWATERDISPLAY_H_

#include <LiquidCrystal.h>

class LCDWaterDisplay {
public:
	enum button {btnRIGHT, btnUP, btnDOWN, btnLEFT, btnSELECT, btnNONE};
	enum calibType {WATER, DRY};

	LCDWaterDisplay();
	virtual ~LCDWaterDisplay();

	void initCalibrationMode(calibType type);
	void initMonitorMode();
	void initShowMode();
	void initWatering(bool watering);
	void initRunning(bool running);

	void lcd_clear_line(int l);
	int read_LCD_buttons();

	void displayCalibMode(calibType type);
	void displayCalibValues(int raw, float average);
	void displayShowConstants(int water, int dry);
	void displayRunValues(int raw, float average);


private:
	LiquidCrystal _lcd;
};

#endif /* LCDWATERDISPLAY_H_ */
