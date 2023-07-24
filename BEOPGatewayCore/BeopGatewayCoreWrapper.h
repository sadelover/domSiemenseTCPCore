#pragma once

#include "../BEOPDataEngineCore/BEOPDataEngineCore.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "../BEOPDataLink/BEOPDataLinkManager.h"
#include "../BEOPDataEngineCore/RedundencyManager.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../BEOPDataEngineCore/BEOPLogicManager.h"

#include "CoreFileTransfer.h"
#include "../BEOPDataPoint/DataPointManager.h"
#include "../BEOPDataEngineCore/LogicDllThread.h"
#include "../DB_BasicIO/BEOPDataBaseInfo.h"
#include "../Tools/Util/UtilsFile.h"


class CBeopGatewayCoreWrapper
{
public:
	CBeopGatewayCoreWrapper(void);
	~CBeopGatewayCoreWrapper(void);

	bool 			Run();
	bool 			Init(wstring strDBFilePath);
	bool 			Exit();
	bool 			Release();
	bool 			Reset();


	bool 			UpdateInputOutput();
	bool 			UpdateS3DBUploaded();
	bool 			UpdateResetCommand();

	////20171018//////////////////////////////////////////////////////////////////////
	bool			UpdateSaveLog();				//更新存储log总开关
	//////////////////////////////////////////////////////////////////////////

	wstring			PreInitFileDirectory_4db();
	void 			SaveAndClearLog();
	void 			PrintLog(const wstring &strLog,bool bSaveLog = true);
	
	bool			SumRestartTime(int nMinute = 40);					//统计时间内自动重启3次以上 再重启需要延迟1分钟
	string			GetHostIP();
	std::wstring	Replace(const wstring& orignStr, const wstring& oldStr, const wstring& newStr, bool& bReplaced);
	bool			OutPutLogString(CString strOut);
	void 			GetHostIPs(vector<string> & IPlist); //多IP的core的获取

public:
	CBeopDataEngineCore *	m_pDataEngineCore;
	CBEOPDataAccess *		m_pDataAccess_Arbiter;
	CRedundencyManager *	m_pRedundencyManager;
	CBEOPLogicManager *		m_pLogicManager;
	CCoreFileTransfer  *	m_pFileTransferManager;
	CCoreFileTransfer *		m_pFileTransferManagerSend;

	HANDLE					m_hEventActiveStandby;
	HANDLE 					m_Backupthreadhandle;
	
	bool 					m_bFirstInit;
	bool 					m_bFitstStart;

	vector<SYSTEMTIME>		g_tLogAll;
	vector<wstring>			g_strLogAll;
	vector<COleDateTime>	m_vecReStartTime;			//自动重启时间

	wstring 				m_strDBFilePath;
	int						m_nDBFileType; //0:s3db, 1:4db;
	wstring  				m_strIniFilePath;
	wstring  				m_strWatchExePath;
	wstring  				m_strLogFilePath;
	SYSTEMTIME  			m_stNow;
	CProcessView			m_PrsV;
	COleDateTime			m_oleStartTime;
	COleDateTime			m_oleRunTime;				//运行时间
	COleDateTime			m_oleCheckEveryHourTime;				//运行时间
	int						m_nDelayWhenStart;
};

