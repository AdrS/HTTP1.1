#include "HTTP.hpp"

using std::string; using std::pair; using std::make_pair;

//NOTE: this assumes line end in '\n' and might fail if this is not the case
size_t normalizeLineEnding(char *line, size_t len) {
	if(len > 1 && line[len - 2] == '\r') {
		line[len - 2] = '\n';
		line[len - 1] = '\0';
		return len - 1;
	}
	return len;
}

//tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
//    "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
//TODO: lookup tables would be faster
bool tchar(char c) {
	return isalnum(c) || c == '!' || ('#' <= c && c <= '\'') || c == '*' ||
			c == '+' || c == '-' || c == '.' || c == '^' || c == '_' ||
			c == '`' || c == '|' || c == '~';
}

//token = 1*tchar
bool isToken(const char *t) {
	//empty string => not token
	if(!*t) return false;
	//all characters should be tchars
	while(*t) if(!tchar(*t++)) return false;
	return true;
}

bool validHeaderKey(const char *k) {
	return isToken(k);
}

//NOTE: this does not support obsolete folded headers
//valid header values must begin and end with visible characters and
//include only visible characters, spaces, and tabs ie: '\t'
bool validHeaderValue(const char *v) {
	//check that first char is visible (note: this also rules out empty values)
	if(!isgraph(*v++)) return false;
	while(*v) {
		if(!(*v == ' ' || *v == '\t' || isgraph(*v))) return false;
		++v;
	}
	//check that last character is visible
	return isgraph(*(v - 1));
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
		if(!validHeaderKey(buf)) throw HTTPError(400);

		//Header keys are case insensitive (values may or may not be)
		//Normalize header keys to lowercase
		//TODO: lots of broken software does case sensitive comparisons
		//	and experts word to be capitalized
		//TODO: write a capitalize function
		for(char *i = buf; *i; ++i) *i = tolower(*i);

		string key(buf);

		///////////PARSE HEADER VALUE///////////////
		//skip over whitespace
		while(*pos == ' ' || *pos == '\t') ++pos;

		//store last non whitespace for trailing OWS removal
		char *lastNonWS = pos, *hf = pos;
		while(*pos != '\n') {
			if(*pos != ' ' && *pos != '\t') lastNonWS = pos;
			++pos;
		}
		lastNonWS[1] = '\0';
		if(!validHeaderValue(hf)) throw HTTPError(400);

		string value(hf);

		///////////HANDLE COMBINING OF HEADERS/////
		auto res = hm.insert(make_pair(key, value));
		//if header key already exists, then combine values
		if(!res.second) {
			auto it = res.first;
			//if cookie header, use ';' as seperator
			if(it->first == "set-cookie" || it->first == "cookie") {
				it->second.append(1,';');
			} else {
				it->second.append(1,',');
			}
			it->second.append(value);
		}
	}
	return hm;
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

Reply::Reply(int status) : status(status), body(nullptr), length(0) {
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
		port(port), con(host.c_str(), port) {
	//Host is required for all HTTP/1.1 requests so lets add it
	setHeader("host", host, false);
}

void Client::disconnect() {
	con.close();
}

void Client::reconnect() {
	con.connect(host.c_str(), port);
}

void Client::reconnect(const std::string& host, int port) {
	this->host = host;
	this->port = port;
	//new host => must change header
	setHeader("host", this->host, false);
	con.connect(this->host.c_str(), port);
}

//return true only if header already present
//if key is Cookie or Set-Cookie, ';' is used as delimiter. Otherwise ',' is used.
bool Client::setHeader(const std::string& key, const std::string& value, bool append) {
	return false;
}

//If removes header and return true (if header present).
bool Client::removeHeader(const std::string& key) {
	return false;
}

//all the methods
Reply Client::get(const std::string& target) {
	return Reply(400);
}

Reply Client::head(const std::string& target) {
	return Reply(400);
}

Reply Client::post(const std::string& target, char *body, size_t length) {
	return Reply(400);
}

//fetch options for a specific target
Reply Client::options(const std::string& target) {
	return Reply(400);
}

//fetch global options ie: "OPTIONS * HTTP/1.1\r\n"
Reply Client::options() {
	return options("*");
}
