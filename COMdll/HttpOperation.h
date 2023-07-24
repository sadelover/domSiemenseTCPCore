#pragma once

#include <afxinet.h>

class CHttpOperation
{
public:
	CHttpOperation(const CString strIp, const int nPort, const CString strDir, bool bHttps = false);
	~CHttpOperation(void);

public:
	bool	HttpPostGetRequest(CString& strResult);
	bool	HttpPostGetRequestEx(const int nType, const char* pData, CString& strResult, CString strHeader = _T("Content-Type: application/x-www-form-urlencoded"),  bool bHttps = false);

private:
	bool	CreateConnection(const CString strHost, const INTERNET_PORT nPort, bool bHttps = false);	//创建Http Post连接
	bool	CloseConnection(void);												//关闭连接



	CHttpConnection*	m_pConnection;
	CInternetSession	m_Isession;

	CString				m_strIp;
	int					m_nPort;
	CString				m_strDir;


	int m_nServerTimeOut;
	int m_nReceiveTimeOut;
};

