/*
 * LCDDisplay.cpp
 *
 *  Created on: 4 Nov 2017
 *      Author: Nicolas Lurkin
 */

#include "LCDWaterDisplay.h"
#include <Arduino.h>

/**
 * Constructor. Initialises the LCD display driver.
 */
LCDWaterDisplay::LCDWaterDisplay() :
	_lcd(8, 9, 4, 5, 6, 7) //List of used pins
{
   _lcd.begin(16, 2);               // start the library
   _lcd.setCursor(0,0);             // set the LCD cursor position
}

/**
 * Destructor.
 */
LCDWaterDisplay::~LCDWaterDisplay() {
}

/**
 * Prepare the display for the calibration mode. This is the initial state where
 * the user is asked to prepare the calibration and press the Select button.
 *
 * ###################
 * #CALIB mode       #
 * #Put {mode} -> SEL#
 * ###################
 *
 * @param type: Specify the phase of the calibration mode (wet or dry).
 */
void LCDWaterDisplay::initCalibrationMode(calibType type){
	// First line
	lcd_clear_line(0);
	_lcd.print("CALIB mode");

	//Second line
	lcd_clear_line(1);
	if(type==WATER)
		_lcd.print("Put water -> SEL");
	else if(type==DRY)
		_lcd.print("Put dry -> SEL");
}

/**
 * Prepare the display for the monitor mode.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R:{val}  {fr}%@~#
 * ##################
 */
void LCDWaterDisplay::initMonitorMode(){
	// First line
	lcd_clear_line(0);
	_lcd.print("Moisture,SEL=Cal");
}

/**
 * Prepare the display for the Show mode.
 *
 * ##################
 * #Calib const     #
 * #W:{val} D:{val} #
 * ##################
 */
void LCDWaterDisplay::initShowMode(){
	// First line
	lcd_clear_line(0);
	_lcd.print("Calib const");
}

/**
 * Clear the specified line and returns the cursor at the beginning of the line.
 * @param l: Display line to clear
 */
void LCDWaterDisplay::lcd_clear_line(uint8_t l){
	_lcd.setCursor(0, l);
	_lcd.print("                ");
	_lcd.setCursor(0,l);
}

/**
 * Read the currently pressed button.
 * From Mark Bramwell, July 2010
 * @return Enum type corresponding to the read button
 */
int LCDWaterDisplay::read_LCD_buttons(){
    int adc_key_in = analogRead(0);       // read the value from the digital pin

    // my buttons when read are centered at these values: 0, 144, 329, 504, 741
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

/**
 * Display the sensor values on the second line. To be used during calibration mode.
 *
 * ###################
 * #CALIB mode {type}#
 * #R:{val}  A:{val} #
 * ###################
 *
 * @param raw: Raw sensor value
 * @param average: Average sensor value
 */
void LCDWaterDisplay::displayCalibValues(int raw, float average) {
	lcd_clear_line(1);
	_lcd.print("R:");
	_lcd.print(raw);
	_lcd.setCursor(8,1);
	_lcd.print("A:");
	_lcd.print(average);
}

/**
 * Display the calibration type at the end of the first line. To be used during calibration mode.
 *
 * ###################
 * #CALIB mode {type}#
 * #R:{val}  A:{val} #
 * ###################
 *
 * @param type: Enum corresponding to the type to display
 */
void LCDWaterDisplay::displayCalibMode(calibType type) {
	_lcd.setCursor(11,0);
	if(type==WATER)
		_lcd.print("water");
	else if(type==DRY)
		_lcd.print("dry");
}

/**
 * Display dynamic sensor values while in monitoring mode.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R: {val}  {fr}% #
 * ##################
 *
 * @param raw: Raw sensor value
 * @param perc: Percentage value according to the calibration wet and dry
 */
void LCDWaterDisplay::displayRunValues(int raw, float perc) {
	lcd_clear_line(1);
	_lcd.print("R:");
	_lcd.print(raw);
	_lcd.setCursor(8,1);
	_lcd.print((int)perc);
	_lcd.setCursor(11,1);
	_lcd.print("%");
}

/**
 * Display calibration constants on SHOW mode.
 *
 * ##################
 * #Calib const     #
 * #W:{val} D:{val} #
 * ##################
 *
 * @param water: Wet constant
 * @param dry: Dry constant
 */
void LCDWaterDisplay::displayShowConstants(int water, int dry) {
	lcd_clear_line(1);
	_lcd.print("W:");
	_lcd.print(water);
	_lcd.setCursor(8,1);
	_lcd.print("D:");
	_lcd.print(dry);
}

/**
 * Switch on/off the watering symbol (~) in monitoring mode at the end of the second line.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R:{val}  {fr}%@~#
 * ##################
 *
 * @param watering: True to display, false to hide
 */
void LCDWaterDisplay::initWatering(bool watering) {
	_lcd.setCursor(15,2);
	if(watering)
		_lcd.print("~");
	else
		_lcd.print(" ");
}

/**
 * Switch on/off the running symbol (@) in monitoring mode at the one but last character of the second line.
 *
 * ##################
 * #Moisture,SEL=Cal#
 * #R:{val}  {fr}%@~#
 * ##################
 *
 * @param running: True to display, false to hide
 */
void LCDWaterDisplay::initRunning(bool running) {
	_lcd.setCursor(14,1);
	if(running)
		_lcd.print("@");
	else
		_lcd.print(" ");
}
