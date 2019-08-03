/*
 * NetworkSerial.h
 *
 *  Created on: 2 Aug 2019
 *      Author: Nicolas Lurkin
 */

#ifndef NETWORKSTREAM_H_
#define NETWORKSTREAM_H_

#include <Arduino.h>
#include "ESP8266Wifi.h"
#include "HTTPServer.h"

class NetworkStream : public Stream {
public:
	static const uint16_t NETWORK_RX_BUFFER_SIZE = 64;
	static const uint16_t NETWORK_TX_BUFFER_SIZE = 64;

	NetworkStream(ESP8266Wifi &wifi);
	virtual ~NetworkStream();

	void begin(uint16_t port=80);

	virtual size_t write(uint8_t);
	virtual int read();
	virtual int available();
	virtual int availableForWrite();
	virtual int peek();
	virtual void flush();
	virtual void clear();

	void setDestination(String address, uint16_t port);
private:
	static size_t addChar(char* buffer, uint16_t &pos, uint16_t &size, uint16_t max_size, uint8_t v);
	uint16_t _dest_port;
	uint16_t _rx_pos, _rx_size;
	uint16_t _tx_pos, _tx_size;
	String _dest_address;
	ESP8266Wifi &_wifi;
	HTTPServer _server;
	char _rx_buffer[NETWORK_RX_BUFFER_SIZE];
	char _tx_buffer[NETWORK_TX_BUFFER_SIZE];
};

#endif /* NETWORKSTREAM_H_ */
