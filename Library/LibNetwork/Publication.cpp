/*
 * Publication.cpp
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */


#include "Publication.h"


template <>
void Publication<double>::to_string(char* buff){
  //to_string_base(&buff);
  dtostrf(_value, 7, 2, buff);
}

template <>
void Publication<int>::to_string(char* buff){
//  to_string_base(&buff);
  itoa(_value, buff, 10);
}

template <>
void Publication<bool>::to_string(char* buff){
//  to_string_base(&buff);
  if(_value)
    *buff = '1';
  else
    *buff = '0';
  buff[1]='\0';
}


template <>
char* Publication<double>::def_string(char* buff){
  to_string_base(&buff);
  strcpy(buff, "(D,0);");
  buff += 6;
  return buff;
}

template <>
char* Publication<int>::def_string(char* buff){
  to_string_base(&buff);
  strcpy(buff, "(I,0);");
  buff += 6;
  return buff;
}

template <>
char* Publication<bool>::def_string(char* buff){
  to_string_base(&buff);
  strcpy(buff, "(B,0);");
  buff += 6;
  return buff;
}
