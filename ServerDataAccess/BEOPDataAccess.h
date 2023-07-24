#pragma once
#include <string>
#include <vector>
#include <hash_map>

using namespace std;

#include "../DB_BasicIO/DatabaseSession.h"
#include "../DB_BasicIO/RealTimeDataAccess.h"
#include "../DB_BasicIO/WarningConfigItem.h"

#include "../ServerDataAccess/BEOPDataAccessBase.h"

using Beopdatalink::CRealTimeDataEntry;
using Beopdatalink::CRealTimeDataAccess;

using Beopdatalink::COperationRecords;

using Beopdatalink::CUserOnlineEntry;

#define _ERROR_VALUE_ 0x00 //错误值


#define  OPCBADVALUE -777.77   //读到的值为非法的double值.

typedef struct tag_PhyPointValueStruct 
{
	SYSTEMTIME   time;
	//wstring   physicalid;
	wstring strValue;

}gPhyPointValueStruct;

enum Beop_CONTROL_MODE      //操作模式  
{
	Mode_System_Manual,
	Mode_System_Auto,
	Mode_System_Measure,
	Mode_System_FreeCool,
};


enum Beop_SYSTEM_ACTION
{
	Act_Empty,			//空闲中
	Act_RunOptimizing,	//执行优化动作中
	Act_Starting,		//开机中
	Act_Closing,		//关机中
	Act_Standbying,		//待机中
	Act_CHProtecting,	//排热量保护中
};


class CBEOPDataAccess : public CBEOPDataAccessBase
{
public:
	CBEOPDataAccess(void);
	~CBEOPDataAccess(void);

	 bool InitConnection(bool bRealtimeDB, bool bLogDB, bool bCommonDB);
	 bool DisConnection();
	 bool StartUpdatePTValThread(void);


	 void SetDBparam(gDataBaseParam mdbParam);
	 CRealTimeDataAccess* GetRealTimeDataAccess();
	 void TerminateAllThreads();
//private:
public:
	 bool GetPhyPointValueRecord();
public:
	bool GetValue(const tstring& strName, bool &bParam);
	bool GetValue(const tstring& strName, double &dbParam);
	bool GetValue(const tstring& strName, int &uiParam);
	bool SetValue( const tstring& strName, double dParam );
	bool SetValue( const tstring& strName, bool bParam );
	bool SetValue( const tstring& strName, int nParam );
	bool GetValue(const tstring& strName, wstring &strPsaram);
	bool SetValue( const tstring& strName, wstring nParam );
	bool SetValueDirect(const tstring & strName,const wstring & strParam);

	bool	ReadRealTimeData_Output(vector<CRealTimeDataEntry>& entrylist);
	bool	UpdatePointData(const CRealTimeDataEntry& entry);
	bool	DeleteRealTimeData_Output(vector<CRealTimeDataEntry>& entrylist);
	bool    DeleteEntryInOutput(const string& pointname);
	
	void				SetSystemAction(Beop_SYSTEM_ACTION eAction);
	Beop_SYSTEM_ACTION	GetSystemAction();
	CString&			GetActionString();
	bool				IsSystemFree();

	void SetDataBaseParam(gDataBaseParam dataBaseParam);

	bool	ReadLog(vector< pair<wstring,wstring> >& loglist, const SYSTEMTIME& start, const SYSTEMTIME& end);
	bool	ReadLog(vector< pair<wstring,wstring> >& loglist, const int nCount);
	bool	InsertLog(const wstring& loginfo);
	bool	InsertLog(const vector<SYSTEMTIME> & sysTimeList, const vector<wstring> & loginfoList);
	bool    DeleteLog(const SYSTEMTIME tEnd);

	bool    AddOperationRecord(const wstring &strUserName, const wstring & strOperationContent);
	virtual bool	AddWarning(int nWarningLevel , const wstring &strWarningContent,const wstring &strBindPoint, int nRuleID,
		const wstring &strOfPosition,const wstring &strOfSystem,const wstring &strOfDepartment,const wstring &strOfGroup,const wstring &strTag,
		const wstring &strInfoDetail, const wstring &strInfoRange, const wstring &strBindPointValue,
		const wstring &strUnitProperty03,const wstring &strUnitProperty04,const wstring &strUnitProperty05);
	bool	ReadWarning(vector<CWarningEntry>& resultlist, const SYSTEMTIME& start, const SYSTEMTIME& end);
	bool	DeleteWarning();

	bool	ReadWarningConfig(vector<CWarningConfigItem>& configList);
	wstring ReadOrCreateCoreDebugItemValue(wstring strItemName);

	bool    ClearLogicOutputPointRecordBeforeTime(wstring wstrTime);
	bool    CheckAndRepairHistoryDataTableHealthy();
	wstring ReadOrCreateCoreDebugItemValue_Defalut(wstring strItemName,wstring strValue = L"0");
	bool     ReadCoreDebugItemValue(wstring strItemName, wstring &strItemValue);
	bool     ReadCoreDebugItemValue(wstring strItemName, wstring strItemSub, wstring &strItemValue);
	bool    WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue);
	bool    WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemNameSub,const wstring &strItemValue);
	bool    WriteCoreDebugItemValue2(const wstring &strItemName, const wstring &strItemValue);

	//logic related
	bool    ClearLogicParameters();
	bool    ReadLogicParameters(wstring &strThreadName, wstring &strLogicName, wstring &strSetType, wstring &strVariableName, wstring &strLinkName, wstring &strLinkType);
	bool    DeleteLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType);
	bool	InsertLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType, wstring strCondition);


	bool	GetPointExist(const wstring& strName);
	virtual bool	InsertOperationRecord(const SYSTEMTIME& st, const wstring& remark, const wstring& wstrUser);
	bool	ReadAllOperationRecord(std::vector<COperationRecords>&  vecOperation, const COleDateTime& timeStart, const COleDateTime& timeStop);
	bool	DeleteOperationRecord();


	bool	SetStatus(const wstring& statustype, int value);
	bool	GetStatus(const wstring statustype, int nStatus);

	bool	RegisterUserRecord(const CUserOnlineEntry& entry);
	bool	UpdateUserOnlineTime(wstring username, wstring usertype, SYSTEMTIME time);
	bool	ReadUserRecord(vector<CUserOnlineEntry>& resultlist);
	bool	DeleteUserRecord(wstring username, wstring usertype);

	bool	IsUserOnline(wstring username, wstring usertype);
	int		GetClientUserCount();

	bool    GetServerTime(SYSTEMTIME &time);

	bool	InsertHistoryData(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle, const vector<CRealTimeDataEntry>& datalist);

	int		GetSpanTimeMonths(COleDateTime& start, const COleDateTime& end);

	bool    UpdatePointValueBuffer(const wchar_t* lpszName, const wchar_t* lpszValue);
	bool    UpdatePointValueInput(const wchar_t* lpszName, const wchar_t* lpszValue);
	bool    UpdatePointValueOutput(const wchar_t* lpszName, const wchar_t* lpszValue);

	void	LogPointNotInMapError(wstring strPointName);
	
	bool	InsertS3DBPath(const wstring& s3dbpathname,const wstring& s3dbpathvalue);
	bool	GetS3DBPath(const wstring& s3dbpathname,wstring& s3dbpathvalue);

	bool GetHistoryValue(const wstring& strName,const SYSTEMTIME &st,const int &nTimeFormat,  wstring &strParam);
	bool GetHistoryValue(const wstring& strName,const SYSTEMTIME &stStart,const SYSTEMTIME &stEnd,const int &nTimeFormat,  wstring &strParam);

	bool UpdateSavePoints(const vector<DataPointEntry>& ptList);

	bool DownloadFile(wstring strFileID, wstring strFileSavePath);
	bool	UpdateLibInsertIntoFilestorage(const wstring strFilePathName, const wstring strFileName, const wstring strFileId);
	bool DeleteFile(wstring strFileID);

	//////////////////////////////////////////////////////////////////////////
	bool	CreateScheduleTableIfNotExist();
	bool    CreateHistoryDataStatusTableIfNotExist();
	bool	GetScheduleInfo(const string strDate,const string strTimeFrom,const string strTimeTo, const int nDayOfWeek,vector<Beopdatalink::ScheduleInfo>& vecSchedule);
	bool	DeleteScheduleInfo(const int nID);
	bool	UpdateScheduleInfoExecuteDate(const int nID,const string strDate);
	bool	GetScheduleExist();
	void    LogInfo( COleDateTime oleNow,CString strErr );
	bool    UpdateHistoryStatusTable(CString strProcessName);

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//读取数据库配置参数
	bool	ReadMysqlVariable(wstring wstrVariableName, wstring& wstrVariableValue);	//读取数据配置
	bool	SetSaveLogFlag(bool bSaveLog);												//设置存储Log总开关
	//////////////////////////////////////////////////////////////////////////

public:
	hash_map<wstring, gPhyPointValueStruct> GetValueRecordMap();
//private:	
public:

	void	SetDataUpdate(bool bUpdate);
	//2s一次，读取
	static UINT WINAPI ThreadFunc_UpdatePhyPointValRecord(LPVOID lparam);
	void	UpdatePhyPointValRecord();
	bool	ExportLog(const SYSTEMTIME& start, const SYSTEMTIME& end, const wstring& filepath);
	bool	InitConnection2(void);
protected:
	vector<CString>						m_vctOpcValue;
	CDatabaseSeesion					m_dbSession;
	vector<gPhyPointValueStruct>				m_vecPhyPointValueRecord;
	hash_map<wstring, gPhyPointValueStruct>     m_mapPhyPointValueRecord;
	vector<CRealTimeDataEntry>					m_entrylist;
	Beopdatalink::CLogDBAccess								m_pLogDBAccess;				//LOG
	CRealTimeDataAccess							m_pRealTimeDataAccess;		//实时数据库
	Beopdatalink::CCommonDBAccess								m_pCommonDBAccess;			//常规数据库操作
	bool m_bExitThread;
private:
	HANDLE					m_threadhandle_UpdatePhyPointValRecord;
	gDataBaseParam			m_dbParam;
	CString					m_strSysAction;
	Beop_SYSTEM_ACTION		m_eSysAction;
	Project::Tools::Mutex	m_lock;	
	bool					m_bThreadActive;

	wstring					m_strLogFilePath;
	bool					m_bSaveLogFlag;					//存储Log总开关

};

