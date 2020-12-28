/*
 * MQTTControl.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include "MQTTControl.h"
#include "Packet.h"
#include "PublicationBase.h"

MQTTControl::MQTTControl(ESP8266Wifi &wifi) :
 _mqtt_owned(true),
 _mqtt(new MQTTClient(wifi))
{
}

MQTTControl::MQTTControl(MQTTClient &mqtt) :
 _mqtt_owned(false),
 _mqtt(&mqtt)
{
}

MQTTControl::~MQTTControl() {
  if(_mqtt && _mqtt_owned){
    delete _mqtt;
    _mqtt = nullptr;
  }
}

void MQTTControl::setDestination(const char *address, uint16_t port) {
  _mqtt->setDestination(address, port);
}

void MQTTControl::begin() {
  _mqtt->begin();
  _mqtt->connect();
}

bool MQTTControl::updatePublications(uint8_t nPubReady, PublicationBase *readyPub[MAX_PUBLICATIONS]) {
  char buff1[MAX_MESSAGE_LENGTH] = "";

  for(uint8_t iPub=0; iPub<nPubReady; ++iPub){
      Serial.print("Updating publication ");
      Serial.println(readyPub[iPub]->getName());
      readyPub[iPub]->to_string(buff1);
      _mqtt->publish(readyPub[iPub]->getName(), buff1);
      readyPub[iPub]->updated(false);
      delay(10);
  }

  return true;
}

bool MQTTControl::checkSubscriptions(char *sname, char *value) {
  return _mqtt->listen(sname, value);
}

bool MQTTControl::addCommand(PublicationBase *cmd) {
  if(!ControlServer::addCommand(cmd))
    return false;

  return _mqtt->subscribe(cmd->getName());
}

bool MQTTControl::publishAdvertise(const char *services) {
  _mqtt->publish("advertise", services);
  delay(10);
  return true;
}
