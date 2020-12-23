/*
 * Buffer.cpp
 *
 *  Created on: 3 Aug 2019
 *      Author: Nicolas Lurkin
 */

#include "Buffer.h"

Buffer::Buffer(size_t size):
	_allow_overwrite(true),
	_size(size)
{
	_buffer = new char[_size+1];
	_p_begin = &_buffer[_size-5];
	_p_end = _p_begin;
}

Buffer::~Buffer() {
	delete[] _buffer;
}

void Buffer::clear() {
	_p_end = _p_begin;
}

size_t Buffer::push(char c) {
	if (len() == _size) {//Buffer full
		if(!_allow_overwrite) // Not allowed to overwrite previous data if full
			return 0;
		else
			increment(_p_begin); // Allowed to overwrite previous data. Loose oldest character
	}
	*_p_end = c;
	increment(_p_end);
	return 1;
}

size_t Buffer::len() {
	if(_p_end<_p_begin)
		return _p_end-_p_begin+_size;
	return _p_end-_p_begin;
}

char Buffer::read() {
	if(_p_begin==_p_end)
		return '\0';

	char v = *_p_begin;
	increment(_p_begin);
	return v;
}

char Buffer::peek() {
	return *_p_begin;
}

size_t Buffer::get(char *dest, size_t max) {
	size_t strlen = len();
	if(strlen==0){ //Shortcut, nothing to do
		dest[0] = '\0';
		return 0;
	}
	size_t t_len = 0;
	if(strlen>max-1)
		strlen = max-1;

	if(_p_begin+strlen>_buffer+_size){
		size_t t_len = _buffer+_size-_p_begin+1;
		memcpy(dest, _p_begin, t_len);
		increment(_p_begin, t_len);
		dest += t_len;
		strlen -= t_len;
	}

	memcpy(dest, _p_begin, strlen);
	dest[strlen+t_len] = '\0';
	increment(_p_begin, strlen);
	return strlen+t_len;
}

size_t Buffer::get(char *dest, size_t max, char until) {
	size_t strlen = len();
	if(strlen>max-1)
		strlen = max-1;

	size_t read = 0;
	while(read<strlen && _p_begin!=_p_end){
		dest[read++] = *_p_begin;
		increment(_p_begin);
		if(dest[read-1]==until)
			break;
	}
	dest[read] = '\0';
	return read;
}

void Buffer::increment(char *&ptr, size_t len) {
	ptr += len;
	if (ptr>_buffer+_size)
		ptr = ptr-_size-1;

}

String Buffer::getString() {
	char buffer[_size];
	get(buffer, _size);
	return String(buffer);
}

void Buffer::print() {
	char * p = _p_begin;
	Serial.println("Buffer state," + String((int)_p_begin) + "," + String((int)_p_end));
	while(p!=_p_end){
		if(*p=='\r')
			Serial.print("\\r");
		else if(*p=='\n')
			Serial.print("\\n");
		else
			Serial.print(*p);
		increment(p);
	}
	Serial.println("-----");
}
