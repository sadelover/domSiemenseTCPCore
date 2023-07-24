#include "StdAfx.h"
#include "HttpOperation.h"
#include <string>

using namespace std;


#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p){delete[] (p);  (p)=NULL;} }
#endif

CHttpOperation::CHttpOperation(const CString strIp, const int nPort, const CString strDir, bool bHttps )
	: m_pConnection(NULL)
	, m_strIp(strIp)
	, m_nPort(nPort)
	, m_strDir(strDir)
{

	m_nReceiveTimeOut = 300000;
	m_nServerTimeOut=300000;
	CreateConnection(m_strIp, m_nPort, bHttps);
}


CHttpOperation::~CHttpOperation(void)
{
	CloseConnection();
}

bool CHttpOperation::CreateConnection(const CString strHost, const INTERNET_PORT nPort, bool bHttps)
{
	if (NULL == m_pConnection)
	{
		try
		{

			m_Isession.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, m_nReceiveTimeOut);
			m_Isession.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, m_nServerTimeOut);

			if(bHttps)
			{
				m_pConnection = m_Isession.GetHttpConnection((LPCTSTR)strHost, INTERNET_FLAG_SECURE, nPort);
			}
			else
			{
				m_pConnection = m_Isession.GetHttpConnection((LPCTSTR)strHost, nPort);
			}
			
		}
		catch (...)
		{
			return false;
		}

		if (NULL == m_pConnection)
		{
			return false;
		}
	}

	return true;
}

bool CHttpOperation::CloseConnection(void)
{
	if (m_pConnection != NULL)
	{
		m_pConnection->Close();
		delete m_pConnection;
		m_pConnection = NULL;
	}

	m_Isession.Close();
	return true;
}

bool CHttpOperation::HttpPostGetRequest(CString& strResult)
{
	if (NULL == m_pConnection)
	{
		return false;
	}

	string strData = "name=domlogic";
	wstring HTTP_HEADER = L"Content-Type: application/x-www-form-urlencoded";
	CHttpFile*	pFile = NULL;
	WCHAR*	pUnicode = NULL;

	try
	{
		pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, m_strDir);
		if (pFile != NULL && pFile->SendRequest(HTTP_HEADER.c_str(), (DWORD)HTTP_HEADER.length(), (LPVOID*)strData.c_str(), strData.length()))
		{
			string strHtml;
			char szChars[1025] = {0};
			while ((pFile->Read((void*)szChars, 1024)) > 0)
			{
				strHtml += szChars;
			}
			int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, NULL, 0);
			pUnicode = new WCHAR[unicodeLen + 1];
			memset(pUnicode, 0, (unicodeLen+1)*sizeof(wchar_t));

			MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, pUnicode, unicodeLen);
			strResult.Format(_T("%s"), pUnicode);

			SAFE_DELETE_ARRAY(pUnicode);
			if (pFile != NULL)
			{
				pFile->Close();
				delete pFile;
				pFile = NULL;
			}
		}
	}
	catch (...)
	{
		SAFE_DELETE_ARRAY(pUnicode);
		if (pFile != NULL)
		{
			pFile->Close();
			delete pFile;
			pFile = NULL;
		}

		return false;
	}


	return true;
}

bool CHttpOperation::HttpPostGetRequestEx(const int nType, const char* pData, CString& strResult, CString strHeader, bool bHttps)
{
	if (NULL == m_pConnection)
	{
		return false;
	}

	wstring HTTP_HEADER = strHeader.GetString();
	CHttpFile*	pFile = NULL;
	WCHAR*	pUnicode = NULL;

	int nHttpType = CHttpConnection::HTTP_VERB_POST;
	if (0 == nType)
	{
		nHttpType = CHttpConnection::HTTP_VERB_POST;
	}
	else if (1 == nType)
	{
		nHttpType = CHttpConnection::HTTP_VERB_GET;
	}
	else
	{	// error
		return false;
	}

	try
	{
		if(bHttps)
		{
			pFile = m_pConnection->OpenRequest(nHttpType, m_strDir,0,1,0,0,INTERNET_FLAG_SECURE);
		}
		else
		{
			pFile = m_pConnection->OpenRequest(nHttpType, m_strDir);
		}
		
		int nDataLen = strlen(pData);
		if (pFile != NULL && pFile->SendRequest(HTTP_HEADER.c_str(), (DWORD)HTTP_HEADER.length(), (LPVOID*)pData, nDataLen))
		{
			string strHtml;
			char szChars[10001] = {0};
			while ((pFile->Read((void*)szChars, 10000)) > 0)
			{
				strHtml += szChars;
			}
			int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, NULL, 0);
			pUnicode = new WCHAR[unicodeLen + 1];
			memset(pUnicode, 0, (unicodeLen+1)*sizeof(wchar_t));

			MultiByteToWideChar(CP_UTF8, 0, strHtml.c_str(), -1, pUnicode, unicodeLen);
			strResult.Format(_T("%s"), pUnicode);

			SAFE_DELETE_ARRAY(pUnicode);

			if (pFile != NULL)
			{
				pFile->Close();
				delete pFile;
				pFile = NULL;
			}
		}
	}
	catch (...)
	{
		if (pFile != NULL)
		{
			pFile->Close();
			delete pFile;
			pFile = NULL;
		}
		return false;
	}

	return true;
}
