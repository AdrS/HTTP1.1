#include "URL.hpp"
#include <cstdint>

//unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
bool unreserved(char c) {
	return isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

//sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
bool subDelim(char c) {
	return c == '!' || c == '$' || ('&' <= c && c <= ',') || c == ';' || c == '=';
}

//gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
bool genDelim(char c) {
	return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@';
}

//reserved = gen-delims / sub-delims
bool reserved(char c) {
	return genDelim(c) || subDelim(c);
}

//takes character 0-9 | a-f | A-F and returns corrisponding hex value
char decodeHexChar(char c) {
	if('0' <= c && c <= '9') return c - '0';
	if('A' <= c && c <= 'F') return c - 'A' + 10;
	if('A' <= c && c <= 'f') return c - 'a' + 10;
	throw InvalidHexChar();
	return 'a';
}

//assumes string is null terminated
size_t percentDecode(char *str) {
	char *outPos = str, *inPos = str;
	while(*inPos) {
		//if percent encoded character found
		//cout << "str: " << str << " inPos " << inPos << " outPos " << outPos << endl;
		if(*inPos == '%') {
			//check that encoding is valid
			if(!(isxdigit(inPos[1]) && isxdigit(inPos[2]))) throw InvalidPercentEncoding();

			*outPos++ = decodeHexChar(inPos[1]) * 16 + decodeHexChar(inPos[2]);
			inPos += 3;
		} else {
			*outPos++ = *inPos++;
		}
	}
	//return new str length
	return outPos - str;
}

//assumes string is null terminated
size_t percentDecodeUnreserved(char *str) {
	char *outPos = str, *inPos = str;
	char c;
	while(*inPos) {
		//if percent encoded character found
		if(*inPos == '%') {
			//check that encoding is valid
			if(!(isxdigit(inPos[1]) && isxdigit(inPos[2]))) throw InvalidPercentEncoding();

			c = decodeHexChar(inPos[1]) * 16 + decodeHexChar(inPos[2]);
			//only decode if c is unreserved
			if(unreserved(c)) {
				*outPos++ = c;
				inPos += 3;
			} else {
				//otherwise just leave existing percent encoding
				*outPos++ = *inPos++;
				*outPos++ = *inPos++;
				*outPos++ = *inPos++;
			}
		} else {
			*outPos++ = *inPos++;
		}
	}
	//make sure to null terminate output
	*outPos = '\0';
	//return new str length
	return outPos - str;
}

static char nib2hex(uint8_t nib) {
	return nib < 10 ? nib + '0' : nib - 10 + 'A';
}

size_t percentEncode(const char *in, char *out, size_t ilen, size_t olen) {
	//TODO: still not sure which characters to exclude from percent encoding
	//	could add safe chars option like python urllib.quote
	//	ex: urlib.quote treats '/' as safe by default, so I will too
	assert(olen >= 3*ilen + 1);
	char *opos = out;
	for(size_t i = 0; i < ilen; ++i) {
		//do not percent encode unreserved characters
		if(unreserved(in[i]) || in[i] == '/') {
			*opos++ = in[i];
		} else {
			*opos++ = '%';
			*opos++ = nib2hex((uint8_t)in[i] >> 4);
			*opos++ = nib2hex((uint8_t)in[i] & 15);
		}
	}
	//null terminate so printing is easy
	*opos = '\0';
	return opos - out;
}

/**********************************************************************
                     GRAMMER FOR HTTP URLS
 **********************************************************************
request-target = origin-form
                    / absolute-form
                    / authority-form
                    / asterisk-form
//NOTE: ie only plan on supporting origin and asterisk form (for now)
//for all direct requests (ie: not throught proxy) except CONNECT and OPTIONS
origin-form    = absolute-path [ "?" query ]

//for making requests to proxy other than CONNECT and OPTIONS
absolute-form  = absolute-URI

//only for CONNECT requests
authority-form = authority

//only for OPTIONS
asterisk-form  = "*"

absolute-path = 1*( "/" segment )
segment       = *pchar
pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

query = *( pchar / "/" / "?" )

Host = uri-host [ ":" port ]
**********************************************************************/
