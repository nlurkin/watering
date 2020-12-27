/*
 * MQTTPacket.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include "Packet.h"

namespace MQTT {
Packet::Packet() {
}

Packet::Packet(char *buff) {
  memcpy(&_fixed_header, buff, sizeof(uint8_t));
  buff += sizeof(uint8_t);
  for(int i=0; i<4; ++i) {
    memcpy(&_fixed_header._len[i], buff, sizeof(uint8_t));
    buff += sizeof(uint8_t);
    if(_fixed_header._len[i]<128)
      break;
  }
  int16_t hsize = getExpectedHeaderSize();
  if(hsize>0){
    _var_header._n_bytes = hsize;
  }
  else if(hsize==-2){
    uint16_t s_size = (buff[0]>>8) + buff[1];
    _var_header._n_bytes = s_size + 2;
  }
  memcpy(_var_header._bytes, buff, _var_header._n_bytes * sizeof(uint8_t));
  buff += _var_header._n_bytes;
  int16_t psize = getExpectedPayloadSize();
  if(psize>0){
    _payload._n_bytes = psize;
  }
  else if(psize<0){
    _payload._n_bytes = getRemainLen()-_var_header._n_bytes;
  }
  memcpy(_payload._bytes, buff, _payload._n_bytes * sizeof(uint8_t));
}

Packet::~Packet() {
}

void Packet::setCtrlType(ctrl_type ctrl_type) {
  _fixed_header._ctrl_type = ctrl_type;
}

void Packet::setMsgType(uint8_t val) {
  _fixed_header._type = val;
}

void Packet::computeRLength() {
  uint32_t len = _var_header._n_bytes + _payload._n_bytes;
  _fixed_header._len[3] = int(len/2097152);
  len = len % 2097152;
  _fixed_header._len[2] = int(len/16384);
  len = len % 16384;
  _fixed_header._len[1] = int(len/128);
  len = len % 128;
  _fixed_header._len[0] = int(len);

  if(_fixed_header._len[3]){
    _fixed_header._len[2] += 128;
    _fixed_header._len[1] += 128;
    _fixed_header._len[0] += 128;
  }
  else if(_fixed_header._len[2]){
    _fixed_header._len[1] += 128;
    _fixed_header._len[0] += 128;
  }
  else if(_fixed_header._len[1]){
    _fixed_header._len[0] += 128;
  }
}

bool Packet::addVarHeader(uint8_t h) {
  if(_var_header._n_bytes>=128)
    return false;
  _var_header._bytes[_var_header._n_bytes++] = h;
  return true;
}

bool Packet::addVarHeader(const char *h) {
  uint8_t n_bytes = strlen(h);
  if(n_bytes+2 > 128-_var_header._n_bytes)
    return false;
  _var_header._bytes[_var_header._n_bytes++] = 0;
  _var_header._bytes[_var_header._n_bytes++] = n_bytes;
  for(uint8_t i=0; i<n_bytes; ++i){
    _var_header._bytes[_var_header._n_bytes++] = h[i];
  }
  return true;
}

bool Packet::addPayload(uint8_t p) {
  if(_payload._n_bytes>=128)
    return false;
  _payload._bytes[_payload._n_bytes++] = p;
  return true;
}

bool Packet::addPayload(const char *p) {
  uint8_t n_bytes = strlen(p);
  if(n_bytes+2 > 128-_payload._n_bytes)
    return false;
  _var_header._bytes[_var_header._n_bytes++] = 0;
  _var_header._bytes[_var_header._n_bytes++] = n_bytes;
  for(uint8_t i=0; i<n_bytes; ++i){
    _payload._bytes[_payload._n_bytes++] = p[i];
  }
  return true;
}

uint32_t Packet::fillBuffer(char *buffer) {
  char* p = buffer;
  memcpy(p, &_fixed_header, sizeof(uint8_t));
  p += sizeof(uint8_t);
  for(int i=0; i<4; ++i) {
    if(_fixed_header._len[i]){
      memcpy(p, &_fixed_header._len[i], sizeof(uint8_t));
      p += sizeof(uint8_t);
    }
  }
  memcpy(p, _var_header._bytes, _var_header._n_bytes * sizeof(uint8_t));
  p += _var_header._n_bytes;
  memcpy(p, _payload._bytes, _payload._n_bytes * sizeof(uint8_t));
  return (p-buffer) + _payload._n_bytes;
}

void Packet::print() {
  Serial.println("MQTT Packet");
  Serial.println(" -- Fix header --");
  Serial.print(_fixed_header._type);
  Serial.print(" ");
  Serial.println(_fixed_header._ctrl_type);
  Serial.print("RLen:");
  Serial.print(getRemainLen());
  Serial.print(" -> ");
  for(int i=0; i<4; ++i){
    Serial.print(_fixed_header._len[i]);
    Serial.print(":");
  }
  Serial.println();
  Serial.println(" -- Var header --");
  for(int i=0; i<_var_header._n_bytes; ++i){
    Serial.print(_var_header._bytes[i]);
    Serial.print(" -> ");
    Serial.println((char)_var_header._bytes[i]);
  }
  Serial.println(" -- Payload --");
  for(int i=0; i<_payload._n_bytes; ++i) {
    Serial.print(_payload._bytes[i]);
    Serial.print(" -> ");
    Serial.println((char)_payload._bytes[i]);
  }
}

uint32_t Packet::getTotalLen() {
  return sizeof(FixHeader) + _var_header._n_bytes + _payload._n_bytes;
}

uint32_t Packet::getRemainLen() {
  return _fixed_header._len[0] + _fixed_header._len[1]*128 + _fixed_header._len[2]*16384 + _fixed_header._len[3]*2097152;
}

int16_t Packet::getExpectedHeaderSize() {
  switch (_fixed_header._ctrl_type) {
  case (CONNECT):
    break;
  case (CONNACK):
    return 2;
    break;
  case (PUBLISH): {
    return -2;
    break;
  }
  }

  return 0;
}

int16_t Packet::getExpectedPayloadSize() {
  switch (_fixed_header._ctrl_type) {
  case (CONNECT):
    break;
  case (CONNACK):
    return 0;
    break;
  case (PUBLISH):
    return -1;
    break;
  }

  return 0;
}

size_t Packet::getVarHeaderString(uint8_t byte, char *buff) const {
  uint16_t len = (_var_header._bytes[byte]<<8) + _var_header._bytes[byte+1];
  for(uint8_t i=0; i<len; ++i){
    buff[i] = _var_header._bytes[i+2];
  }
  buff[len] = '\0';
  return len;
}

uint16_t Packet::getVarHeader16_t(uint8_t byte) const {
  return (_var_header._bytes[byte]<<8) + _var_header._bytes[byte+1];
}

size_t Packet::getPayloadString(uint8_t byte, char *buff) const {
  uint16_t len = (_payload._bytes[byte]<<8) + _payload._bytes[byte+1];
  for(uint8_t i=0; i<len; ++i){
    Serial.print(_payload._bytes[i+2]);
    buff[i] = _payload._bytes[i+2];
  }
  buff[len] = '\0';
  return len;
}

size_t Packet::getFullPayload(char *buff) const {
  uint8_t offset=0;
  if(_payload._bytes[0]==0) // Must be the format with size:value
    offset = 2;

  for(uint8_t i=0; i<_payload._n_bytes-offset; ++i)
    buff[i] = _payload._bytes[i+offset];

  buff[_payload._n_bytes-offset] = '\0';
  return _payload._n_bytes;
}

} /* namespace MQTT */
