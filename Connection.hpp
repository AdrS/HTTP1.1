#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <iostream>

#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

class ConnectionError : std::exception {};
class BadFileDescriptorError : ConnectionError {};
class NameResolutionError : ConnectionError {};
class ConnectionEstablishmentError : ConnectionError {};

class ReadError : ConnectionError {};
class NoData : ReadError {};

class WriteError : ConnectionError {};

const size_t BUF_SIZE = 10;
class Connection {
	int fd;
	char readBuf[BUF_SIZE];
	ssize_t rsize;	//amount of unprocessed data in buffer
	char *rpos;	//position in read buffer
	char writeBuf[BUF_SIZE];
	ssize_t wsize;
	char *wpos;
	//TODO: add support for timeouts

	//Do not want to have to deal with these yet
	Connection(const Connection &);
	const Connection& operator=(const Connection&);
public:
	//
	Connection(const char* host, int port);
	//takes the file descriptor of the existing socket to wrap
	Connection(int fd);
	~Connection();
	char readChar();
	size_t read(char *buf, size_t n);
	//returns true if internal read buffer still has data
	bool dataLeft();
	//reads upto and including the next '\n' from the connection
	size_t readLine(char *buf, size_t MAX_LINE);
	void sendChar(char c);
	size_t send(const char *buf, size_t n);
	//writes line to connection. Unless addNewline is false, a newline is appended
	void sendLine(const char* line, bool addNewline = true);
	//flushes the send buffer
	void flush();
	//closes the underlying connection
	void close();
};

#endif
