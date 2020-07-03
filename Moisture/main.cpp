/*
 * main.cpp
 *
 *  Created on: 2 Jul 2017
 *      Author: Nicolas Lurkin
 */

#include <Arduino.h>
#include "AutomaticWater.h"
#include "ESP8266Wifi.h"
#include "NetworkStream.h"
#include "RemoteControl.h"

//TODO add detection of sensor failure and permanently disable pump
//TODO add detection of empty tank and permanently disable pump

AutomaticWater waterSystem(22);
ESP8266Wifi wifi;
NetworkStream mySerial(wifi);
RemoteControl pubServer(wifi);

const char ssid[] = {""};
const char pwd[]  = {""};

unsigned long last_millis;

void setup(){
	Serial.begin(115200);
	Serial1.begin(115200);

	Serial.println(F("----- Arduino WIFI -----"));
	Serial.println(F("Checking ESP8266 connection..."));
	while (!wifi.checkBoardConnection())
		delay(100);
	Serial.println(F("Connection established"));

	wifi.restartBoard();
	delay(1000);

	uint8_t trials = 0;
	while (!wifi.checkWifiConnection() && trials < 10) {
		++trials;
		wifi.connectWifi(ssid, pwd);
		delay(1000);
	}
	Serial.println(F("Connected to wifi"));
	mySerial.setDestination("192.168.1.20", 8000);
	mySerial.begin(80);
	pubServer.setDestination("192.168.1.20", 8000);

	// Set tick at 1s - used for pump and sensor, not for display and buttons
	waterSystem.setTickInterval(1000);
	waterSystem.addCircuit(10, 50, 40);
	waterSystem.addCircuit(11, 51, 41);
	waterSystem.initSystem();
	waterSystem.setPublicationServer(&pubServer);

	last_millis = millis();
}

void loop(){
	if(!wifi.isConnected()){
		//Seems we lost the wifi... try to reconnect
		wifi.connectWifi(ssid, pwd);
		wifi.checkWifiConnection();
	}

	if (Serial.available() > 0) {
		String command = Serial.readStringUntil('\n');
		wifi.sendSomething(command.c_str());
	}
	else if (mySerial.available() > 0) {
		String command = mySerial.readStringUntil('\n');
		wifi.sendSomething(command.c_str());
	}

	wifi.readAndPrint();

	// But run at 0.1s
	// This ensures enough reactivity when buttons are pressed
	if(millis()-last_millis>100){
		waterSystem.tick();
		last_millis = millis();
		//delay(100);
	}
	pubServer.serve();

}

