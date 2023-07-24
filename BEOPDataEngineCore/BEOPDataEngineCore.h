
#pragma once
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 BeopDATAENGINECORE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// BeopDATAENGINECORE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。


#include "DB_BasicIO/RealTimeDataAccess.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "DataBaseSetting.h"

class CBEOPDataLinkManager;
class CDataPointManager;
class CLogicPointManager;
class CAlarmPointManager;
class Beopdatalink::CRealTimeDataAccess;
class Beopdatalink::CCommonDBAccess;
class CMDOPointConfigDataAccess;
//class CDTUSender;
class CEngineRegister;
class CBeopGatewayCoreWrapper;

// 此类是从 BeopDataEngineCore.dll 导出的
class CBeopDataEngineCore 
{

public:
	CBeopDataEngineCore(std::wstring strDBFilePath);
	~CBeopDataEngineCore(void);

	bool InitDBConnection();
	bool InitEngine();
	bool InitLogic();
	bool Exit();
	bool Init(bool bDTUEnabled,bool bDTUChecked,bool bDTURecCmd,int nDTUPort,int nDTUSendMinType,bool bModbusReadOne,int nModbusInterval,bool bDTUDisableSendAllData,bool bTCPEnable,wstring strTCPName,wstring strTCPServerIP,int nTCPServerPort,int nSendAllData,bool bRunDTUEngine);
	bool TerminateScanWarningThread();
	bool TerminateReconnectThread();
	void SetDataAccess(CBEOPDataAccess *pAccess);
	void	SetMsgWnd(CWnd* msgwnd);

	Beopdatalink::CRealTimeDataAccess* GetRealTimeDataAccess();
	Beopdatalink::CCommonDBAccess * GetCommonDBAccess();
	Beopdatalink::CLogDBAccess * GetLogDBAccess();
	void SetRealTimeDataAccess(Beopdatalink::CRealTimeDataAccess* pAccess);
	void SetCommonDBAccess(Beopdatalink::CCommonDBAccess *pAccess);
	void SetLogDBAccess(Beopdatalink::CLogDBAccess *pAccess);
	bool SetDBAcessState( bool bState );
	void ReadRealDBparam(gDataBaseParam &dbParam);

	void ReadEquipmentEnableOption();

	void EnableLog(BOOL bEnable);
	//CDTUSender * GetDTUSender();

	bool UpdateInputTable();
	bool	InitPoints(vector<wstring> & ignorePointList, int nProcessNumber);
	CBEOPDataLinkManager * GetDataLink();

	CDataPointManager* GetDataPointManager();
	//CBEOPDataAccess *GetServerDataAccess();

	bool	InsertRedundancyOperation(wstring wstrRedundancyIp);
	
	DataBaseSetting LoadConfig();
	bool SaveConfig(DataBaseSetting &dbset);
	wstring GetConfigVersion();

	bool CheckDBExist(wstring wstrDBName = L"beopdata");
	bool ReBuildDatabase(wstring wstrDBName = L"beopdata");
	bool DropDatabase(wstring wstrDBName = L"beopdata");
	bool DatabaseUpdateFrom22To23(wstring wstrDBName = L"beopdata");
	bool DatabaseUpdateFrom23To24(wstring wstrDBName = L"beopdata");
	bool DatabaseCreateNewInputOutputTable(wstring wstrDBName = L"beopdata");
	double GetMySQLDBVersion(wstring wstrDBName = L"beopdata");
	bool	SetDefaultDBIP(string strIP);
	bool	SetDefaultDBUser(string strUser);

	bool ReconnectRedundancyDB();
	string	RelaceStr(char* strSrc, char* strFind, char* strReplace);

	void 			PrintLog(const wstring &strLog,bool bSaveLog = true);

	static  UINT WINAPI ThreadFunc_ScanWarning(LPVOID lparam);
	static  UINT WINAPI ThreadFunc_ConnectRedundancyDB(LPVOID lparam);
private:

	void	InitCommandLog();

private:
	CDataPointManager*	m_pointmanager;
	Beopdatalink::CRealTimeDataAccess* m_dbcon;
	Beopdatalink::CCommonDBAccess* m_dbcon_common;
	Beopdatalink::CCommonDBAccess* m_dbcon_Redundancy;
	Beopdatalink::CLogDBAccess*	m_dbcon_log;
	CBEOPDataAccess *m_pDataAccess;
	//CDTUSender*	m_pdtusender;

	CBEOPDataLinkManager *m_pDataLink;

	CEngineRegister*	m_pEngineRegister;

	BOOL m_bLog;

public:

	HANDLE  m_hThread_WarningScan;
	HANDLE  m_hThread_ReconRedundancyDB;

public:
	//配置参数
	wstring m_strDBFileName;
	DataBaseSetting m_dbset;

	int m_nSendPackageInterval;
	int m_nMainDropInterval;		

	int m_nSystemType;

	bool m_bExitThread;
	bool m_bThreadExitFinished;
	bool m_bDBAcessSuccess; //数据库链接是否建立成功


	CBeopGatewayCoreWrapper *m_pGatewayCoreWrapper;
};