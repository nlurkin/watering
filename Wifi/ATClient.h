/*
 * ATClient.h
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef ATCLIENT_H_
#define ATCLIENT_H_

#include <Arduino.h>
#include "Buffer.h"

// https://cdn.sparkfun.com/assets/learn_tutorials/4/0/3/4A-ESP8266__AT_Instruction_Set__EN_v0.30.pdf
class ATClient {
public:
	enum TCP_TYPE {TCP, UDP};
	static const size_t BUFFER_SIZE = 100;
	static const size_t DATA_BUFFER_SIZE = 100;

	ATClient(Stream* serial=&Serial1);
	virtual ~ATClient();

	void setLogSerial(Stream* serial);

	bool sendCommand(const char *cmd);
	bool sendCommand(const __FlashStringHelper *cmd);
	bool sendData(const char *data);
	bool sendDataConfirm(const char *data);

	size_t readUntil(char *to, size_t max, const char c='\n');
	size_t readRaw(char * to, size_t max);
	size_t getLastData(char *to, size_t max);
	size_t dataAvailable() const;

	//General commands
	bool AT();
	bool RST();
	bool GMR();
	bool GSLP() {return false;}
	bool ATE() {return false;}
	bool RESTORE() {return false;}
	bool UART() {return false;} //_cur _def
	bool SLEEP(uint8_t mode);
	bool RFPOWER() {return false;}
	bool RFVDD() {return false;}

	//WIFI commands
	bool CWMODE(uint8_t mode); //_cur _def
	bool CWJAP(const char *ssid, const char *passwd);  //_cur _def
	bool CWLAP();
	bool CWQAP();
	bool CWSAP(const char *ssid, const char *passwd, uint8_t channel, uint8_t ecn); //_cur _def
	bool CWLIF();
	bool CWDHCP(bool en, uint8_t mode); //_cur _def
	bool CWAUTOCONN(bool en);
	bool CIPSTAMAC(uint8_t mac[6]); //_cur _def
	bool CIPAPMAC(uint8_t mac[6]); //_cur _def
	bool CIPSTA(uint8_t ip[4], uint8_t gtw[4] = nullptr, uint8_t netmask[4] = nullptr); //_cur _def
	bool CIPAP(uint8_t ip[4], uint8_t gtw[4], uint8_t netmask[4]); //_cur _def
	bool CWSMART() {return false;} //Must run both CWSTARTSMART and finish with CWSTOPSMART
	bool WPS(bool en);

	//TCP/IP commands
	bool CIPSTATUS();
	bool CIPSTART(TCP_TYPE type, uint8_t ip[4], int port, int8_t link_id=-1, int udp_port=-1, uint8_t udp_mode=0, int keepalive=-1);
	bool CIPSTART(TCP_TYPE type, const char *address, int port, int8_t link_id=-1, int udp_port=-1, uint8_t udp_mode=0, int keepalive=-1);
	bool CIPSEND(const char *data, int link_id=-1, uint8_t ip[4]=nullptr, int port=-1);
	bool CIPSENDEX(uint16_t length, int link_id=-1, uint8_t ip[4]=nullptr, int port=-1);
	bool CIPSENDBUF(const char *data, uint8_t &bufferNr, int link_id=-1);
	bool CIPBUFSTATUS(uint8_t link_id=-1);
	bool CIPCHECKSEQ(uint8_t segment, uint8_t link_id=-1);
	bool CIPBUFRESET(uint8_t link_id=-1);
	bool CIPCLOSE(uint8_t link_id=-1);
	bool CIFSR();
	bool CIPMUX(bool mode);
	bool CIPSERVER(bool on, int port=-1);
	bool CIPMODE() {return false;}
	bool CIPSAVETRANSLINK(bool on, uint8_t ip[4]=nullptr, int port=-1, TCP_TYPE type=TCP, int keepalive=-1, int udp_port=-1);
	bool CIPSTO(int keepalive);
	bool CIUPDATE() {return false;}
	bool PINGA(const char *address);
	bool PINGA(uint8_t ip[4]);
	bool CIPDINFO(bool on);

	//template<uint8_t N>
	//bool checkSequence(const char* seq[N]);
	//template<uint8_t N>
	//bool checkSequenceCapture(const char* seq[N], String (&data)[N]);
private:
	size_t transferBuffer();
	size_t waitData(size_t length);
	char read();

	bool checkAnswer(const char *command);
	bool checkAnswer(const __FlashStringHelper *message);
	bool waitMessage(const char *message);
	bool waitMessage(const __FlashStringHelper *message);

	bool _set_default;
	unsigned long _waitingForAnswer;
	unsigned long _timeout;
	Buffer _dataCapture;
	Buffer _buffer;
	Stream *_atSerial;
	Stream *_logSerial;
};

#endif /* ATCLIENT_H_ */
