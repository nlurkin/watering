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
  static constexpr uint8_t MAX_MESSAGE_IDS = 5;
  MQTTClient(ESP8266Wifi &wifi);
  virtual ~MQTTClient();

  void setDestination(const char *address, uint16_t port);
  void begin();

  bool connect();
  bool publish(const char* pubname, const char *data);
  bool subscribe(const char* pubname);

private:
  bool connected();
  bool send_connect(uint8_t conn);
  uint8_t get_unused_id();
  void free_id(uint16_t id);

  int8_t _connection;
  char *_dest_address;
  uint16_t _dest_port;
  int8_t _msg_ids[MAX_MESSAGE_IDS];
  ESP8266Wifi &_wifi;
};

#endif /* LIBMQTT_MQTTCLIENT_H_ */
