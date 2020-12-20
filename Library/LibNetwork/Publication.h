/*
 * Publication.h
 *
 *  Created on: 7 Jun 2020
 *      Author: Nicolas Lurkin
 */

#ifndef PUBLICATION_H_
#define PUBLICATION_H_

#include "PublicationBase.h"

/*
 *
 */
template <class T>
class Publication : public PublicationBase {
public:
  Publication(const char* name) : PublicationBase(name), _value() {};
  virtual ~Publication(){};

  void updateValue(T newVal){
    if(_value == newVal)
      return;
    _value = newVal;
    updated();
  }

  void to_string(char* buff);
  void from_string(char*) {};
  char* def_string(char* buff);

private:
  T _value;
};

template <>
void Publication<double>::to_string(char* buff);
template <>
void Publication<int>::to_string(char* buff);
template <>
void Publication<bool>::to_string(char* buff);

template <>
char* Publication<double>::def_string(char* buff);
template <>
char* Publication<int>::def_string(char* buff);
template <>
char* Publication<bool>::def_string(char* buff);


#endif /* PUBLICATION_H_ */
