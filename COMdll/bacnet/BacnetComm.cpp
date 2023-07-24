// BacnetComm.cpp 
#include "stdafx.h"
#include "BacnetComm.h"
#include "Tools/EngineInfoDefine.h"
#include "Tools/Util/UtilString.h"
#include <process.h>  //about thread

#include "../LAN_WANComm/NetworkComm.h"
#include "../LAN_WANComm/Tools/ToolsFunction/ToolsFunction.h"
#include "../DebugLog.h"
#include <iostream>
#include <sstream>
//#include "vld.h"
#pragma warning(disable:4267)
//////////////////////////////////////////////////////////////////////////

#include "../bacnet/include/tsm.h"
#include "../bacnet/include/address.h"
#include "../bacnet/include/device.h"
#include "../bacnet/include/datalink.h"
#include "../bacnet/include/handlers.h"
#include "../bacnet/include/client.h"
#include "../bacnet/include/dlenv.h"
#include "../bacnet/include/iam.h"
#include "../bacnet/include/bactext.h"

///////////////////////////////////////////////////////////////////
// All included BACnet objects */
object_functions_t Object_Table[] = {
	{DEVICE_OBJ_FUNCTIONS},
	{MAX_BACNET_OBJECT_TYPE, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};

const int MAX_PROPERTY_VALUES =  64;
const int MAX_WHOIS_PER_HOUR = 3;
static const int  g_nBacnetReqHeaderLength = 57;
static const int  g_nBacnetOneReqLength = 11;

Beopdatalink::CLogDBAccess* CBacnetCtrl::m_logsession = NULL;
CRITICAL_SECTION CBacnetCtrl::m_criticalSection = {0};

vector<DataPointEntry> CBacnetCtrl::m_pointlistAI;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistAO;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistBI;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistBO;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistAV;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistBV;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistMI;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistMO;
vector<DataPointEntry>  CBacnetCtrl::m_pointlistMV;

vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecAI;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecAO;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecBI;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecBO;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecAV;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecBV;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecMI;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecMO;
vector<vector<DataPointEntry>>  CBacnetCtrl::m_vecMV;

SYSTEMTIME	CBacnetCtrl::m_sExecuteTime;
string		CBacnetCtrl::m_strExecuteLog;
string		CBacnetCtrl::m_strUpdateLog;
int			CBacnetCtrl::m_nReqInvokeIdErrCount;
COleDateTime CBacnetCtrl::m_oleUpdateTime;
COleDateTime CBacnetCtrl::m_oleStartTime;
COleDateTime CBacnetCtrl::m_oleSendWhoISTime;
int			CBacnetCtrl::m_nReadMode;
int			CBacnetCtrl::m_nCmdCount;
int			CBacnetCtrl::m_nResponseCount;
int			CBacnetCtrl::m_nLastInvolk;
int			CBacnetCtrl::m_nPointCount;
int			CBacnetCtrl::m_nUpdatePointCount;
int			CBacnetCtrl::m_nOutPut;
int			CBacnetCtrl::m_nBacnetBindOK;
int			CBacnetCtrl::m_nBacnetSendOK;
int			CBacnetCtrl::m_nBacnetResponseOK;
int			CBacnetCtrl::m_nBacnetRecOK;
int			CBacnetCtrl::m_nSendWhiIsCount;
int			CBacnetCtrl::m_nReadLimit;
hash_map<wstring,DWORD>	CBacnetCtrl::m_mapDeviceID;
hash_map<wstring,DataPointEntry*> CBacnetCtrl::m_mapPointInvolk;
hash_map<UINT8,hash_map<wstring,DataPointEntry*>> CBacnetCtrl::m_mapQueryPointInvolk;
HANDLE		CBacnetCtrl::m_sendCmdEvent;
COleDateTime CBacnetCtrl::m_oleSendCmdTime;
hash_map<wstring,int>	CBacnetCtrl::m_mapDeviceResponse;
//=====================================================================
// Default constructor.
//=====================================================================

int CBacnetCtrl::m_nLogLevel = 0; 
CBacnetCtrl::CBacnetCtrl(int nReadInterval, int nReadTypeInterval, int nReadCmdInterval,int nReadMode,int nReadTimeOut,int nPrecision, int nReadLimit, int nWritePriority) 
	:m_readdata_thread(NULL)
	,m_hsendcommand_thread(NULL)
	,m_active_thread(NULL)
	,m_exit_readdata_thread(false)
	,m_exit_sendcommand_thead(false)
	,m_nReadInterval(nReadInterval)
	,m_nReadTypeInterval(nReadTypeInterval)
	,m_nReadCmdInterval(nReadCmdInterval)
	,m_nReadTimeOut(2000)
	,m_nPrecision(nPrecision)
	,m_nWritePriority(nWritePriority)
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_oleSendCmdTime = COleDateTime::GetCurrentTime();
	m_oleSendWhoISTime = COleDateTime::GetCurrentTime();
	m_oleStartTime = m_oleUpdateTime;
	m_strErrInfo = "";
	m_nReadMode = nReadMode;
	m_nCmdCount = 0;
	m_nResponseCount = 0;
	m_nLastInvolk = 0;
	m_nPointCount = 0;
	m_nUpdatePointCount = 0;
	m_nOutPut = 0;
	m_nBacnetBindOK = 0;
	m_nBacnetSendOK = 0;
	m_nBacnetResponseOK = 0;
	m_nBacnetRecOK = 0;
	m_nSendWhiIsCount = 0;
	m_strUpdateLog = "";
	m_nReadLimit = nReadLimit;
	m_mapDeviceID.clear();
	m_mapPointInvolk.clear();
	m_mapQueryPointInvolk.clear();
	m_sendCmdEvent = ::CreateEvent(NULL, false, false, L"");
	m_bNeedActive = false;
	m_mapDeviceResponse.clear();


}

//=====================================================================
// Default destructor.
//=====================================================================
CBacnetCtrl::~CBacnetCtrl()
{
	if (m_readdata_thread){
		CloseHandle(m_readdata_thread);
		m_readdata_thread = NULL;
	}

	if(m_active_thread)
	{
		CloseHandle(m_active_thread);
		m_active_thread = NULL;
	}

	if (m_hsendcommand_thread){
		CloseHandle(m_hsendcommand_thread);
		m_hsendcommand_thread = NULL;
	}
}


void CBacnetCtrl::Disconnect()
{
	m_exit_readdata_thread = true;
	m_exit_sendcommand_thead = true;
	WaitForSingleObject(m_active_thread,INFINITE);
	WaitForSingleObject(m_hsendcommand_thread, INFINITE);
	WaitForSingleObject(m_readdata_thread, INFINITE);
}

//////////////////////////////////////////////////////////////////////////

void CBacnetCtrl::InitServiceHandlers( void)
{
	Device_Initialize_Object_Functions(&Object_Table[0]);
	Device_Init();
	// we need to handle who-is
	//   to support dynamic device binding to us */
	//apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);

	// handle i-am to support binding to other devices */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, MyIAMHandler);

	// set the handler for all the services we don't implement
	//  It is required to send the proper reject message... */
	apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);

	// we must implement read property - it's required! */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY, handler_read_property);

	//handle the ack coming back 
	apdu_set_confirmed_simple_ack_handler(SERVICE_CONFIRMED_WRITE_PROPERTY, CBacnetCtrl::HandleWritePropertyAck);

	// handle the data coming back from confirmed requests */
	apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY, CBacnetCtrl::HandleReadPropertyAck);
	apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE, CBacnetCtrl::HandleReadPropertyMultiAck);

	// handle any errors coming back */
	apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
	apdu_set_abort_handler(MyAbortHandler);
	apdu_set_reject_handler(MyRejectHandler);

}

void CBacnetCtrl::InitServiceHandlersForWhois()
{
	Device_Init();
	/* Note: this applications doesn't need to handle who-is
	it is confusing for the user! */
	/* set the handler for all the services we don't implement
	It is required to send the proper reject message... */
	apdu_set_unrecognized_service_handler_handler
		(handler_unrecognized_service);
	/* we must implement read property - it's required! */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY, handler_read_property);
	/* handle the reply (request) coming back */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, MyIAMAddHandler);
	/* handle any errors coming back */
	apdu_set_abort_handler(MyAbortHandler);
	apdu_set_reject_handler(MyRejectHandler);
}

DataPointEntry* CBacnetCtrl::FindDataFromDeviceIDTypeAddress( DWORD deviceid,UINT type, DWORD address )
{
	switch(type)
	{
	case OBJECT_ANALOG_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAI[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistAI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_ANALOG_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAO[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistAO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_ANALOG_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAV[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistAV[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBI[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistBI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBO[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistBO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBV[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistBV[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMI[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistMI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMO[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistMO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMV[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && GetBacnetDeviceID(entry.GetParam(1)) == deviceid)
				{
					return &m_pointlistMV[deviceIdx];
				}
			}
		}
		break;
	default:
		break;
	}
	return NULL;
}

//init bacnet connection
bool CBacnetCtrl::Connect( )
{
	// setup my info 
	Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
	address_init(m_nReadMode);
	InitServiceHandlers();
	dlenv_init();
	atexit(datalink_cleanup);
	CreateThreadRead();
	m_nReqInvokeIdErrCount = 0;
	m_nCmdCount = 0;
	m_nResponseCount = 0;
	m_nLastInvolk = 0;
	m_nUpdatePointCount = 0;
	m_nBacnetBindOK = 0;
	m_nBacnetSendOK = 0;
	m_nBacnetResponseOK = 0;
	m_nBacnetRecOK = 0;
	InitializeCriticalSection(&m_criticalSection);  //初始化临界区

	return true;
}
//////////////////////////////////////////////////////////////////////////

bool CBacnetCtrl::CreateThreadRead()
{
	m_readdata_thread = (HANDLE)_beginthreadex(NULL,0,ThreadRead,(LPVOID)this,NORMAL_PRIORITY_CLASS,NULL);
	assert(m_readdata_thread);
	int nSleep = 6;
	while(!GetSendCommandThreadExit())
	{
		if(nSleep <= 0)
		{
			break;
		}
		nSleep--;
		Sleep(1000);
	}

	m_active_thread = (HANDLE)_beginthreadex(NULL,0,ThreadActiveCommandsFunc,(LPVOID)this,NORMAL_PRIORITY_CLASS,NULL);
	m_hsendcommand_thread = (HANDLE)_beginthreadex(NULL,0,ThreadSendCommandsFunc,(LPVOID)this,NORMAL_PRIORITY_CLASS,NULL);
	assert(m_hsendcommand_thread);

	return true;
}


UINT WINAPI  CBacnetCtrl::ThreadRead(LPVOID pParam)
{
	CBacnetCtrl *pThis = static_cast<CBacnetCtrl*>(pParam);
	ASSERT(pThis);
	if (NULL == pThis)
		return 1;

	pThis->OnTimerWhois();

	InitServiceHandlers();

	const UINT c_sleep_ms = 10;
	while(!pThis->GetReadDataThreadExit())
	{
		pThis->OnTimerRead();
	}
	return 0;
}

void CBacnetCtrl::OnTimerRead()
{
	// try to bind with the device 
	const UINT timeout = 100;     // milliseconds */
	UINT16 pdu_len = 0;
	//BACNET_ADDRESS targetAddr; // = m_targetAddr;
	BACNET_ADDRESS srcAddr = { 0 };  // address where message came from */    
	UINT8 Rx_Buf[MAX_MPDU] = { 0 }; // buffer used for receive /

	pdu_len = datalink_receive(&srcAddr, &Rx_Buf[0], MAX_MPDU, timeout);
	if (pdu_len) {
		npdu_handler(&srcAddr, &Rx_Buf[0], pdu_len);		
	}
}

void CBacnetCtrl::OnTimerWhois()
{
	uint16_t pdu_len = 0;
	unsigned timeout = 100;     /* milliseconds */
	time_t total_seconds = 0;
	time_t elapsed_seconds = 0;
	time_t last_seconds = time(NULL);
	time_t current_seconds = 0;
	time_t timeout_seconds = apdu_timeout() / 1000 +10;
	bool Error_Detected = false;
	BACNET_ADDRESS src = {  0  }; 
	uint8_t Rx_Buf[MAX_MPDU] = { 0 };
	UINT addressCount = 0;

	//init again
	InitServiceHandlersForWhois();

	// send the request */
	Send_WhoIs(-1, -1);

	for (;;) {
		/* increment timer - exit if timed out */
		current_seconds = time(NULL);
		/* returns 0 bytes on timeout */
		pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
		/* process */
		if (pdu_len) {
			npdu_handler(&src, &Rx_Buf[0], pdu_len);
		}

		if (Error_Detected)
			continue;
		/* increment timer - exit if timed out */
		elapsed_seconds = current_seconds - last_seconds;
		if (elapsed_seconds) {
#if defined(BACDL_BIP) && BBMD_ENABLED
			bvlc_maintenance_timer(elapsed_seconds);
#endif
		}
		total_seconds += elapsed_seconds;
		if (total_seconds > timeout_seconds)
			break;
		/* keep track of time for next check */
		last_seconds = current_seconds;
	}
}


void CBacnetCtrl::MyErrorHandler(
	BACNET_ADDRESS * srcAddr,
	uint8_t invoke_id,
	BACNET_ERROR_CLASS error_class,
	BACNET_ERROR_CODE error_code)
{
	// did nothing.
	CString strLog;
	strLog.Format(_T("ERROR: Bacnet Error %s:%s \n"),Project::Tools::AnsiToWideChar(bactext_error_class_name(error_class)).c_str(),
		Project::Tools::AnsiToWideChar(bactext_error_code_name(error_code)).c_str());
	PrintLog(strLog.GetString(), LOG_ERROR);
}

void CBacnetCtrl::MyAbortHandler(
	BACNET_ADDRESS * srcAddr,
	uint8_t invoke_id,
	uint8_t abort_reason,
	bool server)
{
	// currently do nothing.
	CString strLog;
	strLog.Format(_T("ERROR: Bacnet Abort %s \n"),Project::Tools::AnsiToWideChar(bactext_abort_reason_name(abort_reason)).c_str());
	PrintLog(strLog.GetString(), LOG_ERROR);
}

void CBacnetCtrl::MyRejectHandler(
	BACNET_ADDRESS * src,
	uint8_t invoke_id,
	uint8_t reject_reason)
{
	// did nothing.
}



//////////////////////////////////////////////////////////////////////////
/** Handler for a ReadProperty ACK.
* @ingroup DSRP
* Doesn't actually do anything, except, for debugging, to
* print out the ACK data of a matching request.
*
* @param service_request [in] The contents of the service request.
* @param service_len [in] The length of the service_request.
* @param src [in] BACNET_ADDRESS of the source of the message
* @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
*                          decoded from the APDU header of this message.
*/

void CBacnetCtrl::HandleReadPropertyAck(
	uint8_t * service_request,
	uint16_t service_len,
	BACNET_ADDRESS * src,
	BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
	int len = 0;
	BACNET_READ_PROPERTY_DATA   data;

	len = rp_ack_decode_service_request(service_request, service_len, &data);
	if (len > 0) {
		OnReceiveRead(&data, src,service_data->invoke_id);
	}
}

void CBacnetCtrl::OnReceiveRead(BACNET_READ_PROPERTY_DATA* pData, BACNET_ADDRESS * src,UINT invokeId)
{
	ASSERT(pData);
	if(!pData)  return;
	/* for decode value data */
	BACNET_APPLICATION_DATA_VALUE* dataValue = new BACNET_APPLICATION_DATA_VALUE;
	ASSERT(dataValue);
	memset(dataValue, 0x00, sizeof(BACNET_APPLICATION_DATA_VALUE));

	int len = 0;
	uint8_t *application_data;
	int application_data_len;
	
	application_data = pData->application_data;
	application_data_len = pData->application_data_len;
	// FIXME: what if application_data_len is bigger than 255? 
	// value? need to loop until all of the len is gone... 
	len =  bacapp_decode_application_data(application_data,
		(uint8_t) application_data_len, dataValue);

	if (len < 0){
		SAFE_DELETE(dataValue);
		return;
	}

	uint32_t device_id = 0;
	if(!address_get_device_id(src,&device_id))
	{
		SAFE_DELETE(dataValue);
		return;
	}

	DataPointEntry* saveData = NULL;
	saveData = FindDataByDeviceIDTypeInvokedID(invokeId,pData->object_type,pData->object_instance);
	if(!saveData)
	{
		CString strLog;
		strLog.Format(_T("ERROR: Bacnet FindPoint Fail(Invoke:%d,Type:%d,Instance:%u). \r\n"),invokeId,pData->object_type,pData->object_instance);
		PrintLog(strLog.GetString(), LOG_ERROR);
		
		SAFE_DELETE(dataValue);
		return;
	}
	else
	{
		m_mapDeviceResponse[saveData->GetParam(1)] = 0;
	}
	CopyValue(dataValue, saveData);
	SAFE_DELETE(dataValue);
}

//////////////////////////////////////////////////////////////////////////

void  CBacnetCtrl::HandleReadPropertyMultiAck(UINT8 * service_request,
	UINT16 service_len,
	BACNET_ADDRESS * src,
	BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
	int len = 0;
	//BACNET_READ_ACCESS_DATA rRmpdata;
	BACNET_READ_ACCESS_DATA *pRpmData = NULL;
	BACNET_READ_ACCESS_DATA *old_rpm_data = NULL;
	BACNET_PROPERTY_REFERENCE *rpm_property = NULL;
	BACNET_PROPERTY_REFERENCE *old_rpm_property = NULL;
	BACNET_APPLICATION_DATA_VALUE *dataValue = NULL;
	BACNET_APPLICATION_DATA_VALUE *old_value = NULL;

	const UINT invokeId = service_data->invoke_id;

	pRpmData = new BACNET_READ_ACCESS_DATA; //must delete
	EnterCriticalSection(&m_criticalSection); //等待进入临界区，进入后加锁使其他线程不能进入

	if (pRpmData) 
	{
		len = rpm_ack_decode_service_request(service_request, service_len, pRpmData);
	}
	bool bResponseOK = false;
	if (len > 0) 
	{
		m_nBacnetResponseOK = 1;
		bResponseOK = OnReceiveReadMultiple(pRpmData,src,invokeId);
	}
	
	//clear data
	ClearReadAccessData(&pRpmData);
	LeaveCriticalSection(&m_criticalSection);  //离开 开锁

	if(bResponseOK)
	{
		//收到数据激活下一条命令
		SetSendCmdEvent();
	}
}

bool CBacnetCtrl::OnReceiveReadMultiple(BACNET_READ_ACCESS_DATA* pRpmData, BACNET_ADDRESS * src,UINT invokeId)
{
	ASSERT(pRpmData);
	CString strLog;

	if(!pRpmData)  
		return false;

	if(!src)
		return false;

	uint32_t device_id = 0;
	if(!address_get_device_id(src,&device_id))
		return false;

	BACNET_PROPERTY_REFERENCE *rpm_property = NULL;
	BACNET_APPLICATION_DATA_VALUE *dataValue = NULL;
	BACNET_APPLICATION_DATA_VALUE* old_value = NULL;
	BACNET_PROPERTY_REFERENCE *old_rpm_property = NULL;

	m_oleSendCmdTime = COleDateTime::GetCurrentTime();

	
	//strLog.Format(_T("INFO: Bacnet OnReceiveReadMultiple InvokeId:%d\r\n"),invokeId);
	//PrintLog(strLog.GetString(), LOG_INFO);

	while (pRpmData)
	{
		DataPointEntry* saveData = NULL;
		saveData = FindDataByDeviceIDTypeInvokedID(invokeId,pRpmData->object_type,pRpmData->object_instance);
		if(saveData == NULL)
		{
			strLog.Format(_T("ERROR: Bacnet FindPoint Fail(Invoke:%d,Type:%d,Instance:%u). \r\n"),invokeId,pRpmData->object_type,pRpmData->object_instance);
			PrintLog(strLog.GetString(), LOG_ERROR);
		}
		else
		{
			m_nUpdatePointCount++;
			saveData->SetUpdated();
			m_mapDeviceResponse[saveData->GetParam(1)] = 0;
		}
		
		rpm_property = pRpmData->listOfProperties;
		m_nResponseCount++;
		while (rpm_property) 
		{
			dataValue = rpm_property->value;
			CopyValue(dataValue, saveData);
			rpm_property = rpm_property->next;
		}
		pRpmData = pRpmData->next;
	}

	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	return true;
}
//
////////////////////////////////////////////////////////////////////////////
void CBacnetCtrl::CopyValue(BACNET_APPLICATION_DATA_VALUE* pDataValue, DataPointEntry* saveData)
{
	// ASSERT(pDataValue);
	if(!pDataValue) 
		return;
	if (!saveData)
		return;
	saveData->SetPointTag_Bacnet(pDataValue->tag);
	switch(pDataValue->tag){
	case BACNET_APPLICATION_TAG_BOOLEAN:
		saveData->SetValue(pDataValue->type.Boolean);
		break;
	case BACNET_APPLICATION_TAG_UNSIGNED_INT:
		saveData->SetValue(pDataValue->type.Unsigned_Int);
		break;
	case BACNET_APPLICATION_TAG_SIGNED_INT:
		saveData->SetValue(pDataValue->type.Signed_Int);
		break;
	case BACNET_APPLICATION_TAG_REAL:
		saveData->SetValue(pDataValue->type.Real);
		break;
	case BACNET_APPLICATION_TAG_DOUBLE:
		saveData->SetValue(pDataValue->type.Double);
		break;
	case BACNET_APPLICATION_TAG_ENUMERATED:
		saveData->SetValue(pDataValue->type.Enumerated);
		break;
	default:
		saveData->SetValue(0);
		ASSERT(false);
		break;
	}
}

void CBacnetCtrl::FillWriteBuffer( BACNET_APPLICATION_DATA_VALUE* stValue, double dValue, UINT tag)
{
	stValue->tag = tag;
	switch(tag){
	case BACNET_APPLICATION_TAG_BOOLEAN:
		stValue->type.Boolean =  (dValue>0.1);
		break;
	case BACNET_APPLICATION_TAG_UNSIGNED_INT:
		stValue->type.Unsigned_Int = (UINT) dValue;
		break;
	case BACNET_APPLICATION_TAG_SIGNED_INT:
		stValue->type.Signed_Int = (int)dValue;
		break;
	case BACNET_APPLICATION_TAG_REAL:
		stValue->type.Real = (float)dValue;
		break;
	case BACNET_APPLICATION_TAG_DOUBLE:
		stValue->type.Double = dValue;
		break;
	case BACNET_APPLICATION_TAG_ENUMERATED:
		stValue->type.Enumerated = (uint32_t)dValue;
		break;
	default:
		ASSERT(false);
		break;
	}
}
//////////////////////////////////////////////////////////////////////////

void CBacnetCtrl::HandleWritePropertyAck(
	BACNET_ADDRESS * src,
	uint8_t invoke_id)
{
	//do nothing
}


bool CBacnetCtrl::SendWriteCmdBacnet(DWORD deviceObjInstance   //
	, WORD objType		//
	, DWORD objInstance			//
	, WORD objProperty
	, UINT8 tagType
	, double dValue
	,DWORD pointIndex
	,wstring pointName
	)
{
	bool bOK = true;
	bool ifFound = false;
	const UINT8 objPropertyPriority = m_nWritePriority;  //0 if not set, 1..16 if set
	const INT32 target_Object_Property_Index = -1; //BACNET_ARRAY_ALL;
	UINT max_apdu = 0;
	BACNET_ADDRESS  bacAddr;

	BACNET_APPLICATION_DATA_VALUE  *stValue = new BACNET_APPLICATION_DATA_VALUE;
	memset(stValue, 0, sizeof(BACNET_APPLICATION_DATA_VALUE) );

	stValue->next = NULL;
	const BACNET_APPLICATION_TAG property_tag = (BACNET_APPLICATION_TAG)tagType; //BACNET_APPLICATION_TAG_REAL

	//switch tag type, to set value
	FillWriteBuffer(stValue, dValue, property_tag);

	EnterCriticalSection(&m_criticalSection); //等待进入临界区，进入后加锁使其他线程不能进入


	ifFound = address_bind_request(deviceObjInstance, &max_apdu,  &bacAddr);
	DataPointEntry* saveData = FindDataFromDeviceTypeName(pointIndex,pointName,objType);
	if(!ifFound)
	{
		bOK = false;
		if(saveData)
		{
			CDebugLog::GetInstance()->SetErrPoint(*saveData,ERROR_CUSTOM_BACNET_ADDRESS_BIND_WRITE);
		}		
	}

	if(bOK)
	{
		const byte requestInvokeID = Send_Write_Property_Request(deviceObjInstance,
			(BACNET_OBJECT_TYPE)objType,
			objInstance,
			(BACNET_PROPERTY_ID)objProperty,
			stValue,
			objPropertyPriority,
			target_Object_Property_Index);
		if(saveData)
			saveData->SetBacnetInvokeID(requestInvokeID);
		Sleep(10);
	}

	LeaveCriticalSection(&m_criticalSection); 
	SAFE_DELETE(stValue);
	return bOK;
}



void CBacnetCtrl::SetPointList( const vector<DataPointEntry>& entrylist )
{
	m_nPointCount = entrylist.size();
	m_pointlistAI.clear();
	m_pointlistAO.clear();
	m_pointlistBI.clear();
	m_pointlistBO.clear();
	m_pointlistAV.clear();
	m_pointlistBV.clear();
	m_pointlistMI.clear();
	m_pointlistMV.clear();
	m_pointlistMO.clear();

	if(entrylist.size()>0)			//重新定义m_nReadMode
	{
		wstring strDeviceID = entrylist[0].GetParam(1);
		if (!strDeviceID.empty())
		{
			vector<wstring> vecParam;
			Project::Tools::SplitStringByChar(strDeviceID.c_str(),L'/',vecParam);
			if(vecParam.size() == 1)			//IP或者ID
			{
				m_nReadMode = 0;
				CString strParam = vecParam[0].c_str();
				if(Project::Tools::IsValidIP(Project::Tools::WideCharToAnsi(strParam).c_str()))
				{
					m_nReadMode = 1;
				}
			}
			else if(vecParam.size() == 2)			//IP/ID
			{
				m_nReadMode = 2;
			}
		}
	}

	m_vecDeviceID.clear();
	for(int i=0;i<entrylist.size();i++)
	{
		m_mapDeviceResponse[entrylist[i].GetParam(1)] = 1;			//标记成未收到回复

		if(entrylist[i].GetParam(2) ==L"AI")
			m_pointlistAI.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"AO")
			m_pointlistAO.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"BI")
			m_pointlistBI.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"BO")
			m_pointlistBO.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"AV")
			m_pointlistAV.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"BV")
			m_pointlistBV.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"MI")
			m_pointlistMI.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"MV")
			m_pointlistMV.push_back(entrylist[i]);
		else if(entrylist[i].GetParam(2) ==L"MO")
			m_pointlistMO.push_back(entrylist[i]);
		else
		{
			CString strLog;
			strLog.Format(_T("ERROR: bacnet type not in AI/AO/BI/BO/AV/BV/MI/MO/MV   -> %s \r\n"),entrylist[i].GetShortName().c_str());
			PrintLog(strLog.GetString(), LOG_ERROR);

			CDebugLog::GetInstance()->SetErrPoint(entrylist[i],ERROR_CUSTOM_BACNET_TYPE_INVALID);
		}
		bool bDeviceIExist = false;
		DWORD dwDeviceID = GetBacnetDeviceID(entrylist[i].GetParam(1));
		for(int j=0; j<m_vecDeviceID.size(); ++j)
		{
			if(m_vecDeviceID[j] == dwDeviceID)
			{
				bDeviceIExist = true;
				break;
			}
		}
		if(!bDeviceIExist)
		{
			m_vecDeviceID.push_back(dwDeviceID);
		}
	}
}

void  CBacnetCtrl::ClearReadAccessData(BACNET_READ_ACCESS_DATA** ppRpmData)
{
	if(!ppRpmData) 
		return;

	BACNET_READ_ACCESS_DATA *pRpmData = *ppRpmData;
	BACNET_READ_ACCESS_DATA *old_rpm_object = NULL;
	BACNET_PROPERTY_REFERENCE *rpm_property = NULL;
	BACNET_PROPERTY_REFERENCE *old_rpm_property = NULL;
	BACNET_APPLICATION_DATA_VALUE *value = NULL;
	BACNET_APPLICATION_DATA_VALUE *old_value = NULL;
	while (pRpmData) 
	{
		rpm_property = pRpmData->listOfProperties;
		while (rpm_property)
		{
			value = rpm_property->value;			//释放value
			while (value) 
			{
				old_value = value;
				value = value->next;
				SAFE_DELETE(old_value);
			}
			old_rpm_property = rpm_property;
			rpm_property = rpm_property->next;
			SAFE_DELETE(old_rpm_property);
		}
		old_rpm_object = pRpmData;
		pRpmData = pRpmData->next;
		SAFE_DELETE(old_rpm_object);
	}
	*ppRpmData = NULL;
}

UINT WINAPI CBacnetCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CBacnetCtrl* pthis = (CBacnetCtrl*)lparam;
	if (!pthis){
		return 0;
	}

	while (!pthis->GetSendCommandThreadExit())
	{
		pthis->SendReadCommandsByActive();
		int nSleep = pthis->m_nReadInterval;
		while(!pthis->GetSendCommandThreadExit())
		{
			if(nSleep <= 0)
			{
				break;
			}
			nSleep--;
			Sleep(1000);
		}
	}

	return 0;
}

bool CBacnetCtrl::GetSendCommandThreadExit() const
{
	return m_exit_sendcommand_thead;
}

bool CBacnetCtrl::GetReadDataThreadExit() const
{
	return m_exit_readdata_thread;
}

void CBacnetCtrl::SendReadCommands()
{
	m_nCmdCount = 0;
	m_nUpdatePointCount = 0;
	int nInvolk = 0;
	bool bDeviceNotBind = false;
	for(int i=0; i<m_vecAI.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecAI[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading AI.\r\n", LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_AI);
			bDeviceNotBind = true;
		}	
	}
	if(m_vecAI.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecAO.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecAO[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading AO.\r\n", LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_AO);
			bDeviceNotBind = true;
		}
	}
	if(m_vecAO.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecBI.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecBI[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading BI.\r\n", LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_BI);
			bDeviceNotBind = true;
		}
	}
	if(m_vecBI.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecBO.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecBO[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading BO.\r\n", LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_BO);
			bDeviceNotBind = true;
		}
	}
	if(m_vecBO.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecAV.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecAV[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading AV.\r\n",LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_AV);
			bDeviceNotBind = true;
		}
	}
	if(m_vecAV.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecBV.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecBV[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading BV.\r\n",LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_BV);
			bDeviceNotBind = true;
		}
	}
	if(m_vecBV.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecMI.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecMI[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading MI.\r\n",LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_MI);
			bDeviceNotBind = true;
		}
	}
	if(m_vecMI.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecMO.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecMO[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading MO.\r\n",LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_MO);
			bDeviceNotBind = true;
		}
	}
	if(m_vecMO.size()>0)
		Sleep(m_nReadTypeInterval);

	for(int i=0; i<m_vecMV.size(); ++i)
	{
		if(!SendCommandByGroup(m_nReadLimit, m_vecMV[i], nInvolk))
		{
			PrintLog(L"ERROR: address_bind_request error in reading MV.\r\n",LOG_ERROR);
			CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_BACNET_ADDRESS_BIND_MV);
			bDeviceNotBind = true;
		}
	}
	if(m_vecMV.size()>0)
		Sleep(m_nReadTypeInterval);

	if(bDeviceNotBind)
	{
		OnTimerWhois_();
	}
}

bool CBacnetCtrl::SendReadCommand( DataPointEntry& entry )
{
	UINT max_apdu = 0;
	BACNET_ADDRESS  targetAddr;
	DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
	const int objectIndex = BACNET_ARRAY_ALL;
	const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();

	bool ifFound = address_bind_request(deviceObjInstance, &max_apdu,  &targetAddr);

	if (ifFound) 
	{         
		EnterCriticalSection(&m_criticalSection); 

		const UINT  requestInvokeId = Send_Read_Property_Request(deviceObjInstance,
			objType,
			entry.GetPointAddress_Bacnet(), 
			PROP_PRESENT_VALUE, //(BACNET_PROPERTY_ID)bacnetReadCmd.mObjProperty,      
			objectIndex);
		entry.SetBacnetInvokeID(requestInvokeId);

		hash_map<wstring,DataPointEntry*> mapQueryPointInvolk;
		mapQueryPointInvolk[entry.GetShortName()] = &entry;
		m_mapQueryPointInvolk[requestInvokeId] = mapQueryPointInvolk;

		LeaveCriticalSection(&m_criticalSection);  //离开 开锁
		return true;
	}
	return false;
}

void CBacnetCtrl::GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist )
{
	if(m_oleStartTime == m_oleUpdateTime)
		return;

	int i = 0;
	bool bHasPoint = false;
	for (i = 0; i < m_pointlistAI.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistAI[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	for (i = 0; i < m_pointlistAO.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistAO[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	for (i = 0; i < m_pointlistBI.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistBI[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	for (i = 0; i < m_pointlistBO.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistBO[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	for (i = 0; i < m_pointlistAV.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistAV[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	for (i = 0; i < m_pointlistBV.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistBV[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	//
	for (i = 0; i < m_pointlistMI.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistMI[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	for (i = 0; i < m_pointlistMO.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistMO[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	for (i = 0; i < m_pointlistMV.size(); i++)
	{
		DataPointEntry&  bacnetentry = m_pointlistMV[i];
		if(bacnetentry.GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(bacnetentry.GetMultipleReadValue( bacnetentry.GetValue()),m_nPrecision);
			valuelist.push_back(make_pair(bacnetentry.GetShortName(),strValueSet));
			bHasPoint = true;
		}
	}

	if(bHasPoint)
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		valuelist.push_back(make_pair(TIME_UPDATE_BACNET, strUpdateTime));
		valuelist.push_back(make_pair(LOG_BACNET, Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str())));
	}
}

void CBacnetCtrl::InitDataEntryValue(const wstring& pointname, double fvalue)
{
	int i = 0;
	for (i = 0; i < m_pointlistAI.size(); i++)
	{
		DataPointEntry& entry = m_pointlistAI[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}

	for (i = 0; i < m_pointlistAO.size(); i++)
	{
		DataPointEntry& entry = m_pointlistAO[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}


	for (i = 0; i < m_pointlistBI.size(); i++)
	{
		DataPointEntry& entry = m_pointlistBI[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}

	for (i = 0; i < m_pointlistBO.size(); i++)
	{
		DataPointEntry& entry = m_pointlistBO[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}

	for (i = 0; i < m_pointlistAV.size(); i++)
	{
		DataPointEntry& entry = m_pointlistAV[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}

	for (i = 0; i < m_pointlistBV.size(); i++)
	{
		DataPointEntry& entry = m_pointlistBV[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}

	//
	for (i = 0; i < m_pointlistMI.size(); i++)
	{
		DataPointEntry& entry = m_pointlistMI[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}

	for (i = 0; i < m_pointlistMO.size(); i++)
	{
		DataPointEntry& entry = m_pointlistMO[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}

	for (i = 0; i < m_pointlistMV.size(); i++)
	{
		DataPointEntry& entry = m_pointlistMV[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			entry.SetValue(fvalue);
			return;
		}
	}
}

void CBacnetCtrl::SetValue( const wstring& pointname, double value )
{
	int i = 0;
	for (i = 0; i < m_pointlistAI.size(); i++)
	{
		DataPointEntry& entry = m_pointlistAI[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const WORD objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);

			SendReadCommand(entry); //写完立即读更新
			
			return;
		}
	}

	for (i = 0; i < m_pointlistAO.size(); i++)
	{
		DataPointEntry& entry = m_pointlistAO[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const WORD objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}

	for (i = 0; i < m_pointlistBI.size(); i++)
	{
		DataPointEntry& entry = m_pointlistBI[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const UINT objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}

	for (i = 0; i < m_pointlistBO.size(); i++)
	{
		DataPointEntry& entry = m_pointlistBO[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const UINT objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}

	for (i = 0; i < m_pointlistAV.size(); i++)
	{
		DataPointEntry& entry = m_pointlistAV[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const UINT objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}

	for (i = 0; i < m_pointlistBV.size(); i++)
	{
		DataPointEntry& entry = m_pointlistBV[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const UINT objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}

	//
	for (i = 0; i < m_pointlistMI.size(); i++)
	{
		DataPointEntry& entry = m_pointlistMI[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const UINT objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}

	for (i = 0; i < m_pointlistMO.size(); i++)
	{
		DataPointEntry& entry = m_pointlistMO[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const UINT objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}

	for (i = 0; i < m_pointlistMV.size(); i++)
	{
		DataPointEntry& entry = m_pointlistMV[i];
		if (entry.GetShortName() == pointname)
		{
			const DWORD deviceObjInstance = GetBacnetDeviceID(entry.GetParam(1));
			const BACNET_OBJECT_TYPE objType = (BACNET_OBJECT_TYPE)entry.GetPointType_Bacnet();
			const DWORD objInstance = entry.GetPointAddress_Bacnet();
			const UINT objProperty = PROP_PRESENT_VALUE;
			const UINT8 tagType = entry.GetPointTag_Bacnet();

			SendWriteCmdBacnet(deviceObjInstance, objType, objInstance, objProperty, tagType, entry.GetMultipleReadValue(value,false), entry.GetPointIndex(), entry.GetShortName() );

			Sleep(20);
			SendReadCommand(entry); //写完立即读更新

			return;
		}
	}


}

void CBacnetCtrl::Exit()
{
	m_exit_readdata_thread = true;
	m_exit_sendcommand_thead = true;

	WaitForSingleObject(m_readdata_thread, INFINITE);
	if(m_active_thread)
	{
		WaitForSingleObject(m_active_thread,INFINITE);
	}

	if(m_hsendcommand_thread)
	{
		SetEvent(m_sendCmdEvent);
		WaitForSingleObject(m_hsendcommand_thread, INFINITE);
	}
	m_mapDeviceID.clear();
	m_mapPointInvolk.clear();
	m_mapQueryPointInvolk.clear();
	m_logsession =NULL;
	datalink_cleanup();

	//m_vecpointlist.clear();
	m_pointlistAI.clear();
	m_pointlistAO.clear();
	m_pointlistBI.clear();
	m_pointlistBO.clear();
	m_pointlistAV.clear();
	m_pointlistBV.clear();
	m_pointlistMI.clear();
	m_pointlistMV.clear();
	m_pointlistMO.clear();
	m_vecAI.clear();
	m_vecAO.clear();
	m_vecBI.clear();
	m_vecBO.clear();
	m_vecAV.clear();
	m_vecBV.clear();
	m_vecMI.clear();
	m_vecMO.clear();
	m_vecMV.clear();
	m_mapDeviceID.clear();
	m_mapPointInvolk.clear();
	m_mapQueryPointInvolk.clear();
}

bool CBacnetCtrl::GetValue( const wstring& pointname, double& result )
{
	int i = 0;
	for (i = 0; i < m_pointlistAI.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistAI[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	for (i = 0; i < m_pointlistAO.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistAO[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	for (i = 0; i < m_pointlistBI.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistBI[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	for (i = 0; i < m_pointlistBO.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistBO[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	for (i = 0; i < m_pointlistAV.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistAV[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	for (i = 0; i < m_pointlistBV.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistBV[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	//
	for (i = 0; i < m_pointlistMI.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistMI[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	for (i = 0; i < m_pointlistMO.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistMO[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	for (i = 0; i < m_pointlistMV.size(); i++)
	{
		const DataPointEntry& entry = m_pointlistMV[i];
		if (entry.GetShortName() == pointname){
			Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock);
			result = entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}
	return false;
}

void CBacnetCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CBacnetCtrl::SendCommandByGroup( int groupnum , vector<DataPointEntry> &entryList, int& nInvolkStartFromNum)
{
	bool bNoError = true;
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return bNoError;
	}
	if (groupnum <= 0){
		return bNoError;
	}

	unsigned int pointsize = entryList.size();
	if (pointsize == 0){
		return bNoError;
	}

	int groupcount = pointsize/groupnum;
	if (groupnum*groupcount < pointsize){
		groupcount += 1;
	}

	for (unsigned int i = 0; i < groupcount; i++)
	{
		DWORD deviceObjInstance = 0;
		UINT8 buffer[MAX_PDU] = {0};
		BACNET_READ_ACCESS_DATA *pReadAccessData = new BACNET_READ_ACCESS_DATA;
		if(!pReadAccessData) 
			return bNoError;
		BACNET_READ_ACCESS_DATA* pRpmData = pReadAccessData;

		bool bFirst = true;	
		for (unsigned int j = groupnum *i; j < groupnum*(i+1) && j < pointsize; j++)
		{
//			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock); //发包前加锁

			pRpmData->object_type = (BACNET_OBJECT_TYPE) entryList[j].GetPointType_Bacnet();	//point type
			deviceObjInstance = GetBacnetDeviceID(entryList[j].GetParam(1));	//server id
			pRpmData->object_instance = entryList[j].GetPointAddress_Bacnet(); //address
			
			pRpmData->listOfProperties = new BACNET_PROPERTY_REFERENCE;
			pRpmData->listOfProperties->propertyIdentifier = PROP_PRESENT_VALUE;
			pRpmData->listOfProperties->propertyArrayIndex = BACNET_ARRAY_ALL;
			m_nCmdCount++;
			pRpmData->next =NULL;
			if ( j < (groupnum*(i+1)-1) && j < (pointsize-1)){
				pRpmData->next = new BACNET_READ_ACCESS_DATA;
				pRpmData = pRpmData->next ;
			}
		}
		pRpmData->next = NULL;
		EnterCriticalSection(&m_criticalSection); //等待进入临界区，进入后加锁使其他线程不能进入


		UINT max_apdu = 0;
		BACNET_ADDRESS  targetAddr;
		const bool ifFound = address_bind_request(deviceObjInstance, &max_apdu,  &targetAddr);
		if(!ifFound)
		{
			bNoError = false;
			CString strLog;
			strLog.Format(_T("ERROR: address_bind_request:%u failed.\r\n"),deviceObjInstance);
			PrintLog(strLog.GetString(), LOG_ERROR);

			//
			CDebugLog::GetInstance()->SetErrPoints(entryList,ERROR_CUSTOM_BACNET_ADDRESS_BIND_READ);

			for (unsigned int k = groupnum *i; k < groupnum*(i+1) && k < pointsize; k++)
			{
				const DataPointEntry entry = entryList[k];
				DataPointEntry* saveData = FindDataFromDeviceIDTypeAddress(deviceObjInstance,(BACNET_OBJECT_TYPE) entry.GetPointType_Bacnet(),entry.GetPointAddress_Bacnet());
				if(saveData)
				{
					saveData->SetToUpdate();
				}
			}
		}
		else
		{
			m_nBacnetBindOK = 1;
			const UINT8 reqInvokeId = Send_Read_Property_Multiple_Request(&buffer[0], sizeof(buffer), 
				deviceObjInstance,
				pReadAccessData);
			if(m_nCmdCount >= m_nPointCount)
				m_nLastInvolk = reqInvokeId;
			if(reqInvokeId == 0)
			{
				PrintLog(L"ERROR: Send_Read_Property_Multiple_Request:reqInvokeId = 0.\r\n", LOG_ERROR);
				CDebugLog::GetInstance()->SetErrPoints(entryList,ERROR_CUSTOM_BACNET_READ_MUTIL);
				m_nReqInvokeIdErrCount++;

				if(m_nReqInvokeIdErrCount >= 5)
				{
					m_nReqInvokeIdErrCount = 0;
					FreeAllInvokeId();
				}
			}
			else
			{
				m_nBacnetSendOK = 1;
			}

			if(reqInvokeId >= MAX_TSM_TRANSACTIONS)			//最大开始清除之前所有点位的reqInvokeId
			{
				hash_map<wstring,DataPointEntry*>::iterator itertInvolk = m_mapPointInvolk.begin();
				while(itertInvolk != m_mapPointInvolk.end())
				{
					if((*itertInvolk).second->GetBacnetInvokeID() <= MAX_TSM_TRANSACTIONS-10)		//顺便移除
					{
						(*itertInvolk).second->SetBacnetInvokeID(0);
						itertInvolk = m_mapPointInvolk.erase(itertInvolk);
					}
					else
					{
						itertInvolk++;
					}
				}
				Sleep(20);
			}

			for (unsigned int k = groupnum *i; k < groupnum*(i+1) && k < pointsize; k++)
			{
				const DataPointEntry entry = entryList[k];
				DataPointEntry* saveData = FindDataFromDeviceIDTypeAddress(deviceObjInstance,(BACNET_OBJECT_TYPE) entry.GetPointType_Bacnet(),entry.GetPointAddress_Bacnet());
				if(saveData)
				{
					saveData->SetBacnetInvokeID(reqInvokeId);
					saveData->SetToUpdate();
					m_mapPointInvolk[saveData->GetShortName()] = saveData;
				}
			}
		}
		ClearReadAccessData(&pReadAccessData);
		LeaveCriticalSection(&m_criticalSection);  //离开 开锁

		//Sleep(20); //sleep after network send.

		//ClearReadAccessData(&pReadAccessData);

		//Sleep(50);
		Sleep(m_nReadCmdInterval);
	}
	return bNoError;

}

void CBacnetCtrl::AddLog( const wchar_t* loginfo )
{
	if (m_logsession)
	{
		m_logsession->InsertLog(loginfo);
	}

	_tprintf(loginfo);
}

vector<vector<DataPointEntry>> CBacnetCtrl::SortPointByDeviceID( vector<DWORD> vecDeviceID,vector<DataPointEntry> pointlist )
{
	vector<vector<DataPointEntry>> vecDevicePointList;
	if(vecDeviceID.size() == 0 || pointlist.size() == 0)
		return vecDevicePointList;

	for(int i=0; i<vecDeviceID.size(); ++i)
	{
		//将点按设备ID再划分一次
		vector<DataPointEntry> vecPoint;
		for(int j=0; j<pointlist.size(); ++j)
		{
			if(GetBacnetDeviceID(pointlist[j].GetParam(1)) == vecDeviceID[i])
				vecPoint.push_back(pointlist[j]);
		}
		if(vecPoint.size()>0)
			vecDevicePointList.push_back(vecPoint);
	}
	return vecDevicePointList;
}

vector<vector<DataPointEntry>> CBacnetCtrl::SortPointByDeviceID( vector<DWORD> vecDeviceID,vector<DataPointEntry> pointlist,int nGroupMax )
{
	vector<vector<DataPointEntry>> vecDevicePointList;
	if(vecDeviceID.size() == 0 || pointlist.size() == 0)
		return vecDevicePointList;

	for(int i=0; i<vecDeviceID.size(); ++i)
	{
		//将点按设备ID再划分一次
		vector<DataPointEntry> vecPoint;
		for(int j=0; j<pointlist.size(); ++j)
		{
			if(GetBacnetDeviceID(pointlist[j].GetParam(1)) == vecDeviceID[i])
				vecPoint.push_back(pointlist[j]);
		}

		//将vecPoint点按nGroupMax上限再划分不同vector
		int nPointSize = vecPoint.size();
		int nGroupCount = nPointSize/nGroupMax;
		if (nGroupMax*nGroupCount < nPointSize)
		{
			nGroupCount += 1;
		}

		for (unsigned int k = 0; k < nGroupCount; k++)
		{
			vector<DataPointEntry> vecPoint1;
			for (unsigned int l = nGroupMax *k; l < nGroupMax*(k+1) && l< nPointSize; l++)
			{
				vecPoint1.push_back(vecPoint[l]);
			}
			vecDevicePointList.push_back(vecPoint1);
		}
	}
	return vecDevicePointList;
}

bool CBacnetCtrl::IsMacInitOK()
{
	CToolsFunction toolsFunction;
	IP_ADAPTER_INFO* pIpAdapterInfo = toolsFunction.GetAllLocalMachineEthInfo();
	if(pIpAdapterInfo != NULL)
	{
		while (pIpAdapterInfo)
		{
			// Loop for multi-ip address in a ethernet card.
			IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
			do 
			{
				if (*(pIpAddrString->IpAddress.String) != '\0' && strcmp(pIpAddrString->IpAddress.String, "0.0.0.0") != 0)
				{
					return true;
				}
				else
				{
					pIpAddrString = pIpAddrString->Next;
				}

			} while (pIpAddrString);

			pIpAdapterInfo = pIpAdapterInfo->Next;
		}
		return false;
	}
	return false;
}

void CBacnetCtrl::SaveExecuteOrder(string nOrder )
{
	GetLocalTime(&m_sExecuteTime);
	if(m_strExecuteLog.length()>=900)
		m_strExecuteLog = "";
	std::ostringstream sqlstream;
	sqlstream << nOrder << ",";
	m_strExecuteLog += sqlstream.str();
}

string CBacnetCtrl::GetExecuteOrderLog()
{
	if(m_strExecuteLog.length()<=0)
		return "";

	string strOut;
	strOut += Project::Tools::SystemTimeToString(m_sExecuteTime);
	strOut += "(Bacnet)::";
	strOut += m_strExecuteLog;

	m_strExecuteLog = "";
	return strOut;
}

bool CBacnetCtrl::FreeAllInvokeId()
{
	tsm_free_all_invoke_id();
	return true;
}

void CBacnetCtrl::OutDebugInfo( const wstring strOut, int nLogLevel )
{
	if(nLogLevel> m_nLogLevel)
		return;

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		SYSTEMTIME sNow;
		GetLocalTime(&sNow);
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		CString strLog,strLogPath;
		strLog.Format(_T("%s::%s"),strTime.c_str(),strOut.c_str());
		strLogPath.Format(_T("%s\\bacnet_debug_%d_%02d_%02d.log"),strPath.c_str(),sNow.wYear,sNow.wMonth,sNow.wDay);

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
		}
	}
}

void CBacnetCtrl::SetDebug( int nDebug )
{
	m_nOutPut = nDebug;
}

void CBacnetCtrl::OnTimerWhois_()
{
	COleDateTime oleCurrent = COleDateTime::GetCurrentTime();
	if(m_oleSendWhoISTime.GetHour() != oleCurrent.GetHour())
	{
		m_nSendWhiIsCount = 0;
	}

	if(m_nSendWhiIsCount < MAX_WHOIS_PER_HOUR)
	{
		Send_WhoIs(-1, -1);
		m_nSendWhiIsCount++;
		m_oleSendWhoISTime = COleDateTime::GetCurrentTime();
	}
}

void CBacnetCtrl::GetIPInfo( hash_map<wstring,wstring>& mapIP )
{
	struct in_addr mask,address,broadaddress;
	mask.s_addr = bip_get_addr_mask();
	address.s_addr = bip_get_addr();
	broadaddress.s_addr = bip_get_broadcast_addr();
	CString strMask,strAddess,strBroadAddress;
	strMask.Format(_T("%s"),Project::Tools::AnsiToWideChar(inet_ntoa(mask)).c_str());
	strAddess.Format(_T("%s"),Project::Tools::AnsiToWideChar(inet_ntoa(address)).c_str());
	strBroadAddress.Format(_T("%s"),Project::Tools::AnsiToWideChar(inet_ntoa(broadaddress)).c_str());

	mapIP[L"BacnetMask"] = strMask.GetString();
	mapIP[L"BacnetIP"] = strAddess.GetString();
	mapIP[L"BacnetBroadIP"] = strBroadAddress.GetString();
	if(m_nBacnetBindOK == 0)
	{
		mapIP[L"BacnetFind"] = L"0";
	}
	else
	{
		mapIP[L"BacnetFind"] = L"1";
	}
	if(m_nBacnetSendOK == 0)
	{
		mapIP[L"BacnetSend"] = L"0";
	}
	else
	{
		mapIP[L"BacnetSend"] = L"1";
	}
	if(m_nBacnetResponseOK == 0)
	{
		mapIP[L"BacnetResponse"] = L"0";
	}
	else
	{
		mapIP[L"BacnetResponse"] = L"1";
	}
	//获取收到的所有BacnetID
	unsigned count = address_count();
	count = (count>20)?20:count;
	BACNET_ADDRESS bacnetaddress;
	uint32_t device_id = 0;
	unsigned max_apdu = 0;
	UINT port = 0;
	CString cstr;
	std::ostringstream sqlstream_device;
	for(int i=0; i<count; ++i)
	{
		if (address_get_by_index(i, &device_id, &max_apdu, &bacnetaddress)) 
		{
			if(bacnetaddress.mac_len > 5)
			{
				sqlstream_device << device_id << "|";
			}
		}
	}
	mapIP[L"BacnetDevice"] = Project::Tools::AnsiToWideChar(sqlstream_device.str().c_str());

	//查询网卡
	std::ostringstream sqlstream;
	CToolsFunction toolsFunction;
	IP_ADAPTER_INFO* pIpAdapterInfo = toolsFunction.GetAllLocalMachineEthInfo();
	if(pIpAdapterInfo != NULL)
	{
		while (pIpAdapterInfo)
		{
			// Loop for multi-ip address in a ethernet card.
			IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
			do 
			{
				if (*(pIpAddrString->IpAddress.String) != '\0' && strcmp(pIpAddrString->IpAddress.String, "0.0.0.0") != 0)
				{
					sqlstream << pIpAddrString->IpAddress.String << "|";
				}
				pIpAddrString = pIpAddrString->Next;

			} while (pIpAddrString);

			pIpAdapterInfo = pIpAdapterInfo->Next;
		}
	}
	mapIP[L"EthIP"] = Project::Tools::AnsiToWideChar(sqlstream.str().c_str());
}

void CBacnetCtrl::GetReceiveBacnetID( hash_map<UINT,wstring>& mapBacnetDevice )
{
	unsigned count = address_count();
	BACNET_ADDRESS address;
	uint32_t device_id = 0;
	unsigned max_apdu = 0;
	UINT port = 0;
	CString cstr;
	for(int i=0; i<count; ++i)
	{
		if (address_get_by_index(i, &device_id, &max_apdu, &address)) 
		{
			if(address.mac_len > 5){
				port = address.mac[4] + address.mac[5] * 0x100; 
				cstr.Format(_T("%d.%d.%d.%d:%d"),address.mac[0], address.mac[1],address.mac[2], address.mac[3], port );
				mapBacnetDevice[device_id] = cstr.GetString();
			}
	
		}
	}
}

bool CBacnetCtrl::GetProcNameByPort( int nPort, string &strResult )
{
	bool bSuc = false;
	try
	{
		char pszPort[16] = {0};
		itoa(nPort, pszPort, 10);
		char pResult[256] = {0};

		std::ostringstream sqlstream_port,sqlstream_process;
		wstring strPath;
		strPath = L"C:\\rnbtest";
		if(Project::Tools::FindOrCreateFolder(strPath))
		{
			string strSysPath,strPath_Port,strPath_Process;
			strSysPath = Project::Tools::WideCharToAnsi(strPath.c_str());
			sqlstream_port << strSysPath << "\\bacnetport";
			sqlstream_process << strSysPath << "\\bacnetprocess";
			strPath_Port = sqlstream_port.str();
			strPath_Process = sqlstream_process.str();
			const char* pPortFilePath = strPath_Port.c_str(); 
			const char* pProcessFilePath = strPath_Process.c_str();
			//sprintf(pResult, "cmd /c netstat -ano|findstr :%d  > %s", nPort, pPortFilePath);

			//创建线程  
			CString paramstr;  
			paramstr.Format(_T("/c netstat -ano|findstr :%d  > %s"), nPort, Project::Tools::AnsiToWideChar(pPortFilePath).c_str());
			::ShellExecuteA(NULL, NULL,"cmd.exe",Project::Tools::WideCharToAnsi(paramstr).c_str(), NULL,SW_HIDE);
			Sleep(1000);
			//查找端口号
			FILE *pPortFile = fopen(pPortFilePath, "r");
			if ( pPortFile )
			{
				while ( !feof(pPortFile) )
				{
					memset(pResult, 0, sizeof(pResult));
					fread(pResult, sizeof(pResult), 1, pPortFile);
					pResult[sizeof(pResult)-1] = 0x00;
					string strPortTmp = pResult;
					int ffset = (int)strPortTmp.find_last_of(0x0A);
					if ( ffset > -1 )
					{
						pResult[ffset] = 0x00;
						strPortTmp = strPortTmp.substr(0, ffset);
						if ( !feof(pPortFile) )
						{
							fseek(pPortFile, (long)(strPortTmp.length()+1-sizeof(pResult)), SEEK_CUR);
						}
						ffset = (int)strPortTmp.find_first_of(':');
						if ( ffset > -1 )
						{
							strPortTmp = strPortTmp.substr(ffset+1, 6);
							ffset = (int)strPortTmp.find_last_not_of(' ');
							if ( ffset > -1 )
							{
								strPortTmp = strPortTmp.substr(0, ffset+1);
								if ( strPortTmp == pszPort )
								{
									strPortTmp = pResult;
									ffset = (int)strPortTmp.find_last_of(' ');
									if ( ffset > -1 )
									{
										strPortTmp = strPortTmp.substr(ffset+1);
										//sprintf(pResult, "cmd /c tasklist /fi \"pid eq %s\" /nh> %s", strPortTmp.c_str(), pProcessFilePath);
										//根据端口号查找进程ID
										paramstr.Format(_T("/c tasklist /fi \"pid eq %s\" /nh> %s"), Project::Tools::AnsiToWideChar(strPortTmp.c_str()).c_str(), Project::Tools::AnsiToWideChar(pProcessFilePath).c_str());
										::ShellExecuteA(NULL, NULL,"cmd.exe",Project::Tools::WideCharToAnsi(paramstr).c_str(), NULL,SW_HIDE);
										Sleep(1000);

										FILE *pProcessFile = fopen(pProcessFilePath, "r");
										if ( pProcessFile )
										{
											while (!feof(pProcessFile))
											{
												memset(pResult, 0, sizeof(pResult));
												fread(pResult, sizeof(pResult), 1, pProcessFile);
												pResult[sizeof(pResult)-1] = 0x00;
												string strProcessTmp = pResult;
												int offset = (int)strProcessTmp.find_last_of(0x0A);
												if ( offset > -1 )
												{
													pResult[offset] = 0x00;
													strProcessTmp = strProcessTmp.substr(0, offset);
													if ( !feof(pProcessFile) )
													{
														fseek(pProcessFile, (long)(strProcessTmp.length()+1-sizeof(pResult)), SEEK_CUR);
													}
													if ( 0x0A == pResult[0] )      //首行只有一个字符 0x0A
													{
														strProcessTmp = pResult+1;
													}
													else
													{
														strProcessTmp = pResult;
													}
													ffset = (int)strProcessTmp.find_first_of(' ');
													if ( ffset > -1 )
													{
														{
															{
																{
																	{
																		strProcessTmp = strProcessTmp.substr(0, ffset);
																		if ( "" != strProcessTmp )
																		{
																			//查找成功，结束
																			strResult +=  strProcessTmp;
																			bSuc = true;
																		}
																		continue;
																	}
																}
															}
														}
													}
												}
											}
											fclose(pProcessFile);
										}
										//sprintf(pResult, "cmd /c del %s", pProcessFilePath);
										paramstr.Format(_T("/c del %s"), Project::Tools::AnsiToWideChar(pProcessFilePath).c_str());
										::ShellExecuteA(NULL, NULL,"cmd.exe",Project::Tools::WideCharToAnsi(paramstr).c_str(), NULL,SW_HIDE);	
										if(bSuc)
										{
											continue;
										}
									}
								}
							}
						}
					}
				}
				fclose(pPortFile);
			}
			if(!bSuc)
			{ 
				strResult="";
			};
			//sprintf(pResult, "cmd /c del %s", pPortFilePath);
			paramstr.Format(_T("/c del %s"), Project::Tools::AnsiToWideChar(pPortFilePath).c_str());
			::ShellExecuteA(NULL, NULL,"cmd.exe",Project::Tools::WideCharToAnsi(paramstr).c_str(), NULL,SW_HIDE);
		}
		//GetSysPath(strPath);	
	}
	catch (...)
	{
		return bSuc;
	}
	return bSuc;
}

void CBacnetCtrl::MyIAMHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src)
{
	try
	{
		int len = 0;
		uint32_t device_id = 0;
		unsigned max_apdu = 0;
		int segmentation = 0;
		uint16_t vendor_id = 0;

		(void) service_len;
		len =
			iam_decode_service_request(service_request, &device_id, &max_apdu,
			&segmentation, &vendor_id);

		if(len>0)
		{
			int nReadLimit = (max_apdu-g_nBacnetReqHeaderLength)/g_nBacnetOneReqLength;
			m_nReadLimit = (nReadLimit < m_nReadLimit)?nReadLimit:m_nReadLimit;

			CString strKey = _T("");
			if(m_nReadMode == 0)
			{
				strKey.Format(_T("%u"),device_id);
				m_mapDeviceID[strKey.GetString()] = device_id;
			}
			else if(m_nReadMode == 1)
			{
				if(src)
				{
					strKey.Format(_T("%d.%d.%d.%d"),(int)src->mac[0],(int)src->mac[1], (int)src->mac[2], (int)src->mac[3]);
					device_id = inet_addr(Project::Tools::WideCharToAnsi(strKey).c_str());
					m_mapDeviceID[strKey.GetString()] = device_id;
				}
			}
			else if(m_nReadMode == 2)
			{
				if(src)
				{
					strKey.Format(_T("%d.%d.%d.%d/%u"),(int)src->mac[0],(int)src->mac[1], (int)src->mac[2], (int)src->mac[3],device_id);
					wstring wstrKey = strKey.GetString();
					hash_map<wstring,DWORD>::iterator iter = m_mapDeviceID.find(wstrKey);
					if(iter == m_mapDeviceID.end())
					{
						int nIndex = m_mapDeviceID.size()+1;
						m_mapDeviceID[wstrKey] = nIndex;
						device_id = nIndex;
					}
					else
					{
						device_id = (*iter).second;
					}
				}
			}

			/* only add address if requested to bind */
			address_add_binding(device_id, max_apdu, src);
		}
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}

DWORD CBacnetCtrl::GetBacnetDeviceID(const wstring strBacnetParam1)
{
	try
	{
		if(m_nReadMode == 0)				//ID
		{
			DWORD dwresult = 0;
			dwresult = _ttoi(strBacnetParam1.c_str());
			return dwresult;
		}
		else if(m_nReadMode == 1)			//IP
		{
			return inet_addr(Project::Tools::WideCharToAnsi(strBacnetParam1.c_str()).c_str());
		}
		else
		{
			DWORD dwDeviceID = 0;
			hash_map<wstring,DWORD>::iterator iter = m_mapDeviceID.find(strBacnetParam1);
			if(iter != m_mapDeviceID.end())
			{
				return (*iter).second;
			}
			else
			{
				int nIndex = m_mapDeviceID.size()+1;
				m_mapDeviceID[strBacnetParam1] = nIndex;
				dwDeviceID = nIndex;
			}
			return dwDeviceID;
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return 0;
}

void CBacnetCtrl::MyIAMAddHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src)
{
	try
	{
		int len = 0;
		uint32_t device_id = 0;
		unsigned max_apdu = 0;
		int segmentation = 0;
		uint16_t vendor_id = 0;

		(void) service_len;
		len =
			iam_decode_service_request(service_request, &device_id, &max_apdu,
			&segmentation, &vendor_id);
#if PRINT_ENABLED
		fprintf(stderr, "INFO : Received I-Am Request");
#endif
		if (len != -1) {
#if PRINT_ENABLED
			fprintf(stderr, " from %lu, MAC = %d.%d.%d.%d.%d.%d\n",
				(unsigned long) device_id, src->mac[0], src->mac[1], src->mac[2],
				src->mac[3], src->mac[4], src->mac[5]);
#endif

			int nReadLimit = (max_apdu-g_nBacnetReqHeaderLength)/g_nBacnetOneReqLength;
			m_nReadLimit = (nReadLimit < m_nReadLimit)?nReadLimit:m_nReadLimit;

			CString strKey = _T("");
			if(m_nReadMode == 0)
			{
				strKey.Format(_T("%u"),device_id);
				m_mapDeviceID[strKey.GetString()] = device_id;
			}
			else if(m_nReadMode == 1)
			{
				if(src)
				{
					strKey.Format(_T("%d.%d.%d.%d"),(int)src->mac[0],(int)src->mac[1], (int)src->mac[2], (int)src->mac[3]);
					device_id = inet_addr(Project::Tools::WideCharToAnsi(strKey).c_str());
					m_mapDeviceID[strKey.GetString()] = device_id;
				}
			}
			else if(m_nReadMode == 2)
			{
				if(src)
				{
					strKey.Format(_T("%d.%d.%d.%d/%u"),(int)src->mac[0],(int)src->mac[1], (int)src->mac[2], (int)src->mac[3],device_id);
					wstring wstrKey = strKey.GetString();
					hash_map<wstring,DWORD>::iterator iter = m_mapDeviceID.find(wstrKey);
					if(iter == m_mapDeviceID.end())
					{
						int nIndex = m_mapDeviceID.size()+1;
						m_mapDeviceID[wstrKey] = nIndex;
						device_id = nIndex;
					}
					else
					{
						device_id = (*iter).second;
					}
				}
			}
			address_add(device_id, max_apdu, src);
		} else {
#if PRINT_ENABLED
			fprintf(stderr, ", but unable to decode it.\n");
#endif
		}

		return;
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}

DataPointEntry* CBacnetCtrl::FindDataFromDeviceIDTypeInvokedID(UINT invokeId,UINT type, DWORD address)
{
	switch(type)
	{
	case OBJECT_ANALOG_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAI[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistAI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_ANALOG_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAO[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistAO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_ANALOG_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAV[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistAV[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBI[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistBI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBO[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistBO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBV[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistBV[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMI[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistMI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMO[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistMO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMV[deviceIdx];
				if( entry.GetPointAddress_Bacnet() == address && entry.GetBacnetInvokeID() == invokeId)
				{
					return &m_pointlistMV[deviceIdx];
				}
			}
		}
		break;
	default:
		break;
	}
	return NULL;
}

bool CBacnetCtrl::SendCommandByGroupByActive(vector<DataPointEntry> &entryList, bool& bDeviceNotBind )
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode())
	{
		return false;
	}
	
	unsigned int pointsize = entryList.size();
	if (pointsize <= 0)
	{
		return false;
	}

	DWORD deviceObjInstance = -1;
	if(pointsize > 0)
	{
		deviceObjInstance = GetBacnetDeviceID(entryList[0].GetParam(1));
	}

	if(deviceObjInstance < 0)
		return false;

	//先查找deviceObjInstanceID
	UINT max_apdu = 0;
	BACNET_ADDRESS  targetAddr;
	const bool ifFound = address_bind_request(deviceObjInstance, &max_apdu,  &targetAddr);
	if(!ifFound)			//未找到设备
	{
		CString strLog;
		strLog.Format(_T("ERROR: Bacnet address_bind_request:%u failed.\r\n"),deviceObjInstance);
		PrintLog(strLog.GetString(),LOG_ERROR);

		bDeviceNotBind = true;
		CDebugLog::GetInstance()->SetErrPoints(entryList,ERROR_CUSTOM_BACNET_ADDRESS_BIND_READ);
		for (unsigned int i = 0; i < pointsize; i++)
		{
			const DataPointEntry entry = entryList[i];
			DataPointEntry* saveData = FindDataFromDeviceTypeName(entry.GetPointIndex(),entry.GetShortName(),(BACNET_OBJECT_TYPE) entry.GetPointType_Bacnet());
			if(saveData)
			{
				saveData->SetToUpdate();
			}
			m_nCmdCount++;
		}
		return false;
	}
	else					//找到设备
	{
		//准备数据
		UINT8 buffer[MAX_PDU] = {0};
		BACNET_READ_ACCESS_DATA *pReadAccessData = new BACNET_READ_ACCESS_DATA;
		if(!pReadAccessData) 
			return false;
		BACNET_READ_ACCESS_DATA* pRpmData = pReadAccessData;
		for (unsigned int i = 0; i < pointsize; i++)
		{
			pRpmData->object_type = (BACNET_OBJECT_TYPE) entryList[i].GetPointType_Bacnet();	//point type
			deviceObjInstance = GetBacnetDeviceID(entryList[i].GetParam(1));	//server id
			pRpmData->object_instance = entryList[i].GetPointAddress_Bacnet(); //address
			pRpmData->listOfProperties = new BACNET_PROPERTY_REFERENCE;
			pRpmData->listOfProperties->propertyIdentifier = PROP_PRESENT_VALUE;
			pRpmData->listOfProperties->propertyArrayIndex = BACNET_ARRAY_ALL;
			m_nCmdCount++;
			pRpmData->next =NULL;
			if ( i < (pointsize-1))
			{
				pRpmData->next = new BACNET_READ_ACCESS_DATA;
				pRpmData = pRpmData->next ;
			}
		}
		pRpmData->next = NULL;

		//等待进入临界区，进入后加锁使其他线程不能进入
		EnterCriticalSection(&m_criticalSection); 
		m_nBacnetBindOK = 1;
		const UINT8 reqInvokeId = Send_Read_Property_Multiple_Request(&buffer[0], sizeof(buffer),deviceObjInstance,pReadAccessData);
		m_oleSendCmdTime = COleDateTime::GetCurrentTime();
		if(m_nCmdCount >= m_nPointCount)
			m_nLastInvolk = reqInvokeId;

		if(reqInvokeId == 0)
		{
			PrintLog(L"ERROR: Send_Read_Property_Multiple_Request:reqInvokeId = 0.\r\n", LOG_ERROR);
			CDebugLog::GetInstance()->SetErrPoints(entryList,ERROR_CUSTOM_BACNET_READ_MUTIL);
			m_nReqInvokeIdErrCount++;
			if(m_nReqInvokeIdErrCount >= 5)
			{
				m_nReqInvokeIdErrCount = 0;
				FreeAllInvokeId();						//释放
			}
		}
		else
		{
			//CString strLog;
			//strLog.Format(_T("INFO: Bacnet Send_Read_Property_Multiple_Request InvokeId:%d\r\n"),reqInvokeId);
			//PrintLog(strLog.GetString(), LOG_INFO);
			m_nBacnetSendOK = 1;
		}

		if(m_mapQueryPointInvolk.size() >= 10)
			m_mapQueryPointInvolk.erase(m_mapQueryPointInvolk.begin());		//移除第一个

		hash_map<wstring,DataPointEntry*> mapQueryPointInvolk;
		for (unsigned int k = 0; k < pointsize; k++)
		{
			const DataPointEntry entry = entryList[k];
			DataPointEntry* saveData = FindDataFromDeviceTypeName(entry.GetPointIndex(),entry.GetShortName(),(BACNET_OBJECT_TYPE) entry.GetPointType_Bacnet());
			if(saveData)
			{
				saveData->SetBacnetInvokeID(reqInvokeId);
				saveData->SetToUpdate();
				mapQueryPointInvolk[saveData->GetShortName()] = saveData;
			}
		}
		m_mapQueryPointInvolk[reqInvokeId] = mapQueryPointInvolk;

		ClearReadAccessData(&pReadAccessData);
		LeaveCriticalSection(&m_criticalSection);  //离开 开锁
		Sleep(m_nReadCmdInterval);
	}
	return true;
}

void CBacnetCtrl::SendReadCommandsByActive()
{
	bool bDeviceNotBind = false;
	//PrintLog(_T("\r\n========================BACnet SendReadCommands start=====================\r\n"), LOG_INFO);
	if(address_count() > 0)
	{
		SumReadAndResponse();			

		SetSendCmdEvent();
		for(int i=0; i<m_vecAI.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecAI[i],bDeviceNotBind))			//未发送成功直接激活下一次命令
			{
				SetSendCmdEvent();
			}
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecAI[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecAI.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecAO.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecAO[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}	
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecAO[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecAO.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecBI.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecBI[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}	
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecBI[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecBI.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecBO.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecBO[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecBO[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecBO.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecAV.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecAV[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}	
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecAV[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecAV.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecBV.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecBV[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecBV[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecBV.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecMI.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecMI[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecMI[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecMI.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecMO.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecMO[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecMO[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecMO.size()>0)
			Sleep(m_nReadTypeInterval);

		SetSendCmdEvent();
		for(int i=0; i<m_vecMV.size(); ++i)
		{
			//使用激活模式
			
			if(!SendCommandByGroupByActive(m_vecMV[i],bDeviceNotBind))
			{
				SetSendCmdEvent();
			}
			DWORD r = WaitForSingleObject(GetSendCmdEvent(), m_nReadTimeOut);
			if(r==WAIT_TIMEOUT)
			{
				CString strPointsErr = _T("ERROR: NOT RESPONSE POINTS LIST:");
				vector<DataPointEntry> vecErr = m_vecMV[i];
				for(int xx=0;xx<vecErr.size();xx++)
				{
					strPointsErr += vecErr[xx].GetShortName().c_str();
					strPointsErr+=_T(",");

				}
				strPointsErr+=_T("\r\n");

				PrintLog(strPointsErr.GetString(), LOG_ERROR);

			}
		}
		if(m_vecMV.size()>0)
			Sleep(m_nReadTypeInterval);
	}
	else
	{
		bDeviceNotBind = true;
	}
	m_bNeedActive = false;
	if(bDeviceNotBind)
	{
		OnTimerWhois_();
	}

	//PrintLog(_T("\r\n========================BACnet SendReadCommands end=====================\r\n"), LOG_INFO);
}

void CBacnetCtrl::SetSendCmdEvent()
{
	m_oleSendCmdTime = COleDateTime::GetCurrentTime();
	SetEvent(m_sendCmdEvent);
}

HANDLE CBacnetCtrl::GetSendCmdEvent()
{
	 return m_sendCmdEvent;
}

DataPointEntry* CBacnetCtrl::FindDataByDeviceIDTypeInvokedID( UINT invokeId,UINT type, DWORD address )
{
	hash_map<UINT8,hash_map<wstring,DataPointEntry*>>::iterator itertInvolk1 = m_mapQueryPointInvolk.find(invokeId);
	if(itertInvolk1 != m_mapQueryPointInvolk.end())
	{
		hash_map<wstring,DataPointEntry*>::iterator itertPoint;
		DataPointEntry* saveData = NULL;
		for(itertPoint = (*itertInvolk1).second.begin();itertPoint != (*itertInvolk1).second.end(); ++itertPoint)
		{
			if((*itertPoint).second->GetPointAddress_Bacnet() == address && (BACNET_OBJECT_TYPE)(*itertPoint).second->GetPointType_Bacnet() == type)
			{
				saveData = (*itertPoint).second;				//多个属性相同的点  
				if(saveData && saveData->GetUpdateSignal()>0)
				{
					return saveData;							//优先找未赋值过的
				}
			}
		}
		return saveData;										//否则返回其中的任意一个
	}
	return NULL;
}

UINT WINAPI CBacnetCtrl::ThreadActiveCommandsFunc( LPVOID lparam )
{
	CBacnetCtrl* pthis = (CBacnetCtrl*)lparam;
	if (!pthis)
	{
		return 0;
	}

	while (!pthis->GetSendCommandThreadExit())
	{
		while(!pthis->GetSendCommandThreadExit())
		{
			if(pthis->m_bNeedActive)
			{
				//判断是否超时激活
				//pthis->ActiveSendCmdEvent();
			}
			Sleep(500);
		}
	}

	return 0;
}

void CBacnetCtrl::ActiveSendCmdEvent()
{
	if(m_bNeedActive)
	{
		//判断是否超时
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleSendCmdTime;
		if(oleSpan.GetTotalSeconds() >= m_nReadTimeOut)
		{
			SetSendCmdEvent();
		}
	}
}

void CBacnetCtrl::SumReadAndResponse()
{
	if(m_nCmdCount > 0)			//统计信息
	{
		std::ostringstream sqlstream;
		m_nResponseCount = (m_nResponseCount>m_nCmdCount)?m_nUpdatePointCount:m_nResponseCount;
		sqlstream << "Read(" << m_nCmdCount << ") Response(" << m_nResponseCount << ") UpdatePoint(" << m_nUpdatePointCount << ")";
		m_strUpdateLog = sqlstream.str();

		CString strOut;
		strOut.Format(_T("INFO : Bacnet Engine:%s.\r\n"),Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str()).c_str());
		PrintLog(strOut.GetString(), LOG_INFO);
		
		hash_map<wstring,int>::iterator iterResponse = m_mapDeviceResponse.begin();
		std::ostringstream sqlstream_response;
		while(iterResponse != m_mapDeviceResponse.end())
		{
			if((*iterResponse).second == 1)				//未收到回复  统计错误
			{
				sqlstream_response << Project::Tools::WideCharToAnsi((*iterResponse).first.c_str()) << ",";
			}
			else
			{
				(*iterResponse).second = 1;
			}
			++iterResponse;
		}
		string strResponse = sqlstream_response.str();
		if(strResponse.length() > 0)
		{
			strResponse.erase(--strResponse.end());
			sqlstream_response.str("");
			sqlstream_response << "ERROR: Bacnet UnResponse(" << strResponse << ").\r\n";
			strResponse = sqlstream_response.str();
			PrintLog(Project::Tools::AnsiToWideChar(strResponse.c_str()), LOG_ERROR);
		}
	}
	m_nCmdCount = 0;
	m_nUpdatePointCount = 0;
	m_nResponseCount = 0;
	m_nLastInvolk = 1000;
	m_bNeedActive = true;
}

DataPointEntry* CBacnetCtrl::FindDataFromDeviceTypeName( DWORD index,wstring name,UINT type )
{
	switch(type)
	{
	case OBJECT_ANALOG_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAI[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistAI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_ANALOG_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAO[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistAO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_ANALOG_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistAV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistAV[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistAV[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBI[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistBI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBO[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistBO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_BINARY_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistBV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistBV[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistBV[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_INPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMI.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMI[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistMI[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_OUTPUT:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMO.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMO[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistMO[deviceIdx];
				}
			}
		}
		break;
	case OBJECT_MULTI_STATE_VALUE:
		{
			for(int deviceIdx = 0; deviceIdx < m_pointlistMV.size(); deviceIdx++)
			{
				const DataPointEntry& entry = m_pointlistMV[deviceIdx];
				if(entry.GetPointIndex() == index && entry.GetShortName() == name)
				{
					return &m_pointlistMV[deviceIdx];
				}
			}
		}
		break;
	default:
		break;
	}
	return NULL;
}

void CBacnetCtrl::SortPointList()
{
	//排序点表
	m_vecAI = SortPointByDeviceID(m_vecDeviceID,m_pointlistAI,m_nReadLimit);
	m_vecAO = SortPointByDeviceID(m_vecDeviceID,m_pointlistAO,m_nReadLimit);
	m_vecBI = SortPointByDeviceID(m_vecDeviceID,m_pointlistBI,m_nReadLimit);
	m_vecBO = SortPointByDeviceID(m_vecDeviceID,m_pointlistBO,m_nReadLimit);
	m_vecAV = SortPointByDeviceID(m_vecDeviceID,m_pointlistAV,m_nReadLimit);
	m_vecBV = SortPointByDeviceID(m_vecDeviceID,m_pointlistBV,m_nReadLimit);
	m_vecMI = SortPointByDeviceID(m_vecDeviceID,m_pointlistMI,m_nReadLimit);
	m_vecMO = SortPointByDeviceID(m_vecDeviceID,m_pointlistMO,m_nReadLimit);
	m_vecMV = SortPointByDeviceID(m_vecDeviceID,m_pointlistMV,m_nReadLimit);
}

void CBacnetCtrl::PrintLog( const wstring &strLog, int nLogLevel)
{
	_tprintf(strLog.c_str());
	
	OutDebugInfo(strLog, nLogLevel);
		
}

void PrintAddressCache()
{
	//addressCount = address_count();  
	CString cstr;
	//cstr.Format(_T("address_count = %d"), addressCount);
	//

	BACNET_ADDRESS address;
	uint32_t device_id = 0;
	unsigned max_apdu = 0;
	UINT total_addresses = 0;
	UINT port = 0;
	//output ip , maxapdu
	for (UINT i = 0; i < MAX_ADDRESS_CACHE; i++) {
		if (address_get_by_index(i, &device_id, &max_apdu, &address)) {
			if(address.mac_len > 5){
				//count port
				port = address.mac[4] + address.mac[5] * 0x100; 
				cstr.Format(_T("device=%d, ip=%d.%d.%d.%d:%d"), device_id,  
					address.mac[0], address.mac[1],address.mac[2], address.mac[3], port );
				//ADEBUG(cstr);
			}
			total_addresses++;
		}
		//ADEBUG(cstr);
	}
}
