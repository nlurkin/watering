/*
 * Button.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: nicol
 */

#include "LCDButton.h"

/**
 * Constructor
 *
 * @param pin: Analog pin on which the button state is read
 * @param debounceDelay: Minimum delay to press button for the action to register
 */
LCDButton::LCDButton(uint8_t pin, uint16_t debounceDelay) :
  _pin(pin), _state(btnNONE), _lastState(btnNONE),
  _debounceDelay(debounceDelay), _lastDebounceTime(0)
{
}

/**
 * Read the currently pressed button.
 * From Mark Bramwell, July 2010
 * @return Enum type corresponding to the read button
 */
LCDButton::button LCDButton::read_LCD_buttons(){
    int adc_key_in = analogRead(_pin);       // read the value from the digital pin

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
 * Appropriately checks the change of state of the button.
 * It returns btnNone at all time, except when a button is pressed
 * for long enough (debouneDelay), and only the first time the
 * check is done after pressing the button. If the button is
 * pressed continuously, this methods will return it only once.
 * @return Enum type corresponding to the new button pressed
 */
LCDButton::button LCDButton::check_LCD_push() {
  // Get the currently pressed button
  button reading = read_LCD_buttons();

  // Checks if the buttons has changed the (recorded) last state
  if (reading != _lastState) {
    _lastDebounceTime = millis();
  }
  // Checks if the buttons hasn't changed state for '_debounceDelay' milliseconds.
  if ((millis() - _lastDebounceTime) > _debounceDelay) {
    // Checks if the buttons has changed the (returned) last state
    if (reading != _state) {
      _state = reading;
      return _state;
    }
  }
  _lastState = reading;

  // No change to report
  return btnNONE;
}
