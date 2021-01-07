/*
 * ESP8266Wifi.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "ESP8266Wifi.h"
#include "DebugDef.h"

//\TODO add checkDataCapture possibly after each command

static const char g_SEP_NEWLINE[] PROGMEM = {"\n"};
static const char g_SEP_DOT[]     PROGMEM = {"."};
static const char g_SEP_COLUMN[]  PROGMEM = {":"};

Buffer ESP8266Wifi::_persistent_buffer(ESP8266Wifi::PAYLOAD_SIZE);

ESP8266Wifi::ESP8266Wifi(Stream* serial) :
  _conn_opened{false,false,false,false,false},
  _ip_address{0,0,0,0},
  _mac_address{0,0,0,0,0,0},
  _payload{&_persistent_buffer, nullptr, nullptr, nullptr, nullptr},
  _client(serial),
  _logSerial(&Serial)
{
}

ESP8266Wifi::~ESP8266Wifi() {
  for(uint8_t i=1; i<4; i++){
    if(_payload[i])
      delete _payload[i];
    _payload[i] = nullptr;
  }
}

bool ESP8266Wifi::init(const char* ssid, const char* password, bool doReset, bool debug) {
  if(debug)
    Serial.println(F("Checking ESP8266 connection..."));
  while (!checkBoardConnection())
    delay(100);
  if(debug)
    Serial.println(F("Connection established"));

  delay(100);
  if(doReset){
    restartBoard();
    delay(2000);
  }
  if(debug)
    fw_version();

  uint8_t trials = 0;
  static const uint8_t max_trials = 10;
  while (!checkWifiConnection() && trials < max_trials) {
    ++trials;
    connectWifi(ssid, password);
    delay(1000);
    if(trials==max_trials){
      if(debug)
        Serial.println(F("Unable to connect to wifi"));
      return false;
    }
  }
  if(debug){
    Serial.println(F("Connected to wifi"));
    printMacAddress();
    printIPAddress();
  }
  return true;
}

void ESP8266Wifi::setLogSerial(Stream *serial){
  _logSerial = serial;
  //_client.setLogSerial(_logSerial);
}

bool ESP8266Wifi::sendSomething(const char *cmd) const {
  if(cmd[0]=='&')
    return _client.sendData(cmd+1);
  else if(cmd[0]=='+' && cmd[1]=='+' && cmd[1]=='+')
    return Serial1.print("+++");
  else
    return _client.sendCommand(cmd);
}

bool ESP8266Wifi::sendCommand(const char *cmd) {
  return checkBuffAndReturn(_client.sendCommand(cmd));
}

bool ESP8266Wifi::sendData(const char *data) {
  return checkBuffAndReturn(_client.sendData(data));
}

bool ESP8266Wifi::readAndPrint(unsigned int timeout) {
  static constexpr size_t read_size = 100;
  char response[read_size];
  //delay(10); // Give time to actually fill the buffer. Else we will most likely have only 1 char
  size_t len = _client.readUntil(response, read_size, '\n', timeout);
  bool has_response = len > 0;

  if (has_response) {
    DEBUGS_PLN((*_logSerial), F("Response Received:"));
    while (len > 0) {
      DEBUGS_P((*_logSerial), F("> "));
      DEBUGS_PRAWLN((*_logSerial), response);
      if(startsWith(response, F("+IPD"))){ //+IPD
        read_payload(response, len);
      }
      else if(endsWith(response, F("CONNECT\r")))
        new_connection(response);
      else if(endsWith(response, F("CLOSED\r")))
        end_connection(response);
      else if(endsWith(response, F("WIFI DISCONNECT\r")))
        disconnect();
      len = _client.readUntil(response, read_size, '\n');
    }

    DEBUGS_PRAWLN((*_logSerial), "");
    DEBUGS_PRAWLN((*_logSerial), F("============"));
    DEBUGS_PRAWLN((*_logSerial), "");
  }
  return has_response;
}

bool ESP8266Wifi::checkDataCapture() {
  if(_client.dataAvailable()==0)
    return false;

  char response[ATClient::DATA_BUFFER_SIZE];
  size_t len = _client.copyLastData(response, _client.dataAvailable());

  bool has_response = false;
  //At the moment deal only with +IPD
  DEBUGS_PLN((*_logSerial), F("Response Received:"));
  DEBUGS_P((*_logSerial), F("> "));
  DEBUGS_PRAWLN((*_logSerial), response);
  char * ipd = strstr_P(response, PSTR("+IPD"));
  char * connect = strstr_P(response, PSTR("CONNECT\r"));
  char * close = strstr_P(response, PSTR("CLOSED\r"));
  char * wifi = strstr_P(response, PSTR("WIFI DISCONNECT\r"));
  if(ipd){
    read_payload(ipd, len-(ipd-response));
    has_response = true;
  }
  if(connect){
    new_connection(connect-2);
    has_response = true;
  }
  if(close){
    Serial.println("Closed connection");
    end_connection(close-2);
    has_response = true;
  }
  if(wifi){
    disconnect();
    has_response = true;
  }

  DEBUGS_PRAWLN((*_logSerial), "");
  DEBUGS_PRAWLN((*_logSerial), F("============"));
  DEBUGS_PRAWLN((*_logSerial), "");
  return has_response;
}

bool ESP8266Wifi::checkBoardConnection() const {
  return _client.AT();
}

bool ESP8266Wifi::checkWifiConnection() {
  bool success = checkBuffAndReturn(_client.CIFSR());

  if(!success)
    return false;

  char data[ATClient::DATA_BUFFER_SIZE];
  char ip_line[16] = {'\0'}; //3*4+3+(1 null)
  char mac_line[18] = {'\0'}; //6*2+5+(1 null)
  char *ptr;
  _client.getLastData(data, ATClient::DATA_BUFFER_SIZE);

  ptr = strtok_P(data, g_SEP_NEWLINE);
  while(ptr!=nullptr){
    if(!ip_line[0] && strstr_P(ptr, PSTR("+CIFSR:STAIP"))==ptr) { // This is the ip address line (and we have not got it yet)
      strncpy(ip_line, ptr+14, 15); //Copy the ip address
      ip_line[15] = '\0';
    }
    if(!mac_line[0] && strstr_P(ptr, PSTR("+CIFSR:STAMAC"))==ptr) { // This is the mac address line (and we have not got it yet)
      strncpy(mac_line, ptr+15, 17); //Copy the mac address
      mac_line[17] = '\0';
    }
    ptr = strtok_P(nullptr, g_SEP_NEWLINE);
  }

  // Extract 4 numbers of the IP address (separated by .)
  ptr = strtok_P(ip_line, g_SEP_DOT);
  for(int i=0; i<4 && ptr!=nullptr; ++i){
    _ip_address[i] = strtol(ptr, nullptr, 10);
    ptr = strtok_P(nullptr, g_SEP_DOT);
  }

  // Extract 6 numbers of the MAC address (separated by :)
  ptr = strtok_P(mac_line, g_SEP_COLUMN);
  for(int i=0; i<6 && ptr!=nullptr; ++i){
    _mac_address[i] = strtol(ptr, nullptr, 16);
    ptr = strtok_P(nullptr, g_SEP_COLUMN);
  }
  return isConnected();
}

bool ESP8266Wifi::startServer(int port) const {
  bool success = _client.CIPMUX(true);

  if(success && port>0)
    success = _client.CIPSERVER(true, port);

  return success;
}

bool ESP8266Wifi::stopServer() const {
  return _client.CIPSERVER(false);
}

bool ESP8266Wifi::connectWifi(const char *ssid, const char *passwd) const {
  bool success = _client.CWJAP(ssid, passwd);

  if(!success)
    return false;

  char data[ATClient::DATA_BUFFER_SIZE];
  char *ptr;
  _client.getLastData(data, ATClient::DATA_BUFFER_SIZE);

  bool got_connect = false;
  bool got_ip = false;

  ptr = strtok_P(data, g_SEP_NEWLINE);
  while(ptr!=nullptr){
    if(!got_connect && !got_ip && strstr_P(ptr, PSTR("WIFI DISCONNECT"))==ptr) // If we have this, it must be first
      return false;
    if(!got_ip && strstr_P(ptr, PSTR("WIFI CONNECTED"))==ptr) // We must have this before the IP
      got_connect = true;
    if(got_connect && strstr_P(ptr, PSTR("WIFI GOT IP"))==ptr) // We must have this right after the connect
      got_ip = true;
    ptr = strtok_P(nullptr, g_SEP_NEWLINE);
  }

  return got_connect && got_ip;
}

bool ESP8266Wifi::disConnectWifi() {
  bool success = _client.CWQAP();

  if(!success)
    return false;

  char data[ATClient::DATA_BUFFER_SIZE];
  char *ptr;
  _client.getLastData(data, ATClient::DATA_BUFFER_SIZE);

  ptr = strtok_P(data, g_SEP_NEWLINE);
  while(ptr!=nullptr){
    if(strstr_P(ptr, PSTR("WIFI DISCONNECT"))==ptr){ // Found it
      disconnect();
      return true;
    }
    ptr = strtok_P(nullptr, g_SEP_NEWLINE);
  }

  return false;
}

bool ESP8266Wifi::isConnected() const {
  return _ip_address[0]!=0 || _ip_address[1]!=0  || _ip_address[2]!=0 || _ip_address[3]!=0; //Success if we have a non-zero ip address
}

bool ESP8266Wifi::restartBoard() const {
  if(!_client.RST())
    return false;

  char data[ATClient::DATA_BUFFER_SIZE];
  while(_client.dataAvailable()>0){
    _client.getLastData(data, ATClient::DATA_BUFFER_SIZE);
    _logSerial->print(data);
  }
  return true;
}

bool ESP8266Wifi::fw_version() const {
    bool error = false;
  if(!_client.GMR()){
      _logSerial->println("GMR Error");
      error = true;
  }

  char data[ATClient::DATA_BUFFER_SIZE];
    while(_client.dataAvailable()>0){
        _client.getLastData(data, ATClient::DATA_BUFFER_SIZE);
        _logSerial->print(data);
    }

  return !error;
}

int ESP8266Wifi::openConnection(const char *address, uint16_t port) {
  if(_client.CIPSTART(ATClient::TCP, address, port, 4)){
      init_connection(4);
    return 4;
  }
  return -1;
}

int ESP8266Wifi::openConnection(uint8_t ip[4], uint16_t port) {
  if(_client.CIPSTART(ATClient::TCP, ip, port, 4)){
      init_connection(4);
    return 4;
  }
  return -1;
}

bool ESP8266Wifi::isConnectionOpened(uint8_t conn_number) const {
  if(conn_number>4)
    return false;

  return _conn_opened[conn_number];
}

uint8_t ESP8266Wifi::new_connection(const char *data) {
  uint8_t conn_number = strtoul(data, nullptr, 10);
  if(conn_number>4)
    return 99;

  init_connection(conn_number);
  return conn_number;
}

void ESP8266Wifi::init_connection(uint8_t conn_number) {
    _conn_opened[conn_number] = true;
    if(_payload[conn_number]) //If the payload already exists, clear it
        _payload[conn_number]->clear();
    else // Else create it
        _payload[conn_number] = new Buffer(PAYLOAD_SIZE);
}

uint8_t ESP8266Wifi::end_connection(const char *data) {
  uint8_t conn_number = strtoul(data, nullptr, 10);
  if(conn_number>4)
    return 99;
  _conn_opened[conn_number] = false;

  return conn_number;
}

void ESP8266Wifi::disconnect() {
  memset(_ip_address, 0, 4 * sizeof(uint8_t));
  memset(_mac_address, 0, 6 * sizeof(uint8_t));
}

int8_t ESP8266Wifi::payloadAvailable() const {
  for(int8_t i=4; i>=0; i--)
    // Not available if payload not defined or empty
    if(_payload[i] && _payload[i]->len()>0) return i;
  return -1;
}

bool ESP8266Wifi::payloadAvailable(uint8_t conn) const {
    // Not available if payload not defined or empty
    if(_payload[conn] && _payload[conn]->len()>0) return true;
  return false;
}

int ESP8266Wifi::payloadContainsAt(uint8_t conn_number, const char *str) const {
  // Do not try to check payload for undefined payloads
  if(conn_number>4 || !_payload[conn_number])
    return false;
  return _payload[conn_number]->containsAt(str);
}

size_t ESP8266Wifi::payloadLen(uint8_t conn_number) const {
  // Do not try to check payload for undefined payloads
  if(conn_number>4 || !_payload[conn_number])
    return 0;
  return _payload[conn_number]->len();
}

size_t ESP8266Wifi::getPayload(char *buff, uint8_t conn_number, size_t max) {
  // Do not try to get payload for undefined payloads
  if(conn_number>4 || !_payload[conn_number])
    return 0;
  return _payload[conn_number]->get(buff, max);
}

bool ESP8266Wifi::isHTTPPayloadComplete(int8_t conn) {
    int pos_data = payloadContainsAt(conn, "\r\n\r\n");
    if(pos_data==-1 or (pos_data==int(payloadLen(conn))-4) )
        //Incomplete data
        return false;
    return true;
}

int8_t ESP8266Wifi::waitPayload(int8_t connlisten, char *buff, unsigned long timeout, bool single) {
    unsigned long expire_at = millis()+timeout;
    bool expired = false;
    int8_t conn = -1;
    while(conn==-1){
        readAndPrint(timeout);
        expired = millis()>expire_at;
        if(connlisten==-1) // Listen to any available connection
            conn = payloadAvailable();
        else if(payloadAvailable(connlisten)) // Listen to specific connection
            conn = connlisten;
        if(conn==-1 && expired)
            return -1;
    }

    uint8_t max_try = 0;
    while(!single && !isHTTPPayloadComplete(conn) && isConnectionOpened(conn) && (max_try++<10)){
        readAndPrint();
    }

    getPayload(buff, conn, ESP8266Wifi::PAYLOAD_SIZE);
    return conn;
}

void ESP8266Wifi::printMacAddress() const {
  Serial.print("MAC Address: ");
  for(uint8_t i=0; i<6;++ i){
      if(i>0) Serial.print("::");
    Serial.print(_mac_address[i], HEX);
  }
  Serial.println();
}

void ESP8266Wifi::printIPAddress() const {
  Serial.print("IP Address: ");
  for(uint8_t i=0; i<4;++ i){
    if(i>0) Serial.print(".");
    Serial.print(_ip_address[i]);
  }
  Serial.println();
}

void ESP8266Wifi::read_payload(const char *initdata, size_t len) {
  uint8_t conn_number = strtoul(initdata+5, nullptr, 10);
  // Do not try to read payload for undefined payloads
  if(conn_number>4 || !_payload[conn_number])
    return;
  size_t datas = strtol(initdata+7, nullptr, 10);
  if(datas>PAYLOAD_SIZE) // This is the maximum size of the packet that we are accepting
    return;
  char *data_start = strchr(initdata+7, ':');
  if(data_start==nullptr)
    return;

  size_t init_len = _payload[conn_number]->push(data_start+1, len-(data_start-initdata));
  _payload[conn_number]->push('\n');
  if(init_len==datas-1) {//No more to read
    return;
  }
  char buff[PAYLOAD_SIZE];
  _client.readRaw(buff, datas-init_len-1);
  _payload[conn_number]->push(buff);
}

bool ESP8266Wifi::sendPacket(const char *data, uint8_t conn) {
  return checkBuffAndReturn(_client.CIPSEND(data, conn));
}

bool ESP8266Wifi::sendPacketLen(const char *data, uint8_t conn, size_t len) {
  if(!checkBuffAndReturn(_client.CIPSENDEX(len, conn)))
    return false;
  return checkBuffAndReturn(_client.sendDataConfirm(data, len));
}

bool ESP8266Wifi::closeConnection(uint8_t conn) const {
    if(_conn_opened[conn])
        return _client.CIPCLOSE(conn);
    return true;
}

bool ESP8266Wifi::startsWith(const char *str, const char *search) {
  while( (*str!='\0') && (*search!='\0') ){
    if(*(str++)!=*(search++)) // Not the same char -> we are done
      return false;
  }
  return *search=='\0'; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::startsWith(const char *str, const __FlashStringHelper* search) {
  PGM_P p_search = reinterpret_cast<PGM_P>(search);
  unsigned char c;
  c = pgm_read_byte(p_search++);
  while( (*str!='\0') && (c!='\0') ){
    if(*(str++)!=c) // Not the same char -> we are done
      return false;
    c = pgm_read_byte(p_search++);
  }
  return c=='\0'; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::endsWith(const char *str, const char *search) {
  const char *str_end = str + strlen(str)-1;
  const char *search_end = search + strlen(search)-1;
  while( (str_end!=str) && (search_end+1!=search) ){
    if(*(str_end--)!=*(search_end--)) // Not the same char -> we are done
      return false;
  }
  return search==search_end+1; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::endsWith(const char *str, const __FlashStringHelper* search) {
  const char *str_end = str + strlen(str)-1;
  PGM_P p_search = reinterpret_cast<PGM_P>(search);
  PGM_P p_search_end = p_search + strlen_P(p_search)-1;
  unsigned char c;
  while( (str_end!=str) && (p_search_end+1!=p_search) ){
    c = pgm_read_byte(p_search_end--);
    if(*(str_end--)!=c) // Not the same char -> we are done
      return false;
  }
  return p_search==p_search_end+1; // The loop went through the whole string, finding each character equal
}

bool ESP8266Wifi::checkBuffAndReturn(bool val) {
  checkDataCapture();
  return val;
}
