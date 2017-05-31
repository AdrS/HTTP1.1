
#include "BufferedConnection.hpp"
#include "TlsConnection.hpp"
#include "TcpConnection.hpp"

#include <iostream>

using namespace std;


void BufferedConnection::setupConnection(const char* host, int port, bool tls) {
	if(tls) {
		con = new TlsConnection(host, port);
	} else {
		con = new TcpConnection(host, port);
	}
}

BufferedConnection::BufferedConnection(const char* host, int port, bool tls) : con(nullptr), rsize(0),
		rpos(readBuf), wleft(BUF_SIZE), wpos(writeBuf) {
	setupConnection(host, port, tls);
}

//close existing connection and open a new one
void BufferedConnection::connect(const char* host, int port, bool tls) {
	close();
	setupConnection(host, port, tls);
}

BufferedConnection::~BufferedConnection() {
	close();
}

char BufferedConnection::readChar() {
	if(rsize < 1) {
		rpos = readBuf;
		rsize = con->recv(readBuf, BUF_SIZE);
		if(rsize == 0) throw NoData("not character could be read");
	}
	--rsize;
	return *rpos++;
}

//returns number of bytes read
size_t BufferedConnection::recv(char *buf, size_t n) {
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
		size_t len = con->recv(cur, BUF_SIZE);
		//if recv fail, exception raised by con
		
		//early eof
		if(len == 0) {
			return (cur - buf);
		}
		cur += len;
		n -= len;
	}

	//go back to reading into internal buffer when amount left to read < BUF_SIZE
	while(n > (size_t)rsize) {
		rsize = con->recv(rpos, BUF_SIZE);
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

size_t BufferedConnection::readLine(char *buf, size_t MAX_LINE) {
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

bool BufferedConnection::dataLeft() {
	return rsize > 0;
}

void BufferedConnection::sendChar(char c) {
	//if no space in buffer, then flush
	if(wleft == 0) flush();
	*wpos++ = c;
	--wleft;
}

void BufferedConnection::send(const char *buf, size_t n) {
	//if buffer has enough space for all the data
	if(n <= wleft) {
		memcpy(wpos, buf, n);
		wpos += n;
		wleft -= n;
		//TODO: should I check if buffer is full, so I can flush?
		return;
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
		con->send(buf, BUF_SIZE);
		buf += BUF_SIZE;
		n -= BUF_SIZE;
	}

	//less than BUF_SIZE bytes of data left, so store in internal buffer for now
	memcpy(writeBuf, buf, n);
	wpos = writeBuf + n;
	wleft -= n;
}

void BufferedConnection::sendLine(const char* line, bool addNewline, bool crlf) {
	send(line, strlen(line));
	if(addNewline) {
		if(crlf) sendChar('\r');
		sendChar('\n');
	}
}

//clear out send buffer
void BufferedConnection::flush() {
	con->send(writeBuf, wpos - writeBuf);
	wpos = writeBuf;
	wleft = BUF_SIZE;
}

void BufferedConnection::close() {
	delete con;
	con = nullptr;
	rsize = 0;
	rpos = readBuf;
	wpos = writeBuf;
	wleft = BUF_SIZE;
}
