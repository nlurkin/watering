/*
 * ESP8266Wifi.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ESP8266Wifi.h"

ESP8266Wifi::ESP8266Wifi(Stream* serial) :
	_has_payload({false,false,false,false,false}),
	_ip_address({0,0,0,0}),
	_mac_address({0,0,0,0,0,0}),
	_client(serial),
	_logSerial(&Serial)
{
}

ESP8266Wifi::~ESP8266Wifi() {
}

void ESP8266Wifi::setLogSerial(Stream *serial){
	_logSerial = serial;
	_client.setLogSerial(_logSerial);
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
		_logSerial->println(F("Response Received:"));
		while (response.length() > 0) {
			_logSerial->println("> " + response);
			if(response[0]=='+' && response[1]=='I'){ //+IPD
				read_payload_raw(response);
				break;
			}
			//else if(response.substring(2)=="CONNECT")
			//	new_connection(response);
			response = _client.read();
		}

		_logSerial->println();
		_logSerial->println(F("============"));
		_logSerial->println();
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

	_logSerial->println(_client.getLastDataSize());
	if(_client.getLastDataSize()<2) // Expecting 2 data lines + eventual empty lines
		return false;
	_logSerial->println(data[0]);
	_logSerial->println(data[1]);
	if(data[0].indexOf(F("+CIFSR:STAIP"))!=0) //First one is expected to be IP
		return false;
	if(data[1].indexOf(F("+CIFSR:STAMAC"))!=0) //Second one is expected to be MAC
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
		if(data[0]!=F("WIFI CONNECTED"))
			return false;
		if(data[1]!=F("WIFI GOT IP"))
			return false;
	}
	else{
		if(data[0]!=F("WIFI DISCONNECT"))
			return false;
		if(data[1]!=F("WIFI CONNECTED"))
			return false;
		if(data[2]!=F("WIFI GOT IP"))
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

	if(_client.getLastData()[0]!=F("WIFI DISCONNECT"))
		return false;

	return true;
}

bool ESP8266Wifi::restartBoard() {
	if(!_client.RST())
		return false;
	for(uint8_t i=0; i<_client.getLastDataSize(); ++i){
		_logSerial->println(_client.getLastData()[i]);
	}
	return true;
}

int ESP8266Wifi::openConnection(String address, uint16_t port) {
	if(_client.CIPSTART(ATClient::TCP, address, port, 4))
		return 4;
	return -1;
}

uint8_t ESP8266Wifi::new_connection(String data) {
	//_logSerial->println("New connection" + data.substring(0,1));
	uint8_t conn_number = data.substring(0,1).toInt();
	//_logSerial->println("New connection" + String(conn_number));
	if(conn_number>4)
		return 99;
	_has_payload[conn_number] = true;
	_payload[conn_number] = "";
	return conn_number;
}

int8_t ESP8266Wifi::payloadAvailable() {
	for(int8_t i=4; i>=0; i--)
		if(_has_payload[i]) return i;
	return -1;
}

String ESP8266Wifi::getPayload(uint8_t conn_number) {
	if(conn_number>4)
		return "";
	_has_payload[conn_number] = false;
	String val = _payload[conn_number];
	_payload[conn_number] = "";
	return val;
}

void ESP8266Wifi::read_payload(String initdata) {
	const int MAX_LINES = 20;
	static String buff[20];
	uint8_t curr=1;
	String response = _client.read();
	while (response.length() > 0) {
		_logSerial->println(response);
		if(curr<MAX_LINES)
			buff[curr++] = response;
		else
			_logSerial->println(F("Buffer overflow"));
		response = _client.read();
	}

	buff[0] = initdata.substring(initdata.indexOf(':')+1);
	uint8_t conn_number = initdata.substring(5,6).toInt();
	if(conn_number>4)
		return;

	_has_payload[conn_number] = true;
	for(uint8_t i=0; i<curr; ++i){
		//_logSerial->println("Adding to payload: " + buff[i]);
		_payload[conn_number] += buff[i];
	}
}

bool ESP8266Wifi::sendPacket(String data, uint8_t conn) {
	return _client.CIPSEND(data, conn);
}

bool ESP8266Wifi::closeConnection(uint8_t conn) {
	return _client.CIPCLOSE(conn);
}

void ESP8266Wifi::read_payload_raw(String initdata) {
	String response = _client.readRaw();

	uint8_t conn_number = initdata.substring(5,6).toInt();
	if(conn_number>4)
		return;

	_has_payload[conn_number] = true;
	_payload[conn_number] += initdata.substring(initdata.indexOf(':')+1) + '\n';
	_payload[conn_number] += response;
}
