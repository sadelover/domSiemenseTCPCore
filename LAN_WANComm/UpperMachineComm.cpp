// UpperMachineComm.h implement

#include "stdafx.h"
#include "UpperMachineComm.h"

#include "../DataEngineConfig/CDataEngineAppConfig.h"
#include "../Tools/Util/UtilString.h"

#pragma warning(disable:4267 4996)

extern bool g_bAppNormalExit;
const int HEART_PACKAGE_INTERNAL = 2*1000;


//=========================================================================
// Default constructor.
CUpperMachineComm::CUpperMachineComm()
: m_pIpAdapterInfo(NULL)
,m_bLastTimeNoReceive(false)
//=========================================================================
{
	InitParams();
}

//=========================================================================
// Default destructor.
CUpperMachineComm::~CUpperMachineComm()
//=========================================================================
{
	for (unsigned int i = 0; i < m_vecThreadHandles.size(); i++)
	{
		if(m_vecThreadHandles[i] != NULL)
		{
			CloseHandle(m_vecThreadHandles[i]);
			m_vecThreadHandles[i] = NULL;
		}		
	}

	DeleteCriticalSection(&m_criMachineModes);
}

//=========================================================================
// Initialize parameters.
void CUpperMachineComm::InitParams() 
//=========================================================================
{
	// Initialize the buffer.
	memset(m_csLANDbDatasBuff, 0, MAX_BUFFER_SIZE);
	memset(m_csLANEqmStatusBuff, 0, MAX_BUFFER_SIZE);

	m_vecThreadHandles.clear();

	m_bMainServerRunningMode = false;
	m_bHighPriority = false;
	m_bOtherMachineActive = false;
	m_bHeartActive = true;
    m_bActiveGateway = true;

	InitializeCriticalSection(&m_criMachineModes);

	const wstring lpszOtherMachineIpAddr = CDataEngineAppConfig::GetInstance()->GetAnotherServerIP();
    const wstring wstrGateway = CDataEngineAppConfig::GetInstance()->GetGateWayIP();
	const string strOtherMachineIpAddr = m_toolsFunction.WChar2Ansi(lpszOtherMachineIpAddr.c_str() );
    m_strGateway = m_toolsFunction.WChar2Ansi(wstrGateway.c_str() );

	m_netComm.InitIpAddr_Port(strOtherMachineIpAddr.c_str(), CONNECT_PORT);

	// Get all local machine Ethernet information.
	m_pIpAdapterInfo = m_toolsFunction.GetAllLocalMachineEthInfo();
}

//=========================================================================
// Set the machine mode.
void CUpperMachineComm::SetMachineModes(bool bServerRunning, bool bHighPriority, bool bOtherMachineActive)
//=========================================================================
{
	EnterCriticalSection(&m_criMachineModes);
	m_bMainServerRunningMode = bServerRunning;
	m_bHighPriority = bHighPriority;
	m_bOtherMachineActive = bOtherMachineActive;
	LeaveCriticalSection(&m_criMachineModes);
}

//=========================================================================
// Treate with the received datas.
void CUpperMachineComm::TreatRecvDatas(char* pszRecvBuffer, bool bInit)
//=========================================================================
{
	vector<string> vElement;
	vElement.clear();
	char szRecvBuffer[MAX_BUFFER_SIZE] = {0};
	strcpy_s(szRecvBuffer, MAX_BUFFER_SIZE, pszRecvBuffer);

	// Inquire the received message whether is a listen package. 
	if (strcmp(szRecvBuffer, LISTEN_PACKAGE) == 0)
	{
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
		m_datasHandle.Split(vElement, szRecvBuffer, string(":"));
		if (vElement.size() != 0)
		{
			string strPackHeader(vElement[0]);
			if (!IsHighPriorityMode() && strcmp(strPackHeader.c_str(), "RandomNumber") == 0)
			{
				if (bInit)
				{
					if (vElement.size() >= 2)
					{
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

//=========================================================================
// Send the random number.
void CUpperMachineComm::SendRandomNumber(SOCKET sock)
//=========================================================================
{
	unsigned int nRandomNumber = 0;
	char szRandomNumer[MAX_PATH] = {0};
	char cRecvBuff[MAX_BUFFER_SIZE] = {0};

	srand((unsigned int)(time(0)));         // set the random base number
	nRandomNumber = rand() % RandomNumber;  // get the random number
	m_nRandomNumber = nRandomNumber;
	sprintf(szRandomNumer, "RandomNumber:%d", nRandomNumber);

	for (unsigned int i = 0; i < 1; i++)
	{
		// Send the random number message.
		m_netComm.SendUdpPackage(sock, szRandomNumer);

		if (m_netComm.RecvUdpPackage(sock, cRecvBuff, MAX_BUFFER_SIZE) != SOCKET_ERROR)
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

//=========================================================================
// To start the udp communication.
void CUpperMachineComm::StartUdpComm()
//=========================================================================
{
	// Get the udp communication socket.
	SOCKET sockUdpComm = m_netComm.CreateUdpSock();

	// Set recvfrom return message timeout.
	int nTimeOut = HEART_PACKAGE_INTERNAL*2;
	setsockopt(sockUdpComm, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));

	// Send random number.
	SendRandomNumber(sockUdpComm);

	// Non-main server running mode.
	if (!IsMainServerRunningMode())
	{
		// Create the client socket.
		SOCKET sockUdpComm = m_netComm.CreateUdpSock();

		// Set the recvfrom wait time.
		int nTimeOut = HEART_PACKAGE_INTERNAL*2;
		setsockopt(sockUdpComm, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));

		char cRecvBuff[MAX_BUFFER_SIZE] = {0};
		// Receive stream datas from network firstly.
		if (m_netComm.RecvUdpPackage(sockUdpComm, cRecvBuff, MAX_BUFFER_SIZE) != SOCKET_ERROR)
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

	if (m_upperMachineMode == DOUBLE_MODE){
		// Double upper machine mode.
		StartUdpBroadcastDoubleServer();
	}
}


//=========================================================================
// Start the udp broadcast server thread.
UINT WINAPI CUpperMachineComm::StartUdpBroadcastServerThreadDouble(LPVOID lpVoid)
//=========================================================================
{
	CUpperMachineComm* pThis = reinterpret_cast<CUpperMachineComm*>(lpVoid);

	pThis->m_sockUdpComm = NULL;
	// Get the udp communication socket.
	pThis->m_sockUdpComm = pThis->m_netComm.CreateUdpSock(); 

	// Set the recvfrom wait time.
	const int nTimeOut = HEART_PACKAGE_INTERNAL*2; //
	setsockopt(pThis->m_sockUdpComm, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));
    bool ifReceive = false;

    while (!g_bAppNormalExit){
        pThis->OnTimerUdpBroadcastServerDouble();
		Sleep(HEART_PACKAGE_INTERNAL);
	}

	_endthreadex(0);
	return 1;
}

void CUpperMachineComm::OnTimerUdpBroadcastServerDouble()
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
        if (m_toolsFunction.MatchIpAddress(m_pIpAdapterInfo, szRecvIpAddress)){
            return;  // repeat if not local machine ip address
        }

		//如果不是配置的主机发的包,不处理....
		const wstring lpszOtherMachineIpAddr = CDataEngineAppConfig::GetInstance()->GetAnotherServerIP();
		//LPCTSTR lpszOtherMachineIpAddr = m_toolsFunction.GetProfileValue(_T("doublemachines"), _T("othermachineipaddr"), FILE_CONFIG_DATA);
		string strOtherMachineIpAddr = UtilString::ConvertWideCharToMultiByte(lpszOtherMachineIpAddr);
		if (strcmp(szRecvIpAddress, strOtherMachineIpAddr.c_str()) != 0){
			return;
		}

        // Main server running mode should listen FORCE_RUNNING_AS_MAIN_SERVER message.
        if (!IsMainServerRunningMode() )
        {
            // Treat with the received datas.
            TreatRecvDatas(szRecvBuff, false);
        }

        m_bLastTimeNoReceive = false;
    }
    else
    {
        //when receive no data from other pc, check gateway.
        CheckGatewayActive();

        // Can not receive the package from the main server.
        const bool ifHigh = IsHighPriorityMode();
        bool ifMain = true;
        const bool ifMainOld = IsMainServerRunningMode();
        if(ifMainOld){
            SetMachineModes(ifMainOld, ifHigh, false);
            return;
        }

        if(m_bActiveGateway){
            //can ping through plc
            ifMain = IsMainServerRunningMode() || IsOtherMachineActive();
            
            if(m_bLastTimeNoReceive){
                //already wait once
                ifMain = true;
            }
            else{
                ifMain = false;
                SetMachineModes(ifMain, ifHigh, false);

                const int randret = rand() % 5;
                const int sleeptime = HEART_PACKAGE_INTERNAL * randret;
                Sleep(sleeptime);
                m_bLastTimeNoReceive = true;                
            }
        }
        else{
            ifMain = false;
        }
        //const bool ifMain = (IsMainServerRunningMode() || IsOtherMachineActive() ) && m_bActiveGateway;
        SetMachineModes(ifMain, ifHigh, false);
    }
}

void CUpperMachineComm::CheckGatewayActive()
{
    //must be Local vars.
	m_bActiveGateway = true;
	if(m_strGateway != "localhost")
		m_bActiveGateway = m_ping.Ping(m_strGateway);
}

//=========================================================================
// To start the udp broadcast server(double machine).
void CUpperMachineComm::StartUdpBroadcastDoubleServer()
{
	HANDLE hThread = NULL;
	// Call the UdpBroadcastServerThread thread function.
	hThread = (HANDLE)_beginthreadex(NULL, 0, StartUdpBroadcastServerThreadDouble, this, 0, NULL);
	m_vecThreadHandles.push_back(hThread);
}


//=========================================================================
// Loop send the heart package
// e thread.
UINT WINAPI LoopSendHeartPackageThread(LPVOID lpVoid)
//=========================================================================
{
	CUpperMachineComm* cUpperMachineComm = reinterpret_cast<CUpperMachineComm*>(lpVoid);
	// Get the udp communication socket.
	SOCKET sockUdp = cUpperMachineComm->m_netComm.CreateUdpSock();
	while (!g_bAppNormalExit)
	{
		// Verify whether the local machine running as main server mode.
		if (cUpperMachineComm->IsMainServerRunningMode() && cUpperMachineComm->IsHeartActive()){
			// Send the heart package message.
			cUpperMachineComm->m_netComm.SendUdpPackage(sockUdp, LISTEN_PACKAGE);
			Sleep(HEART_PACKAGE_INTERNAL);
		}
		else{
			Sleep(1000);
		}
	}
	_endthreadex(0);
	return 1;
}

//=========================================================================
// Loop send the heart package.
void CUpperMachineComm::LoopSendHeartPackage()
//=========================================================================
{
	HANDLE hThread = NULL;
	// Call the LoopSendHeartPackageThread thread function.
	hThread = (HANDLE)_beginthreadex(NULL, 0, LoopSendHeartPackageThread, this, 0, NULL);
	m_vecThreadHandles.push_back(hThread);
}

//=========================================================================
// set the status of sending heart package
void CUpperMachineComm::SetHeartStatus(bool bActive)
//=========================================================================
{
	m_bHeartActive = bActive;
}

//=========================================================================
// Is as the server running program mode?
bool CUpperMachineComm::IsMainServerRunningMode() const
//=========================================================================
{
	return m_bMainServerRunningMode;
}

//=========================================================================
// Is as the high priority by compare with the random number?
bool CUpperMachineComm::IsHighPriorityMode() const
//=========================================================================
{
	return m_bHighPriority;
}

//=========================================================================
// Is the other machine activitity?
bool CUpperMachineComm::IsOtherMachineActive() const
//=========================================================================
{
	return m_bOtherMachineActive;
}

//=========================================================================
// Is send heart package in a loop?
bool CUpperMachineComm::IsHeartActive() const
//=========================================================================
{
	return m_bHeartActive;
}

//=========================================================================
// Saved the splitted database datas struct Data_Sensor.
//void CUpperMachineComm::SetSplittedDbDatasDataSensor(Data_Sensor& dsSplit)
////=========================================================================
//{
//	m_DbDatasDataSensor = dsSplit;
//}

//=========================================================================
// Saved the splitted database datas hash_map.
void CUpperMachineComm::SetSplittedDbDatasHashMap(hash_map<int, wstring>& hashmapValue)
//=========================================================================
{
	m_hashmapValue = hashmapValue;
}

//=========================================================================
// Saved the splitted equipment status hash_map.
void CUpperMachineComm::SetSplittedEqmStatusHashMap(hash_map<int, bool>& hashmapEqmStatus)
//=========================================================================
{
	m_hashmapEqmStatus = hashmapEqmStatus;
}

//=========================================================================
// Set the upper machine mode(single or double machines).
void CUpperMachineComm::SetUpperMachineMode(eUpperMachineMode mode)
//=========================================================================
{
	m_upperMachineMode = mode;
}

//=========================================================================
// Get the uppder machine mode(single or double machines.
eUpperMachineMode CUpperMachineComm::GetUpperMachineMode() const
//=========================================================================
{
	return m_upperMachineMode;
}

void CUpperMachineComm::StopSendAndReceiveThread()
{
	for (unsigned int i = 0; i < m_vecThreadHandles.size(); i++)
	{
		::TerminateThread(m_vecThreadHandles[i], 0);
		CloseHandle(m_vecThreadHandles[i]);
		WaitForSingleObject(m_vecThreadHandles[i], INFINITE);
		m_vecThreadHandles[i] = NULL;
	}
}

void CUpperMachineComm::ResumeSendAndReceiveThread()
{
	LoopSendHeartPackage();
	StartUdpBroadcastDoubleServer();
}
