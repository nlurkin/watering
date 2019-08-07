/*
 * ATClient.cpp
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ATClient.h"

ATClient::ATClient(Stream* serial) :
	_set_default(false),
	_waitingForAnswer(0),
	_timeout(2000),
	_lastDataSize(0),
	_atSerial(serial),
	_logSerial(&Serial)
{
}

ATClient::~ATClient() {
}

void ATClient::setLogSerial(Stream *serial){
	_logSerial = serial;
}

bool ATClient::sendCommand(String cmd) {
	_logSerial->print(F("Sending: "));
	_logSerial->println(cmd);
	_logSerial->println();
	_atSerial->println(cmd);
	return true;
}

bool ATClient::sendCommand(const __FlashStringHelper* cmd) {
	_logSerial->print(F("Sending: "));
	_logSerial->println(cmd);
	_logSerial->println();
	_atSerial->println(cmd);
	return true;
}

bool ATClient::sendData(const char *data) {
	_logSerial->print(F("Sending data: "));
	_logSerial->println(data);
	_logSerial->println();
	_atSerial->print(data);
	return true;
}

bool ATClient::sendDataConfirm(String data) {
	_logSerial->print(F("Sending data: "));
	_logSerial->println(data);
	_logSerial->println();
	_atSerial->println(data);
	return waitMessage(F("SEND OK"));
}

const String* ATClient::getLastData() {
	return _lastData;
}

uint8_t ATClient::getLastDataSize() {
	return _lastDataSize;
}

// ######################## General commands
bool ATClient::AT() {
	sendCommand(F("AT"));
	return checkAnswer(F("AT"));
}

bool ATClient::RST() {
	sendCommand(F("AT+RST"));
	bool success = checkAnswer(F("AT+RST"));
	if(!success)
		return false;

	return waitMessage(F("ready"));
}

bool ATClient::GMR() {
	sendCommand(F("AT+GMR"));
	return checkAnswer(F("AT+GMR"));
}


bool ATClient::SLEEP(uint8_t mode) {
	if(mode>2)
		return false;
	char cmd[30];
	sprintf_P(cmd, PSTR("AT+SLEEP=%u"), mode);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

// ######################## WIFI commands
bool ATClient::CWMODE(uint8_t mode) {
	if(mode>3 || mode<1)
		return false;

	char cmd[30];
	if(_set_default) // Store in flash
		sprintf_P(cmd, PSTR("AT+CWMODE_DEF=%u"), mode);
	else //Only temporary
		sprintf_P(cmd, PSTR("AT+CWMODE_CUR=%u"), mode);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CWJAP(String &ssid, String &passwd) {
	char cmd[50];
	if(_set_default) // Store in flash
		sprintf_P(cmd, PSTR("AT+CWJAP_DEF=\"%s\",\"%s\""), ssid, passwd);
	else //Only temporary
		sprintf_P(cmd, PSTR("AT+CWJAP_CUR=\"%s\",\"%s\""), ssid, passwd);

	sendCommand(cmd);
	unsigned long old_to = _timeout;
	_timeout = 5000;
	bool ans = checkAnswer(cmd);
	_timeout = old_to;
	return ans;
}

bool ATClient::CWLAP() {
	sendCommand(F("AT+CWLAP"));
	return checkAnswer(F("AT+CWLAP"));
}

bool ATClient::CWQAP() {
	sendCommand(F("AT+CWQAP"));
	return checkAnswer(F("AT+CWQAP"));
}

bool ATClient::CWSAP(String &ssid, String &passwd, uint8_t channel, uint8_t ecn) {
	if(ecn>4 || ecn==1) //Can be only 0,2,3,4
		return false;

	char cmd[100];
	if(_set_default) // Store in flash
		sprintf_P(cmd, PSTR("AT+CWSAP_DEF=\"%s\",\"%s\",%u,%u"), ssid, passwd, channel, ecn);
	else //Only temporary
		sprintf_P(cmd, PSTR("AT+CWSAP_CUR=\"%s\",\"%s\",%u,%u"), ssid, passwd, channel, ecn);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CWLIF() {
	sendCommand(F("AT+CWLIF"));
	return checkAnswer(F("AT+CWLIF"));
}

bool ATClient::CWDHCP(bool en, uint8_t mode) {
	if(mode>2)
		return false;

	char cmd[30];
	if(_set_default) // Store in flash
		sprintf_P(cmd, PSTR("AT+CWDHCP_DEF=%d,%u"), en, mode);
	else //Only temporary
		sprintf_P(cmd, PSTR("AT+CWDHCP_CUR=%d,%u"), en, mode);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CWAUTOCONN(bool en) {
	char cmd[30];
	sprintf_P(cmd, PSTR("AT+CWAUTOCONN=%d"), en);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPSTAMAC(uint8_t mac[6]) {
	char cmd[30];
	if(_set_default) // Store in flash
		sprintf_P(cmd, PSTR("AT+CIPSTAMAC_DEF=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	else //Only temporary
		sprintf_P(cmd, PSTR("AT+CIPSTAMAC_CUR=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPAPMAC(uint8_t mac[6]) {
	char cmd[30];
	if(_set_default) // Store in flash
		sprintf_P(cmd, PSTR("AT+CIPAPMAC_DEF=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	else //Only temporary
		sprintf_P(cmd, PSTR("AT+CIPAPMAC_CUR=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPSTA(uint8_t ip[4], uint8_t gtw[4], uint8_t netmask[4]) {
	if(netmask!=nullptr && gtw==nullptr)
		return false; //if netmask is set, gtw must be set too

	char cmd[70];
	int len = 0;
	if(_set_default) // Store in flash
		len += sprintf_P(cmd, PSTR("AT+CIPSTA_DEF=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);
	else //Only temporary
		len += sprintf_P(cmd, PSTR("AT+CIPSTA_CUR=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);

	if(gtw!=nullptr)
		len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), gtw[0], gtw[1], gtw[2], gtw[3]);
	if(netmask!=nullptr)
		len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), netmask[0], netmask[1], netmask[2], netmask[3]);

	sendCommand(cmd);
	return checkAnswer(cmd);

}

bool ATClient::CIPAP(uint8_t ip[4], uint8_t gtw[4], uint8_t netmask[4]) {
	if( !(netmask==nullptr && gtw==nullptr) || !(netmask!=nullptr && gtw!=nullptr) )
		return false; //must both be set or nullptr

	char cmd[70];
	int len = 0;
	if(_set_default) // Store in flash
		len += sprintf_P(cmd, PSTR("AT+CIPAP_DEF=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);
	else //Only temporary
		len += sprintf_P(cmd, PSTR("AT+CIPAP_CUR=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);

	if(gtw!=nullptr)
		len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), gtw[0], gtw[1], gtw[2], gtw[3]);
	if(netmask!=nullptr)
		len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), netmask[0], netmask[1], netmask[2], netmask[3]);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::WPS(bool en){
	char cmd[15];
	sprintf_P(cmd, PSTR("AT+WPS=%d"), en);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

// ######################## TCP/IP commands
bool ATClient::CIPSTATUS() {
	sendCommand(F("AT+CIPSTATUS"));
	return checkAnswer(F("AT+CIPSTATUS"));
}

bool ATClient::CIPSTART(TCP_TYPE type, uint8_t ip[4], int port, int8_t link_id, int udp_port, uint8_t udp_mode, int keepalive) {
	if(udp_port!=-1 && type!=UDP) //udp_port and udp_mode valid only in UDP type
		return false;
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;
	if(udp_mode>2)
		return false;
	if(keepalive!=-1 && keepalive>7200)
		return false;

	char cmd[80] PROGMEM = "AT+CIPSTART="; //TODO needs to be static if PROGMEM
	int len = strlen(cmd);
	if(link_id!=-1){
		len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
	}
	if(type==TCP)
		strcat_P(cmd, PSTR("\"TCP\","));
	else if(type==UDP)
		strcat_P(cmd, PSTR("\"UDP\","));
	len = strlen(cmd);

	len += sprintf_P(cmd+len, PSTR("\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);

	if(udp_port!=-1)
		len += sprintf_P(cmd+len, PSTR(",%d,%u"), udp_port, udp_mode);
	if(keepalive!=-1)
		len += sprintf_P(cmd+len, PSTR(",%d"), keepalive);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPSTART(TCP_TYPE type, String address, int port, int8_t link_id, int udp_port, uint8_t udp_mode, int keepalive) {
	if(udp_port!=-1 && type!=UDP) //udp_port and udp_mode valid only in UDP type
		return false;
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;
	if(udp_mode>2)
		return false;
	if(keepalive!=-1 && keepalive>7200)
		return false;

	char cmd[200] = "AT+CIPSTART=";
	int len = strlen(cmd);
	if(link_id!=-1)
		len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
	if(type==TCP)
		strcat_P(cmd, PSTR("\"TCP\","));
	else if(type==UDP)
		strcat_P(cmd, PSTR("\"UDP\","));

	len = strlen(cmd);
	len += sprintf_P(cmd+len, PSTR("\"%s\",%d"), address, port);
	if(udp_port!=-1)
		len += sprintf_P(cmd+len, PSTR(",%d,%u"), port, udp_mode);
	if(keepalive!=-1)
		len += sprintf_P(cmd+len, PSTR(",%d"), keepalive);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPSEND(String &data, int link_id, uint8_t ip[4], int port) {
	if(ip!=nullptr && port==-1) //If ip is set, port must be set
		return false;
	if(ip==nullptr && port!=-1) //Port cannot be set if ip is not
		return false;
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;
	uint16_t datas = data.length();
	if(datas>2048) //Maximum size of a single transmission
		return false;

	char cmd[80] PROGMEM = "AT+CIPSEND=";
	int len = strlen(cmd);
	if(link_id!=-1)
		len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
	len += sprintf_P(cmd+len, PSTR("%u"), datas);
	if(ip!=nullptr)
		len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);

	sendCommand(cmd);
	bool success = waitMessage(F(">"));
	if(!success)
		return false;

	sendData(data);

	return waitMessage(F("SEND OK"));
}

bool ATClient::CIPSENDEX(uint16_t length, int link_id, uint8_t ip[4], int port) {
	if(ip!=nullptr && port==-1) //If ip is set, port must be set
		return false;
	if(ip==nullptr && port!=-1) //Port cannot be set if ip is not
		return false;
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;
	if(length>2048) //Maximum size of a single transmission
		return false;

	char cmd[30] PROGMEM = "AT+CIPSEND=";
	int len = strlen(cmd);
	if(link_id!=-1)
		len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
	len += sprintf_P(cmd+len, PSTR("%u"), length);
	if(ip!=nullptr)
		len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);

	sendCommand(cmd);
	return waitMessage(F(">"));
}

bool ATClient::CIPSENDBUF(String &data, uint8_t &bufferNr, int link_id) {
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;
	uint16_t datas = data.length();
	if(datas>2048) //Maximum size of a single transmission
		return false;

	char cmd[30] PROGMEM = "AT+CIPSENDBUF=";
	int len = strlen(cmd);
	if(link_id!=-1)
		len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
	len += sprintf_P(cmd+len, PSTR("%u"), datas);

	sendCommand(cmd);
	bool success = waitMessage(F(">"));
	if(!success)
		return false;
	sendData(data);

	//Principle of buffer send: do not wait. Check later what was done (CIPBUFSTATUS)
	//success = waitMessage("SEND OK", true);
	//if(!success)
	//	return false;
    //
	//if(_lastDataSize!=1)
	//	return false;
    //
	//String resp;
	//if(link_id==-1)
	//	resp = _lastData[0].substring(0, _lastData[0].length()-8);
	//else
	//	resp = _lastData[0].substring(2, _lastData[0].length()-8);
	//bufferNr = resp.toInt();

	return true;
}

bool ATClient::CIPBUFSTATUS(uint8_t link_id) {
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;

	char cmd[30] PROGMEM = "AT+CIPBUFSTATUS";
	if(link_id!=-1)
		sprintf_P(cmd+strlen(cmd), PSTR("=%u"), link_id);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPCHECKSEQ(uint8_t segment, uint8_t link_id) {
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;

	char cmd[30] = "AT+CIPCHECKSEQ=";
	int len = strlen(cmd);
	if(link_id!=-1)
		len += sprintf_P(cmd+len, PSTR("%u,"), link_id);
	sprintf_P(cmd+len, PSTR("%u"), segment);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPBUFRESET(uint8_t link_id) {
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;

	char cmd[30] PROGMEM = "AT+CIPBUFRESET";
	if(link_id!=-1)
		sprintf_P(cmd+strlen(cmd), PSTR("=%u"), link_id);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPCLOSE(uint8_t link_id) {
	if(link_id!=-1 && link_id>5) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here). 5 is special, close all connections
		return false;

	char cmd[30] PROGMEM = "AT+CIPCLOSE";
	if(link_id!=-1)
		sprintf_P(cmd+strlen(cmd), PSTR("=%u"), link_id);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIFSR() {
	sendCommand(F("AT+CIFSR"));
	return checkAnswer(F("AT+CIFSR"));
}

bool ATClient::CIPMUX(bool mode) {
	char cmd[15];
	sprintf_P(cmd, PSTR("AT+CIPMUX=%d"), mode);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPSERVER(bool on, int port) {
	char cmd[30];
	int len = sprintf_P(cmd, PSTR("CIPSERVER=%d"), on);
	if(on && port>=0)
		sprintf_P(cmd+len, PSTR(",%d"), port);

	sendCommand(cmd);
	return checkAnswer(cmd);
}


bool ATClient::CIPSAVETRANSLINK(bool on, uint8_t ip[4], int port, TCP_TYPE type, int keepalive, int udp_port) {
	if(udp_port!=-1 && type!=UDP) //udp_port and udp_mode valid only in UDP type
		return false;
	if(keepalive!=-1 && keepalive>7200)
		return false;
	if(on && ip==nullptr)
		return false;
	if(ip!=nullptr && port==-1)
		return false;

	char cmd[80];
	int len = sprintf_P(cmd, PSTR("AT+CIPSAVETRANSLINK=%d"), on);
	if(on)
		len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);
	if(type==TCP)
		strcat_P(cmd, PSTR(",\"TCP\""));
	else if(type==UDP)
		strcat_P(cmd, PSTR(",\"UDP\""));
	len = strlen(cmd);
	if(keepalive>=0)
		len += sprintf_P(cmd+len, PSTR(",%d"), keepalive);
	if(udp_port!=-1)
		len += sprintf_P(cmd+len, PSTR(",%d"), udp_port);

	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPSTO(int keepalive) {
	if(keepalive<0 || keepalive>7200)
		return false;

	char cmd[20];
	sprintf_P(cmd, PSTR("AT+CIPSTO=%d"), keepalive);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::PINGA(const char *address) {
	char cmd[100];
	sprintf_P(cmd, PSTR("AT+PING=\"%s\""), address);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::PINGA(uint8_t ip[4]) {
	char cmd[30];
	sprintf_P(cmd, PSTR("AT+PING=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

bool ATClient::CIPDINFO(bool on) {
	char cmd[20];
	sprintf_P(cmd, PSTR("AT+CIPDINFO=%d"), on);
	sendCommand(cmd);
	return checkAnswer(cmd);
}

//######################## PRIVATE ##################
String ATClient::read() {
	String response;
	if (_atSerial->available() > 0) {
		response = _atSerial->readStringUntil('\n');
	}

	return response;
}

String ATClient::readRaw() {
	String response;
	while (_atSerial->available() > 0) {
		response = _atSerial->readString();
	}

	return response;
}

String ATClient::readWait() {
	String response;
	bool got_answer = false;
	_waitingForAnswer = millis();
	while ((_atSerial->available() > 0)
			|| ((millis()-_waitingForAnswer) < _timeout && !got_answer)) {
		response = _atSerial->readStringUntil('\n');
		response.trim();
		if (response.length() > 0)
			got_answer = true;
	}
	return response;
}

bool ATClient::checkAnswer(String command) {
	String answer;
	bool got_command = false;
	bool got_ok = false;
	bool got_error = false;
	unsigned long startWait = millis()-1;
	_lastDataSize = 0;
	do{
		answer = readWait();
		if(!got_command && answer==command)
			got_command = true;
		else if(!got_ok && answer=="OK")
			got_ok = true;
		else if(!got_error && answer=="ERROR")
			got_error = true;
		else
			addDataLine(answer);
		//_logSerial->println("CA:" + answer);
	}
	while(answer.length()!=0 && (!got_ok && !got_error)  && (millis()-startWait)<5*_timeout);

	if(got_error){
		_logSerial->println(F("Command failed with ERROR"));
	}

	return got_ok;
}

bool ATClient::waitMessage(String message, bool anywhere) {
	String answer;
	bool got_message = false;
	unsigned long startWait = millis()-1;
	_lastDataSize = 0;
	do{
		answer = readWait();
		if(!got_message && ((anywhere && answer.indexOf(message)!=-1) || (answer==message)))
			got_message = true;
		else
			addDataLine(answer);
		if(got_message && anywhere)
			addDataLine(answer);
		//_logSerial->println("WM: " + answer);
	}
	while(answer.length()!=0 && !got_message && (millis()-startWait)<5*_timeout);

	return got_message;
}

template<uint8_t N>
bool ATClient::checkSequence(const char *seq[N]) {
	for (uint8_t i = 0; i < N; ++i) {
		String answer = readWait();
		//_logSerial->println("S: " + answer + " " + answer.length() + " " + answer.indexOf(seq[i]));
		if (answer.length() == 0 || answer.indexOf(seq[i]) != 0)
			return false;
	}

	return true;
}

template<uint8_t N>
bool ATClient::checkSequenceCapture(const char *seq[N], String (&data)[N]) {
	for (uint8_t i = 0; i < N; ++i) {
		String answer = readWait();
		//_logSerial->println("SC:" + answer);
		if (answer.length() == 0 || answer.indexOf(seq[i]) != 0)
			return false;
		int seq_length = String(seq[i]).length();
		if (answer.length() > seq_length)
			data[i] = answer.substring(seq_length);
	}

	return true;
}

void ATClient::addDataLine(String data){
	if(_lastDataSize>=MAX_DATA_LINES)
		return;
	_lastData[_lastDataSize++] = data;
}
