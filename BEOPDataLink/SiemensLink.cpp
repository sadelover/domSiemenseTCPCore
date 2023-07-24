#include "StdAfx.h"
#include "SiemensLink.h"

#include "../COMdll/S7UDPCtrl.h"
#include "../LAN_WANComm/PingIcmp.h"

CSiemensLink::CSiemensLink(CBEOPDataLinkManager* datalinker, vector<_SiemensPLCProp> propPLCList,Beopdatalink::CCommonDBAccess*	dbsession_history)
	:m_datalinker(datalinker)
	,m_dbsession_history(dbsession_history)
{

	m_hDataReadThread = NULL;
	m_hDataCheckRetryConnectionThread = NULL;

	m_bExitThread = false;
	
	for(int i=0;i< propPLCList.size();i++)
	{
		CS7UDPCtrl *pCtrl = new CS7UDPCtrl(propPLCList[i].strIP,  propPLCList[i].nSlack, propPLCList[i].nSlot);
		pCtrl->SetHistoryDbCon(m_dbsession_history);
		m_mapS7Ctrl[propPLCList[i].strIP] = pCtrl;

		m_pS7CtrlList.push_back(pCtrl); //删除将在这里维护，golding ,map容易出现一个ip对多个指针，就会内存泄漏
	}

}


CSiemensLink::~CSiemensLink(void)
{
	unsigned int i = 0;
	for(i=0;i<m_pS7CtrlList.size();i++)
	{
		SAFE_DELETE(m_pS7CtrlList[i]);
	}

	m_pS7CtrlList.clear();
}

void CSiemensLink::SetPointList( const std::vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

bool CSiemensLink::Init(CString strRebootPointValueDefine)
{
	map<CString, double> mapRebootPV;
	if(strRebootPointValueDefine.GetLength()>0)
	{
		vector<wstring> wstrArray;
		Project::Tools::SplitStringByChar(strRebootPointValueDefine, '|', wstrArray);
		for(int i=0;i<wstrArray.size();i++)
		{
			vector<wstring> wstrPV;
			Project::Tools::SplitStringByChar(wstrArray[i].c_str(), ',', wstrPV);

			if(wstrPV.size()==2)
			{
				double fValue = _wtof(wstrPV[1].c_str());
				mapRebootPV[wstrPV[0].c_str()] = fValue;
			}
		}
	}

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	int i=0;
	COleDateTime tUpdateTime = COleDateTime::GetCurrentTime();

	wstring wstrFileName;
	Project::Tools::GetModuleExeFileName(wstrFileName);
	CString strModuleFileName = wstrFileName.c_str();
	strModuleFileName = strModuleFileName.Left(strModuleFileName.GetLength()-4);


	while(iter3 != m_mapS7Ctrl.end())
	{
		CS7UDPCtrl *pS7Ctrl = iter3->second;

		vector<CString> strRebootPNameList;
		vector<double> strRebootPValueList;

		std::vector<DataPointEntry> tempSiemenslist;
		for(int j=0;j<m_pointlist.size();j++)
		{
			if(m_pointlist[j].GetParam(3) == pS7Ctrl->GetIP())
			{
				tempSiemenslist.push_back(m_pointlist[j]);

				map<CString, double>::iterator itr = mapRebootPV.find(m_pointlist[j].GetShortName().c_str());
				if(itr!= mapRebootPV.end())
				{
					strRebootPNameList.push_back(itr->first);
					strRebootPValueList.push_back(itr->second);
				}
			}
		}


		pS7Ctrl->SetRebootPointNameValueCondition(strRebootPNameList, strRebootPValueList);
		pS7Ctrl->SetPointList(tempSiemenslist);
		pS7Ctrl->SetLogSession(m_logsession);
		pS7Ctrl->SetIndex(i);
		pS7Ctrl->Init();

		COleDateTime tNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan tspan = tNow -  tUpdateTime;
		if(tspan.GetTotalSeconds()>=60)
		{
			m_dbsession_history->UpdateHistoryStatusTable(strModuleFileName);
			tUpdateTime = tNow;
		}

		iter3++;
		i++;
	}


	m_hDataReadThread = (HANDLE) _beginthreadex(NULL, 0, ThreadReadAllPLCData, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	if (!m_hDataReadThread)
	{
		return false;
	}

}



void	CSiemensLink::TerminateReadThread()
{
	m_bExitThread = true;
	WaitForSingleObject(m_hDataReadThread, INFINITE);
	WaitForSingleObject(m_hDataCheckRetryConnectionThread, INFINITE);
}


UINT WINAPI CSiemensLink::ThreadReadAllPLCData(LPVOID lparam)
{
	CSiemensLink* pthis = (CSiemensLink*) lparam;
	COleDateTime tupdate = COleDateTime::GetCurrentTime();


	wstring wstrFileName;
	Project::Tools::GetModuleExeFileName(wstrFileName);
	CString strModuleFileName = wstrFileName.c_str();
	strModuleFileName = strModuleFileName.Left(strModuleFileName.GetLength()-4);


	if (pthis != NULL)
	{

		while (!pthis->m_bExitThread)
		{
			TimeCounter tc;
			tc.Start();
			//_tprintf(_T("ThreadReadAllPLCData Read Start\r\n"));
			map<wstring,CS7UDPCtrl*>::iterator iter3 = pthis->m_mapS7Ctrl.begin();
			while(iter3 != pthis->m_mapS7Ctrl.end())
			{
				CS7UDPCtrl *pS7Ctrl = iter3->second;
				Project::Tools::Scoped_Lock<Mutex>	scopelock(pthis->m_lock);

				
				if(!pS7Ctrl->ReadDataOnce())
				{
					
				}
				else
				{

				}
				iter3++;
			}

			//CString strTemp;
			//strTemp.Format(_T("ThreadReadAllPLCData All End(Cost:%.2f s)\r\n"), double(tc.Now()/1000.0));
			//_tprintf(strTemp.GetString());


			COleDateTime tnow = COleDateTime::GetCurrentTime();
			if((tnow-tupdate).GetTotalSeconds()>=30.0)
			{
				if(pthis->m_dbsession_history)
					pthis->m_dbsession_history->UpdateHistoryStatusTable(strModuleFileName);

				tupdate = tnow;
			}

			Sleep(1000);


		}
	}

	return 0;
}

UINT WINAPI CSiemensLink::ThreadCheckAndRetryConnection(LPVOID lparam)
{
	CSiemensLink* pthis = (CSiemensLink*) lparam;

	
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			//读Simens 失败次数重连间隔
			int nReadErrConnectCount = pthis->GetErrrConnectCount();
			map<wstring,CS7UDPCtrl*>::iterator iter3 = pthis->m_mapS7Ctrl.begin();
			while(iter3 != pthis->m_mapS7Ctrl.end())
			{
				CS7UDPCtrl *pS7Ctrl = iter3->second;
				bool bNeedCheck = false;
				{//必须有大括号，好让锁解掉
					Project::Tools::Scoped_Lock<Mutex>	scopelock(pthis->m_lock);
					bNeedCheck = pS7Ctrl->GetErrCount()>=nReadErrConnectCount || !pS7Ctrl->GetConnectSuccess();
				}
				
				if(bNeedCheck)
				{
					string strIP;
					Project::Tools::WideCharToUtf8(pS7Ctrl->GetIP(), strIP);
					CPingIcmp ping;
					bool bPinged = ping.PingV2(strIP);
					if(bPinged)
					{
						if(ping.test_tcp_ip_port(strIP, 102))
						{
							CString strLog = _T("PLC Read Failed, but IP Ping and port Success, try connect\r\n");
							pS7Ctrl->PrintLog(strLog.GetString());

							Project::Tools::Scoped_Lock<Mutex>	scopelock(pthis->m_lock);
							pS7Ctrl->ExitPLCConnection();
							pS7Ctrl->InitPLCConnection();
							pS7Ctrl->ClrPLCError();
						}
						
					}
					else
					{
						CString strLog;
						strLog.Format(_T("PLC IP %s Ping Failed, Please Check IP With Ping.\r\n"), pS7Ctrl->GetIP().c_str());
						pS7Ctrl->PrintLog(strLog.GetString());
						if(pS7Ctrl->GetConnectSuccess())
						{
							if(!ping.test_tcp_ip_port(strIP, 102))
							{
								strLog.Format(_T("PLC IP %s Port Check Failed After working some time.\r\n"), pS7Ctrl->GetIP().c_str());
								pS7Ctrl->PrintLog(strLog.GetString());

								Project::Tools::Scoped_Lock<Mutex>	scopelock(pthis->m_lock);
								pS7Ctrl->ExitPLCConnection();
								pS7Ctrl->SetConnectSuccess(false);
							}
						}
						Sleep(1000);
					}
					
				}

				iter3++;
			}

			
			Sleep(10000);
		}
	}

	return 0;
}


bool CSiemensLink::Exit()
{
	TerminateReadThread();

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		if(iter3->second)
		{
			iter3->second->Exit();
		}

		iter3++;
	}

	m_mapS7Ctrl.clear();

	return true;
}

void CSiemensLink::GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist )
{
	_tprintf(L"SiemensLink::GetValueSet All Start, Waiting Lock\r\n");
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	TimeCounter tc;
	tc.Start();
	_tprintf(L"SiemensLink::GetValueSet All Start\r\n");

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		iter3->second->GetValueSet(valuelist);

		iter3++;
	}
	CString strTemp;
	strTemp.Format(_T("SiemensLink::GetValueSet All End(Cost:%.2f s)\r\n"), double(tc.Now()/1000.0));
	_tprintf(strTemp.GetString());
}

bool CSiemensLink::SetValue( const wstring& pointname, double fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		bool bSetValueSuccess =  iter3->second->SetValue(pointname, fValue);

		if(bSetValueSuccess)
			return true;

		iter3++;
	}

	return false;
}

bool CSiemensLink::SetValue( const wstring& pointname, float fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		bool bSetValueSuccess =  iter3->second->SetValue(pointname, fValue);

		if(bSetValueSuccess)
		{

			return true;
		}

		iter3++;
	}

	return false;
}

void CSiemensLink::InitDataEntryValue( const wstring& pointname, double fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		iter3->second->InitDataEntryValue(pointname, fValue);

		iter3++;
	}


}

bool CSiemensLink::GetValue( const wstring& pointname, double& fValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);

	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		if(iter3->second->GetValue(pointname, fValue))
			return true;

		iter3++;
	}

	return false;
}

void CSiemensLink::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

vector<string> CSiemensLink::GetExecuteOrderLog()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	vector<string> vecLog;
	map<wstring,CS7UDPCtrl*>::iterator iter3 = m_mapS7Ctrl.begin();
	while(iter3 != m_mapS7Ctrl.end())
	{
		string strLog = iter3->second->GetExecuteOrderLog();
		if(strLog.length()>0)
			vecLog.push_back(strLog);
		iter3++;
	}
	return vecLog;
}

int CSiemensLink::GetErrrConnectCount()
{
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		wstring wstrReadErrConnectCount = m_dbsession_history->ReadOrCreateCoreDebugItemValue(L"errrconnectcount");
		if(wstrReadErrConnectCount == L"" || wstrReadErrConnectCount == L"0")
		{
			wstrReadErrConnectCount = L"30";
			m_dbsession_history->WriteCoreDebugItemValue(L"errrconnectcount",wstrReadErrConnectCount);
		}
		return _wtoi(wstrReadErrConnectCount.c_str());
	}
	return 30;
}

