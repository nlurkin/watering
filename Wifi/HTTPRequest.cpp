/*
 * HTTPRequest.cpp
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#include "HTTPRequest.h"

HTTPRequest::HTTPRequest(String payload):
	_request_type(UNDEF),
	_answer_code(0)
{
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
			_header += line + "\r\n";
		else{
			_body = payload.substring(curr_pos);
			break;
		}
	}
}

bool HTTPRequest::needs_answer() {
	return _request_type!=ANSWER;
}

bool HTTPRequest::decodeHeader(String line) {
	if(_request_type==UNDEF){
		if(line.startsWith("GET"))
			_request_type = GET;
		else if(line.startsWith("POST"))
			_request_type = POST;
		else if(line.startsWith("PUT"))
			_request_type = PUT;
		else if(line.startsWith("DEL"))
			_request_type = DEL;
		else if(line.startsWith("HEAD"))
			_request_type = HEAD;
		else if(line.startsWith("HTTP")){
			_request_type = ANSWER;
			int first_space = line.indexOf(' ');
			String s_code = line.substring(first_space, line.indexOf(' ', first_space+1));
			Serial.println("Extracting HTTP answer code" + s_code + "(" + line + ")");
			_answer_code = s_code.toInt();
			_answer_reason = line.substring(line.indexOf(' ', first_space+1));
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
	if(_request_type==GET)
		Serial.println("> Type: GET");
	else if(_request_type==POST)
		Serial.println("> Type: POST");
	else if(_request_type==PUT)
		Serial.println("> Type: PUT");
	else if(_request_type==DEL)
		Serial.println("> Type: DEL");
	else if(_request_type==HEAD)
		Serial.println("> Type: HEAD");
	else if(_request_type==ANSWER)
		Serial.println("> " + String(_answer_code) + ": " + _answer_reason);

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
