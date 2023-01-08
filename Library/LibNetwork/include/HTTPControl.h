/*
 * HTTPControl.h
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#ifndef LIBNETWORK_HTTPCONTROL_H_
#define LIBNETWORK_HTTPCONTROL_H_

#include "ControlServer.h"
#include "ESP8266Wifi.h"

class HTTPControl: public ControlServer {
public:
  HTTPControl(ESP8266Wifi &wifi);
  virtual ~HTTPControl();

  void setDestination(const char *address, uint16_t port);
  void begin(uint16_t port=80);

  virtual bool updatePublications(uint8_t nPubReady, PublicationBase *readyPub[MAX_PUBLICATIONS]);
  virtual bool checkSubscriptions(char *sname, char *value);
  virtual bool publishAdvertise(const char * services);

private:
  char *_dest_address;
  uint16_t _dest_port;
  ESP8266Wifi &_wifi;
};

#endif /* LIBNETWORK_HTTPCONTROL_H_ */
