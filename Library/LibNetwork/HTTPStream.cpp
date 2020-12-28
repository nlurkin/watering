/*
 * HTTPStream.cpp
 *
 *  Created on: 28 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include <HTTPStream.h>
#include "HTTPRequest.h"

HTTPStream::HTTPStream(ESP8266Wifi &wifi) :
  _dest_address(nullptr),
  _dest_port(0),
  _wifi(wifi),
  _server(wifi)
{
}

HTTPStream::~HTTPStream() {
  if(_dest_address)
    delete[] _dest_address;
  _dest_address = nullptr;
}

void HTTPStream::begin(uint16_t port) {
  if(_server.startServer(port)){
    NetworkStream::begin();
  }
}

int HTTPStream::available() {
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

void HTTPStream::flush() {
  HTTPRequest r = HTTPRequest::http_post("/api/v1/arduino_console");
  char buff[NETWORK_TX_BUFFER_SIZE+HTTPRequest::MAX_HEADER_LENGTH]; //Must be able to contain data + header
  _tx_buffer.get(buff, NETWORK_TX_BUFFER_SIZE);
  clear();
  r.setConnectionType(HTTPRequest::CONN_CLOSE);
  r.addContent(buff);
  int conn = _wifi.openConnection(_dest_address, _dest_port);
  r.generate();
  r.getRawRequest(buff);
    if(_wifi.sendPacket(buff, conn)){
        if(!HTTPRequest::wait200OK(_wifi, conn))
            _wifi.closeConnection(conn); // Probably not closed since server did not understand
    }
}

void HTTPStream::setDestination(const char *address, uint16_t port) {
  _dest_address = new char[strlen(address)+1];
  strcpy(_dest_address, address);
  _dest_port = port;
}

