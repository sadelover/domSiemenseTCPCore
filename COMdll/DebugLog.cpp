// NetworkComm.h dll implement.

#include "stdafx.h"
#include "DebugLog.h"
#include <iostream>
#include <sstream>
// Static object.
CDebugLog* CDebugLog::m_csDebugCommObj = NULL;
Mutex CDebugLog::m_mutexLock;
Beopdatalink::CCommonDBAccess*	CDebugLog::m_dbsession_history = NULL;
hash_map<int,COleDateTime>	CDebugLog::m_mapErrCode;
hash_map<wstring,ErrPoint>	CDebugLog::m_mapErrPoint;
hash_map<wstring,wstring>   CDebugLog::m_mapParamValue;
vector<DebugLogInfo>		CDebugLog::m_vecDebugLog;	
bool						CDebugLog::m_bExitThread;
HANDLE						CDebugLog::m_houtthread;
bool						CDebugLog::m_bOutDebugLog;
//======================================================================
// Default constructor.
CDebugLog::CDebugLog()
//======================================================================
{
	// Initialize parameters and objects.
	InitParams();
	
}

//======================================================================
// Default destructor.
CDebugLog::~CDebugLog()
//======================================================================
{
}

//======================================================================
// Get the singleton instance of assign object.
CDebugLog* CDebugLog::GetInstance()
//======================================================================
{
	if (m_csDebugCommObj == NULL)
	{
		Scoped_Lock<Mutex> mut(m_mutexLock);
		m_csDebugCommObj = new CDebugLog();
		if(m_csDebugCommObj != NULL)
		{
			m_houtthread = (HANDLE) _beginthreadex(NULL, 0, ThreadOutPutErrFunc, m_csDebugCommObj, NORMAL_PRIORITY_CLASS, NULL);
		}
		atexit(DestroyInstance);
	}

	return m_csDebugCommObj;
}

//======================================================================
// Destroy the singleton instance of assign object.
void CDebugLog::DestroyInstance()
//======================================================================
{
	m_bExitThread = true;
	if(m_houtthread)
	{
		CloseHandle(m_houtthread);
		m_houtthread = NULL;
	}

	if (m_csDebugCommObj != NULL)
	{
		delete m_csDebugCommObj;
		m_csDebugCommObj = NULL;
	}
}

//======================================================================
// Initialize parameters and object.
void CDebugLog::InitParams()
//======================================================================
{
	m_bExitThread = false;
	m_bOutDebugLog = false;
	m_houtthread = NULL;
	m_mapErrCode.clear();
	m_mapErrPoint.clear();
	m_mapParamValue.clear();
	UpdateDebugSetting();
}

bool CDebugLog::UpdateDebugSetting()
{
	wstring strDebugType = L"0";
	wstring strDebugLevel = L"0";
	wstring strDebugOut = L"0";
	wstring strOutPutDebug = L"0";
	if(m_dbsession_history)
	{
		strDebugType = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debugtype",L"0");
		strDebugLevel = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debuglevel",L"0");
		strDebugOut = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debugout",L"0");
		strOutPutDebug = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debug",L"0");
	}
	m_nDebugType = _wtoi(strDebugType.c_str());
	m_nDebugOut = _wtoi(strDebugOut.c_str());
	m_nDebugLevel = _wtoi(strDebugLevel.c_str());
	m_bOutDebugLog = _wtoi(strOutPutDebug.c_str());
	return true;
}

bool CDebugLog::DebugLog( E_DEBUG_LOG_TYPE type,E_DEBUG_LOG_LEVEL level,wstring strLog )
{
	if(level == E_DEBUG_LOG_LEVEL_BASE)
	{
		//输出log
		PrintLog(strLog);
		return true;
	}

	if(m_nDebugType == E_DEBUG_LOG_NULL || m_nDebugLevel == E_DEBUG_LOG_LEVEL_NULL)
		return false;

	if(level != E_DEBUG_LOG_LEVEL_BASE)
	{
		if(m_nDebugType == E_DEBUG_LOG_ALL)
		{
			if(m_nDebugLevel == E_DEBUG_LOG_LEVEL_All)
			{
				//输出log
				PrintLog(strLog);
			}
			else
			{
				if(m_nDebugLevel == level)
				{
					//输出log
					PrintLog(strLog);
				}
			}
		}
		else
		{
			if(m_nDebugType == type)
			{
				if(m_nDebugLevel == E_DEBUG_LOG_LEVEL_All || level == E_DEBUG_LOG_LEVEL_BASE)
				{
					//输出log
					PrintLog(strLog);
				}
				else
				{
					if(m_nDebugLevel == level)
					{
						//输出log
						PrintLog(strLog);
					}
				}
			}	
		}	
	}
	return true;
}

void CDebugLog::SetHistoryDbCon( Beopdatalink::CCommonDBAccess* phisdbcon )
{
	m_dbsession_history = phisdbcon;
}

bool CDebugLog::SetErrPoints( vector<DataPointEntry> vecPoints ,int nErrCode)
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_mutexLock);
	for(int i=0; i<vecPoints.size(); ++i)
	{
		COleDateTime oleNow = COleDateTime::GetCurrentTime();
		ErrPoint point;
		point.nErrCode = nErrCode;
		point.oleTime = oleNow;
		wstring strPoint = vecPoints[i].GetShortName();
		point.strPointName = strPoint;
		point.nPointType = vecPoints[i].GetPointType();

		m_mapErrCode[nErrCode] = oleNow;
		m_mapErrPoint[strPoint] = point;
	}
	return true;
}

bool CDebugLog::SetErrPoint( DataPointEntry entry ,int nErrCode)
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_mutexLock);
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	ErrPoint point;
	point.nErrCode = nErrCode;
	point.oleTime = oleNow;
	wstring strPoint = entry.GetShortName();
	point.strPointName = strPoint;
	point.nPointType = entry.GetPointType();

	m_mapErrCode[nErrCode] = oleNow;
	m_mapErrPoint[strPoint] = point;
	return true;
}

bool CDebugLog::SetErrPoint( wstring strPoint,int nPointType, int nErrCode )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_mutexLock);
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	ErrPoint point;
	point.nErrCode = nErrCode;
	point.oleTime = oleNow;
	point.strPointName = strPoint;
	point.nPointType = nPointType;

	m_mapErrCode[nErrCode] = oleNow;
	m_mapErrPoint[strPoint] = point;
	return true;
}

bool CDebugLog::SetErrPoint( vector<wstring> vecNames,int nPointType,int nErrCode )
{
	return true;
}

bool CDebugLog::SetErrCode( int nErrCode ,bool bOutput)
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_mutexLock);
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	m_mapErrCode[nErrCode] = oleNow;
	if(bOutput)
	{
		OutErrCodeLog(oleNow,nErrCode);
	}
	return true;
}

UINT WINAPI CDebugLog::ThreadOutPutErrFunc( LPVOID lparam )
{
	CDebugLog* pthis = (CDebugLog*)lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			int nWaitSleep = 60;
			while(!pthis->m_bExitThread)
			{
				if(nWaitSleep>=0)
				{
					nWaitSleep--;
					Sleep(1000);	
				}
				else
				{
					break;
				}
			}

			pthis->OutPutErr();
			pthis->OutPutDebug();
		}
	}

	return 0;
}

bool CDebugLog::OutPutErr()
{
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		wstring strOutErrCode = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"outputerrcode",L"0");
		wstring strOutErrPoint = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"outputerrpoint",L"0");
		wstring strErrMinute = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"outputerrminute",L"10");
		if(strOutErrCode == L"1")
		{
			UpdateErrCode(_wtoi(strErrMinute.c_str()));
		}

		if(strOutErrPoint == L"1")
		{
			UpdateErrPoints(_wtoi(strErrMinute.c_str()));
		}
	}
	return true;
}

bool CDebugLog::UpdateErrCode(int nErrMinute)
{
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		vector<Beopdatalink::ErrCodeInfo> vecErrCode;
		hash_map<int,COleDateTime>::iterator iter =	m_mapErrCode.begin();
		while(iter != m_mapErrCode.end())
		{
			COleDateTime oleTime = (*iter).second;
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - oleTime;
			if(oleSpan.GetMinutes() <= nErrMinute)
			{
				Beopdatalink::ErrCodeInfo info;
				info.nErrCode = (*iter).first;
				info.oleErrTime = oleTime;
				vecErrCode.push_back(info);
			}
			++iter;
		}
		return m_dbsession_history->InsertErrCodeInfo(vecErrCode);
	}
	return false;
}

bool CDebugLog::UpdateErrPoints(int nErrMinute)
{
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		vector<Beopdatalink::ErrPointInfo> vecErrPoint;
		hash_map<wstring,ErrPoint>::iterator iter =	m_mapErrPoint.begin();
		while(iter != m_mapErrPoint.end())
		{
			ErrPoint point = (*iter).second;
			COleDateTime oleTime = point.oleTime;
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - oleTime;
			if(oleSpan.GetMinutes() <= nErrMinute)
			{
				Beopdatalink::ErrPointInfo info;
				info.nErrCode = point.nErrCode;
				info.nPointType = point.nPointType;
				info.strPointName = Project::Tools::WideCharToAnsi(point.strPointName.c_str());
				info.oleErrTime = point.oleTime;
				vecErrPoint.push_back(info);
			}
			++iter;
		}
		return m_dbsession_history->InsertErrPoints(vecErrPoint);
	}
	return false;
}

bool CDebugLog::UpdateErrInfo()
{
	return true;
}

void CDebugLog::OutErrCodeLog( COleDateTime oleNow,int nErrCode )
{
	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		CString strLogPath;
		strLogPath.Format(_T("%s\\err_%d_%02d_%02d.log"),strPath.c_str(),oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());

		CString strLog;
		CString strTime;
		strTime.Format(_T("%02d:%02d:%02d"),oleNow.GetHour(),oleNow.GetMinute(),oleNow.GetSecond());
		strLog.Format(_T("%s::%d.\n"),strTime,nErrCode);

		char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		setlocale( LC_ALL, "chs" );
		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
		}
		setlocale( LC_CTYPE, old_locale ); 
		free( old_locale ); 	


	}
}

bool CDebugLog::Exit()
{
	m_bExitThread = true;
	m_dbsession_history = NULL;
	if(m_houtthread)
	{
		CloseHandle(m_houtthread);
		m_houtthread = NULL;
	}
	return true;
}

bool CDebugLog::SetBacnetInfo( wstring strParamName,char* strParamValue )
{
	//m_mapParamValue[strParamName] = Project::Tools::AnsiToWideChar(strParamValue).c_str();
	m_mapParamValue[strParamName] = L"";
	return true;
}

bool CDebugLog::SaveBacnetInfo()
{
	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		hash_map<wstring,wstring>::iterator iter1 = m_mapParamValue.begin();
		while(iter1 != m_mapParamValue.end())
		{
			wstring strParamName = (*iter1).first;
			wstring strParamValue = (*iter1).second;
			m_dbsession_history->WriteCoreDebugItemValue(strParamName,strParamValue);
			iter1++;
		}
		return true;
	}
	return false;
}

bool CDebugLog::SetDebugLog( wstring strLog )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_mutexLock);
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	DebugLogInfo log;
	log.oleTime = oleNow;
	log.strDebugLog = strLog;
	m_vecDebugLog.push_back(log);
	return true;
}

bool CDebugLog::OutPutDebug()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_mutexLock);
	if(m_vecDebugLog.size() <= 0)
		return true;

	if(m_dbsession_history && m_dbsession_history->IsConnected())
	{
		wstring strOutDebug = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"debug",L"0");
		m_bOutDebugLog = _wtoi(strOutDebug.c_str());
		if(m_bOutDebugLog)
		{
			std::ostringstream sqlstream;
			for(int i=0; i<m_vecDebugLog.size(); ++i)
			{
				wstring strTime;
				Project::Tools::OleTime_To_String(m_vecDebugLog[i].oleTime,strTime);
				sqlstream << Project::Tools::WideCharToAnsi(strTime.c_str()) << " - " << Project::Tools::WideCharToAnsi(m_vecDebugLog[i].strDebugLog.c_str()) << "\n";
			}

			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );

			CString strLog;
			strLog = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
			wstring strPath;
			GetSysPath(strPath);
			strPath += L"\\log";
			if(Project::Tools::FindOrCreateFolder(strPath))
			{
				CString strLogPath;
				SYSTEMTIME st;
				GetLocalTime(&st);
				strLogPath.Format(_T("%s\\debug_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);
				//记录Log
				CStdioFile	ff;
				if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
				{
					ff.Seek(0,CFile::end);
					ff.WriteString(strLog);
					ff.Close();
				}
			}
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 

		}
	}
	m_vecDebugLog.clear();
	return true;
}

bool CDebugLog::GetOutDebugFlag()
{
	return m_bOutDebugLog;
}

void CDebugLog::PrintLog( const wstring &strOut,bool bSaveLog /*= true*/ )
{
	_tprintf(strOut.c_str());
	if(bSaveLog)
	{
		if(m_nDebugOut)
		{
			Project::Tools::Scoped_Lock<Mutex> guardlock(m_mutexLock);
			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );

			CString strLog;
			SYSTEMTIME st;
			GetLocalTime(&st);
			wstring strTime;
			Project::Tools::SysTime_To_String(st,strTime);
			strLog += strTime.c_str();
			strLog += _T(" - ");
			strLog += strOut.c_str();

			wstring strPath;
			GetSysPath(strPath);
			strPath += L"\\log";
			if(Project::Tools::FindOrCreateFolder(strPath))
			{
				CString strLogPath;
				strLogPath.Format(_T("%s\\debug_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);
				//记录Log
				CStdioFile	ff;
				if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
				{
					ff.Seek(0,CFile::end);
					ff.WriteString(strLog);
					ff.Close();
				}
			}
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
		}
	}	
}
