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

#define CORE_VERSION _T("V1.0.5")

class CBeopGatewayCoreWrapper
{
public:
	CBeopGatewayCoreWrapper(void);
	~CBeopGatewayCoreWrapper(void);

	bool Run();
	bool Exit();
	bool UpdateResetCommand();
	bool Reset();
	bool Init(wstring strDBFilePath);
	bool Release();
	bool InitCheckMySQLDB(wstring wstrServerIP,wstring wstrDBName=L"beopdata");
	wstring PreInitFileDirectory();
	void RunLogic();
	void SaveAndClearLog();
	void PrintLog(const wstring &strLog,bool bSaveLog = true);
	void UpdateCheckLogic();
	bool ReadLogic();
	std::wstring Replace(const wstring& orignStr, const wstring& oldStr, const wstring& newStr, bool& bReplaced);
	bool OutPutLogString(CString strOut);
	bool FeedDog();				//Î¹¹·

	CBeopDataEngineCore *m_pDataEngineCore;
	CBEOPDataAccess * m_pDataAccess_Arbiter;
	CBEOPLogicManager *m_pLogicManager;
	bool m_bFirstInit;
	bool m_bExitCore;

	vector<SYSTEMTIME> g_tLogAll;
	vector<wstring> g_strLogAll;
	wstring m_strDBFilePath;
	wstring m_strLogFilePath;
	SYSTEMTIME m_stNow;

};

