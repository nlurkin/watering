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
	_p_begin = &_buffer[0];
	_p_end = _p_begin;
}

Buffer::~Buffer() {
	delete[] _buffer;
}

void Buffer::clear() {
	_p_end = _p_begin;
}

void Buffer::drop(size_t nchar) {
	if(len()<nchar)
		_p_begin = _p_end;
	else
		_p_begin += nchar;
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

size_t Buffer::push(const char *str) {
	char *p = str;
	while(*p!='\0'){
		if(push(*p)==0) return p-str; //Buffer full, return the number of char inserted so far
		++p;
	}
	return p-str;
}

size_t Buffer::len() const {
	if(_p_end<_p_begin)
		return _p_end-_p_begin+_size;
	return _p_end-_p_begin;
}

char Buffer::peek() const {
	return *_p_begin;
}

bool Buffer::startsWith(const char *str) const {
	char *p = _p_begin;
	while( (p!=_p_end) && (*str!='\0') ){
		if(*(p++)!=*(str++)) // Not the same char -> we are done
			return false;
	}
	return *str=='\0'; // The loop went through the whole string, finding each character equal
}

bool Buffer::startsWith(const __FlashStringHelper* str) const {
	char *p = _p_begin;
	PGM_P p_search = reinterpret_cast<PGM_P>(str);
	unsigned char c;
	c = pgm_read_byte(p_search++);
	while( (p!=_p_end) && (c!='\0') ){
		if(*(p++)!=c) // Not the same char -> we are done
			return false;
		c = pgm_read_byte(p_search++);
	}
	return c=='\0'; // The loop went through the whole string, finding each character equal
}

void Buffer::print() const {
	char * p = _p_begin;
	Serial.print(F("Buffer state ("));
	Serial.print((int)_p_begin);
	Serial.print(F(","));
	Serial.print((int)_p_end);
	Serial.print(F(") ---"));
	while(p!=_p_end){
		if(*p=='\r')
			Serial.print(F("\\r"));
		else if(*p=='\n')
			Serial.print(F("\\n"));
		else
			Serial.print(*p);
		increment(p);
	}
	Serial.println(F("-----"));
}

char Buffer::read() {
	if(_p_begin==_p_end)
		return '\0';

	char v = *_p_begin;
	increment(_p_begin);
	return v;
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
		strlen -= t_len-1;
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

String Buffer::getString() {
	char buffer[_size];
	get(buffer, _size);
	return String(buffer);
}

void Buffer::increment(char *&ptr, size_t len) const {
	ptr += len;
	if (ptr>_buffer+_size)
		ptr = ptr-_size-1;

}
