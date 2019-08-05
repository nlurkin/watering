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

	void clear();

	char peek();
	inline size_t len();

	size_t push(char c);
	char read();
	size_t get(char *dest, size_t max);
	size_t get(char *dest, size_t max, char until);

private:
	void increment(char*& ptr, size_t len=1);
	bool _allow_overwrite;
	size_t _size;
	char* _buffer;
	char* _p_begin;
	char* _p_end;
};

#endif /* BUFFER_H_ */
