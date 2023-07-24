//BacnetComm.h
#pragma once

#include "tools/CustomTools/CustomTools.h"
#include "../bacnet/include/bacdef.h"
#include "../bacnet/include/apdu.h"
#include "../bacnet/include/device.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

//struct BACNET_ADDRESS;
//struct BACNET_CONFIRMED_SERVICE_ACK_DATA;
struct BACnet_Read_Property_Data;
typedef struct BACnet_Read_Property_Data BACNET_READ_PROPERTY_DATA;

struct BACnet_Read_Access_Data;
typedef struct BACnet_Read_Access_Data BACNET_READ_ACCESS_DATA;

struct BACnet_Application_Data_Value;
typedef struct BACnet_Application_Data_Value BACNET_APPLICATION_DATA_VALUE;

const UINT C_BANECT_PROPERTY = 85;

class CBacnetServerCtrl
{
public: 
	CBacnetServerCtrl();             	 						          // default consructor
	virtual ~CBacnetServerCtrl();    	 						          // default destructor

public:

	void	SetPointList(const vector<DataPointEntry>& entrylist);

	static void    Init_Service_Handlers();
	
	bool			StartBacnetServer();
	bool			ExitBacnetServer();
	bool			GetReceiveThreadExit();
	bool            CreateThreadReceive();

	void	Exit();

	static UINT WINAPI      ThreadReceive(LPVOID pParam);

	//  处理函数
	static void ResponseIAmHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src);
	static void ResponseWhoIsHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src);
	static void ResponseReadProperty(
		uint8_t * service_request,
		uint16_t service_len,
		BACNET_ADDRESS * src,
		BACNET_CONFIRMED_SERVICE_DATA * service_data);

	static void  InitObjectPoint0();
	static char * GetObjectName0(uint32_t object_instance);
	static bool GetDeviceObjectInstance(uint32_t object_instance);
	static int ReadDeviceObjects(BACNET_READ_PROPERTY_DATA * rp_data);

protected:

	//点的列表
	 vector<DataPointEntry>	m_vecpointlist;  

	Beopdatalink::CLogDBAccess* m_logsession;
	Project::Tools::Mutex	m_lock;

	HANDLE              m_receive_thread;		//接收线程
	bool				m_bExitReceive;


public:
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);


};
