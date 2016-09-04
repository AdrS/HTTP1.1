#include <iostream>
#include <cassert>
#include <cctype>
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


void test_parseHeaders() {
	const size_t BUF_SIZE = 100;
	char buf[BUF_SIZE];

	while(true) {
		Connection c("localhost", 1234);
		try {
			HeaderMap hm = parseHeaders(c, buf, BUF_SIZE);
			cout << "Headers:" << endl;
			for(auto &it : hm) {
				cout << it.first << ": " << it.second << endl;
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

void test_client_connection_mangament() {
}

void test_get() {
	Client c("localhost", 1234);
	c.headers.insert("Cookie", "uid=1234");
	c.headers.insert("User-Agent", "Adrian's HTTP client");
	Reply r = c.get("/");
	cout << "Status: " << r.status << endl;
	for(auto &i : r.headers) {
		cout << i.first << ": " << i.second << endl;
	}
}

void test_sendChunked() {
	Connection c("localhost", 1234);
	string s = "When in the course of human events, it becomes necessary for one people\nto I forget the rest. ... We the people, in order to form a more perfect union, establish justice, and ensure domestic tranquility, provide to for the general defense...";
	sendChunked(c, s.c_str(), s.length(), nullptr, 10);
	HeaderMap hm;
	hm.insert("Custom-header", "abcdefg");
	hm.insert("Cookie", "uid=1234");
	
	sendChunked(c, s.c_str(), s.length(), &hm, 20);
	c.close();
}

void test_parseChunkLen() {
	//test on invalid
	assert(parseChunkLen("\n") == -1);
	assert(parseChunkLen("ssfda\n") == -1);
	assert(parseChunkLen(" AF\n") == -1);
	assert(parseChunkLen(" AF\r\n") == -1);

	//test on plain chunk lines
	assert(parseChunkLen("AF\r\n") == 0xAF);
	assert(parseChunkLen("123\r\n") == 0x123);
	assert(parseChunkLen("123\n") == 0x123);

	//test with chunk extensions (yes I know these are invalid chunk extensions,
	//but they can be ignored so who cares)
	assert(parseChunkLen("123sfs f kas;\n") == 0x123);
	assert(parseChunkLen("AFsfs f kas;\n") == 0xAF);
}

void test_parseChunked() {
	while(true) {
		try {
			Connection c("localhost", 1234);
			char buf[1024];
			auto res = parseChunked(c, buf, 1024);

			cout << "read " << res.second << " bytes " << endl;
			for(size_t i = 0; i < res.second; ++i) {
				if(isgraph(res.first[i])) {
					cout << res.first[i];
				} else {
					cout << '.';
				}
				if(i % 40) cout << endl;
			}
			cout << endl;
		} catch(HTTPError &e) {
			cout << "HTTPError" << endl;
		} catch(ReadError &e) {
			cout << "ReadError" << endl;
		}
	}
}

struct RL_case {
	string method;
	string target;
	bool pe;
};
void test_sendRequestLine() {
	RL_case tests[] = {
		{"GET", "\x7\x1\x1/index.html", true},
		{"GET", "/index.html", true},
		{"GET", "/Camels are ???.html ", true},
		{"GET", "/Camels are ???.html ", true},
		{"GET", "/index.html", false},
		{"OPTIONS", "/index.html", true},
		{"OPTIONS", "***********", true},
		{"OPTIONS", "*", true},
		{"OPTIONS", "*", false}};
	tests[0].target[1] = '\0'; //workaround to put null in middle of string
	char next;
	for(size_t i = 0; i < 9; ++i) {
		cout << "Next? ";
		cin >> next;
		try {
			Connection c("localhost", 1234);
			sendRequestLine(c, tests[i].method, tests[i].target, tests[i].pe);
			c.close();
		} catch (...) {
			cout << "Error" << endl;
		}
	}
}

int main() {
	cout << "Starting HTTP tests ..." << endl;
	test_normalizeLineEnding();
//	test_parseHeaders();
	test_parseVersion();
	test_client_connection_mangament();
//	test_get();
//	test_sendChunked();
	test_parseChunkLen();
//	test_parseChunked();
	test_sendRequestLine();
	return 0;
}
