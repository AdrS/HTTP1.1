#include "TlsConnection.hpp"

#include <cassert>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

static const char *CA_STORE_PATH = "/home/adrian/Documents/openssl_demo/certs/cacerts.pem";

static const char *CIPHER_PREFERENCES = "kEECDH+ECDSA:kEECDH:kEDH:HIGH:!RC4:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!DSS:!PSK:!SRP:!kECDH:!CAMELLIA:!IDEA:!SEED";

bool TlsConnection::openssl_loaded = false;
const SSL_METHOD* TlsConnection::method = nullptr;

TlsConnection::TlsConnection(const char *host, int port) : ctx(nullptr),
	ssl(nullptr), bio(nullptr) {
	if(!openssl_loaded) {
		init_openssl();
	}
	connect(host, port);
}

TlsConnection::~TlsConnection() {
	close();
}

void TlsConnection::connect(const char *host, int port) {
	close();
	setupConnection(host, port);
}

size_t TlsConnection::recv(char *buf, size_t len) {
	//returns amount of data read
	assert(buf);
	assert(bio);

	size_t left = len;
	int r;
	do {
		r = BIO_read(bio, buf, left);
		//successfully read something
		if(r > 0) {
			left -= r;
			buf += r;
		}
	} while(r != 0 && (left > 0 || BIO_should_retry(bio)));
	ensure(r >= 0);
	return len - left;
}

void TlsConnection::send(const char *buf, size_t len) {
	assert(buf);
	assert(bio);

	size_t left = len;
	do {
		int r = BIO_write(bio, buf, left);
		//successfully wrote something
		if(r > 0) {
			left -= r;
			buf += r;
		}
	} while(left > 0 || BIO_should_retry(bio));
	ensure(!left);
}

//attempts to make TLS connection (TCP handshake, TLS handshake + verification)
void TlsConnection::setupConnection(const char* host, int port) {
	assert(!ctx);
	assert(!ssl);
	assert(!bio);
	assert(host);
	assert(openssl_loaded);
	assert(port > 0 && port < (1<<16));
	///////////MAKE CONTEXT/////////
	ctx = SSL_CTX_new(method);
	//ssl/ssl_lib.c:2630:SSL_CTX *SSL_CTX_new(const SSL_METHOD *meth)
	ensure(ctx);

	//make server certificates be verified
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	//set maximum certificate chain depth to verify (before raising error)
	SSL_CTX_set_verify_depth(ctx, 4);

	//disable old versions of SSL + compression
	//see: include/openssl/ssl.h for list of all options
	const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
				SSL_OP_NO_TLSv1 | SSL_OP_NO_COMPRESSION;
				//not about SSL_OP_NO_TICKET?
	SSL_CTX_set_options(ctx, flags);

	//load root certificate store
	ensure(SSL_CTX_load_verify_locations(ctx, CA_STORE_PATH, NULL));

	//////////PREPARE FOR CONNECTION////////
	bio = BIO_new_ssl_connect(ctx);
	ensure(bio);

	//Set host and port
	ensure(BIO_set_conn_hostname(bio, host));
	ensure(BIO_set_conn_int_port(bio, &port));

	BIO_get_ssl(bio, &ssl);
	ensure(ssl);

	//enable hostname verification
	X509_VERIFY_PARAM *vpm = SSL_get0_param(ssl);
	//For list of verification flags see: X509_VERIFY_PARAM_set_hostflags
	//TODO: figure out what these mean:
	//X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS
	//X509_V_FLAG_X509_STRICT
	//X509_V_FLAG_CRL_CHECK		Have to set set up CRL list?
	//X509_V_FLAG_CRL_CHECK_ALL

	X509_VERIFY_PARAM_set1_host(vpm, host, 0);

	////////SET CIPHER SUIT PREFERENCES//////
	ensure(SSL_set_cipher_list(ssl, CIPHER_PREFERENCES));

	//Use Server Name Indication
	ensure(SSL_set_tlsext_host_name(ssl, host) == 1);
	/* include/openssl/tls1.h:272:# define SSL_set_tlsext_host_name(s,name) \
	 * include/openssl/ssl.h:1713:long SSL_ctrl(SSL *ssl, int cmd, long larg, void *parg);
	 */

	///////MAKE ACTUAL CONNECTION//////
	//tcp 3 way handshake + TLS handshake
	ensure(BIO_do_connect(bio) == 1);
	ensure(BIO_do_handshake(bio) == 1);
	/* include/openssl/bio.h:408:# define BIO_do_handshake(b) \
	 *  BIO_ctrl(b,BIO_C_DO_STATE_MACHINE,0,NULL)
	 */

	//check that server gave us a certificate
	X509* cert = SSL_get_peer_certificate(ssl);
	ensure(cert);
	X509_free(cert); //don't need it

	//verify certificate chain
	ensure(SSL_get_verify_result(ssl) == X509_V_OK);
}

void TlsConnection::close() {
	//TODO:
	if(ctx) {
		SSL_CTX_free(ctx);
		ctx = nullptr;
	}
	if(ssl) {
		ssl = nullptr;
	}
	if(bio) {
		BIO_flush(bio);	//closing connection, not much we can do
		BIO_free_all(bio);
		bio = nullptr;
	}
}

void TlsConnection::ensure(bool cond) {
	if(!cond) {
		ERR_print_errors_fp(stderr);
		close();
		//TODO: clean up object state + throw exception
		assert(false);
	}
}

//initializes the openssl cipher suite tables + loads error strings
void TlsConnection::init_openssl() {
	//register TLS ciphers and digests
	//always returns 1, TODO: not reentrant fix this
	SSL_library_init();

	//no return value
	SSL_load_error_strings(); 
	//ERR_load_BIO_strings(); //could not find documentation on this do I need it?

	//Because all connections will support the same versions
	//we only need to get method once
	method = SSLv23_method();
	assert(method);
	/* include/openssl/ssl.h:1738:#define SSLv23_client_method    TLS_client_method
	 * include/openssl/ssl.h:1743:__owur const SSL_METHOD *TLS_client_method(void);
	 *
	 * They use macros to define TLS_client_method. Returns pointer to static local
	 * variable.
	 * ssl/ssl_locl.h:1940:# define IMPLEMENT_tls_meth_func(version, flags, mask, func_name, s_accept, \
	 * ssl/methods.c:18:IMPLEMENT_tls_meth_func(TLS_ANY_VERSION, 0, 0,
	 * 											TLS_method,
	 * 											....
	 */
	//TODO: won't bother with cleanup code yet (see: EV_cleanup)
	openssl_loaded = true;
}
