// MyClientIOCP.cpp: implementation of the MyClientIOCP class.

#include "stdafx.h"
#include "MyClientIOCP.h"
#include "../ToolsFunction/PopWindowDlg.h"

#pragma warning(disable:4244 4996)


// Default constructor.
MyClientIOCP::MyClientIOCP() 
: m_hHeartPackage(NULL),
  m_popWindow(NULL)
{
	m_bFlood = FALSE;
	m_sSendText = "";
	m_iNumberOfMsg = 0;
	m_bAutoReconnect = FALSE;
	m_sAutoReconnect = "";
	m_iAutoReconnect = 0;
	m_bHasClientAppUpdate = false;
	m_bAutoUpdateClose = false;
	m_hWnd = NULL;

	m_toolsFunction.SetLogFilePath(TEXT("LogFile.dat"));
	m_hHeartPackage = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_popWindow = new CPopWindow();
}

// Default Destructor.
MyClientIOCP::~MyClientIOCP()
{
	if (m_hWnd != NULL)
	{
		CloseHandle(m_hWnd);
		m_hWnd = NULL;
	}
	if (m_hHeartPackage != NULL)
	{
		CloseHandle(m_hHeartPackage);
		m_hHeartPackage = NULL;
	}
}

// A package have arrived.
void MyClientIOCP::NotifyReceivedPackage(CIOCPBuffer *pOverlapBuff,int nSize,ClientContext *pContext)
{
	////////////////////////////////////////////////////////
	// SO you would process your data here
	// 
	// I'm just going to post message so we can see the data

	/*
	* 0 = Text.
	* 1 = FileTransfer.
	* 2 = StartFileTransfer.
	* 3 = AbortFileTransfer.
	*/
	BYTE PackageType = pOverlapBuff->GetPackageType();

	switch (PackageType)
	{
	case Job_SendText2Client:             // send string message
		{
			// Package the string text.
			Packagetext(pOverlapBuff, nSize, pContext);
		}
		break;
	case Job_SendFileInfo:                // send file information
		{
			// Set the file transmit mode.
			SetFileTransmitMode(true);
			// File package transfer.
			PackageFileTransfer(pOverlapBuff, nSize, pContext);
		}
		break;	
	case Job_StartFileTransfer:           // transfer file
		{
			// Set the file transmit mode.
			SetFileTransmitMode(true);
			// Start to file package transfer.
			PackageStartFileTransfer(pOverlapBuff, nSize, pContext);
		}
		break;
	case Job_AbortFileTransfer:           // abort transfer file
		{
#ifdef TRANSFER_FILE_FUNCTIONALITY
			// Set the no file transmit mode.
			SetFileTransmitMode(false);
			// Disable to send file.
			DisableSendFile(pContext->m_Socket);
#endif
		}
		break;
	case Message_HeartPackage:            // heart package
		{
			// Set the no file transmit mode.
			SetFileTransmitMode(false);
			// The server is active.
			SetEvent(m_hHeartPackage);
		}
		break;
	case Message_RealTimeUpdateDatas:     // realtime update datas message
		{
		}
		break;
	case Message_AnalysisDatas:           // analysis datas message
		{
		}
		break;
	case Message_IsNeedClientUpdateApp:   // is need client update application?
		{
		}
		break;
	case Message_NotNeedClientUpdateApp:  // client need not update application message
		{
			// Set the no file transmit mode.
			SetFileTransmitMode(false);
		}
		break;
	case Message_NeedClientUpdateApp:     // client need update application message 
		{
// 			int nResult = MessageBox(NULL, _T("有新文件需要更新，是否立即更新？  \r\n 注意：如果立即更新，系统将退出，待更新完成后再重新启动！在更新前，请注意保存好重要数据。"), _T("提示"), MB_YESNO | MB_ICONINFORMATION);
// 			if (nResult == IDYES)
// 			{
// 				SendMessageOperation(pContext, Message_ClientUpdateApp);
// 			}
// 			else if (nResult == IDNO)
// 			{
// 				SendMessageOperation(pContext, Message_NotNeedClientUpdateApp);
// 			}
		}
		break;
	case Message_ClientUpdateApp:        // client update application message
		{
// 			SetHasClientAppUpdate(true);
		}
		break;
	case Message_ClientUpdateAppFinish:  // client update application finish message
		{
			// Set the no file transmit mode.
			SetFileTransmitMode(false);

			// Show the pop message window.
			ShowPopWindow();
		}
		break;
	default:
		break;
	};
}

// This functions defines what should be done when a job from the work queue is arrived(not used). 
void MyClientIOCP::ProcessJob(JobItem *pJob, IOCPS* pServer)
{
	switch (pJob->m_command)
	{
	case Job_SendText2Client:
	// do Some job here. 
		break;
	} 
}

// Write log to screen.
// void MyClientIOCP::AppendLog(CString msg)
// {
// 	TRACE("%s\r\n",msg);
// 	int nLen = msg.GetLength();
// 	char *pBuffer = new char[nLen+1]; 
// 	char* pstrMsg = "";
// 	wsprintfA(pstrMsg, "%S", msg.GetBuffer(0));
// //	strcpy(pBuffer,(const char*)msg);
// 	strcpy(pBuffer, pstrMsg);
// 	pBuffer[nLen]='\0';
// 	BOOL bRet=::PostMessage(m_hWnd, WM_LOGG_APPEND, 0, (LPARAM) pBuffer);
// }

// Write log to file (adapter mode).
void MyClientIOCP::AppendLog(CStringA msg)
{
	m_toolsFunction.AppendLog2File(msg);
}

bool MyClientIOCP::SendMessageOperation(int nClientID, BYTE messageType)
{
	bool bRet = false;
	m_ContextMapLock.Lock();
	ClientContext* pContext = FindClient(nClientID);

	if (pContext == NULL)
	{
		return false;
	}
	bRet = SendMessageOperation(pContext, messageType);
	m_ContextMapLock.Unlock();
	return bRet;
}

bool MyClientIOCP::SendMessageOperation(ClientContext* pContext, BYTE messageType)
{
	CIOCPBuffer *pOverlapBuff = AllocateBuffer(IOWrite);
	if (pOverlapBuff)
	{
// 		LPTSTR lpszLocation = _T("location");
// 		if (messageType == Message_ClientUpdateApp)
// 		{
// 			lpszLocation = GetProfileValue(_T("client"), _T("location"), FILE_CONGIG_DATA);
// 		}

//		if (pOverlapBuff->CreatePackage(messageType, lpszLocation))
		if (pOverlapBuff->CreatePackage(messageType))
		{
			ASend(pContext, pOverlapBuff);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// A new client have connected.
void MyClientIOCP::NotifyNewConnection(ClientContext *pcontext)
{
	unsigned int *pBuffer = new unsigned int;
	*pBuffer = pcontext->m_Socket;
	::PostMessage(m_hWnd, WM_NEW_CONNECTION, 0, (LPARAM)pBuffer);
}

// A client have disconnected.
void MyClientIOCP::NotifyDisconnectedClient(ClientContext *pContext)
{
	unsigned int *pBuffer = new unsigned int;
	if (pBuffer)
	{
		*pBuffer = pContext->m_Socket;
	}
	::PostMessage(m_hWnd, WM_CLIENTDISCONNECTED, 0, (LPARAM)pBuffer);
	m_StatusLock.Lock();
	if (m_bAutoReconnect)
	{
	    // Wait A Litterbit.	
		srand((unsigned)time(NULL));
		Sleep(rand() % 79);   // prime number 79, 59, 61, 67, 71, 89, 97, 101 generates a better uniform value 
		Connect(m_sAutoReconnect,m_iAutoReconnect);
	}

	m_StatusLock.Unlock();
}

// A new client have connected and setting new client ClientContext.
void MyClientIOCP::NotifyNewClientContext(ClientContext *pContext)
{
	pContext->m_iNumberOfReceivedMsg = 0;
	pContext->m_bUpdate = TRUE;
}

// Text in a Package is arrived. 
void MyClientIOCP::Packagetext(CIOCPBuffer *pOverlapBuff, int nSize, ClientContext *pContext)
{
	CStringA txt = "";
	BYTE type;
	if (pOverlapBuff->GetPackageInfo(type, txt))
	{
		// To be sure that pcontext suddenly does not dissapear by disconnection.
		m_ContextMapLock.Lock();
		pContext->m_ContextLock.Lock();
		pContext->m_sReceived = txt;
		
		// Update that we have data.
		pContext->m_iNumberOfReceivedMsg++;
		pContext->m_ContextLock.Unlock();
		m_ContextMapLock.Unlock();

		// Update Statistics. 
		m_StatusLock.Lock();
		m_iNumberOfMsg++;
		m_StatusLock.Unlock();

		// Send back the message if we are echoing. 
		// Send Flood if needed. 
		BOOL bRet = FALSE;
		if (m_bFlood)
		{
			bRet = BuildPackageAndSend(pContext, m_sSendText);
		}
	}
}

// This function is called when the remote connection, whant to send a file.    
void MyClientIOCP::PackageFileTransfer(CIOCPBuffer *pOverlapBuff, int nSize, ClientContext *pContext)
{
#ifdef TRANSFER_FILE_FUNCTIONALITY
	CString sFileName = _T("");
	UINT iMaxFileSize = 0;
	BYTE dummy = 0;
	if (pOverlapBuff->GetPackageInfo(dummy, iMaxFileSize, sFileName))
	{
		// Get The Current Path name and application name.
		CStringA sFilePath = "";		
		char drive[_MAX_DRIVE] = {0};
		char dir[_MAX_DIR] = {0};
		char fname[_MAX_FNAME] = {0};
		char ext[_MAX_EXT] = {0};
		CStringA strTemp = "";

		GetModuleFileNameA(NULL, strTemp.GetBuffer(MAX_PATH), MAX_PATH);
		strTemp.ReleaseBuffer();

		_splitpath(strTemp, drive, dir, fname, ext);
		sFilePath = drive;       // drive
		sFilePath += dir;        // dir
		sFilePath+= UPDATE_FILES_DIRECTORY;
		CreateDirectoryA(sFilePath, NULL);
		sFilePath += sFileName;  // name 
		TRACE("Incoming File >%s.  \r\n", sFilePath);

		// Perpare for Receive
		TCHAR lpszFilePath[MAX_PATH] = {0};
		wsprintfW(lpszFilePath, _T("%S"), sFilePath);
		if (PrepareReceiveFile(pContext->m_Socket, lpszFilePath, iMaxFileSize))
		{
			// Send start file transfer.
			CIOCPBuffer *pOverlapBuff = AllocateBuffer(IOWrite);
			if (pOverlapBuff)
			{
				if (pOverlapBuff->CreatePackage(Job_StartFileTransfer))
				{
					ASend(pContext, pOverlapBuff);
				}
			}
		}
		else
		{
		    // Abort Transfer. 
			CIOCPBuffer *pOverlapBuff = AllocateBuffer(IOWrite);
			if (pOverlapBuff)
			{
				if(pOverlapBuff->CreatePackage(Job_AbortFileTransfer))
			     ASend(pContext, pOverlapBuff);
			}
		}
		
		// To be sure that pcontext Suddenly does not dissapear.
		m_ContextMapLock.Lock();
		pContext->m_ContextLock.Lock();
		pContext->m_sReceived = sFilePath;

		// Update that we have data.
		pContext->m_iNumberOfReceivedMsg++;
		pContext->m_ContextLock.Unlock();
		m_ContextMapLock.Unlock();

		// Update Statistics. 
		m_StatusLock.Lock();
		m_iNumberOfMsg++;
		m_StatusLock.Unlock();
	}
#endif
}

// The remote Connections whant to start the transfer. 
void MyClientIOCP::PackageStartFileTransfer(CIOCPBuffer *pOverlapBuff, int nSize, ClientContext *pContext)
{
#ifdef TRANSFER_FILE_FUNCTIONALITY
	StartSendFile(pContext->m_Socket);
#endif 
}

BOOL MyClientIOCP::BuildPackageAndSend(int ClientID, CStringA sText)
{
	BOOL bRet = FALSE;
	m_ContextMapLock.Lock();
	ClientContext* pContext = FindClient(ClientID);

	if (pContext == NULL)
	{
		return FALSE;
	}

	bRet = BuildPackageAndSend(pContext, sText);
	m_ContextMapLock.Unlock();
	return bRet;
}

// Build the a text message message and send it. 
BOOL MyClientIOCP::BuildPackageAndSend(ClientContext *pContext, CStringA sText)
{
	CIOCPBuffer *pOverlapBuff = AllocateBuffer(IOWrite);
	if (pOverlapBuff)
	{
		if (pOverlapBuff->CreatePackage(Job_SendText2Client, sText))
		{
			return ASend(pContext, pOverlapBuff);
		}
		else
		{
			AppendLog("CreatePackage(0,sText) Failed in BuildPackageAndSend.  \r\n");
			ReleaseBuffer(pOverlapBuff);
			return FALSE;
		}
	}
	else
	{
		CStringA msg;
		msg.Format("Could not allocate memory ASend: %s.  \r\n", ErrorCode2Text(WSAGetLastError()));
		AppendLog(msg);
		DisconnectClient(pContext->m_Socket);
		return FALSE;
	}

	return FALSE;	
}

BOOL MyClientIOCP::BuildFilePackageAndSend(int ClientID, CStringA sFile)
{
	BOOL bRet = FALSE;
	m_ContextMapLock.Lock();
	ClientContext* pContext = FindClient(ClientID);
	if (pContext == NULL)
	{
		return FALSE;
	}

	bRet = BuildFilePackageAndSend(pContext, sFile);
	m_ContextMapLock.Unlock();

	return bRet;
}

BOOL MyClientIOCP::BuildPackageAndSendToAll(CStringA sText)
{
	CIOCPBuffer *pOverlapBuff = AllocateBuffer(IOWrite);
	if (pOverlapBuff)
	{
		if(pOverlapBuff->CreatePackage(0, sText))
		{
			return ASendToAll(pOverlapBuff);
		}
		else
		{
			AppendLog("CreatePackage(0,sText) Failed in BuildPackageAndSendToAll.  \r\n");
			ReleaseBuffer(pOverlapBuff);
			return FALSE;
		}
	}
	else
	{
		CStringA msg;
		msg.Format("Could not allocate memory ASend: %s.  \r\n", ErrorCode2Text(WSAGetLastError()));
		AppendLog(msg);
		return FALSE;
	}

	return FALSE;	
}

BOOL MyClientIOCP::BuildPackageAndSend(int ClientID, CStringA sText, BYTE bType)
{
	BOOL bRet = FALSE;
	m_ContextMapLock.Lock();
	ClientContext* pContext = FindClient(ClientID);

	if (pContext == NULL)
	{
		return FALSE;
	}

	bRet = BuildPackageAndSend(pContext, sText, bType);
	m_ContextMapLock.Unlock();

	return bRet;
}

BOOL MyClientIOCP::BuildPackageAndSend(ClientContext *pContext, CStringA sText, BYTE bType)
{
	CIOCPBuffer *pOverlapBuff = AllocateBuffer(IOWrite);
	if (pOverlapBuff)
	{
		if (pOverlapBuff->CreatePackage(bType, sText))
		{
			return ASend(pContext, pOverlapBuff);
		}
		else
		{
			AppendLog("CreatePackage(0,sText) Failed in BuildPackageAndSend.  \r\n");
			ReleaseBuffer(pOverlapBuff);
			return FALSE;
		}
	}
	else
	{
		CStringA msg;
		msg.Format("Could not allocate memory ASend: %s.  \r\n", ErrorCode2Text(WSAGetLastError()));
		AppendLog(msg);
		DisconnectClient(pContext->m_Socket);
		return FALSE;
	}

	return FALSE;
}

// Perpares for file transfer and sends a package containing information about the file. 
BOOL MyClientIOCP::BuildFilePackageAndSend(ClientContext *pContext,CStringA sFile)
{
#ifdef TRANSFER_FILE_FUNCTIONALITY
	TCHAR tszFile[MAX_PATH] = {0};
	wsprintfW(tszFile, _T("%S"), sFile);
	return PrepareSendFile(pContext->m_Socket, (LPCTSTR)tszFile);
#else
	return FALSE;
#endif
}

BOOL MyClientIOCP::BuildStartFileTransferPackageAndSend(int ClientID)
{
	BOOL bRet = FALSE;
	m_ContextMapLock.Lock();
	ClientContext* pContext = FindClient(ClientID);
	if (pContext == NULL)
	{
		return FALSE;
	}

	bRet = BuildStartFileTransferPackageAndSend(pContext);
	m_ContextMapLock.Unlock();

	return bRet;
}

// Send a "Start the file transfer package" to the remote connection. 
BOOL MyClientIOCP::BuildStartFileTransferPackageAndSend(ClientContext *pContext)
{
	CIOCPBuffer *pOverlapBuff = AllocateBuffer(IOWrite);
	if (pOverlapBuff)
	{
		if (pOverlapBuff->CreatePackage(Job_StartFileTransfer))
		{
			return ASend(pContext,pOverlapBuff);
		}	
	}
	else
	{
		CStringA msg;
		msg.Format("Could not allocate memory ASend: %s.  \r\n", ErrorCode2Text(WSAGetLastError()));
		AppendLog(msg);
		DisconnectClient(pContext->m_Socket);
		return FALSE;
	}

	return TRUE;	
}

// Get the signal of client application update.
bool MyClientIOCP::GetHasClientAppUpdate() const
{
	return m_bHasClientAppUpdate;
}

// Set the signal of client application update.
void MyClientIOCP::SetHasClientAppUpdate(const bool bUpdate)
{
	m_bHasClientAppUpdate = bUpdate;
}

// To upload update files to server.
void MyClientIOCP::UploadUpdateFilesToServer()
{
#ifdef TRANSFER_FILE_FUNCTIONALITY

 	CFileDialog dlg( TRUE, _T("*.*"), _T("*.*"),
 		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
 		_T("All kind of files"));
 	if (dlg.DoModal() == IDOK)
 	{
 		CString sFilenameTemp = dlg.GetFileName();
		LPSTR sFilename = "";
		wsprintfA(sFilename, "%S", sFilenameTemp);
 		if (BuildFilePackageAndSend(GetCurrentClientContext(), sFilename))
 		{
 			MessageBox(NULL, _T("Sending file to the server.  \r\n"), _T("Information"), MB_OK);
 		}
 		else
 		{
 			MessageBox(NULL, _T("Error: send file to the server failed.  \r\n"), _T("Error"), MB_OK);
 		}
 	}
#else
	MessageBox(NULL, _T("FileTransfer is Disabled. Define TRANSFER_FILE_FUNCTIONALITY and recompile.  \r\n"), _T("Error"), MB_OK);
#endif
}

// Show the pop window.
void MyClientIOCP::ShowPopWindow()
{
	if (m_popWindow != NULL)
	{
		m_popWindow->ShowPopWindow();
	}
}

// Close the pop window.
void MyClientIOCP::ClosePopWindow()
{
	if (m_popWindow != NULL)
	{
		m_popWindow->ClosePopWindow();
	}
}