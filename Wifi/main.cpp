/*
 * main.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include <HardwareSerial.h>
#include "ESP8266Wifi.h"
#include "HTTPServer.h"
#include "NetworkStream.h"

ESP8266Wifi wifi;
#define USE_NETWORK_STREAM 1 //Comment this to use the standard Serial port

#ifdef USE_NETWORK_STREAM
NetworkStream mySerial(wifi);
Stream &logSerial = mySerial;
#else
Stream &logSerial = Serial;
#endif

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
	mySerial.setDestination("192.168.0.20", 8000);
	mySerial.begin(80);
	wifi.setLogSerial(&mySerial);
}

void loop() {
	if (Serial.available() > 0) {
		String command = Serial.readStringUntil('\n');
		wifi.sendSomething(command);
	}
	else if (logSerial.available() > 0) {
		String command = logSerial.readStringUntil('\n');
		wifi.sendSomething(command);
	}

	wifi.readAndPrint();
}
