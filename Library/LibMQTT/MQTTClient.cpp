/*
 * MQTTClient.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include <MQTTClient.h>
#include "Packet.h"

MQTTClient::MQTTClient(ESP8266Wifi &wifi) :
  _connection(-1),
  _dest_address(nullptr),
  _dest_port(0),
  _wifi(wifi)
{
  for(uint8_t i=0;i<MAX_MESSAGE_IDS; ++i)
    _msg_ids[i] = -1;
}

MQTTClient::~MQTTClient() {
}

void MQTTClient::setDestination(const char *address, uint16_t port) {
  _dest_address = new char[strlen(address)+1];
  strcpy(_dest_address, address);
  _dest_port = port;
}

bool MQTTClient::connect() {
  if(!connected()) {
    _connection = -1;
    uint8_t conn = _wifi.openConnection(_dest_address, _dest_port);
    if(send_connect(conn))
      _connection = conn;
  }
  return _connection!=-1;
}

bool MQTTClient::publish(const char *pubname, const char *data) {
  if(!connect()) // Unable to establish connection
    return false;

  MQTT::Packet packet;
  packet.setCtrlType(MQTT::PUBLISH);
  packet.setMsgType(MQTT::RETAIN);

  packet.addVarHeader(pubname);                //Pubname

  packet.addPayload(data);

  packet.computeRLength();

  char buff[MAX_PACKET_LENGTH];
  if(packet.getTotalLen()>MAX_PACKET_LENGTH)
    return false;
  uint32_t len = packet.fillBuffer(buff);
  return _wifi.sendPacketLen(buff, _connection, len);
}

bool MQTTClient::subscribe(const char *pubname) {
  if(!connect()) // Unable to establish connection
    return false;

  MQTT::Packet packet;
  packet.setCtrlType(MQTT::SUBSCRIBE);
  packet.setMsgType(MQTT::QOS_1);

  uint8_t msg_id = get_unused_id();
  packet.addVarHeader((uint8_t)0);      // Message ID (MSB)
  packet.addVarHeader((uint8_t)msg_id); // Message ID (LSB)

  packet.addPayload(pubname);         // Publication name
  packet.addPayload((uint8_t)0);

  packet.computeRLength();

  char buff[MAX_PACKET_LENGTH];
  if(packet.getTotalLen()>MAX_PACKET_LENGTH)
    return false;
  uint32_t len = packet.fillBuffer(buff);

  return _wifi.sendPacketLen(buff, _connection, len);
}


bool MQTTClient::send_connect(uint8_t conn) {
  MQTT::Packet packet;
  packet.setCtrlType(MQTT::CONNECT);

  packet.addVarHeader("MQTT");     //Protocol name
  packet.addVarHeader(0x04);       //Protocol version
  packet.addVarHeader((uint8_t)0); //Connect flags
  packet.addVarHeader((uint8_t)0); //Keep alive MSB
  packet.addVarHeader(120);        //Keep alive LSB

  packet.addPayload("arduino");

  packet.computeRLength();

  char buff[MAX_PACKET_LENGTH];
  if(packet.getTotalLen()>MAX_PACKET_LENGTH)
    return false;
  uint32_t len = packet.fillBuffer(buff);

  if(_wifi.sendPacketLen(buff, conn, len)){
    if(_wifi.waitPayload(conn, buff, 1000, true)==conn){
      MQTT::Packet response = (buff);
      return response.getFixedHeader()._ctrl_type==MQTT::CONNACK;
    }
  }
  return false;
}

bool MQTTClient::connected() {
  if(_connection==-1)
    return false;
  return _wifi.isConnectionOpened(_connection);
}

void MQTTClient::begin() {
  _wifi.startServer();
}

bool MQTTClient::listen(char *pubname, char*pubdata) {
  if(!connect()) // Unable to establish connection
    return false;

  char buff[ESP8266Wifi::PAYLOAD_SIZE];
  int8_t conn = _wifi.waitPayload(_connection, buff, 10, true);
  if(conn==-1)
      return false;

  MQTT::Packet packet(buff);
  packet.print();


  switch (packet.getFixedHeader()._ctrl_type) {
  case(MQTT::SUBACK): {
    uint16_t msg_id = packet.getVarHeader16_t(0);
    free_id(msg_id);
    return false;
  }
  case(MQTT::PUBLISH): {
    packet.getVarHeaderString(0, pubname);
    packet.getFullPayload(pubdata);
    break;
  }
  }
  return true;
}

uint8_t MQTTClient::get_unused_id() {
  bool found = false;
  uint8_t id=1;
  while(!found){
    found = true;
    for(uint8_t i=0; i<MAX_MESSAGE_IDS; ++i){
      if(_msg_ids[i]==id){
        ++id;
        found = false;
        break;
      }
    }
  }

  for(uint8_t i=0; i<MAX_MESSAGE_IDS; ++i){
    if(_msg_ids[i]==-1){
      _msg_ids[i] = id;
      break;
    }
  }
  return id;
}

void MQTTClient::free_id(uint16_t id) {
  for(uint8_t i=0; i<MAX_MESSAGE_IDS; ++i){
    if((uint16_t)_msg_ids[i]==id){
      _msg_ids[i] = -1;
      return;
    }
  }
}
