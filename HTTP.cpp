#include "HTTP.hpp"

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
	}
}
