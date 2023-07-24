#ifndef __REDUNDENCY_H__
#define __REDUNDENCY_H__

#pragma once
#include <process.h>     // used for _beginthreadex()
#include <stdlib.h>      
#include <time.h>        // used for random number
#include <vector>
#include "ToolsFunction.h"
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

//////////////////////////////////////////////////////////////////////////
#define LISTEN_PACKAGE                    "Listen_Master"                    // listen package
#define CONNECT_PORT                      8303                             	  // c/s connect port
#define MAX_BUFFER_SIZE                   8192                             	  // max buffer size to transmit datas
#define RandomNumber					  1000
//////////////////////////////////////////////////////////////////////////

class CRedundencyCtrl
{
public:
	CRedundencyCtrl();
	virtual ~CRedundencyCtrl();                         // default destructor

	static CRedundencyCtrl* GetInstance();              // get the singleton instance of assign object
	static void DestroyInstance();                   // destroy the singleton instance of assign objec

	void	SetRedundencyInfo(wstring strLocalMachineIpAddr,wstring strOtherMachineIpAddr,int nPort,int nTimeOutMaxLimit=15);
	
	void	StartUdpComm();                             // to start the udp communication
	void	SetMachineModes(__in bool bServerRunning, __in bool bHighPriority, __in bool bOtherMachineActive);

	bool	IsMainServerRunningMode() const;     // is as the server running program mode?
	bool	IsHighPriorityMode() const;          // is as the high priority by compare with the random number?
	bool	IsOtherMachineActive() const;        // is the other machine activitity?

	void	StopSendAndReceiveThread();
	void	ResumeSendAndReceiveThread();

protected:
	void	InitParams();                               // initialize parameters
	SOCKET	CreateUdpSock(); 
	void	SendUdpPackage(__in SOCKET sock, __in const char* pszBuffer);  // send the udp package
	int		RecvUdpPackage(__in SOCKET sock, __out char* pszBuffer, __in int nMaxBuffer);  // receive the udp package
	bool	InitSockLib();			// initialize parameters

	void	TreatRecvDatas(__in char* pszRecvBuffer, __in bool bInit);  // treat with the received datas
	void	SendRandomNumber(__in SOCKET sock);         // send the random number

	void	StartUdpBroadcastDoubleServer();          // to start the udp broadcast server
	void	LoopSendHeartPackage();              // loop send the heart package

	static UINT WINAPI StartUdpBroadcastServerThreadDouble(LPVOID lpVoid);
	static UINT WINAPI LoopSendHeartPackageThread(LPVOID lpVoid);
	void               OnTimerUdpBroadcastServerDouble();

public:
    SOCKET                  m_sockUdpComm ;
	char					m_pszIpAddr[MAX_PATH];
	int						m_nPort;
	int						m_nTimeOutCount;		//超时次数
	int						m_nTimeOutMaxLimit;		//超时次数上限
	COleDateTime			m_oleReceiveTime;		//上一次收到广播时间

	bool					m_bMainServerRunningMode;                   // the server running mode
	bool					m_bHighPriority;                        	 // the high priority of double running machine
	bool					m_bOtherMachineActive;                  	 // the other machine active?
	bool					m_bLastTimeNoReceive;
	vector<HANDLE>          m_vecThreadHandles;     

	wstring					m_strLocalMachineIpAddr;
	wstring					m_strOtherMachineIpAddr;
	CToolsFunction			m_toolsFunction;
	IP_ADAPTER_INFO*		m_pIpAdapterInfo;               // ip adapter information
	UINT                    m_nRandomNumber;                    // random number
protected:
	static CRedundencyCtrl*    m_csRedundencCommObj;     // singleton object
	static bool					m_bExitThreadHandle;
	CRITICAL_SECTION        m_criMachineModes;              // the critical section of machine modes
};

#endif  // __REDUNDENCY_H__