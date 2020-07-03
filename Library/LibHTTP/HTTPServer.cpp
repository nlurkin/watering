/*
 * HTTPServer.cpp
 *
 *  Created on: 31 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "HTTPServer.h"
#include "HTTPRequest.h"

HTTPServer::~HTTPServer() {
}

HTTPServer::HTTPServer(ESP8266Wifi &wifi):
	_port(80),
	_wifi(wifi)
{
}

bool HTTPServer::startServer(uint16_t port) {
	_port = port;
	return _wifi.startServer(port);
}

bool HTTPServer::stopServer() {
	return _wifi.stopServer();
}

String HTTPServer::loop() {
	int8_t conn = _wifi.payloadAvailable();
	String data;
	char buff[ESP8266Wifi::PAYLOAD_SIZE];
	if (conn != -1) {
		_wifi.getPayload(buff, conn, ESP8266Wifi::PAYLOAD_SIZE);
		HTTPRequest http(buff);
		http.print();
		if(http.needs_answer()){
			HTTPRequest answer = HTTPRequest::http_200();
			answer.generate();
			answer.getRawRequest(buff);
			_wifi.sendPacket(buff, conn);
			//_wifi.closeConnection(conn);
		}
		data = http.getData();
	}
	return data;
}

bool HTTPServer::sendData(const char *address, uint16_t port) {
	HTTPRequest r = HTTPRequest::http_post("/");
	char buff[HTTPRequest::MAX_PACKET_LENGTH];
	strcpy_P(buff, PSTR("{s:atm,v:5}"));
	r.addContent(buff);
	int conn = _wifi.openConnection(address, port);
	r.generate();
	r.getRawRequest(buff);
	_wifi.sendPacket(buff, conn);

	return true;
}
