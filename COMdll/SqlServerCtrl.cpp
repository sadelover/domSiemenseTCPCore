#pragma once
#include "StdAfx.h"
#include "SqlServerCtrl.h"
#include "atlstr.h"
#include "../LAN_WANComm/PingIcmp.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/EngineInfoDefine.h"
#include <iostream>
#include <sstream>

const string BEOP_OUTPUT = "realtimedata_output";
const string BEOP_OPERATION = "operation_record";

CSqlServerCtrl::CSqlServerCtrl( void )
	: m_hupdatethread(NULL)
	, m_hupdatepointthread(NULL)
	, m_bExitUpdateThread(FALSE)
	, m_nTableType(0)
	, m_strLocalIP("")
	, m_pConnection(NULL)
	, m_pRecordSet(NULL)
{
	m_vecTableName.clear();
	m_vecCrossTableName.clear();
	m_vecLongTableName.clear();
	m_vecHLongTableName.clear();
	m_vecHCrossTableName.clear();
	m_HCrossTableDataMap.clear();
	m_runTimeDataMap.clear();
	m_runTimeDataMap1.clear();
	m_TableColumn0.clear();
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_strErrInfo = "";
	m_nEngineIndex = 0;

	//初始化COM环境
	::CoInitialize(NULL);
}

CSqlServerCtrl::~CSqlServerCtrl( void )
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

bool CSqlServerCtrl::Init()
{
	if (m_pointlist.empty()){
		return false;
	}

	m_strLocalIP = GetHostIP();

	for(int i=0; i<m_pointlist.size(); i++)
	{
		wstring strName = m_pointlist[i].GetParam(7);
		m_nTableType = _wtoi(m_pointlist[i].GetParam(9).c_str());
		if(m_nTableType == 1)
		{
			int nPos = IsExistInVec(strName,m_vecCrossTableName);
			if(nPos==m_vecCrossTableName.size())	//不存在
			{
				m_vecCrossTableName.push_back(strName);
			}
		}
		else if(m_nTableType == 2)
		{
			int nPos = IsExistInVec(strName,m_vecHLongTableName);
			if(nPos==m_vecHLongTableName.size())	//不存在
			{
				m_vecHLongTableName.push_back(strName);
			}
		}
		else if(m_nTableType == 3)
		{
			wstring strOrderColumn = m_pointlist[i].GetParam(10);
			wstring strFilterColumn = m_pointlist[i].GetParam(2);
			int nPos = IsExistInVec(strName,strOrderColumn,strFilterColumn,m_vecHCrossTableName);
			if(nPos==m_vecHCrossTableName.size())	//不存在
			{
				vector<wstring> vecFilter;
				Project::Tools::SplitStringByChar(strFilterColumn.c_str(),L'/',vecFilter);
				if(vecFilter.size() == 2)
				{
					_MysqlCrossTableUnit entry;
					entry.strFilterColumn = vecFilter[0];
					entry.nFilterColumnIndex = _wtoi(vecFilter[1].c_str());
					entry.strTableName = strName;
					entry.strOrderColumn = strOrderColumn;

					vector<wstring> vecFilter_;
					wstring strFilterValue = m_pointlist[i].GetParam(1);
					Project::Tools::SplitStringByChar(strFilterValue.c_str(),L'#',vecFilter_);
					if(vecFilter_.size() == 2)
						entry.vecFilter.push_back(vecFilter_[0]);
					m_vecHCrossTableName.push_back(entry);
				}
			}
			else		//存在
			{
				_MysqlCrossTableUnit& entry = m_vecHCrossTableName[nPos];
				vector<wstring> vecFilter_;
				wstring strFilterValue = m_pointlist[i].GetParam(1);
				Project::Tools::SplitStringByChar(strFilterValue.c_str(),L'#',vecFilter_);
				if(vecFilter_.size() == 2)
				{
					if(IsExistInVec(vecFilter_[0],entry.vecFilter) == entry.vecFilter.size())		//不存在放入
					{
						entry.vecFilter.push_back(vecFilter_[0]);
					}
				}
			}
		}
		else
		{
			int nPos = IsExistInVec(strName,m_vecLongTableName);
			if(nPos==m_vecLongTableName.size())	//不存在
			{
				m_vecLongTableName.push_back(strName);
			}
		}
	}
	m_strhost = m_pointlist[0].GetParam(3);
	m_strusername = m_pointlist[0].GetParam(4);
	m_strpassword = m_pointlist[0].GetParam(5);
	m_strdatabase = m_pointlist[0].GetParam(8);
	m_nPort = _wtoi(m_pointlist[0].GetParam(6).c_str());
	
	bool bInitOK = true;
	if(ConnectSqlDB(m_strhost,m_strusername,m_strpassword,m_strdatabase))
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("connect fail");
		bInitOK = false;
	}
	m_hupdatethread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdateValueFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hupdatepointthread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdatePointFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	return bInitOK;
}

UINT WINAPI CSqlServerCtrl::ThreadUpdateValueFunc( LPVOID lparam )
{
	CSqlServerCtrl* pthis = (CSqlServerCtrl*)lparam;
	if (pthis != NULL)
	{
		pthis->QueryTableColumn0();
		while(!pthis->m_bExitUpdateThread)
		{
			pthis->UpdateValue();
			int nTimeCount = 5;
			while(!pthis->m_bExitUpdateThread && nTimeCount>=0)
			{
				nTimeCount--;
				Sleep(1000);
			}
		}
	}
	return 0;
}

void CSqlServerCtrl::TerminateUpdateValueThread()
{
	::TerminateThread(m_hupdatethread, 0);
	WaitForSingleObject(m_hupdatethread, INFINITE);
}

void CSqlServerCtrl::UpdateValue()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);

	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	//查纵表
	for(int i= 0; i<m_vecLongTableName.size(); i++)
	{
		if(IsConnected())
		{
			CString strSql;
			strSql.Format(_T("select * from %s;"),m_vecLongTableName[i].c_str());
			
			try
			{
				if(m_pConnection->GetState() != adStateClosed)
					m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
			}
			catch (_com_error e)
			{
			}

			if(m_pRecordSet)
			{
				_variant_t vsnum;
				try
				{
					while(!m_pRecordSet->ENDOFFILE)
					{	
						string strName = "";
						string strValue = "";
						vsnum = m_pRecordSet->GetCollect(_variant_t((long)1));	
						if(vsnum.vt!=VT_NULL)
						{
							strName = (char*)(_bstr_t)vsnum;
						}

						vsnum = m_pRecordSet->GetCollect(_variant_t((long)2));	
						if(vsnum.vt!=VT_NULL)
						{
							strValue = (char*)(_bstr_t)vsnum;
						}

						m_runTimeDataMap[Project::Tools::UTF8ToWideChar(strName.c_str())]  = Project::Tools::UTF8ToWideChar(strValue.c_str());
					}
				}
				catch (_com_error e)
				{
					
				}
			}
			Sleep(10);
		}
	}

	//查横表
	std::ostringstream sqlstream;
	for(int i= 0; i<m_vecCrossTableName.size(); i++)
	{
		if(IsConnected())
		{
			//先拿表头
			string strTableName_Utf8,strDBName_Utf8;
			Project::Tools::WideCharToUtf8(m_vecCrossTableName[i],strTableName_Utf8);
			Project::Tools::WideCharToUtf8(m_strdatabase,strDBName_Utf8);

			sqlstream.str("");
			sqlstream << "SELECT COLUMN_NAME FROM information_schema.COLUMNS c WHERE c.TABLE_NAME = '"
				<< strTableName_Utf8 << "' and c.TABLE_SCHEMA='" << strDBName_Utf8 << "';";

			CString strSql;
			strSql = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
			vector<wstring> vecColumn;
			try
			{
				if(m_pConnection->GetState() != adStateClosed)
					m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
			}
			catch (_com_error e)
			{
			}

			if(m_pRecordSet)
			{
				_variant_t vsnum;
				try
				{
					while(!m_pRecordSet->ENDOFFILE)
					{	
						string strName = "";
						vsnum = m_pRecordSet->GetCollect(_variant_t((long)0));	
						if(vsnum.vt!=VT_NULL)
						{
							strName = (char*)(_bstr_t)vsnum;
						}
						vecColumn.push_back(Project::Tools::UTF8ToWideChar(strName.c_str()));
					}
				}
				catch (_com_error e)
				{

				}
			}
			Sleep(10);

			if(vecColumn.size()>0)
			{
				string strColum0_Utf8;
				Project::Tools::WideCharToUtf8(vecColumn[0],strColum0_Utf8);
				sqlstream.str("");
				sqlstream << "SELECT * from " << strTableName_Utf8 << " order by " << strColum0_Utf8 << " desc limit 0,1;";

				strSql = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
				vector<wstring> vecColumn;
				try
				{
					if(m_pConnection->GetState() != adStateClosed)
						m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
				}
				catch (_com_error e)
				{
				}

				if(m_pRecordSet)
				{
					_variant_t vsnum;
					try
					{
						while(!m_pRecordSet->ENDOFFILE)
						{	
							for(int i=0; i<vecColumn.size(); ++i)
							{
								string strName = "";
								vsnum = m_pRecordSet->GetCollect(_variant_t((long)i));	
								if(vsnum.vt!=VT_NULL)
								{
									strName = (char*)(_bstr_t)vsnum;
								}
								m_runTimeDataMap[vecColumn[i]] = Project::Tools::UTF8ToWideChar(strName.c_str());
							}
						}
					}
					catch (_com_error e)
					{

					}
				}
				Sleep(10);
			}
		}
	}

	//查历史纵表
	for(int i= 0; i<m_vecHLongTableName.size(); i++)
	{
		wstring strTableName = m_vecHLongTableName[i];
		map<wstring,wstring>::iterator iter = m_TableColumn0.find(strTableName);
		if(iter != m_TableColumn0.end())
		{
			wstring strColumn0 = iter->second;
			if(IsConnected())
			{
				//先拿表头
				string strTableName_Utf8,strDBName_Utf8,strColumn0_Utf8,strTime_Utf8;
				Project::Tools::WideCharToUtf8(strTableName,strTableName_Utf8);
				Project::Tools::WideCharToUtf8(m_strdatabase,strDBName_Utf8);
				Project::Tools::WideCharToUtf8(strColumn0,strColumn0_Utf8);

				sqlstream.str("");
				sqlstream << "select * from " << strTableName_Utf8 << " order by " << strColumn0_Utf8 << " desc limit 0,1";

				CString strSql;
				strSql = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
				try
				{
					if(m_pConnection->GetState() != adStateClosed)
						m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
				}
				catch (_com_error e)
				{
				}

				if(m_pRecordSet)
				{
					_variant_t vsnum;
					try
					{
						while(!m_pRecordSet->ENDOFFILE)
						{	
							vsnum = m_pRecordSet->GetCollect(_variant_t((long)0));	
							if(vsnum.vt!=VT_NULL)
							{
								strTime_Utf8 = (char*)(_bstr_t)vsnum;
							}
						}
					}
					catch (_com_error e)
					{

					}
				}
				Sleep(10);

				sqlstream.str("");
				sqlstream << "select * from " << strTableName_Utf8 << " where " << strColumn0_Utf8 << "='"
					<< strTime_Utf8 << "';";
				strSql = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
				vector<wstring> vecColumn;
				try
				{
					if(m_pConnection->GetState() != adStateClosed)
						m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
				}
				catch (_com_error e)
				{
				}

				if(m_pRecordSet)
				{
					_variant_t vsnum;
					try
					{
						while(!m_pRecordSet->ENDOFFILE)
						{	
							string strName = "";
							string strValue = "";
							vsnum = m_pRecordSet->GetCollect(_variant_t((long)1));	
							if(vsnum.vt!=VT_NULL)
							{
								strName = (char*)(_bstr_t)vsnum;
							}

							vsnum = m_pRecordSet->GetCollect(_variant_t((long)2));	
							if(vsnum.vt!=VT_NULL)
							{
								strValue = (char*)(_bstr_t)vsnum;
							}

							m_runTimeDataMap1[strTableName][Project::Tools::UTF8ToWideChar(strName.c_str())] = Project::Tools::UTF8ToWideChar(strValue.c_str());
						}
					}
					catch (_com_error e)
					{

					}
				}
				Sleep(10);
			}
		}
	}

	//查历史横表
	for(int i= 0; i<m_vecHCrossTableName.size(); i++)
	{
		string strFilterColumn_Utf8,strOrderColumn_Utf8,strTName_Utf8;
		wstring wstrTableName = m_vecHCrossTableName[i].strTableName;
		Project::Tools::WideCharToUtf8(m_vecHCrossTableName[i].strOrderColumn,strOrderColumn_Utf8);
		Project::Tools::WideCharToUtf8(m_vecHCrossTableName[i].strFilterColumn,strFilterColumn_Utf8);
		Project::Tools::WideCharToUtf8(wstrTableName,strTName_Utf8);

		int nCoulumnCount = m_vecHCrossTableName[i].nColumnCount;
		int nFilterColumnIndex = m_vecHCrossTableName[i].nFilterColumnIndex;
		//先查所有待索引字段
		for(int j=0; j<m_vecHCrossTableName[i].vecFilter.size(); ++j)
		{
			string strFilterValue_Utf8;
			wstring strFilterValue = m_vecHCrossTableName[i].vecFilter[j];
			Project::Tools::WideCharToUtf8(strFilterValue,strFilterValue_Utf8);
			sqlstream.str("");
			sqlstream << "select * from " << strTName_Utf8 << " where " << strFilterColumn_Utf8
				<<" ='" << strFilterValue_Utf8 << "' order by " << strOrderColumn_Utf8 << " desc limit 0,1";

			if(IsConnected())
			{
				CString strSql = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
				vector<wstring> vecColumn;
				try
				{
					if(m_pConnection->GetState() != adStateClosed)
						m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
				}
				catch (_com_error e)
				{
				}

				if(m_pRecordSet)
				{
					_variant_t vsnum;
					try
					{
						while(!m_pRecordSet->ENDOFFILE)
						{	
							CString strKey;
							strKey.Format(_T("%s/%s"),wstrTableName.c_str(),strFilterValue.c_str());
							map<int,wstring> mapValue;
							for(int i=0; i<nCoulumnCount; ++i)
							{
								string strValue = "";
								vsnum = m_pRecordSet->GetCollect(_variant_t((long)i));	
								if(vsnum.vt!=VT_NULL)
								{
									strValue = (char*)(_bstr_t)vsnum;
									mapValue[i] = Project::Tools::UTF8ToWideChar(strValue.c_str());
								}
							}
							m_HCrossTableDataMap[strKey.GetString()] = mapValue;
						}
					}
					catch (_com_error e)
					{

					}
				}
				Sleep(10);
			}
		}
	}
}

void CSqlServerCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CSqlServerCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	for (unsigned int i = 0; i < m_pointlist.size(); i++){
		const DataPointEntry&  modbusentry = m_pointlist[i];
		valuelist.push_back(make_pair(modbusentry.GetShortName(), modbusentry.GetSValue()));
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_MYSQL,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
		strName.Format(_T("%s%d"),ERROR_MYSQL,m_nEngineIndex);
		//valuelist.push_back(make_pair(strName, Project::Tools::AnsiToWideChar(m_strErrInfo.c_str())));
	}
}

int CSqlServerCtrl::IsExistInVec( wstring strSer, vector<wstring> vecSer )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strSer == vecSer[i])
		{
			return i;
		}
	}
	return i;
}

int CSqlServerCtrl::IsExistInVec(wstring strTName,wstring strOrderColumn, wstring strFilterColumn,vector<_MysqlCrossTableUnit> vecSer )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		CString strFilter;
		strFilter.Format(_T("%s/%d"),vecSer[i].strFilterColumn.c_str(),vecSer[i].nFilterColumnIndex);
		if(strTName == vecSer[i].strTableName && strOrderColumn == vecSer[i].strOrderColumn && strFilterColumn == strFilter.GetString() )
		{
			return i;
		}
	}
	return i;
}

DataPointEntry* CSqlServerCtrl::FindEntry( wstring nRuntimeID )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetParam(1) == nRuntimeID)
		{
			return &m_pointlist[i];
		}
	}

	return NULL;
}

UINT WINAPI CSqlServerCtrl::ThreadUpdatePointFunc( LPVOID lparam )
{
	CSqlServerCtrl* pthis = (CSqlServerCtrl*)lparam;
	if (pthis != NULL)
	{
		while (true){

			if(pthis->m_bExitUpdateThread)
				break;

			pthis->UpdatePoint();
			Sleep(2000);
		}
	}
	return 0;
}

void CSqlServerCtrl::TerminateUpdatePointThread()
{
	::TerminateThread(m_hupdatepointthread, 0);
	WaitForSingleObject(m_hupdatepointthread, INFINITE);
}

void CSqlServerCtrl::UpdatePoint()
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry&  mysqlentry = m_pointlist[i];
		int nDataType  = _wtoi(mysqlentry.GetParam(9).c_str());
		if(nDataType == 0)
		{
			map<wstring,wstring>::iterator iter = m_runTimeDataMap.find(mysqlentry.GetParam(1));
			if(iter != m_runTimeDataMap.end())
			{
				wstring dValue = iter->second;
				m_pointlist[i].SetSValue(dValue);
			}
		}
		else if(nDataType == 3)
		{
			wstring strTaName = mysqlentry.GetParam(7);
			wstring strParam1 = mysqlentry.GetParam(1);
			vector<wstring> vecParam1;
			Project::Tools::SplitStringByChar(strParam1.c_str(),L'#',vecParam1);
			if(vecParam1.size() == 2)
			{
				CString strTableKey;
				strTableKey.Format(_T("%s/%s"),strTaName.c_str(),vecParam1[0].c_str());
				int nIndex = _wtoi(vecParam1[1].c_str());
				map<wstring,map<int,wstring>>::iterator iter = m_HCrossTableDataMap.find(strTableKey.GetString());
				if(iter != m_HCrossTableDataMap.end())
				{
					map<int,wstring>::iterator iterValue = iter->second.find(nIndex);
					if(iterValue != iter->second.end())
					{
						wstring dValue = iterValue->second;
						m_pointlist[i].SetSValue(dValue);
					}
				}
			}
		}
		else
		{
			map<wstring,map<wstring,wstring>>::iterator itert = m_runTimeDataMap1.find(mysqlentry.GetParam(7));
			if(itert != m_runTimeDataMap1.end())
			{
				map<wstring,wstring>::iterator itert1 = itert->second.find(mysqlentry.GetParam(1));
				if(itert1 != itert->second.end())
				{
					m_pointlist[i].SetSValue(itert1->second);
				}
			}
		}
	}
}

void CSqlServerCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CSqlServerCtrl::Exit()
{
	m_bExitUpdateThread = true;
	
	if(m_hupdatethread)
		WaitForSingleObject(m_hupdatethread,INFINITE);

	if(m_hupdatepointthread)
		WaitForSingleObject(m_hupdatepointthread,INFINITE);

	ExitConnect();
	return true;
}

bool CSqlServerCtrl::SetValue( wstring strName, wstring strValue )
{
	return true;
}

string CSqlServerCtrl::GetHostIP()
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

bool CSqlServerCtrl::QueryTableColumn0()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	std::ostringstream sqlstream;
	for(int i= 0; i<m_vecHLongTableName.size(); i++)
	{
		MYSQL_ROW  row;								//记录集	
		//先拿表头
		string strTableName_Utf8,strDBName_Utf8;
		Project::Tools::WideCharToUtf8(m_vecHLongTableName[i],strTableName_Utf8);
		Project::Tools::WideCharToUtf8(m_strdatabase,strDBName_Utf8);

		sqlstream.str("");
		sqlstream << "SELECT COLUMN_NAME FROM information_schema.COLUMNS c WHERE c.TABLE_NAME = '"
			<< strTableName_Utf8 << "' and c.TABLE_SCHEMA='" << strDBName_Utf8 << "';";
		if(IsConnected())
		{
			vector<wstring> vecColumn;
			CString strSql;
			strSql = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
			try
			{
				if(m_pConnection->GetState() != adStateClosed)
					m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
			}
			catch (_com_error e)
			{
			}

			if(m_pRecordSet)
			{
				_variant_t vsnum;
				try
				{
					while(!m_pRecordSet->ENDOFFILE)
					{	
						string strName = "";
						vsnum = m_pRecordSet->GetCollect(_variant_t((long)0));	
						if(vsnum.vt!=VT_NULL)
						{
							strName = (char*)(_bstr_t)vsnum;
						}
						m_TableColumn0[m_vecHLongTableName[i]] = Project::Tools::UTF8ToWideChar(strName.c_str());
					}
				}
				catch (_com_error e)
				{

				}
			}
			Sleep(10);
		}
	}

	sqlstream.str("");
	for(int i= 0; i<m_vecHCrossTableName.size(); i++)
	{
		MYSQL_ROW  row;								//记录集	
		//先拿表头
		string strTableName_Utf8,strDBName_Utf8;
		Project::Tools::WideCharToUtf8(m_vecHCrossTableName[i].strTableName,strTableName_Utf8);
		Project::Tools::WideCharToUtf8(m_strdatabase,strDBName_Utf8);

		sqlstream.str("");
		sqlstream << "SELECT COLUMN_NAME FROM information_schema.COLUMNS c WHERE c.TABLE_NAME = '"
			<< strTableName_Utf8 << "' and c.TABLE_SCHEMA='" << strDBName_Utf8 << "';";

		if(IsConnected())
		{
			vector<wstring> vecColumn;
			CString strSql;
			strSql = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
			try
			{
				if(m_pConnection->GetState() != adStateClosed)
					m_pRecordSet = m_pConnection->Execute((_bstr_t)strSql,NULL,adCmdText);
			}
			catch (_com_error e)
			{
			}

			map<int,wstring> mapColumn;
			int nColumnIndex = 0;
			if(m_pRecordSet)
			{
				_variant_t vsnum;
				try
				{
					while(!m_pRecordSet->ENDOFFILE)
					{	
						string strName = "";
						vsnum = m_pRecordSet->GetCollect(_variant_t((long)0));	
						if(vsnum.vt!=VT_NULL)
						{
							strName = (char*)(_bstr_t)vsnum;
						}
						mapColumn[nColumnIndex] = Project::Tools::UTF8ToWideChar(strName.c_str());
						nColumnIndex++;
					}
				}
				catch (_com_error e)
				{

				}
			}
			m_vecHCrossTableName[i].nColumnCount = mapColumn.size();
			m_HCrossTableColumn[m_vecHCrossTableName[i].strTableName] = mapColumn;
			Sleep(10);
		}
	}

	return true;
}

void CSqlServerCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

bool CSqlServerCtrl::ConnectSqlDB( const wstring& host, const wstring& username, const wstring& password, const wstring& database )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	try
	{
		HRESULT hr = m_pConnection.CreateInstance("ADODB.Connection");//创建Connection对象
		if(SUCCEEDED(hr))
		{
			wstring strConnect = L"Provider=SQLOLEDB; Server=" + host + L";Database=" + database + L"; uid=" + username + L"; pwd=" + password + L";";
			hr = m_pConnection->Open(strConnect.c_str(),"","",adModeUnknown);//NULL、adConnectUnspecified、//建立与服务器连接
		}
	}
	catch(_com_error &e)
	{
		
	}

	if(m_pConnection == NULL)
	{
		return false;
	}
	return true;
}

void CSqlServerCtrl::ExitConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	if(m_pRecordSet != NULL)
	{
		m_pRecordSet.Release();
	}
	m_pConnection->Close();
	::CoUninitialize();
}

bool CSqlServerCtrl::IsConnected()
{
	if(m_pConnection && m_pConnection->GetState() != adStateClosed)
		return true;
	return false;
}

UINT WINAPI CSqlServerCtrl::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CSqlServerCtrl* pthis = (CSqlServerCtrl*)lparam;
	if (pthis != NULL)
	{
		while(!pthis->m_bExitUpdateThread)
		{
			pthis->ReConnect();
			int nTimeCount = 5;
			while(!pthis->m_bExitUpdateThread && nTimeCount>=0)
			{
				nTimeCount--;
				Sleep(1000);
			}
		}
	}
	return 0;
}

bool CSqlServerCtrl::ReConnect()
{
	if(!IsConnected())			//connect连接失败
	{
		if(ConnectSqlDB(m_strhost,m_strusername,m_strpassword,m_strdatabase))
		{
			return true;
		}
		return false;
	}
	return true;
}
