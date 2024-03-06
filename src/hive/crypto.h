#pragma once

#include <fstream>

#include <core/wrapper.h>

class ICrypto
{
public:
	struct CEncFileInfo
	{
		void *m_pFilePtr;
		void *m_CtxPtr;
	};

public:
	ICrypto() { }

	virtual void *StartEncrypt(const unsigned char *pKey, const unsigned char *pIV) { return 0x0; }
	virtual int Encrypt(void *pCtxPtr, const unsigned char *pPlainText, int PlainLength, unsigned char *pCipherText) { mem_copy(pCipherText, pPlainText, PlainLength); return PlainLength; }
	virtual int FinishEncrypt(void *pCtxPtr, unsigned char *pCipherText) { return 0; }
	virtual void *StartDecrypt(const unsigned char *pKey, const unsigned char *pIV) { return 0x0; }
	virtual int Decrypt(void *pCtxPtr, const unsigned char *pCipherText, int CipherLength, unsigned char *pPlainText) { mem_copy(pPlainText, pCipherText, CipherLength);  return CipherLength; }
	virtual int FinishDecrypt(void *pCtxPtr, unsigned char *pPlainText) { return 0; }
	virtual void Hash(const char *pInput, const char *pSalt, int SaltLength, const unsigned char *pKey, int KeyLengths) {}
	virtual CEncFileInfo *EncFileStart(const char *pPath, const unsigned char *pKey, const unsigned char *pIV) { return 0x0; }
	virtual void EncFileWrite(CEncFileInfo *pInfo, const unsigned char *pData, int Size) { }
	virtual void EncFileFinish(CEncFileInfo *pInfo) { }
	virtual int EncFileBlockSize() { return 0; }
	virtual const char *EncFileSalt() { return ""; }
};