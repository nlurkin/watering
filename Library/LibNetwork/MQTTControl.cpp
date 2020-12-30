/*
 * MQTTControl.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include "MQTTControl.h"
#include "Packet.h"
#include "PublicationBase.h"

MQTTControl::MQTTControl(ESP8266Wifi &wifi, const char *name) :
 _mqtt_owned(true),
 _mqtt(new MQTTClient(wifi, name))
{
  _name = new char[min(strlen(name)+1, PublicationBase::MAX_NAME_LENGTH+1)];
  strncpy(_name, name, PublicationBase::MAX_NAME_LENGTH);
  _name[PublicationBase::MAX_NAME_LENGTH] = '\0';
}

MQTTControl::MQTTControl(MQTTClient &mqtt, const char *name) :
 _mqtt_owned(false),
 _mqtt(&mqtt)
{
  _name = new char[min(strlen(name)+1, PublicationBase::MAX_NAME_LENGTH+1)];
  strcpy(_name, name);
  _name[PublicationBase::MAX_NAME_LENGTH] = '\0';
}

MQTTControl::~MQTTControl() {
  if(_mqtt && _mqtt_owned){
    delete _mqtt;
    _mqtt = nullptr;
  }
  if(_name)
    delete[] _name;
  _name = nullptr;
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
  char pubname[PublicationBase::MAX_NAME_LENGTH*2];

  for(uint8_t iPub=0; iPub<nPubReady; ++iPub){
      Serial.print("Updating publication ");
      readyPub[iPub]->to_string(buff1);
      strcpy(pubname, _name);
      pubname[strlen(_name)] = '/';
      strcpy(pubname + (strlen(_name)+1), readyPub[iPub]->getName());
      Serial.println(pubname);
      _mqtt->publish(pubname, buff1);
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

  char pubname[PublicationBase::MAX_NAME_LENGTH*2];
  strcpy(pubname, _name);
  strcpy_P(pubname + strlen(_name), PSTR("/cmd/"));
  strcpy(pubname + (strlen(_name)+5), cmd->getName());
  return _mqtt->subscribe(pubname);
}

bool MQTTControl::publishAdvertise(const char *services) {
  char pubname[PublicationBase::MAX_NAME_LENGTH*2];
  strcpy(pubname, _name);
  pubname[strlen(_name)] = '/';
  strcpy_P(pubname + (strlen(_name)+1), PSTR("advertise"));
  _mqtt->publish(pubname, services);
  delay(10);
  return true;
}
