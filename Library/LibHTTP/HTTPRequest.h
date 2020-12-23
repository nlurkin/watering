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
		char _answer_reason[30];
		char _content_type[30];
		char _path[30];
	};
	typedef struct HTTPHeader header_t;

public:
	static constexpr size_t MAX_HEADER_LENGTH=200;
	static constexpr size_t MAX_DATA_LENGTH  =200;
	static constexpr size_t MAX_PACKET_LENGTH=MAX_HEADER_LENGTH + MAX_DATA_LENGTH;

	HTTPRequest();
	HTTPRequest(const char *payload);
	virtual ~HTTPRequest();

	void   print() const;
	bool   needs_answer() const;
	size_t getTotalLength() const;

	const char* getData() const;
	void getRawRequest(char *to) const;

	void addContent(const char *data);
	size_t generate();

	static HTTPRequest http_200();
	static HTTPRequest http_post();
private:
	void extractParts(const char *payload);
	void decodeHeader(const char *line);

	header_t _header;
	char _raw_header[MAX_HEADER_LENGTH];
	char _body[MAX_DATA_LENGTH];
};

#endif /* HTTPREQUEST_H_ */
