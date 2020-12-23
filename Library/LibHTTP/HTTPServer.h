/*
 * HTTPServer.h
 *
 *  Created on: 31 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "ESP8266Wifi.h"

class HTTPServer {
public:
	HTTPServer(ESP8266Wifi &wifi);
	virtual ~HTTPServer();

	bool startServer(uint16_t port);
	bool stopServer();

	String loop();

	bool sendData(String address, uint16_t port);

private:
	uint16_t _port;
	ESP8266Wifi &_wifi;
};

#endif /* HTTPSERVER_H_ */
