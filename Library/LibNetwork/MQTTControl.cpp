/*
 * MQTTControl.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include <MQTTControl.h>
#include "Packet.h"
#include "PublicationBase.h"

MQTTControl::MQTTControl(ESP8266Wifi &wifi) :
 _mqtt(wifi){
}

MQTTControl::~MQTTControl() {
}

void MQTTControl::setDestination(const char *address, uint16_t port) {
  _mqtt.setDestination(address, port);
}

void MQTTControl::begin() {
  _mqtt.begin();
}

bool MQTTControl::updatePublications(uint8_t nPubReady, PublicationBase *readyPub[MAX_PUBLICATIONS]) {
  char buff1[MAX_MESSAGE_LENGTH] = ""; //Must be able to contain data + header

  for(uint8_t iPub=0; iPub<nPubReady; ++iPub){
      Serial.print("Updating publication ");
      Serial.println(readyPub[iPub]->getName());
      readyPub[iPub]->to_string(buff1);
      _mqtt.publish(readyPub[iPub]->getName(), buff1);
      readyPub[iPub]->updated(false);
      delay(10);
  }

  return true;
}

bool MQTTControl::checkSubscriptions(char *sname, char *value) {
  return true;
}

bool MQTTControl::publishAdvertise(const char *services) {
  _mqtt.publish("advertise", services);
  delay(10);
  return true;
}
