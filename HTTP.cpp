#include "HTTP.hpp"

using std::string;

const size_t Client::BUF_SIZE;

//NOTE: this assumes line end in '\n' and might fail if this is not the case
size_t normalizeLineEnding(char *line, size_t len) {
	if(len > 1 && line[len - 2] == '\r') {
		line[len - 2] = '\n';
		line[len - 1] = '\0';
		return len - 1;
	}
	return len;
}


//TODO: should I treat repeated header names as an error?
//A sender MUST NOT generate multiple header fields with the same field name
//A recipient MAY combine multiple header fields with the same field name into
//one "field-name: field-value" pair by appending folling field values
//seperating with commas. Only problem is this does not work for cookies
//because they use ';' as seperator)
//see: https://arxiv.org/pdf/cs/0105018v1.pdf Appendix 2.3
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

//HTTP-name = %x48.54.54.50 ; HTTP
//HTTP-version = HTTP-name "/" DIGIT "." DIGIT
//parsers http version string ie; "HTTP/x.x", raises exception if invalid
void parseVersion(char *vs, int& major, int& minor) {
	size_t l = strlen(vs);
	if(l != 8) throw HTTPError(400);
	major = vs[5] - '0';
	minor = vs[7] - '0';
	if(major > 9 || minor > 9) throw HTTPError(400);
	vs[5] = vs[7] = 'x';
	if(strcmp(vs, "HTTP/x.x")) throw HTTPError(400);
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

Client::Client(const std::string& host, int port) : host(host),
	port(port), con(host.c_str(), port) { }

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
