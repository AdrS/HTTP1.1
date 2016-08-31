#include "Connection.hpp"

//using std::string;
using namespace std;

Connection::Connection(const string& host, int port) : fd(-1), rsize(0),
								rpos(readBuf), wsize(0), wpos(writeBuf) {
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

Connection::Connection(int fd) : fd(fd), rsize(0), rpos(readBuf), wsize(0),
															wpos(writeBuf) {
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
	if(rsize < 1) {
		rpos = readBuf;
		rsize = recv(fd, readBuf, BUF_SIZE, 0);
		if(rsize < 0) {
			rsize = 0;
			throw ReadError();
		}
		if(rsize == 0) throw NoData();
	}
	--rsize;
	return *rpos++;
}

//returns number of bytes read
size_t Connection::read(char *buf, size_t n) {
	char *cur = buf;
	//if read buffer already has enough data, then this is easy
	if(n <= (size_t)rsize) {
		memcpy(cur, rpos, n);
		rsize -= n;
		rpos += n;
		return n;
	}

	//copy over data in buffer
	memcpy(cur, rpos, rsize);
	cur += rsize;
	n -= rsize;
	rpos = readBuf;
	rsize = 0;

	//while there is still lots of data, read directly into output buffer
	//(faster than reading to internal buffer and then copying)
	while(n >= BUF_SIZE) {
		ssize_t len = recv(fd, cur, BUF_SIZE, 0);
		if(len < 0) throw ReadError();
		//early eof
		if(len == 0) {
			return (cur - buf);
		}
		cur += len;
		n -= len;
	}

	//go back to reading into internal buffer when amount left to read < BUF_SIZE
	while(n > (size_t)rsize) {
		rsize = recv(fd, rpos, BUF_SIZE, 0);
		if(rsize < 0) {
			rsize = 0;
			throw ReadError();
		}
		if(rsize == 0) {
			return (cur - buf);
		}

		// if not enough was read to finish request
		if((size_t)rsize < n) {
			memcpy(cur, rpos, rsize);
			cur += rsize;
			n -= rsize;
			rsize = 0;
			rpos = readBuf;
		} else {
			memcpy(cur, rpos, n);
			rsize -= n;
			rpos += n;
			cur += n;
			break;
		}
	}
	return (cur - buf);
}

string Connection::read(size_t n) {
	/*
	string s;
	s.reserve(n);
	while(n > 0) {
	}
	*/
	return "";
}

string Connection::readLine() {
	return "";
}

bool Connection::dataLeft() {
	return rsize > 0;
}

void Connection::sendChar(char c) {
}

size_t Connection::send(const char *buf, size_t n) {
	return 0;
}

void Connection::sendLine(const string& line, bool addNewline) {
}

//clear out send buffer
void Connection::flush() {
	while(wsize > 0) {
		ssize_t sent = ::send(fd, wpos, wsize, 0);
		if(sent < 0) throw WriteError();
		wsize -= sent;
		wpos += sent;
	}
}

void Connection::close() {
	if(fd != -1) {
		flush();
		//TODO: handle errors
		::close(fd);
	}
}
