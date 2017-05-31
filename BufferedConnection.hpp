#ifndef BUFFERED_CONNECTION_HPP
#define BUFFERED_CONNECTION_HPP

#include <stdexcept>
#include <cstddef>
#include "BaseConnection.hpp"

const size_t BUF_SIZE = 10;

class BufferedConnection {
public:
	BufferedConnection(const char *host, int port, bool tls = false);
	virtual ~BufferedConnection();
	char readChar();
	//returns true if internal read buffer still has data
	bool dataLeft();
	//reads upto MAX_LINE - 1 bytes from connection until a '\n' or '\0' is read
	//copies newline character and adds null terminator
	size_t readLine(char *buf, size_t MAX_LINE);
	void sendChar(char c);
	//writes line to connection. Unless addNewline is false, a newline is appended
	//use crlf option to specify type of newline sent
	void sendLine(const char* line, bool addNewline = true, bool crlf = false);
	//flushes the send buffer
	void flush();
	//close existing connection and opens a new one
	void connect(const char* host, int port, bool tls = false);
	//read up to n bytes into buf and returns the number of bytes read
	size_t recv(char *buf, size_t n);
	//sends first n bytes in buffer
	size_t send(const char *buf, size_t n);
	//closes the underlying connection
	void close();
private:
	const BufferedConnection& operator=(const BufferedConnection&);
	BufferedConnection(const BufferedConnection&);

	void setupConnection(const char *host, int port, bool tls);


	BaseConnection *con;
	char readBuf[BUF_SIZE];
    ssize_t rsize;  //amount of unprocessed data in buffer
    char *rpos;     //position in read buffer
    char writeBuf[BUF_SIZE];
    size_t wleft;   //space left at end of buffer
    char *wpos; 
};

//TODO: replace Connection.hpp with BufferedConnection
//	=> modify HTTP to use BufferedConnection
//	=> parse url to determine whether or not to use TLS

#endif
