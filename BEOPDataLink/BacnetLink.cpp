#include "StdAfx.h"
#include "BacnetLink.h"
#include "../COMdll/DebugLog.h"
#include "../COMdll/bacnet/BacnetComm.h"
//#include "vld.h"
CBacnetEngine::CBacnetEngine(CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	dbsession_history)
	:m_datalinker(datalinker)
	,m_dbsession_history(dbsession_history)
{
	//读配置
	wstring wstrReadInterval = L"30";
	wstring wstrReadTypeInterval = L"1";
	wstring wstrReadCmdInterval = L"500";
	wstring wstrReadMode = L"0";	
	wstring wstrPrecision = L"6";
	wstring wstrReadTimeOut = L"2000";
	wstring wstrReadLimit = L"20";
	wstring wstrWritePriority = L"0";
	if(m_dbsession_history)
	{
		wstrReadInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"readinterval",L"30");
		wstrReadTypeInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"readtypeinterval",L"1000");
		wstrReadCmdInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"readcmdinterval",L"500");
		wstrReadMode = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"readbacnetmode",L"0");
		wstrReadTimeOut = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"readtimeout",L"2000");
		wstrReadLimit = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"bacnetreadlimit",L"20");
		wstrPrecision = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"precision",L"2");
		wstrWritePriority = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"writepriority",L"0");

	}
	m_bacnet_session = new CBacnetCtrl(_wtoi(wstrReadInterval.c_str()),_wtoi(wstrReadTypeInterval.c_str()),_wtoi(wstrReadCmdInterval.c_str()),_wtoi(wstrReadMode.c_str()),
		_wtoi(wstrReadTimeOut.c_str()),_wtoi(wstrPrecision.c_str()),_wtoi(wstrReadLimit.c_str()),_wtoi(wstrWritePriority.c_str()) );
}


CBacnetEngine::~CBacnetEngine(void)
{
}

void CBacnetEngine::SetPointList( const std::vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;

}

bool CBacnetEngine::Init()
{
	m_bacnet_session->SetPointList(m_pointlist);
	m_bacnet_session->SetLogSession(m_logsession);
	int nTry = 60;		//最多尝试1分钟   防止刚开机进入时网络ip还没分配到
	bool bMacIsOK = false;
	while(nTry>0)
	{
		if(m_bacnet_session->IsMacInitOK())
		{
			bMacIsOK = true;
			break;
		}
		nTry--;
		Sleep(1000);
	}

	if(!bMacIsOK)
	{
		Project::Tools::PrintLog(L"ERROR: Ip address init failed.\r\n");
		return false;
	}

	//检测udp 47808端口
	string strProgranmName = "";
	wstring strProgramName = L"";
	if(m_bacnet_session->GetProcNameByPort(47808,strProgranmName))
	{
		if(strProgranmName.length() >0)
		{
			strProgramName = Project::Tools::AnsiToWideChar(strProgranmName.c_str());
			CString strLog;
			strLog.Format(_T("ERROR: Bacnet UDP Bacnet port (47808) is occupied(%s).\r\n"),strProgramName.c_str());
			Project::Tools::PrintLog(strLog.GetString());
		}
	}
	//
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		m_dbsession_history->WriteCoreDebugItemValue(L"BacnetPortOccupy",strProgramName);
	}

	//检测环境变量设置IP 是否和本机IP相符
	char* pBacnetINFACE = getenv("BACNET_IFACE");
	if(pBacnetINFACE)
	{
		vector<string> vecLocalIP;
		GetHostIPs(vecLocalIP);
		bool bFindIP = false;
		for(int i=0; i<vecLocalIP.size(); ++i)
		{
			if(strcmp(pBacnetINFACE,vecLocalIP[i].c_str()) == 0)
			{
				bFindIP = true;
				break;
			}
		}
		if(!bFindIP)
		{
			CString strLog;
			strLog.Format(_T("ERROR: BACNET_IFACE(%s) is not Local IP.\r\n"),Project::Tools::AnsiToWideChar(pBacnetINFACE).c_str());
			Project::Tools::PrintLog(strLog.GetString());
		}
		else
		{
			CString strLog;
			strLog.Format(_T("INFO: BACNET_IFACE(%s) found in Local IPList.\r\n"),Project::Tools::AnsiToWideChar(pBacnetINFACE).c_str());
			Project::Tools::PrintLog(strLog.GetString());
		}
	}
	else
	{
		CString strLog;
		strLog.Format(_T("INFO: BACNET_IFACE not found in ENVIRONMENT VARS.\r\n"));
		Project::Tools::PrintLog(strLog.GetString());
	}

	m_bacnet_session->Connect();
	Sleep(1000);
	m_bacnet_session->SortPointList();
	ClearPointList();
	return true;
}

void CBacnetEngine::GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist )
{
	return m_bacnet_session->GetValueSet(valuelist);
}

void CBacnetEngine::SetValue( const wstring& pointname, double value )
{
	return m_bacnet_session->SetValue(pointname, value);
}

void CBacnetEngine::InitDataEntryValue( const wstring& pointname, double value )
{
	return m_bacnet_session->InitDataEntryValue(pointname, value);

}

void CBacnetEngine::ExitThread()
{
	return m_bacnet_session->Exit();
}

bool CBacnetEngine::GetValue( const wstring& pointname, double& value )
{
	if(m_bacnet_session)
		return m_bacnet_session->GetValue(pointname, value);

	return false;
}

void CBacnetEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CBacnetEngine::Exit()
{
	ExitThread();
	return true;
}

string CBacnetEngine::GetExecuteOrderLog()
{
	if(m_bacnet_session)
		return m_bacnet_session->GetExecuteOrderLog();
	return "";
}

void CBacnetEngine::ClearPointList()
{
	m_pointlist.clear();
}

void CBacnetEngine::SetDebug( int nDebug )
{
	if(m_bacnet_session)
		return m_bacnet_session->SetDebug(nDebug);
}

bool CBacnetEngine::SaveBacnetInfo()
{
	hash_map<wstring,wstring> mapIP;
	m_bacnet_session->GetIPInfo(mapIP);
	hash_map<wstring,wstring>::iterator itert1 = mapIP.begin();
	while(itert1 != mapIP.end())
	{
		if(m_dbsession_history && m_dbsession_history->IsConnected())
		{
			wstring strName = (*itert1).first;
			wstring strValue = (*itert1).second;
			m_dbsession_history->WriteCoreDebugItemValue(strName,strValue);
		}
		itert1++;
	}
	return true;
}

void CBacnetEngine::GetHostIPs( vector<string> & IPlist )
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
