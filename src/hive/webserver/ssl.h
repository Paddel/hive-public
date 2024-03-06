#pragma once

class ISSL
{
private:

public:
	virtual void Init() {}
	virtual int Accept(char *pAddress, int& Port) { return -1; }
	virtual void *CreateSSL(int Socket) { return 0x0; }
	virtual int Write(const void *pSSL, const char *pData, int Size) { return 0; }
	virtual int Read(const void *pSSL, void *pData, int MaxSize) { return 0; }
	virtual void Close(const void *pSSL) {}
	virtual long long CertificateExpiresIn() { return -1; }
};

#ifndef CONF_DUMMY

struct ssl_ctx_st;
struct ssl_st;

class CSSL : public ISSL
{
private:
	ssl_ctx_st *m_pCtx;
	int m_WebSock;

	void InitOpenssl();
	void CleanupOpenssl();
	ssl_ctx_st *CreateContext();
	void ConfigureContext(ssl_ctx_st *pCtx);

public:
	void Init();
	int Accept(char *pAddress, int& Port);
	void *CreateSSL(int Socket);
	int Write(const void *pSSL, const char *pData, int Size);
	int Read(const void *pSSL, void *pData, int MaxSize);
	void Close(const void *pSSL);
	long long CertificateExpiresIn();
};

#endif