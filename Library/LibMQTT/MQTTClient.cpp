/*
 * MQTTClient.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include <MQTTClient.h>
#include "Packet.h"

MQTTClient::MQTTClient(ESP8266Wifi &wifi) :
  _dest_address(nullptr),
  _dest_port(0),
  _wifi(wifi),
  _connection(-1)
{
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

  char buff[512];
  uint32_t len = packet.fillBuffer(buff);
  return _wifi.sendPacketLen(buff, _connection, len);
}


bool MQTTClient::send_connect(uint8_t conn) {
  MQTT::Packet packet;
  packet.setCtrlType(MQTT::CONNECT);
  packet.setMsgType(0);

  packet.addVarHeader("MQTT");     //Protocol name
  packet.addVarHeader(0x04);       //Protocol version
  packet.addVarHeader((uint8_t)0); //Connect flags
  packet.addVarHeader((uint8_t)0); //Keep alive MSB
  packet.addVarHeader(120);        //Keep alive LSB

  packet.addPayload("arduino");

  packet.computeRLength();

  char buff[512];
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
