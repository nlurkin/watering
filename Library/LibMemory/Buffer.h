/*
 * Buffer.h
 *
 *  Created on: 3 Aug 2019
 *      Author: Nicolas Lurkin
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <Arduino.h>

class Buffer {
public:
  Buffer(size_t size);
  virtual ~Buffer();

  void   clear();
  void   drop(size_t nchar);
  size_t push(char c);
  size_t push(const char *str);

  inline size_t len() const;
  char peek() const;
  bool startsWith(const char *str) const;
  bool startsWith(const __FlashStringHelper *str) const;
  int  containsAt(const char *str) const;
  void print() const;

  char   read();
  size_t get(char *dest, size_t max);
  size_t get(char *dest, size_t max, char until);
  String getString();

private:
  void increment(char*& ptr, size_t len=1) const;

  bool _allow_overwrite;
  char* _buffer;
  char* _p_begin;
  char* _p_end;
  size_t _size;
};

#endif /* BUFFER_H_ */
