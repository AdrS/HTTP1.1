#include <cassert>
#include <iostream>
#include <cstring>
#include "URL.hpp"

using namespace std;

void test_unreserved() {
	cout << "Testing unreserved ..." << endl;
	const char *ur = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";
	const char *p;
	for(int i = 0; i < 256; ++i) {
		bool u = unreserved((char)i);
		for(p = ur; *p; ++p) {
			if(*p == (char)i) {
				assert(u);
				break;
			}
		}
		if(!*p) assert(!u);
	}
	cout << "PASS!!!" << endl;
}

void test_subDelim() {
	cout << "Testing subDelim ..." << endl;
	const char *sd = "!$&\'()*+,;=";
	const char *p;
	for(int i = 0; i < 256; ++i) {
		bool s = subDelim((char)i);
		for(p = sd; *p; ++p) {
			if(*p == (char)i) {
				assert(s);
				break;
			}
		}
		if(!*p) assert(!s);
	}
	cout << "PASS!!!" << endl;
}

void test_genDelim() {
	cout << "Testing genDelim ..." << endl;
	const char *gd = ":/?#[]@";
	const char *p;
	for(int i = 0; i < 256; ++i) {
		bool g = genDelim((char)i);
		for(p = gd; *p; ++p) {
			if(*p == (char)i) {
				assert(g);
				break;
			}
		}
		if(!*p) assert(!g);
	}
	cout << "PASS!!!" << endl;
}

void test_decodeHexChar() {
	cout << "Testing decodeHexChar ..." << endl;
	assert(decodeHexChar('0') == 0);
	assert(decodeHexChar('9') == 9);
	assert(decodeHexChar('a') == 10);
	assert(decodeHexChar('A') == 10);
	assert(decodeHexChar('F') == 15);
	
	try {
		decodeHexChar('?');
		assert(false);
	} catch(InvalidHexChar& e) {}
	cout << "PASS!!!" << endl;
}

void test_case(size_t (*f)(char *), const char *in, const char *out, size_t outLen) {
	char buf[1024];
	cout << "Testing on \"" << in << "\"" << endl;
	strcpy(buf, in);
	assert(f(buf) == outLen);
	buf[outLen] = '\0';
	cout << "Output \"" << buf << "\"" << endl;
	for(size_t i = 0; i < outLen; ++i) assert(buf[i] == out[i]);
}

void test_percentDecode_case(const char *in, const char *out, size_t outLen) {
	test_case(percentDecode, in, out, outLen);
}
const char *ALL_BYTES = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
void test_percentDecode() {
	cout << "Testing percentDecode ..." << endl;
	test_percentDecode_case("hello people", "hello people", strlen("hello people"));
	test_percentDecode_case("h%41llo%20people", "hAllo people", strlen("hAllo people"));
	test_percentDecode_case("hello%20people", "hello people", strlen("hello people"));
	test_percentDecode_case("%00%01%02%03%04%05%06%07%08%09%0a%0b%0c%0d%0e%0f%10%11%12%13%14%15%16%17%18%19%1a%1b%1c%1d%1e%1f%20%21%22%23%24%25%26%27%28%29%2a%2b%2c%2d%2e%2f%30%31%32%33%34%35%36%37%38%39%3a%3b%3c%3d%3e%3f%40%41%42%43%44%45%46%47%48%49%4a%4b%4c%4d%4e%4f%50%51%52%53%54%55%56%57%58%59%5a%5b%5c%5d%5e%5f%60%61%62%63%64%65%66%67%68%69%6a%6b%6c%6d%6e%6f%70%71%72%73%74%75%76%77%78%79%7a%7b%7c%7d%7e%7f%80%81%82%83%84%85%86%87%88%89%8a%8b%8c%8d%8e%8f%90%91%92%93%94%95%96%97%98%99%9a%9b%9c%9d%9e%9f%a0%a1%a2%a3%a4%a5%a6%a7%a8%a9%aa%ab%ac%ad%ae%af%b0%b1%b2%b3%b4%b5%b6%b7%b8%b9%ba%bb%bc%bd%be%bf%c0%c1%c2%c3%c4%c5%c6%c7%c8%c9%ca%cb%cc%cd%ce%cf%d0%d1%d2%d3%d4%d5%d6%d7%d8%d9%da%db%dc%dd%de%df%e0%e1%e2%e3%e4%e5%e6%e7%e8%e9%ea%eb%ec%ed%ee%ef%f0%f1%f2%f3%f4%f5%f6%f7%f8%f9%fa%fb%fc%fd%fe%ff", ALL_BYTES, 256);

	//test bad percent encodings:
	//invalid hex char
	try {
		test_percentDecode_case("%$$hello", nullptr, 0);
		assert(false);
	} catch(InvalidPercentEncoding& e) {}
	//no enought chars ex: "%A"
	try {
		test_percentDecode_case("%afhello%4", nullptr, 0);
		assert(false);
	} catch(InvalidPercentEncoding& e) {}
	cout << "PASS!!!" << endl;
}

void test_percentDecodeUnreserved_case(const char *in, const char *out, size_t outLen) {
	test_case(percentDecodeUnreserved, in, out, outLen);
}

void test_percentDecodeUnreserved() {
	cout << "Testing percentDecodeUnreserved ..." << endl;
	test_percentDecodeUnreserved_case("hello people", "hello people", strlen("hello people"));
	test_percentDecodeUnreserved_case("hello%20people", "hello%20people", strlen("hello%20people"));
	test_percentDecodeUnreserved_case("%61%62%63%64%65%66%67%68%69%6a%6b%6c%6d%6e%6f%70%71%72%73%74%75%76%77%78%79%7a%41%42%43%44%45%46%47%48%49%4a%4b%4c%4d%4e%4f%50%51%52%53%54%55%56%57%58%59%5a%30%31%32%33%34%35%36%37%38%39%2d%2e%5f%7e", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~", 26*2 + 10 + 4);
	//test bad percent encodings:
	//invalid hex char
	try {
		test_percentDecodeUnreserved_case("%$$hello", nullptr, 0);
		assert(false);
	} catch(InvalidPercentEncoding& e) {}
	//no enought chars ex: "%A"
	try {
		test_percentDecodeUnreserved_case("%afhello%4", nullptr, 0);
		assert(false);
	} catch(InvalidPercentEncoding& e) {}
	cout << "PASS!!!" << endl;
}

void percentEncode_case(const char *in, const char *out, size_t ilen, size_t olen) {
	char buf[1024];
	size_t l = percentEncode(in, buf, ilen, 3*ilen + 1);
	assert(l == olen);
	assert(strcmp(buf, out) == 0);
}
void test_percentEncode() {
	cout << "Testing percentEncode ... " << endl;
	percentEncode_case("today a camel&egg", "today%20a%20camel%26egg",
					strlen("today a camel/egg"),strlen("today%20a%20camel%26egg"));
	percentEncode_case(ALL_BYTES, "%00%01%02%03%04%05%06%07%08%09%0A%0B%0C%0D%0E%0F%10%11%12%13%14%15%16%17%18%19%1A%1B%1C%1D%1E%1F%20%21%22%23%24%25%26%27%28%29%2A%2B%2C-./0123456789%3A%3B%3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%7B%7C%7D~%7F%80%81%82%83%84%85%86%87%88%89%8A%8B%8C%8D%8E%8F%90%91%92%93%94%95%96%97%98%99%9A%9B%9C%9D%9E%9F%A0%A1%A2%A3%A4%A5%A6%A7%A8%A9%AA%AB%AC%AD%AE%AF%B0%B1%B2%B3%B4%B5%B6%B7%B8%B9%BA%BB%BC%BD%BE%BF%C0%C1%C2%C3%C4%C5%C6%C7%C8%C9%CA%CB%CC%CD%CE%CF%D0%D1%D2%D3%D4%D5%D6%D7%D8%D9%DA%DB%DC%DD%DE%DF%E0%E1%E2%E3%E4%E5%E6%E7%E8%E9%EA%EB%EC%ED%EE%EF%F0%F1%F2%F3%F4%F5%F6%F7%F8%F9%FA%FB%FC%FD%FE%FF", 256, 634);
	
	cout << "PASS!!!" << endl;
}

void test_validDomainName() {
	//for use in validating host header I do not what this to be treated as valid
	assert(!validDomainName(""));
	assert(!validDomainName("."));

	//bad chars
	assert(!validDomainName("http://en.cppreference.com"));

	//bad label(s)
	assert(!validDomainName("1asf"));
	assert(!validDomainName("-asf"));
	assert(!validDomainName("cabage.asf-.bs"));
	assert(!validDomainName("hi.as&f"));
	//too long label
	assert(!validDomainName("a.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
	//empty label
	assert(!validDomainName("hi..f"));

	//too long domain
	assert(!validDomainName("ba.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a"));

	//max length
	assert(validDomainName("a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a.a"));

	assert(validDomainName("tools.ietf.org"));
	assert(validDomainName("en.cppreference.com"));
	assert(validDomainName("e3.c--ppre1234ference.com.kAt5-3at-mice.tld"));
}

void test_validHostname() {
	assert(validHostname("www.adrianstoll.com"));
	assert(validHostname("en.wikipedia.ord"));
	assert(validHostname("127.0.0.1"));
	assert(validHostname("localhost"));
	assert(validHostname("2001:0db8:85a3:0000:0000:8a2e:0370:7334"));
	assert(validHostname("::"));

	assert(!validHostname("257.0.0.1"));
	assert(!validHostname("200H:0db8:85a3:0000:0000:8a2e:0370:7334"));
}

int main() {
	cout << "Starting URL.cpp tests ..." << endl;
	test_unreserved();
	test_subDelim();
	test_genDelim();
	test_decodeHexChar();
	test_percentDecode();
	test_percentDecodeUnreserved();
	test_percentEncode();
	test_validDomainName();
	test_validHostname();
	cout << "All tests complete." << endl;
	return 0;
}
