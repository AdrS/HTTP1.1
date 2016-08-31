#include "Connection.hpp"

using std::string;

Connection::Connection(const string& host, int port) : fd(-1), rsize(0), wsize(0) {
	//do domain name lookup
	addrinfo hints;
	addrinfo *res, *rp;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	//Want IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;//Want TCP
	//all other options are set to default (because of the memset)

	if(getaddrinfo(host.c_str(), nullptr, &hints, &res)) {
		//TODO: add more details to exception, see gai_strerror()
		throw NameResolutionError();
	}

	//try connecting to the returned addresses
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
		if(connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
	}

	freeaddrinfo(res);

	//getting to the end of the list means no connection could be made
	if(rp == nullptr) {
		fd = -1;
		throw ConnectionEstablishmentError();
	}
}

Connection::Connection(int fd) : fd(fd), rsize(0), wsize(0) {
	//verify that file descriptor is for a socket
	struct stat statbuf;
	if(fstat(fd, &statbuf) == -1 || !S_ISSOCK(statbuf.st_mode)) {
		this->fd = -1;
		throw BadFileDescriptorError();
	}
}

Connection::~Connection() {
	//TODO: if close ever throws exception have this catch it (destructor
	//	throwing exception == bad)
	close();
}

char Connection::readChar() {
	return 0;
}

void Connection::read(char *buf, size_t n) {
}

string Connection::read(size_t n) {
	return "";
}

string Connection::readLine() {
	return "";
}

void Connection::sendChar(char c) {
}

void Connection::send(const char *buf, size_t n) {
}

void Connection::sendLine(const string& line, bool addNewline) {
}

void Connection::flush() {
}

void Connection::close() {
	if(fd != -1) {
		flush();
		//TODO: handle errors
		::close(fd);
	}
}
