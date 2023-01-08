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

template <>
char* Command<double>::def_string(char* buff){
  to_string_base(&buff);
  strcpy_P(buff, PSTR("(D,1);"));
  buff += 6;
  return buff;
}

template <>
char* Command<int>::def_string(char* buff){
  to_string_base(&buff);
  strcpy_P(buff, PSTR("(I,1);"));
  buff += 6;
  return buff;
}

template <>
char* Command<bool>::def_string(char* buff){
  to_string_base(&buff);
  strcpy_P(buff, PSTR("(B,1);"));
  buff += 6;
  return buff;
}
