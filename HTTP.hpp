#ifndef HTTP_H
#define HTTP_H

#include <cctype>
#include "Connection.hpp"

bool tchar(char c);
bool isToken(const char *t);
bool validHeaderKey(const char *k);
//NOTE: will not accept folded header values (which are obsolete)
bool validHeaderValue(const char *v);

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
