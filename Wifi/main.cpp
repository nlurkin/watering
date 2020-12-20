/*
 * main.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#define USE_NETWORK_STREAM 1 //Comment this to use the standard Serial port

#include <Arduino.h>
#include <HardwareSerial.h>
#include "LibESP8266/ESP8266Wifi.h"
#include "LibNetwork/NetworkStream.h"
#include "LibPublications/PubServer.h"
#include "LibPublications/Publication.h"

ESP8266Wifi wifi;

#ifdef USE_NETWORK_STREAM
NetworkStream mySerial(wifi);
Stream &logSerial = mySerial;
ControlServer myPubServer(wifi);
Publication<double> pub_temperature("S1_T");
Publication<int> pub_temperature1("S2_T");
#else
Stream &logSerial = Serial;
#endif

const char ssid[] PROGMEM = {""};
const char pwd[]  PROGMEM = {""};

unsigned long last_update;

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
		delay(1000);
	}
	Serial.println(F("Connected to wifi"));
	#ifdef USE_NETWORK_STREAM
	mySerial.setDestination("192.168.1.20", 8000);
	mySerial.begin(80);
	#endif

	myPubServer.setDestination("192.168.1.20", 8000);
	myPubServer.addPublication(&pub_temperature);
	myPubServer.addPublication(&pub_temperature1);

	last_update = 0;
}

void loop() {
	if(!wifi.isConnected()){
		//Seems we lost the wifi... try to reconnect
		wifi.connectWifi(ssid, pwd);
		wifi.checkWifiConnection();
		return;
	}

	//We will reach here only if we have wifi
	if (Serial.available() > 0) {
		String command = Serial.readStringUntil('\n');
		wifi.sendSomething(command.c_str());
	}
	else if (logSerial.available() > 0) {
		String command = logSerial.readStringUntil('\n');
		if(command.compareTo("report")){
			mySerial.println("This is a report");
		}
		else
			wifi.sendSomething(command.c_str());
	}

	if(millis()-last_update>5000){
		pub_temperature.updateValue(5.2);
		pub_temperature1.updateValue(10);
		last_update = millis();
	}
	myPubServer.serve();
	wifi.readAndPrint();
}
