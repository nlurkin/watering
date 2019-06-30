/*
 * LCDDisplay.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: nicol
 */

#include "LCDWaterDisplay.h"
#include <Arduino.h>

LCDWaterDisplay::LCDWaterDisplay() :
	_lcd(8, 9, 4, 5, 6, 7) //List of used pins
{
   _lcd.begin(16, 2);               // start the library
   _lcd.setCursor(0,0);             // set the LCD cursor position
}

LCDWaterDisplay::~LCDWaterDisplay() {
}

void LCDWaterDisplay::initCalibrationMode(calibType type){
	lcd_clear_line(0);
	_lcd.print("CALIB mode");
	lcd_clear_line(1);
	if(type==WATER)
		_lcd.print("Put water -> SEL");
	else if(type==DRY)
		_lcd.print("Put dry -> SEL");
}

void LCDWaterDisplay::initMonitorMode(){
	lcd_clear_line(0);
	_lcd.print("Moisture,SEL=Calib");
}

void LCDWaterDisplay::initShowMode(){
	lcd_clear_line(0);
	_lcd.print("Calib const");
}


void LCDWaterDisplay::lcd_clear_line(int l){
	_lcd.setCursor(0, l);
	_lcd.print("                ");
	_lcd.setCursor(0,l);
}

int LCDWaterDisplay::read_LCD_buttons(){               // read the buttons
    int adc_key_in = analogRead(0);       // read the value from the sensor

    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result

    if (adc_key_in > 1000) return btnNONE;

    // For V1.1 us this threshold
    if (adc_key_in < 50)   return btnRIGHT;
    if (adc_key_in < 250)  return btnUP;
    if (adc_key_in < 450)  return btnDOWN;
    if (adc_key_in < 650)  return btnLEFT;
    if (adc_key_in < 850)  return btnSELECT;

    return btnNONE;                // when all others fail, return this.
}

void LCDWaterDisplay::displayCalibValues(int raw, float average) {
	lcd_clear_line(1);
	_lcd.print("R:");
	_lcd.print(raw);
	_lcd.setCursor(8,1);
	_lcd.print("A:");
	_lcd.print(average);
}

void LCDWaterDisplay::displayCalibMode(calibType type) {
	_lcd.setCursor(11,0);
	if(type==WATER)
		_lcd.print("water");
	else if(type==DRY)
		_lcd.print("dry");
}

void LCDWaterDisplay::displayRunValues(int raw, float average) {
	//_lcd.setCursor(0,1);
	lcd_clear_line(1);
	_lcd.print("R:");
	_lcd.print(raw);
	_lcd.setCursor(8,1);
	_lcd.print((int)average);
	_lcd.setCursor(11,1);
	_lcd.print("%");
}

void LCDWaterDisplay::displayShowConstants(int water, int dry) {
	lcd_clear_line(1);
	_lcd.print("W:");
	_lcd.print(water);
	_lcd.setCursor(8,1);
	_lcd.print("D:");
	_lcd.print(dry);
}

void LCDWaterDisplay::initWatering(bool watering) {
	_lcd.setCursor(15,2);
	if(watering)
		_lcd.print("~");
	else
		_lcd.print(" ");
}

void LCDWaterDisplay::initRunning(bool running) {
	_lcd.setCursor(14,1);
	if(running)
		_lcd.print("@");
	else
		_lcd.print(" ");
}
