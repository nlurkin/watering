/*
 * main.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include <Adafruit_BME280.h>

#include "LibLCD/LCDDisplay.h"
#include "MenuWelcome.h"
#include "MenuBME.h"

Adafruit_BME280 bme;
LCDDisplay lcd(100);

double temperature = 0;
double pressure = 0;
double humidity= 0;
double altitude= 0;

MenuWelcome _m_welcome(lcd.get_lcd_handle(), "My Project");
MenuBME     _m_bme(lcd.get_lcd_handle());

void setup() {
	Serial.begin(115200);

	Serial.println(F("BME280 test"));
	if (!bme.begin(0x76)) {
		Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
		while (1);
	}

	lcd.add_menu(&_m_welcome);
	lcd.add_menu(&_m_bme);
}

int interval = 0;

void loop() {

	lcd.tick();
	if(interval % 20 == 0){
		temperature = bme.readTemperature();
		pressure = bme.readPressure()/100;
		humidity = bme.readHumidity();
		altitude = bme.readAltitude(1019.66);

		Serial.print(F("Temperature = "));
		Serial.print(temperature);
		Serial.println(" *C");

		Serial.print(F("Pressure = "));
		Serial.print(pressure); //displaying the Pressure in hPa, you can change the unit
		Serial.println(" hPa");

		Serial.print(F("Humidity = "));
		Serial.print(humidity); //displaying the Pressure in hPa, you can change the unit
		Serial.println(" %");

		Serial.print(F("Approx altitude = "));
		Serial.print(altitude); //The "1019.66" is the pressure(hPa) at sea level in day in your region
		Serial.println(" m");                    //If you don't know it, modify it until you get your current altitude

		Serial.println();
		_m_bme.set_values(temperature, pressure, humidity, altitude);
		interval = 0;
	}
	++interval;
    delay(100);
}
