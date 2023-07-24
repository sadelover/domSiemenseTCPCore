#ifndef __IOCP_CLIENT_H__
#define __IOCP_CLIENT_H__



#include <process.h>     // used for _beginthreadex()
#include <string.h>
using namespace std;

#include "DatasHandle.h"
#include "Tools/IocpComm/MyClientIocp.h"
#include "../LAN_WANComm/PingIcmp.h"

#define MAX_SQL_FILE_SIZE 1024 * 1024 * 10  // 10M


class CIocpClient
{
public:
	CIocpClient();
	virtual ~CIocpClient();

public:
	inline bool InitIocpClient();                              // initialize the iocp client
	bool StartIocpClient();                                    // start the iocp client
	void StopIocpClient();                                     // to stop the iocp client
	void ConnectToServer(__in const char* pszIpAddr, 
		__in int& nPort);                                      // connect to the iocp server
	
	virtual bool    SendNetworkDatas(__in const string& strDatas);        // send network datas
	void SendInitInfoToServer();                               // send the initial information
	void SendHeartPackage();                                   // send a heart package
	bool SqlWriteToFileAndSend(__in const string& strDatas,
		__in LPCTSTR lpszSqlFilePath);
	void StartUpdateFiles();                                   // start update files 
	inline void ClosePopWindow();                              // close the pop window
 
	void GetUpdateSql(__out string& strDest, __in const string& strSrc);  // get the update sql statement
	void GetConnectIpPort(__out char* pszIpAddr, __out int& nPort);       // get the connect ip address and port

	inline bool IsConnectedToServer();    // is the client connected to the server?
	inline void SetConnectStatus(__in bool bStatus);  // set the status of the client connect to the server

	// Get the disconnected error information.
	_tstring GetDisconnectErrorInfo() const;

	inline bool IsTestStatus();           // is the client test status?
	inline void SetTestStatus(__in bool bTestStatus);  // set the client test status

private:
	void InitParams();                    // initialize parameters
	void InitUpdateFilesInfo();           // initialize the update files information 

	bool VerifyPopWindowShow();           // verify whether show the pop window

	ULONGLONG GetFileSize(LPCTSTR lpszFileName);  // get the assign file size

    static UINT WINAPI StartIocpClientThread(LPVOID lpVoid);

protected:
	MyClientIOCP m_clientIocp;            // the iocp client object

private:
	HANDLE m_hThread;
	CToolsFunction m_toolsFunction;

	char m_szSqlFileName[MAX_PATH];       // the update sql statement file name
	char m_szSqlFilePath[MAX_PATH];       // the update sql statement file path

	bool m_bIsServerConnected;            // verify the client whether connected to the server
	bool m_bIsTestStatus;                 // verify the client whether the test status
	bool m_bShowPopWindow;                // verify whether show the pop window
	CRITICAL_SECTION m_criServerConnect;  // the critical section of the m_bIsServerConnected vairant
};

#endif  // __IOCP_CLIENT_H__