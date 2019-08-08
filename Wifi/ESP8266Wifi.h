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
	static constexpr size_t PAYLOAD_SIZE = 500;

	ESP8266Wifi(Stream* serial=&Serial1);
	virtual ~ESP8266Wifi();

	void setLogSerial(Stream* serial);

	bool sendSomething(const char *cmd) const;
	bool sendCommand(const char *cmd) const;
	bool sendData(const char *data) const;
	bool readAndPrint();

	bool checkBoardConnection() const;
	bool checkWifiConnection();
	bool connectWifi(const char *ssid, const char *passwd) const;
	bool disConnectWifi();

	bool startServer(int port) const;
	bool stopServer() const;

	bool sendPacket(const char *data, uint8_t conn) const;
	bool closeConnection(uint8_t conn) const;
	int  openConnection(const char *address, uint16_t port) const;
	int  openConnection(uint8_t ip[4], uint16_t port) const;

	bool restartBoard() const;

	int8_t payloadAvailable() const;
	size_t getPayload(char *buff, uint8_t conn_number, size_t max);
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
	mutable ATClient _client;
	Stream* _logSerial;
};

#endif /* ESP8266WIFI_H_ */
