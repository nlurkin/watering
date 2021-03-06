/*
 * ESP8266Wifi.h
 *
 *  Created on: 27 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef ESP8266WIFI_H_
#define ESP8266WIFI_H_

#include <Arduino.h>

#include "ATClient.h"
#include "Buffer.h"

class ESP8266Wifi {
public:
  static constexpr size_t PAYLOAD_SIZE = 400; //Payload received from network. Typically ~200 for header + data (max 200, coherent with HTTPRequest)

  ESP8266Wifi(Stream* serial=&Serial1);
  virtual ~ESP8266Wifi();

  bool checkCommunication(bool debug=false);
  bool init(const char* ssid, const char* password, bool doReset=false, bool debug=false);

  void setLogSerial(Stream* serial);

  bool sendSomething(const char *cmd) const;
  bool sendCommand(const char *cmd);
  bool sendData(const char *data);
  bool readAndPrint(unsigned int timeout=1000);
  bool checkDataCapture();

  bool checkBoardConnection() const;
  bool checkWifiConnection();
  bool connectWifi(const char *ssid, const char *passwd) const;
  bool disConnectWifi();
  bool isConnected() const;

  bool startServer(int port=-1) const;
  bool stopServer() const;

  bool sendPacket(const char *data, uint8_t conn);
  bool sendPacketLen(const char *data, uint8_t conn, size_t len);
  bool closeConnection(uint8_t conn) const;
  int  openConnection(const char *address, uint16_t port);
  int  openConnection(uint8_t ip[4], uint16_t port);
  bool isConnectionOpened(uint8_t conn_number) const;

  bool restartBoard() const;
  bool fw_version() const;

  int8_t payloadAvailable() const;
  bool   payloadAvailable(uint8_t conn_number) const;
  int payloadContainsAt(uint8_t conn_number, const char* str) const;
  size_t payloadLen(uint8_t conn_number) const;
  size_t getPayload(char *buff, uint8_t conn_number, size_t max);
  bool isHTTPPayloadComplete(int8_t conn);
  int8_t waitPayload(int8_t connlisten, char *buff, unsigned long timeout, bool single=false);

  void printMacAddress() const;
  void printIPAddress() const;
private:
  static bool startsWith(const char *str, const char *search);
  static bool startsWith(const char *str, const __FlashStringHelper *search);
  static bool endsWith(const char *str, const char *search);
  static bool endsWith(const char *str, const __FlashStringHelper *search);
  uint8_t new_connection(const char *data);
  uint8_t end_connection(const char *data);
  void disconnect();
  void read_payload(const char *initdata, size_t len);
  void init_connection(uint8_t conn_number);
  bool checkBuffAndReturn(bool val);

  static Buffer _persistent_buffer;
  bool _conn_opened[5];
  uint8_t _ip_address[4];
  uint8_t _mac_address[6];
  Buffer *_payload[5];
  mutable ATClient _client;
  Stream* _logSerial;
};

#endif /* ESP8266WIFI_H_ */
