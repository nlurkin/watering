/*
 * PubServer.h
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#ifndef PUBSERVER_H_
#define PUBSERVER_H_

#include <Arduino.h>

class PublicationBase;
/*
 * TODO: Write an advertise method that published the list of available publications, including type/units
 * TODO: implement command with persistent state (inhibit command + is it inhibited), or maybe that is in the interface where we link a command with a publication...
 */
class ControlServer {
public:
  static constexpr uint8_t MAX_COMMANDS = 5;
  static constexpr uint8_t MAX_PUBLICATIONS = 20;
  static constexpr size_t MAX_MESSAGE_LENGTH = 70;

  ControlServer();
  virtual ~ControlServer();

  virtual bool addPublication(PublicationBase* pub);
  virtual bool addCommand(PublicationBase* cmd);

  bool serve(bool force=false);
  bool listen();
  bool advertise();

  virtual bool updatePublications(uint8_t nPubReady, PublicationBase *readyPub[MAX_PUBLICATIONS]) = 0;
  virtual bool checkSubscriptions(char *sname, char *value) = 0;
  virtual bool publishAdvertise(const char * services) = 0;

  uint8_t get_num_commands() { return _num_commands; }
  uint8_t get_num_publications() { return _num_publications; }
  PublicationBase *get_command(uint8_t cmd_num);
  PublicationBase *get_publication(uint8_t pub_num);

private:
  uint8_t _num_publications;
  uint8_t _num_commands;
  PublicationBase * _publications[MAX_PUBLICATIONS];
  PublicationBase * _commands[MAX_COMMANDS];
};

#endif /* PUBSERVER_H_ */
