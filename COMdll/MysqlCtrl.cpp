#pragma once
#include "StdAfx.h"
#include "MysqlCtrl.h"
#include "atlstr.h"
#include "../LAN_WANComm/PingIcmp.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/EngineInfoDefine.h"
#include <iostream>
#include <sstream>

const string BEOP_OUTPUT = "realtimedata_output";
const string BEOP_OPERATION = "operation_record";

CMysqlCtrl::CMysqlCtrl( void )
	: m_hupdatethread(NULL)
	, m_hupdatepointthread(NULL)
	, m_bExitUpdateThread(FALSE)
	, m_nTableType(0)
	, m_strLocalIP("")
	, m_hDataCheckRetryConnectionThread(NULL)
	, m_nMysqlNetworkErrorCount(0)
	, m_bConnectOK(FALSE)
	, m_bQueryColumnOK(FALSE)
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
}

CMysqlCtrl::~CMysqlCtrl( void )
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

bool CMysqlCtrl::Init()
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
	
	m_bConnectOK = true;

	CString strInfo;
	strInfo.Format(_T("MySQL Engine Connect: host(%s), username(%s), pwd(%s), port(%d), database(%s)\r\n"), m_strhost.c_str(),
		m_strusername.c_str(), m_strpassword.c_str(), m_nPort, m_strdatabase.c_str());
	PrintLog(strInfo.GetString());

	if(!ConnectToDB(m_strhost,m_strusername,m_strpassword,m_strdatabase,m_nPort))
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("connect fail");
		m_bConnectOK = false;

		PrintLog(_T("MySQL Engine Connect failed"));
	}
	else
	{
		PrintLog(_T("MySQL Engine Connect success"));
	}
	m_hupdatethread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdateValueFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hupdatepointthread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdatePointFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	return m_bConnectOK;
}

UINT WINAPI CMysqlCtrl::ThreadUpdateValueFunc( LPVOID lparam )
{
	CMysqlCtrl* pthis = (CMysqlCtrl*)lparam;
	if (pthis != NULL)
	{
		while(!pthis->m_bExitUpdateThread)
		{
			if(!pthis->m_bQueryColumnOK)
			{
				pthis->QueryTableColumn0();
			}
			else
			{
				pthis->UpdateValue();
			}

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

void CMysqlCtrl::TerminateUpdateValueThread()
{
	::TerminateThread(m_hupdatethread, 0);
	WaitForSingleObject(m_hupdatethread, INFINITE);
}

void CMysqlCtrl::UpdateValue()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	if(!m_bConnectOK)
		return;

	bool bQuereSuccess = false;
	//查纵表
	for(int i= 0; i<m_vecLongTableName.size(); i++)
	{
		MYSQL_ROW							row;								//记录集			
		string strName = Project::Tools::WideCharToUtf8(m_vecLongTableName[i].c_str());
		string strSql = "select * from ";
		strSql += strName;

		MYSQL_RES* result = QueryAndReturnResult(strSql.c_str());
		if (!result)
		{
			continue;
		}
		while(row = FetchRow(result))		//读每一个point
		{
			m_runTimeDataMap[Project::Tools::UTF8ToWideChar(row[1])]  = Project::Tools::UTF8ToWideChar(row[2]);
		}
		FreeReturnResult(result);
		Sleep(10);
		bQuereSuccess = true;
	}

	//查横表
	std::ostringstream sqlstream;
	for(int i= 0; i<m_vecCrossTableName.size(); i++)
	{
		MYSQL_ROW  row;								//记录集	
		//先拿表头
		string strTableName_Utf8,strDBName_Utf8;
		Project::Tools::WideCharToUtf8(m_vecCrossTableName[i],strTableName_Utf8);
		Project::Tools::WideCharToUtf8(m_strdatabase,strDBName_Utf8);

		sqlstream.str("");
		sqlstream << "SELECT COLUMN_NAME FROM information_schema.COLUMNS c WHERE c.TABLE_NAME = '"
			<< strTableName_Utf8 << "' and c.TABLE_SCHEMA='" << strDBName_Utf8 << "';";
		
		vector<wstring> vecColumn;
		MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
		if (!result)
		{
			continue;
		}
		while(row = FetchRow(result))		//读每一个column
		{
			vecColumn.push_back(Project::Tools::UTF8ToWideChar(row[0]));
		}
		FreeReturnResult(result);
		Sleep(10);
		bQuereSuccess = true;

		sqlstream.str("");
		if(vecColumn.size()>0)
		{
			string strColum0_Utf8;
			Project::Tools::WideCharToUtf8(vecColumn[0],strColum0_Utf8);
			sqlstream << "SELECT * from " << strTableName_Utf8 << " order by " << strColum0_Utf8 << " desc limit 0,1;";
		}

		result = QueryAndReturnResult(sqlstream.str().c_str());
		if (!result)
		{
			continue;
		}

		int length = static_cast<int>(GetRowNum(result));
		if (0 == length){
			continue;
		}

		if(row = FetchRow(result) )
		{
			for(int i=0; i<vecColumn.size(); ++i)
			{
				m_runTimeDataMap[vecColumn[i]] = Project::Tools::UTF8ToWideChar(row[i]);
			}
		}
		FreeReturnResult(result);
		bQuereSuccess = true;
	}

	//查历史纵表
	for(int i= 0; i<m_vecHLongTableName.size(); i++)
	{
		wstring strTableName = m_vecHLongTableName[i];
		map<wstring,wstring>::iterator iter = m_TableColumn0.find(strTableName);
		if(iter != m_TableColumn0.end())
		{
			wstring strColumn0 = iter->second;

			MYSQL_ROW  row;								//记录集	
			//先拿表头
			string strTableName_Utf8,strDBName_Utf8,strColumn0_Utf8,strTime_Utf8;
			Project::Tools::WideCharToUtf8(strTableName,strTableName_Utf8);
			Project::Tools::WideCharToUtf8(m_strdatabase,strDBName_Utf8);
			Project::Tools::WideCharToUtf8(strColumn0,strColumn0_Utf8);

			sqlstream.str("");
			sqlstream << "select * from " << strTableName_Utf8 << " order by " << strColumn0_Utf8 << " desc limit 0,1";
			MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
			if (!result)
			{
				continue;
			}

			if(row = FetchRow(result))		//读每一个column
			{
				strTime_Utf8 = row[0];
			}
			FreeReturnResult(result);
			Sleep(10);
			bQuereSuccess = true;

			sqlstream.str("");
			sqlstream << "select * from " << strTableName_Utf8 << " where " << strColumn0_Utf8 << "='"
				<< strTime_Utf8 << "';";

			result = QueryAndReturnResult(sqlstream.str().c_str());
			if (!result)
			{
				continue;
			}

			while(row = FetchRow(result))		//读每一个列表
			{
				m_runTimeDataMap1[strTableName][Project::Tools::UTF8ToWideChar(row[1])] = Project::Tools::UTF8ToWideChar(row[2]);
			}
			FreeReturnResult(result);
			Sleep(10);
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

			MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
			if (!result)
			{
				continue;
			}
			MYSQL_ROW  row;								//记录集	
			while(row = FetchRow(result))		//读每一个列表
			{
				CString strKey;
				strKey.Format(_T("%s/%s"),wstrTableName.c_str(),strFilterValue.c_str());
				map<int,wstring> mapValue;
				for(int i=0; i<nCoulumnCount; ++i)
				{
					if(row[i] != NULL)
					{
						mapValue[i] = Project::Tools::UTF8ToWideChar(row[i]);
					}
				}
				m_HCrossTableDataMap[strKey.GetString()] = mapValue;
			}
			FreeReturnResult(result);
			Sleep(10);
			bQuereSuccess = true;
		}
	}

	if(!bQuereSuccess)
	{
		PrintLog(L"ERROR: Mysql Update Value failed!\r\n",false);
		SetNetworkError();
	}
	else
	{
		m_oleUpdateTime = COleDateTime::GetCurrentTime();
	}
}

void CMysqlCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CMysqlCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleUpdateTime;
	if(oleSpan.GetTotalMinutes() <= 30)
	{
		for (unsigned int i = 0; i < m_pointlist.size(); i++)
		{
			const DataPointEntry&  modbusentry = m_pointlist[i];
			valuelist.push_back(make_pair(modbusentry.GetShortName(), modbusentry.GetSValue()));
		}
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

int CMysqlCtrl::IsExistInVec( wstring strSer, vector<wstring> vecSer )
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

int CMysqlCtrl::IsExistInVec(wstring strTName,wstring strOrderColumn, wstring strFilterColumn,vector<_MysqlCrossTableUnit> vecSer )
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

DataPointEntry* CMysqlCtrl::FindEntry( wstring nRuntimeID )
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

UINT WINAPI CMysqlCtrl::ThreadUpdatePointFunc( LPVOID lparam )
{
	CMysqlCtrl* pthis = (CMysqlCtrl*)lparam;
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

void CMysqlCtrl::TerminateUpdatePointThread()
{
	::TerminateThread(m_hupdatepointthread, 0);
	WaitForSingleObject(m_hupdatepointthread, INFINITE);
}

void CMysqlCtrl::UpdatePoint()
{
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleUpdateTime;
	if(oleSpan.GetTotalMinutes() <= 30)
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
}

void CMysqlCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CMysqlCtrl::Exit()
{
	m_bExitUpdateThread = true;
	
	if(m_hupdatethread)
		WaitForSingleObject(m_hupdatethread,INFINITE);

	if(m_hupdatepointthread)
		WaitForSingleObject(m_hupdatepointthread,INFINITE);

	return true;
}

bool CMysqlCtrl::SetValue( wstring strName, wstring strValue )
{
	bool bFindPoint = false;
	DataPointEntry  mysqlpoint;
	for (unsigned int i = 0; i < m_pointlist.size(); i++){
		mysqlpoint = m_pointlist[i];
		if(mysqlpoint.GetShortName() == strName)
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
	wstring strOldValue = mysqlpoint.GetSValue();
	if(strOldValue == strValue)
		bResult  = true;

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	std::ostringstream sqlstream;
	string strValue_Utf8;
	Project::Tools::WideCharToUtf8(strValue,strValue_Utf8);
	if(!bResult)
	{
		if(_wtoi(mysqlpoint.GetParam(9).c_str()) == 0)		//纵表才能修改
		{
			sqlstream.str("");
			sqlstream << "delete from " << BEOP_OUTPUT << " where pointname='" 
				<< Project::Tools::WideCharToAnsi(mysqlpoint.GetParam(1).c_str())<< "';";
			Execute(sqlstream.str());

			sqlstream.str("");
			sqlstream << "insert into " << BEOP_OUTPUT << " values ('"
				<< Project::Tools::WideCharToAnsi(mysqlpoint.GetParam(1).c_str())<< "','" <<strValue_Utf8 << "');";
			string sql_temp = sqlstream.str();
			bResult = Execute(sql_temp);
		}
		
	}

	if(bResult)
	{
		//更新本内存
		m_runTimeDataMap[strName] = strValue;

		//成功的话插入一条操作记录到远程
		string strOldValue_Utf8;
		Project::Tools::WideCharToUtf8(strOldValue,strOldValue_Utf8);

		string strDescription_Utf8;
		wstring strDescription = mysqlpoint.GetDescription();
		if(strDescription.length() <= 0)
		{
			Project::Tools::WideCharToUtf8(strName,strDescription_Utf8);
		}
		else
		{
			Project::Tools::WideCharToUtf8(mysqlpoint.GetDescription(),strDescription_Utf8);
		}
		
		sqlstream.str("");
		sqlstream << "insert into " << BEOP_OPERATION << " (user, OptRemark) values ('remote','remote pc("
			<< m_strLocalIP << ") change "
			<< strDescription_Utf8<< " value ("
			<< strOldValue_Utf8 << "->" << strValue_Utf8 << ")');";
		Execute(sqlstream.str());
	}

	return bResult;
}

string CMysqlCtrl::GetHostIP()
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

bool CMysqlCtrl::QueryTableColumn0()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	m_bQueryColumnOK = false;
	
	if(!m_bConnectOK)
		return false;

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

		vector<wstring> vecColumn;
		MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
		if (!result)
		{
			continue;
		}
		if(row = FetchRow(result))		//读第一个column
		{
			m_TableColumn0[m_vecHLongTableName[i]] = Project::Tools::UTF8ToWideChar(row[0]);
		}
		FreeReturnResult(result);
		Sleep(10);

		m_bQueryColumnOK = true;
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

		vector<wstring> vecColumn;
		MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
		if (!result)
		{
			continue;
		}
		map<int,wstring> mapColumn;
		int nColumnIndex = 0;
		while(row = FetchRow(result))		//读第一个column
		{
			mapColumn[nColumnIndex] = Project::Tools::UTF8ToWideChar(row[0]);
			nColumnIndex++;
		}
		m_vecHCrossTableName[i].nColumnCount = mapColumn.size();
		m_HCrossTableColumn[m_vecHCrossTableName[i].strTableName] = mapColumn;
		FreeReturnResult(result);
		Sleep(10);

		m_bQueryColumnOK = true;
	}

	if(m_vecHLongTableName.size() == 0 || m_vecHCrossTableName.size() == 0)
		m_bQueryColumnOK = true;

	return true;
}

void CMysqlCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

UINT WINAPI CMysqlCtrl::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CMysqlCtrl* pthis = (CMysqlCtrl*)lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitUpdateThread)
		{
			if(pthis->m_nMysqlNetworkErrorCount>=6 || !pthis->m_bConnectOK)
			{
				if(pthis->ReConnect())
					pthis->ClearNetworkError();
			}

			int nPollSleep = 60;
			while (!pthis->m_bExitUpdateThread)
			{
				if(nPollSleep <= 0)
				{
					break;
				}
				nPollSleep--;
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool CMysqlCtrl::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_dblock);
	PrintLog(L"INFO : Mysql ReConnecting...\r\n",false);
	m_bConnectOK = false;
	m_bQueryColumnOK = false;	

	if(!ConnectToDB(m_strhost,m_strusername,m_strpassword,m_strdatabase,m_nPort))
	{
		PrintLog(L"ERROR: Mysql ReConnecting failed!\r\n");
		m_bConnectOK = false;
	}
	else
	{
		PrintLog(L"INFO : Mysql reconnecting success!\r\n",false);
		m_bConnectOK = true;
	}
	return m_bConnectOK;
}

void CMysqlCtrl::SetNetworkError()
{
	if(m_nMysqlNetworkErrorCount<10000)
		m_nMysqlNetworkErrorCount++;
}

void CMysqlCtrl::ClearNetworkError()
{
	m_nMysqlNetworkErrorCount = 0;
}

void CMysqlCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
