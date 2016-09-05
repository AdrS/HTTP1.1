#include "HTTP.hpp"

using std::string; using std::min; using std::pair; using std::unique_ptr; using std::list;

const char* INFO[] = {
/*100*/ "Continue",
/*101*/ "Switching Protocols"
};

const char *SUCCESS[] = {
/*200*/ "OK",
/*201*/ "Created",
/*202*/ "Accepted",
/*203*/ "Non-Authoritative Information",
/*204*/ "No Content",
/*205*/ "Reset Content",
/*206*/ "Partial Content"
};

const char *REDIRECTION[] = {
/*300*/ "Multiple Choices",
/*301*/ "Moved Permanently",
/*302*/ "Found",
/*303*/ "See Other",
/*304*/ "Not Modified",
/*305*/ "Use Proxy",
		"Switch Proxy",	//No longer used
/*307*/ "Temporary Redirect"
};

const char *CLIENT_ERROR[] = {
/*400*/ "Bad Request",
/*401*/ "Unauthorized",
/*402*/ "Payment Required",
/*403*/ "Forbidden",
/*404*/ "Not Found",
/*405*/ "Method Not Allowed",
/*406*/ "Not Acceptable",
/*407*/ "Proxy Authentication Required",
/*408*/ "Request Time-out",
/*409*/ "Conflict",
/*410*/ "Gone",
/*411*/ "Length Required",
/*412*/ "Precondition Failed",
/*413*/ "Request Entity Too Large",
/*414*/ "Request-URI Too Large",
/*415*/ "Unsupported Media Type",
/*416*/ "Requested range not satisfiable",
/*417*/ "Expectation Failed",
/*418*/ "I'm a teapot"
};

const char *SERVER_ERROR[] = {
/*500*/ "Internal Server Error",
/*501*/ "Not Implemented",
/*502*/ "Bad Gateway",
/*503*/ "Service Unavailable",
/*504*/ "Gateway Time-out",
/*505*/ "HTTP Version not supported"
};

const size_t Client::BUF_SIZE;
const size_t ClientConnection::BUF_SIZE;

//NOTE: this assumes line ends with '\n' and might fail if this is not the case
size_t normalizeLineEnding(char *line, size_t len) {
	if(len > 1 && line[len - 2] == '\r') {
		line[len - 2] = '\n';
		line[len - 1] = '\0';
		return len - 1;
	}
	return len;
}

//A sender MUST NOT generate multiple header fields with the same field name
//A recipient MAY combine multiple header fields with the same field name
HeaderMap parseHeaders(Connection& c, char *buf, size_t BUF_SIZE) {
	HeaderMap hm;
	
	//while there are still headers
	while(true) {
		size_t len = c.readLine(buf, BUF_SIZE);
		//check that an actual line was read
		if(len == 0 || buf[len - 1] != '\n') throw HTTPError(400);

		len = normalizeLineEnding(buf, len);
		//empty line signals end of headers
		if(strcmp(buf, "\n") == 0) break;

		//header-field   = field-name ":" OWS field-value OWS
		//OWS = *(SP / HT)
		//find ':' which seperates key and value
		///////////PARSE HEADER KEY/////////////////
		char *pos = buf;
		while(*pos != ':' && *pos != '\n') ++pos;

		//check that there actually is a seperator
		if(*pos == '\n') throw HTTPError(400);
		*pos++ = '\0';

		char *key = buf;

		///////////PARSE HEADER VALUE///////////////
		//skip over whitespace
		while(*pos == ' ' || *pos == '\t') ++pos;

		//store last non whitespace for trailing OWS removal
		char *lastNonWS = pos, *value = pos;
		while(*pos != '\n') {
			if(*pos != ' ' && *pos != '\t') lastNonWS = pos;
			++pos;
		}
		lastNonWS[1] = '\0';

		//this takes care of header validation
		hm.insert(key, value);
	}
	return hm;
}

//TODO: make final CRLF optional
//send headers across connection followed by final empty line (CRLF)
size_t sendHeaders(Connection& c, const HeaderMap& headers) {
	size_t len = 0;
	for(auto &h : headers) {
		len += c.send(h.first.c_str(), h.first.length());

		c.sendChar(':');

		len += c.send(h.second.c_str(), h.second.length());
		c.sendChar('\r');
		c.sendChar('\n');

		len += 3;
	}
	//send final CRLF to signal end of headers
	c.sendChar('\r');
	c.sendChar('\n');
	return len + 2;
}

size_t sendChunked(Connection& c, const char *buf, size_t len,
	const HeaderMap *trailers, size_t chunkSize) {
	size_t total = len;
	char lb[16];	//stores chunk length (in hex) and CRLF (16 bytes is overkill)
	while(len > 0) {
		size_t curLen = min(len, chunkSize);
		total += snprintf(lb, 16, "%X\r\n", curLen);
		c.sendLine(lb, false);
		c.send(buf, curLen);

		buf += curLen;
		len -= curLen;

		c.sendChar('\r');
		c.sendChar('\n');
		total += 2;
	}
	c.send("0\r\n", 3);
	total += 3;
	if(trailers) {
		//TODO: check for illegal trailer headers
		//NOTE: from what I have read, not very many clients support trailers
		//	so this is probably a waste of time
		total += sendHeaders(c, *trailers);
	} else {
		// send final CRLF
		c.sendChar('\r');
		c.sendChar('\n');
	}
	return total;
}

//lines must end in a newline
//return -1 if line does not start with valid hex digits
int parseChunkLen(const char *line) {
	int n = 0;
	if(!isxdigit(*line)) return -1;
	do {
		n <<= 4;
		n += decodeHexChar(*line++);
		//does not detect overflow
	} while(isxdigit(*line));
	return n;
}

//takes connection and a buffer to read lines into
//returns pointer to decoded chunked body + size of body
ChunkPair parseChunked(Connection& c, char *buf, size_t BUF_SIZE) {
	size_t totalLength = 0;
	size_t len;

	//use unique_ptr so that if there is an exception, chunks get freed
	list<ChunkPair> chunks;
	//for each chunk
	while(true) {
		len = c.readLine(buf, BUF_SIZE);

		//check that an actual line was read
		if(len == 0 || buf[len - 1] != '\n') throw HTTPError(400);

		//get chunk length (and ignore extensions)
		int chunkLen = parseChunkLen(buf);
		if(chunkLen == -1) throw HTTPError(400);
		if(chunkLen == 0) break;

		//make space for chunk
		char *chunk = new char[chunkLen];
		try {
			//read chunk
			if(c.read(chunk, chunkLen) != (size_t)chunkLen) {
				//handle case where we don't get full chunks worth
				//TODO: clean this part up
				throw std::exception();
			}
			totalLength += (size_t)chunkLen;
		} catch(std::exception &e) {
			//make sure to clean up
			delete[] chunk;
			throw e;
		}
		chunks.push_back(make_pair(unique_ptr<char[]>(chunk), (size_t)chunkLen));
		chunk = nullptr;

		//TODO: write a read CRLF function
		len = c.readLine(buf, BUF_SIZE);

		len = normalizeLineEnding(buf, len);
		if(strcmp("\n", buf)) throw HTTPError(400);
	}

	//Ignore trailers for now
	//TODO: should give error if trailers received when TE: trailers is not sent
	parseHeaders(c, buf, BUF_SIZE);

	//merge chunks into contiguous block
	char* body = new char[totalLength]; //NOTE: should work even if length is 0
	size_t cur = 0;
	for(auto &i : chunks) {
		memcpy(body + cur, i.first.get(), i.second);
		cur += i.second;
	}
	assert(cur == totalLength);
	return make_pair(unique_ptr<char[]>(body), totalLength);
}

Reply::Reply(int status, const HeaderMap& headers): body(nullptr), length(0),
		status(status), headers(headers) {
	assert(status >= 100 && status <= 599);
}

//move construtor
Reply::Reply(Reply&& r) {
	status = r.status;
	headers = r.headers;
	body = r.body;
	r.body = nullptr;
	length = r.length;
}

////////////////////CLIENT CODE//////////////////////
Client::Client(const std::string& host, int port) : host(host),
	port(port), con(host.c_str(), port), keepAlive(true) { }
//keep alive is true by default for HTTP/1.1 (not HTTP/1.0)

void Client::disconnect() {
	con.close();
}

void Client::reconnect() {
	con.connect(host.c_str(), port);
}

void Client::reconnect(const std::string& host, int port) {
	this->host = host;
	this->port = port;
	con.connect(this->host.c_str(), port);
}

//WARNING: when sending OPTIONS * HTTP/1.1\r\n, DO NOT URL ENCODE TARGET
//sends request line for given target (percent encodes first by default)
size_t Client::sendRequestLine(const string& method, const string& target, bool encode) {
	//request-line   = method SP request-target SP HTTP-version CRLF
	size_t totalLen = 0;

	totalLen += con.send(method.c_str(), method.length());
	con.sendChar(' ');

	//handle percent encoding
	if(encode) {
		size_t peLen = 3*target.length() + 1;
		unique_ptr<char[]> peTarget(new char[peLen]);
		peLen = percentEncode(target.c_str(), peTarget.get(), target.length(), peLen);

		totalLen += con.send(peTarget.get(), peLen);
	} else {
		totalLen += con.send(target.c_str(), target.length());
	}
	con.sendLine(" HTTP/1.1\r\n", false);
	return totalLen + 12;
}

//read status line and returns status code
int Client::parseStatusLine() {
	string s;
	return parseStatusLine(s);
}

//read status line and returns status code and reasonPhrase
int Client::parseStatusLine(std::string& reasonPhrase) {
	size_t len = con.readLine(buf, BUF_SIZE);
	//check that an actual line was read
	if(len == 0 || buf[len - 1] != '\n') throw HTTPError(400);
	len = normalizeLineEnding(buf, len);

	//check that version is HTTP/1.1
	if(strncmp("HTTP/1.1 ", buf, 9)) throw HTTPError(400);

	//get status
	int status = atoi(buf + 9);
	if(status < 100 || status > 599) throw HTTPError(400);

	//there should be space between status and reason phrase
	if(buf[12] != ' ') throw HTTPError(400);

	//rest of line is reason phrase (except newline)
	reasonPhrase = string(buf + 13, buf + len - 1);
	return status;
}

//DO remember to send Host header first (do no keep it in HeaderMap)
//all the methods
Reply Client::get(const std::string& target) {
	con.send("GET ",4);

	//TODO: lots of overlap with all the other method code
	con.send(target.c_str(), target.length());
	con.send(" HTTP/1.1\r\n", 11);
	//send host header first
	con.send("Host:", 5);
	con.send(host.c_str(), host.length());
	con.send("\r\n", 2);

	//send rest of headers (I assume host is not in this list)
	sendHeaders(con, headers);
	con.flush();

	
	//parse status line
	con.readLine(buf, BUF_SIZE);

	//parse headers
	HeaderMap replyHeaders = parseHeaders(con, buf, BUF_SIZE);

	//get body
	return Reply(400, replyHeaders);
}

Reply Client::head(const std::string& target) {
	return Reply(400, HeaderMap());
}

Reply Client::post(const std::string& target, char *body, size_t length) {
	return Reply(400, HeaderMap());
}

//fetch options for a specific target
Reply Client::options(const std::string& target) {
	return Reply(400, HeaderMap());
}

//fetch global options ie: "OPTIONS * HTTP/1.1\r\n"
Reply Client::options() {
	return options("*");
}

////////////////////SERVER CODE//////////////////////
ClientConnection::ClientConnection(int fd) : con(fd), keepAlive(true) {}

//close connection with client
void ClientConnection::close() {
	con.close();
}

//reuse object for new client connection
void ClientConnection::reuse(int fd) {
	//reset options
	keepAlive = true;
	con.connect(fd);
}

//reason phrase should consists only of tabs, spaces, and visible characters
//in particular it should have no newlines or carriage returns
void ClientConnection::sendStatusLine(int status, const char* reason) {
	if(status < 100 || status > 599) throw std::exception();
	//reason-phrase  = *( HTAB / SP / VCHAR / obs-text )
	char buf[14];
	snprintf(buf, 14, "HTTP/1.1 %d ", status);
	con.send(buf, 13);
	con.sendLine(reason, true, true);
}

const char *reasonPhrase(int status) {
	size_t es = sizeof(INFO[0]);
	if(status < 100) return nullptr;

	size_t s = (size_t)status; //to suppress signed unsigned comparison warnings

	//if in lookup table => return it
	//otherwise default to x00 reason phrase

	//1xx: Informational
	if(s < 100 + sizeof(INFO)/es) return INFO[s - 100];
	if(s < 200) return INFO[0];
	//2xx: Success
	if(s < 200 + sizeof(SUCCESS)/es) return SUCCESS[s - 200];
	if(s < 300) return SUCCESS[0];
	//3xx: Redirection
	if(s < 300 + sizeof(REDIRECTION)/es) return REDIRECTION[s - 300];
	if(s < 400) return REDIRECTION[0];
	//4xx: Client Error
	if(s < 400 + sizeof(CLIENT_ERROR)/es) return CLIENT_ERROR[s - 400];
	if(s < 500) return CLIENT_ERROR[0];
	//5xx: Server Error
	if(s < 500 + sizeof(SERVER_ERROR)/es) return SERVER_ERROR[s - 500];
	if(s < 600) return SERVER_ERROR[0];

	return nullptr;
}

//same as above, except default reason phrases are used
void ClientConnection::sendStatusLine(int status) {
	sendStatusLine(status, reasonPhrase(status));
}
