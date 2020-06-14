/*
 * ESP8266Wifi.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ESP8266Wifi.h"

static const char g_SEP_NEWLINE[] PROGMEM = {"\n"};
static const char g_SEP_DOT[]     PROGMEM = {"."};
static const char g_SEP_COLUMN[]  PROGMEM = {":"};

Buffer ESP8266Wifi::_persistent_buffer(ESP8266Wifi::PAYLOAD_SIZE);

ESP8266Wifi::ESP8266Wifi(Stream* serial) :
	_conn_opened({false,false,false,false,false}),
	_ip_address({0,0,0,0}),
	_mac_address({0,0,0,0,0,0}),
	_payload({&_persistent_buffer, nullptr, nullptr, nullptr, nullptr}),
	_client(serial),
	_logSerial(&Serial)
{
}

ESP8266Wifi::~ESP8266Wifi() {
	for(uint8_t i=1; i<4; i++){
		if(_payload[i])
			delete _payload[i];
		_payload[i] = nullptr;
	}
}

void ESP8266Wifi::setLogSerial(Stream *serial){
	_logSerial = serial;
	//_client.setLogSerial(_logSerial);
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
	static constexpr size_t read_size = 100;
	char response[read_size];
	delay(10); // Give time to actually fill the buffer. Else we will most likely have only 1 char
	size_t len = _client.readUntil(response, read_size, '\n');
	bool has_response = len > 0;

	if (has_response) {
		_logSerial->println(F("Response Received:"));
		while (len > 0) {
			_logSerial->print(F("> "));
			_logSerial->println(response);
			if(startsWith(response, F("+IPD"))){ //+IPD
				read_payload(response);
				break;
			}
			else if(endsWith(response, F("CONNECT\r")))
				new_connection(response);
			else if(endsWith(response, F("CLOSED\r")))
				new_connection(response);
			else if(endsWith(response, F("WIFI DISCONNECT\r")))
				disconnect();
			len = _client.readUntil(response, read_size, '\n');
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

	char data[ATClient::DATA_BUFFER_SIZE];
	char ip_line[16] = {'\0'}; //3*4+3+(1 null)
	char mac_line[18] = {'\0'}; //6*2+5+(1 null)
	char *ptr;
	_client.getLastData(data, ATClient::DATA_BUFFER_SIZE);

	ptr = strtok_P(data, g_SEP_NEWLINE);
	while(ptr!=nullptr){
		if(!ip_line[0] && strstr_P(ptr, PSTR("+CIFSR:STAIP"))==ptr) { // This is the ip address line (and we have not got it yet)
			strncpy(ip_line, ptr+14, 15); //Copy the ip address
			ip_line[15] = '\0';
		}
		if(!mac_line[0] && strstr_P(ptr, PSTR("+CIFSR:STAMAC"))==ptr) { // This is the mac address line (and we have not got it yet)
			strncpy(mac_line, ptr+15, 17); //Copy the mac address
			mac_line[17] = '\0';
		}
		ptr = strtok_P(nullptr, g_SEP_NEWLINE);
	}

	// Extract 4 numbers of the IP address (separated by .)
	ptr = strtok_P(ip_line, g_SEP_DOT);
	for(int i=0; i<4 && ptr!=nullptr; ++i){
		_ip_address[i] = strtol(ptr, nullptr, 10);
		ptr = strtok_P(nullptr, g_SEP_DOT);
	}

	// Extract 6 numbers of the MAC address (separated by :)
	ptr = strtok_P(mac_line, g_SEP_COLUMN);
	for(int i=0; i<6 && ptr!=nullptr; ++i){
		_mac_address[i] = strtol(ptr, nullptr, 16);
		ptr = strtok_P(nullptr, g_SEP_COLUMN);
	}
	return isConnected();
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

	char data[ATClient::DATA_BUFFER_SIZE];
	char *ptr;
	_client.getLastData(data, ATClient::DATA_BUFFER_SIZE);

	bool got_connect = false;
	bool got_ip = false;

	ptr = strtok_P(data, g_SEP_NEWLINE);
	while(ptr!=nullptr){
		if(!got_connect && !got_ip && strstr_P(ptr, PSTR("WIFI DISCONNECT"))==ptr) // If we have this, it must be first
			return false;
		if(!got_ip && strstr_P(ptr, PSTR("WIFI CONNECTED"))==ptr) // We must have this before the IP
			got_connect = true;
		if(got_connect && strstr_P(ptr, PSTR("WIFI GOT IP"))==ptr) // We must have this right after the connect
			got_ip = true;
		ptr = strtok_P(nullptr, g_SEP_NEWLINE);
	}

	return got_connect && got_ip;
}

bool ESP8266Wifi::disConnectWifi() {
	bool success = _client.CWQAP();

	if(!success)
		return false;

	char data[ATClient::DATA_BUFFER_SIZE];
	char *ptr;
	_client.getLastData(data, ATClient::DATA_BUFFER_SIZE);

	bool got_disconnect = false;

	ptr = strtok_P(data, g_SEP_NEWLINE);
	while(ptr!=nullptr){
		if(strstr_P(ptr, PSTR("WIFI DISCONNECT"))==ptr){ // Found it
			disconnect();
			return true;
		}
		ptr = strtok_P(nullptr, g_SEP_NEWLINE);
	}

	return false;
}

bool ESP8266Wifi::isConnected() const {
	return _ip_address[0]!=0 || _ip_address[1]!=0  || _ip_address[2]!=0 || _ip_address[3]!=0; //Success if we have a non-zero ip address
}

bool ESP8266Wifi::restartBoard() const {
	if(!_client.RST())
		return false;

	char data[ATClient::DATA_BUFFER_SIZE];
	char *ptr;
	while(_client.dataAvailable()>0){
		_client.getLastData(data, ATClient::DATA_BUFFER_SIZE);
		_logSerial->print(data);
	}

	return true;
}

int ESP8266Wifi::openConnection(const char *address, uint16_t port) const {
	if(_client.CIPSTART(ATClient::TCP, address, port, 4))
		return 4;
	return -1;
}

int ESP8266Wifi::openConnection(uint8_t ip[4], uint16_t port) const {
	if(_client.CIPSTART(ATClient::TCP, ip, port, 4))
		return 4;
	return -1;
}

uint8_t ESP8266Wifi::new_connection(const char *data) {
	uint8_t conn_number = strtoul(data, nullptr, 10);
	if(conn_number>4)
		return 99;
	_conn_opened[conn_number] = true;
	if(_payload[conn_number]) //If the payload already exists, clear it
		_payload[conn_number]->clear();
	else // Else create it
		_payload[conn_number] = new Buffer(PAYLOAD_SIZE);

	return conn_number;
}

uint8_t ESP8266Wifi::end_connection(const char *data) {
	uint8_t conn_number = strtoul(data, nullptr, 10);
	if(conn_number>4)
		return 99;
	_conn_opened[conn_number] = false;
	if(conn_number==0) // _payload for connection 0 cannot be deleted
		_payload[conn_number]->clear();
	else{ // Delete of not needed
		delete _payload[conn_number];
		_payload[conn_number] = nullptr;
	}

	return conn_number;
}

void ESP8266Wifi::disconnect() {
	memset(_ip_address, 0, 4 * sizeof(uint8_t));
	memset(_mac_address, 0, 6 * sizeof(uint8_t));
}

int8_t ESP8266Wifi::payloadAvailable() const {
	for(int8_t i=4; i>=0; i--)
		// Not available if payload not defined or empty
		if(_payload[i] && _payload[i]->len()>0) return i;
	return -1;
}

size_t ESP8266Wifi::getPayload(char *buff, uint8_t conn_number, size_t max) {
	// Do not try to get payload for undefined payloads
	if(conn_number>4 || !_payload[conn_number])
		return 0;
	return _payload[conn_number]->get(buff, max);
}

void ESP8266Wifi::read_payload(const char *initdata) {
	uint8_t conn_number = strtoul(initdata+5, nullptr, 10);
	// Do not try to read payload for undefined payloads
	if(conn_number>4 || !_payload[conn_number])
		return;
	size_t datas = strtol(initdata+7, nullptr, 10);
	if(datas>PAYLOAD_SIZE) // This is the maximum size of the packet that we are accepting
		return;
	char *data_start = strchr(initdata+7, ':');
	if(data_start==nullptr)
		return;

	size_t init_len = _payload[conn_number]->push(data_start+1);
	_payload[conn_number]->push('\n');
	char buff[PAYLOAD_SIZE];
	_client.readRaw(buff, datas-init_len-1);
	_payload[conn_number]->push(buff);
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
	char *str_end = str + strlen(str)-1;
	char *search_end = search + strlen(search)-1;
	while( (str_end!=str) && (search_end+1!=search) ){
		if(*(str_end--)!=*(search_end--)) // Not the same char -> we are done
			return false;
	}
	return search==search_end+1; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::endsWith(const char *str, const __FlashStringHelper* search) {
	char *str_end = str + strlen(str)-1;
	PGM_P p_search = reinterpret_cast<PGM_P>(search);
	PGM_P p_search_end = p_search + strlen_P(p_search)-1;
	unsigned char c;
	while( (str_end!=str) && (p_search_end+1!=p_search) ){
		c = pgm_read_byte(p_search_end--);
		if(*(str_end--)!=c) // Not the same char -> we are done
			return false;
	}
	return p_search==p_search_end+1; // The loop went through the whole string, finding each character equal
}
