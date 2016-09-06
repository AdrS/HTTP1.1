#ifndef HEADER_MAP_H
#define HEADER_MAP_H

#include <cctype>
#include <exception>
#include <map>
#include <string>
#include <utility>

class InvalidHeaderKey : std::exception {};
class InvalidHeaderValue : std::exception {};

bool tchar(char c);
bool isToken(const char *t);
bool validHeaderKey(const char *k);
//NOTE: folded header values are not supported (they are obsolete)
bool validHeaderValue(const char *v);

typedef std::map<std::string, std::string>::const_iterator HeaderMapIterator;

//This is basically a wrapper around a map ensures that all the key value pairs
//are valid and that all keys are normalized for comparison (HTTP headers are
//case insensitive). It also handles the combining of repeated headers
class HeaderMap {
	std::map<std::string, std::string> m;
public:
	//returns iterator to inserted/updated element returns true if element is new
	//if append, the value is appended to any existing value using ';' as a separator
	//for cookies and ',' for all other headers
	std::pair<HeaderMapIterator, bool>
		insert(const std::string& key, const std::string& value, bool append = true);
	//returns iterator pointing to element (or cend if element not present)
	HeaderMapIterator find(const std::string& key) const;
	//removes element with key from container, returns true if element removed
	bool erase(const std::string& key);
	HeaderMapIterator begin() const;
	HeaderMapIterator end() const;
	size_t size() const;
	bool empty() const;
};

std::string capitalize(std::string s);
std::string lowercase(std::string s);

#endif
