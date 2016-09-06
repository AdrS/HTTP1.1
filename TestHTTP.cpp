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

void test_client_connection_mangament() {
}

void test_head() {
	while(true) {
		string host;
		string target;
		cout << "Enter host and target: ";
		cin >> host >> target;
		try {
			Client c(host);
			c.headers.insert("Cookie", "uid=1234");
			c.headers.insert("User-Agent", "Adrian's HTTP client");
			c.headers.insert("Connection", "close");
			Reply r = c.head(target);
			cout << "Status: " << r.status << endl;
			for(auto &i : r.headers) {
				cout << i.first << ": " << i.second << endl;
			}
		} catch(...) {
			cout << "Some error" << endl;
		}
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
				if(i % 40 == 39) cout << endl;
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

void test_parseStatusLine() {
	while(true) {
		try {
			Client c("localhost", 1234);
			string rf;
			int status = c.parseStatusLine(rf);
			cout << "status: " << status  << "(" << rf << ")" << endl;
		} catch(HTTPError &e) {
			cout << "HTTPError " << e.status << endl;
		}
	}
}

void test_parseRequestLine() {
	while(true) {
		try {
			ClientConnection c(openTCP("localhost",1234));
			string method;
			string target;
			c.parseRequestLine(method, target);
			cout << method << " \"";
			for(auto &c : target) {
				if(isgraph(c)) cout << c;
				else cout << "\\x" << hex << (int)c << dec;
			}
			cout << "\"" << endl;
		} catch(HTTPError &e) {
			cout << "HTTPError " << e.status << endl;
		} catch(InvalidPercentEncoding &e) {
			cout << "Invalid percent encoding" << endl;
		}
	}
}

void test_get() {
	while(true) {
		string host;
		string target;
		cout << "Enter host and target: ";
		cin >> host >> target;
		try {
			Client c(host);
			c.headers.insert("User-Agent", "Adrian's HTTP client");
			c.headers.insert("Connection", "close");
			Reply r = c.get(target);
			cout << "Status: " << r.status << endl;
			for(auto &i : r.headers) {
				cout << i.first << ": " << i.second << endl;
			}
			if(r.length) {
				cout << "Body: " << endl;
				for(size_t i = 0; i < r.length; ++i) {
					char c = r.body[i];
					if(isgraph(c)) cout << c;
					else cout << "\\x" << hex << (int)c << dec;
				}
			} else {
				cout << "No body" << endl;
			}
			cout << "==================================================" << endl;
		} catch(...) {
			cout << "Some error" << endl;
		}
	}
}

void test_post() {
	while(true) {
		string host;
		string target;
		string payload;
		cout << "Enter host, target, payload: ";
		cin >> host >> target >> payload;
		try {
			Client c(host);
			c.headers.insert("User-Agent", "Adrian's HTTP client");
			c.headers.insert("Connection", "close");
			Reply r = c.post(target, payload.c_str(), payload.length());
			cout << "Status: " << r.status << endl;
			for(auto &i : r.headers) {
				cout << i.first << ": " << i.second << endl;
			}
			if(r.length) {
				cout << "Body: " << endl;
				for(size_t i = 0; i < r.length; ++i) {
					char c = r.body[i];
					if(isgraph(c)) cout << c;
					else cout << "\\x" << hex << (int)c << dec;
				}
			} else {
				cout << "No body" << endl;
			}
			cout << "\n==================================================" << endl;
		} catch(...) {
			cout << "Some error" << endl;
		}
	}
}

int main() {
	cout << "Starting HTTP tests ..." << endl;
	test_normalizeLineEnding();
//	test_parseHeaders();
	test_client_connection_mangament();
//	test_head();
//	test_sendChunked();
	test_parseChunkLen();
//	test_parseChunked();
	test_sendRequestLine();
	test_reasonPhrase();
//	test_sendStatusLine();
//	test_parseStatusLine();
//	test_parseRequestLine();
//	test_get();
	test_post();
	return 0;
}
