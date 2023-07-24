#include "stdafx.h"
#include "AES256.h"

AES256*     AES256::m_csAESCommObj = NULL;
Mutex		AES256::m_mutexLock;

AES256::AES256()
	: m_hCryptProv(0)
	, m_hkey(0)
	, m_iv(NULL)
	, m_bInitSuccess(false)
{
}

AES256::~AES256()
{
}

AES256 * AES256::GetInstance()
{
	if (m_csAESCommObj == NULL)
	{
		Scoped_Lock<Mutex> mut(m_mutexLock);
		m_csAESCommObj = new AES256();
		atexit(DestroyInstance);
	}
	return m_csAESCommObj;
}

void AES256::DestroyInstance()
{
	if (m_csAESCommObj != NULL)
	{
		m_csAESCommObj->Exit();
		delete m_csAESCommObj;
		m_csAESCommObj = NULL;
	}
}

bool AES256::Encrypt(char ** cipherText, unsigned long* cLen, char * plainText, unsigned long pLen)
{
	if (m_bInitSuccess)
	{
		if (!Encrypt_(m_hkey, cipherText, cLen, (unsigned char *)plainText, pLen))
		{
			printf("ERROR: AES256 Encryption Fail\r\n");
			return false;
		}
		return true;
	}
	return false;
}

bool AES256::Decrypt(unsigned char ** decrypted, char * cipherText, unsigned long cLen)
{
	if (m_bInitSuccess)
	{
		if (!Decrypt_(m_hkey, decrypted, cipherText, cLen))
		{
			printf("ERROR: AES256 Decryption Fail\r\n");
			return false;
		}
		return true;
	}
	return false;
}

bool AES256::Init(char * pKey)
{
	if (m_bInitSuccess == false)
	{
		if (pKey != NULL)
		{
			if (!CryptoInit(&m_hkey, &m_hCryptProv, &m_iv, (const unsigned char*)pKey, strlen(pKey)))
			{
				printf("ERROR: AES256 Init Fail!\r\n");
				return false;
			}
			m_bInitSuccess = true;
			return m_bInitSuccess;
		}
		return false;
	}
	return true;
}

bool AES256::Exit()
{
	if (m_bInitSuccess)
	{
		m_bInitSuccess = false;
		CryptoUninit(m_hkey, m_hCryptProv);
		SAFE_FREE(m_iv);
	}
	return true;
}

void AES256::PrintLog(const wstring &strLog)
{
	_tprintf(strLog.c_str());
}

bool AES256::CryptoInit(HCRYPTKEY *key, HCRYPTPROV *provider, unsigned char **iv, const unsigned char *password, unsigned long pLen)
{
	unsigned long mode = CRYPT_MODE_CBC;
	unsigned long blockSize, blockSizeLen = sizeof(unsigned long);

	if (!CryptAcquireContextW(provider, NULL, NULL, PROV_RSA_AES, 0))
	{
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if (!CryptAcquireContextW(provider, NULL, NULL, PROV_RSA_AES, CRYPT_NEWKEYSET))
			{
				printf("ERROR: %d\n", GetLastError());
				return false;
			}
		}
		else
		{
			printf("ERROR: %d\n", GetLastError());
			return false;
		}
	}

	if (!GenerateKey(key, *provider, CALG_AES_256, password, pLen))
	{
		printf("ERROR: %d\n", GetLastError());
		if (*provider) CryptReleaseContext(*provider, 0);
		return false;
	}

	if (!CryptSetKeyParam(*key, KP_MODE, (BYTE *)&mode, 0))
	{
		printf("ERROR: %d\n", GetLastError());
		if (*key) CryptDestroyKey(*key);
		if (*provider) CryptReleaseContext(*provider, 0);
		return false;
	}

	if (!CryptGetKeyParam(*key, KP_BLOCKLEN, (BYTE *)&blockSize, &blockSizeLen, 0))
	{
		printf("ERROR: %d\n", GetLastError());
		if (*key) CryptDestroyKey(*key);
		if (*provider) CryptReleaseContext(*provider, 0);
		return false;
	}

	blockSize /= 8;

	*iv = (unsigned char *)malloc(blockSize * sizeof(unsigned char));
	if (*iv == NULL)
	{
		if (*key) CryptDestroyKey(*key);
		if (*provider) CryptReleaseContext(*provider, 0);
		return false;
	}
	SecureZeroMemory(*iv, blockSize * sizeof(unsigned char));

	if (!CryptGenRandom(*provider, blockSize, *iv))
	{
		printf("ERROR: %d\n", GetLastError());
		SAFE_FREE(*iv);
		if (*key) CryptDestroyKey(*key);
		if (*provider) CryptReleaseContext(*provider, 0);
		return false;
	}

	if (!CryptSetKeyParam(*key, KP_IV, *iv, 0))
	{
		printf("ERROR: %d\n", GetLastError());
		SAFE_FREE(*iv);
		if (*key) CryptDestroyKey(*key);
		if (*provider) CryptReleaseContext(*provider, 0);
		return false;
	}

	return true;
}

bool AES256::Encrypt_(HCRYPTKEY key, char **cipherText, unsigned long *cLen, unsigned char *plainText, unsigned long pLen)
{
	unsigned long len = 0;
	unsigned char *encrypted = 0;
	unsigned long enLen = 0;

	len = pLen + 1;

	if (!CryptEncrypt(key, 0, TRUE, 0, NULL, &len, 0))
	{
		if (key) CryptDestroyKey(key);
		return false;
	}

	enLen = len;

	encrypted = (unsigned char *)malloc(len * sizeof(unsigned char));
	if (encrypted == NULL)
	{
		if (key) CryptDestroyKey(key);
		return false;
	}
	SecureZeroMemory(encrypted, len * sizeof(unsigned char));

	memcpy_s(encrypted, len, plainText, pLen + 1);

	len = pLen + 1;
	if (!CryptEncrypt(key, 0, TRUE, 0, encrypted, &len, enLen))
	{
		SAFE_FREE(encrypted);
		if (key) CryptDestroyKey(key);
		return false;
	}

	if (!Base64EncodeA(cipherText, cLen, encrypted, enLen))
	{
		SAFE_FREE(encrypted);
		if (key) CryptDestroyKey(key);
		return false;
	}

	SAFE_FREE(encrypted);

	return true;
}

bool AES256::Decrypt_(HCRYPTKEY key, unsigned char ** plainText, char * cipherText, unsigned long cLen)
{
	unsigned long len = 0;
	unsigned long decodedLen = 0;
	unsigned char *decoded = 0;

	if (!Base64DecodeA(&decoded, &decodedLen, cipherText, cLen))
	{
		if (key) CryptDestroyKey(key);
		return false;
	}

	*plainText = (unsigned char *)malloc(decodedLen * sizeof(unsigned char));
	if (*plainText == NULL)
	{
		if (key) CryptDestroyKey(key);
		return false;
	}
	SecureZeroMemory(*plainText, decodedLen * sizeof(unsigned char));

	memcpy_s(*plainText, decodedLen, decoded, decodedLen);

	SAFE_FREE(decoded);

	len = decodedLen;
	if (!CryptDecrypt(key, 0, TRUE, 0, *plainText, &len))
	{
		SAFE_FREE(*plainText);
		if (key) CryptDestroyKey(key);
		return false;
	}

	return true;
}

void AES256::CryptoUninit(HCRYPTKEY key, HCRYPTPROV provider)
{
	if (key) if (!CryptDestroyKey(key)) printf("ERROR: %d\n", GetLastError());
	if (provider) if (!CryptReleaseContext(provider, 0)) printf("ERROR: %d\n", GetLastError());
}

bool AES256::GenerateKey(HCRYPTKEY *key, HCRYPTPROV provider, ALG_ID algid, const unsigned char *password, unsigned long pLen)
{
	if (!provider || password == NULL)
		return false;

	HCRYPTHASH hash;

	if (!CryptCreateHash(provider, CALG_SHA1, 0, 0, &hash))
	{
		printf("ERROR: %d\n", GetLastError());
		return false;
	}

	if (!hash)
		return false;

	if (!CryptHashData(hash, password, pLen, 0))
	{
		CryptDestroyHash(hash);
		return false;
	}

	if (!CryptDeriveKey(provider, algid, hash, CRYPT_EXPORTABLE, key))
	{
		CryptDestroyHash(hash);
		return false;
	}

	CryptDestroyHash(hash);
	return true;
}

bool AES256::Base64EncodeW(WCHAR ** dest, unsigned long * dlen, const unsigned char * src, unsigned long slen)
{
	if (src == NULL)
		return false;

	if (!CryptBinaryToStringW(src, slen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, dlen))
		return false;

	*dest = (WCHAR *)malloc(*dlen * sizeof(WCHAR));
	if (*dest == NULL) return false;
	SecureZeroMemory(*dest, *dlen * sizeof(WCHAR));

	if (!CryptBinaryToStringW(src, slen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, *dest, dlen))
	{
		SAFE_FREE(*dest);
		return false;
	}

	return true;
}

bool AES256::Base64EncodeA(char ** dest, unsigned long * dlen, const unsigned char * src, unsigned long slen)
{
	if (src == NULL)
		return false;

	if (!CryptBinaryToStringA(src, slen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, dlen))
		return false;

	*dest = (char *)malloc(*dlen * sizeof(char));
	if (*dest == NULL) return false;
	SecureZeroMemory(*dest, *dlen * sizeof(char));

	if (!CryptBinaryToStringA(src, slen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, *dest, dlen))
	{
		SAFE_FREE(*dest);
		return false;
	}

	return true;
}

bool AES256::Base64DecodeW(unsigned char ** dest, unsigned long * dlen, const WCHAR * src, unsigned long slen)
{
	if (src == NULL)
		return false;

	if (!CryptStringToBinaryW(src, slen, CRYPT_STRING_BASE64, NULL, dlen, NULL, NULL))
		return false;

	*dest = (unsigned char *)malloc((*dlen + 1) * sizeof(unsigned char));
	if (*dest == NULL) return false;
	SecureZeroMemory(*dest, (*dlen + 1) * sizeof(unsigned char));

	if (!CryptStringToBinaryW(src, slen, CRYPT_STRING_BASE64, *dest, dlen, NULL, NULL))
	{
		SAFE_FREE(*dest);
		return false;
	}

	return true;
}

bool AES256::Base64DecodeA(unsigned char ** dest, unsigned long * dlen, const char * src, unsigned long slen)
{
	if (src == NULL)
		return false;

	if (!CryptStringToBinaryA(src, slen, CRYPT_STRING_BASE64, NULL, dlen, NULL, NULL))
		return false;

	*dest = (unsigned char *)malloc((*dlen + 1) * sizeof(unsigned char));
	if (*dest == NULL) return false;
	SecureZeroMemory(*dest, (*dlen + 1) * sizeof(unsigned char));

	if (!CryptStringToBinaryA(src, slen, CRYPT_STRING_BASE64, *dest, dlen, NULL, NULL))
	{
		SAFE_FREE(*dest);
		return false;
	}

	return true;
}

