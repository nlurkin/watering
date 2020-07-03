/*
 * PubServer.cpp
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#include "../LibHTTP/HTTPRequest.h"
#include "ControlServer.h"
#include "PublicationBase.h"

ControlServer::ControlServer(ESP8266Wifi &wifi) :
	_num_publications(0),
	_publications{nullptr},
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

void ControlServer::setDestination(const char *address, uint16_t port) {
	_dest_address = new char[strlen(address)+1];
	strcpy(_dest_address, address);
	_dest_port = port;
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
			int conn = _wifi.openConnection(_dest_address, _dest_port);
			if(_wifi.sendPacket(buff, conn)){
				Serial.println("Send successful");
			}
			_wifi.closeConnection(conn);
			updatedPublications[iPub]->updated(false);
	}

	return true;
}
