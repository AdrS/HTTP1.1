#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include "BaseConnection.hpp"

class TcpConnection : BaseConnection {
public:
	TcpConnection(const char* host, int port);
	virtual ~TcpConnection();
	//close existing connection and opens a new one
	virtual void connect(const char* host, int port);
	//read up to n bytes into buf and returns the number of bytes read
	virtual size_t recv(char *buf, size_t n);
	//sends first n bytes in buffer
	virtual void send(const char *buf, size_t n);
	//closes the underlying connection
	virtual void close();
private:
	//handles dns lookup + making connection
	void setupConnection(const char* host, int port);

	int fd;
};

#endif
