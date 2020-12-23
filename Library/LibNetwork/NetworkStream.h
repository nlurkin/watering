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
#include "Buffer.h"

class NetworkStream : public Stream {
public:
  static const uint16_t NETWORK_RX_BUFFER_SIZE = 64; // Matching what we have in Serial
  static const uint16_t NETWORK_TX_BUFFER_SIZE = 64; // Matching what we have in Serial

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

  void setDestination(const char *address, uint16_t port);
private:
  static size_t addChar(char* buffer, uint16_t &pos, uint16_t &size, uint16_t max_size, uint8_t v);

  char *_dest_address;
  uint16_t _dest_port;
  ESP8266Wifi &_wifi;
  HTTPServer _server;
  Buffer _rx_buffer;
  Buffer _tx_buffer;
};

#endif /* NETWORKSTREAM_H_ */
