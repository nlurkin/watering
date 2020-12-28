/*
 * NetworkSerial.cpp
 *
 *  Created on: 2 Aug 2019
 *      Author: Nicolas Lurkin
 */

#include "NetworkStream.h"

#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))

NetworkStream::NetworkStream() :
  _rx_buffer(NETWORK_RX_BUFFER_SIZE),
  _tx_buffer(NETWORK_TX_BUFFER_SIZE)
{
}

NetworkStream::~NetworkStream() {
}

void NetworkStream::begin() {
  static const char message[] PROGMEM = {"Server started on port 80"};
  Serial.println(FPSTR(message));
  println(FPSTR(message));
}

size_t NetworkStream::write(uint8_t v) {
  if(v=='\n'){
    _tx_buffer.push(v);
    flush();
    return 1;
  }
  else if(availableForWrite()<=1){ //Last char we can write
    //Better flush
    _tx_buffer.push(v);
    flush();
    return 1;
  }
  else
    return _tx_buffer.push(v);
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
  return NETWORK_TX_BUFFER_SIZE-_tx_buffer.len();
}

void NetworkStream::clear() {
  _tx_buffer.clear();
}

size_t NetworkStream::addChar(char *buffer, uint16_t &pos, uint16_t &size, uint16_t max_size, uint8_t v) {
  if (size == max_size) {
    return 0;
  } else {
    int p = pos + size;
    if (p >= (int)max_size) {
      p -= max_size;
    }
    buffer[p] = v;
    size++;
    return 1;
  }
}
