#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <exception>
#include <string>
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

class Connection {
	int fd;
	char readBuf[1024];
	size_t rsize;
	char writeBuf[1024];
	size_t wsize;
	//TODO: add support for timeouts

	//Do not want to have to deal with these yet
	Connection(const Connection &);
	const Connection& operator=(const Connection&);
public:
	//
	Connection(const std::string& host, int port);
	//takes the file descriptor of the existing socket to wrap
	Connection(int fd);
	~Connection();
	char readChar();
	void read(char *buf, size_t n);
	std::string read(size_t n);
	//reads upto and including the next '\n' from the connection
	std::string readLine();
	void sendChar(char c);
	void send(const char *buf, size_t n);
	//writes line to connection. Unless addNewline is false, a newline is appended
	void sendLine(const std::string& line, bool addNewline = true);
	//flushes the send buffer
	void flush();
	//closes the underlying connection
	void close();
};

#endif
