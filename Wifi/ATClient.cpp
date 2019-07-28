/*
 * ATClient.cpp
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ATClient.h"

ATClient::ATClient() :
	_set_default(false),
	_waitingForAnswer(0),
	_timeout(2000),
	_lastDataSize(0) {
}

ATClient::~ATClient() {
}

bool ATClient::sendCommand(String cmd) {
	Serial.println("Sending: " + cmd);
	Serial.println();
	Serial1.println("AT+" + cmd);
	return true;
}

bool ATClient::sendData(String data) {
	Serial.println("Sending data: " + data);
	Serial.println();
	Serial1.println(data);
	return true;
}

bool ATClient::sendDataConfirm(String data) {
	Serial.println("Sending data: " + data);
	Serial.println();
	Serial1.println(data);
	return waitMessage("SEND OK");
}

const String* ATClient::getLastData() {
	return _lastData;
}

uint8_t ATClient::getLastDataSize() {
	return _lastDataSize;
}

// ######################## General commands
bool ATClient::AT() {
	Serial1.println("AT");
	return checkAnswer("AT");
}

bool ATClient::RST() {
	sendCommand("RST");
	bool success = checkAnswer("AT+RST");
	if(!success)
		return false;

	return waitMessage("ready");
}

bool ATClient::GMR() {
	sendCommand("GMR");
	return checkAnswer("AT+GMR");
}


bool ATClient::SLEEP(uint8_t mode) {
	if(mode>2)
		return false;
	String cmd = "SLEEP="+String(mode);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

// ######################## WIFI commands
bool ATClient::CWMODE(uint8_t mode) {
	if(mode>3 || mode<1)
		return false;

	String command;
	if(_set_default) // Store in flash
		command = "CWMODE_DEF=" + mode;
	else //Only temporary
		command = "CWMODE_CUR=" + mode;

	sendCommand(command);
	return checkAnswer("AT+" + command);
}

bool ATClient::CWJAP(String &ssid, String &passwd) {
	String cmd;
	if(_set_default) // Store in flash
		cmd = "CWJAC_DEF=";
	else //Only temporary
		cmd = "CWJAC_CUR=";

	cmd += "\""+ssid+"\",\""+passwd+"\"";
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CWLAP() {
	sendCommand("CWLAP");
	return checkAnswer("AT+CWLAP");
}

bool ATClient::CWQAP() {
	sendCommand("CWQAP");
	return checkAnswer("AT+CWQAP");
}

bool ATClient::CWSAP(String &ssid, String &passwd, uint8_t channel, uint8_t ecn) {
	if(ecn>4 || ecn==1) //Can be only 0,2,3,4
		return false;

	String cmd;
	if(_set_default) // Store in flash
		cmd = "CWSAP_DEF=";
	else //Only temporary
		cmd = "CWSAP_CUR=";

	cmd += "\"" + ssid + "\",\"" + passwd + "," + String(channel) + "," + String(ecn);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CWLIF() {
	sendCommand("CWLIF");
	return checkAnswer("AT+CWLIF");
}

bool ATClient::CWDHCP(bool en, uint8_t mode) {
	if(mode>2)
		return false;

	String cmd;
	if(_set_default) // Store in flash
		cmd = "CWDHCP_DEF=";
	else //Only temporary
		cmd = "CWDHCP_CUR=";

	cmd += String(mode) + "," + String(en);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CWAUTOCONN(bool en) {
	String cmd = "CWAUTOCONN=" + String(en);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPSTAMAC(uint8_t mac[6]) {
	String cmd;
	if(_set_default) // Store in flash
		cmd = "CIPSTAMAC_DEF=\"";
	else //Only temporary
		cmd = "CIPSTAMAC_CUR=\"";

	cmd += formMAC(mac) + "\"";
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPAPMAC(uint8_t mac[6]) {
	String cmd;
	if(_set_default) // Store in flash
		cmd = "CIPAPMAC_DEF=\"";
	else //Only temporary
		cmd = "CIPAPMAC_CUR=\"";

	cmd += formMAC(mac) + "\"";
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPSTA(uint8_t ip[4], uint8_t gtw[4], uint8_t netmask[4]) {
	if(netmask!=nullptr && gtw==nullptr)
		return false; //if netmask is set, gtw must be set too

	String cmd;
	if(_set_default) // Store in flash
		cmd = "CIPSTA_DEF=\"";
	else //Only temporary
		cmd = "CIPSTA_CUR=\"";

	cmd += formIP(ip) + "\"";
	if(gtw!=nullptr)
		cmd += ",\"" + formIP(gtw) + "\"";
	if(netmask!=nullptr)
		cmd += ",\"" + formIP(netmask) + "\"";

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);

}

bool ATClient::CIPAP(uint8_t ip[4], uint8_t gtw[4], uint8_t netmask[4]) {
	if( !(netmask==nullptr && gtw==nullptr) || !(netmask!=nullptr && gtw!=nullptr) )
		return false; //must both be set or nullptr

	String cmd;
	if(_set_default) // Store in flash
		cmd = "CIPAP_DEF=\"";
	else //Only temporary
		cmd = "CIPAP_CUR=\"";

	cmd += formIP(ip) + "\"";
	if(gtw!=nullptr)
		cmd += ",\"" + formIP(gtw) + "\"";
	if(netmask!=nullptr)
		cmd += ",\"" + formIP(netmask) + "\"";

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::WPS(bool en){
	String cmd = "WPS=" + String(en);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

// ######################## TCP/IP commands
bool ATClient::CIPSTATUS() {
	sendCommand("CIPSTATUS");
	return checkAnswer("AT+CIPSTATUS");
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

	String cmd = "CIPSTART=";
	if(link_id!=-1)
		cmd += String(link_id) + ",";
	if(type==TCP)
		cmd += "\"TCP\",";
	else if(type==UDP)
		cmd += "\"UDP\",";
	cmd += "\"" + formIP(ip) + "\"," + String(port);
	if(udp_port!=-1)
		cmd += "," + String(port) + "," + String(udp_mode);
	if(keepalive!=-1)
		cmd += "," + String(keepalive);

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
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

	String cmd = "CIPSTART=";
	if(link_id!=-1)
		cmd += String(link_id) + ",";
	if(type==TCP)
		cmd += "\"TCP\",";
	else if(type==UDP)
		cmd += "\"UDP\",";
	cmd += "\"" + address + "\"," + String(port);
	if(udp_port!=-1)
		cmd += "," + String(port) + "," + String(udp_mode);
	if(keepalive!=-1)
		cmd += "," + String(keepalive);

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
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

	String cmd = "CIPSEND=";
	if(link_id!=-1)
		cmd += String(link_id) + ",";
	cmd += String(datas);
	if(ip!=nullptr)
		cmd += ",\"" + formIP(ip) + "\"," + String(port);

	sendCommand(cmd);
	bool success = waitMessage(">");
	if(!success)
		return false;

	sendData(data);

	return waitMessage("SEND OK");
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

	String cmd = "CIPSENDEX=";
	if(link_id!=-1)
		cmd += String(link_id) + ",";
	cmd += String(length);
	if(ip!=nullptr)
		cmd += ",\"" + formIP(ip) + "\"," + String(port);

	sendCommand(cmd);
	return waitMessage(">");
}

bool ATClient::CIPSENDBUF(String &data, uint8_t &bufferNr, int link_id) {
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;
	uint16_t datas = data.length();
	if(datas>2048) //Maximum size of a single transmission
		return false;

	String cmd = "CIPSENDBUF=";
	if(link_id!=-1)
		cmd += String(link_id) + ",";
	cmd += String(datas);

	sendCommand(cmd);
	bool success = waitMessage(">");
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

	String cmd = "CIPBUFSTATUS";
	if(link_id!=-1)
		cmd += "=" + String(link_id);

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPCHECKSEQ(uint8_t segment, uint8_t link_id) {
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;

	String cmd = "CIPCHECKSEQ=";
	if(link_id!=-1)
		cmd += String(link_id) + ",";
	cmd += String(segment);

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPBUFRESET(uint8_t link_id) {
	if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
		return false;

	String cmd = "CIPBUFRESET";
	if(link_id!=-1)
		cmd += "=" + String(link_id);

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPCLOSE(uint8_t link_id) {
	if(link_id!=-1 && link_id>5) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here). 5 is special, close all connections
		return false;

	String cmd = "CIPCLOSE";
	if(link_id!=-1)
		cmd += "=" + String(link_id);

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIFSR() {
	sendCommand("CIFSR");
	return checkAnswer("AT+CIFSR");
}

bool ATClient::CIPMUX(bool mode) {
	String cmd = "CIPMUX=" + mode;
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPSERVER(bool on, int port) {
	String cmd = "CIPSERVER=" + on;
	if(on && port>=0)
		cmd += "," + String(port);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
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

	String cmd = "CIPSAVETRANSLINK=" + String(on);
	if(on)
		cmd += ",\"" + formIP(ip) + "\"," + String(port);
	if(type==TCP)
		cmd += ",\"TCP\"";
	else if(type==UDP)
		cmd += ",\"UDP\"";
	if(keepalive>=0)
		cmd += "," + String(keepalive);
	if(udp_port!=-1)
		cmd += "," + String(udp_port);

	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPSTO(int keepalive) {
	if(keepalive<0 || keepalive>7200)
		return false;

	String cmd = "CIPSTO=" + String(keepalive);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::PINGA(String address) {
	String cmd = "PING=\"" + address + "\"";
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::PINGA(uint8_t ip[4]) {
	String cmd = "PING=\"" + formIP(ip) + "\"";
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

bool ATClient::CIPDINFO(bool on) {
	String cmd = "CIPDINFO=" + String(on);
	sendCommand(cmd);
	return checkAnswer("AT+" + cmd);
}

//######################## PRIVATE ##################
String ATClient::formMAC(uint8_t mac[6]){
	return String(mac[0], HEX) + ":" + String(mac[1], HEX) + ":"
			+ String(mac[2], HEX) + ":" + String(mac[3], HEX) + ":" + String(mac[4], HEX)
			+ ":" + String(mac[5], HEX);
}

String ATClient::read() {
	String response;
	while (Serial1.available() > 0) {
		response = Serial1.readStringUntil('\n');
	}

	return response;
}

String ATClient::readRaw() {
	String response;
	while (Serial1.available() > 0) {
		response = Serial1.readString();
	}

	return response;
}

String ATClient::readWait() {
	String response;
	bool got_answer = false;
	_waitingForAnswer = millis();
	while ((Serial1.available() > 0)
			|| ((millis()-_waitingForAnswer) < _timeout && !got_answer)) {
		response = Serial1.readStringUntil('\n');
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
		//Serial.println("CA:" + answer);
	}
	while(answer.length()!=0 && (!got_ok && !got_error)  && (millis()-startWait)<5*_timeout);

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
		//Serial.println("WM: " + answer);
	}
	while(answer.length()!=0 && !got_message && (millis()-startWait)<5*_timeout);

	return got_message;
}

String ATClient::formIP(uint8_t ip[4]){
	return String(ip[0]) + "." + String(ip[1]) + "."
			+ String(ip[2]) + "." + String(ip[3]);
}

template<uint8_t N>
bool ATClient::checkSequence(const char *seq[N]) {
	for (uint8_t i = 0; i < N; ++i) {
		String answer = readWait();
		//Serial.println("S: " + answer + " " + answer.length() + " " + answer.indexOf(seq[i]));
		if (answer.length() == 0 || answer.indexOf(seq[i]) != 0)
			return false;
	}

	return true;
}

template<uint8_t N>
bool ATClient::checkSequenceCapture(const char *seq[N], String (&data)[N]) {
	for (uint8_t i = 0; i < N; ++i) {
		String answer = readWait();
		//Serial.println("SC:" + answer);
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
