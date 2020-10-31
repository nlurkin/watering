/*
 * PubServer.cpp
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#include "../LibHTTP/HTTPRequest.h"
#include "ControlServer.h"
#include "PublicationBase.h"

#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))

ControlServer::ControlServer(ESP8266Wifi &wifi) :
	_num_publications(0),
	_num_commands(0),
	_publications{nullptr},
	_commands{nullptr},
	_dest_address(nullptr),
	_dest_port(0),
	_wifi(wifi)
{
}

ControlServer::~ControlServer() {
	if(_dest_address)
		delete[] _dest_address;
	_dest_address = nullptr;
}

bool ControlServer::addPublication(PublicationBase *pub) {
	if(_num_publications>=MAX_PUBLICATIONS)
		return false;

	_publications[_num_publications++] = pub;
	return true;
}

bool ControlServer::addCommand(PublicationBase *cmd) {
	if(_num_commands>=MAX_COMMANDS)
		return false;

	_commands[_num_commands++] = cmd;
	return true;
}

void ControlServer::setDestination(const char *address, uint16_t port) {
	_dest_address = new char[strlen(address)+1];
	strcpy(_dest_address, address);
	_dest_port = port;
}

void ControlServer::begin(uint16_t port){
	if(_wifi.startServer(port)){
		static const char message[] PROGMEM = {"Server started on port 80"};
		Serial.println(FPSTR(message));
	}
}

bool ControlServer::serve() {
	char buff1[MAX_MESSAGE_LENGTH] = ""; //Must be able to contain data + header
	char buff[MAX_MESSAGE_LENGTH+HTTPRequest::MAX_HEADER_LENGTH]; //Must be able to contain data + header
	uint8_t nPubReady=0;
	PublicationBase *updatedPublications[MAX_PUBLICATIONS];
	for(uint8_t iPub=0; iPub<_num_publications; ++iPub){
		if(_publications[iPub]->isUpdated()){
			Serial.print("Publication ready: ");
			Serial.println(iPub);
			updatedPublications[nPubReady++] = _publications[iPub];
		}
	}

	if(nPubReady==0)
		return false;
	int conn = -1;
	if(nPubReady>0)
		conn = _wifi.openConnection(_dest_address, _dest_port);
	for(uint8_t iPub=0; iPub<nPubReady; ++iPub){
			Serial.print("Updating publication ");
			Serial.println(updatedPublications[iPub]->getName());
			char path_buf[30] = "/api/v1/";
			strcpy(path_buf+8, updatedPublications[iPub]->getName());
			HTTPRequest r = HTTPRequest::http_post(path_buf);
			updatedPublications[iPub]->to_string(buff1);
			r.addContent(buff1);
			if(iPub<nPubReady-1)
				r.setConnectionType(HTTPRequest::CONN_KEEPALIVE);
			else
				r.setConnectionType(HTTPRequest::CONN_CLOSE);
			r.generate();
			r.getRawRequest(buff);

			if(_wifi.sendPacket(buff, conn)){
				Serial.println("Send successful");
			}
			updatedPublications[iPub]->updated(false);
	}
	if(conn!=-1)
		_wifi.closeConnection(conn);

	return true;
}

bool ControlServer::listen() {
	_wifi.readAndPrint();
	int8_t conn = _wifi.payloadAvailable();
	if(conn==-1)
		return false;

	const char* data;
	char buff[ESP8266Wifi::PAYLOAD_SIZE];

	uint8_t max_try = 0;
	while(!isPayloadComplete(conn) && _wifi.isConnectionOpened(conn) && (max_try++<10)){
		delay(10);
		_wifi.readAndPrint();
	}

	_wifi.getPayload(buff, conn, ESP8266Wifi::PAYLOAD_SIZE);
	HTTPRequest http(buff);
	if(http.needs_answer()){
		HTTPRequest answer = HTTPRequest::http_200();
		answer.generate();
		answer.getRawRequest(buff);
		_wifi.sendPacket(buff, conn);
	}
	data = http.getData();
	if(strstr_P(data, PSTR("/api/v1/"))==data){
		char sname[PublicationBase::MAX_NAME_LENGTH+1];
		char value[30];
		uint16_t offset = 8;
		uint16_t urllen = strchr(data+offset, ':')-data-offset;
		uint16_t copylen = min(urllen, PublicationBase::MAX_NAME_LENGTH);
		strncpy(sname, data + offset, copylen);
		sname[copylen] = '\0';
		offset += copylen+1;
		strncpy(value, data + offset, 30);
		value[29] = '\0';
		for(uint8_t iCmd=0; iCmd<_num_commands; ++iCmd){
			if(strcmp(_commands[iCmd]->getName(), sname)==0){
				_commands[iCmd]->from_string(value);
			}
		}
	}
	return true;
}

bool ControlServer::isPayloadComplete(int8_t conn) {
	int pos_data = _wifi.payloadContainsAt(conn, "\r\n\r\n");
	Serial.print(pos_data);
	Serial.print(":");
	Serial.println(int(_wifi.payloadLen(conn)));
	if(pos_data==-1 or (pos_data==int(_wifi.payloadLen(conn))-4) )
		//Incomplete data
		return false;
	return true;
}
