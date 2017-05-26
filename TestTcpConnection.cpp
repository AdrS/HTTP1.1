#include "TcpConnection.hpp"

#include <iostream>
#include <string>
#include <cassert>
#include <unistd.h>

using std::endl; using std::cout; using std::cin; using std::string;

const int BUF_SIZE = 1024;

void test_Connection_from_host_and_port() {
	cout << "Testing TcpConnection(host, port) ..." << endl;

	//test that it throws error on bad domain name or port
	try {
		TcpConnection c("kdsfdjasdfasdf.this.domain.not.exist", 80);
		assert(false);
	} catch(NameResolutionError &e) {}
	try {
		TcpConnection c("thishas\n%#4@Fbad chars", 80);
		assert(false);
	} catch(NameResolutionError &e) {}
	try {
		TcpConnection c("localhost", -324);
		assert(false);
	} catch(ConnectionEstablishmentError &e) {}

	cout << "Connecting to localhost" << endl;
	//FIXME: Have to actually have so a server listening here
	cout << "@ port 1234..." << endl;
	TcpConnection c("localhost", 1234);
	cout << "@ port 1235..." << endl;
	TcpConnection c2("127.0.0.1", 1235);
	cout << "Pass!!!" << endl;
}


void test_send() {
	cout << "Testing TcpConnection::send(buf, n) ..." << endl;
	
	TcpConnection c("localhost", 1234);
	c.send("12345678901234567890", 21);
	sleep(20);
	char buf[20000];
	c.send(buf, 20000);
	c.close();
	cout << "Pass??" << endl;
}

void test_recv() {
	cout << "Testing TcpConnection::recv(buf, n) ..." << endl;
	char buf[BUF_SIZE];
	TcpConnection c("localhost", 1234);
	//small reads
	for(size_t i = 0; i < 10; ++i) {
		cout << "Reading " << i << "..." << endl;
		size_t r = c.recv(buf, i);
		buf[r] = '\0';
		cout << "\"" <<  buf << "\"" << endl;
		cout << "Read " << r << endl;
	}
	//large reads
	for(int i = 0; i < 3; ++i) {
		cout << "Reading ..." << endl;
		size_t r = c.recv(buf, BUF_SIZE - 1);
		buf[r] = '\0';
		cout << "\"" <<  buf << "\"" << endl;
		cout << "Read " << r << endl;
	}
}

void test_connect() {
	char buf[1024];
	size_t len = 0;
	string host = "localhost";
	int port = 1234;
	TcpConnection c(host.c_str(), port);
	bool connected = true;
	while(true) {
		while(true) {
			c.send("ping\n", 5);
			try {
				len = c.recv(buf, 10);
				buf[len] = '\0';
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
	test_send();
	return 0;
}
/*
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <sys/select.h>
#include "TcpConnection.hpp"

const int BUF_SIZE = 1024;
int main(const int argc, const char **argv) {
	if(argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(1);
	}
	TcpConnection con(argv[1], atoi(argv[2]));

	//This is bad code ??
	char rbuf[BUF_SIZE];
	char sbuf[BUF_SIZE];
	int s_len = 0, r_len = 0;
	int cfd = con.getFileDescriptor();

	//listen on stdin + connection
	fd_set read_set;
	fd_set write_set;
	FD_ZERO(&read_set);

	FD_ZERO(&write_set);

	//because file descriptors are assigned sequential con will have largest
	int max_fd = con.getFileDescriptor();
	for(;;) {
		//listen on stdin + connection
		FD_SET(0, &read_set);
		FD_SET(cfd, &read_set);

		//write to stdout + connection
		FD_SET(cfd, &write_set);

		int ret = select(max_fd + 1, &read_set, &write_set, nullptr, nullptr);
		if(ret == -1) {
			perror("select()");
		}
		//able to write to connection and have data to write
		if(FD_ISSET(cfd, &write_set) && s_len) {
			con.send(sbuf, s_len);
			s_len = 0;
		}
		//something to read from connection
		if(FD_ISSET(con.getFileDescriptor(), &read_set)) {
			r_len = con.recv(rbuf, BUF_SIZE - 1);
			rbuf[r_len] = '\0';
			//because this will be terminal, writings is fast
			//	=> blocking does not matter
			printf("%s", rbuf);
		}
		//something to read from user and space in buffer
		if(FD_ISSET(0, &read_set) && s_len != BUF_SIZE) {
			//append to send buffer
			r_len = read(0, sbuf, BUF_SIZE - s_len);
			if(r_len == -1) {
				perror("read()");
			}
			s_len += r_len;
		}
	}
	return 0;
}
*/
