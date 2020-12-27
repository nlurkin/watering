/*
 * PubServer.cpp
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#include "ControlServer.h"
#include "PublicationBase.h"
#include "DebugDef.h"

#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))

ControlServer::ControlServer() :
  _num_publications(0),
  _num_commands(0),
  _publications{nullptr},
  _commands{nullptr}
{
}

ControlServer::~ControlServer() {
}

bool ControlServer::addPublication(PublicationBase *pub) {
  if(_num_publications>=MAX_PUBLICATIONS)
    return false;

  _publications[_num_publications++] = pub;
  return true;
}

bool ControlServer::addCommand(PublicationBase *cmd) {
  if(_num_commands>=MAX_COMMANDS)
    return false;

  _commands[_num_commands++] = cmd;
  return true;
}

bool ControlServer::serve(bool force) {
  uint8_t nPubReady=0;
  PublicationBase *updatedPublications[MAX_PUBLICATIONS];
  for(uint8_t iPub=0; iPub<_num_publications; ++iPub){
    if(_publications[iPub]->isUpdated() || force){
      Serial.print("Publication ready: ");
      Serial.println(iPub);
      updatedPublications[nPubReady++] = _publications[iPub];
    }
  }

  if(nPubReady==0)
    return false;

  return updatePublications(nPubReady, updatedPublications);
}

bool ControlServer::listen() {
  char sname[PublicationBase::MAX_NAME_LENGTH+1];
  char value[30];

  if(!checkSubscriptions(sname, value))
    return false;

  for(uint8_t iCmd=0; iCmd<_num_commands; ++iCmd){
    if(strcmp(_commands[iCmd]->getName(), sname)==0){
      DEBUG_P("Command ready: ");
      DEBUG_PRAWLN(sname);
      _commands[iCmd]->from_string(value);
    }
  }
  return true;
}

bool ControlServer::advertise() {
  char buff[MAX_MESSAGE_LENGTH] = ""; //Must be able to contain data + header
  char* p = buff;
  for(uint8_t iPub=0; iPub<_num_publications; ++iPub){
    p = _publications[iPub]->def_string(p);
  }
  for(uint8_t iPub=0; iPub<_num_commands; ++iPub){
    p = _commands[iPub]->def_string(p);
  }

  return publishAdvertise(buff);
}
