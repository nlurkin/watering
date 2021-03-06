/*
 * MQTTPacket.h
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef LIBMQTT_PACKET_H_
#define LIBMQTT_PACKET_H_

#include <Arduino.h>

namespace MQTT {

enum ctrl_type {
  CONNECT = 1, CONNACK = 2, PUBLISH = 3, PUBACK = 4, PUBREC = 5, PUBREL = 6, PUBCOMP = 7,
  SUBSCRIBE = 8, SUBACK = 9, UNSUBSCRIBE = 10, UNSUBACK = 11, PINGREQ = 12, PINGRESP = 13,
  DISCONNECT = 14
};
enum CONNECT_FLAGS {
  USERNAME=0x80, PASSWORD=0x40, WILL_RETAIN=0x20, WILL_QOS_1=0x10, WILL_QOS_2=0x8, WILL_FLAG=0x4, CLEAN_SESSION=0x2
};
enum PUB_FLAGS {
  RETAIN = 0x1, QOS_1 = 0x2, QOS_2 = 0x4, DUP = 0x8
};

struct FixHeader {
  FixHeader() : _type(0), _ctrl_type(0), _len{0,0,0,0} {}
  uint8_t _type:4;
  uint8_t _ctrl_type:4;
  uint8_t _len[4];
};
struct VarHeader {
  VarHeader() : _n_bytes(0), _bytes{0} {};
  uint8_t _n_bytes;
  uint8_t _bytes[128];
};
struct Payload {
  Payload() : _n_bytes(0), _bytes{0} {};
  uint8_t _n_bytes;
  uint8_t _bytes[128];
};

class Packet {
public:
  Packet();
  Packet(char *buff);
  virtual ~Packet();

  void setCtrlType(ctrl_type ctrl_type);
  void setMsgType(uint8_t val);
  void computeRLength();
  bool addVarHeader(uint8_t h);
  bool addVarHeader(const char *h);
  bool addVarHeader(const __FlashStringHelper *h);
  bool addPayload(uint8_t p);
  bool addPayload(const char *p);
  bool addPayload(const __FlashStringHelper *p);

  uint32_t fillBuffer(char* buffer);
  uint32_t getTotalLen();
  uint32_t getRemainLen();

  int16_t getExpectedHeaderSize();
  int16_t getExpectedPayloadSize();

  void print();

  const FixHeader& getFixedHeader() const {
    return _fixed_header;
  }

  const Payload& getPayload() const {
    return _payload;
  }

  const VarHeader& getVarHeader() const {
    return _var_header;
  }

  size_t getVarHeaderString(uint8_t byte, char *buff) const;
  uint16_t getVarHeader16_t(uint8_t byte) const;
  size_t getPayloadString(uint8_t byte, char *buff) const;
  size_t getFullPayload(char *buff) const;

private:
  FixHeader _fixed_header;
  VarHeader _var_header;
  Payload   _payload;
};
} /* namespace MQTT */
#endif /* LIBMQTT_PACKET_H_ */
