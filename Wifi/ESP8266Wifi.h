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
	ESP8266Wifi();
	virtual ~ESP8266Wifi();

	bool sendSomething(String cmd);
	bool sendCommand(String cmd);
	bool sendData(String data);
	bool readAndPrint();

	bool checkBoardConnection();
	bool checkWifiConnection();
	bool connectWifi(String ssid, String passwd);
	bool disConnectWifi();

	bool startServer(int port);
	bool stopServer();

	bool restartBoard();

private:
	String _ssid;
	String _passwd;
	uint8_t _ip_address[4];
	uint8_t _mac_address[6];
	ATClient _client;
};

#endif /* ESP8266WIFI_H_ */
