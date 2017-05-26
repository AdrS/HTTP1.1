#include "TcpConnection.hpp"

#include <errno.h>
#include <cstring>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdio.h>

TcpConnection::TcpConnection(const char* host, int port) : fd(-1) {
	setupConnection(host, port);
}

TcpConnection::~TcpConnection() {
	close();
}

//close existing connection and open a new one
void TcpConnection::connect(const char* host, int port) {
	close();
	setupConnection(host, port);
}

size_t TcpConnection::recv(char *buf, size_t n) {
	assert(fd != -1);
	size_t left = n;
	ssize_t read = 0;
	do {
		read = ::recv(fd, buf, left, 0);
		if(read >= 0) {
			//date (or eof) got read
			buf += read;
			left -= read;
		} else if(errno != EINTR) {
			throw ReadError(strerror(errno));
		}
	} while(left > 0 && read != 0);
	return n - left;
}

void TcpConnection::send(const char *buf, size_t n) {
	assert(fd != -1);
	do {
		ssize_t sent = ::send(fd, buf, n, 0);
		if(sent >= 0) {
			//data got sent
			buf += sent;
			n -= sent;
		} else if(errno != EINTR) {
			//TODO: handle EAGAIN?
			//handle write error (signals are ok)
			//TODO: strerror is not thread safe
			throw WriteError(strerror(errno));
		}
	} while(n > 0);
}

void TcpConnection::close() {
	if(fd != -1) {
		//from the manpage: close() should not be retried after an EINTR
		::close(fd);
		//TODO: what to do on error? (destructor calls this so be careful with exceptions)
	}
	//prevent accidental use after closing/reset state
	fd = -1;
}

//establishes a TCP connection to the given host at the given port
void TcpConnection::setupConnection(const char* host, int port) {
	assert(fd == -1);

	//do domain name lookup
	addrinfo hints;
	addrinfo *res, *rp;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	//Want IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;//Want TCP
	//all other options are set to default (because of the memset)

	int r = getaddrinfo(host, nullptr, &hints, &res);
	if(r) {
		throw NameResolutionError(gai_strerror(r));
	}

	//try connecting to the returned addresses
	//FIXME: this does not work
	for(rp = res; rp != nullptr; rp = rp->ai_next) {
		fd = socket(rp->ai_family, SOCK_STREAM , rp->ai_protocol);
		//fill out port (TODO: could have just filled out the serv param to
		//					getaddrinfo)
		if(rp->ai_family == AF_INET) {
			sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(rp->ai_addr);
			addr->sin_port = htons(port);
		} else {
			sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(rp->ai_addr);
			addr->sin6_port = htons(port);
		}
		if(fd == -1) continue;
		//TODO: handle interuptions by signals
		if(::connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
	}

	freeaddrinfo(res);

	//getting to the end of the list means no connection could be made
	if(rp == nullptr) {
		fd = -1;
		throw ConnectionEstablishmentError("could not establish connection to host");
	}
}
