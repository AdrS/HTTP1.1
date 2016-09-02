#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <iostream>
#include <errno.h>

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
	char *rpos;		//position in read buffer
	char writeBuf[BUF_SIZE];
	size_t wleft;	//space left at end of buffer
	char *wpos;		//location to append data to
	//TODO: add support for timeouts

	//Do not want to have to deal with these yet
	const Connection& operator=(const Connection&);
	Connection(const Connection &);

	//this does the work of setting up connections for the
	//constructor and reconnect
	void setupConnection(const char* host, int port);
public:
	//
	Connection(const char* host, int port);
	//takes the file descriptor of the existing socket to wrap
	Connection(int fd);
	~Connection();
	//close existing connection and open a new one
	void connect(const char* host, int port);
	char readChar();
	size_t read(char *buf, size_t n);
	//returns true if internal read buffer still has data
	bool dataLeft();
	//reads upto MAX_LINE - 1 bytes from connection until a '\n' or '\0' is read
	//copies newline character and adds null terminator
	size_t readLine(char *buf, size_t MAX_LINE);
	void sendChar(char c);
	//send upto the first n characters of buf and return number of characters
	//successfully sent
	size_t send(const char *buf, size_t n);
	//writes line to connection. Unless addNewline is false, a newline is appended
	void sendLine(const char* line, bool addNewline = true);
	//flushes the send buffer
	void flush();
	//closes the underlying connection
	void close();
};

#endif
