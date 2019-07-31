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

HTTPRequest::HTTPRequest(String payload)
{
	_header._request_type = UNDEF;
	if(payload.length()==0)
		return;
	extractParts(payload);
}

HTTPRequest::~HTTPRequest() {
}

void HTTPRequest::extractParts(String payload) {
	int curr_pos = 0;
	int lend = payload.indexOf('\r');
	int length = payload.length();
	bool in_header = false;
	//Serial.println("Extracting parts " + String(lend) + " " + String(length));
	while(lend<length && lend!=-1){
		String line = payload.substring(curr_pos, lend);
		//Serial.println("Line:" + String(curr_pos) + "," + String(lend) + " " + line);
		//Serial.println("&");
		curr_pos = lend+2;
		lend = payload.indexOf('\r', curr_pos);
		in_header = decodeHeader(line);
		if(in_header)
			_header._raw_header += line + "\r\n";
		else{
			_body = payload.substring(curr_pos);
			break;
		}
	}
}

bool HTTPRequest::needs_answer() {
	return _header._request_type!=ANSWER;
}

bool HTTPRequest::decodeHeader(String line) {
	if(_header._request_type==UNDEF){
		if(line.startsWith("GET"))
			_header._request_type = GET;
		else if(line.startsWith("POST"))
			_header._request_type = POST;
		else if(line.startsWith("PUT"))
			_header._request_type = PUT;
		else if(line.startsWith("DEL"))
			_header._request_type = DEL;
		else if(line.startsWith("HEAD"))
			_header._request_type = HEAD;
		else if(line.startsWith("HTTP")){
			_header._request_type = ANSWER;
			int first_space = line.indexOf(' ');
			String s_code = line.substring(first_space, line.indexOf(' ', first_space+1));
			Serial.println("Extracting HTTP answer code" + s_code + "(" + line + ")");
			_header._answer_code = s_code.toInt();
			_header._answer_reason = line.substring(line.indexOf(' ', first_space+1));
		}
		return true;
	}
	else{
		//Understand header
		if(line.length()==0)
			return false;
	}
	return true;
}

void HTTPRequest::print(){
	if(_header._request_type==GET)
		Serial.println("> Type: GET");
	else if(_header._request_type==POST)
		Serial.println("> Type: POST");
	else if(_header._request_type==PUT)
		Serial.println("> Type: PUT");
	else if(_header._request_type==DEL)
		Serial.println("> Type: DEL");
	else if(_header._request_type==HEAD)
		Serial.println("> Type: HEAD");
	else if(_header._request_type==ANSWER)
		Serial.println("> " + String(_header._answer_code) + ": " + _header._answer_reason);

	int curr_pos = 0;
	int lend = _body.indexOf('\n');
	int length = _body.length();
	while(lend<length && lend!=-1){
		String line = _body.substring(curr_pos, lend);
		curr_pos = lend+1;
		lend = _body.indexOf('\n', curr_pos);
		Serial.println("> " + line);
	}
	String line = _body.substring(curr_pos);
	Serial.println("> " + line);
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
