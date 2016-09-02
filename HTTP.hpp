#ifndef HTTP_H
#define HTTP_H

#include <iostream>
#include <exception>
#include <cctype>
#include <cassert>
#include <string>
#include <map>
#include <utility>
#include "Connection.hpp"

//TODO: most of this could be made private, but is public for now to. Make sure to move
//comments when to cpp

//TODO: make exceptions for HTTP errors have reason string
//	ex: "headers too long"

//replaces CRLF at end of line (if present) with LF '\0' and returns new length
size_t normalizeLineEnding(char *line, size_t len);

//	facilitate easier testing
bool tchar(char c);
bool isToken(const char *t);
bool validHeaderKey(const char *k);
//NOTE: will not accept folded header values (which are obsolete)
bool validHeaderValue(const char *v);

class HTTPError : std::exception {
public:
	HTTPError(const int status_code) : status(status_code) {}
	const int status;
};

class InvalidHeaderKey : std::exception {};
class InvalidHeaderValue : std::exception {};

typedef std::map<std::string, std::string> HeaderMap;

//reads headers starting after the start line until there is an empty line
//NOTE: this treats LF and CRLF as both being empty lines
//this takes a pointer to the buffer to reuse so memory is conserved
HeaderMap parseHeaders(Connection& c, char *buf, size_t BUF_SIZE);

//parsers http version string ie; HTTP/x.x
//NOTE: this function modifies the string
void parseVersion(char *vs, int& major, int& minor);

class Reply {
	int status;
	HeaderMap headers;
	char *body;	//TODO: does unique_ptr make more sense?
	size_t length;
	//do not have to deal with this
	Reply(const Reply&);
	const Reply& operator=(const Reply&);
public:
	Reply(int status);
	Reply(Reply&& r);
};

class Client {
	std::string host;
	int port;
	Connection con;
	HeaderMap headers;
	//implement these later
	Client(const Client&);
	const Client& operator=(const Client&);
public:
	//create TCP connection to host:port
	Client(const std::string& host, int port = 80);
	//No need for custom destructor (yet)
	//~Client();
	//close connection
	void disconnect();
	//closes and reopens connection
	void reconnect();
	//closes existing connection and connects to new host
	void reconnect(const std::string& host, int port = 80);

	//return true only if header already present
	//if key is Cookie or Set-Cookie, ';' is used as delimiter. Otherwise ',' is used.
	bool setHeader(const std::string& key, const std::string& value, bool append = true);

	//If removes header and return true (if header present).
	bool removeHeader(const std::string& key);

	//all the methods
	Reply get(const std::string& target);
	Reply head(const std::string& target);
	Reply post(const std::string& target, char *body, size_t length);
	//fetch options for a specific target
	Reply options(const std::string& target);
	//fetch global options ie: "OPTIONS * HTTP/1.1\r\n"
	Reply options();
};


#endif
