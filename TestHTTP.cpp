#include <iostream>
#include <cassert>
#include "HTTP.hpp"

using namespace std;
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
int main() {
	cout << "Starting HTTP tests ..." << endl;
	test_tchar();
	test_isToken();
	test_validHeaderValue();
	return 0;
}
