/*
 * HTTPRequest.cpp
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "HTTPRequest.h"

HTTPRequest::HTTPRequest()
{
}

HTTPRequest::HTTPRequest(const char *payload)
{
	_header._request_type = UNDEF;
	if(payload[0]=='\0') //Empty payload
		return;
	extractParts(payload);
}

HTTPRequest::~HTTPRequest() {
}

void HTTPRequest::print(){
	if(_header._request_type==GET)
		Serial.println(F("> Type: GET"));
	else if(_header._request_type==POST)
		Serial.println(F("> Type: POST"));
	else if(_header._request_type==PUT)
		Serial.println(F("> Type: PUT"));
	else if(_header._request_type==DEL)
		Serial.println(F("> Type: DEL"));
	else if(_header._request_type==HEAD)
		Serial.println(F("> Type: HEAD"));
	else if(_header._request_type==ANSWER){
		char buff[50];
		sprintf_P(buff, PSTR("> %d: %s"), _header._answer_code, _header._answer_reason);
		Serial.println(buff);
	}

	Serial.println(F("---- START BODY ----"));
	Serial.println(_body);
	Serial.println(F("----  END BODY  ----"));
}

bool HTTPRequest::needs_answer() {
	return _header._request_type!=ANSWER;
}

size_t HTTPRequest::getTotalLength() {
	return strlen(_raw_header) + strlen(_body);
}

const char* HTTPRequest::getData() {
	return _body;
}

void HTTPRequest::getRawRequest(char *to) {
	strcpy(to, _raw_header);
	strcat(to, _body);
}

void HTTPRequest::addContent(const char *data) {
	size_t length = strlen(data);
	strncat(_body, data, 1024-_header._length);
	_header._length += length;
}

size_t HTTPRequest::generate() {
	char buf[50];

	if (_header._request_type == ANSWER)
		sprintf_P(buf, PSTR("HTTP/%u.%u %u %s\r\n"), _header._version_major,
				_header._version_minor, _header._answer_code,
				_header._answer_reason);
	else if (_header._request_type == POST)
		sprintf_P(buf, PSTR("POST %s HTTP/%u.%u\r\n"), _header._path,
				_header._version_major, _header._version_minor);
	strcpy(_raw_header, buf);
	if (_header._connection == CONN_CLOSE)
		strcat_P(_raw_header, PSTR("Connection: close\r\n"));
	sprintf_P(buf, PSTR("Content-Type: %s\r\n"), _header._content_type);
	strcat(_raw_header, buf);

	size_t body_len = strlen(_body);
	if (body_len > 0) {
		sprintf_P(buf, PSTR("Content-Length: %u\r\n"), body_len);
		strcat(_raw_header, buf);
	}

	return getTotalLength();
}

void HTTPRequest::extractParts(const char *payload) {
	char *start_data = strstr_P(payload, PSTR("\r\n\r\n"))+4; //Separation between header and data are 2 blank lines
	size_t header_len = start_data-payload-4;
	size_t data_len = strlen(start_data);
	if(header_len>=MAX_HEADER_LENGTH) //Maximum header length, do no process
		return;
	if(data_len>=MAX_DATA_LENGTH) //Maximum data length, do no process
		return;

	strncpy(_raw_header, payload, header_len); //Does not include a null-terminating char
	_raw_header[header_len] = '\0';
	strncpy(_body, start_data, data_len); //Does include the null-terminating char

	char *ptr = strtok(_raw_header, "\n");
	while(ptr!=nullptr){
		decodeHeader(ptr);
		strtok(nullptr, "\n");
	}
}

void HTTPRequest::decodeHeader(const char *line) {
	if(_header._request_type==UNDEF){
		if(line[0]=='G' && line[1]=='E' && line[2]=='T')
			_header._request_type = GET;
		else if(line[0]=='P' && line[1]=='O' && line[2]=='S' && line[3]=='T')
			_header._request_type = POST;
		else if(line[0]=='P' && line[1]=='U' && line[2]=='T')
			_header._request_type = PUT;
		else if(line[0]=='D' && line[1]=='E' && line[2]=='L')
			_header._request_type = DEL;
		else if(line[0]=='H' && line[1]=='E' && line[2]=='A' && line[3]=='D')
			_header._request_type = HEAD;
		else if(line[0]=='H' && line[1]=='T' && line[2]=='T' && line[3]=='P') {
			// Line looks like
			// "HTTP/1.1 200 OK"
			_header._request_type = ANSWER;
			char *space = strchr(line, ' ');
			_header._answer_code = strtol(space+1, &space, 10);
			strncpy(_header._answer_reason, space, 30);
		}
	}
	else{
		//Decode and understand other header line
	}
}

HTTPRequest HTTPRequest::http_200() {
	HTTPRequest h;
	h._header._request_type = ANSWER;
	h._header._version_major = 1;
	h._header._version_minor = 1;
	h._header._answer_code = 200;
	h._header._connection = CONN_CLOSE;
	h._header._answer_reason = "OK";
	h._header._content_type = "text/html";
	h._header._length = 0;
	return h;
}

HTTPRequest HTTPRequest::http_post() {
	HTTPRequest h;
	h._header._request_type = POST;
	h._header._version_major = 1;
	h._header._version_minor = 1;
	h._header._answer_code = 0;
	h._header._connection = CONN_UNDEF;
	h._header._answer_reason = "";
	h._header._content_type = "application/json";
	h._header._length = 0;
	h._header._path = "/arduino";
	return h;
}
