//BacnetComm.h
#pragma once

#include "tools/CustomTools/CustomTools.h"
#include "../bacnet/include/bacdef.h"
#include "../bacnet/include/apdu.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include <hash_map>
#include "DB_BasicIO/RealTimeDataAccess.h"
//#include "vld.h"
//struct BACNET_ADDRESS;
//struct BACNET_CONFIRMED_SERVICE_ACK_DATA;
struct BACnet_Read_Property_Data;
typedef struct BACnet_Read_Property_Data BACNET_READ_PROPERTY_DATA;

struct BACnet_Read_Access_Data;
typedef struct BACnet_Read_Access_Data BACNET_READ_ACCESS_DATA;

struct BACnet_Application_Data_Value;
typedef struct BACnet_Application_Data_Value BACNET_APPLICATION_DATA_VALUE;

const UINT C_BANECT_PROPERTY = 85;

#define LOG_ERROR 0
#define LOG_WARNING 1
#define LOG_INFO 2

void PrintAddressCache();

class CBacnetCtrl
{
public: 
	CBacnetCtrl(int nReadInterval, int nReadTypeInterval, int nReadCmdInterval,int nReadMode,int nReadTimeOut,int nPrecision=6, int nReadLimit = 20, int nWritePriority=0);             	 						          // default consructor
	virtual ~CBacnetCtrl();    	 						          // default destructor

public:

	void	SetPointList(const vector<DataPointEntry>& entrylist);
	void	SortPointList();
	static void	SetDebug(int nDebug);
	bool	IsMacInitOK();		//网卡是否准备好
    virtual bool	Connect( );
    void	Disconnect(); 
	void	GetIPInfo(hash_map<wstring,wstring>& mapIP);
	bool	GetProcNameByPort(int nPort, string &strResult);
	
	// write
    bool SendWriteCmdBacnet(DWORD deviceObjInstance,
							WORD objType,
							DWORD objInstance,
							WORD objProperty,
							UINT8 tagType,
							double dValue,
							DWORD pointIndex,
							wstring pointName);

	vector<vector<DataPointEntry>> SortPointByDeviceID(vector<DWORD> vecDeviceID,vector<DataPointEntry> pointlist);

public:
    static void    InitServiceHandlers();
    void            InitServiceHandlersForWhois();
	
    bool            CreateThreadRead();
	
	static  DataPointEntry*    FindDataFromDeviceIDTypeAddress(DWORD deviceid,UINT type, DWORD address);
	static  DataPointEntry*    FindDataFromDeviceTypeName(DWORD index,wstring name,UINT type);
	static  DataPointEntry*    FindDataFromDeviceIDTypeInvokedID(UINT invokeId,UINT type, DWORD address);
	
    static UINT WINAPI      ThreadRead(LPVOID pParam);
	
    void            OnTimerRead();
    void            OnTimerWhois();
	void            OnTimerWhois_();

    static void            OnReceiveRead(BACNET_READ_PROPERTY_DATA* pData, BACNET_ADDRESS * src,UINT invokeId);
    static bool            OnReceiveReadMultiple(BACNET_READ_ACCESS_DATA* pRpmData, BACNET_ADDRESS * src,UINT invokeId);

    static void            CopyValue(BACNET_APPLICATION_DATA_VALUE* pDataValue, DataPointEntry* bacData );
    static void            FillWriteBuffer(BACNET_APPLICATION_DATA_VALUE* pDataValue, double dValue, UINT tag);

	
    static void		MyAbortHandler(BACNET_ADDRESS * src, uint8_t invoke_id,
										  uint8_t abort_reason, bool server);
	static void		MyIAMHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src);
	static void     MyIAMAddHandler(uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src);

    static void     MyErrorHandler(BACNET_ADDRESS * src, uint8_t invoke_id,
										  BACNET_ERROR_CLASS error_class, BACNET_ERROR_CODE error_code);
    static void     MyRejectHandler(BACNET_ADDRESS * src, uint8_t invoke_id, uint8_t reject_reason);
    static void     HandleReadPropertyAck(UINT8 * service_request,
										  UINT16 service_len,
										  BACNET_ADDRESS * src,
										  BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);
    static void            HandleReadPropertyMultiAck(UINT8 * service_request,
													  UINT16 service_len,
													  BACNET_ADDRESS * src,
													  BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);
    static void            HandleWritePropertyAck(BACNET_ADDRESS * src, uint8_t invoke_id);

		
	static void        ClearReadAccessData(BACNET_READ_ACCESS_DATA** pRpmData);

	//send thread
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);

	static DWORD GetBacnetDeviceID(const wstring strBacnetParam1);


	bool	GetSendCommandThreadExit() const;
	bool	GetReadDataThreadExit() const;

	void	SendReadCommands();

	static void	GetReceiveBacnetID(hash_map<UINT,wstring>& mapBacnetDevice);

	bool	SendReadCommand(DataPointEntry& entry);

	bool	SendCommandByGroup(int groupnum , vector<DataPointEntry> &entryList, int& nInvolkStartFromNum);

	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );

	void	SetValue(const wstring& pointname, double value);
	void	InitDataEntryValue(const wstring& pointname, double fvalue); //不发送通信指令, add by golding

	bool	GetValue(const wstring& pointname, double& result);

	void	Exit();

	static void	AddLog(const wchar_t* loginfo);
	static	bool	FreeAllInvokeId();

	static	void	OutDebugInfo(const wstring strOut, int nLogLevel);

	//////////////////////////////////////////////////////////////////////////
	static UINT WINAPI ThreadActiveCommandsFunc(LPVOID lparam);
	HANDLE	GetSendCmdEvent();			//获取激活事件
	static void	SetSendCmdEvent();			//设置激活事件
	void	ActiveSendCmdEvent();			//设置激活事件
	void	SendReadCommandsByActive();
	bool	SendCommandByGroupByActive(vector<DataPointEntry> &entryList, bool& bDeviceNotBind);		//激活发送
	vector<vector<DataPointEntry>> SortPointByDeviceID(vector<DWORD> vecDeviceID,vector<DataPointEntry> pointlist,int nGroupMax);

	static  DataPointEntry*    FindDataByDeviceIDTypeInvokedID(UINT invokeId,UINT type, DWORD address);
	void	SumReadAndResponse();						//每次发送前统计上一次的收发情况
	//////////////////////////////////////////////////////////////////////////
	static void 	PrintLog(const wstring &strLog, int nLevel);
protected:
    
	HANDLE              m_readdata_thread;		//接收线程
	bool                m_exit_readdata_thread;

	HANDLE              m_active_thread;		//接收线程
	
	HANDLE				m_hsendcommand_thread;			//发送线程
	bool                m_exit_sendcommand_thead;
	   
	//点的列表
	static  std::vector<DataPointEntry>	m_vecpointlist;  

	static  std::vector<DataPointEntry>  m_pointlistAI;
	static  std::vector<DataPointEntry>  m_pointlistAO;
	static  std::vector<DataPointEntry>  m_pointlistBI;
	static  std::vector<DataPointEntry>  m_pointlistBO;
	static  std::vector<DataPointEntry>  m_pointlistAV;
	static  std::vector<DataPointEntry>  m_pointlistBV;
	static  std::vector<DataPointEntry>  m_pointlistMI;
	static  std::vector<DataPointEntry>  m_pointlistMO;
	static  std::vector<DataPointEntry>  m_pointlistMV;

	//按设备号排序
	static  std::vector<vector<DataPointEntry>>  m_vecAI;
	static  std::vector<vector<DataPointEntry>>  m_vecAO;
	static  std::vector<vector<DataPointEntry>>  m_vecBI;
	static  std::vector<vector<DataPointEntry>>  m_vecBO;
	static  std::vector<vector<DataPointEntry>>  m_vecAV;
	static  std::vector<vector<DataPointEntry>>  m_vecBV;
	static  std::vector<vector<DataPointEntry>>  m_vecMI;
	static  std::vector<vector<DataPointEntry>>  m_vecMO;
	static  std::vector<vector<DataPointEntry>>  m_vecMV;

	static	int			m_nReqInvokeIdErrCount;
	static	int			m_nReadMode;		//默认0为读index  1为读ip  2为IP/ID
	static	int			m_nCmdCount;		//命令个数
	static	int			m_nResponseCount;	//请求回复个数
	static	int			m_nUpdatePointCount;//更新点
	static	int			m_nLastInvolk;
	static	int			m_nPointCount;		//点表总数
	static	int			m_nOutPut;			//1：输出信息
	static	int			m_nBacnetBindOK;	//0:bacnet bind fail
	static	int			m_nBacnetSendOK;	//0:send fail
	static	int			m_nBacnetResponseOK;//0:rec fail
	static	int			m_nBacnetRecOK;
	Project::Tools::Mutex	m_lock;

	static hash_map<wstring,DWORD>	m_mapDeviceID;		//IP/ID作为key
	static hash_map<wstring,DataPointEntry*> m_mapPointInvolk;		//已赋值Invol的点位指针
	static hash_map<wstring,int>			m_mapDeviceResponse;		//设备响应情况
	//static hash_map<wstring,DataPointEntry*> m_mapQueryPointInvolk;		//已赋值Invol的查找点位指针
	static hash_map<UINT8,hash_map<wstring,DataPointEntry*>> m_mapQueryPointInvolk;		//已赋值Invol的查找点位指针  最多存储10个Involk的查询数据
	static HANDLE		m_sendCmdEvent;

private:
	static Beopdatalink::CLogDBAccess* m_logsession;
	static CRITICAL_SECTION m_criticalSection;
public:
	static void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	static SYSTEMTIME	m_sExecuteTime;
	static string		m_strExecuteLog;
	//错误log
	static void	SaveExecuteOrder(string nOrder);
	string	GetExecuteOrderLog();
	int		m_nReadInterval;
	int		m_nReadTypeInterval;
	int		m_nReadCmdInterval;
	static COleDateTime	m_oleUpdateTime;
	static COleDateTime	m_oleStartTime;
	static COleDateTime m_oleSendWhoISTime;			//有设备号未找到时，每小时最多广播1次
	static	int			m_nSendWhiIsCount;			//广播次数
	string			m_strErrInfo;
	static	string m_strUpdateLog;
	int						m_nPrecision;				//数据精度 默认6
	static  int				m_nReadLimit;				//批量读取个数
	vector<DWORD>		m_vecDeviceID;
	//////////////////////////////////////////////////////////////////////////
	int					m_nReadTimeOut;							//超时时间
	bool				m_bNeedActive;							//需要激活事件
	static COleDateTime	m_oleSendCmdTime;						//发送命令时间
	static	wstring		m_wstrDeviceID;							//设备号信息
	//////////////////////////////////////////////////////////////////////////

	static int m_nLogLevel; //0:ERROR, 1:WARNING,2:INFO

	int m_nWritePriority;
};
