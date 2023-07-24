#ifndef __MY_CLIENT_IOCP_H__
#define __MY_CLIENT_IOCP_H__
#pragma once

#define WM_NEW_CONNECTION	     WM_APP + 0x1001
#define WM_LOGG_APPEND	         WM_NEW_CONNECTION + 1
#define WM_CLIENTDISCONNECTED    WM_NEW_CONNECTION + 2
#define UPDATE_FILES_DIRECTORY   _T("UpdateFiles\\")
#define UPDATE_FILE_APPLICATION  _T("UpdateFilesAuxility.exe")

#include "Iocps.h"             // base class
#include "../../Definition.h"
#include "../ToolsFunction/ToolsFunction.h"
#include "../ToolsFunction/PopWindowDlg.h"
//#include "../ToolsFunction/PopWindow.h"


class MyClientIOCP : public IOCPS  
{
public:
	MyClientIOCP();
	virtual ~MyClientIOCP();

	virtual void NotifyNewClientContext(ClientContext *pContext);    // a new client have connected and setting new client ClientContext
	virtual void NotifyDisconnectedClient(ClientContext *pContext);  // a client have disconnected
    virtual void NotifyNewConnection(ClientContext *pcontext);       // a new client have connected
	virtual void NotifyReceivedPackage(CIOCPBuffer *pOverlapBuff, int nSize, ClientContext *pContext);  // a package have arrived
	
	virtual inline void ProcessJob(JobItem *pJob, IOCPS* pServer);   // called to do some work
	virtual void AppendLog(CStringA msg);                            // write log to file (adapter mode)

	// Build a text message and send it.
	BOOL BuildPackageAndSend(ClientContext *pContext, CStringA sText);
	BOOL BuildPackageAndSend(int ClientID,CStringA sText);
	BOOL BuildPackageAndSend(int ClientID, CStringA sText, BYTE bType);
	BOOL BuildPackageAndSend(ClientContext *pContext, CStringA sText, BYTE bType);
	BOOL BuildPackageAndSendToAll(CStringA sText);

	// Build the message type and send it.
	bool SendMessageOperation(int nClientID, BYTE messageType);
	bool SendMessageOperation(ClientContext* pContext, BYTE messageType);

	// Builds a Package containing info about a filetranser and send it to the Remote Computer.
	BOOL BuildFilePackageAndSend(ClientContext *pContext, CStringA sFile);
	BOOL BuildFilePackageAndSend(int ClientID, CStringA sFile);

	// Build StartFilsTransfer.
	BOOL BuildStartFileTransferPackageAndSend(ClientContext *pContext);
	BOOL BuildStartFileTransferPackageAndSend(int ClientID);

	bool GetHasClientAppUpdate() const;                 // get the signal of client application update
	void SetHasClientAppUpdate(const bool bUpdate);     // set the signal of client application update

	void UploadUpdateFilesToServer();                   // to upload update files to server

	void ShowPopWindow();                               // show the pop window
	void ClosePopWindow();                              // close the pop window

public:
	CCriticalSection m_StatusLock;
	HANDLE m_hHeartPackage;                             // the event of heart package

protected:
	void Packagetext(CIOCPBuffer *pOverlapBuff, int nSize, ClientContext *pContext);               // text in a package is arrived
	void PackageFileTransfer(CIOCPBuffer *pOverlapBuff, int nSize, ClientContext *pContext);       // a file transfer info package is arrived 
	void PackageStartFileTransfer(CIOCPBuffer *pOverlapBuff, int nSize, ClientContext *pContext);  // a start transfering package is arrived

private:
	HWND m_hWnd;
	CPopWindow* m_popWindow;
	CToolsFunction m_toolsFunction;

	volatile BOOL m_bFlood;        // used to flood
	CStringA m_sSendText;
	// Auoreconnect stuff. 
	BOOL m_bAutoReconnect;
	CStringA m_sAutoReconnect;
	int m_iAutoReconnect;
	int m_iNumberOfMsg;

	bool m_bHasClientAppUpdate;    // verify has client application need update?
	bool m_bAutoUpdateClose;       // auto update close
};

#endif  // __MY_CLIENT_IOCP_H__
