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
  static constexpr size_t  MAX_MESSAGE_LENGTH = 300;
  static constexpr size_t  MAX_HEADER_LENGTH = 50;
  static constexpr size_t  MAX_PACKET_LENGTH = MAX_MESSAGE_LENGTH + MAX_HEADER_LENGTH;

  MQTTClient(ESP8266Wifi &wifi, const char* name);
  virtual ~MQTTClient();

  void setDestination(const char *address, uint16_t port);
  void setDestination(const __FlashStringHelper *address, uint16_t port);
  void setUserPass(const char *username, const char *password);
  void setUserPass(const __FlashStringHelper *username, const __FlashStringHelper *password);
  void begin();

  bool connect();
  bool publish(const char* pubname, const char *data);
  uint8_t subscribe(const char* pubname);
  bool listen(char *pubname, char*pubdata);
  bool id_used(uint16_t id);
  void free_id(uint16_t id);

private:
  bool connected();
  bool send_connect(uint8_t conn);
  uint8_t get_unused_id();

  int8_t _connection;
  char *_dest_address;
  uint16_t _dest_port;
  char *_name;
  char *_username;
  char *_password;
  int8_t _msg_ids[MAX_MESSAGE_IDS];
  ESP8266Wifi &_wifi;
};

#endif /* LIBMQTT_MQTTCLIENT_H_ */
