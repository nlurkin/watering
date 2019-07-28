/*
 * main.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include "ESP8266Wifi.h"

ESP8266Wifi wifi;

void setup() {
	Serial.begin(115200);
	Serial1.begin(115200);

	while(!wifi.checkBoardConnection()) delay(1000);
	Serial.println("Connection established");

	wifi.restartBoard();
	delay(5000);

	while(!wifi.checkWifiConnection()){
		wifi.connectWifi("WIFI_SSID", "WIFI_PASSWD");
		delay(10000);
	}
	Serial.println("Connected to wifi");
}

void loop() {
  if (Serial.available() > 0)
  {
	String command = Serial.readStringUntil('\n');
	wifi.sendSomething(command);
  }

  wifi.readAndPrint();
}
