/*
 * HTTPRequest.cpp
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "HTTPRequest.h"

static const char g_SEP_NEWLINE[] PROGMEM = {"\n"};

HTTPRequest::HTTPRequest() :
	_raw_header{'\0'},
	_body{'\0'}
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

void HTTPRequest::print() const {
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
		Serial.print(F("> "));
		Serial.print(_header._answer_code);
		Serial.print(F(": "));
		Serial.println(_header._answer_reason);
	}

	Serial.println(F("---- START BODY ----"));
	Serial.println(_body);
	Serial.println(F("----  END BODY  ----"));
}

bool HTTPRequest::needs_answer() const {
	return _header._request_type!=ANSWER;
}

size_t HTTPRequest::getTotalLength() const {
	return strlen(_raw_header) + strlen(_body);
}

const char* HTTPRequest::getData() const {
	return _body;
}

const HTTPRequest::header_t& HTTPRequest::getHeader() const {
    return _header;
}

void HTTPRequest::getRawRequest(char *to) const {
	strcpy(to, _raw_header);
	strcat(to, _body);
}

void HTTPRequest::addContent(const char *data) {
	size_t length = strlen(data);
	strncat(_body, data, MAX_DATA_LENGTH-_header._length);
	_header._length += length;
}

void HTTPRequest::setConnectionType(HDR_CONN type) {
	_header._connection = type;
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
	else if (_header._connection == CONN_KEEPALIVE)
		strcat_P(_raw_header, PSTR("Connection: Keep-Alive\r\nKeep-Alive: timeout=5, max=100\r\n"));
	sprintf_P(buf, PSTR("Content-Type: %s\r\n"), _header._content_type);
	strcat(_raw_header, buf);

	size_t body_len = strlen(_body);
	if (body_len > 0) {
		sprintf_P(buf, PSTR("Content-Length: %u\r\n"), body_len);
		strcat(_raw_header, buf);
	}

	strcat_P(_raw_header, PSTR("\r\n"));
	Serial.println(F("---- Header ---"));
	Serial.println(_raw_header);
	Serial.println(F("---- Body ---"));
	Serial.print(_body);
	Serial.println(F("---- End ---"));
	return getTotalLength();
}

void HTTPRequest::extractParts(const char *payload) {
	char *start_data = strstr_P(payload, PSTR("\r\n\r\n"))+4; //Separation between header and data are 2 blank lines
	size_t header_len = start_data-payload;
	if(start_data==nullptr) //Request incomplete. Data not present.
		return;
	size_t data_len = strlen(start_data);
	if(header_len>=MAX_HEADER_LENGTH) //Maximum header length, do no process
		return;
	if(data_len>=MAX_DATA_LENGTH) //Maximum data length, do no process
		return;

	strncpy(_raw_header, payload, header_len); //Does not include a null-terminating char
	_raw_header[header_len] = '\0';
	strncpy(_body, start_data, data_len+1); //Does include the null-terminating char

	char *ptr = strtok_P(_raw_header, g_SEP_NEWLINE);
	while(ptr!=nullptr){
		decodeHeader(ptr);
		ptr = strtok_P(nullptr, g_SEP_NEWLINE);
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
	strcpy_P(h._header._answer_reason, PSTR("OK"));
	strcpy_P(h._header._content_type, PSTR("text/html"));
	h._header._length = 0;
	return h;
}

HTTPRequest HTTPRequest::http_post(const char* path) {
	HTTPRequest h;
	h._header._request_type = POST;
	h._header._version_major = 1;
	h._header._version_minor = 1;
	h._header._answer_code = 0;
	h._header._connection = CONN_UNDEF;
	h._header._answer_reason[0] = '\0';
	strcpy_P(h._header._content_type, PSTR("application/json"));
	h._header._length = 0;
	//strcpy_P(h._header._path, PSTR("/"));
	strcpy(h._header._path, path);
	return h;
}
