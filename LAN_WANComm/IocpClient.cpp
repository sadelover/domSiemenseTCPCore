// IocpClient.h implement

#include "stdafx.h"
#include "IocpClient.h"

extern HWND g_hMainWnd;
extern bool g_bAutoUpdateClose;
extern bool g_bAppNormalExit;


//======================================================================
CIocpClient::CIocpClient()
: m_hThread(NULL)
//======================================================================
{
	// Initialize the parameters and objects.
	InitParams();
}

//======================================================================
CIocpClient::~CIocpClient()
//======================================================================
{
	// Free the thread handle resource.
	if (m_hThread != NULL)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	if (g_bAppNormalExit)
	{
		// Close the pop window and recycle the resouce.
		ClosePopWindow();
	}

	// Delete the critical section object.
	DeleteCriticalSection(&m_criServerConnect);
}

//======================================================================
// Initialize parameters.
void CIocpClient::InitParams()
//======================================================================
{
	// Get the update files information.
//	CUpdateFiles::GetInstance()->GetXmlFileUpdateInfo();

	// Initialize the update files information.
	InitUpdateFilesInfo();

	m_bIsServerConnected = false;
	m_bIsTestStatus = false;
	m_bShowPopWindow = false;

	// Initialize the cirtical section object.
	InitializeCriticalSection(&m_criServerConnect);
}

//======================================================================
// Initialize the update files information.
void CIocpClient::InitUpdateFilesInfo()
//======================================================================
{
	LPTSTR lpszCurrentAppPath = TEXT("");
	LPTSTR lpszClientLocation = TEXT("");
	TCHAR tszSqlFilePath[MAX_PATH] = {0};
	char szClientLocation[MAX_PATH] = {0};

	// Initialize the update sql statements file name and file path.
	memset(m_szSqlFileName, 0, MAX_PATH);
	memset(m_szSqlFilePath, 0, MAX_PATH);

	// Get the current application path.
	lpszCurrentAppPath = m_toolsFunction.GetCurretAppPath();
	_tcscpy_s(tszSqlFilePath, MAX_PATH, lpszCurrentAppPath);
	wsprintfA(m_szSqlFilePath, "%S", tszSqlFilePath);
	lpszClientLocation = m_toolsFunction.GetProfileValue(_T("client"), _T("location"), FILE_CONFIG_DATA);
	if (_tcscmp(lpszClientLocation, _T("")) != 0)
	{
		wsprintfA(szClientLocation, "%S", lpszClientLocation);
		strcat_s(m_szSqlFileName, MAX_PATH, szClientLocation);
		strcat_s(m_szSqlFilePath, MAX_PATH, szClientLocation);
	}

	strcat_s(m_szSqlFileName, MAX_PATH, "_UpdateSqlStatements.dat");
	strcat_s(m_szSqlFilePath, MAX_PATH, "_UpdateSqlStatements.dat");
}

//======================================================================
// Initialize the iocp client.
bool CIocpClient::InitIocpClient()
//======================================================================
{
	// Start the client.
	if (!(m_clientIocp.Start(-1, 1201, 1, 0, 0, 0, 1, 1, 4)))
	{
		// Start the client failed.
		TRACE("Start the iocp client failed.  \r\n");
		return false;
	}

	// Verify whether show the pop window?
	m_bShowPopWindow = VerifyPopWindowShow();
	if (m_bShowPopWindow)
	{
		// Show the pop window.
		m_clientIocp.ShowPopWindow();
	}

	return true;
}

//======================================================================
// Get the connect ip address and port.
void CIocpClient::GetConnectIpPort(char* pszIpAddr, int& nPort)
//======================================================================
{
	LPTSTR lpszAdress = TEXT("");
	LPTSTR lpszPort = TEXT("");
	char szAddress[MAX_PATH] = {0};
	int iPort = 0;

	// Get the ip address from the dat file.
	lpszAdress = m_clientIocp.GetProfileValue(_T("server"), _T("host"), FILE_CONFIG_DATA);
	wsprintfA(szAddress, "%S", lpszAdress);

	// Get the port from the dat file.
	lpszPort = m_clientIocp.GetProfileValue(_T("server"), _T("port"), FILE_CONFIG_DATA);
	iPort = _ttoi(lpszPort);

	strcpy_s(pszIpAddr, MAX_PATH, szAddress);
	nPort = iPort;
}

//======================================================================
// Send the initial information.
void CIocpClient::SendInitInfoToServer()
//======================================================================
{
	char szFormatSendDatas[MAX_PATH] = {0};
	char szClientLocation[MAX_PATH] = {0};
	LPTSTR lpszClientType = TEXT("");
	LPTSTR lpszClientLocation = TEXT("");

	// Verify the client location.
	lpszClientLocation = m_clientIocp.GetProfileValue(_T("client"), _T("location"), FILE_CONFIG_DATA);
	if (_tcscmp(lpszClientLocation, _T("")) != 0)
	{
		wsprintfA(szClientLocation, "%S", lpszClientLocation);
		m_clientIocp.BuildPackageAndSend(m_clientIocp.GetCurrentClientContext(), szClientLocation, Message_ClientLocation);
	}

	Sleep(100);

	// Verify the client type.
	lpszClientType = m_clientIocp.GetProfileValue(_T("client"), _T("type"), FILE_CONFIG_DATA);
	if (_tcscmp(lpszClientType, _T("realtime_update_client")) == 0 && !m_bShowPopWindow)
	{
		// Set the file transmit mode.
		m_clientIocp.SetFileTransmitMode(true);

		// Format the string.
		sprintf_s(szFormatSendDatas, 
			MAX_PATH, 
			"realtime_update_client:%s",
			"1.0");//CUpdateFiles::GetInstance()->m_pUpdateContents.strUpdateFilesPackVersion.c_str());

		// Send the client type to the server.
		m_clientIocp.BuildPackageAndSend(m_clientIocp.GetCurrentClientContext(), szFormatSendDatas, Message_ClientType);
	}
}

//======================================================================
// Send a heart package.
void CIocpClient::SendHeartPackage()
//======================================================================
{
	if (!m_clientIocp.IsFileTransmitMode())
	{
		// Send the heart package to the server.
		m_clientIocp.SendMessageOperation(m_clientIocp.GetCurrentClientContext(), Message_HeartPackage);
		DWORD dwResult = WaitForSingleObject(m_clientIocp.m_hHeartPackage, 5000);
		if (dwResult == WAIT_OBJECT_0)
		{
			// Set the server as a connected status.
			SetConnectStatus(true);

			// Send heart package every 2 seconds.
			Sleep(2000);
		}
		else
		{
			// Set the server as a non-connected status.
			SetConnectStatus(false);
		}
	}
}

//======================================================================
// Connect to the iocp server.
void CIocpClient::ConnectToServer(const char* pszIpAddr, int& nPort)
//======================================================================
{
	// Verify the connect status.
	if (!IsConnectedToServer())
	{
		int nConnectCount = 5;
		char szGreetInfo[MAX_PATH] = {0};
		strcpy_s(szGreetInfo, MAX_PATH, "A client connected to the server.");

		while (1)
		{
			// Connect to server.
			if (!(m_clientIocp.Connect(pszIpAddr, nPort)))
			{
				// Connect to server failed,continue to connect before 5 connect counts.
				if (nConnectCount == 0)
				{
					// Can't connect to the server.
					SetConnectStatus(false);
				}
				else
				{
					// Try to connect the server again.
					nConnectCount--;
				}

				Sleep(1000);
				continue;
			}
			else
			{
				// Connected to the server.
				SetConnectStatus(true);
				// Connect to server success, send the connected message to the server.
				m_clientIocp.BuildPackageAndSend(m_clientIocp.GetCurrentClientContext(), szGreetInfo);
				break;
			}
		}

		// Send the initialize information to the server.
		SendInitInfoToServer();
	}
	else
	{
		if (g_bAutoUpdateClose)
		{
			// Update files.
			StartUpdateFiles();
		}

		// Send the heart package.
		SendHeartPackage();
	}
}

//======================================================================
// Start the iocp client thread.
UINT WINAPI CIocpClient::StartIocpClientThread(LPVOID lpVoid)
//======================================================================
{	
	CIocpClient* cIocpClient = reinterpret_cast<CIocpClient*>(lpVoid);

	char szIpAddr[MAX_PATH] = {0};
	int nPort = 0;

	// Initialize the iocp client.
	cIocpClient->InitIocpClient();

	// Get the connect ip address & port.
	cIocpClient->GetConnectIpPort(szIpAddr, nPort);
	if (strcmp(szIpAddr, "") == 0)
	{
		cIocpClient->SetTestStatus(true);
		cIocpClient->SetConnectStatus(true);
		_endthreadex(0);
		return 1;
	}

	while (!g_bAppNormalExit)
	{
		// Connect to the server.
		cIocpClient->ConnectToServer(szIpAddr, nPort);
		Sleep(100);
	}

	_endthreadex(0);
	return 1;
}

//======================================================================
// To start the iocp client.
bool CIocpClient::StartIocpClient() 
//======================================================================
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, StartIocpClientThread, this, 0, NULL);

	return true;
}

//==================================================================
// To stop the iocp client.
void CIocpClient::StopIocpClient()
//==================================================================
{
	m_clientIocp.ShutDown();
}

// Write the sql statements to file and send the file to the server.
bool CIocpClient::SqlWriteToFileAndSend(const string& strDatas, LPCTSTR lpszSqlFilePath)
{
	TCHAR tszSqlFilePath[MAX_PATH_SIZE] = {0};
	wsprintfW(tszSqlFilePath, TEXT("%S"), m_szSqlFilePath);
	
	// Set the file full path.
	m_toolsFunction.SetDatasFilePath(tszSqlFilePath);
	// Append realtime update datas to file.
	m_toolsFunction.AppendDatas2File(strDatas.c_str());

	// Send the update sql statements file to the server.
	m_clientIocp.BuildFilePackageAndSend(m_clientIocp.GetCurrentClientContext(), m_szSqlFilePath);

	// Wait for the finished of file transmit.
	WaitForSingleObject(m_clientIocp.m_FileTransmitSuccHandle, INFINITE);
	// Set the file transmit mode.
	m_clientIocp.SetFileTransmitMode(false);
	Sleep(500);
	
	// Delete the transmited file.
	if (!DeleteFile(lpszSqlFilePath))
	{
		DWORD dwErrorCode = GetLastError();
	}

	return true;
}

//======================================================================
// To send network datas.
bool CIocpClient::SendNetworkDatas(const string& strDatas)
//======================================================================
{
	if (IsTestStatus())
	{
		return true;
	}

	TCHAR tszSqlFilePath[MAX_PATH] = {0};

	// strDest contents are empty.
	if (strcmp(strDatas.c_str(), "") == 0)
	{
		TRACE("strDatas contents are empty.  \r\n");
		return false;
	}

	// Connect to server success, send the connected message to the server.
	if (IsConnectedToServer())
	{
		wsprintfW(tszSqlFilePath, _T("%S"), m_szSqlFilePath);
		if (m_toolsFunction.IsFileExist(tszSqlFilePath))
		{
			// Re-connected to the server and write sql statements
			// to a file, then send the file to the server, after
			// pro-disconnected.
			SqlWriteToFileAndSend(strDatas, tszSqlFilePath);

			// Send the message of upload update sql statements file finish.
			m_clientIocp.BuildPackageAndSend(m_clientIocp.GetCurrentClientContext(), 
				m_szSqlFileName, 
				Message_UploadUpdateSqlFileFinish);
		}
		else
		{
			// Send the realtime update datas to the server.
			m_clientIocp.BuildPackageAndSend(m_clientIocp.GetCurrentClientContext(), 
				strDatas.c_str(), 
				Message_RealTimeUpdateDatas);
		}
	}
	else
	{
		//-- Ping the server failed and write the realtime update datas to a file.
		
		ULONGLONG lFileSize = GetFileSize(tszSqlFilePath);
		if (lFileSize >= MAX_SQL_FILE_SIZE)
		{
			DeleteFile(tszSqlFilePath);
			Sleep(200);
		}

		TCHAR tszSqlFilePath[MAX_PATH_SIZE] = {0};
		wsprintfW(tszSqlFilePath, TEXT("%S"), m_szSqlFilePath);
		// Set the file full path.
		m_toolsFunction.SetDatasFilePath(tszSqlFilePath);
		// Append realtime update datas to file.
		m_toolsFunction.AppendDatas2File(strDatas.c_str());
	}
    return true;
}

//======================================================================
// Is the client connected to the server?
bool CIocpClient::IsConnectedToServer()
//======================================================================
{
	EnterCriticalSection(&m_criServerConnect);
	bool bIsServerConnected = m_bIsServerConnected;
	LeaveCriticalSection(&m_criServerConnect);
	return bIsServerConnected;
}

//======================================================================
// Set the status of the client connect to the server.
void CIocpClient::SetConnectStatus(bool bStatus)
//======================================================================
{
	EnterCriticalSection(&m_criServerConnect);
	m_bIsServerConnected = bStatus;
	LeaveCriticalSection(&m_criServerConnect);
}

//======================================================================
// Is the client test status?
bool CIocpClient::IsTestStatus()
//======================================================================
{
	return m_bIsTestStatus;
}

//======================================================================
// Set the client test status.
void CIocpClient::SetTestStatus(__in bool bTestStatus)
//======================================================================
{
	m_bIsTestStatus = bTestStatus;
}

//======================================================================
// Get the disconnected error information.
_tstring CIocpClient::GetDisconnectErrorInfo() const
//======================================================================
{
	_tstring strErrorInfo(TEXT(""));
	int nErrorNode = WSAGetLastError();

	switch(nErrorNode)
	{
	case 10060:
		{
			strErrorInfo = TEXT("网络连接超时：请检查网络硬件设备，如交换机、路由器！  \r\n");
		}
		break;
	case 10061:
		{
			strErrorInfo = TEXT("网络连接错误：远程服务器程序没有响应，请联系程序提供商！  \r\n");
		}
		break;
	case 10065:
		{
			strErrorInfo = TEXT("网络连接错误：请检查网络硬件设备，如网卡！  \r\n");
		}
		break;
	default:
		{
			strErrorInfo = TEXT("网络连接错误：请仔细检查软硬件环境！  \r\n");
		}
		break;
	}

	return strErrorInfo;
}

//======================================================================
// Start to update files. 
void CIocpClient::StartUpdateFiles()
//======================================================================
{
	TCHAR tszUpdateFilesApp[MAX_PATH] = {0};

	if (g_hMainWnd != NULL)
	{
		_tcscpy_s(tszUpdateFilesApp, MAX_PATH, m_toolsFunction.GetCurretAppPath());
		_tcscat_s(tszUpdateFilesApp, MAX_PATH, UPDATE_FILE_APPLICATION);
		
		// Shell execute the update files application.
		ShellExecute(NULL, 
			_T("open"),
			tszUpdateFilesApp, 
			NULL, 
			m_toolsFunction.GetCurretAppPath(), 
			SW_SHOWNORMAL);
		
		// Send WM_CLOSE message.
		::SendMessage(g_hMainWnd, WM_CLOSE, NULL, NULL);
		::PostQuitMessage(0);
	}
}

//======================================================================
// Close the pop window.
void CIocpClient::ClosePopWindow()
//======================================================================
{
	m_clientIocp.ClosePopWindow();
}

//======================================================================
// Verify whether show the pop window.
bool CIocpClient::VerifyPopWindowShow()
//======================================================================
{
	char szByteReadValue[MAX_PATH] = {0};
	LPBYTE lpByteReadValue = NULL;
	bool bResult = false;

	lpByteReadValue = new BYTE[MAX_PATH];
	memset(lpByteReadValue, NULL, MAX_PATH);

	// Read the value of the client update files status from the register. 
	m_toolsFunction.RegReadValue(HKEY_CURRENT_USER, REG_VALUE_PATH, "HasUpdateFiles", lpByteReadValue);
	// Save the register key value by read.
	strcpy_s(szByteReadValue, MAX_PATH, reinterpret_cast<const char*>(lpByteReadValue));

	// There are update files which have not been update.
	if (strcmp(szByteReadValue, "yes") != 0)
	{
		bResult = false;
	}
	else
	{
		bResult = true;
	}

	// Clear the read value buffer resource.
	delete[] lpByteReadValue;
	lpByteReadValue = NULL;

	return bResult;
}

//======================================================================
// Get the assign file size.
ULONGLONG CIocpClient::GetFileSize(LPCTSTR lpszFileName)
//======================================================================
{
	ULONGLONG lFileSize = 0;
	CFileStatus fileStatus;
	BOOL bResult = CFile::GetStatus(lpszFileName, fileStatus);
	if (bResult)
	{
		lFileSize = fileStatus.m_size;
	}

	return lFileSize;
}

//======================================================================
// Get the update sql statement.
void CIocpClient::GetUpdateSql(string& strDest, const string& strSrc)
//======================================================================
{
	if (strcmp(strSrc.c_str(), "") != 0)
	{
		string strDestTemp;
		strDestTemp.clear();

		CDatasHandle datasHandle;
		datasHandle.CreatePackage(strDestTemp, strSrc);

		strDest = strDestTemp;
	}
}
