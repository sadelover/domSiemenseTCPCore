// NetworkComm.h dll implement.

#include "stdafx.h"
#include "NetworkComm.h"

// Global object.
HWND g_hMainWnd = NULL;
bool g_bAutoUpdateClose = false;
bool g_bAppNormalExit = false;

// Static object.
CNetworkComm* CNetworkComm::m_csNetworkCommObj = NULL;
Mutex CNetworkComm::m_mutexLock;


//======================================================================
// Default constructor.
CNetworkComm::CNetworkComm()
//======================================================================
{
	// Initialize parameters and objects.
	InitParams();
	
	// Delete the old log file.
	DeleteOldLogFile();
}

//======================================================================
// Default destructor.
CNetworkComm::~CNetworkComm()
//======================================================================
{
}

//======================================================================
// Get the singleton instance of assign object.
CNetworkComm* CNetworkComm::GetInstance()
//======================================================================
{
	if (m_csNetworkCommObj == NULL)
	{
		Scoped_Lock<Mutex> mut(m_mutexLock);
		m_csNetworkCommObj = new CNetworkComm();
		atexit(DestroyInstance);
	}

	return m_csNetworkCommObj;
}

//======================================================================
// Destroy the singleton instance of assign object.
void CNetworkComm::DestroyInstance()
//======================================================================
{
	if (m_csNetworkCommObj != NULL)
	{
		delete m_csNetworkCommObj;
		m_csNetworkCommObj = NULL;
	}
}

//======================================================================
// Initialize the message parameter struct.
void CNetworkComm::InitMessageParam(const tagMESSAGEPARAM& tMessageParam)
//======================================================================
{
	m_pMessageParam = new MessageParam;
	memcpy(m_pMessageParam, &tMessageParam, sizeof(tagMESSAGEPARAM));
}

//======================================================================
// Initialize parameters and object.
void CNetworkComm::InitParams()
//======================================================================
{
	// Set Log file path.
 	m_toolsFunction.SetLogFilePath(TEXT("LogFile.dat"));
}

//======================================================================
// Delete the old log file.
void CNetworkComm::DeleteOldLogFile()
//======================================================================
{
	TCHAR tszLogFilePath[MAX_PATH_SIZE] = {0};

	_tcscpy_s(tszLogFilePath, MAX_PATH_SIZE, m_toolsFunction.GetLogFilePath());
	if (_tcscmp(tszLogFilePath, TEXT("")) != 0 
	 && m_toolsFunction.IsFileExist(tszLogFilePath))
	{
		BOOL bResult = DeleteFile(tszLogFilePath);
	}
}

//======================================================================
// To start the udp communication.
//void CNetworkComm::StartUdpComm()
////======================================================================
//{
//	m_upperMachineComm.StartUdpComm();
//}

//======================================================================
// To start the udp communication client.
//void CNetworkComm::StartUdpCommClient()
////======================================================================
//{
//	m_upperMachineComm.StartUdpCommClient();
//}

//======================================================================
// To run the udp broadcast.
//void CNetworkComm::UdpBroadcastRunning(const Data_Sensor& dataSensor,
//									   const hash_map<int, wstring>& hashmapValue, 
//									   const hash_map<int, int>& hashmapEqmStatus)
////======================================================================
//{
//	// Network stream datas realtime update.
//	m_upperMachineComm.UdpBroadcastRunning(dataSensor, hashmapValue, hashmapEqmStatus);
//}

//======================================================================
// To run the udp broadcast.
//void CNetworkComm::UdpBroadcastRunning2(const Data_Sensor& dataSensor,
//									    const hash_map<int, wstring>& hashmapValue, 
//									    const hash_map<int, int>& hashmapEqmStatus)
////======================================================================
//{
//	// Network stream datas realtime update.
//	m_upperMachineComm.UdpBroadcastRuuning2(dataSensor, hashmapValue, hashmapEqmStatus);
//}

//======================================================================
// Force running as main server.
//void CNetworkComm::ForceRunningAsMainServer()
////======================================================================
//{
//	m_upperMachineComm.ForceRunningAsMainServer();
//}

//======================================================================
// Get the splitted database datas struct Data_Sensor.
//Data_Sensor CNetworkComm::GetSplittedDbDatasDataSensor()
////======================================================================
//{
//	return m_DbDatasDataSensor;
//}

//======================================================================
// Get the splitted database datas hash_map.
hash_map<int, wstring> CNetworkComm::GetSplittedDbDatasHashMap()
//======================================================================
{
	return m_hashmapValue;
}

//======================================================================
// Get the splitted equipment status hash_map.
hash_map<int, bool> CNetworkComm::GetSplittedEqmStatusHashMap() 
//======================================================================
{
	return m_hashmapEqmStatus;
}

//======================================================================
// Is as the server running program mode?
//bool CNetworkComm::IsMainServerRunningMode() const
////======================================================================
//{
//	return m_upperMachineComm.IsMainServerRunningMode();
//}

//======================================================================
// Is as the high priority by compare with the random number?
//bool CNetworkComm::IsHighPriorityMode() const
////======================================================================
//{
//	return m_upperMachineComm.IsHighPriorityMode();
//}

//======================================================================
// Is the other machine activitity?
//bool CNetworkComm::IsOtherMachineActive() const
////======================================================================
//{
//	return m_upperMachineComm.IsOtherMachineActive();
//}

//======================================================================
// Set the upper machine mode.
//void CNetworkComm::SetUpperMachineMode(eUpperMachineMode mode)
////======================================================================
//{
//	m_upperMachineComm.SetUpperMachineMode(mode);
//}

//======================================================================
// Get the upper machine mode.
//eUpperMachineMode CNetworkComm::GetUpperMachineMode() const
////======================================================================
//{
//	return m_upperMachineComm.GetUpperMachineMode();
//}

//======================================================================
// To start the iocp client.
//bool CNetworkComm::StartIocpClient()
////======================================================================
//{
//	return m_iocpClient.StartIocpClient();
//}

//======================================================================
// To stop the iocp client.
//void CNetworkComm::StopIocpClient()
////======================================================================
//{
//	m_iocpClient.StopIocpClient();
//}

//======================================================================
// To send network datas.
bool CNetworkComm::SendNetworkDatas(const string& strDatas)
//======================================================================
{
	const bool ifok = __super::SendNetworkDatas(strDatas);
    Sleep(200); // must sleep
    return ifok;
}

//======================================================================
// Get the update sql statement.
//void CNetworkComm::GetUpdateSqlStatement(string& strDest, const string& strSrc)
////======================================================================
//{
//	m_iocpClient.GetUpdateSql(strDest, strSrc);
//}

//======================================================================
// Is auto update close?
bool CNetworkComm::IsAutoUpdateClose() const
//======================================================================
{
	return g_bAutoUpdateClose;
}

//======================================================================
// is application normal exit?
bool CNetworkComm::IsAppNormalExit() const
//======================================================================
{
	return g_bAppNormalExit;
}

//======================================================================
// is the client connected to the server?
//bool CNetworkComm::IsConnectedToServer()
////======================================================================
//{
//	return m_iocpClient.IsConnectedToServer();
//}

//======================================================================
// Set the status of the application normal exit.
void CNetworkComm::SetAppNormalExit(bool bNormalExit)
//======================================================================
{
	g_bAppNormalExit = bNormalExit;
}

//======================================================================
// Set the main dialog window handle.
void CNetworkComm::SetMainWndHandle(HWND hMainWnd)
//======================================================================
{
	g_hMainWnd = hMainWnd;
}
