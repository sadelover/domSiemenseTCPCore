#pragma once

/*
AES256 - AES 256 CBC encryption and Base64 encoding with CryptoAPI and C
Copyright (C) 2016  @maldevel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include <stdbool.h>
#include <Windows.h>
#include <Wincrypt.h>
#include <stdio.h>
#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)
#pragma comment(lib,"crypt32") 
#include "Tools/CustomTools.h"

#define SAFE_FREE(x) if(x) { free(x); x=NULL; }

class AES256
{
public:
	AES256();
	~AES256();

	static AES256* GetInstance();																						// get the singleton instance of assign object
	static void		DestroyInstance();																						// destroy the singleton instance of assign objec
	
	//
	bool	Encrypt(char **cipherText, unsigned long *cLen, char *plainText, unsigned long pLen);							//加密
	bool	Decrypt(unsigned char **decrypted, char *cipherText, unsigned long cLen);									//解密
	bool	Init(char* pKey);
	bool	Exit();
	
private:
	void	PrintLog(const wstring &strLog);																	//打印显示log
	bool	CryptoInit(HCRYPTKEY *key, HCRYPTPROV *provider, unsigned char **iv, const unsigned char *password, unsigned long pLen);
	bool	Encrypt_(HCRYPTKEY key, char **cipherText, unsigned long *cLen, unsigned char *plainText, unsigned long pLen);
	bool	Decrypt_(HCRYPTKEY key, unsigned char **plainText, char *cipherText, unsigned long cLen);
	void	CryptoUninit(HCRYPTKEY key, HCRYPTPROV provider);
	bool	GenerateKey(HCRYPTKEY *key, HCRYPTPROV provider, ALG_ID algid, const unsigned char *password, unsigned long pLen);

	bool	Base64EncodeW(WCHAR **dest, unsigned long *dlen, const unsigned char *src, unsigned long slen);
	bool	Base64EncodeA(char **dest, unsigned long *dlen, const unsigned char *src, unsigned long slen);

	bool	Base64DecodeW(unsigned char **dest, unsigned long *dlen, const WCHAR *src, unsigned long slen);
	bool	Base64DecodeA(unsigned char **dest, unsigned long *dlen, const char *src, unsigned long slen);

private:
	static AES256*		m_csAESCommObj;     // singleton object
	static Mutex		m_mutexLock;        // mutex object
	
	bool				m_bInitSuccess;		//初始化成功
	
	HCRYPTPROV			m_hCryptProv;
	HCRYPTKEY			m_hkey;
	unsigned char*		m_iv;


};
