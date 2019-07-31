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

void HTTPServer::loop() {
	int8_t conn = _wifi.payloadAvailable();
	if (conn != -1) {
		HTTPRequest http(_wifi.getPayload(conn));
		http.print();
		if(http.needs_answer()){
			HTTPRequest answer = HTTPRequest::http_200();
			//wifi.sendPacket("HTTP/1.1 200 OK\r\nContent-Type: application/text\r\nContent-Length: 10\r\n\r\n", conn);
			String d = "Hey you!\r\n";
			answer.addContent(d);
			_wifi.sendPacket(answer.generate(), conn);
			_wifi.closeConnection(conn);
		}
	}

}

bool HTTPServer::sendData(String address, uint16_t port) {
	HTTPRequest r = HTTPRequest::http_post();
	String d = "{s:atm,v:5}";
	r.addContent(d);
	int conn = _wifi.openConnection(address, port);
	_wifi.sendPacket(r.generate(), conn);

	return true;
}
