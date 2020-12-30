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

  for(size_t i=0; i<MAX_COMMANDS; ++i){
    _cmd_msg_id[i] = 0;
  }
}

MQTTControl::MQTTControl(MQTTClient &mqtt, const char *name) :
 _mqtt_owned(false),
 _mqtt(&mqtt)
{
  _name = new char[min(strlen(name)+1, PublicationBase::MAX_NAME_LENGTH+1)];
  strcpy(_name, name);
  _name[PublicationBase::MAX_NAME_LENGTH] = '\0';

  for(size_t i=0; i<MAX_COMMANDS; ++i){
    _cmd_msg_id[i] = 0;
  }
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
  // Check subscriptions succeeded
  for(uint8_t i=0; i<get_num_commands(); ++i){
    if(_cmd_msg_id[i]!=0){
      if(_mqtt->id_used(_cmd_msg_id[i])){
        //subscription not succeeded, redo it
        _mqtt->free_id(_cmd_msg_id[i]);
        requestSubscription(get_publication(i), i);
      }
      else{
        // Ack packet received. Subscription successful
        _cmd_msg_id[i] = 0;
      }
    }
  }

  // Check subscriptions updates
  char pubname[PublicationBase::MAX_NAME_LENGTH*2];
  bool ready = _mqtt->listen(pubname, value);

  if(ready)
    strcpy(sname, pubname+strlen(_name)+5);
  return ready;
}

bool MQTTControl::addCommand(PublicationBase *cmd) {
  if(!ControlServer::addCommand(cmd))
    return false;

  requestSubscription(cmd, get_num_publications());
  return true;
}

void MQTTControl::requestSubscription(PublicationBase *cmd, uint8_t pubnum) {
  char pubname[PublicationBase::MAX_NAME_LENGTH*2];
  strcpy(pubname, _name);
  strcpy_P(pubname + strlen(_name), PSTR("/cmd/"));
  strcpy(pubname + (strlen(_name)+5), cmd->getName());
  _cmd_msg_id[pubnum] = _mqtt->subscribe(pubname);
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
