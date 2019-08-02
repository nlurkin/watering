/*
 * NetworkSerial.cpp
 *
 *  Created on: 2 Aug 2019
 *      Author: Nicolas Lurkin
 */

#include "NetworkStream.h"

#include "HTTPRequest.h"

NetworkStream::NetworkStream(ESP8266Wifi &wifi) :
	_dest_port(0),
	_rx_pos(0),
	_rx_size(0),
	_tx_pos(0),
	_tx_size(0),
	_wifi(wifi),
	_server(wifi)
{
}

NetworkStream::~NetworkStream() {
}

void NetworkStream::begin(uint16_t port) {
	if(_server.startServer(port)){
		const char message[] PROGMEM = {"Server started on port 80"};
		Serial.println(message);
		println(message);
	}
}

size_t NetworkStream::write(uint8_t v) {
	return addChar(_tx_buffer, _tx_pos, _tx_size, NETWORK_TX_BUFFER_SIZE, v);
}

int NetworkStream::available() {
	String data;
	if(_wifi.payloadAvailable()>=0)
		data = _server.loop();

	int len = data.length();
	size_t written = 1;
	int pos=0;
	while(pos<len && written>0)
		written = addChar(_rx_buffer, _rx_pos, _rx_size, NETWORK_RX_BUFFER_SIZE, data[pos++]);

	return _rx_size;
}

int NetworkStream::peek() {
	return _rx_size == 0 ? -1 : _rx_buffer[_rx_pos];
}

int NetworkStream::read() {
	if (_rx_size == 0) {
		return -1;
	} else {
		int ret = _rx_buffer[_rx_pos];
		_rx_pos++;
		_rx_size--;
		if (_rx_pos == NETWORK_RX_BUFFER_SIZE) {
			_rx_pos = 0;
		}
		return ret;
	}
}

int NetworkStream::availableForWrite() {
	return NETWORK_TX_BUFFER_SIZE - _tx_size;
}

void NetworkStream::flush() {
	HTTPRequest r = HTTPRequest::http_post();
	String d;
	d.reserve(_tx_size);
	for(unsigned int i=0; i<_tx_size; ++i)
		d+=_tx_buffer[i];
	r.addContent(d);
	int conn = _wifi.openConnection(_dest_address, _dest_port);
	_wifi.sendPacket(r.generate(), conn);
}

void NetworkStream::setDestination(String address, uint16_t port) {
	_dest_address = address;
	_dest_port = port;
}

size_t NetworkStream::addChar(char *buffer, uint16_t &pos, uint16_t &size, uint16_t max_size, uint8_t v) {
	if (size == max_size) {
		return 0;
	} else {
		int p = pos + size;
		if (p >= max_size) {
			p -= max_size;
		}
		buffer[p] = v;
		size++;
		return 1;
	}
}
