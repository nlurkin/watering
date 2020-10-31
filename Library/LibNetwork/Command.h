/*
 * CommandBase.h
 *
 *  Created on: 26 Oct 2020
 *      Author: Nicolas Lurkin
 */

#ifndef LIBRARIES_LIBNETWORK_COMMAND_H_
#define LIBRARIES_LIBNETWORK_COMMAND_H_

#include "PublicationBase.h"

template <class T>
class Command : public PublicationBase{
public:
	Command(const char* name) : PublicationBase(name), _value() {};
	virtual ~Command() {};

	void to_string(char* buff) { buff[0]='\0'; };
	void from_string(char* buff);

	T getValue() { return _value; };

private:
	T _value;
};

template <>
void Command<double>::from_string(char* buff);
template <>
void Command<int>::from_string(char* buff);
template <>
void Command<bool>::from_string(char* buff);

#endif /* LIBRARIES_LIBNETWORK_COMMAND_H_ */
