/*
 * CommandBase.cpp
 *
 *  Created on: 26 Oct 2020
 *      Author: Nicolas Lurkin
 */

#include <Command.h>

template <>
void Command<double>::from_string(char* buff){
  char* endptr;
  _value = strtod(buff, &endptr);
  //TODO check for error
  updated();
}

template <>
void Command<int>::from_string(char* buff){
  _value = atoi(buff);
  updated();
}

template <>
void Command<bool>::from_string(char* buff){
  if(*buff=='1')
    _value = true;
  else
    _value = false;
  updated();
}
