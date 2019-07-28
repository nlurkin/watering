/*
 * HTTPRequest.h
 *
 *  Created on: 28 Jul 2019
 *      Author: Nicolas Lurkin
 */

#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <Arduino.h>

class HTTPRequest {
public:
	enum REQ_TYPE {UNDEF, GET, POST, PUT, DEL, HEAD, ANSWER};
	HTTPRequest(String payload);
	virtual ~HTTPRequest();

	void print();
	bool needs_answer();
private:
	void extractParts(String payload);
	bool decodeHeader(String line);
	REQ_TYPE _request_type;
	uint16_t _answer_code;
	uint8_t _host_ip[4];
	String _header;
	String _body;
	String _answer_reason;
};

#endif /* HTTPREQUEST_H_ */
