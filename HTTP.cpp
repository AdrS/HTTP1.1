#include "HTTP.hpp"

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
