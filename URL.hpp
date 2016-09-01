#ifndef URL_H
#define URL_H

#include <exception>
#include <cctype>
#include <cstddef>

bool unreserved(char c);
bool subDelim(char c);
bool genDelim(char c);
bool reserved(char c);

char decodeHexChar(char c);

class InvalidPercentEncoding : std::exception {};
class InvalidHexChar : std::exception {};

//percent decodes all characters and returns new size
//Note: because %00 could be in the string, null termination is cannot be used
//throws error on invalid percent encoding
size_t percentDecode(char *str);

//Percent decodes characters in unreserved set and returns new length.
//Note: Because unreserved characters do not include '\0' this function is able
//to preserve null termination
size_t percentDecodeUnreserved(char *str);

#endif
