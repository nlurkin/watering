/*
 * RemoteControl.h
 *
 *  Created on: 14 Jun 2020
 *      Author: Nicolas Lurkin
 */

#ifndef REMOTECONTROL_H_
#define REMOTECONTROL_H_

#include "ControlServer.h"
/*
 *
 */
class RemoteControl : public ControlServer {
public:
	RemoteControl(ESP8266Wifi &wifi);
	virtual ~RemoteControl();
};

#endif /* REMOTECONTROL_H_ */
