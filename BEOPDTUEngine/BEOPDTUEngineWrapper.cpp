#include "stdafx.h"
#include "BEOPDTUEngineWrapper.h"
#include "DumpFile.h"
#include "Tools/DogTcpCtrl.h"

#define DTU_VERSION _T("V1.0.1 2017.09.12")

CBEOPDTUEngineWrapper::CBEOPDTUEngineWrapper()
	: m_pDataAccess(NULL)
	, m_pBEOPDTUManager(NULL)
	, m_strDBFilePath(L"")
	, m_bExitEngine(false)
{
	m_oleLastDleteTime = COleDateTime::GetCurrentTime();
}

CBEOPDTUEngineWrapper::~CBEOPDTUEngineWrapper()
{
}

bool CBEOPDTUEngineWrapper::Run()
{
	//初始化Dump文件
	DeclareDumpFile();

	//检测保证程序唯一性
	HANDLE m_hmutex_instancelock = CreateMutex(NULL, FALSE, L"BEOPDTUEngine");
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		PrintLog(L"ERROR: Another BEOPDTUEngine is running, Start failed.\r\n");
		Sleep(10000);
		exit(0);
		return false;
	}

	wchar_t wcChar[1024];
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	wsprintf(wcChar, L"---- BEOPDTUEngine(%s) starting ----\r\n", DTU_VERSION);
	PrintLog(wcChar);

	while (!Init())
	{
		if (m_bExitEngine)
		{
			return false;
		}
		Release();
		Sleep(30000);
	}

	tnow = COleDateTime::GetCurrentTime();
	wstring wstrNow;
	Project::Tools::OleTime_To_String(tnow, wstrNow);
	wsprintf(wcChar, L"INFO : BEOPDTUEngine.exe start successful(%s)!\r\n", wstrNow.c_str());
	PrintLog(wcChar);

	//
	m_pDataAccess->WriteCoreDebugItemValue(L"dtuservertime",wstrNow);
	m_pDataAccess->WriteCoreDebugItemValue(L"dtustarttime",wstrNow);		//DTU 启动时间
	m_pDataAccess->WriteCoreDebugItemValue(L"dtuversion",DTU_VERSION);		//写入DTU的版本号

	//初始化Dog
	CDogTcpCtrl::GetInstance(true)->SetDogMode(E_SLAVE_DTUENGINE);

	FeedDog();

	while (!m_bExitEngine)
	{
		//在线活动时间
		WriteActiveTime();

		//喂狗
		if(!m_bExitEngine)
			FeedDog();

		//删除过期文件
		if(!m_bExitEngine)
			DeleteBackupFolderByDate();

		//检测DTU仍然处于工作状态
		if(!m_bExitEngine)
			CheckDTUMangerWorking();

		int nSleep = 30;
		while (!m_bExitEngine && nSleep >= 0)
		{
			nSleep--;
			Sleep(1000);
		}
	}		
	return true;
}

/*
初始化函数:
1:获取数据库配置
2:创建数据库连接
3:创建数据传输类
*/
bool CBEOPDTUEngineWrapper::Init()
{
	wchar_t wcChar[1024];
	
	//读取服务器数据库
	wstring wstrRoot, wstrInitPath;
	Project::Tools::GetSysPath(wstrRoot);
	wstrInitPath = wstrRoot + L"\\domcore.ini";

	//数据库名
	GetPrivateProfileString(L"core", L"defaultdb", L"", wcChar, 1024, wstrInitPath.c_str());
	wstring wstrDBName = wcChar;
	if (wstrDBName == L"" || wstrDBName == L"0")
		wstrDBName = L"beopdata";
	WritePrivateProfileString(L"core", L"defaultdb", wstrDBName.c_str(), wstrInitPath.c_str());

	if (m_pDataAccess == NULL)
	{
		m_pDataAccess = new CDataHandler();
	}
	
	if (m_pDataAccess)
	{
		if (m_pDataAccess->ConnectToDB(L"localhost", L"root", L"RNB.beop-2013", wstrDBName, 3306) == false)
		{
			PrintLog(L"ERROR: Connect MysqlDB Fail...\r\n");
		}
		else
		{
			if (m_pBEOPDTUManager == NULL)
			{
				m_pBEOPDTUManager = new CBEOPDTUManager(m_pDataAccess);
			}
			if (m_pBEOPDTUManager)
			{
				if (m_pBEOPDTUManager->Init() == false)
				{
					PrintLog(L"ERROR: Init DTUHandler Fail...\r\n");
				}
				else
				{
					return true;
				}
			}
		}		
	}
	return false;
}

bool CBEOPDTUEngineWrapper::Exit()
{
	PrintLog(L"INFO : Exit...\r\n");

	m_bExitEngine = true;

	//先退出
	if(m_pBEOPDTUManager)
	{
		m_pBEOPDTUManager->Exit();
		delete m_pBEOPDTUManager;
		m_pBEOPDTUManager = NULL;
	}

	if(m_pDataAccess)
	{
		delete m_pDataAccess;
		m_pDataAccess =NULL;
	}
	return false;
}

bool CBEOPDTUEngineWrapper::Release()
{
	if (m_pBEOPDTUManager)
	{
		m_pBEOPDTUManager->Exit();
		delete m_pBEOPDTUManager;
		m_pBEOPDTUManager = NULL;
	}

	if (m_pDataAccess)
	{
		delete m_pDataAccess;
		m_pDataAccess = NULL;
	}

	return true;
}

void CBEOPDTUEngineWrapper::PrintLog(const wstring & strLog)
{
	_tprintf(strLog.c_str());
}

bool CBEOPDTUEngineWrapper::DeleteBackupFolderByDate()
{
	try
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleLastDleteTime;
		if(oleSpan.GetTotalHours() >= 1)
		{
			if(m_pDataAccess && m_pDataAccess->IsConnected())
			{
				wstring wstrBackupDay = m_pDataAccess->ReadOrCreateCoreDebugItemValue_Defalut(L"backupday",L"90");
				int nSaveDay = _wtoi(wstrBackupDay.c_str());

				wstring strPath,strFolder;
				GetSysPath(strFolder);
				strPath = strFolder;
				strPath += L"\\backup";
				if(Project::Tools::FindOrCreateFolder(strPath))
				{
					COleDateTime oleDeadDate = COleDateTime::GetCurrentTime() - COleDateTimeSpan(nSaveDay,0,0,0);
					CString strDeadFolder;
					strDeadFolder.Format(_T("%04d%02d%02d"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteBackupDirectory(strPath.c_str(),strDeadFolder);			
				}

				//删除err目录下的log
				strPath = strFolder;
				strPath += L"\\log";
				if(Project::Tools::FindOrCreateFolder(strPath))
				{
					COleDateTime oleDeadDate = COleDateTime::GetCurrentTime() - COleDateTimeSpan(nSaveDay,0,0,0);
					//err file
					CString strDeadFolder;
					strDeadFolder.Format(_T("err_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("err_"),_T(".log"),strDeadFolder);

					//opc err file
					CString strOPCErrFile;
					strOPCErrFile.Format(_T("opc_err_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("opc_err_"),_T(".log"),strOPCErrFile);

					//s7udpctrl err file
					CString strS7UdpCtrlErrFile;
					strS7UdpCtrlErrFile.Format(_T("s7udpctrl_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("s7udpctrl_"),_T(".log"),strS7UdpCtrlErrFile);

					//dtucmderr_ err file
					CString strDTUCmdErrFile;
					strDTUCmdErrFile.Format(_T("dtucmderr_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("dtucmderr_"),_T(".log"),strDTUCmdErrFile);
				}

				//删除log目录下的log
				strPath = strFolder;
				strPath += L"\\log";
				if(Project::Tools::FindOrCreateFolder(strPath))
				{
					COleDateTime oleDeadDate = COleDateTime::GetCurrentTime() - COleDateTimeSpan(nSaveDay,0,0,0);
					//opc_debug file
					CString strOPCFolder;
					strOPCFolder.Format(_T("opc_debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("opc_debug_"),_T(".log"),strOPCFolder);

					//bacnet_debug_ file
					CString strBacnetFile;
					strBacnetFile.Format(_T("bacnet_debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("bacnet_debug_"),_T(".log"),strBacnetFile);

					//modbus_debug_ err file
					CString strModbusFile;
					strModbusFile.Format(_T("modbus_debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("modbus_debug_"),_T(".log"),strModbusFile);

					//debug_ err file
					CString strDebugFile;
					strDebugFile.Format(_T("debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("debug_"),_T(".log"),strDebugFile);

					//dtuservercmd_ err file
					CString strDTUCmdFile;
					strDTUCmdFile.Format(_T("dtuservercmd_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("dtuservercmd_"),_T(".log"),strDTUCmdFile);

					//router err file
					CString strRouterErrFile;
					strRouterErrFile.Format(_T("routerstate_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("routerstate_"),_T(".log"),strRouterErrFile);
				}

			}
			m_oleLastDleteTime = COleDateTime::GetCurrentTime();
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
	return true;
}

bool CBEOPDTUEngineWrapper::FindAndDeleteBackupDirectory( CString strDir,CString strDeadFolder )
{
	try
	{
		CFileFind tempFind;
		CString strFind;
		strFind.Format(_T("%s\\*.*"),strDir);
		BOOL IsFinded = tempFind.FindFile(strFind);
		CString strBackupPath;
		while (IsFinded)
		{
			IsFinded = tempFind.FindNextFile();
			if (!tempFind.IsDots()) 
			{
				if (tempFind.IsDirectory())
				{
					CString strDirName = tempFind.GetFileName();
					CString strDirPath = tempFind.GetFilePath();
					if(strDirName <= strDeadFolder)
					{
						DelteFoder(strDirPath);
					}
					else
					{
						FindAndDeleteBackupDirectory(strDirPath,strDeadFolder);
					}
				}
			}
		}

		tempFind.Close();
		return true;
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
	return false;
}

bool CBEOPDTUEngineWrapper::FindAndDeleteErrFile( CString strDir,CString strDeadFileName )
{
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\*.*"),strDir);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strDirName = tempFind.GetFileName();
				CString strDirPath = tempFind.GetFilePath();
				if(strDirName == strDeadFileName)
				{
					DelteFoder(strDirPath);
				}
			}
		}
	}
	tempFind.Close();
	return true;
}

bool CBEOPDTUEngineWrapper::FindAndDeleteErrFile( CString strDir,CString strFilterName,CString strFilterPri,CString strDeadFileName )
{
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\%s*%s"),strDir,strFilterName,strFilterPri);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strDirName = tempFind.GetFileName();
				CString strDirPath = tempFind.GetFilePath();
				if(strDirName <= strDeadFileName)
				{
					DelteFile(strDirPath);
				}
			}
		}
	}
	tempFind.Close();
	return true;
}

bool CBEOPDTUEngineWrapper::DelteFoder( CString strDir )
{
	try
	{
		strDir += '\0';
		SHFILEOPSTRUCT    shFileOp;  
		memset(&shFileOp,0,sizeof(shFileOp));  
		shFileOp.wFunc    = FO_DELETE;  
		shFileOp.pFrom    = strDir;  
		shFileOp.pTo    = NULL;  
		shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;  
		int nResult = ::SHFileOperation(&shFileOp);  
		if(nResult != 0)
		{
			CString strResult;
			strResult.Format(_T("Delete File Error:%d"),nResult);
		}
		return nResult == 0;
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
	return false;
}

bool CBEOPDTUEngineWrapper::DelteFile( CString strFilePath )
{
	try
	{
		strFilePath += '\0';
		SHFILEOPSTRUCT    shFileOp;  
		memset(&shFileOp,0,sizeof(shFileOp));  
		shFileOp.wFunc    = FO_DELETE;  
		shFileOp.pFrom    = strFilePath;  
		shFileOp.pTo    = NULL;  
		shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION|FOF_ALLOWUNDO|FOF_NOCONFIRMMKDIR;  
		int nResult = ::SHFileOperation(&shFileOp);  
		if(nResult != 0)
		{
			CString strResult;
			strResult.Format(_T("Delete files error:%d"),nResult);
		}
		return nResult == 0;
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
	return false;
}

bool CBEOPDTUEngineWrapper::WriteActiveTime()
{
	if(m_pDataAccess && m_pDataAccess->IsConnected())
	{
		SYSTEMTIME sNow;
		GetLocalTime(&sNow);
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		return m_pDataAccess->WriteCoreDebugItemValue(L"dtuservertime",strTime);
	}
	return false;
}

bool CBEOPDTUEngineWrapper::CheckDTUMangerWorking()
{
	if(m_pBEOPDTUManager)
	{
		int nTimeOut = 0;
		if(m_pBEOPDTUManager->CheckTimeOut(nTimeOut))
		{
			CString strLog;
			strLog.Format(_T("INFO : ReInit DTUHandler.\r\n"),nTimeOut);
			PrintLog(strLog.GetString());
			return ReInitDTU();
		}
	}
	return true;
}

bool CBEOPDTUEngineWrapper::ReInitDTU()
{
	//先退出
	if(m_pBEOPDTUManager)
	{
		m_pBEOPDTUManager->Exit();
		delete m_pBEOPDTUManager;
		m_pBEOPDTUManager = NULL;
	}

	//重新创建
	if (m_pBEOPDTUManager == NULL)
	{
		m_pBEOPDTUManager = new CBEOPDTUManager(m_pDataAccess);
	}

	if (m_pBEOPDTUManager)
	{
		if (m_pBEOPDTUManager->Init() == false)
		{
			PrintLog(L"ERROR: ReInit DTUHandler Fail...\r\n");
		}
		else
		{
			return true;

		}
	}
	return false;
}

bool CBEOPDTUEngineWrapper::FeedDog( bool bFirst /*= true*/ )
{
	bool bFeed = true;
	if(bFirst == false)		//需要判断条件
	{
		//判断条件去喂狗
		if(m_pDataAccess)
		{
			//检测dtuservertime
			wstring wstrServerServerTime = m_pDataAccess->ReadOrCreateCoreDebugItemValue_Defalut(L"dtuservertime",L"");
			if(wstrServerServerTime != L"" && wstrServerServerTime != L"0")
			{
				COleDateTime oleServerTime;
				Project::Tools::String_To_OleTime(wstrServerServerTime,oleServerTime);
				COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - oleServerTime;
				if(abs(oleSpan.GetTotalMinutes())>=10)
				{
					bFeed = false;
				}
			}
		}
	}

	if(bFeed)
	{
		return CDogTcpCtrl::GetInstance()->SendActiveSignal();
	}
	return false;
}
