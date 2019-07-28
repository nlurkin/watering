/*
 * main.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include <Battery.h>

Battery batt = Battery(6200, 9000, A14);

void setup() {
	Serial.begin(115200);
	while (!Serial);
	batt.begin(5000, 2.0);
}

void loop() {
	digitalWrite(30, HIGH);
	delay(500);
	Serial.print("Battery voltage is ");
	Serial.print(batt.voltage());
	Serial.print(" (");
	Serial.print(batt.level());
	Serial.println("%)");
	digitalWrite(30, LOW);
	delay(500);
}

