/*
 * PublicationBase.cpp
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#include "PublicationBase.h"

PublicationBase::PublicationBase(const char* name) :
	_updated(false)
{
	strncpy(_name, name, PublicationBase::MAX_NAME_LENGTH);
	_name[min(MAX_NAME_LENGTH, strlen(name))] = 0;
}

PublicationBase::~PublicationBase() {
}

void PublicationBase::to_string_base(char **buff) {
	strcpy(*buff, _name);
	*buff += strlen(_name);
	**buff = ':';
	*buff += 1;
}
