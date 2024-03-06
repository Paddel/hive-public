#pragma once

#include <hive/crypto.h>

#define PBKDF2_KEY_LEN 32

#ifndef CONF_DUMMY

class CCrypto : public ICrypto
{
private:
	void HandleErrors();

public:
	
	void *StartEncrypt(const unsigned char *pKey, const unsigned char *pIV);
	int Encrypt(void *pCtxPtr, const unsigned char *pPlainText, int PlainLength, unsigned char *pCipherText);
	int FinishEncrypt(void *pCtxPtr, unsigned char *pCipherText);
	void *StartDecrypt(const unsigned char *pKey, const unsigned char *pIV);
	int Decrypt(void *pCtxPtr, const unsigned char *pCipherText, int CipherLength, unsigned char *pPlainText);
	int FinishDecrypt(void *pCtxPtr, unsigned char *pPlainText);
	void Hash(const char *pInput, const char *pSalt, int SaltLength, const unsigned char *pKey, int KeyLengths);
	CEncFileInfo *EncFileStart(const char *pPath, const unsigned char *pKey, const unsigned char *pIV);
	void EncFileWrite(CEncFileInfo *pInfo, const unsigned char *pData, int Size);
	void EncFileFinish(CEncFileInfo *pInfo);
};

#endif