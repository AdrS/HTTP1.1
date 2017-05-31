#ifndef TLS_CONNECTION_HPP
#define TLS_CONNECTION_HPP

#include "BaseConnection.hpp"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

class TlsConnection : public BaseConnection {
public:
	TlsConnection(const char* host, int port);
	virtual ~TlsConnection();
	//close existing connection and opens a new one
	virtual void connect(const char* host, int port);
	//read up to n bytes into buf and returns the number of bytes read
	virtual size_t recv(char *buf, size_t n);
	//sends first n bytes in buffer
	virtual void send(const char *buf, size_t n);
	//closes the underlying connection + cleans up connection resources
	virtual void close();
private:
	//loads cipher info into interal tables + loads error strings
	static void init_openssl();
	//here is where the "magic" happens
	void setupConnection(const char* host, int port);

	//TODO: replace this
	// if condition is false, prints out openssl error queue //and exits
	void ensure(bool cond);

	static bool openssl_loaded;
	const static SSL_METHOD *method;

	SSL_CTX *ctx;
	SSL *ssl;
	BIO *bio;
};

//TODO: add tls specific exceptions

#endif
