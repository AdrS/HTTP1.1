#ifndef HTTP_H
#define HTTP_H

#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include "Connection.hpp"
#include "HeaderMap.hpp"
#include "URL.hpp"

//TODO: make exceptions for HTTP errors have reason string
//	ex: "headers too long"
class HTTPError : std::exception {
public:
	HTTPError(const int status_code) : status(status_code) {}
	const int status;
};

//replaces CRLF at end of line (if present) with LF '\0' and returns new length
size_t normalizeLineEnding(char *line, size_t len);

//send headers across connection followed by final empty line (CRLF)
size_t sendHeaders(Connection& c, const HeaderMap& headers);

//reads headers starting after the start line until there is an empty line
//NOTE: this treats LF and CRLF as both being empty lines
//this takes a pointer to the buffer to use for storing lines
HeaderMap parseHeaders(Connection& c, char *buf, size_t BUF_SIZE);

//sends data as chunks and if given trailers sends those too
//returns total length of data sent
//NOTE: only send trailers if client says they support them
size_t sendChunked(Connection& c, const char *buf, size_t len,
	const HeaderMap *trailers = nullptr, size_t chunkSize = 1024);

//for internal use (make private after testing)
int parseChunkLen(const char *line);

typedef std::pair<std::unique_ptr<char[]>, size_t> ChunkPair;
//takes connection and a buffer to read lines into
//returns pointer to decoded chunked body + size of body
ChunkPair parseChunked(Connection& c, char *buf, size_t BUF_SIZE);

//size_t sendBody(Connection& c, char *body, size_t len, bool chunked);

//NOTE: if Connection: close, then server does not have to send Content-Length and
//	can signal end of payload by closing connection

class Reply {
	char *body;	//TODO: does unique_ptr make more sense?
	size_t length;
	//do not have to deal with this
	Reply(const Reply&);
	const Reply& operator=(const Reply&);
public:
	int status;
	std::string reason;
	HeaderMap headers;

	Reply(int status, const HeaderMap& headers);
	Reply(Reply&& r);
};

//HTTP client implementation
class Client {
	//internal buffer for storing lines
	static const size_t BUF_SIZE = 8096;
	char buf[BUF_SIZE];

	std::string host;
	int port;
	Connection con;
	bool keepAlive;

	//implement these later
	Client(const Client&);
	const Client& operator=(const Client&);

	//sends request line for given target (percent encodes first by default)
	//WARNING: when sending OPTIONS * HTTP/1.1\r\n, SET ENCODE TO FALSE
	size_t sendRequestLine(const std::string& method,
		const std::string& target, bool encode = true);
public:
	//read status line and returns status code
	int parseStatusLine();
	//read status line and returns status code and reasonPhrase
	int parseStatusLine(std::string& reasonPhrase);

	HeaderMap headers;
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

	//all the methods
	Reply get(const std::string& target);
	Reply head(const std::string& target);
	Reply post(const std::string& target, char *body, size_t length);
	//fetch options for a specific target
	Reply options(const std::string& target);
	//fetch global options ie: "OPTIONS * HTTP/1.1\r\n"
	Reply options();
};

//holds server state associated with a connection
class ClientConnection {
	static const size_t BUF_SIZE = 8096;
	char buf[BUF_SIZE];

	Connection con;
	bool keepAlive;
public:
	void sendStatusLine(int status, const char* reason);
	//same as above, except default reason phrases are used
	void sendStatusLine(int status);
	
	//reads request line and parses method and target
	void parseRequestLine(std::string& method, std::string& target);

	ClientConnection(int fd);
	//close connection with client
	void close();
	//reuse object for new client connection
	void reuse(int fd);
};


extern const char *INFO[];
extern const char *SUCCESS[];
extern const char *REDIRECTION[];
extern const char *CLIENT_ERROR[];
extern const char *SERVER_ERROR[];

//returns correct reason phrase for status code
const char *reasonPhrase(int status);

#endif
