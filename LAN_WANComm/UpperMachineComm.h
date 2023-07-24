#ifndef __UPPER_MACHINE_COMM_H__
#define __UPPER_MACHINE_COMM_H__

#pragma once

#include "Definition.h"
#include "DatasHandle.h"
#include "Tools/NetComm/NetComm.h"
#include "PingIcmp.h"
#include <process.h>     // used for _beginthreadex()
#include <stdlib.h>      
#include <time.h>        // used for random number
#include <vector>
using namespace std;

//////////////////////////////////////////////////////////////////////////

#define LISTEN_PACKAGE                    "Listen_Package"                    // listen package
#define FORCE_RUNNING_AS_MAIN_SERVER      "Force_Running_As_Main_Server"      // force running as main server
#define CALLBACK_FORCE_RUNNING_MESSAGE    "Callback_Force_Running_Message"    // callback force running as main server                                 
#define CONFIRM_FORCE_RUNNING_MESSAGE     "Confirm_Force_Running_Message"     // confirm force running as backup server
#define RandomNumber 1000

//////////////////////////////////////////////////////////////////////////

enum eUpperMachineMode
{
	NULL_MODE,
	SINGLE_MODE,
	DOUBLE_MODE,
};


class CUpperMachineComm
{
public:
	CUpperMachineComm();
	virtual ~CUpperMachineComm();

	//-----------------------------------------------------------
	// Upper machine server(common interface). 
	//-----------------------------------------------------------
	void StartUdpComm();                             // to start the udp communication
	void SendBroadcastDatas(__in SOCKET sock);       // send the broadcast datas
	void TreatRecvDatas(__in char* pszRecvBuffer, __in bool bInit);  // treat with the received datas
	void TreatRealtimeDatas(__in const char* pszRecvDatas, __in int nRecvSize);  // treat with the real time datas
	void SetUpperMachineMode(__in eUpperMachineMode mode);  // set the upper machine mode(single or double machines)
	eUpperMachineMode GetUpperMachineMode() const;  // get the uppder machine mode(single or double machines)

	//-----------------------------------------------------------
	// Single upper machine mode.
	//-----------------------------------------------------------
	//void UdpBroadcastRunning(__in const Data_Sensor& dataSensor, 
	//	__in const hash_map<int, wstring>& hashmapValue, 
	//	__in const hash_map<int, int>& hashmapEqmStatus);  // to run the udp broadcast

	////-----------------------------------------------------------
	//// Double upper machine mode.
	////-----------------------------------------------------------
	//void UdpBroadcastRuuning2(__in const Data_Sensor& dataSensor,
	//	__in const hash_map<int, wstring>& hashmapValue,
	//	__in const hash_map<int, int>& hashmapEqmStatus);  // to run the udp broadcast(double machine)

//	void ForceRunningAsMainServer();                 // force running as main server
//	void ConfirmRunningAsBackupServer(__in SOCKET sockUdpComm, 
//		__in SOCKADDR FAR* sockAddr, 
//		__in int nSize);							 // confirm force running as backup server
	inline void SetMachineModes(__in bool bServerRunning, 
		__in bool bHighPriority, 
		__in bool bOtherMachineActive);

	inline bool IsMainServerRunningMode() const;     // is as the server running program mode?
	inline bool IsHighPriorityMode() const;          // is as the high priority by compare with the random number?
	inline bool IsOtherMachineActive() const;        // is the other machine activitity?
	inline bool IsHeartActive() const;               // is send heart package in a loop

	//-----------------------------------------------------------
	// upper machine client.
	//-----------------------------------------------------------	
	void StartUdpCommClient();                       // to start the udp communication client
	//inline void SetSplittedDbDatasDataSensor(__in Data_Sensor& dsSplit);             // saved the splitted database datas struct Data_Sensor
	inline void SetSplittedDbDatasHashMap(__in hash_map<int, wstring>& hashmapValue);// saved the splitted database datas hash_map
	inline void SetSplittedEqmStatusHashMap(__in hash_map<int, bool>& hashmapEqmStatus);  // saved the splitted equipment status hash_map

protected:
	//-----------------------------------------------------------
	// Upper machine server(common interface). 
	//-----------------------------------------------------------
	//void GetUpdateDatas(__in const Data_Sensor& dataSensor, 
	//	__in const hash_map<int, wstring>& hashmapValue);  // get the update datas
	void GetUpdateEqmStatus(__in const hash_map<int, int>& hashmapEqmStatus);  // get the update equipment status

	//-----------------------------------------------------------
	// Single upper machine mode.
	//-----------------------------------------------------------
	//void StartUdpBroadcastServer();           // to start the udp broadcast server

	//-----------------------------------------------------------
	// Double upper machine mode.
	//-----------------------------------------------------------
	void StartUdpBroadcastDoubleServer();          // to start the udp broadcast server
	void LoopSendHeartPackage();              // loop send the heart package

	void SetHeartStatus(__in bool bActive);   // set the status of sending heart package

	//-----------------------------------------------------------
	// Upper machine client.
	//-----------------------------------------------------------
//	void StartUdpBroadcastClient();			 // to start the udp broadcast client

  ///  static UINT WINAPI StartUdpBroadcastServerThread(LPVOID lpVoid);
    static UINT WINAPI StartUdpBroadcastServerThreadDouble(LPVOID lpVoid);
    void               OnTimerUdpBroadcastServerDouble();

    void               CheckGatewayActive();


public:
	void	StopSendAndReceiveThread();
	void	ResumeSendAndReceiveThread();

private:
	void InitParams();                               // initialize parameters

	//-----------------------------------------------------------
	// Double upper machine mode.
	//-----------------------------------------------------------
	void SendRandomNumber(__in SOCKET sock);         // send the random number

public:
	CNetComm m_netComm;                              // the network communication class

	pMessageParam m_pMessageParam;                   // message structure
	IP_ADAPTER_INFO* m_pIpAdapterInfo;               // ip adapter information

	CToolsFunction m_toolsFunction;

protected:
	CDatasHandle m_datasHandle;

private:
	vector<HANDLE>          m_vecThreadHandles;                             

	bool m_bMainServerRunningMode;                   // the server running mode
	bool m_bHighPriority;                        	 // the high priority of double running machine
	bool m_bOtherMachineActive;                  	 // the other machine active?
	bool m_bHeartActive;
    bool m_bActiveGateway;
    bool m_bLastTimeNoReceive;

	CRITICAL_SECTION        m_criMachineModes;              // the critical section of machine modes

	//Data_Sensor             m_DbDatasDataSensor;                 // database datas double vector
	hash_map<int, wstring>  m_hashmapValue;           // database datas string hash_map
	hash_map<int, bool>     m_hashmapEqmStatus;          // equipment status hash_map

	UINT                    m_nRandomNumber;                    // random number
	eUpperMachineMode       m_upperMachineMode;

	char m_csLANDbDatasBuff[MAX_BUFFER_SIZE];    	 // sendto/recvfrom database datas buffer
	char m_csLANEqmStatusBuff[MAX_BUFFER_SIZE];  	 // sendto/recvfrom equipment status buffer

    SOCKET                  m_sockUdpComm ;
	CPingIcmp				m_ping;
    string                  m_strGateway;
};

#endif  // __UPPER_MACHINE_COMM_H__