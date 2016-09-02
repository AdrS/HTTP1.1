#include <iostream>
#include <cassert>
#include "HTTP.hpp"

using namespace std;

void test_normalizeLineEnding() {
	char buf[1024];
	strcpy(buf, "\n");
	assert(normalizeLineEnding(buf, 1) == 1);
	assert(strcmp(buf, "\n") == 0);
	strcpy(buf, "\r\n");
	assert(normalizeLineEnding(buf, 2) == 1);
	assert(strcmp(buf, "\n") == 0);
	strcpy(buf, "123456789\r\n");
	assert(normalizeLineEnding(buf, 11) == 10);
	assert(strcmp(buf, "123456789\n") == 0);
	strcpy(buf, "123456789\n");
	assert(normalizeLineEnding(buf, 10) == 10);
	assert(strcmp(buf, "123456789\n") == 0);
}

void test_tchar() {
	cout << "testing tchar ..." << endl;

	const char *tc = "!#$%&'*+-.^_`|~abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const char *p;
	for(int i = 0; i < 256; ++i) {
		bool t = tchar((char)i);
		for(p = tc; *p; ++p) {
			if(*p == (char)i) {
				assert(t);
				break;
			}
		}
		if(!*p) assert(!t);
	}
	cout << "PASS!!!" << endl;
}

void test_isToken() {
	cout << "testing isToken ..." << endl;
	assert(isToken("hello"));
	assert(isToken("$df1"));

	assert(!isToken(""));
	assert(!isToken("asdf sd"));
	assert(!isToken("asdfsd "));
	cout << "PASS!!!" << endl;
}

void test_validHeaderValue() {
	cout << "testing validHeaderValue ..." << endl;
	assert(validHeaderValue("hello"));
	assert(validHeaderValue("sdfkj;alsk jlkfa8098709234 *6789568214ac sjdf s"));
	assert(validHeaderValue("sdf\t\t\t   s"));

	assert(!validHeaderValue(""));
	assert(!validHeaderValue(" asdf"));
	assert(!validHeaderValue("asdf "));
	assert(!validHeaderValue("\tasdf"));
	assert(!validHeaderValue("as \ndf"));
	assert(!validHeaderValue("as \rdf"));
	assert(!validHeaderValue("as \bdf"));
	assert(!validHeaderValue("as \007df"));
	cout << "PASS!!!" << endl;
}

void test_parseHeaders() {
	const size_t BUF_SIZE = 100;
	char buf[BUF_SIZE];

	while(true) {
		Connection c("localhost", 1234);
		try {
			HeaderMap hm = parseHeaders(c, buf, BUF_SIZE);
			cout << "Headers:" << endl;
			for(auto&& i : hm) {
				cout << i.first << ": " << i.second << endl;
			}
		} catch(...) {
			cout << "Error ... " << endl;
		}
	}
}

void test_parseVersion() {
	char buf[32];
	int major, minor;
	strcpy(buf, "HTTP/0.9");
	parseVersion(buf, major, minor);
	assert(major == 0 && minor == 9);

	strcpy(buf, "HTTP/1.1");
	parseVersion(buf, major, minor);
	assert(major == 1 && minor == 1);

	strcpy(buf, "HTTP/0.a");
	try {
		parseVersion(buf, major, minor);
		assert(false);
	} catch(HTTPError &e) {}

	strcpy(buf, "HttP/1.1");
	try {
		parseVersion(buf, major, minor);
		assert(false);
	} catch(HTTPError &e) {}

	strcpy(buf, "H.1");
	try {
		parseVersion(buf, major, minor);
		assert(false);
	} catch(HTTPError &e) {}
}

void test_lower() {
	assert(lower("Adrian") == "adrian");
	assert(lower("CAMELS") == "camels");
	assert(lower("C++0x") == "c++0x");
}

int main() {
	cout << "Starting HTTP tests ..." << endl;
	test_normalizeLineEnding();
	test_tchar();
	test_isToken();
	test_validHeaderValue();
//	test_parseHeaders();
	test_parseVersion();
	test_lower();
	return 0;
}
