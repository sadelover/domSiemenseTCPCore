#include "stdafx.h"
#include "RedundencyCtrl.h"
#include "../Tools/CustomTools/CustomTools.h"

const int HEART_PACKAGE_INTERNAL = 2*1000;
CRedundencyCtrl* CRedundencyCtrl::m_csRedundencCommObj = NULL;
bool CRedundencyCtrl::m_bExitThreadHandle = false;

CRedundencyCtrl::CRedundencyCtrl()
{
	m_csRedundencCommObj = NULL;
	m_sockUdpComm = NULL;
	m_nTimeOutCount = 0;
	m_bExitThreadHandle = false;
	m_oleReceiveTime = COleDateTime::GetCurrentTime();
	InitSockLib();
	InitParams();
}

CRedundencyCtrl::~CRedundencyCtrl()
{
	m_bExitThreadHandle = true;
	for (unsigned int i = 0; i < m_vecThreadHandles.size(); i++)
	{
		if(m_vecThreadHandles[i] != NULL)
		{
			WaitForSingleObject(m_vecThreadHandles[i],2000);
			CloseHandle(m_vecThreadHandles[i]);
			m_vecThreadHandles[i] = NULL;
		}		
	}
	if (m_sockUdpComm != NULL)
	{
		closesocket(m_sockUdpComm);
		m_sockUdpComm = NULL;
	}
	DeleteCriticalSection(&m_criMachineModes);
	WSACleanup();
}

CRedundencyCtrl* CRedundencyCtrl::GetInstance()
{
	if (m_csRedundencCommObj == NULL)
	{
		m_csRedundencCommObj = new CRedundencyCtrl();
		atexit(DestroyInstance);
	}
	return m_csRedundencCommObj;
}

void CRedundencyCtrl::DestroyInstance()
{
	if (m_csRedundencCommObj != NULL)
	{
		delete m_csRedundencCommObj;
		m_csRedundencCommObj = NULL;
	}
}

void CRedundencyCtrl::SetRedundencyInfo( wstring strLocalMachineIpAddr,wstring strOtherMachineIpAddr,int nPort ,int nTimeOutMaxLimit)
{
	m_strLocalMachineIpAddr = strLocalMachineIpAddr;
	m_strOtherMachineIpAddr = strOtherMachineIpAddr;
	m_nPort = nPort;
	m_nTimeOutMaxLimit = nTimeOutMaxLimit;
	strcpy_s(m_pszIpAddr, MAX_PATH, Project::Tools::WideCharToAnsi(m_strOtherMachineIpAddr.c_str()).c_str());
}

void CRedundencyCtrl::StartUdpComm()
{
	// Get the udp communication socket.
	SOCKET sockUdpComm = CreateUdpSock();

	// Set recvfrom return message timeout.
	int nTimeOut = HEART_PACKAGE_INTERNAL*2;
	setsockopt(sockUdpComm, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));

	// Send random number.
	SendRandomNumber(sockUdpComm);

	// Non-main server running mode.
	if (!IsMainServerRunningMode())
	{
		// Create the client socket.
		SOCKET sockUdpComm = CreateUdpSock();

		// Set the recvfrom wait time.
		int nTimeOut = HEART_PACKAGE_INTERNAL*2;
		setsockopt(sockUdpComm, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));

		char cRecvBuff[MAX_BUFFER_SIZE] = {0};
		// Receive stream datas from network firstly.
		if (RecvUdpPackage(sockUdpComm, cRecvBuff, MAX_BUFFER_SIZE) != SOCKET_ERROR)
		{
			if (strcmp(cRecvBuff, "") != 0)
			{
				// Treate with the received datas.
				TreatRecvDatas(cRecvBuff, false);
			}
			else
			{
				if (IsHighPriorityMode())
				{
					// Set the machine running mode.
					SetMachineModes(true, true, false);
				}
				else
				{
					// Set the machine running mode.
					SetMachineModes(true, false, false);
				}
			}
		}
	}

	// Send the heart package by a loop.
	LoopSendHeartPackage();

	StartUdpBroadcastDoubleServer();
}

void CRedundencyCtrl::SetMachineModes( __in bool bServerRunning, __in bool bHighPriority, __in bool bOtherMachineActive )
{
	if(!m_bExitThreadHandle)
	{
		EnterCriticalSection(&m_criMachineModes);
		m_bMainServerRunningMode = bServerRunning;
		m_bHighPriority = bHighPriority;
		m_bOtherMachineActive = bOtherMachineActive;
		LeaveCriticalSection(&m_criMachineModes);
	}
}

bool CRedundencyCtrl::IsMainServerRunningMode() const
{
	return m_bMainServerRunningMode;
}

bool CRedundencyCtrl::IsHighPriorityMode() const
{
	return m_bHighPriority;
}

bool CRedundencyCtrl::IsOtherMachineActive() const
{
	return m_bOtherMachineActive;
}

void CRedundencyCtrl::StopSendAndReceiveThread()
{
	for (unsigned int i = 0; i < m_vecThreadHandles.size(); i++)
	{
		::TerminateThread(m_vecThreadHandles[i], 0);
		CloseHandle(m_vecThreadHandles[i]);
		WaitForSingleObject(m_vecThreadHandles[i], INFINITE);
		m_vecThreadHandles[i] = NULL;
	}
}

void CRedundencyCtrl::ResumeSendAndReceiveThread()
{
	LoopSendHeartPackage();
	StartUdpBroadcastDoubleServer();
}

void CRedundencyCtrl::InitParams()
{
	m_vecThreadHandles.clear();
	m_bMainServerRunningMode = false;
	m_bHighPriority = false;
	m_bOtherMachineActive = false;
	m_bLastTimeNoReceive = false;
	m_nTimeOutCount = 0;
	m_oleReceiveTime = COleDateTime::GetCurrentTime();
	m_nTimeOutMaxLimit = 15;
	m_sockUdpComm = NULL;
	m_nPort = CONNECT_PORT;
	InitializeCriticalSection(&m_criMachineModes);
	// Get all local machine Ethernet information.
	m_pIpAdapterInfo = m_toolsFunction.GetAllLocalMachineEthInfo();
}

SOCKET CRedundencyCtrl::CreateUdpSock()
{
	if (m_sockUdpComm == NULL)
	{
		// Create the udp socket.
		m_sockUdpComm = socket(AF_INET, SOCK_DGRAM, 0);

		// Config the  bind struct SOCKADDR_IN.
		SOCKADDR_IN sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(m_nPort);
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);  

		if (bind(m_sockUdpComm, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != 0)    
		{    
			// Bind failed.
			closesocket(m_sockUdpComm);
			m_sockUdpComm = NULL;    
		}

		// Set the bind socket address reuseable.
		BOOL bReuseAddr = TRUE;
		setsockopt(m_sockUdpComm, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuseAddr, sizeof(bReuseAddr));
	}

	return m_sockUdpComm;
}

void CRedundencyCtrl::SendUdpPackage( __in SOCKET sock, __in const char* pszBuffer )
{
	// Config ip address and communication port.
	SOCKADDR_IN addrSend;
	addrSend.sin_addr.S_un.S_addr = inet_addr(m_pszIpAddr);
	addrSend.sin_family = AF_INET;
	addrSend.sin_port = htons(m_nPort);
	int len = sizeof(SOCKADDR);

	// Send the udp package.
	int nSendResult = sendto(sock, pszBuffer, static_cast<int>(strlen(pszBuffer))+1, 0, (SOCKADDR*)&addrSend, static_cast<int>(sizeof(SOCKADDR_IN)));
	if (nSendResult == SOCKET_ERROR)
	{
		TRACE("Send udp package failed. WSAGetLastError() return %d.  \r\n", WSAGetLastError());
	}
}

int CRedundencyCtrl::RecvUdpPackage( __in SOCKET sock, __out char* pszBuffer, __in int nMaxBuffer )
{
	char szBuff[MAX_BUFFER_SIZE] = {0};
	int nRecvSize = 0;

	if ((nRecvSize = recvfrom(sock, szBuff, nMaxBuffer, 0, NULL, NULL)) == SOCKET_ERROR)
	{
		int nError = WSAGetLastError();
		if (nError != WSAETIMEDOUT)
		{
			TRACE("Received udp package failed. WSAGetLastError() return %d.  \r\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
		return WSAETIMEDOUT;
	}
	memcpy(pszBuffer, szBuff, nRecvSize);
	return nRecvSize;
}

bool CRedundencyCtrl::InitSockLib()
{
	WSADATA wsaData;
	memset(&wsaData, 0, sizeof(WSADATA));

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		TRACE("Winsock2 library start up failed. GetLastError() return: %d", WSAGetLastError());
		WSACleanup();
		return false;
	}
	return true;
}

void CRedundencyCtrl::TreatRecvDatas( __in char* pszRecvBuffer, __in bool bInit )
{
	vector<string> vElement;
	vElement.clear();
	char szRecvBuffer[MAX_BUFFER_SIZE] = {0};
	strcpy_s(szRecvBuffer, MAX_BUFFER_SIZE, pszRecvBuffer);

	// Inquire the received message whether is a listen package. 
	if (strcmp(szRecvBuffer, LISTEN_PACKAGE) == 0)
	{
		m_oleReceiveTime = COleDateTime::GetCurrentTime();
		if (IsMainServerRunningMode())
		{
			if (IsHighPriorityMode())
			{
				SetMachineModes(true, true, false);
			}
			else
			{
				SetMachineModes(false, false, true);
			}
		}
		else
		{
			if (IsHighPriorityMode())
			{
				SetMachineModes(false, true, true);
			}
			else
			{
				SetMachineModes(false, false, true);
			}
		}
	}
	else
	{
		// Split datas.
		Project::Tools::SplitString(szRecvBuffer,";",vElement);
		if (vElement.size() != 0)
		{
			string strPackHeader(vElement[0]);
			if (!IsHighPriorityMode() && strcmp(strPackHeader.c_str(), "RedundencyNumber") == 0)
			{
				if (bInit)
				{
					if (vElement.size() >= 2)
					{
						m_oleReceiveTime = COleDateTime::GetCurrentTime();
						string strCallbackRandomNumber(vElement[1]);
						unsigned int nCallbackRandomNumber = atoi(strCallbackRandomNumber.c_str());
						if (nCallbackRandomNumber < m_nRandomNumber)
						{
							SetMachineModes(true, true, false);
						}
						else 
						{
							SetMachineModes(false, false, true);
						}
					}
					else
					{
						TRACE("Incoming values less than expected, check the incoming values!  \r\n");
					}
				}
			}
		}
	}
}

void CRedundencyCtrl::SendRandomNumber( __in SOCKET sock )
{
	unsigned int nRandomNumber = 0;
	char szRandomNumer[MAX_PATH] = {0};
	char cRecvBuff[MAX_BUFFER_SIZE] = {0};

	srand((unsigned int)(time(0)));         // set the random base number
	nRandomNumber = rand() % RandomNumber;  // get the random number
	m_nRandomNumber = nRandomNumber;
	sprintf(szRandomNumer, "RedundencyNumber:%d", nRandomNumber);

	for (unsigned int i = 0; i < 1; i++)
	{
		// Send the random number message.
		SendUdpPackage(sock, szRandomNumer);

		if (RecvUdpPackage(sock, cRecvBuff, MAX_BUFFER_SIZE) != SOCKET_ERROR)
		{
			if (strcmp(cRecvBuff, "") != 0)
			{
				TreatRecvDatas(cRecvBuff, true);
				break;
			}
			else
			{
				SetMachineModes(true, true, false);
			}
		}
		else
		{
			SetMachineModes(true, true, false);
		}
	}
}

void CRedundencyCtrl::StartUdpBroadcastDoubleServer()
{
	HANDLE hThread = NULL;
	// Call the UdpBroadcastServerThread thread function.
	hThread = (HANDLE)_beginthreadex(NULL, 0, StartUdpBroadcastServerThreadDouble, this, 0, NULL);
	m_vecThreadHandles.push_back(hThread);
}

void CRedundencyCtrl::LoopSendHeartPackage()
{
	HANDLE hThread = NULL;
	// Call the LoopSendHeartPackageThread thread function.
	hThread = (HANDLE)_beginthreadex(NULL, 0, LoopSendHeartPackageThread, this, 0, NULL);
	m_vecThreadHandles.push_back(hThread);
}

UINT WINAPI CRedundencyCtrl::StartUdpBroadcastServerThreadDouble( LPVOID lpVoid )
{
	CRedundencyCtrl* pThis = reinterpret_cast<CRedundencyCtrl*>(lpVoid);

	// Get the udp communication socket.
	pThis->m_sockUdpComm = pThis->CreateUdpSock(); 

	// Set the recvfrom wait time.
	const int nTimeOut = HEART_PACKAGE_INTERNAL*2; //
	setsockopt(pThis->m_sockUdpComm, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));
	bool ifReceive = false;

	while (!pThis->m_bExitThreadHandle)
	{
		pThis->OnTimerUdpBroadcastServerDouble();
		Sleep(HEART_PACKAGE_INTERNAL);
	}
	return 1;
}

UINT WINAPI CRedundencyCtrl::LoopSendHeartPackageThread( LPVOID lpVoid )
{
	CRedundencyCtrl* pThis = reinterpret_cast<CRedundencyCtrl*>(lpVoid);
	// Get the udp communication socket.
	SOCKET sockUdp = pThis->CreateUdpSock();
	while (!pThis->m_bExitThreadHandle)
	{
		// Verify whether the local machine running as main server mode.
		if (pThis->IsMainServerRunningMode())
		{
			// Send the heart package message.
			pThis->SendUdpPackage(sockUdp, LISTEN_PACKAGE);
			Sleep(HEART_PACKAGE_INTERNAL);
		}
		else
		{
			Sleep(1000);
		}
	}
	_endthreadex(0);
	return 1;
}

void CRedundencyCtrl::OnTimerUdpBroadcastServerDouble()
{
	char szRecvBuff[MAX_BUFFER_SIZE] = {0};
	char szRecvIpAddress[MAX_PATH] = {0};
	//SOCKET sockUdpComm = NULL;
	SOCKADDR_IN sockAddrClient;
	int nSize = sizeof(SOCKADDR_IN); 

	//clear mem at 1st
	memset(szRecvBuff, NULL, MAX_BUFFER_SIZE);

	// Receive stream datas from network firstly.
	bool ifReceive = recvfrom(m_sockUdpComm, szRecvBuff, MAX_BUFFER_SIZE, 0, (SOCKADDR FAR*)&sockAddrClient, &nSize) != SOCKET_ERROR;
	if(ifReceive && (strcmp(szRecvBuff, "") != 0) )
	{
		// Get the send package host ip address.
		strcpy_s(szRecvIpAddress, MAX_PATH, inet_ntoa(sockAddrClient.sin_addr));  

		//如果是本机发的包,则不处理.
		if (m_toolsFunction.MatchIpAddress(m_pIpAdapterInfo, szRecvIpAddress))
		{
			return;  // repeat if not local machine ip address
		}

		//如果不是配置的主机发的包,不处理....
		string strOtherMachineIpAddr = Project::Tools::WideCharToAnsi(m_strOtherMachineIpAddr.c_str());
		if (strcmp(szRecvIpAddress, strOtherMachineIpAddr.c_str()) != 0)
		{
			return;
		}

		// Main server running mode should listen FORCE_RUNNING_AS_MAIN_SERVER message.
		if (!IsMainServerRunningMode())
		{
			// Treat with the received datas.
			TreatRecvDatas(szRecvBuff, false);
		}

		m_nTimeOutCount = 0;
	}
	else
	{
		//when receive no data from other pc, check gateway.
		// Can not receive the package from the main server.
		const bool ifHigh = IsHighPriorityMode();
		bool ifMain = true;
		const bool ifMainOld = IsMainServerRunningMode();
		if(ifMainOld)
		{
			SetMachineModes(ifMainOld, ifHigh, false);
			return;
		}

		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReceiveTime;
		if(oleSpan.GetTotalSeconds() >= m_nTimeOutMaxLimit)
		{
			SetMachineModes(true, ifHigh, false);
		}
	}
}
