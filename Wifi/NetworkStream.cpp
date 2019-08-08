/*
 * NetworkSerial.cpp
 *
 *  Created on: 2 Aug 2019
 *      Author: Nicolas Lurkin
 */

#include "NetworkStream.h"

#include "HTTPRequest.h"

NetworkStream::NetworkStream(ESP8266Wifi &wifi) :
	_dest_address(nullptr),
	_dest_port(0),
	_wifi(wifi),
	_server(wifi),
	_rx_buffer(NETWORK_RX_BUFFER_SIZE),
	_tx_buffer(NETWORK_TX_BUFFER_SIZE)
{
}

NetworkStream::~NetworkStream() {
	if(_dest_address)
		delete[] _dest_address;
	_dest_address = nullptr;
}

void NetworkStream::begin(uint16_t port) {
	if(_server.startServer(port)){
		static const char message[] PROGMEM = {"Server started on port 80"};
		Serial.println(message);
		println(message);
	}
}

size_t NetworkStream::write(uint8_t v) {
	if(v=='\n'){
		_tx_buffer.push(v);
		flush();
		return 1;
	}
	else
		return _tx_buffer.push(v);
}

int NetworkStream::available() {
	String data;
	if(_wifi.payloadAvailable()>=0)
		data = _server.loop();
	else
		return 0;

	int len = data.length();
	size_t written = 1;
	int pos=0;
	while(pos<len && written>0)
		written = _rx_buffer.push(data[pos++]);

	return _rx_buffer.len();
}

int NetworkStream::peek() {
	return _rx_buffer.len() == 0 ? -1 : _rx_buffer.peek();
}

int NetworkStream::read() {
	if (_rx_buffer.len() == 0)
		return -1;
	else
		return _rx_buffer.read();
}

int NetworkStream::availableForWrite() {
	return _tx_buffer.len();
}

void NetworkStream::flush() {
	HTTPRequest r = HTTPRequest::http_post();
	char buff[NETWORK_TX_BUFFER_SIZE*2]; //Must be able to contain data + header
	_tx_buffer.get(buff, NETWORK_TX_BUFFER_SIZE);
	clear();
	r.addContent(buff);
	int conn = _wifi.openConnection(_dest_address, _dest_port);
	r.generate();
	r.getRawRequest(buff);
	_wifi.sendPacket(buff, conn);
	_wifi.closeConnection(conn);
}

void NetworkStream::clear() {
	_tx_buffer.clear();
}

void NetworkStream::setDestination(const char *address, uint16_t port) {
	_dest_address = new char[strlen(address)+1];
	strcpy(_dest_address, address);
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
