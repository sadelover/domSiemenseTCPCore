#pragma once

#include "windef.h"
#include "Equipment.h"

class COPCEngine;
class CModbusEngine;
class CMemoryLink;
class CBacnetEngine;
class CDataPointManager;
class CProtocol104Engine;
class CMysqlEngine;
class CSqlServerEngine;
//class CDTUSender;
class CBacnetServerEngine;
class CSiemensLink;
class CFCBusEngine;
class CSqliteEngine;
class CCO3PEngine;
class CWebEngine;
class CDiagnoseLink;

#include "DB_BasicIO/RealTimeDataAccess.h"
#include "BEOPDataPoint/DataPointEntry.h"
//#include "../DTUDataUpload/DTUSender.h"
#include "../COMdll/ModbusTcp/TCPDataSender.h"
#include "SiemensLink.h"
#include "../COMdll/ProcessView.h"
#include "../COMdll/ModbusTcp/DogTcpCtrl.h"

#include <map>
using std::map;
using namespace Beopdatalink;

#include "DataEngineMode.h"

#define DATA_BUFFER_SIZE 2048		//缓存大小

struct MysqlServer
{
	wstring strIP;
	wstring strSchema;
};

struct SqlServer
{
	wstring strIP;
	wstring strSchema;
};

struct SqliteServer
{
	wstring strFilePath;
	wstring strFileTable;
};

struct FCServer
{
	wstring strIP;
	wstring strPort;
};

struct OPCServer
{
	wstring strIP;
	wstring strServerName;
	int		nCount;		
};

struct WebServer
{
	wstring strIP;
	wstring strUser;
	wstring strPsw;
};

inline bool new_time(wstring const& lhs, wstring const& rhs)
{
	return lhs < rhs; //从小到大排序
}

class CBEOPDataLinkManager
{
public:
	CBEOPDataLinkManager();
	CBEOPDataLinkManager(CDataPointManager* pointmanager, Beopdatalink::CRealTimeDataAccess* pdbcon, bool bRealSite, bool bDTUEnabled,bool bDTUChecked,bool bDTURecCmd,int nDTUSendMinType =3,int nDTUPort = 1,bool bModbusReadOne=false,int nModbusInterval=100,bool bDTUDisableSendAllData=false,bool bTCPEnable=false,wstring strTCPName=L"",wstring strTCPServerIP=L"114.215.172.232",int nTCPServerPort=9500,int nSendAllData=1,bool bRunDTUEngine=false);
	~CBEOPDataLinkManager(void);

	virtual bool Init();
	virtual bool Exit();
	virtual bool InitOPC();
	
	static	unsigned int WINAPI ScanWarningAndOperation(LPVOID lpVoid);
	static  unsigned int WINAPI SendDTUReceiveHandleThread(LPVOID lpVoid);
	static	unsigned int WINAPI ThreadProjectDB(LPVOID lpVoid);		//定时扫描磁盘空间，自动清除数据库表

	virtual bool    InitInputTable();
	virtual bool	UpdateInputTable();
	virtual bool	UpdateOutputParams(wstring &strChangedValues);
	virtual bool SplitRealtimeDataEntryListToGroups(vector<CRealTimeDataEntry> &allEntryList, vector<vector<CRealTimeDataEntry>> &splitGroupList, int nMaxSizeOfGroup);

	virtual void	GetOPCValueSets(vector<pair<wstring, wstring> >& opcvaluesets);
	virtual void    GetSiemensTCPValueSets(vector<pair<wstring, wstring> >& vSets);
	virtual	void	GetModbusValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	virtual	void	GetCO3PValueSets(vector<pair<wstring, wstring> >& co3pvaluesets);
	virtual	void	GetProtocol104ValueSets(vector<pair<wstring, wstring> >& protocol104valuesets);
	virtual	void	GetMysqlValueSets(vector<pair<wstring, wstring> >& mysqlvaluesets);
	virtual	void	GetSqlServerValueSets(vector<pair<wstring, wstring> >& sqlvaluesets);
	virtual	void	GetSqliteValueSets(vector<pair<wstring, wstring> >& sqlitevaluesets);
	virtual	void	GetBacnetValueSets(vector<pair<wstring, wstring> >& bacnetvaluesets);
	virtual	void	GetFCbusValueSets(vector<pair<wstring, wstring> >& fcbusvaluesets);
	virtual	void	GetWebValueSets(vector<pair<wstring, wstring> >& fcbusvaluesets);
	virtual	void	GetModbusEquipmentValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	virtual	void	GetPersagyControllerValueSets(vector<pair<wstring, wstring> >& valuesets);
	virtual	void	GetThirdPartyValueSets(vector<pair<wstring, wstring> >& valuesets);
	virtual	void	GetOPCUAValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	virtual	void	GetBacnetPyValueSets(vector<pair<wstring, wstring> >& modbusvaluesets);
	
	virtual	bool	WriteOPCValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteModbusValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteCO3PValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteMemoryPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteBacnetPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool    WriteSiemensTCPPoint(wstring strIP, const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteFCbusValue(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteMysqlPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteSqlServerPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteSqlitePoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool	WriteWebPoint(const Beopdatalink::CRealTimeDataEntry& entry);

	virtual bool	WriteWirelessPoint(const Beopdatalink::CRealTimeDataEntry& entry);
	virtual bool    WriteModbusEquipmentPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WritePersagyControllerPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WriteOPCUAPoint( const Beopdatalink::CRealTimeDataEntry& entry );
	virtual bool    WriteBacnetPyPoint( const Beopdatalink::CRealTimeDataEntry& entry );


	virtual bool  WriteThirdPartyPoint( const Beopdatalink::CRealTimeDataEntry& entry );

	bool	UpdateHistoryTable(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nCount);
	bool	GenerateDTURealData(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle,int nSendAll);
	bool    CheckCreateHistoryTable(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle);
	bool    UpdatePointBuffer();
	bool	GenerateAndZipDataBack(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,wstring& strFileName,wstring& strFilePath);
	bool	GenerateDataBack(const SYSTEMTIME &st, wstring& strFileName, wstring& strFileFolder, vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	bool	InsertRedundancyInput();

	bool	GenerateDTURealDataS5(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle,int nSendAll);
	bool	GenerateAndZipDataBackS5(const SYSTEMTIME &st, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,wstring& strFileName,wstring& strFilePath);
	bool	GenerateDataBackS5(const SYSTEMTIME &st, wstring& strFileName, wstring& strFileFolder, vector<Beopdatalink::CRealTimeDataEntry> &entrylist);

	void	GenerateEntryList(vector<Beopdatalink::CRealTimeDataEntry>& resultlist);

	bool    IsEquipmentEnabled(Equipment &equip);

	void	SetPointManager(CDataPointManager* pointmanager);
	void	SetDbCon(Beopdatalink::CRealTimeDataAccess* pdbcon);
	void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
	void	SetRedundancyDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
	void	SetLogDbCon(Beopdatalink::CLogDBAccess* plogdb);

	void	ChangeValue(wstring &strChangedValues);

	void	SetMsgWnd(CWnd* msgwnd);

	void	SendLogMsg(LPCTSTR logFrom, LPCTSTR loginfo);

	void	CleatOutputData();

	void	ExitTAllThread();

	bool	GetExitThread_Input() const;
	bool	GetExitThread_Output() const;
	bool	GetExitThread_History() const;

	void	AddLog(const wstring& loginfo);
	wstring AutoSupportSimense1200Address(wstring strAddress);

	//尝试启动opc。持续3分钟，直到启动成功为止。
	bool	InitOPC(COPCEngine* opcengine,int nOPCClientIndex = -1);
	vector<vector<DataPointEntry>> GetOPCServer(vector<DataPointEntry> opcpoint,int nMaxSize,wstring strDefaultIP);	
	int	IsExistInVec(wstring strSer, wstring strProgram,vector<OPCServer>& vecSer,int nMaxSize);

	Beopdatalink::CLogDBAccess*	GetLogAccess();
	COPCEngine*	GetOPCEngine();
	CBacnetEngine *GetBacnetEngine();
	CMemoryLink *GetMemoryEngine();
	CDiagnoseLink*	GetDiagnoseEngine();
	CBacnetServerEngine *GetBacnetServerEngine();

	int GetModbusEngineCount();
	int GetCO3PEngineCount();
	int GetProtocol104EngineCount();

	//取得一个点的值。
	double	GetValue(const wstring& pointname);
	bool FindS7CPUInfo(const vector<_SiemensPLCProp> & ppList, const _SiemensPLCProp &pp);
	void	InitPointsValueFromDB();

	bool	IsMasterMode();
	void	SetEngineMode(int modevalue);
	BeopDataEngineMode::EngineMode GetEngineMode() ;
	virtual void	UpdateMainServerMode();		//更新主从.

	bool	InitModbusEngine(vector<DataPointEntry> modbuspoint, int nReadCmdMs = 50,int nMutilReadCount=99,int nIDInterval=500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false,bool bSaveErrLog = false);
	vector<vector<DataPointEntry>> GetModbusServer(vector<DataPointEntry> modbuspoint);	
	int	IsExistInVec(wstring strSer, vector<wstring> vecSer);

	bool	InitCO3PEngine(vector<DataPointEntry> co3ppoint, int nReadCmdMs = 50,int nRollInterval=2,int nTimeOut=5000,int nPrecision=6);
	vector<vector<DataPointEntry>> GetCO3PServer(vector<DataPointEntry> co3ppoint);	
	
	bool	InitFCbusEngine(vector<DataPointEntry> fcbuspoint, int nReadCmdMs = 50,int nPrecision=6);
	vector<vector<DataPointEntry>> GetFCbusServer(vector<DataPointEntry> fcbuspoint);	
	int	IsExistInVec(wstring strSer, wstring strPort,vector<FCServer> vecSer);

	int IsExistInVec(wstring strIP,wstring strSechema, vector<MysqlServer> vecSer);
	int IsExistInVec(wstring strIP,wstring strSechema, vector<SqlServer> vecSer);
	int IsExistInVec(wstring strPath,wstring strTName, vector<SqliteServer> vecSer);
	int IsExistInVec(wstring strIP,wstring strUser, wstring strPsw,vector<WebServer> vecSer);

	bool	InitProtocol104Engine(vector<DataPointEntry> protocol04point);
	vector<vector<DataPointEntry>> GetProtocol104Server(vector<DataPointEntry> protocol04point);	

	bool	InitMysqlEngine(vector<DataPointEntry> mysqlpoint);
	vector<vector<DataPointEntry>> GetMysqlServer(vector<DataPointEntry> mysqlpoint);

	bool	InitSqlServerEngine(vector<DataPointEntry> sqlpoint);
	vector<vector<DataPointEntry>> GetSqlServer(vector<DataPointEntry> sqlpoint);

	bool	InitSqliteEngine(vector<DataPointEntry> sqlitepoint);
	vector<vector<DataPointEntry>> GetSqliteServer(vector<DataPointEntry> sqlitepoint);	

	bool	InitWebEngine(vector<DataPointEntry> webpoint);
	vector<vector<DataPointEntry>> GetWebServer(vector<DataPointEntry> webpoint);

	void EnableLog(BOOL bEnable);
	BOOL IsLogEnabled();

	void GetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void GetStoreRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void SetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void CompareModifiedRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, map<string, wstring> &entrylistMapBuffer, vector<Beopdatalink::CRealTimeDataEntry> &newEntrylist);
	void CompareAndUpdateRealTimeDataEntryList(SYSTEMTIME sNow);	//保存变化数据 更新变化时间
	void GetModifiedRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nMinutes);			//得到指定分钟内时间

	void GetRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist, const POINT_STORE_CYCLE & tCycle);


	void UpdateRealTimeDataEntryList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);
	void UpdateOutputParamsByList(vector<Beopdatalink::CRealTimeDataEntry> &entrylist);

	string BuildDTUSendData(int nPointStoreType, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nCount);
	string BuildDTUSendData_Change(int nPointStoreType, vector<Beopdatalink::CRealTimeDataEntry> &entrylist,int nChange,int nCount);
	string GenerateDTUUpdateInfo_Change(int nPointStoreType,int nUpdateSize,int nCountSize,Beopdatalink::CRealTimeDataEntry& enrty);		// 统计DTU发送的数据
	string GenerateDTUUpdateInfo(int nPointStoreType,int nUpdateSize,int nCountSize);		// 统计DTU发送的数据
	string BuildDTUSendUnit01(vector<unitStateInfo> unitlist);

	HANDLE	GetSendDTUEvent();

	bool InsertRedundancyUnit01(string strRedundancyIP);
	string GetHostIP();

	
	bool	SetSendDTUEventAndType(int nDTUDataType, SYSTEMTIME tSendDTUTime);
	bool	SendDTUData();
	bool	ScanWarning();				//扫描报警
	bool	ScanOperation();			//扫描处理操作
	bool	ScanUserOperation();			//扫描用户操作

	bool	SendCoreStart(SYSTEMTIME tSendDTUTime);									//Core启动时发送
	bool	SendAllRealData(SYSTEMTIME tSendDTUTime);			//发送全部实时数据
	bool	SendCoreStart();								//DTU 发送启动标志
	bool    UpdateHistoryStatusTable(CString strProcessName);

	bool	HandleCmdFromDTUServer();							//处理来自DTUServer的命令
	bool	ActiveLostSendEvent();								//有待发送丢失队列则激活
	bool	UpdateWatch();										//处理更新看门狗
	bool	UpdateDebug();										//查询调试信息输出
	bool	ChangeValueByDTUServer(string strPointName, string strValue);
	bool	ChangeValuesByDTUServer(string strReceive);
	bool	ChangeUnit01ByDTUServer(string strParamName, string strValue);
	bool	SendHistoryDataFile(string strStart,string strEnd);
	bool	RestartCore();		//重启Core
	bool	RestartWatch();		//重启Watch
	bool	RestartLogic();		//重启Logic
	bool	SynUnit01();		//同步Unit01
	bool	SynRealData(POINT_STORE_CYCLE nType);	//同步某种类型数据
	bool	SendRealData(POINT_STORE_CYCLE nType);	//同步某种类型数据
	bool	ExecuteDTUCmd(string strReceive);			//执行Sql语句
	bool	AckHeartBeat();						//回应服务器心跳包
	bool	AckSynSystemTime(string strSystemTime);						//回应服务器同步系统时间
	bool	AckCoreVersion();									//回应Core版本号
	bool	AckReSendData(string strIndex);					//回应丢失重发数据
	bool	AckReSendLostData(string strMinutes,string strPacketIndex);			//回应丢失掉线期间数据
	bool	AckReSendLostZipData(string strMinutes,string strFileNames);			//回应丢失掉线期间数据
	bool	AckUpdateExeFile(string strExeName);								//回应更新文件成功标志
	bool	AckLostExeFile(string strLostFile);								//回应更新文件成功标志
	bool	AckReSendFileData(string strFileName);					//回应丢失重发数据
	bool	AckReStartDTU();									//回应重启DTU
	void	ReadCSVFileAndSendHistory(string strPath);
	bool	ReadDataAndGenerateHistoryFile(string strFileName,string strFolder,string strPath);		//数据文件不存在时候，读取数据库重新生成地址
	bool	ScanSendHistoryQueue(int nSecond = 4);
	bool	SendOneLostHistoryData();							//发送一个待发送的历史丢失数据

	bool	StoreFileInfo(wstring strFolder,wstring strFileName,int nPointCount);
	int		GetFileInfoPointCont(string strPath,string strFileName);

	wstring ConvertPointType(POINT_STORE_CYCLE nType);

	bool	SendExecuteOrderLog();    //发送Log并存储到日志文件
	bool	OutPutLogString(SYSTEMTIME st,string strOut);


	void	SplitString(const std::string& source, const char* sep, std::vector<string>& resultlist);

	bool	CheckDiskFreeSpace(string strPath);	

	bool	CloseWatchDog();		 //关闭当前运行Watch
	bool	OutPutUpdateLog(wstring strLog);		//记录更新Log
	bool	UpdatePrivilege();						//提升权限
	bool	ImproveProcPriv();
	bool	RunBatCmd_SetSystemTime(SYSTEMTIME st);
	bool	SaveBacnetInfo();

	//////////////////////////////////////////////////////////////////////////
	bool	RestartByMaster(E_SLAVE_MODE nRestartExe, E_SLAVE_REASON nReason,wstring strPath);
	void 	PrintLog(const wstring &strOut,bool bSaveLog = false);
public:
	int		m_nDTUDataType;		//0 实时数据； 1 报警   2 报警操作 3: Unit01 4: ExecuteLog
	bool m_bRealSite;
	bool	m_bFirstStart;
	int m_nUpdateInputCount;
	int m_nUpdateOutputCount;

	map<wstring, Equipment> m_mapEquipment;

private:
	//data engine will hold this two result.
	COPCEngine* m_opcengine;
	CMemoryLink*  m_memoryengine;
	CDiagnoseLink* m_pDiagnoseEngine;
	CBacnetEngine* m_bacnetengine;
	CBacnetServerEngine * m_bacnetServerEngine;
	vector<wstring>	m_vecModbusSer;
	vector<wstring>	m_vecCO3PSer;
	vector<wstring>	m_vecProtocol104Ser;
	vector<MysqlServer>	m_vecMysqlSer;
	vector<WebServer>	m_vecWebSer;
	vector<SqlServer>	m_vecSqlSer;
	vector<SqliteServer>	m_vecSqliteSer;
	vector<FCServer>	m_vecFCSer;
	vector<OPCServer>	m_vecOPCSer;
	map<wstring,CModbusEngine*> m_mapMdobusEngine;
	map<wstring,CCO3PEngine*> m_mapCO3PEngine;
	map<wstring,CFCBusEngine*> m_mapFCBusEngine;
	map<wstring,CProtocol104Engine*> m_mapProtocol104Engine;
	map<wstring,CMysqlEngine*> m_mapMysqlEngine;		//IP+数据库作为Key
	map<wstring,CSqlServerEngine*> m_mapSqlServerEngine;	//IP+数据库作为Key
	map<wstring,CSqliteEngine*>	m_mapSqliteEngine;		//地址+表名作为key
	map<int,COPCEngine*>	m_mapOPCEngine;				//index编号OPC引擎（OPC点表超过10000时候启用）
	map<wstring,CWebEngine*> m_mapWebEngine;			//IP+用户名+密码 作为Key
	CSiemensLink * m_pSiemensEngine;

	bool m_bModbusEquipmentEngineEnable;
	bool m_bThirdPartyEngineEnable;
	bool m_bPersagyControllerEngineEnable;
	bool m_bOPCUAEngineEnable;
	bool m_bBacnetPyEngineEnable;


	HANDLE	m_hupdateinput;
	bool	m_bexitthread_input;
	HANDLE	m_hupdateoutput;
	bool	m_bexitthread_output;
	HANDLE	m_hhistorydata;
	HANDLE	m_hhistoryS5data;
	bool	m_bexitthread_history;
	
	// data engine will not not hold the object.
	Beopdatalink::CRealTimeDataAccess*	m_dbsession_realtime;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
	Beopdatalink::CCommonDBAccess*	m_dbsession_Redundancy;
	Beopdatalink::CLogDBAccess*			m_dbsession_log;
	CDataPointManager*	m_pointmanager;

	vector<CRealTimeDataEntry>	m_vecOutputEntry;

	Project::Tools::Mutex m_lock_vecMemoryPointChangeList;
	Project::Tools::Mutex m_toollock;			//工具锁
	vector<CRealTimeDataEntry>	m_vecMemoryPointChangeList;

	CWnd*	m_msgwnd;

	BeopDataEngineMode::EngineMode	m_enginemode;
	Project::Tools::Mutex m_enginemode_lock;

	CRITICAL_SECTION m_RealTimeData_CriticalSection;

	vector<Beopdatalink::CRealTimeDataEntry> m_RealTimeDataEntryList;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_S5;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_M1;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_M5;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_H1;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_D1;
	map<string, wstring> m_RealTimeDataEntryListBufferMap_Month1;
	POINT_STORE_CYCLE m_nMinTCycle;

	map<string,CRealTimeDataEntry>	m_RealTimeDataMap;
	map<string,int>					m_mapSendLostHistory;

	BOOL m_bLog;

	COleDateTime m_tLastHistorySaved;
	COleDateTime m_oleLastRemoteSetting;		//最后收到远程更新点时候 2分钟内加快发送时间
	COleDateTime	m_oleCoreStart;				//Core启动时间
	bool			m_bUpdateBacnetInfo;		//更新Bacnet信息，仅在Core启动后第5分钟内执行一次

	bool m_bDTUEnabled;
	bool		m_bdtuSuccess;
	HANDLE		m_senddtuevent;
	HANDLE		m_hsenddtuhandle;
	HANDLE		m_hReceivedtuhandle;
	HANDLE		m_hscanwarninghandle;				//扫描报警
	string		m_strSendBuf;
	string		m_strSendWarningBuf;				//报警数据
	string		m_strSendOperationBuf;				//报警操作
	string		m_strSendUserOperationBuf;			//用户操作
	SYSTEMTIME  m_tSendDTUTime;						//DTU发送时间
	Project::Tools::Mutex m_dtu_lock;
	Project::Tools::Mutex m_dtu_ack_lock;

	bool m_bFirstInitRedundancyBuffer;
	int	 m_nDTUPort;
	int	 m_nDTUSendMinType;
	bool m_bDTUChecked;
	bool m_bDTURecCmd;
	bool m_bModbusreadonebyone;
	int  m_nMobusInterval;
	int	 m_nCO3PInterval;								//ms
	int	 m_nCO3PRecevieTimeOut;				//ms
	int	 m_nCO3PRollInterval;				//s
	int	m_nMutilReadCount;		//批量连读个数 默认99
	int		m_nIDInterval;			//不同ID之间的间隔 默认500ms
	int		m_nModbusTimeOut;			//modbus超时
	int		m_nModbusPollInterval;		//modbus轮询间隔
	bool m_bDTUDisableSendAllData;
	bool		m_bVecCopying;					//正在copy不能添加

	bool	m_bTCPEnable;						//TCP网络发送模式
	wstring m_strTCPServerIP;					//TCP服务器地址
	wstring m_strTCPName;						//用于标识TCP项目
	int		m_nTCPServerPort;					//TCP服务器端口

	string		m_strAck;							//修改点ACK回复内容合并发送
	string		m_strZipPath;
	string      m_strSendRealFilePath;				//需要立即发送的实时包地址
	int			m_nSendRealFileCount;				//需要立即发送的实时包点数
	SYSTEMTIME m_ackTime;							//回复时间
	int			m_nStoreZip;					//0 不存储 1:5s 2:1m  3:5m 4:30m 5:1h 6:1d
	int			m_nSendAll;						//0 不发送全部  1：每次发送全部
	int			m_nSendPointCount;				//发送个数
	int			m_nOPCClientMaxPoint;			//OPCClient最大点数限制
	int			m_nMutilCount;					//OPC批量读取点个数
	int			m_nOPCCmdSleep;					//OPC读点间隔（从缓存）
	int			m_nOPCCmdSleepFromDevice;		//OPC读点间隔(从设备)
	int			m_nOPCPollSleep;				//OPC轮询间隔（从缓存）
	int			m_nOPCPollSleepFromDevice;		//OPC轮询间隔(从设备)
	int			m_nReconnectMinute;				//重连后N分钟内不存储数据
	int			m_nSendRemoteSet;				//远程设置过后Ns内发送5秒数据
	bool		m_bMutilOPCClientMode;			//多OPCClient模式
	bool		m_bMutilOPCClientUseThread;		//多OPCClient模式使用线程模式
	bool		m_bStoreHistory;				//是否存储历史数据
	bool		m_bEnableSecurity;				//开启进程安全设置项
	bool		m_bDisableCheckQuality;			//开启禁用品质数据检测
	int			m_nOPCMainThreadOPCPollSleep;	//OPC轮询间隔s（主进程从缓存）
	bool		m_bAsync20Mode;					//异步读写模式
	int			m_nRefresh20Interval;			//异步刷新间隔
	int			m_nLanguageID;					//语言ID
	int			m_nMultiAddItems;				//批量添加点
	int			m_nUpdateRate;					//向服务器发送请求的刷新率
	wstring		m_strLastSendOperation;			//最后发送操作记录时间

	char			m_cSpiltBuffer[DATA_BUFFER_SIZE];			//数据解析缓存
	//map<string, wstring>		m_RealTimeDataEntryListBufferMap_Month1;

	CProcessView		m_PrsV;

	int			m_nRemoteSetOPC;
	int			m_nRemoteSetModbus;
	int			m_nRemoteSetBacnet;
	int			m_nRemoteSetSiemens;
	int			m_nRemoteSetMysql;
	int			m_nRemoteSetSqlite;
	int			m_nRemoteSetSqlServer;
	int			m_nOPCCheckReconnectInterval;	//OPC检测重连间隔（min）
	int			m_nOPCReconnectInertval;		//OPC N分钟数据未更新重连(min)
	int			m_nDebugOPC;
	int			m_nPrecision;				//数据精度 默认6
	int			m_nMaxInsertLimit;			//数据库一次性插入最大限制
	bool		m_bDisSingleRead;
	bool		m_bSaveErrLog;			//是否存储错误日
	bool		m_bRunDTUEngine;			//是否启用DTUEngine程序

	int         m_nLoopOnceInterval;
};
