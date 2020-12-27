/*
 * HTTPControl.cpp
 *
 *  Created on: 27 Dec 2020
 *      Author: Nicolas Lurkin
 */

#include <HTTPControl.h>

#include "HTTPRequest.h"
#include "HTTPControl.h"
#include "PublicationBase.h"
#include "DebugDef.h"

#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))

HTTPControl::HTTPControl(ESP8266Wifi &wifi) :
  _dest_address(nullptr),
  _dest_port(0),
  _wifi(wifi)
{
}

HTTPControl::~HTTPControl() {
  if(_dest_address)
    delete[] _dest_address;
  _dest_address = nullptr;
}

void HTTPControl::setDestination(const char *address, uint16_t port) {
  _dest_address = new char[strlen(address)+1];
  strcpy(_dest_address, address);
  _dest_port = port;
}

void HTTPControl::begin(uint16_t port){
  if(_wifi.startServer(port)){
    static const char message[] PROGMEM = {"Server started on port 80"};
    Serial.println(FPSTR(message));
  }
}

bool HTTPControl::updatePublications(uint8_t nPubReady, PublicationBase *readyPub[MAX_PUBLICATIONS]) {
  char buff1[MAX_MESSAGE_LENGTH] = ""; //Must be able to contain data + header
  char buff[MAX_MESSAGE_LENGTH+HTTPRequest::MAX_HEADER_LENGTH]; //Must be able to contain data + header

  int conn = -1;
  if(nPubReady>0)
    conn = _wifi.openConnection(_dest_address, _dest_port);
  for(uint8_t iPub=0; iPub<nPubReady; ++iPub){
      Serial.print("Updating publication ");
      Serial.println(readyPub[iPub]->getName());
      char path_buf[30] = "/api/v1/";
      strcpy(path_buf+8, readyPub[iPub]->getName());
      HTTPRequest r = HTTPRequest::http_post(path_buf);
      readyPub[iPub]->to_string(buff1);
      r.addContent(buff1);
      if(iPub<nPubReady-1)
        r.setConnectionType(HTTPRequest::CONN_KEEPALIVE);
      else
        r.setConnectionType(HTTPRequest::CONN_CLOSE);
      r.generate();
      r.getRawRequest(buff);

      if(_wifi.sendPacket(buff, conn))
          HTTPRequest::wait200OK(_wifi, conn);
      readyPub[iPub]->updated(false);
      delay(10);
  }
  if(conn!=-1)
    _wifi.closeConnection(conn);

  return true;
}

bool HTTPControl::checkSubscriptions(char *sname, char *value) {
  char buff[ESP8266Wifi::PAYLOAD_SIZE];
  const char* data;
  int8_t conn = _wifi.waitPayload(-1, buff, 1000);
  if(conn==-1)
      return false;

  HTTPRequest http(buff);
  #ifdef DEBUG
  http.print();
  #endif

  if(http.needs_answer()){
    HTTPRequest answer = HTTPRequest::http_200();
    answer.generate();
    answer.getRawRequest(buff);
    _wifi.sendPacket(buff, conn);
  }
  data = http.getData();
  DEBUG_P("Data received by CS:'");
  DEBUG_PRAW(data);
  DEBUG_PRAWLN("'");
  DEBUG_PLN(strstr_P(data, PSTR("/api/v1/")));
  if(strstr_P(data, PSTR("/api/v1/"))==data){
    uint16_t offset = 8;
    uint16_t urllen = strchr(data+offset, ':')-data-offset;
    uint16_t copylen = min(urllen, PublicationBase::MAX_NAME_LENGTH);
    strncpy(sname, data + offset, copylen);
    sname[copylen] = '\0';
    offset += copylen+1;
    strncpy(value, data + offset, 30);
    value[29] = '\0';
  }
  return true;
}

bool HTTPControl::publishAdvertise(const char *services) {
  int conn = _wifi.openConnection(_dest_address, _dest_port);
  if(conn==-1)
    return false;
  char buff[MAX_MESSAGE_LENGTH+HTTPRequest::MAX_HEADER_LENGTH] = ""; //Must be able to contain data + header
  HTTPRequest r = HTTPRequest::http_post("/api/v1/advertise");
  r.addContent(services);
  r.setConnectionType(HTTPRequest::CONN_CLOSE);
  r.generate();
  r.getRawRequest(buff);

  if(_wifi.sendPacket(buff, conn))
      HTTPRequest::wait200OK(_wifi, conn);
  delay(10);
  _wifi.closeConnection(conn);
  return true;
}
