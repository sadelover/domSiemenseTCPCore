#include "StdAfx.h"
#include "BeopGatewayCoreWrapper.h"
#include "DumpFile.h"
#include "../COMdll/ModbusTcp/DogTcpCtrl.h"
#include "../BEOPDataLink/MemoryLink.h"

const int	  g_Restart_Count = 3;

/************************************************************************/
/* 
 0.0.2: 支持西门子与编程接口错开选项 siemense_communication_type
/************************************************************************/
#define CORE_VERSION _T("0.0.2")

//优化数据库压力
#define CORE_VERSION _T("0.0.3")

//加速读取
#define CORE_VERSION _T("0.0.4")


//读取失败处理的log及机制改进
#define CORE_VERSION _T("0.0.5")

//IP不通的LOG也加入了
#define CORE_VERSION _T("0.0.6")

//daveFree机制改变
#define CORE_VERSION _T("0.0.7")

//增加监视某个点的log
#define CORE_VERSION _T("0.0.8")


//修改心跳包的位置
#define CORE_VERSION _T("0.0.9")

//将PLC的ping机制暂时略过
#define CORE_VERSION _T("0.0.10")


//增加PLC引擎的PLC报警
#define CORE_VERSION _T("0.0.11")


//提速
#define CORE_VERSION _T("0.0.12")


//增加点位屏蔽功能
#define CORE_VERSION _T("0.0.13")

//心跳包提前
#define CORE_VERSION _T("0.0.14")


//东莞项目工贝多个组网，发现新引擎如果ping不通时仍然强制连接会导致正常控制器都timeout，于是升级ping机制
#define CORE_VERSION _T("0.0.15")

//代码合并，支持多进程访问
#define CORE_VERSION _T("0.1.1")


//优化了首次挑错读取
#define CORE_VERSION _T("0.1.2")
//增加core_status更新真名
#define CORE_VERSION _T("0.1.3")


//R4类型数据倍率多算一次的bug解决
#define CORE_VERSION _T("0.0.19")

//增加日誌
#define CORE_VERSION _T("0.0.20")

//增加重启键值定义
#define CORE_VERSION _T("0.1.4")


//禁止delete realtimedata_simense_tcp
#define CORE_VERSION _T("0.1.5")


//重大bug修复：指令多引擎分解确保不丢失
#define CORE_VERSION _T("0.1.6")



//增加日誌
#define CORE_VERSION _T("0.1.7")

//1025错误后断开连接,解决了极个别PLC会在-1025 timeout后出现读值成功但是值为很奇怪的异常数据问题
#define CORE_VERSION _T("0.1.8")

//增加版本号version_domSiemenseTCPCore
#define CORE_VERSION _T("0.1.9")

//隐藏主窗口，避免print占用多余IO
#define CORE_VERSION _T("0.1.10")

//ping窗口显示时间放长至2秒，方便运维发现
#define CORE_VERSION _T("0.1.11")


//启动时如果很多PLC不在线，那么ping太多次容易耗时太久，特别是PLC数量较多的项目
#define CORE_VERSION _T("0.1.12")


//【重大升级】解决配置了VW类型整形+倍率时的设定值点，写入后会读取0或者超大数的bug
#define CORE_VERSION _T("0.2.1")

//小合并
#define CORE_VERSION _T("0.2.2")
//1025错误时不退，Free去掉
#define CORE_VERSION _T("0.2.3")
//去除DTU代码+1025错误后重连一次
#define CORE_VERSION _T("0.2.4")
//增加启动时间
#define CORE_VERSION _T("0.2.5")

CBeopGatewayCoreWrapper::CBeopGatewayCoreWrapper(void)
{

	m_pDataEngineCore = NULL;
	m_hEventActiveStandby = NULL;
	m_Backupthreadhandle = NULL;
	 m_pDataAccess_Arbiter = NULL;
	m_pRedundencyManager = NULL;
	m_pLogicManager = NULL;
	m_pFileTransferManager = NULL;
	m_pFileTransferManagerSend = NULL;
	m_bFirstInit = true;
	m_bFitstStart = true;
	GetLocalTime(&m_stNow);
	m_oleStartTime = COleDateTime::GetCurrentTime();
	m_oleRunTime = COleDateTime::GetCurrentTime();
	m_oleCheckEveryHourTime = COleDateTime::GetCurrentTime()-COleDateTimeSpan(100,0,0,0);
	g_tLogAll.clear();
	g_strLogAll.clear();

	m_nDelayWhenStart = 30;

	m_nDBFileType = 0;
}

CBeopGatewayCoreWrapper::~CBeopGatewayCoreWrapper(void)
{
	g_tLogAll.clear();
	g_strLogAll.clear();
}

bool CBeopGatewayCoreWrapper::UpdateInputOutput()
{
	_tprintf(L"GatewayCoreWrapper::UpdateInputOutput Start\r\n");

	if(!m_pDataEngineCore->UpdateInputTable()) //从现场读取信号，存入realtime_input表中
	{
		PrintLog(L"ERROR: UpdateInputTable Failed. Will delay 60 seconds...\r\n",true);
		Sleep(60*1000);
		return false;
	}


	//output
	COleDateTimeSpan oleSpanTime = COleDateTime::GetCurrentTime() - m_oleStartTime;
	if(!m_bFitstStart && m_nDelayWhenStart <=oleSpanTime.GetTotalSeconds())			//启动超过1分钟后再执行策略输出结果
	{
		wstring strValueChanged;
		bool bUpdate = m_pDataEngineCore->GetDataLink()->UpdateOutputParams(strValueChanged);
		if(!bUpdate)
		{
			PrintLog(L"ERROR: Update Output Values Failed.\r\n",false);
		}
		else if(strValueChanged.length()>0)
		{
			PrintLog(strValueChanged.c_str(),false);
		}
	}
	else												//未满1分钟清空策略输出结果
	{
		if(m_nDelayWhenStart >oleSpanTime.GetTotalSeconds())
		{
			CString strPrintTemp;
			strPrintTemp.Format(_T("INFO : domSiemenseTCPCore will enable set value after %d seconds.\r\n"), int(m_nDelayWhenStart-oleSpanTime.GetTotalSeconds()));
			PrintLog(strPrintTemp.GetString(),false);
		}

		m_pDataEngineCore->GetDataLink()->CleatOutputData();
		m_bFitstStart = false;
	}
	return true;
}

bool CBeopGatewayCoreWrapper::Run()
{
	//初始化Dump文件
	DeclareDumpFile();

	wstring wstrFileName;
	Project::Tools::GetModuleExeFileName(wstrFileName);
	CString strModuleFileName = wstrFileName.c_str();
	strModuleFileName = strModuleFileName.Left(strModuleFileName.GetLength()-4);

	HANDLE m_hmutex_instancelock = CreateMutex(NULL, FALSE, strModuleFileName.GetString());
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		CString strErrMsg;
		strErrMsg.Format(L"ERROR: Another %s is running, Start failed.\r\n", strModuleFileName.GetString());
		PrintLog(strErrMsg.GetString(),true);
		Sleep(10000);
		exit(0);
		return false;
	}

	wchar_t wcChar[1024];
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	wsprintf(wcChar, L"---- domSiemenseTCPCore(%s) starting ----\r\n", CORE_VERSION);
	PrintLog(wcChar);

	m_strDBFilePath = PreInitFileDirectory_4db();

	

	while(!Init(m_strDBFilePath))
	{
		if(m_pDataEngineCore && m_pDataEngineCore->m_bExitThread)
		{
			return false;
		}
		Release();
		PrintLog(L"ERROR: Init DB File Failed, Try again after 30 seconds automatic...\r\n",false);
		Sleep(30000);
	}


	m_bFirstInit = false;



	wstring strTimeServer;
	COleDateTime oleNow;
	oleNow = COleDateTime::GetCurrentTime();
	Project::Tools::OleTime_To_String(oleNow,strTimeServer);

	CString strProcessVersionKey = _T("version_") + strModuleFileName;
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(strProcessVersionKey.GetString(), CORE_VERSION);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(strProcessVersionKey.GetString(), CORE_VERSION, strTimeServer);

	m_pDataAccess_Arbiter->UpdateHistoryStatusTable(strModuleFileName);

	int nTimeCount = 15;  //30s更新一次ini文件
	while(!m_pDataEngineCore->m_bExitThread)
	{
		Sleep(500);
		
		GetLocalTime(&m_stNow);
		//获取输入，存入input，执行输出
		if(!m_pDataEngineCore->m_bExitThread)
			UpdateInputOutput();

		oleNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleSpan = oleNow - m_oleRunTime;

		//以下是30秒做一次的事情
		if(oleSpan.GetTotalSeconds() >= 30)
		{
			//更新服务器时间
			wstring strTimeServer;
			Project::Tools::OleTime_To_String(oleNow,strTimeServer);

		}
//		if(!m_bExitCore)
//			UpdateSaveLog();

	}

	return true;

}

bool CBeopGatewayCoreWrapper::UpdateResetCommand()
{
	if(m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"reset")==L"1")
	{
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"resetlogic",L"1");
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"reset", L"0");
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"corestatus", L"busy");
		
		PrintLog(L"INFO : Reset Signal Recved, Restarting...\r\n");
		Reset();
		PrintLog(L"INFO : Core restarted, initializing...\r\n\r\n");
		
		OutPutLogString(_T("domcore.exe manual restart"));
		Init(m_strDBFilePath);
	}

	return true;
}

bool CBeopGatewayCoreWrapper::Reset()
{
	PrintLog(L"INFO : Start Unloading Logic Threads...\r\n",false);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"resetlogic", L"0");

	PrintLog(L"INFO : Exit DataEngineCore...\r\n",false);
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->SetDBAcessState(false);
		m_pDataEngineCore->TerminateScanWarningThread();
		m_pDataEngineCore->TerminateReconnectThread();

		m_pDataEngineCore->Exit();
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		PrintLog(L"Successfully.\r\n",false);
	}
	else
		PrintLog(L"ERROR.\r\n",false);
	

	if(m_pFileTransferManager)
	{
		m_pFileTransferManager->m_pDataAccess = NULL;
		delete(m_pFileTransferManager);
		m_pFileTransferManager = NULL;
	}
	
	if(m_pFileTransferManagerSend)
	{
		m_pFileTransferManagerSend->m_pDataAccess = NULL;
		delete(m_pFileTransferManagerSend);
		m_pFileTransferManagerSend = NULL;

	}

	PrintLog(L"INFO : Disconnect Database for Logic...\r\n",false);
	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
		PrintLog(L"Successfully",false);
	}
	else
		PrintLog(L"ERROR.\r\n",false);

	if(m_pRedundencyManager)
	{
		delete(m_pRedundencyManager);
		m_pRedundencyManager = NULL;

	}

	return true;

}


wstring CBeopGatewayCoreWrapper::PreInitFileDirectory_4db()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	
	wstring strDBFileName;

	wstring cstr4DBDir = Project::Tools::GetParentPath(cstrFile);

	CFileFind s3dbfilefinder;
	wstring strSearchPath = cstr4DBDir + L"\\domdb.4db";
	wstring filename;
	BOOL bfind = s3dbfilefinder.FindFile(strSearchPath.c_str());
	wstring SourcePath, DisPath;
	vector<wstring> strAvailableFileNameList;
	while (bfind)
	{
		bfind = s3dbfilefinder.FindNextFile();
		filename = s3dbfilefinder.GetFileName();

		strAvailableFileNameList.push_back(filename);
	}

	if(strAvailableFileNameList.size()>0)
	{
		strDBFileName = cstr4DBDir + L"\\" + strAvailableFileNameList[0];
		for(int nFileIndex=1; nFileIndex<strAvailableFileNameList.size();nFileIndex++)
		{
			wstring strFileNameToDelete = cstrFile + L"\\" + strAvailableFileNameList[nFileIndex];
			if(!UtilFile::DeleteFile(strFileNameToDelete.c_str()))
			{
				_tprintf(L"ERROR: Delete UnNeccesary DB Files Failed!");
			}
		}
	}

	//log目录
	wstring strLogDir = cstrFile + L"\\log";
	if(!UtilFile::DirectoryExists(strLogDir))
	{
		UtilFile::CreateDirectory(strLogDir);
	}


	return strDBFileName;
}


bool CBeopGatewayCoreWrapper::Init(wstring strDBFilePath)
{
	CString strTemp;

	if(m_pFileTransferManager && m_pFileTransferManager->m_bIsTransmitting)
		return false;

	if(strDBFilePath.length()<=0)
		return false;

	wchar_t wcChar[1024];
	if(m_pDataEngineCore == NULL)
	{
		m_pDataEngineCore = new CBeopDataEngineCore(strDBFilePath);
		m_pDataEngineCore->m_pGatewayCoreWrapper = this;
	}

	if(m_pDataEngineCore->m_strDBFileName.length()<=0)
	{
		PrintLog(L"ERROR: None DB File Found.\r\n");
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	CString strLog;
	strLog.Format(_T("INFO : Reading DBFile:%s\r\n"),m_pDataEngineCore->m_strDBFileName.c_str());
	PrintLog(strLog.GetString(),false);

	//读取服务器数据库
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\config.ini";
	m_strIniFilePath = exepath;
	m_strLogFilePath  = cstrFile + L"\\domwatch.log";

	//init dataaccess
	gDataBaseParam realDBParam;

	if(realDBParam.strDBIP.length()<=0 ||
		realDBParam.strUserName.length()<=0)
	{
		PrintLog(L"ERROR: Read s3db Info(IP,Name) Failed, Please Check the s3db File.\r\n");	//edit 2016-03-11 S3db数据表project_config信息不见了？
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	//更新ini文件 服务器时间
	GetLocalTime(&m_stNow);
	wstring strTimeServer;
	Project::Tools::SysTime_To_String(m_stNow, strTimeServer);

	//wchar_t charServerIP[1024];
	//GetPrivateProfileString(L"core", L"server", L"", charServerIP, 1024, exepath.c_str());
	//wstring wstrServerIP = charServerIP;
	//if(wstrServerIP.length()<=7)
	//{
	//	wstrServerIP = L"localhost";
	//}
	//WritePrivateProfileString(L"core", L"server", wstrServerIP.c_str(), exepath.c_str());

	//realDBParam.strDBIP = Project::Tools::WideCharToAnsi(wstrServerIP.c_str());

	//读取选项
	wchar_t charDelayWhenStart[1024];
	GetPrivateProfileString(L"beopgateway", L"delaywhenstart", L"", charDelayWhenStart, 1024, exepath.c_str());
	wstring wstrDelayWhenStart = charDelayWhenStart;
	if(wstrDelayWhenStart.length()<=0)
	{
		wstrDelayWhenStart = L"15";
	}

	WritePrivateProfileString(L"beopgateway", L"delaywhenstart", wstrDelayWhenStart.c_str(), exepath.c_str());
	 m_nDelayWhenStart = _wtoi(wstrDelayWhenStart.c_str());


	 wchar_t charRootPassword[1024];
	 GetPrivateProfileString(L"core", L"dbpassword", L"", charRootPassword, 1024, exepath.c_str());
	 wstring wstrDBPassword = charRootPassword;
	 if(wstrDBPassword == L"")
		 wstrDBPassword = L"RNB.beop-2013";

	 realDBParam.strDBPsw = Project::Tools::WideCharToAnsi(wstrDBPassword.c_str());


	 wstring wstrDBName;
	 Project::Tools::UTF8ToWideChar(realDBParam.strRealTimeDBName, wstrDBName);
	 wstring wstrServerIP ;
	 Project::Tools::UTF8ToWideChar(realDBParam.strDBIP, wstrServerIP);

	 if(m_pDataAccess_Arbiter == NULL)
		 m_pDataAccess_Arbiter = new CBEOPDataAccess;
	 realDBParam.strDBName = "beopdata";
	 realDBParam.strRealTimeDBName = "beopdata";
	 realDBParam.strDBIP = "127.0.0.1";
	 realDBParam.nPort = 3306;
	 m_pDataAccess_Arbiter->SetDBparam(realDBParam);
	 if(m_pDataAccess_Arbiter->InitConnection(true, true, true)==false)
	 {
		 m_pDataAccess_Arbiter->TerminateAllThreads();
		 delete(m_pDataAccess_Arbiter);
		 m_pDataAccess_Arbiter = NULL;
		 return false;
	 }


	 wstring wstrFileName;
	 Project::Tools::GetModuleExeFileName(wstrFileName);
	 CString strModuleFileName = wstrFileName.c_str();
	 strModuleFileName = strModuleFileName.Left(strModuleFileName.GetLength()-4);
	 m_pDataAccess_Arbiter->UpdateHistoryStatusTable(strModuleFileName);//连接好就更新一次心跳包避免被误杀

	Project::Tools::SysTime_To_OleTime(m_stNow,m_oleStartTime);

	m_pDataEngineCore->Init(false,false,false,-1,-1,false,200,true,false,_T(""),_T(""),-1,false,-1);
	m_pDataEngineCore->m_dbset.strDBIP = "127.0.0.1";
	if(!m_pDataEngineCore->InitDBConnection())
	{
		PrintLog(L"ERROR: InitDBConnection Failed.\r\n");
		return false;
	}


	wstring wstrIgnoreListDef = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"point_ignore_list");
	vector<wstring> wstrIgnoreList;
	if(wstrIgnoreListDef!=_T("0"))
	{
		Project::Tools::SplitStringByChar(wstrIgnoreListDef.c_str(), ',', wstrIgnoreList);

	}

	strTemp.Format(_T("INFO : Reading Ignore Points Count:%d...\r\n"), wstrIgnoreList.size());
	PrintLog(strTemp.GetString(), false);

	PrintLog(L"INFO : Reading Points in DB file...\r\n",false);

	if(strModuleFileName.GetLength()>18)
		strModuleFileName = strModuleFileName.Mid(18);
	else
		strModuleFileName = _T("");
	int nProcessNumber = 0;
	if(strModuleFileName.GetLength()>0)
	{
		nProcessNumber = _wtoi(strModuleFileName.GetString());
	}
	strTemp.Format(_T("Process Running Number: %d\r\n"), nProcessNumber);
	PrintLog(strTemp.GetString());
	if(!m_pDataEngineCore->InitPoints(wstrIgnoreList, nProcessNumber))
	{
		PrintLog(L"ERROR: Reading Points Failed...\r\n");
		return false;
	}

	CDataPointManager* pPointManager = m_pDataEngineCore->GetDataPointManager();
	if(pPointManager->GetAllPointCount() > 0)
	{
		std::ostringstream sqlstream;
		sqlstream << "INFO : Points List:";

		if(pPointManager->GetSiemensPointCount() > 0)
			sqlstream << " SiemensTCP:" << pPointManager->GetSiemensPointCount() << ",";

		string strPointLog = sqlstream.str();
		strPointLog.erase(--strPointLog.end());

		sqlstream.str("");
		sqlstream << strPointLog << ".\r\n";
		PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));
	}
	else
	{
		PrintLog(L"ERROR: Points List Empty.\r\n");
		return false;
	}


	PrintLog(L"INFO : Starting Engine...\r\n");
	if(!m_pDataEngineCore->InitEngine())
	{
		PrintLog(L"ERROR: InitEngine Failed!\r\n");
		return false;
	}

	//清除
	pPointManager->ClearAllPointListExceptOPC();

	//Sleep(800000);
	m_pDataEngineCore->EnableLog(FALSE);

	//init dataenginecore
	m_pDataEngineCore->SetDataAccess(m_pDataAccess_Arbiter);

	m_pDataEngineCore->SetDBAcessState(true);


	m_hEventActiveStandby = CreateEvent(NULL, TRUE, FALSE, NULL);

	
	//m_pDataEngineCore->GetDataLink()->m_bRestartAppWhenIPAliveFromDead;

	COleDateTime tnow = COleDateTime::GetCurrentTime();	
	strLog.Format(_T("INFO : %s domSiemenseTCPCore(%s) started successfully!\r\n"), tnow.Format(L"%Y-%m-%d %H:%M:%S "), CORE_VERSION);
	PrintLog(strLog.GetString());

	return true;
}



bool CBeopGatewayCoreWrapper::SumRestartTime( int nMinute /*= 40*/ )
{
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	if(m_vecReStartTime.size() >= g_Restart_Count)
	{
		m_vecReStartTime.pop_back();
	}
	m_vecReStartTime.push_back(oleNow);
	int nSize = m_vecReStartTime.size();
	if(nSize == g_Restart_Count && g_Restart_Count>0)
	{
		COleDateTimeSpan oleSpan = oleNow - m_vecReStartTime[0];
		if(oleSpan.GetTotalMinutes() <= nMinute)
		{
			m_vecReStartTime.clear();
			Sleep(60*1000);
			return true;
		}
	}
	return false;
}

void CBeopGatewayCoreWrapper::SaveAndClearLog()
{
	if(g_strLogAll.size()<=0)
		return;

	//if(m_pDataAccess_Arbiter)
	//	m_pDataAccess_Arbiter->InsertLog(g_tLogAll, g_strLogAll);

	CString strLogAll;
	for(int i=0;i<g_tLogAll.size();i++)
	{
		if(i>=g_strLogAll.size())
			continue;

		string time_utf8 = Project::Tools::SystemTimeToString(g_tLogAll[i]);

		wstring wstrTime ;
		Project::Tools::UTF8ToWideChar(time_utf8, wstrTime);

		CString strOneItem;
		strOneItem.Format(_T("%s    %s"), wstrTime.c_str(), g_strLogAll[i].c_str());
		strLogAll+=strOneItem;
	}

	try
	{
		wstring strPath;
		GetSysPath(strPath);
		strPath += L"\\log";
		if(Project::Tools::FindOrCreateFolder(strPath))
		{
			COleDateTime oleNow = COleDateTime::GetCurrentTime();
			CString strLogPath;
			strLogPath.Format(_T("%s\\domSiemenseCore_%d_%02d_%02d.log"),strPath.c_str(),oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());


			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );
			//记录Log
			CStdioFile	ff;
			if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			{
				ff.Seek(0,CFile::end);
				ff.WriteString(strLogAll);
				ff.Close();
			}
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 	
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
	

	g_strLogAll.clear();
	g_tLogAll.clear();

}

//bSaveLog: 强制立即写入log文件
void CBeopGatewayCoreWrapper::PrintLog(const wstring &strLog,bool bSaveLog)
{
	_tprintf(strLog.c_str());

	if(bSaveLog==false)
		return;

	SYSTEMTIME st;
	GetLocalTime(&st);
	g_tLogAll.push_back(st);
	g_strLogAll.push_back(strLog);
	if(bSaveLog)
	{
		SaveAndClearLog();
	}
	else if(g_strLogAll.size()>=100)
	{
		// 超过一定条数量，将记录到数据库，清空
		SaveAndClearLog();
	}
	
}

void CBeopGatewayCoreWrapper::GetHostIPs(vector<string> & IPlist)    
{    
    WORD wVersionRequested = MAKEWORD(2, 2);    
    WSADATA wsaData;    
    if (WSAStartup(wVersionRequested, &wsaData) != 0)    
    {  
        return;  
    }    
    char local[255] = {0};    
    gethostname(local, sizeof(local));    
    hostent* ph = gethostbyname(local);    
    if (ph == NULL)  
    {  
        return ;  
    }  
    in_addr addr;    
    HOSTENT *host=gethostbyname(local);    
    string localIP;    
    //当有多个ip时,j就是所有ip的个数   inet_ntoa(*(IN_ADDR*)host->h_addr_list[i] 这里的i就是对应的每个ip   
    for(int i=0;;i++)   
    {  
        memcpy(&addr, ph->h_addr_list[i], sizeof(in_addr)); //    
        localIP=inet_ntoa(addr);   
        IPlist.push_back(localIP);  
        if(host->h_addr_list[i]+host->h_length>=host->h_name)   
        {  
            break; //如果到了最后一条  
        }  
    }    
    WSACleanup();    
    
    return;  
}    

string CBeopGatewayCoreWrapper::GetHostIP()
{
	WORD wVersionRequested = MAKEWORD(2, 2);   
	WSADATA wsaData;   
	if (WSAStartup(wVersionRequested, &wsaData) != 0)   
		return "";   
	char local[255] = {0};   
	gethostname(local, sizeof(local));   
	hostent* ph = gethostbyname(local);   
	if (ph == NULL)   
		return "0";   
	in_addr addr;   
	memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr));   
	std::string localIP;   
	localIP.assign(inet_ntoa(addr));   
	WSACleanup();   
	return localIP;   
}

std::wstring CBeopGatewayCoreWrapper::Replace( const wstring& orignStr, const wstring& oldStr, const wstring& newStr, bool& bReplaced )
{
	size_t pos = 0;
	wstring tempStr = orignStr;
	wstring::size_type newStrLen = newStr.length();
	wstring::size_type oldStrLen = oldStr.length();
	bReplaced = false;
	while(true)
	{
		pos = tempStr.find(oldStr, pos);
		if (pos == wstring::npos) break;

		tempStr.replace(pos, oldStrLen, newStr);        
		pos += newStrLen;
		bReplaced = true;
	}

	return tempStr; 
}


bool CBeopGatewayCoreWrapper::OutPutLogString( CString strOut )
{
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	CString strLog;
	strLog += tnow.Format(_T("%Y-%m-%d %H:%M:%S "));
	strLog += strOut;
	strLog += _T("\n");

	//记录Log
	CStdioFile	ff;
	if(ff.Open(m_strLogFilePath.c_str(),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
	{
		ff.Seek(0,CFile::end);
		ff.WriteString(strLog);
		ff.Close();
		return true;
	}
	return false;
}

bool CBeopGatewayCoreWrapper::Exit()
{
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->m_bExitThread = true;

	}
	if(m_pRedundencyManager)		//退出待机
		m_pRedundencyManager->ActiveSlaveEvent();
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->Exit();
		delete m_pDataEngineCore;
		m_pDataEngineCore = NULL;
	}
	g_tLogAll.clear();
	g_strLogAll.clear();
	return true;
}


bool CBeopGatewayCoreWrapper::UpdateS3DBUploaded()
{
	if(m_pFileTransferManagerSend)
	{
		wstring wstrUpLoadIP = L"";
		wstrUpLoadIP = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"uploadip",wstrUpLoadIP);
		if(wstrUpLoadIP.length() >0)
		{
			m_pFileTransferManagerSend->m_strPath = m_pDataEngineCore->m_strDBFileName.c_str();
			m_pFileTransferManagerSend->m_bFileSent	= false;
			m_pFileTransferManagerSend->StartAsSender(wstrUpLoadIP.c_str(),3458);
			m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"uploadip",L"");
		}
	}
	return true;
}

bool CBeopGatewayCoreWrapper::Release()
{
	if(m_pLogicManager)
	{
		delete m_pLogicManager;
		m_pLogicManager = NULL;
	}

	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->SetDBAcessState(false);
		m_pDataEngineCore->TerminateScanWarningThread();
		m_pDataEngineCore->TerminateReconnectThread();

		m_pDataEngineCore->Exit();
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;

	}

	if(m_pFileTransferManager)
	{
		m_pFileTransferManager->m_pDataAccess = NULL;
		delete(m_pFileTransferManager);
		m_pFileTransferManager = NULL;
	}

	if(m_pFileTransferManagerSend)
	{
		m_pFileTransferManagerSend->m_pDataAccess = NULL;
		delete(m_pFileTransferManagerSend);
		m_pFileTransferManagerSend = NULL;
	}

	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
	}

	if(m_pRedundencyManager)
	{
		delete(m_pRedundencyManager);
		m_pRedundencyManager = NULL;
	}

	return true;
}



bool CBeopGatewayCoreWrapper::UpdateSaveLog()
{
	bool bSaveLog = true;
	if(m_pDataAccess_Arbiter)
	{
		wstring wstrSaveLogTable = L"1"; 
		wstrSaveLogTable = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue_Defalut(L"SaveLogTable",wstrSaveLogTable);
		m_pDataAccess_Arbiter->SetSaveLogFlag(_wtoi(wstrSaveLogTable.c_str()));
		return true;
	}
	return false;
}
