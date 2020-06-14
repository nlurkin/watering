/*
 * PubServer.h
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#ifndef PUBSERVER_H_
#define PUBSERVER_H_

#include <Arduino.h>
#include "../LibESP8266/ESP8266Wifi.h"

class PublicationBase;
/*
 *
 */
class ControlServer {
public:
	static constexpr size_t MAX_PUBLICATIONS = 20;
	static constexpr size_t MAX_MESSAGE_LENGTH = MAX_PUBLICATIONS + 50;

	ControlServer(ESP8266Wifi &wifi);
	virtual ~ControlServer();

	bool addPublication(PublicationBase* pub);

	bool serve();
	void setDestination(const char *address, uint16_t port);

private:
	uint8_t _num_publications;
	PublicationBase * _publications[MAX_PUBLICATIONS];
	char *_dest_address;
	uint16_t _dest_port;
	ESP8266Wifi &_wifi;
};

#endif /* PUBSERVER_H_ */
