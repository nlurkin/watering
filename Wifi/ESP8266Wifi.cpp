/*
 * ESP8266Wifi.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ESP8266Wifi.h"

ESP8266Wifi::ESP8266Wifi(Stream* serial) :
	_conn_opened({false,false,false,false,false}),
	_ip_address({0,0,0,0}),
	_mac_address({0,0,0,0,0,0}),
	_payload({Buffer(PAYLOAD_SIZE), Buffer(PAYLOAD_SIZE), Buffer(PAYLOAD_SIZE), Buffer(PAYLOAD_SIZE), Buffer(PAYLOAD_SIZE)}),
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

bool ESP8266Wifi::sendSomething(const char *cmd) const {
	if(cmd[0]=='&')
		return _client.sendData(cmd+1);
	else if(cmd[0]=='+' && cmd[1]=='+' && cmd[1]=='+')
		return Serial1.print("+++");
	else
		return _client.sendCommand(cmd);
}

bool ESP8266Wifi::sendCommand(const char *cmd) const {
	return _client.sendCommand(cmd);
}

bool ESP8266Wifi::sendData(const char *data) const {
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
			else if(endsWith(response, F("CONNECT")))
				new_connection(response);
			len = _client.readUntil(response, 200, '\n');
		}

		_logSerial->println();
		_logSerial->println(F("============"));
		_logSerial->println();
	}
	return has_response;
}

bool ESP8266Wifi::checkBoardConnection() const {
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

bool ESP8266Wifi::startServer(int port) const {
	bool success = _client.CIPMUX(true);

	if(success)
		success = _client.CIPSERVER(true, port);

	return success;
}

bool ESP8266Wifi::stopServer() const {
	return _client.CIPSERVER(false);
}

bool ESP8266Wifi::connectWifi(const char *ssid, const char *passwd) const {
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
		if(strstr_P(ptr, PSTR("WIFI DISCONNECT"))==ptr){ // Found it
			memset(_ip_address, 0, 4 * sizeof(uint8_t));
			memset(_mac_address, 0, 6 * sizeof(uint8_t));
			return true;
		}
	}

	return false;
}

bool ESP8266Wifi::restartBoard() const {
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

int ESP8266Wifi::openConnection(const char *address, uint16_t port) const {
	if(_client.CIPSTART(ATClient::TCP, address, port, 4))
		return 4;
	return -1;
}

uint8_t ESP8266Wifi::new_connection(const char *data) {
	uint8_t conn_number = strtoul(data, nullptr, 10);
	if(conn_number>4)
		return 99;
	_conn_opened[conn_number] = true;
	_payload[conn_number] = "";
	return conn_number;
}

int8_t ESP8266Wifi::payloadAvailable() const {
	for(int8_t i=4; i>=0; i--)
		if(_payload[i].len()>0) return i;
	return -1;
}

size_t ESP8266Wifi::getPayload(char *buff, uint8_t conn_number, size_t max) {
	if(conn_number>4)
		return "";
	return _payload[conn_number].get(buff, max);
}

void ESP8266Wifi::read_payload(const char *initdata) {
	uint8_t conn_number = strtoul(initdata+5, nullptr, 10);
	if(conn_number>4)
		return;
	size_t datas = strtol(initdata+7, nullptr, 10);
	if(datas>1024) // Only accepting up to 1024 bytes packets
		return;
	char *data_start = strchr(initdata+7, ':');
	if(data_start==nullptr)
		return;

	_payload[conn_number].push(data_start+1);
	char buff[1024];
	_client.readRaw(buff, datas);
	_payload[conn_number].push(buff);
}

bool ESP8266Wifi::sendPacket(const char *data, uint8_t conn) const {
	return _client.CIPSEND(data, conn);
}

bool ESP8266Wifi::closeConnection(uint8_t conn) const {
	return _client.CIPCLOSE(conn);
}

bool ESP8266Wifi::startsWith(const char *str, const char *search) {
	while( (*str!='\0') && (*search!='\0') ){
		if(*(str++)!=*(search++)) // Not the same char -> we are done
			return false;
	}
	return *search=='\0'; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::startsWith(const char *str, const __FlashStringHelper* search) {
	PGM_P p_search = reinterpret_cast<PGM_P>(search);
	unsigned char c;
	c = pgm_read_byte(p_search++);
	while( (*str!='\0') && (c!='\0') ){
		if(*(str++)!=c) // Not the same char -> we are done
			return false;
		c = pgm_read_byte(p_search++);
	}
	return c=='\0'; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::endsWith(const char *str, const char *search) {
	char *str_end = str + strlen(str);
	char *search_end = search + strlen(search);
	while( (str_end!=str) && (search_end!=search) ){
		if(*(str_end--)!=*(search_end--)) // Not the same char -> we are done
			return false;
	}
	return search==search_end; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::endsWith(const char *str, const __FlashStringHelper* search) {
	char *str_end = str + strlen(str);
	PGM_P p_search = reinterpret_cast<PGM_P>(search);
	PGM_P p_search_end = p_search + strlen_P(p_search);
	unsigned char c;
	c = pgm_read_byte(p_search_end--);
	while( (str_end!=str) && (p_search_end!=p_search) ){
		if(*(str_end--)!=c) // Not the same char -> we are done
			return false;
		c = pgm_read_byte(p_search_end--);
	}
	return c=='\0'; // The loop went through the whole string, finding each character equal
}

