#ifndef HTTP_H
#define HTTP_H

#include <exception>
#include <cctype>
#include <string>
#include <map>
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

class InvalidHeaderKey : std::exception {};
class InvalidHeaderValue : std::exception {};
class HeadersTooLong : std::exception {};

typedef std::map<std::string, std::string> HeaderMap;

//reads headers starting after the start line until there is an empty line
//NOTE: this treats LF and CRLF as both being empty lines
//this takes a pointer to the buffer to reuse so memory is conserved
HeaderMap parseHeaders(Connection& c, char *buf, size_t BUF_SIZE);

class Reply {
	int status;
	//headers
	char *body;
	size_t length;
public:
	Reply();
	Reply(int status);
	Reply(const Reply&);
};

class Client {
public:
	Client();
	~Client();
	Reply get(const char* url);
	Reply head(const char* url);
	Reply post(const char* url, char *body, size_t contentLength);
};


//TODO: make this private eventually
void parseMethod(char *mstart);
void parseTarget(char *tstart);
void parseVersion(char *vstart);
int parseStatusCode(char *sstart);
void parseRequestLine(char *rline);
void parseStatusLine(char *rline);

#endif
