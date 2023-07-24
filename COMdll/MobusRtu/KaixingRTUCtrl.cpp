#pragma once

#include "StdAfx.h"
#include "KaixingRTUCtrl.h"
#include "../COMdll//DebugLog.h"
#include "Tools/EngineInfoDefine.h"

HANDLE			CKaixingRTUCtrl::m_sendCmdEvent;
COleDateTime CKaixingRTUCtrl::m_oleSendCmdTime;

CKaixingRTUCtrl::CKaixingRTUCtrl(void)
	:m_hsendthread(NULL)
	,m_nSendReadCmdTimeInterval(30)
	,m_nRecevieTimeOut(5000)
	,m_nRecvedSlave(-1)
	,m_bExitThread(false)
	,m_bReadOneByOneMode(false)
	,m_nMutilReadCount(99)
	,m_nIDInterval(500)
	,m_nPortnr(0)
	,m_nBaud(9600)
	,m_cParity('N')
	,m_bInitPortSuccess(false)
	,m_nModbusNetworkErrorCount(0)
	,m_StartAddress(0)
	,m_WriteStartAddress(0)
	,m_nOutPut(0)
	,m_nReadNum(0)
	,m_nPollSendInterval(2)
	,m_nPrecision(6)
	,m_bDisSingleRead(false)
	,m_logsession(NULL)
	,m_bSaveErrLog(false)
	,m_bConnectFirstSuccess(false)
	,m_nReadTimeOut(2000)
	,m_nEvent(0)
	,m_nConcentratorType(115)
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_oleLastUpdateLog = COleDateTime::GetCurrentTime();
	m_pSendData = NULL;
	m_hDataCheckRetryConnectionThread = NULL;
	m_strErrInfo = "";
	m_nEngineIndex = 0;
	m_pointlist.clear();
	m_mapErrLog.clear();
	m_mapPointQuery.clear();

	m_sendCmdEvent = ::CreateEvent(NULL, false, false, L"");
	m_bNeedActive = false;
	m_oleSendCmdTime = COleDateTime::GetCurrentTime();

	InitializeCriticalSection(&m_CriticalSection);
	InitializeCriticalSection(&m_CriticalPointSection);
}

CKaixingRTUCtrl::~CKaixingRTUCtrl(void)
{
	if (m_hsendthread != NULL)
	{
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	if(m_hDataCheckRetryConnectionThread != NULL)
	{
		::CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
	ClosePort();
	DeleteCriticalSection(&m_CriticalSection);
	DeleteCriticalSection(&m_CriticalPointSection);
}

//static long rxdatacount=0;  //该变量用于接收字符计数
bool CKaixingRTUCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(length == 2)
	{
		//
		if(pRevData[0] == 0x54)				//回复头 T
		{
			if(pRevData[1] == 0xFE)			//终端设备无应答
			{

			}
			else if(pRevData[1] == 0xFD)	//集中器忙
			{

			}
			else if(pRevData[1] == 0xFF)	//集中器忙
			{

			}
			else if(pRevData[1] == 0x00)	//集中器第一次连接回应 执行成功
			{
				if(m_nEvent == 1)
				{
					m_bConnectFirstSuccess = true;
					SetEvent(m_sendCmdEvent);		//激活连接第二步事件
				}
			}
		}
	}
	else
	{
		if(pRevData[0] == 0x54)						//回复头 T
		{
			int nLength = pRevData[1];				//本单元数据长度len
			//需要累加和 异或和校验
			if(Is_DataValid(pRevData,length))
			{
				int nFunCode = pRevData[2];				//命令码
				if(nFunCode == 1)						//断开集中器
				{

				}
				else if(nFunCode == 201)				//断开集中器 (201)(新)
				{

				}
				else if(nFunCode == 4)					//校正集中器时间
				{

				}
				else if(nFunCode == 8)					//读集中器时间
				{
					if(nLength == 11)
					{
						CString strTime;
						strTime.Format(_T("20%02d-%02d-%02d %02d:%02d:%02d"),pRevData[9],pRevData[7],pRevData[6],pRevData[5],pRevData[4],pRevData[3]);
						WORD wCount = 0;
						wCount = pRevData[10];
						wCount = (wCount&0x00ff)<<8;
						wCount |= (pRevData[11]&0x00ff);
					}
				}
				else if(nFunCode == 50)					//设置电表的工作状态
				{

				}
				else if(nFunCode == 63)					//抄收指定表号数据及状态（单项）
				{

				}
				else if(nFunCode == 64)					//读取表的多项表底值（热量计等）
				{

				}
				else if(nFunCode == 115)				//连接115集中器第二步
				{
					if(nLength == 15)
					{
						CString strTime;
						strTime.Format(_T("20%02d-%02d-%02d %02d:%02d:%02d"),pRevData[9],pRevData[7],pRevData[6],pRevData[5],pRevData[4],pRevData[3]);
						WORD wCount = 0;
						wCount = pRevData[10];
						wCount = (wCount&0x00ff)<<8;
						wCount |= (pRevData[11]&0x00ff);
					}
				}
				else if(nFunCode == 215)				//连接215集中器第二步
				{
					if(nLength == 21)
					{
						CString strTime;
						strTime.Format(_T("20%02d-%02d-%02d %02d:%02d:%02d"),pRevData[19],pRevData[17],pRevData[16],pRevData[15],pRevData[14],pRevData[13]);
					}
				}
			}
		}
	}
	return true;
}

/***************************************************
*@brief:发送命令(串口通讯)
*@param: BYTE *pData 接收数据, BYTE bLength 接收数据长度
*@return:void
*@author: ZHW 2010-08-30
****************************************************/
BOOL CKaixingRTUCtrl::Is_DataValid(BYTE *pData, WORD dwLength)
{
	/*成功或失败的信号*/
	WORD wCRC			= 0;
	BYTE byData[2]	    = {0};
	if (dwLength <= 2){
		return FALSE;
	}
	wCRC = m_Modbus.CRC16___(pData, dwLength-2);
	memcpy(byData, &wCRC, sizeof(WORD));

	if ((byData[0]==pData[dwLength-1])
		&& (byData[1]==pData[dwLength-2]))
	{
		return TRUE;
	}
	return FALSE;
}

/***************************************************
*@brief:初始化COM
*@param: UINT nPortnr,串口号  UINT bBaud波特率
*@return: BOOL, TRUE成功，FALSE失败
*@author: ZHW 2011-04-01
****************************************************/
bool CKaixingRTUCtrl::InitCOMPort(UINT nPortnr, UINT bBaud, char parity)
{
	if (!InitPort(nPortnr, bBaud, parity) || !StartMonitoring())
	{
		return false;
	}
	return true;
}

UINT WINAPI CKaixingRTUCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CKaixingRTUCtrl* pthis = (CKaixingRTUCtrl*)lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			//
			int nPollSleep = pthis->m_nPollSendInterval;
			while (!pthis->m_bExitThread)
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

void CKaixingRTUCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CKaixingRTUCtrl::SetSendCmdTimeInterval( int nTimeMs,int nMutilReadCount/*=99*/,int nIDInterval /*= 500*/ ,int nTimeOut,int nPollInterval,int nPrecision,bool bDisSingleRead,bool bSaveErrLog)
{
	m_nSendReadCmdTimeInterval = nTimeMs;
	m_nMutilReadCount = nMutilReadCount;
	m_nIDInterval = nIDInterval;
	m_nRecevieTimeOut = nTimeOut;
	m_nPollSendInterval = nPollInterval;
	m_nPrecision = nPrecision;
	m_bDisSingleRead = bDisSingleRead;
	m_bSaveErrLog = bSaveErrLog;
}

bool CKaixingRTUCtrl::Init()
{
	if (m_pointlist.empty())
	{
		return false;
	}

	if(!InitCOMPort(m_nPortnr,m_nBaud,m_cParity))
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("connect fail");
		m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
		m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
		return false;
	}
	m_bInitPortSuccess = true;
	m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	return true;
}

bool CKaixingRTUCtrl::Exit()
{
	m_bExitThread = true;
	if(m_hsendthread)
	{
		WaitForSingleObject(m_hsendthread, 5000);
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	if(m_hDataCheckRetryConnectionThread)
	{
		WaitForSingleObject(m_hsendthread, 5000);
		::CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
	ClosePort();
	return true;
}

void CKaixingRTUCtrl::SetPortAndBaud( UINT nPortnr, UINT bBaud, char parity /*= 'N'*/ )
{
	m_nPortnr = nPortnr;
	m_nBaud = bBaud;
	m_cParity = parity;
}

void CKaixingRTUCtrl::SetNetworkError()
{
	if(m_nModbusNetworkErrorCount<10000)
		m_nModbusNetworkErrorCount++;
}

void CKaixingRTUCtrl::ClearNetworkError()
{
	m_nModbusNetworkErrorCount = 0;
}

void CKaixingRTUCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry&  modbusentry = m_pointlist[i];
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_MODBUS,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
	}
}

bool CKaixingRTUCtrl::GetValue( const wstring& pointname, double& result )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
			result =  entry.GetValue();
			return true;
		}
	}

	return false;
}

bool CKaixingRTUCtrl::SetValue( const wstring& pointname, double fValue )
{
	
	return false;
}

bool CKaixingRTUCtrl::InitDataEntryValue( const wstring& pointname, double value )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
			entry.SetValue(value);
			return true;
		}
	}

	return false;
}

void CKaixingRTUCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CKaixingRTUCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

UINT WINAPI CKaixingRTUCtrl::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CKaixingRTUCtrl* pthis = (CKaixingRTUCtrl*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(pthis->GetErrorCount() >= 5 || !pthis->m_bConnectOK)
			{
				pthis->ReInitPort();
			}

			//更新存储log
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - pthis->m_oleLastUpdateLog;
			if(oleSpan.GetTotalMinutes() >= 5)
			{
				pthis->SaveLog();
				pthis->m_oleLastUpdateLog = COleDateTime::GetCurrentTime();
			}

			int nPollSleep = 5;
			while (!pthis->m_bExitThread)
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

bool CKaixingRTUCtrl::ReInitPort()
{
	EnterCriticalSection(&m_CriticalSection);
	m_bInitPortSuccess = ReConnect() && StartMonitoring();
	if(m_bInitPortSuccess)
		_tprintf(L"ReConnect Modbus RTU(Port:%d) Success.\r\n",m_nPortNr);
	LeaveCriticalSection(&m_CriticalSection);
	return m_bInitPortSuccess;
}

bool CKaixingRTUCtrl::OutPutModbusReceive( const unsigned char* pRevData, DWORD length ,WORD wStart)
{
	if(m_nOutPut>0)
	{
		try
		{
			wstring strPath;
			GetSysPath(strPath);
			strPath += L"\\log";
			if(Project::Tools::FindOrCreateFolder(strPath))
			{
				COleDateTime oleNow = COleDateTime::GetCurrentTime();
				CString strLogPath;
				strLogPath.Format(_T("%s\\modbus_debug_%d_%02d_%02d.log"),strPath.c_str(),oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());

				CString strLog;
				std::ostringstream sqlstream;
				sqlstream << oleNow.GetHour() << ":" << oleNow.GetMinute() << ":" << oleNow.GetSecond() << "(" << wStart  <<") [" << charToHexStr((unsigned char*)pRevData,length) << "]\n";
				strLog = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
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
	return false;
}

bool CKaixingRTUCtrl::SetDebug( int nDebug )
{
	m_nOutPut = nDebug;
	return true;
}

string* CKaixingRTUCtrl::byteToHexStr( unsigned char* byte_arr, int arr_len )
{
	string* hexstr=new string();  
	for (int i=0;i<arr_len;i++)  
	{  
		char hex1;  
		char hex2;  
		/*借助C++支持的unsigned和int的强制转换，把unsigned char赋值给int的值，那么系统就会自动完成强制转换*/  
		int value=byte_arr[i];  
		int S=value/16;  
		int Y=value % 16;  
		//将C++中unsigned char和int的强制转换得到的商转成字母  
		if (S>=0&&S<=9)  
			hex1=(char)(48+S);  
		else  
			hex1=(char)(55+S);  
		//将C++中unsigned char和int的强制转换得到的余数转成字母  
		if (Y>=0&&Y<=9)  
			hex2=(char)(48+Y);  
		else  
			hex2=(char)(55+Y);  
		//最后一步的代码实现，将所得到的两个字母连接成字符串达到目的  
		*hexstr=*hexstr+hex1+hex2;  
	}  
	return hexstr;  
}

string CKaixingRTUCtrl::charToHexStr( unsigned char* byte_arr, int arr_len )
{
	std::ostringstream sqlstream;
	for (int i=0;i<arr_len;i++)  
	{  
		char hex1;  
		char hex2;  
		/*借助C++支持的unsigned和int的强制转换，把unsigned char赋值给int的值，那么系统就会自动完成强制转换*/  
		int value=byte_arr[i];  
		int S=value/16;  
		int Y=value % 16;  
		//将C++中unsigned char和int的强制转换得到的商转成字母  
		if (S>=0&&S<=9)  
			hex1=(char)(48+S);  
		else  
			hex1=(char)(55+S);  
		//将C++中unsigned char和int的强制转换得到的余数转成字母  
		if (Y>=0&&Y<=9)  
			hex2=(char)(48+Y);  
		else  
			hex2=(char)(55+Y);  
		//最后一步的代码实现，将所得到的两个字母连接成字符串达到目的  
		sqlstream << hex1 << hex2 << " ";
	}  
	return sqlstream.str().c_str();  
}

void CKaixingRTUCtrl::SendReadCommandsByActive()
{
	//先尝试连接集中器 
	if(m_bInitPortSuccess)
	{
		//
		if(SendConnectFirstStep())
		{

		}
	}

}

HANDLE CKaixingRTUCtrl::GetSendCmdEvent()
{
	return m_sendCmdEvent;
}

void CKaixingRTUCtrl::SetSendCmdEvent()
{
	m_oleSendCmdTime = COleDateTime::GetCurrentTime();
	SetEvent(m_sendCmdEvent);
}

bool CKaixingRTUCtrl::SendConnectFirstStep()
{
	if(m_bInitPortSuccess)
	{
		//准备命令
		ZeroMemory(m_bSendCmd, sizeof(m_bSendCmd));
		m_bSendCmd[0] = 0x53;
		m_bSendCmd[1] = 0x07;
		m_bSendCmd[2] = 0x55;
		m_bSendCmd[3] = 0x5A;
		m_bSendCmd[4] = 0x55;
		m_bSendCmd[5] = 0x5A;
		m_bSendCmd[6] = 0x55;
		m_bSendCmd[7] = 0x5A;

		//连接集中器第一步
		m_nEvent = 1;
		int nTryCount = 3;
		while(m_bConnectFirstSuccess == false && nTryCount>0)
		{
			SendCmd(m_bSendCmd,8);
			WaitForSingleObject(GetSendCmdEvent(), 2000);
			nTryCount--;
		}

		return m_bConnectFirstSuccess;
	}
	return false;
}

void CKaixingRTUCtrl::SendCmd( char *pData, BYTE bLength )
{
	if(m_bConnectOK)
	{
		EnterCriticalSection(&m_CriticalSection);
		WriteToPort(pData, bLength);
		LeaveCriticalSection(&m_CriticalSection);
	}
}

bool CKaixingRTUCtrl::SendConnectSecondStep()
{
	if(m_bInitPortSuccess)
	{
		if(m_nConcentratorType = 115)
		{

		}
		else if(m_nConcentratorType = 215)
		{

		}



		//准备命令
		ZeroMemory(m_bSendCmd, sizeof(m_bSendCmd));
		m_bSendCmd[0] = 0x53;
		m_bSendCmd[1] = 0x07;
		m_bSendCmd[2] = 0x55;
		m_bSendCmd[3] = 0x5A;
		m_bSendCmd[4] = 0x55;
		m_bSendCmd[5] = 0x5A;
		m_bSendCmd[6] = 0x55;
		m_bSendCmd[7] = 0x5A;

		//连接集中器第一步
		int nTryCount = 3;
		while(m_bConnectFirstSuccess == false && nTryCount>0)
		{
			SendCmd(m_bSendCmd,8);
			WaitForSingleObject(GetSendCmdEvent(), 2000);
			nTryCount--;
		}

		return m_bConnectFirstSuccess;
	}
	return false;
}
