#ifndef CONF_DUMMY

#include <fstream>

#include <core/wrapper.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

#include "crypto.h"

void CCrypto::HandleErrors()
{
	SSL_load_error_strings();
	unsigned long n = ERR_get_error();
	char buf[1024];
	print("SSL Error: %s", ERR_error_string(n, buf));
}

void *CCrypto::StartEncrypt(const unsigned char *pKey, const unsigned char *pIV)
{
	EVP_CIPHER_CTX *pCtx = EVP_CIPHER_CTX_new();

	if (pCtx == 0x0)
		HandleErrors();

	if (EVP_EncryptInit_ex(pCtx, EVP_aes_256_cbc(), NULL, pKey, pIV) != 1)
		HandleErrors();

	return pCtx;
}

int CCrypto::Encrypt(void *pCtxPtr, const unsigned char *pPlainText, int PlainLength, unsigned char *pCipherText)
{
	EVP_CIPHER_CTX *pCtx = (EVP_CIPHER_CTX *)pCtxPtr;

	int Length = 0;
    if(EVP_EncryptUpdate(pCtx, pCipherText, &Length, pPlainText, PlainLength) != 1)
        HandleErrors();

    return Length;
}

int CCrypto::FinishEncrypt(void *pCtxPtr, unsigned char *pCipherText)
{
	EVP_CIPHER_CTX *pCtx = (EVP_CIPHER_CTX *)pCtxPtr;
	int Length = 0;

	if (EVP_EncryptFinal_ex(pCtx, pCipherText, &Length) != 1)
		HandleErrors();

	EVP_CIPHER_CTX_free(pCtx);
	return Length;
}

void *CCrypto::StartDecrypt(const unsigned char *pKey, const unsigned char *pIV)
{
	EVP_CIPHER_CTX *pCtx = EVP_CIPHER_CTX_new();

	if (pCtx == 0x0)
		HandleErrors();

	if (EVP_DecryptInit_ex(pCtx, EVP_aes_256_cbc(), NULL, pKey, pIV) != 1)
		HandleErrors();

	return pCtx;
}

int CCrypto::Decrypt(void *pCtxPtr, const unsigned char *pCipherText, int CipherLength, unsigned char *pPlainText)
{
	EVP_CIPHER_CTX *pCtx = (EVP_CIPHER_CTX *)pCtxPtr;

	int Length = 0;
    if(EVP_DecryptUpdate(pCtx, pPlainText, &Length, pCipherText, CipherLength) != 1)
        HandleErrors();

    return Length;
}

int CCrypto::FinishDecrypt(void *pCtxPtr, unsigned char *pPlainText)
{
	EVP_CIPHER_CTX *pCtx = (EVP_CIPHER_CTX *)pCtxPtr;
	int Length = 0;

	if (EVP_DecryptFinal_ex(pCtx, pPlainText, &Length) != 1)
		HandleErrors();

	EVP_CIPHER_CTX_free(pCtx);
	return Length;
}

void CCrypto::Hash(const char *pInput, const char *pSalt, int SaltLength, const unsigned char *pKey, int KeyLengths)
{
	PKCS5_PBKDF2_HMAC (pInput, str_length(pInput), (const unsigned char*)pSalt, SaltLength, 4096, EVP_sha512(), KeyLengths, (unsigned char *)pKey);
}

CCrypto::CEncFileInfo *CCrypto::EncFileStart(const char *pPath, const unsigned char *pKey, const unsigned char *pIV)
{
	CEncFileInfo *pInfo = new CEncFileInfo();
	std::ofstream *pFile = new std::ofstream();

	pFile->open(pPath, std::ios::binary);
	
	if(pFile->is_open() == false)
	{
		delete pInfo;
		delete pFile;
		return 0x0;
	}

	pInfo->m_pFilePtr = pFile;
	pInfo->m_CtxPtr = StartEncrypt(pKey, pIV);
	return pInfo;
}

void CCrypto::EncFileWrite(CEncFileInfo *pInfo, const unsigned char *pData, int Size)
{
	if(pInfo->m_pFilePtr == 0x0)
		return;

	std::ofstream *pFile = (std::ofstream *)pInfo->m_pFilePtr;

	unsigned char *aCypher = new unsigned char[Size * 3];
	int EncSize = Encrypt(pInfo->m_CtxPtr, pData, Size, aCypher);
	pFile->write((const char *)aCypher, EncSize);
	delete[] aCypher;
}

void CCrypto::EncFileFinish(CEncFileInfo *pInfo)
{
	if(pInfo->m_pFilePtr == 0x0)
		return;

	std::ofstream *pFile = (std::ofstream *)pInfo->m_pFilePtr;

	unsigned char *aCypher = new unsigned char[17];
	int EncSize = FinishEncrypt(pInfo->m_CtxPtr, aCypher);
	pFile->write((const char *)aCypher, EncSize);
	delete[] aCypher;

	pFile->close();
	
	delete pFile;
	delete pInfo;
}

#endif