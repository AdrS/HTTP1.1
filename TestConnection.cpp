#include <iostream>
#include <string>
#include <cassert>
#include "Connection.hpp"

using std::endl; using std::cout; using std::cin; using std::string;

void test_Connection_from_fd() {
	cout << "Testing Connection(int fd) ..." << endl;

	//this should work
	int fd1 = socket(AF_INET, SOCK_STREAM, 0);
	Connection c(fd1);

	//this should not (assuming stdin is not a socket)
	try {
		Connection(0);
		assert(false); //should raise exception if fd is not a socket
	} catch(BadFileDescriptorError& e) { }

	cout << "Pass!!!" << endl;
}

void test_Connection_from_host_and_port() {
	cout << "Testing Connection(host, port) ..." << endl;

	//test that it throws error on bad domain name or port
	try {
		Connection c("kdsfdjasdfasdf.this.domain.not.exist", 80);
		assert(false);
	} catch(NameResolutionError &e) {}
	try {
		Connection c("thishas\n%#4@Fbad chars", 80);
		assert(false);
	} catch(NameResolutionError &e) {}
	try {
		Connection c("localhost", -324);
		assert(false);
	} catch(ConnectionEstablishmentError &e) {}

	cout << "Connecting to localhost @ port 1234..." << endl;
	//FIXME: Have to actually have so a server listening here
	Connection c("localhost", 1234);
	Connection c2("127.0.0.1", 1234);
	cout << "Pass!!!" << endl;
}

void test_readChar() {
	cout << "Testing Connection::readChar() ..." << endl;
	Connection c("localhost", 1234);
	for(size_t i = 0; i < BUF_SIZE + 1; ++i) {
		cout << c.readChar() << endl;
	}
}

void test_read_buf() {
	cout << "Testing Connection::read(buf, n) ..." << endl;
	char buf[3*BUF_SIZE];
	Connection c("localhost", 1234);
	for(size_t i = 0; i < 10; ++i) {
		cout << "Reading " << i << "..." << endl;
		size_t r = c.read(buf, i);
		buf[r] = '\0';
		cout << "\"" <<  buf << "\"" << endl;
		cout << "Read " << r << endl;
	}
	for(int i = 0; i < 3; ++i) {
		cout << "Reading ..." << endl;
		size_t r = c.read(buf, 3*BUF_SIZE - 1);
		buf[r] = '\0';
		cout << "\"" <<  buf << "\"" << endl;
		cout << "Read " << r << endl;
	}
}

void test_readLine() {
	char buf[1024];
	Connection c("localhost", 1234);
	for(int i = 0; i < 4; ++i) {
		size_t l = c.readLine(buf, 32);
		cout << "read " << l << " characters \"" << buf << "\"" << endl;
	}
}

void test_send_char() {
	Connection c("localhost", 1234);
	//NOTE: I changed BUF_SIZE to be small to make testing easier
	for(int i = 0; i < 20; ++i) {
		c.sendChar('a');
	}
	c.sendChar('\n');
}

void test_sendLine() {
	Connection c("localhost", 1234);
	string input;
	while(true) {
		cin >> input;
		if(input == "quit") break;
		c.sendLine(input.c_str());
	}
}

void test_connect() {
	char buf[1024];
	size_t len = 0;
	string host = "localhost";
	int port = 1234;
	Connection c(host.c_str(), port);
	bool connected = true;
	while(true) {
		while(true) {
			strcpy(buf, "ping");
			c.sendLine(buf, 4);
			try {
				len = c.readLine(buf, 1024);
			} catch(ReadError &e) {
				cout << "Read error, closing connection" << endl;
				c.close();
				break;
			}
			if(len == 0) {
				cout << "Connection closed" << endl;
				break;
			}
			cout << "Read " << len  << " bytes from " << host << ":" << port << endl;
			cout << buf << endl;
		}
		connected = false;
		while(!connected) {
			do {
				cout << "Enter new host and port: ";
				cin >> host >> port;
				if(host == "quit") return;
			} while(port < 1 || port > (2 << 16) - 1);
			try {
				c.connect(host.c_str(), port);
				connected = true;
			} catch(NameResolutionError &e) {
				cout << "Name resolution error" << endl;
			} catch(ConnectionEstablishmentError &e) {
				cout << "Connection Establishment Error" << endl;
			}
		}
	}
}

int main() {
	cout << "Starting connection tests..." << endl;
	test_connect();
	return 0;
}
