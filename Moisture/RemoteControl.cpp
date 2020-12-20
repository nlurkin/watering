/*
 * RemoteControl.cpp
 *
 *  Created on: 14 Jun 2020
 *      Author: Nicolas Lurkin
 */

#include "RemoteControl.h"

RemoteControl::RemoteControl(ESP8266Wifi &wifi) :
	ControlServer(wifi)
{
}

RemoteControl::~RemoteControl() {
}

