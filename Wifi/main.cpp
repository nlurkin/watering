/*
 * main.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include "ESP8266Wifi.h"
#include "HTTPRequest.h"

ESP8266Wifi wifi;

void setup() {
	Serial.begin(115200);
	Serial1.begin(115200);

	Serial.println("----- Arduino WIFI -----");
	Serial.println("Checking ESP8266 connection...");
	while (!wifi.checkBoardConnection())
		delay(100);
	Serial.println("Connection established");

	wifi.restartBoard();
	delay(3000);

	while (!wifi.checkWifiConnection()) {
		wifi.connectWifi("WIFI_SSID", "WIFI_PASSWD");
		delay(10000);
	}
	Serial.println("Connected to wifi");
	if (wifi.startServer(80))
		Serial.println("Server started on port 80");
}

void loop() {
	if (Serial.available() > 0) {
		String command = Serial.readStringUntil('\n');
		wifi.sendSomething(command);
	}

	wifi.readAndPrint();

	int8_t conn = wifi.payloadAvailable();
	if (conn != -1) {
		HTTPRequest http(wifi.getPayload(conn));
		Serial.println("Got http payload");
		http.print();
		if(http.needs_answer()){
			wifi.sendPacket("HTTP/1.1 200 OK\r\nContent-Type: application/text\r\nContent-Length: 10\r\n\r\n", conn);
			wifi.sendPacket("Hey you!\r\n", conn);
			wifi.closeConnection(conn);
		}
	}
}
