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
  USERNAME=0x1, PASSWORD=0x2, WILL_RETAIN=0x4, WILL_QOS_1=0x8, WILL_QOS_2=0x10, WILL_FLAG=0x20, CLEAN_SESSION=0x40
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
  bool addPayload(uint8_t p);
  bool addPayload(const char *p);

  uint32_t fillBuffer(char* buffer);
  uint32_t getTotalLen();

  uint8_t getExpectedHeaderSize();
  uint8_t getExpectedPayloadSize();

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

private:
  FixHeader _fixed_header;
  VarHeader _var_header;
  Payload   _payload;
};
} /* namespace MQTT */
#endif /* LIBMQTT_PACKET_H_ */
