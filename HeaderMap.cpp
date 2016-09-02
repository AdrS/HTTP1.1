#include "HeaderMap.hpp"

using std::string; using std::pair; using std::make_pair;

//TODO: could have this take a reference to string
string capitalize(string s) {
	bool cap = true;
	for(auto i = s.begin(); i != s.end(); ++i) {
		if(isalpha(*i)) {
			if(cap) {
				*i = toupper(*i);
				cap = false;
			} else {
				*i = tolower(*i);
			}
		} else {
			cap = true;
		}
	}
	return s;
}

//tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
//    "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
//TODO: lookup tables would be faster (maybe)
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

//returns iterator to inserted/updated element returns true if element is new
//if append, the value is appended to any existing value using ';' as a separator
//for cookies and ',' for all other headers
pair<HeaderMapIterator, bool> HeaderMap::insert(const string& key, const string& value, bool append) {
	if(!validHeaderKey(key.c_str())) throw InvalidHeaderKey();
	if(!validHeaderValue(value.c_str())) throw InvalidHeaderValue();

	//normalize all keys (in http: Host: _blah_ <==> host: _blah_ <==> hOSt: _blah_
	string nKey = capitalize(key);
	
	auto res = m.insert(make_pair(nKey, value));
	
	///////////HANDLE COMBINING OF HEADERS/////
	//if header key already exists
	if(!res.second) {
		if(append) {
			auto it = res.first;
			//if cookie header, use ';' as seperator
			if(it->first == "Set-Cookie" || it->first == "Cookie") {
				it->second.append(1,';');
			} else {
				it->second.append(1,',');
			}
			it->second.append(value);
		} else {
			//overwrite existing value
			res.first->second = value;
		}
	}
	return res;
}

//returns iterator pointing to element (or cend if element not present)
HeaderMapIterator HeaderMap::find(const string& key) const {
	return m.find(capitalize(key));
}

//removes element with key from container, returns true if element removed
bool HeaderMap::erase(const string& key) {
	return m.erase(capitalize(key)) > 0;
}

HeaderMapIterator HeaderMap::begin() const {
	return m.cbegin();
}

HeaderMapIterator HeaderMap::end() const {
	return m.cend();
}

size_t HeaderMap::size() const {
	return m.size();
}

bool HeaderMap::empty() const {
	return m.empty();
}
