#pragma once
#include "StdAfx.h"
#include "SqliteCtrl.h"
#include "atlstr.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/EngineInfoDefine.h"
#include <iostream>
#include <sstream>


CSqliteCtrl::CSqliteCtrl( void )
	: m_hupdatethread(NULL)
	, m_hupdatepointthread(NULL)
	, m_bExitUpdateThread(FALSE)
	, m_strS3dbPath("")
	, m_strS3dbPsw("")
	, m_strS3dbTName("")
	, m_strS3dbNameColumn("")
	, m_strS3dbValueColumn("")
	, m_nS3dbInterval(60)
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_strErrInfo = "";
	m_nEngineIndex = 0;
}

CSqliteCtrl::~CSqliteCtrl( void )
{
	if (m_hupdatethread != NULL)
	{
		::CloseHandle(m_hupdatethread);
		m_hupdatethread = NULL;
	}

	if(m_hupdatepointthread)
	{
		::CloseHandle(m_hupdatepointthread);
		m_hupdatepointthread = NULL;
	}
}

bool CSqliteCtrl::Init()
{
	try
	{
		if (m_pointlist.empty()){
			return false;
		}
		m_runTimeDataMap.clear();

		wstring strFilePath = m_pointlist[0].GetParam(1);
		Project::Tools::WideCharToUtf8(strFilePath, m_strS3dbPath);
		Project::Tools::WideCharToUtf8(m_pointlist[0].GetParam(2), m_strS3dbPsw);
		Project::Tools::WideCharToUtf8(m_pointlist[0].GetParam(3), m_strS3dbTName);
		Project::Tools::WideCharToUtf8(m_pointlist[0].GetParam(4), m_strS3dbNameColumn);
		Project::Tools::WideCharToUtf8(m_pointlist[0].GetParam(5), m_strS3dbValueColumn);
		if(m_pointlist[0].GetParam(7).length() >0)
			m_nS3dbInterval = _wtoi(m_pointlist[0].GetParam(7).c_str());

		if(m_strS3dbPath.length() <= 0 || m_strS3dbTName.length() <= 0 || m_strS3dbNameColumn.length() <= 0 || m_strS3dbValueColumn.length() <= 0)
		{
			CString strLog;
			strLog.Format(_T("ERROR: Sqlite Engine(%d) Param is not valid.\r\n"),m_nEngineIndex);
			PrintLog(strLog.GetString(),false);
			return false;
		}

		//查找文件是否存在

		if(!Project::Tools::FindFileExist(strFilePath))
		{
			CString strLog;
			strLog.Format(_T("ERROR: Sqlite Engine(%d) File is not exist.\r\n"),m_nEngineIndex);
			PrintLog(strLog.GetString(),false);
			return false;
		}

		m_hupdatethread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdateValueFunc, this, NORMAL_PRIORITY_CLASS, NULL);
		if (!m_hupdatethread){
			return false;
		}

		m_hupdatepointthread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdatePointFunc, this, NORMAL_PRIORITY_CLASS, NULL);
		if (!m_hupdatepointthread){
			return false;
		}
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

UINT WINAPI CSqliteCtrl::ThreadUpdateValueFunc( LPVOID lparam )
{
	CSqliteCtrl* pthis = (CSqliteCtrl*)lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitUpdateThread)
		{
			pthis->UpdateValue();
			int nSleep = pthis->m_nS3dbInterval;
			while (!pthis->m_bExitUpdateThread)
			{
				Sleep(1000);
				nSleep--;
				if(nSleep <= 0)
				{
					break;
				}
			}
		}
	}
	return 0;
}

void CSqliteCtrl::TerminateUpdateValueThread()
{
	::TerminateThread(m_hupdatethread, 0);
	WaitForSingleObject(m_hupdatethread, INFINITE);
}

void CSqliteCtrl::UpdateValue()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	try
	{
		m_oleUpdateTime = COleDateTime::GetCurrentTime();

		CSqliteAcess access(m_strS3dbPath,m_strS3dbPsw);
		if(access.GetOpenState())
		{
			std::ostringstream sqlstream_select;
			sqlstream_select << "select " << m_strS3dbNameColumn << "," << m_strS3dbValueColumn << " from " << m_strS3dbTName;
			string sql_ = sqlstream_select.str();
			char *ex_sql = const_cast<char*>(sql_.c_str());
			int re = access.SqlQuery(ex_sql);
			string str;
			while(true)
			{
				if(SQLITE_ROW != access.SqlNext())
				{
					break;
				}
				wstring strPointName,strPointValue; 
				if (access.getColumn_Text(0) !=NULL)
				{
					strPointName = Project::Tools::UTF8ToWideChar(access.getColumn_Text(0));
				}
				if (access.getColumn_Text(1) !=NULL)
				{
					strPointValue = Project::Tools::UTF8ToWideChar(access.getColumn_Text(1));
				}
				m_runTimeDataMap[strPointName]  = strPointValue;
			}
			access.SqlFinalize();
			access.CloseDataBase();
		}
		else
		{
			CString strLog;
			strLog.Format(_T("ERROR: Sqlite Engine(%d) Query Failed.\r\n"),m_nEngineIndex);
			PrintLog(strLog.GetString(),false);
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
}

void CSqliteCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CSqliteCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	try
	{
		for (unsigned int i = 0; i < m_pointlist.size(); i++){
			const DataPointEntry&  modbusentry = m_pointlist[i];
			valuelist.push_back(make_pair(modbusentry.GetShortName(), modbusentry.GetSValue()));
		}

		if(m_pointlist.size() >0 )
		{
			wstring strUpdateTime;
			Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

			CString strName;
			strName.Format(_T("%s%d"),TIME_UPDATE_SQLITE,m_nEngineIndex);
			valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
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
}

UINT WINAPI CSqliteCtrl::ThreadUpdatePointFunc( LPVOID lparam )
{
	CSqliteCtrl* pthis = (CSqliteCtrl*)lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitUpdateThread)
		{
			pthis->UpdatePoint();
			Sleep(2000);
		}
	}
	return 0;
}

void CSqliteCtrl::TerminateUpdatePointThread()
{
	::TerminateThread(m_hupdatepointthread, 0);
	WaitForSingleObject(m_hupdatepointthread, INFINITE);
}

void CSqliteCtrl::UpdatePoint()
{
	try
	{
		for (unsigned int i = 0; i < m_pointlist.size(); i++)
		{
			const DataPointEntry&  mysqlentry = m_pointlist[i];
			map<wstring,wstring>::iterator iter = m_runTimeDataMap.find(mysqlentry.GetParam(6));
			if(iter != m_runTimeDataMap.end())
			{
				wstring dValue = iter->second;
				m_pointlist[i].SetSValue(dValue);
			}
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
}

void CSqliteCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CSqliteCtrl::Exit()
{
	m_bExitUpdateThread = true;
	
	if(m_hupdatethread)
		WaitForSingleObject(m_hupdatethread,INFINITE);

	if(m_hupdatepointthread)
		WaitForSingleObject(m_hupdatepointthread,INFINITE);

	return true;
}

bool CSqliteCtrl::SetValue( wstring strName, wstring strValue )
{
	try
	{
		bool bFindPoint = false;
		DataPointEntry  sqlitepoint;
		for (unsigned int i = 0; i < m_pointlist.size(); i++){
			sqlitepoint = m_pointlist[i];
			if(sqlitepoint.GetShortName() == strName)
			{
				bFindPoint = true;		
				break;
			}
		}

		if(!bFindPoint)
		{
			return false;
		}

		bool bResult = false;
		wstring strOldValue = sqlitepoint.GetSValue();
		if(strOldValue == strValue)
			bResult  = true;

		Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
		std::ostringstream sqlstream;
		string strValue_Utf8,strName_Utf8;
		Project::Tools::WideCharToUtf8(strValue,strValue_Utf8);
		Project::Tools::WideCharToUtf8(sqlitepoint.GetParam(6),strName_Utf8);
		if(!bResult)
		{
			CSqliteAcess access(m_strS3dbPath,m_strS3dbPsw);
			std::ostringstream sqlstream_update;
			sqlstream_update << "update " << m_strS3dbTName << " set " << m_strS3dbValueColumn << " ='" << strValue_Utf8 << "' where " << m_strS3dbNameColumn <<  "= '" << strName_Utf8 << "';";
			string sql_ = sqlstream_update.str();
			if (SQLITE_OK == access.SqlExe(sql_.c_str()))
			{
				bResult = true;
			}
			access.CloseDataBase();
		}

		if(bResult)
		{
			//更新本内存
			m_runTimeDataMap[strName] = strValue;
		}
		return bResult;
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

void CSqliteCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

void CSqliteCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
