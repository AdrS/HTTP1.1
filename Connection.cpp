#include "Connection.hpp"

using namespace std;

void Connection::setupConnection(const char* host, int port) {
	//NOTE: initialization of fd, rsize, rpos, wleft, wpos is taken
	//care of by the constructor and close (which are always called
	//right before this)

	//do domain name lookup
	addrinfo hints;
	addrinfo *res, *rp;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	//Want IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;//Want TCP
	//all other options are set to default (because of the memset)

	if(getaddrinfo(host, nullptr, &hints, &res)) {
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
		if(::connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
	}

	freeaddrinfo(res);

	//getting to the end of the list means no connection could be made
	if(rp == nullptr) {
		fd = -1;
		throw ConnectionEstablishmentError();
	}
}

Connection::Connection(const char* host, int port) : fd(-1), rsize(0),
		rpos(readBuf), wleft(BUF_SIZE), wpos(writeBuf) {
	setupConnection(host, port);
}

Connection::Connection(int fd) : fd(fd), rsize(0), rpos(readBuf), wleft(BUF_SIZE),
															wpos(writeBuf) {
	//verify that file descriptor is for a socket
	struct stat statbuf;
	if(fstat(fd, &statbuf) == -1 || !S_ISSOCK(statbuf.st_mode)) {
		this->fd = -1;
		throw BadFileDescriptorError();
	}
}

//close existing connection and open a new one
void Connection::connect(const char* host, int port) {
	close();
	setupConnection(host, port);
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

			//TODO: add descriptions to errors strerror(errno)
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
		if(len < 0) {
			//TODO: remove me
			cout << strerror(errno) << endl;
			throw ReadError();
		}
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
			//TODO: remove me
			cout << strerror(errno) << endl;
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

size_t Connection::readLine(char *buf, size_t MAX_LINE) {
	size_t len = 0;
	char c;
	while(len < MAX_LINE - 1) {
		try {
			//TODO: I do not like this, make readChar act like getchar
			c = readChar(); 
		} catch(NoData &e) {
			break;
		}
		if(c == '\0') break;
		*buf++ = c;
		++len;
		if(c == '\n') break;
	}
	*buf = '\0';
	return len;
}

bool Connection::dataLeft() {
	return rsize > 0;
}

void Connection::sendChar(char c) {
	//if no space in buffer, then flush
	if(wleft == 0) flush();
	*wpos++ = c;
	--wleft;
}

size_t Connection::send(const char *buf, size_t n) {
	size_t total_len = n;
	//if buffer has enough space for all the data
	if(n <= wleft) {
		memcpy(wpos, buf, n);
		wpos += n;
		wleft -= n;
		//TODO: should I check if buffer is full, so I can flush?
		return n;
	}

	//fill up reminder of internal buffer and send it off
	memcpy(wpos, buf, wleft);
	buf += wleft;
	n -= wleft;
	wpos += wleft;
	wleft = 0;
	flush();

	//skip copying to internal buffer while buf has >= BUF_SIZE data left
	while(n >= BUF_SIZE) {
		ssize_t sent = ::send(fd, buf, BUF_SIZE, 0);
		if(sent < 0) throw WriteError();
		buf += sent;
		n -= sent;
	}

	//less than BUF_SIZE bytes of data left, so store in internal buffer for now
	memcpy(writeBuf, buf, n);
	wpos = writeBuf + n;
	wleft -= n;
	return total_len;
}

void Connection::sendLine(const char* line, bool addNewline) {
	send(line, strlen(line));
	if(addNewline) {
		sendChar('\n');
	}
}

//clear out send buffer
void Connection::flush() {
	char *cur = writeBuf;
	while(cur != wpos) {
		ssize_t sent = ::send(fd, cur, wpos - cur, 0);
		if(sent < 0) throw WriteError();
		cur += sent;
	}
	wpos = writeBuf;
	wleft = BUF_SIZE;
}

void Connection::close() {
	if(fd != -1) {
		flush();
		//TODO: handle errors
		::close(fd);
	}
	//prevent accidental use after closing
	fd = -1;
	rsize = 0;
	rpos = readBuf;
}
