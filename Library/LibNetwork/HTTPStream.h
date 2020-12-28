/*
 * HTTPStream.h
 *
 *  Created on: 28 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef LIBRARIES_LIBNETWORK_HTTPSTREAM_H_
#define LIBRARIES_LIBNETWORK_HTTPSTREAM_H_

#include "NetworkStream.h"
#include "ESP8266Wifi.h"
#include "HTTPServer.h"

class HTTPStream: public NetworkStream {
public:
  HTTPStream(ESP8266Wifi &wifi);
  virtual ~HTTPStream();

  virtual void begin(uint16_t port=80);
  virtual int available();
  virtual void flush();

  void setDestination(const char *address, uint16_t port);

private:
  char *_dest_address;
  uint16_t _dest_port;
  ESP8266Wifi &_wifi;
  HTTPServer _server;
};

#endif /* LIBRARIES_LIBNETWORK_HTTPSTREAM_H_ */
