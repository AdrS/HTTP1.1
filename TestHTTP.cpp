#include <iostream>
#include <cassert>
#include <cctype>
#include <utility>
#include "HTTP.hpp"

using namespace std;

//TODO: THIS is duplicated code :(
int openTCP(const char *host, int port) {
	//do domain name lookup
	addrinfo hints;
	addrinfo *res, *rp;
	int fd;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	//Want IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;//Want TCP
	//all other options are set to default (because of the memset)

	if(getaddrinfo(host, nullptr, &hints, &res)) {
		//TODO: add more details to exception, see gai_strerror()
		throw NameResolutionError();
	}

	//try connecting to the returned addresses
	for(rp = res; rp != nullptr; rp = rp->ai_next) {
		fd = socket(rp->ai_family, SOCK_STREAM , rp->ai_protocol);
		//fill out port (TODO: could have just filled out the serv param to
		//					getaddrinfo)
		if(rp->ai_family == AF_INET) {
			sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(rp->ai_addr);
			addr->sin_port = htons(port);
		} else {
			sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(rp->ai_addr);
			addr->sin6_port = htons(port);
		}
		if(fd == -1) continue;
		if(::connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
	}

	//getting to the end of the list means no connection could be made
	if(rp == nullptr) {
		fd = -1;
		throw ConnectionEstablishmentError();
	}

	freeaddrinfo(res);
	return fd;
}

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
	/* method this is testing is private now
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
			Client c("localhost", 1234);
			c.sendRequestLine(tests[i].method, tests[i].target, tests[i].pe);
			c.disconnect();
		} catch (...) {
			cout << "Error" << endl;
		}
	} */
}

void test_reasonPhrase() {
	//check that defaulting to x00 works
	assert(INFO[0] == reasonPhrase(199));
	assert(SUCCESS[0] == reasonPhrase(299));
	assert(REDIRECTION[0] == reasonPhrase(399));
	assert(CLIENT_ERROR[0] == reasonPhrase(499));
	assert(SERVER_ERROR[0] == reasonPhrase(599));

	assert(!reasonPhrase(99));
	assert(!reasonPhrase(600));

	assert(INFO[1] == reasonPhrase(101));
	assert(SUCCESS[4] == reasonPhrase(204));
	assert(REDIRECTION[3] == reasonPhrase(303));
	assert(CLIENT_ERROR[14] == reasonPhrase(414));
	assert(SERVER_ERROR[0] == reasonPhrase(500));
}

void test_sendStatusLine() {
	//writing move did not work
	ClientConnection c(openTCP("localhost",1234));
	try {
		c.sendStatusLine(900, "yoyo");
		assert(false);
	} catch(exception &e) {
	}
	c.sendStatusLine(123, "My custom reason phrase");
	c.sendStatusLine(404);
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
	test_reasonPhrase();
	test_sendStatusLine();
	return 0;
}
