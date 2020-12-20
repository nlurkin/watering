/*
 * PubServer.h
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#ifndef PUBSERVER_H_
#define PUBSERVER_H_

#include <Arduino.h>
#include "../LibESP8266/ESP8266Wifi.h"

class PublicationBase;
/*
 * TODO: Write an advertise method that published the list of available publications, including type/units
 * TODO: implement command with persistent state (inhibit command + is it inhibited), or maybe that is in the interface where we link a command with a publication...
 */
class ControlServer {
public:
  static constexpr size_t MAX_COMMANDS = 5;
  static constexpr size_t MAX_PUBLICATIONS = 20;
  static constexpr size_t MAX_MESSAGE_LENGTH = 70;

  ControlServer(ESP8266Wifi &wifi);
  virtual ~ControlServer();

  bool addPublication(PublicationBase* pub);
  bool addCommand(PublicationBase* cmd);

  bool serve();
  bool listen();
  void setDestination(const char *address, uint16_t port);
  void begin(uint16_t port=80);

private:

  uint8_t _num_publications;
  uint8_t _num_commands;
  PublicationBase * _publications[MAX_PUBLICATIONS];
  PublicationBase * _commands[MAX_COMMANDS];
  char *_dest_address;
  uint16_t _dest_port;
  ESP8266Wifi &_wifi;
};

#endif /* PUBSERVER_H_ */
