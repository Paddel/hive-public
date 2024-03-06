#ifndef CONF_DUMMY

#include <core/detect.h>
#include <core/wrapper.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include "ssl.h"

#define FILE_CERT "cert.pem"
#define FILE_CHAIN "chain.pem"
#define FILE_KEY "privkey.pem"

void CSSL::InitOpenssl()
{
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
	m_WebSock = 0;
}

void CSSL::CleanupOpenssl()
{
	EVP_cleanup();
}

ssl_ctx_st *CSSL::CreateContext()
{
	const SSL_METHOD *pMethod = SSLv23_server_method();
	ssl_ctx_st *pCtx = SSL_CTX_new(pMethod);

	if (!pCtx)
		print("Unable to create SSL context");

	return pCtx;
}

void CSSL::ConfigureContext(ssl_ctx_st *pCtx)
{
	SSL_CTX_set_ecdh_auto(pCtx, 1);

	if (SSL_CTX_use_certificate_file(pCtx, FILE_CERT, SSL_FILETYPE_PEM) <= 0)
		print("Could not load Certificate %s", FILE_CERT);

	if (!SSL_CTX_load_verify_locations(pCtx, FILE_CHAIN, 0x0))
		print("Could not load CA %s", FILE_CHAIN);

	SSL_CTX_set_verify_depth(pCtx, 1);

	if (SSL_CTX_use_PrivateKey_file(pCtx, FILE_KEY, SSL_FILETYPE_PEM) <= 0)
		print("Could not load Key %s", FILE_KEY);

	char const *pCipherList = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA256:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:ECDHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES256-SHA256:!TLSv1";
	//char const *pCipherList = "ALL:!RC4:!DES:!3DES:!MD5:!EXP";
	SSL_CTX_set_cipher_list(pCtx, pCipherList);
}

void CSSL::Init()
{
	InitOpenssl();
	m_pCtx = CreateContext();
	ConfigureContext(m_pCtx);

	m_WebSock = net_tcp_create(443);
	if (net_tcp_listen(m_WebSock, 128))
		print("Webserver SSL could not listen");


	/*net_tcp_close_socket(sock);
	SSL_CTX_free(ctx);
	cleanup_openssl();*/
}

int CSSL::Accept(char *pAddress, int& Port)
{
	return net_tcp_accept(m_WebSock, pAddress, Port);
}

void *CSSL::CreateSSL(int Socket)
{
	if (Socket == -1)
		return 0x0;

	static LOCK Lock = lock_create();
	//lock_wait(Lock);

	ssl_st *pSSL = SSL_new(m_pCtx);
	int result = SSL_set_fd(pSSL, Socket);
	if (SSL_accept(pSSL) <= 0)
	{
#ifdef CONF_DUMMY
		SSL_load_error_strings();
		unsigned long n = ERR_get_error();
		char buf[1024];
		print("Could not accept SSL Connection: %s", ERR_error_string(n, buf));
#endif
		SSL_free(pSSL);
		net_tcp_close_socket(Socket);
		pSSL = 0x0;
	}

	//lock_unlock(Lock);

	return pSSL;
}

int CSSL::Write(const void *pSSL, const char *pData, int Size)
{
	return SSL_write((ssl_st *)pSSL, pData, Size);
}

int CSSL::Read(const void *pSSL, void *pData, int MaxSize)
{
	return SSL_read((ssl_st *)pSSL, pData, MaxSize);
}

void CSSL::Close(const void *pSSL)
{
	SSL_free((ssl_st *)pSSL);
}

long long CSSL::CertificateExpiresIn()
{
	FILE *pFile = fopen(FILE_CERT, "rb");
    if (!pFile)
        return -1;
    X509* pCert = PEM_read_X509_AUX(pFile, NULL, NULL, NULL);
	fclose(pFile);

    if (!pCert)
        return -1;

	ASN1_TIME *pNotAfter = X509_get_notAfter(pCert);

	if(pNotAfter == 0x0)
	{
		X509_free(pCert);
		return -1;
	}

	struct tm TimeStamp;
    ASN1_TIME_to_tm(X509_get_notAfter(pCert), &TimeStamp);
	X509_free(pCert);
	return mktime(&TimeStamp) - time(0);
}

#endif