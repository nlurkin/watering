/*
 * ATClient.cpp
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ATClient.h"
#include "DebugDef.h"

ATClient::ATClient(Stream* serial) :
  _set_default(false),
  _last_error(NO_ERROR),
  _timeout(5000),
  _atSerial(serial),
  _logSerial(&Serial),
  _dataCapture(DATA_BUFFER_SIZE)
#ifdef BUFFERED
  ,_buffer(BUFFER_SIZE)
#endif
{
}

ATClient::~ATClient() {
}

void ATClient::setLogSerial(Stream *serial){
  _logSerial = serial;
}

bool ATClient::sendCommand(const char *cmd) {
  _dataCapture.clear();
  DEBUGS_P((*_logSerial), F("Sending: "));
  DEBUGS_PRAWLN((*_logSerial), cmd);
  _atSerial->println(cmd);
  return true;
}

bool ATClient::sendCommand(const __FlashStringHelper* cmd) {
  _dataCapture.clear();
  DEBUGS_P((*_logSerial), F("Sending: "));
  DEBUGS_PRAWLN((*_logSerial), cmd);
  _atSerial->println(cmd);
  return true;
}

bool ATClient::sendData(const char *data, size_t size) {
  _dataCapture.clear();
  DEBUGS_P((*_logSerial), F("Sending data: "));
  DEBUGS_PRAW((*_logSerial), size);
  DEBUGS_PRAWLN((*_logSerial), data);
  DEBUGS_PLN((*_logSerial), F("--- end data ---"));
  if(size==0)
      _atSerial->println(data);
    else
      _atSerial->write(data, size);
  return true;
}

bool ATClient::sendDataConfirm(const char *data, size_t size) {
  _dataCapture.clear();
  DEBUGS_P((*_logSerial), F("Sending data: "));
  DEBUGS_PRAW((*_logSerial), size);
  DEBUGS_PRAWLN((*_logSerial), data);
  DEBUGS_PLN((*_logSerial), F("--- end data ---"));
  if(size==0)
    _atSerial->println(data);
  else
    _atSerial->write(data, size);
  return waitMessage(F("SEND OK"));
}

size_t ATClient::readUntil(char * to, size_t max, const char c_search, unsigned int timeout) {
#ifdef BUFFERED
  if(transferBuffer()==0){
    *to = '\0';
    return 0;
  }

  return _buffer.get(to, max, c_search);
#else
  unsigned int oldto;
  if(timeout!=1000){
    oldto = _atSerial->getTimeout();
    _atSerial->setTimeout(timeout);
  }
  size_t len = _atSerial->readBytesUntil(c_search, to, max-1);
  if(timeout!=1000)
    _atSerial->setTimeout(oldto);
  if(len>0){
    to[len++] = '\0';
  }
  else
    to[0] = '\0';
  return len;
#endif
}

size_t ATClient::readRaw(char * to, size_t max) {
#ifdef BUFFERED
  if(transferBuffer()==0){
    *to = '\0';
    return 0;
  }

  return _buffer.get(to, max);
#else
  size_t avail=_atSerial->available();
  char *ptr = to;
  uint8_t iter=0;
  while(avail>0 && max>0){
      size_t len = _atSerial->readBytes(ptr, min(avail, max));
      ptr += len;
      avail=_atSerial->available();
      max -= len;
      iter=0;
      while(avail==0 && max>0 && iter++<10){
          delay(1);
          avail=_atSerial->available();
      }
  }

  *ptr = '\0';
  return ptr-to;
#endif
}

size_t ATClient::getLastData(char *to, size_t max) {
  return _dataCapture.get(to, max);
}

size_t ATClient::copyLastData(char *to, size_t max) {
  return _dataCapture.copyContent(to, max);
}

size_t ATClient::dataAvailable() const {
  return _dataCapture.len();
}

// ######################## General commands
bool ATClient::AT() {
  sendCommand(F("AT"));
  return checkAnswer(F("AT"));
}

bool ATClient::RST() {
  sendCommand(F("AT+RST"));
  bool success = checkAnswer(F("AT+RST"));
  if(!success)
    return false;

  return waitMessage(F("ready"));
}

bool ATClient::GMR() {
  sendCommand(F("AT+GMR"));
  return checkAnswer(F("AT+GMR"));
}


bool ATClient::SLEEP(uint8_t mode) {
  if(mode>2)
    return false;
  char cmd[15]; //11
  sprintf_P(cmd, PSTR("AT+SLEEP=%u"), mode);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

// ######################## WIFI commands
bool ATClient::CWMODE(uint8_t mode) {
  if(mode>3 || mode<1)
    return false;

  char cmd[20]; //16
  if(_set_default) // Store in flash
    sprintf_P(cmd, PSTR("AT+CWMODE_DEF=%u"), mode);
  else //Only temporary
    sprintf_P(cmd, PSTR("AT+CWMODE_CUR=%u"), mode);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CWJAP(const char *ssid, const char *passwd) {
  if(strlen(ssid)+strlen(passwd)>60) // Avoid buffer overflow
    return false;

  char cmd[80]; //19 char without ssid and passwd
  if(_set_default) // Store in flash
    sprintf_P(cmd, PSTR("AT+CWJAP_DEF=\"%s\",\"%s\""), ssid, passwd);
  else //Only temporary
    sprintf_P(cmd, PSTR("AT+CWJAP_CUR=\"%s\",\"%s\""), ssid, passwd);

  sendCommand(cmd);
  unsigned long old_to = _timeout;
  _timeout = 20000;
  bool ans = checkAnswer(cmd);
  _timeout = old_to;
  return ans;
}

bool ATClient::CWLAP() {
  sendCommand(F("AT+CWLAP"));
  return checkAnswer(F("AT+CWLAP"));
}

bool ATClient::CWQAP() {
  sendCommand(F("AT+CWQAP"));
  return checkAnswer(F("AT+CWQAP"));
}

bool ATClient::CWSAP(const char *ssid, const char *passwd, uint8_t channel, uint8_t ecn) {
  if(ecn>4 || ecn==1) //Can be only 0,2,3,4
    return false;
  if(strlen(ssid)+strlen(passwd)>60) // Avoid buffer overflow
    return false;

  char cmd[85]; //23 without ssid and passwd
  if(_set_default) // Store in flash
    sprintf_P(cmd, PSTR("AT+CWSAP_DEF=\"%s\",\"%s\",%u,%u"), ssid, passwd, channel, ecn);
  else //Only temporary
    sprintf_P(cmd, PSTR("AT+CWSAP_CUR=\"%s\",\"%s\",%u,%u"), ssid, passwd, channel, ecn);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CWLIF() {
  sendCommand(F("AT+CWLIF"));
  return checkAnswer(F("AT+CWLIF"));
}

bool ATClient::CWDHCP(bool en, uint8_t mode) {
  if(mode>2)
    return false;

  char cmd[20]; //18
  if(_set_default) // Store in flash
    sprintf_P(cmd, PSTR("AT+CWDHCP_DEF=%d,%u"), en, mode);
  else //Only temporary
    sprintf_P(cmd, PSTR("AT+CWDHCP_CUR=%d,%u"), en, mode);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CWAUTOCONN(bool en) {
  char cmd[20]; //16
  sprintf_P(cmd, PSTR("AT+CWAUTOCONN=%d"), en);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPSTAMAC(uint8_t mac[6]) {
  char cmd[40]; //37
  if(_set_default) // Store in flash
    sprintf_P(cmd, PSTR("AT+CIPSTAMAC_DEF=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  else //Only temporary
    sprintf_P(cmd, PSTR("AT+CIPSTAMAC_CUR=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPAPMAC(uint8_t mac[6]) {
  char cmd[40]; //36
  if(_set_default) // Store in flash
    sprintf_P(cmd, PSTR("AT+CIPAPMAC_DEF=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  else //Only temporary
    sprintf_P(cmd, PSTR("AT+CIPAPMAC_CUR=\"%X:%X:%X:%X:%X:%X\""), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPSTA(uint8_t ip[4], uint8_t gtw[4], uint8_t netmask[4]) {
  if(netmask!=nullptr && gtw==nullptr)
    return false; //if netmask is set, gtw must be set too

  char cmd[85]; //83
  int len = 0;
  if(_set_default) // Store in flash
    len += sprintf_P(cmd, PSTR("AT+CIPSTA_DEF=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);
  else //Only temporary
    len += sprintf_P(cmd, PSTR("AT+CIPSTA_CUR=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);

  if(gtw!=nullptr)
    len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), gtw[0], gtw[1], gtw[2], gtw[3]);
  if(netmask!=nullptr)
    len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), netmask[0], netmask[1], netmask[2], netmask[3]);

  sendCommand(cmd);
  return checkAnswer(cmd);

}

bool ATClient::CIPAP(uint8_t ip[4], uint8_t gtw[4], uint8_t netmask[4]) {
  if( !(netmask==nullptr && gtw==nullptr) || !(netmask!=nullptr && gtw!=nullptr) )
    return false; //must both be set or nullptr

  char cmd[85]; //82
  int len = 0;
  if(_set_default) // Store in flash
    len += sprintf_P(cmd, PSTR("AT+CIPAP_DEF=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);
  else //Only temporary
    len += sprintf_P(cmd, PSTR("AT+CIPAP_CUR=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);

  if(gtw!=nullptr)
    len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), gtw[0], gtw[1], gtw[2], gtw[3]);
  if(netmask!=nullptr)
    len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\""), netmask[0], netmask[1], netmask[2], netmask[3]);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::WPS(bool en){
  char cmd[10]; //8
  sprintf_P(cmd, PSTR("AT+WPS=%d"), en);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

// ######################## TCP/IP commands
bool ATClient::CIPSTATUS() {
  sendCommand(F("AT+CIPSTATUS"));
  return checkAnswer(F("AT+CIPSTATUS"));
}

bool ATClient::CIPSTART(TCP_TYPE type, uint8_t ip[4], uint16_t port, int8_t link_id, int32_t udp_port, uint8_t udp_mode, int keepalive) {
  if(udp_port!=-1 && type!=UDP) //udp_port and udp_mode valid only in UDP type
    return false;
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;
  if(udp_mode>2)
    return false;
  if(keepalive!=-1 && keepalive>7200)
    return false;
  if(udp_port>65535) //Maximum port number
    return false;

  char cmd[60]; //57
  strcpy_P(cmd, PSTR("AT+CIPSTART="));
  int len = strlen(cmd);
  if(link_id!=-1){
    len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
  }
  if(type==TCP)
    strcat_P(cmd, PSTR("\"TCP\","));
  else if(type==UDP)
    strcat_P(cmd, PSTR("\"UDP\","));
  len = strlen(cmd);

  len += sprintf_P(cmd+len, PSTR("\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);

  if(udp_port!=-1)
    len += sprintf_P(cmd+len, PSTR(",%d,%u"), udp_port, udp_mode);
  if(keepalive!=-1)
    len += sprintf_P(cmd+len, PSTR(",%d"), keepalive);

  sendCommand(cmd);
  bool ans = checkAnswer(cmd);
  if(!ans){
      int at = _dataCapture.containsAt("ALREADY CONNECTED");
      Serial.println(at);
      return at!=-1;
  }
  return ans;
}

bool ATClient::CIPSTART(TCP_TYPE type, const char *address, uint16_t port, int8_t link_id, int32_t udp_port, uint8_t udp_mode, int keepalive) {
  if(udp_port!=-1 && type!=UDP) //udp_port and udp_mode valid only in UDP type
    return false;
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;
  if(udp_mode>2)
    return false;
  if(keepalive!=-1 && keepalive>7200)
    return false;
  if(udp_port>65535) //Maximum port number
    return false;
  if(strlen(address)>105) //Max address size to avoid buffer overflow
    return false;

  char cmd[150]; //42 without the address
  strcpy_P(cmd, PSTR("AT+CIPSTART="));
  int len = strlen(cmd);
  if(link_id!=-1)
    len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
  if(type==TCP)
    strcat_P(cmd, PSTR("\"TCP\","));
  else if(type==UDP)
    strcat_P(cmd, PSTR("\"UDP\","));

  len = strlen(cmd);
  len += sprintf_P(cmd+len, PSTR("\"%s\",%d"), address, port);
  if(udp_port!=-1)
    len += sprintf_P(cmd+len, PSTR(",%d,%u"), port, udp_mode);
  if(keepalive!=-1)
    len += sprintf_P(cmd+len, PSTR(",%d"), keepalive);

  sendCommand(cmd);
  bool ans = checkAnswer(cmd);
  if(!ans){
    int at = _dataCapture.containsAt("ALREADY CONNECTED");
    if(at!=-1){
      // Already opened
      Serial.println(at);
      return true;
    }
    at = _dataCapture.containsAt("Link type ERROR");
    if(at!=-1){
      // Probably not the correct mux
      Serial.println(at);
      _last_error = LINK_TYPE;
      return false;
    }

  }
  return ans;
}

bool ATClient::CIPSEND(const char *data, int link_id, uint8_t ip[4], int32_t port) {
  if(ip!=nullptr && port==-1) //If ip is set, port must be set
    return false;
  if(ip==nullptr && port!=-1) //Port cannot be set if ip is not
    return false;
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;
  if(port>65535) //Maximum port number
    return false;
  uint16_t datas = strlen(data);
  if(datas>2048) //Maximum size of a single transmission
    return false;

  char cmd[45]; //42
  strcpy_P(cmd, PSTR("AT+CIPSEND="));
  int len = strlen(cmd);
  if(link_id!=-1)
    len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
  len += sprintf_P(cmd+len, PSTR("%u"), datas);
  if(ip!=nullptr)
    len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);

  sendCommand(cmd);
  bool success = waitMessage(F(">"));
  if(!success)
    return false;

  unsigned long old_to = _timeout;
  sendData(data);
  _timeout = 5000;
  bool ans = waitMessage(F("SEND OK"));
  _timeout = old_to;
  return ans;
}

bool ATClient::CIPSENDEX(uint16_t length, int link_id, uint8_t ip[4], int32_t port) {
  if(ip!=nullptr && port==-1) //If ip is set, port must be set
    return false;
  if(ip==nullptr && port!=-1) //Port cannot be set if ip is not
    return false;
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;
  if(port>65535) //Maximum port number
    return false;
  if(length>2048) //Maximum size of a single transmission
    return false;

  char cmd[45]; //42
  strcpy_P(cmd, PSTR("AT+CIPSEND="));
  int len = strlen(cmd);
  if(link_id!=-1)
    len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
  len += sprintf_P(cmd+len, PSTR("%u"), length);
  if(ip!=nullptr)
    len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);

  sendCommand(cmd);
  return waitMessage(F(">"));
}

bool ATClient::CIPSENDBUF(const char *data, uint8_t &/*bufferNr*/, int link_id) {
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;
  uint16_t datas = strlen(data);
  if(datas>2048) //Maximum size of a single transmission
    return false;

  char cmd[25]; //21
  strcpy_P(cmd, PSTR("AT+CIPSENDBUF="));
  int len = strlen(cmd);
  if(link_id!=-1)
    len += sprintf_P(cmd+len, PSTR("%d,"), link_id);
  len += sprintf_P(cmd+len, PSTR("%u"), datas);

  sendCommand(cmd);
  bool success = waitMessage(F(">"));
  if(!success)
    return false;
  sendData(data);

  //Principle of buffer send: do not wait. Check later what was done (CIPBUFSTATUS)
  //success = waitMessage("SEND OK", true);
  //if(!success)
  //  return false;
    //
  //if(_lastDataSize!=1)
  //  return false;
    //
  //String resp;
  //if(link_id==-1)
  //  resp = _lastData[0].substring(0, _lastData[0].length()-8);
  //else
  //  resp = _lastData[0].substring(2, _lastData[0].length()-8);
  //bufferNr = resp.toInt();

  return true;
}

bool ATClient::CIPBUFSTATUS(int8_t link_id) {
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;

  char cmd[20]; //18
  strcpy_P(cmd, PSTR("AT+CIPBUFSTATUS"));
  if(link_id!=-1)
    sprintf_P(cmd+strlen(cmd), PSTR("=%u"), link_id);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPCHECKSEQ(uint8_t segment, int8_t link_id) {
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;
  if(segment>=1000)
    return false;

  char cmd[25]; //22
  strcpy_P(cmd, PSTR("AT+CIPCHECKSEQ="));
  int len = strlen(cmd);
  if(link_id!=-1)
    len += sprintf_P(cmd+len, PSTR("%u,"), link_id);
  sprintf_P(cmd+len, PSTR("%u"), segment);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPBUFRESET(int8_t link_id) {
  if(link_id!=-1 && link_id>4) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here)
    return false;

  char cmd[20]; //17
  strcpy_P(cmd, PSTR("AT+CIPBUFRESET"));
  if(link_id!=-1)
    sprintf_P(cmd+strlen(cmd), PSTR("=%u"), link_id);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPCLOSE(int8_t link_id) {
  if(link_id!=-1 && link_id>5) //Maximum 4 links in CIPMUX=1, if CIPMUX=0, must be -1 (we do not check ourselves here). 5 is special, close all connections
    return false;

  char cmd[15]; //14
  strcpy_P(cmd, PSTR("AT+CIPCLOSE"));
  if(link_id!=-1)
    sprintf_P(cmd+strlen(cmd), PSTR("=%u"), link_id);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIFSR() {
  sendCommand(F("AT+CIFSR"));
  return checkAnswer(F("AT+CIFSR"));
}

bool ATClient::CIPMUX(bool mode) {
  char cmd[15]; //12
  sprintf_P(cmd, PSTR("AT+CIPMUX=%d"), mode);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPSERVER(bool on, int32_t port) {
  if(port>65535) //Maximum port number
    return false;

  char cmd[25]; //21
  int len = sprintf_P(cmd, PSTR("AT+CIPSERVER=%d"), on);
  if(on && port>=0)
    sprintf_P(cmd+len, PSTR(",%d"), port);

  sendCommand(cmd);
  return checkAnswer(cmd);
}


bool ATClient::CIPSAVETRANSLINK(bool on, uint8_t ip[4], int32_t port, TCP_TYPE type, int keepalive, int32_t udp_port) {
  if(udp_port!=-1 && type!=UDP) //udp_port and udp_mode valid only in UDP type
    return false;
  if(keepalive!=-1 && keepalive>7200)
    return false;
  if(on && ip==nullptr)
    return false;
  if(ip!=nullptr && port==-1)
    return false;
  if(port>65535) //Maximum port number
    return false;
  if(udp_port>65535) //Maximum port number
    return false;

  char cmd[65]; //63
  int len = sprintf_P(cmd, PSTR("AT+CIPSAVETRANSLINK=%d"), on);
  if(on)
    len += sprintf_P(cmd+len, PSTR(",\"%u.%u.%u.%u\",%d"), ip[0], ip[1], ip[2], ip[3], port);
  if(type==TCP)
    strcat_P(cmd, PSTR(",\"TCP\""));
  else if(type==UDP)
    strcat_P(cmd, PSTR(",\"UDP\""));
  len = strlen(cmd);
  if(keepalive>=0)
    len += sprintf_P(cmd+len, PSTR(",%d"), keepalive);
  if(udp_port!=-1)
    len += sprintf_P(cmd+len, PSTR(",%d"), udp_port);

  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPSTO(int keepalive) {
  if(keepalive<0 || keepalive>7200)
    return false;

  char cmd[20]; //15
  sprintf_P(cmd, PSTR("AT+CIPSTO=%d"), keepalive);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::PINGA(const char *address) {
  if(strlen(address)>105) //Max address size to avoid buffer overflow
    return false;

  char cmd[120]; //11 without the address
  sprintf_P(cmd, PSTR("AT+PING=\"%s\""), address);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::PINGA(uint8_t ip[4]) {
  char cmd[30]; //26
  sprintf_P(cmd, PSTR("AT+PING=\"%u.%u.%u.%u\""), ip[0], ip[1], ip[2], ip[3]);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

bool ATClient::CIPDINFO(bool on) {
  char cmd[15]; //14
  sprintf_P(cmd, PSTR("AT+CIPDINFO=%d"), on);
  sendCommand(cmd);
  return checkAnswer(cmd);
}

//######################## PRIVATE ##################
#ifdef BUFFERED
size_t ATClient::transferBuffer() {
  while (_atSerial->available() > 0)
    _buffer.push(_atSerial->read());

  return _buffer.len();
}
#endif

size_t ATClient::waitData(size_t length) {
  unsigned long start_time = millis();
  size_t available = 0;
  do {
#ifdef BUFFERED
    available = transferBuffer();
#else
    available = _atSerial->available();
#endif
  }
  while( (available<length) && ((millis()-start_time) < _timeout) );

  return available;
}

char ATClient::read() {
#ifdef BUFFERED
  if(transferBuffer()==0)
    return '\0';

  return _buffer.read();
#else
  return _atSerial->read();
#endif
}

bool ATClient::checkAnswer(const char *command) {
  if(!waitMessage(command))
    return false;
  return waitMessage(F("OK"));
}

bool ATClient::checkAnswer(const __FlashStringHelper* command) {
  if(!waitMessage(command))
    return false;
  return waitMessage(F("OK"));
}

bool ATClient::waitMessage(const char *message) {
  bool got_first_char = false;
  bool got_message = false;
  size_t str_len = strlen(message);
  unsigned long start_time = millis();
  size_t available = waitData(str_len);

#ifndef BUFFERED
  // The buffer to contain temporary data matching the message is shorter than the message.
  // We assume that there will be no string longer than 20 characters coming from the _atSerial
  // where the first 20 characters are identical to the message, but with differences after.
  // If the assumption is wrong, we will lose the last characters.
  char buff[20];
  char c;
  size_t pos = 0;
#endif

  // Loop while we have enough data in buffer and while we are not in timeout
  while( available>0 && ((millis()-start_time) < _timeout) ){
#ifdef BUFFERED
    if(_buffer.startsWith(message)) { //If the buffer starts with what we are looking for, we are done
      got_message = true;
      _buffer.drop(str_len); //Make sure the buffer removes the message we were looking for
      break;
    }
    // Else at least the first character is wrong and we should add it to the captured data
    _dataCapture.push(_buffer.read());
    // Then wait for at least as much data as we need
    available = waitData(str_len);
#else
    c = read();
    DEBUGS_PRAW((*_logSerial), c);
    if(!got_first_char && (c=='\r' || c=='\n')){
      available = waitData(str_len); // One of the char was empty space, need to wait for the full message
      continue;
    }
    if(c==message[pos]) { // Current char is okay
      if(pos<20) //Do not overflow
        buff[pos] = c;
      ++pos;
      if(pos==str_len){ // This is the end of the message. We have it.
        got_message = true;
        break;
      }
    }
    else{ //We might have had a bit of string looking like the message, but it is not it
      for(size_t i=0; i<pos && i<20; ++i)
        _dataCapture.push(buff[i]); // Fill the data capture with all the characters we thought were part of the message
      _dataCapture.push(c); // Add the last character that is not like the message
      pos = 0; //Restart from the beginning of the message
      // Then wait for at least as much data as we need
      available = waitData(str_len);
    }
    //No need to wait here, we had enough data and we didn't finish going through
#endif
  }
  #ifdef DEBUG
  _dataCapture.print();
  #endif

  return got_message;
}

bool ATClient::waitMessage(const __FlashStringHelper* message) {
  bool got_message = false;
  bool got_first_char = false;
  size_t str_len = strlen_P(reinterpret_cast<PGM_P>(message));
  unsigned long start_time = millis();
  size_t available = waitData(str_len);

#ifndef BUFFERED
  // The buffer to contain temporary data matching the message is shorter than the message.
  // We assume that there will be no string longer than 20 characters coming from the _atSerial
  // where the first 20 characters are identical to the message, but with differences after.
  // If the assumption is wrong, we will lose the last characters.
  char buff[20];
  char c;
  size_t pos = 0;
  PGM_P p_msg = reinterpret_cast<PGM_P>(message);
  char c_msg = pgm_read_byte(p_msg);
#endif

  // Loop while we have enough data in buffer and while we are not in timeout
  while( available>0 && ((millis()-start_time) < _timeout) ){
#ifdef BUFFERED
    if(_buffer.startsWith(message)) { //If the buffer starts with what we are looking for, we are done
      got_message = true;
      _buffer.drop(str_len); //Make sure the buffer removes the message we were looking for
      break;
    }
    // Else at least the first character is wrong and we should add it to the captured data
    _dataCapture.push(_buffer.read());
    // Then wait for at least as much data as we need
    available = waitData(str_len);
#else
    c = read();
    DEBUGS_PRAW((*_logSerial), c);
    if(!got_first_char && (c=='\r' || c=='\n')){
      available = waitData(str_len);
      continue;
    }
    got_first_char = true;
    if(c==c_msg) { // Current char is okay
      if(pos<20) //Do not overflow
        buff[pos] = c;
      ++pos;
      if(pos==str_len){ // This is the end of the message. We have it.
        got_message = true;
        break;
      }
      c_msg = pgm_read_byte(p_msg+pos);
    }
    else{ //We might have had a bit of string looking like the message, but it is not it
      for(size_t i=0; i<pos && i<20; ++i)
        _dataCapture.push(buff[i]); // Fill the data capture with all the characters we thought were part of the message
      _dataCapture.push(c); // Add the last character that is not like the message
      pos = 0; //Restart from the beginning of the message
      c_msg = pgm_read_byte(p_msg);
      // Then wait for at least as much data as we need
      available = waitData(str_len);
    }
    //No need to wait here, we had enough data and we didn't finish going through
#endif
  }
  #ifdef DEBUG
  _dataCapture.print();
  #endif
  return got_message;
}
