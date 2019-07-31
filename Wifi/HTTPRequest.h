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
	enum REQ_TYPE {UNDEF, GET, POST, PUT, DEL, HEAD, ANSWER};
	enum HDR_CONN {CONN_UNDEF, CONN_CLOSE};
	struct HTTPHeader {
		REQ_TYPE _request_type;
		HDR_CONN _connection;
		uint8_t _version_major;
		uint8_t _version_minor;
		uint16_t _answer_code;
		uint16_t _length;
		String _answer_reason;
		String _content_type;
		String _path;
		String _raw_header;
	};
	typedef struct HTTPHeader header_t;
public:
	HTTPRequest();
	HTTPRequest(String payload);
	virtual ~HTTPRequest();

	void print();
	bool needs_answer();
	
	static HTTPRequest http_200();
	static HTTPRequest http_post();
private:
	void extractParts(String payload);
	bool decodeHeader(String line);
	header_t _header;
	String _body;
};

#endif /* HTTPREQUEST_H_ */
