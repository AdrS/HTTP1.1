#ifndef URL_H
#define URL_H

#include <exception>
#include <cctype>
#include <cstddef>
#include <cassert>

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

//percent encodes all characters except unreserved ones and '/', length of new string
// (not including null terminator) is returned.
//NOTE: the reason there are two len parameters is to remind the caller that
//  it is required that len(out) >= 3*len(in) + 1, so that there is enough
//	space in the event that all characters must be percent encoded. The extra
//	byte is for null termination and output can be treated as normal char*
size_t percentEncode(const char *in, char *out, size_t ilen, size_t olen);

#endif
