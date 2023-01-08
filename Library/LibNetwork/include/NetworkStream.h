/*
 * NetworkSerial.h
 *
 *  Created on: 2 Aug 2019
 *      Author: Nicolas Lurkin
 */

#ifndef NETWORKSTREAM_H_
#define NETWORKSTREAM_H_

#include <Arduino.h>
#include "Buffer.h"

class NetworkStream : public Stream {
public:
  static const uint16_t NETWORK_RX_BUFFER_SIZE = 64; // Matching what we have in Serial
  static const uint16_t NETWORK_TX_BUFFER_SIZE = 64; // Matching what we have in Serial

  NetworkStream();
  virtual ~NetworkStream();

  virtual void begin();

  virtual size_t write(uint8_t);
  virtual int read();
  virtual int availableForWrite();
  virtual int peek();
  virtual void clear();

private:
  static size_t addChar(char* buffer, uint16_t &pos, uint16_t &size, uint16_t max_size, uint8_t v);

protected:
  Buffer _rx_buffer;
  Buffer _tx_buffer;
};

#endif /* NETWORKSTREAM_H_ */
