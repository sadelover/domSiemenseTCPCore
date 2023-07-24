#include "StdAfx.h"
#include "ScheduleThread.h"
#include <iostream>

CScheduleThread::CScheduleThread(CBEOPDataAccess* pDataAccess)
	:m_pDataAccess(pDataAccess)
	, m_bCreateTable(false)
	, m_bExitThread(false)
	, m_hSchedulehandle(NULL)
{
	m_mapScheduleTask.clear();
}


CScheduleThread::~CScheduleThread(void)
{
	
}

bool CScheduleThread::Init()
{
	m_bExitThread = false;
	m_hSchedulehandle = (HANDLE) _beginthreadex(NULL, 0, ThreadFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	return true;
}

bool CScheduleThread::Exit()
{
	m_bExitThread = true;
	m_mapScheduleTask.clear();
	if (m_hSchedulehandle != NULL)
	{
		::CloseHandle(m_hSchedulehandle);
		m_hSchedulehandle = NULL;
	}
	return true;
}

bool CScheduleThread::CreateTable()
{
	if(m_pDataAccess && !m_bCreateTable)
	{
		m_bCreateTable = m_pDataAccess->CreateScheduleTableIfNotExist();
	}
	return m_bCreateTable;
}

UINT WINAPI CScheduleThread::ThreadFunc( LPVOID lparam )
{
	CScheduleThread* pScheduleHandle = reinterpret_cast<CScheduleThread*>(lparam);
	if (NULL == pScheduleHandle)
	{
		return 0;
	}
	int nScanUserOperation = 1;		//25s扫描一次
	pScheduleHandle->CreateTable();		//创建表
	Sleep(1000);
	while(!pScheduleHandle->m_bExitThread)
	{
		pScheduleHandle->CheckScehduleResult();

		//5中执行一次
		pScheduleHandle->HandleScheduleTask();
		int nPollSleep = 5;
		while (!pScheduleHandle->m_bExitThread)
		{
			if(nPollSleep <= 0)
			{
				break;
			}
			nPollSleep--;
			Sleep(1000);
		}
	}
	return 1;
}

bool CScheduleThread::HandleScheduleTask()
{
	if(m_pDataAccess)
	{
		COleDateTime oleFrom = COleDateTime::GetCurrentTime();
		CString strTimeFrom,strDate;
		strTimeFrom.Format(_T("%02d:%02d:00"),oleFrom.GetHour(),oleFrom.GetMinute());
		strDate.Format(_T("%04d-%02d-%02d"),oleFrom.GetYear(),oleFrom.GetMonth(),oleFrom.GetDay());
		string strTimeFrom_Ansi,strDate_Ansi;
		strTimeFrom_Ansi = Project::Tools::WideCharToAnsi(strTimeFrom);
		strDate_Ansi = Project::Tools::WideCharToAnsi(strDate);
		vector<Beopdatalink::ScheduleInfo> vecSchedule;
		if(m_pDataAccess->GetScheduleInfo(strDate_Ansi,strTimeFrom_Ansi,strTimeFrom_Ansi,oleFrom.GetDayOfWeek(),vecSchedule))
		{
			for(int i=0; i<vecSchedule.size(); ++i)
			{
				wstring strPointName = Project::Tools::AnsiToWideChar(vecSchedule[i].strPoint.c_str());
				wstring strPointValue = Project::Tools::AnsiToWideChar(vecSchedule[i].strValue.c_str());
				
				vector<wstring> vecPointName;
				Project::Tools::SplitStringByChar(strPointName.c_str(), ',', vecPointName);
				for(int j=0;j<vecPointName.size();j++)
				{
					m_pDataAccess->SetValue(vecPointName[j],strPointValue);
				}

				
				SYSTEMTIME sExecute;
				GetLocalTime(&sExecute);
				vecSchedule[i].sExecute = sExecute;
				m_mapScheduleTask[vecSchedule[i].nID] = vecSchedule[i];
			}
			return true;
		}
	}
	return false;
}

bool CScheduleThread::CheckScehduleResult()
{
	hash_map<int,Beopdatalink::ScheduleInfo>::iterator iter = m_mapScheduleTask.begin();
	while(iter != m_mapScheduleTask.end())
	{
		string strScheduleName = (*iter).second.strName;
		string strPointName = (*iter).second.strPoint;
		string strPointValue = (*iter).second.strValue;
		SYSTEMTIME sExecute = (*iter).second.sExecute;
		int		nLoop = (*iter).second.nLoop;
		int		nID = (*iter).first;
		bool bUpdateSuccess = false;
		if(m_pDataAccess)
		{
			wstring strValue;
			if(m_pDataAccess->GetValue(Project::Tools::AnsiToWideChar(strPointName.c_str()),strValue))
			{
				if(strValue == Project::Tools::AnsiToWideChar(strPointValue.c_str()))
				{
					bUpdateSuccess = true;
				}
			}	
		}

		//写log
		if(m_pDataAccess)
		{
			std::ostringstream sqlstream;
			sqlstream << "Schedule( " << strScheduleName << ") act. Value set:" << strPointValue;
			wstring strValue = Project::Tools::AnsiToWideChar(sqlstream.str().c_str());
			m_pDataAccess->InsertLog(strValue);

			//写操作记录
			m_pDataAccess->InsertOperationRecord(sExecute,strValue,L"Schedule");

			if(nLoop == 0)	//不重复	清除该条记录
			{
				m_pDataAccess->DeleteScheduleInfo(nID);
			}
			else
			{
				COleDateTime oleFrom = COleDateTime::GetCurrentTime();
				CString strDate;
				strDate.Format(_T("%04d-%02d-%02d"),oleFrom.GetYear(),oleFrom.GetMonth(),oleFrom.GetDay());
				string strDate_Ansi = Project::Tools::WideCharToAnsi(strDate);
				m_pDataAccess->UpdateScheduleInfoExecuteDate(nID,strDate_Ansi);
			}
		}

		//移除
		iter = m_mapScheduleTask.erase(iter);
	}
	return true;
}
