#ifndef __NETWORK_COMM_H__
#define __NETWORK_COMM_H__

#include "UpperMachineComm.h"
#include "IocpClient.h"
#include "../Tools/CustomTools/CustomTools.h"


class CNetworkComm : 
    public CUpperMachineComm
    , public CIocpClient
{
public:
	virtual ~CNetworkComm();                         // default destructor

 	static CNetworkComm* GetInstance();              // get the singleton instance of assign object
	static void DestroyInstance();                   // destroy the singleton instance of assign objec

	//-----------------------------------------------------------
	// Upper machine client.
	//-----------------------------------------------------------
	//inline void StartUdpCommClient();				           // to start the udp communication client
	void InitMessageParam(__in const tagMESSAGEPARAM& tMessageParam);  // initialize the message parameter struct
	//Data_Sensor GetSplittedDbDatasDataSensor();         // get the splitted database datas struct Data_Sensor
	hash_map<int, wstring> GetSplittedDbDatasHashMap(); // get the splitted database datas hash_map
	hash_map<int, bool> GetSplittedEqmStatusHashMap();  // get the splitted equipment status hash_map

	//-----------------------------------------------------------
	// Remote socket client.
	//-----------------------------------------------------------
	//inline bool StartIocpClient();                             // to start the iocp client
	//inline void StopIocpClient();                              // to stop the iocp client
	virtual bool SendNetworkDatas(__in const string& strDatas); // to send network datas
	
	bool IsAutoUpdateClose() const;			           // is auto update close?
	bool IsAppNormalExit() const;                       // is application normal exit?
	//bool IsConnectedToServer();                         // is the client connected to the server?
	void SetAppNormalExit(__in bool bNormalExit);       // set the status of the application normal exit
	void SetMainWndHandle(__in HWND hMainWnd);          // set the main dialog window handle
	void SetMainDlgWindowHandle(__in HWND hWnd);        // set the main dialog window handle	
	//void GetUpdateSqlStatement(__out string& strDest, __in const string& strSrc);  // get the update sql statement

protected:
	void InitParams();                    // initialize parameters and object
	void DeleteOldLogFile();              // delete the old log file

private:
	CNetworkComm();                              // default constructor

public:
	//CIocpClient         m_iocpClient;
	//CUpperMachineComm m_upperMachineComm;

private:
	static CNetworkComm*    m_csNetworkCommObj;     // singleton object
	static Mutex            m_mutexLock;                    // mutex object

	pMessageParam           m_pMessageParam;               // message structure

	//Data_Sensor             m_DbDatasDataSensor;             // database datas double vector
	hash_map<int, wstring>  m_hashmapValue;       // database datas string hash_map
	hash_map<int, bool>     m_hashmapEqmStatus;      // equipment status hash_map

	CToolsFunction m_toolsFunction;
};

#endif  // __NETWORK_COMM_H__
