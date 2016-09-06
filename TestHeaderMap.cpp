#include <iostream>
#include <cassert>
#include "HeaderMap.hpp"

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

void test_capitalize() {
	assert(capitalize("Adrian") == "Adrian");
	assert(capitalize("aDrian") == "Adrian");
	assert(capitalize("iPhone 6") == "Iphone 6");
	assert(capitalize("c++1y") == "C++1Y");
}

void test_lowercase() {
	assert(lowercase("Adrian") == "adrian");
	assert(lowercase("adrian") == "adrian");
	assert(lowercase("aDRian") == "adrian");
	assert(lowercase("38 ADRIANS!!!") == "38 adrians!!!");
}

void test_insert() {
	HeaderMap hm;
	//test on invalid key, value
	try {
		auto r = hm.insert("Yogurt ", "peach");
		cout << r.second << endl;	//just to get rid of unused var warning
		assert(false);
	} catch (InvalidHeaderKey &e) {
		assert(hm.empty() && hm.size() == 0);
	}
	try {
		auto r = hm.insert("Yogurt", "camel\nflavor");
		cout << r.second << endl;	//just to get rid of unused var warning
		assert(false);
	} catch (InvalidHeaderValue &e) {
		assert(hm.empty() && hm.size() == 0);
	}

	//test insertion of new element
	auto r = hm.insert("cookie", "uid=1234");
	assert(r.second && r.first->first == "Cookie" && r.first->second == "uid=1234");
	assert(!hm.empty() && hm.size() == 1);

	//test appending
	r = hm.insert("Cookie", "url=/home");
	assert(!r.second && r.first->first == "Cookie" && r.first->second == "uid=1234;url=/home");
	assert(!hm.empty() && hm.size() == 1);

	//test overwriting
	r = hm.insert("coOkIe", "uid=4321", false);
	assert(!r.second && r.first->first == "Cookie" && r.first->second == "uid=4321");
	assert(!hm.empty() && hm.size() == 1);

	//test adding multiple
	r = hm.insert("Host", "www.bbc.co.uk");
	assert(hm.size() == 2);
}

void test_find() {
	HeaderMap hm;
	hm.insert("Host", "www.google.com");
	hm.insert("Cookie", "id=1234;lang=en");
	hm.insert("User-Agent", "???");

	//iterators work
	for(auto &i : hm) {
		cout << i.first << ": " << i.second << endl;
	}

	auto it = hm.find("Host");
	assert(it->first == "Host" && it->second == "www.google.com");

	//check that search is case insensitive
	assert(hm.find("user-agent") == hm.find("user-Agent"));

	//test search for nonexisting element
	assert(hm.find("Keep-alive") == hm.end());
}

void test_erase() {
	HeaderMap hm;
	hm.insert("Host", "www.google.com");
	hm.insert("Cookie", "id=1234;lang=en");
	hm.insert("User-Agent", "???");

	assert(hm.size() == 3);

	//erase nonexistant
	assert(!hm.erase("erase"));
	
	assert(hm.erase("cookie") && hm.size() == 2);
	assert(!hm.erase("cookie") && hm.size() == 2);
}

int main() {
	cout << "Testing HeaderMap..." << endl;
	test_tchar();
	test_isToken();
	test_validHeaderValue();
	test_capitalize();
	test_lowercase();
	test_insert();
	test_find();
	test_erase();
	return 0;
}
