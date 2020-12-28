/*
 * MQTTStream.cpp
 *
 *  Created on: 28 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include <MQTTStream.h>

MQTTStream::MQTTStream(ESP8266Wifi &wifi) :
  _mqtt_owned(true),
  _mqtt(new MQTTClient(wifi))
{
}

MQTTStream::MQTTStream(MQTTClient &mqtt) :
  _mqtt_owned(false),
  _mqtt(&mqtt)
{
}

MQTTStream::~MQTTStream() {
  if(_mqtt && _mqtt_owned){
    delete _mqtt;
    _mqtt = nullptr;
  }
}


void MQTTStream::setDestination(const char *address, uint16_t port) {
  _mqtt->setDestination(address, port);
}

void MQTTStream::begin() {
  _mqtt->begin();
  _mqtt->connect();
  NetworkStream::begin();
}

int MQTTStream::available() {
  // \TODO this is broken. How do we know when we read a NetworkStream publication or a control publication
  char pubname[MQTTClient::MAX_HEADER_LENGTH];
  char data[MQTTClient::MAX_MESSAGE_LENGTH];
  if(!_mqtt->listen(pubname, data))
    return 0;

  int len = strlen(data);
  size_t written = 1;
  int pos=0;
  while(pos<len && written>0)
    written = _rx_buffer.push(data[pos++]);

  return _rx_buffer.len();
}

void MQTTStream::flush() {
  char buff[NETWORK_TX_BUFFER_SIZE];
  _tx_buffer.get(buff, NETWORK_TX_BUFFER_SIZE);
  clear();
  _mqtt->publish("arduino_console", buff);
}
