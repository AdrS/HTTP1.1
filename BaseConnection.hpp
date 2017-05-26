#ifndef BASE_CONNECTION_HPP
#define BASE_CONNECTION_HPP

#include <stdexcept>
#include <cstddef>

class BaseConnection {
public:
	virtual ~BaseConnection() {}
	//close existing connection and opens a new one
	virtual void connect(const char* host, int port) = 0;
	//read up to n bytes into buf and returns the number of bytes read
	virtual size_t recv(char *buf, size_t n) = 0;
	//sends first n bytes in buffer
	virtual void send(const char *buf, size_t n) = 0;
	//closes the underlying connection
	virtual void close() = 0;
};

class BaseConnectionError : public std::runtime_error {
public:
	BaseConnectionError(const char *what_arg) : std::runtime_error(what_arg) {}
};

class NameResolutionError : public BaseConnectionError {
public:
	NameResolutionError(const char *what_arg) : BaseConnectionError(what_arg) {}
};

class ConnectionEstablishmentError : public BaseConnectionError {
public:
	ConnectionEstablishmentError(const char *what_arg) : BaseConnectionError(what_arg) {}
};

class ReadError : public BaseConnectionError {
public:
	ReadError(const char *what_arg) : BaseConnectionError(what_arg) {}
};

class NoData : public ReadError {
public:
	NoData(const char *what_arg) : ReadError(what_arg) {}
};

class WriteError : public BaseConnectionError {
public:
	WriteError(const char *what_arg) : BaseConnectionError(what_arg) {}
};

#endif
