/*
 * MQTTControl.h
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef LIBMQTT_MQTTCONTROL_H_
#define LIBMQTT_MQTTCONTROL_H_

#include "MQTTClient.h"
#include "ControlServer.h"

class MQTTControl : public ControlServer {
public:
  MQTTControl(ESP8266Wifi &wifi, const char *name);
  MQTTControl(MQTTClient &mqtt, const char *name);
  virtual ~MQTTControl();

  void setDestination(const char *address, uint16_t port);
  void begin();

  virtual bool addCommand(PublicationBase* cmd);

  virtual bool updatePublications(uint8_t nPubReady, PublicationBase *readyPub[MAX_PUBLICATIONS]);
  virtual bool checkSubscriptions(char *sname, char *value);
  virtual bool publishAdvertise(const char * services);

private:
  void requestSubscription(PublicationBase *cmd, uint8_t pubnum);

  bool _mqtt_owned;
  uint8_t _cmd_msg_id[MAX_COMMANDS];
  char * _name;
  MQTTClient *_mqtt;
};

#endif /* LIBMQTT_MQTTCONTROL_H_ */
