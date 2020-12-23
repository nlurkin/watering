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

bool ESP8266Wifi::sendSomething(const char *cmd) {
	if(cmd[0]=='&')
		return _client.sendData(cmd+1);
	else if(cmd[0]=='+' && cmd[1]=='+' && cmd[1]=='+')
		return Serial1.print("+++");
	else
		return _client.sendCommand(cmd);
}

bool ESP8266Wifi::sendCommand(const char *cmd) {
	return _client.sendCommand(cmd);
}

bool ESP8266Wifi::sendData(const char *data) {
	return _client.sendData(data);
}

bool ESP8266Wifi::readAndPrint() {
	char response[200];
	size_t len = _client.readUntil(response, 200, '\n');
	bool has_response = len > 0;

	if (has_response) {
		_logSerial->println(F("Response Received:"));
		while (len > 0) {
			_logSerial->print("> ");
			_logSerial->println(response);
			if(startsWith(response, F("+IPD"))){ //+IPD
				read_payload(response);
				break;
			}
			//else if(response.substring(2)=="CONNECT")
			//	new_connection(response);
			len = _client.readUntil(response, 200, '\n');
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

	char data[200];
	char ip_line[18] = {'\0'};
	char mac_line[20] = {'\0'};
	char *ptr;
	_client.getLastData(data, 200);

	ptr = strtok(data, "\n");
	while(ptr!=nullptr){
		if(!ip_line[0] && strstr_P(ptr, PSTR("+CIFSR:STAIP"))==ptr) { // This is the ip address line (and we have not got it yet)
			strncpy(ip_line, ptr+14, 17); //Copy the ip address
			ip_line[17] = '\0';
		}
		if(!mac_line[0] && strstr_P(ptr, PSTR("+CIFSR:STAMAC"))==ptr) { // This is the mac address line (and we have not got it yet)
			strncpy(mac_line, ptr+15, 17); //Copy the mac address
			mac_line[17] = '\0';
		}
	}

	// Extract 4 numbers of the IP address (separated by .)
	ptr = strtok(ip_line, ".");
	for(int i=0; i<4 && ptr!=nullptr; ++i){
		_ip_address[i] = strtol(ptr, nullptr, 10);
		ptr = strtok(nullptr, ".");
	}

	// Extract 6 numbers of the MAC address (separated by :)
	ptr = strtok(ip_line, ":");
	for(int i=0; i<6 && ptr!=nullptr; ++i){
		_mac_address[i] = strtol(ptr, nullptr, 16);
		ptr = strtok(nullptr, ":");
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

bool ESP8266Wifi::connectWifi(const char *ssid, const char *passwd) {
	bool success = _client.CWJAP(ssid, passwd);

	if(!success)
		return false;

	char data[200];
	char *ptr;
	_client.getLastData(data, 200);

	bool got_connect = false;
	bool got_ip = false;


	ptr = strtok(data, "\n");
	while(ptr!=nullptr){
		if(!got_connect && !got_ip && strstr_P(ptr, PSTR("WIFI DISCONNECT"))==ptr) // If we have this, it must be first
			return false;
		if(!got_ip && strstr_P(ptr, PSTR("WIFI CONNECTED"))==ptr) // We must have this before the IP
			got_connect = true;
		if(got_connect && strstr_P(ptr, PSTR("WIFI GOT IP"))==ptr) // We must have this right after the connect
			got_ip = true;
	}

	return got_connect && got_ip;
}

bool ESP8266Wifi::disConnectWifi() {
	bool success = _client.CWQAP();

	if(!success)
		return false;

	char data[200];
	char *ptr;
	_client.getLastData(data, 200);

	bool got_disconnect = false;

	ptr = strtok(data, "\n");
	while(ptr!=nullptr){
		if(strstr_P(ptr, PSTR("WIFI DISCONNECT"))==ptr) // Found it
			return true;
	}

	return false;
}

bool ESP8266Wifi::restartBoard() {
	if(!_client.RST())
		return false;

	char data[200];
	char *ptr;
	while(_client.dataAvailable()>0){
		_client.getLastData(data, 200);
		_logSerial->print(data);
	}

	return true;
}

int ESP8266Wifi::openConnection(String address, uint16_t port) {
	if(_client.CIPSTART(ATClient::TCP, address.c_str(), port, 4))
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
	String response = _client.readUntil();
	while (response.length() > 0) {
		_logSerial->println(response);
		if(curr<MAX_LINES)
			buff[curr++] = response;
		else
			_logSerial->println(F("Buffer overflow"));
		response = _client.readUntil();
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
	return _client.CIPSEND(data.c_str(), conn);
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
