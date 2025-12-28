/*
 * MQTTStream.h
 *
 *  Created on: 28 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef LIBNETWORK_MQTTSTREAM_H_
#define LIBNETWORK_MQTTSTREAM_H_

#include "NetworkStream.h"
#include "MQTTClient.h"

class MQTTStream: public NetworkStream {
public:
  MQTTStream(ESP8266Wifi &wifi);
  MQTTStream(MQTTClient &mqtt);
  virtual ~MQTTStream();

  void setDestination(const char *address, uint16_t port);
  virtual void begin();
  virtual int available();
  virtual void flush();

private:
  bool _mqtt_owned;
  MQTTClient *_mqtt;
};

#endif /* LIBNETWORK_MQTTSTREAM_H_ */
