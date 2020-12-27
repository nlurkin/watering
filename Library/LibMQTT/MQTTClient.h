/*
 * MQTTClient.h
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef LIBMQTT_MQTTCLIENT_H_
#define LIBMQTT_MQTTCLIENT_H_

#include "ESP8266Wifi.h"

class MQTTClient {
public:
  MQTTClient(ESP8266Wifi &wifi);
  virtual ~MQTTClient();

  void setDestination(const char *address, uint16_t port);
  void begin();

  bool connect();
  bool publish(const char* pubname, const char *data);

private:
  bool connected();
  bool send_connect(uint8_t conn);

  int8_t _connection;
  char *_dest_address;
  uint16_t _dest_port;
  ESP8266Wifi &_wifi;
};

#endif /* LIBMQTT_MQTTCLIENT_H_ */
