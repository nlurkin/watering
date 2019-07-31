/*
 * main.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include "ESP8266Wifi.h"
#include "HTTPServer.h"

ESP8266Wifi wifi;
HTTPServer server(wifi);

char ssid[] = "";
char pwd[]  = "";

void setup() {
	Serial.begin(115200);
	Serial1.begin(115200);

	Serial.println(F("----- Arduino WIFI -----"));
	Serial.println(F("Checking ESP8266 connection..."));
	while (!wifi.checkBoardConnection())
		delay(100);
	Serial.println(F("Connection established"));

	wifi.restartBoard();
	delay(3000);

	uint8_t trials = 0;
	while (!wifi.checkWifiConnection() && trials<10) {
		++trials;
		wifi.connectWifi(ssid, pwd);
		delay(10000);
	}
	Serial.println(F("Connected to wifi"));
	if (server.startServer(80))
		Serial.println(F("Server started on port 80"));
}

void loop() {
	if (Serial.available() > 0) {
		String command = Serial.readStringUntil('\n');
		if(command.startsWith("DATA"))
			server.sendData("192.168.0.20", 80);
		else
			wifi.sendSomething(command);
	}

	wifi.readAndPrint();
	server.loop();
}
