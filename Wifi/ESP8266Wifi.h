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

class ESP8266Wifi {
public:
	static const size_t PAYLOAD_SIZE = 1024;

	ESP8266Wifi(Stream* serial=&Serial1);
	virtual ~ESP8266Wifi();

	void setLogSerial(Stream* serial);

	bool sendSomething(const char *cmd);
	bool sendCommand(const char *cmd);
	bool sendData(const char *data);
	bool readAndPrint();

	bool checkBoardConnection();
	bool checkWifiConnection();
	bool connectWifi(const char *ssid, const char *passwd);
	bool disConnectWifi();

	bool startServer(int port);
	bool stopServer();

	bool sendPacket(const char *data, uint8_t conn);
	bool closeConnection(uint8_t conn);
	int  openConnection(const char *address, uint16_t port);

	bool restartBoard();

	int8_t payloadAvailable();
	String getPayload(uint8_t conn_number);
private:
	static bool startsWith(const char *str, const char *search);
	static bool startsWith(const char *str, const __FlashStringHelper *search);
	static bool endsWith(const char *str, const char *search);
	static bool endsWith(const char *str, const __FlashStringHelper *search);
	uint8_t new_connection(const char *data);
	void read_payload(const char *initdata);

	bool _conn_opened[5];
	uint8_t _ip_address[4];
	uint8_t _mac_address[6];
	Buffer _payload[5];
	ATClient _client;
	Stream* _logSerial;
};

#endif /* ESP8266WIFI_H_ */
