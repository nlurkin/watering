/*
 * ESP8266Wifi.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ESP8266Wifi.h"

ESP8266Wifi::ESP8266Wifi() :
	_ip_address({0,0,0,0}),
	_mac_address({0,0,0,0,0,0})
{
}

ESP8266Wifi::~ESP8266Wifi() {
}

bool ESP8266Wifi::sendSomething(String cmd) {
	if(cmd[0]=='&')
		return _client.sendData(cmd.substring(1));
	else if(cmd.substring(0,3)=="+++")
		return Serial1.print("+++");
	else
		return _client.sendCommand(cmd);
}

bool ESP8266Wifi::sendCommand(String cmd) {
	return _client.sendCommand(cmd);
}

bool ESP8266Wifi::sendData(String data) {
	return _client.sendData(data);
}

bool ESP8266Wifi::readAndPrint() {
	String response = _client.read();
	bool has_response = response.length() > 0;

	if (has_response) {
		Serial.println("Response Received:");
		while (response.length() > 0) {
			Serial.println(response);
			response = _client.read();
		}

		Serial.println();
		Serial.println("============");
		Serial.println();
	}
	return has_response;
}

bool ESP8266Wifi::checkBoardConnection() {
	return _client.AT();
}

bool ESP8266Wifi::checkWifiConnection() {
	bool success = _client.CIFSR();

	if(!success)
		return false;

	String *data = _client.getLastData();

	Serial.println(_client.getLastDataSize());
	if(_client.getLastDataSize()!=2) // Expecting 2 data lines
		return false;
	Serial.println(data[0]);
	Serial.println(data[1]);
	if(data[0].indexOf("+CIFSR:STAIP")!=0) //First one is expected to be IP
		return false;
	if(data[1].indexOf("+CIFSR:STAMAC")!=0) //Second one is expected to be MAC
		return false;

	String substr = data[0].substring(14, data[0].length()-1); //Remove header and last "

	// Extract 4 numbers of the IP address (separated by .)
	int last_index = 0;
	for(int i=0; i<4; ++i){
		int dotpos = substr.indexOf('.', last_index); //Find next dot
		String add = substr.substring(last_index, dotpos); //Extract number
		_ip_address[i] = add.toInt(); //Save it
		last_index = dotpos+1;
	}

	substr = data[1].substring(15, data[1].length()-1); //Remove header and last "

	// Extract 6 numbers of the MAC address (separated by :)
	last_index = 0;
	for(int i=0; i<6; ++i){
		int dotpos = substr.indexOf(':', last_index); //Find next :
		String add = substr.substring(last_index, dotpos); //Extract number
		_mac_address[i] = strtol(add.c_str(), nullptr, 16); //Save it (from hex)
		last_index = dotpos+1;
	}
	return _ip_address[0]!=0 || _ip_address[1]!=0  || _ip_address[2]!=0 || _ip_address[3]!=0; //Success if we have a non-zero ip address
}

bool ESP8266Wifi::startServer(int port) {
	bool success = _client.CIPMUX(true);

	if(success)
		success = _client.CIPSERVER(true, port);

	return success;
}

bool ESP8266Wifi::stopServer() {
	return _client.CIPSERVER(false);
}

bool ESP8266Wifi::connectWifi(String ssid, String passwd) {
	bool success = _client.CWJAP(ssid, passwd);

	if(!success)
		return false;

	String *data = _client.getLastData();
	uint8_t datas = _client.getLastDataSize();

	if(datas!=2 || datas!=3) //Expecting 2 or 3 data lines
		return false;

	if(datas==4){
		if(data[0]!="WIFI CONNECTED")
			return false;
		if(data[1]!="WIFI GOT IP")
			return false;
	}
	else{
		if(data[0]!="WIFI DISCONNECT")
			return false;
		if(data[1]!="WIFI CONNECTED")
			return false;
		if(data[2]!="WIFI GOT IP")
			return false;
	}
	return true;
}

bool ESP8266Wifi::disConnectWifi() {
	bool success = _client.CWQAP();

	if(!success)
		return false;

	if(_client.getLastDataSize()!=1) // Expecting 1 data line
		return false;

	if(_client.getLastData()[0]!="WIFI DISCONNECT")
		return false;

	return true;
}

bool ESP8266Wifi::restartBoard() {
	if(!_client.RST())
		return false;
	for(uint8_t i=0; i<_client.getLastDataSize(); ++i){
		Serial.println(_client.getLastData()[i]);
	}
	return true;
}
