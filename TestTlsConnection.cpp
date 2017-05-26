#include <stdio.h>
#include <string.h>
#include "TlsConnection.hpp"

int main(const int argc, const char **argv) {
	if(argc != 3) {
		fprintf(stderr, "usage: %s <host> <path>\n", argv[0]);
		exit(1);
	}
	const char *host = argv[1];

	//ignore https:// prefix if present
	if(!strncmp(argv[1], "https://", strlen("https://"))) {
		host += strlen("https://");
	}

	TlsConnection con(host, 443);
	char buf[1024];
	int n = snprintf(buf, 1024,
		"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
		argv[2], host);
	con.send(buf,n);
	while((n = con.recv(buf,1023))) {
		buf[n] = '\0';
		printf("%s", buf);
	}
	return 0;
}
