#pragma once
#include "StdAfx.h"
#include "FCBusTcpCtrl.h"
#include "atlstr.h"

#include "Tools/Maths/MathCalcs.h"
#include "Tools/EngineInfoDefine.h"
//#include "Tools/vld.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../LAN_WANComm/PingIcmp.h"

const int default_portnumber = 5050;

CFCBusTcpCtrl::CFCBusTcpCtrl(void)
	:m_host("")
	,m_port(default_portnumber)
	,m_hsendthread(NULL)
	,m_nSendReadCmdTimeInterval(30)
	,m_hDataCheckRetryConnectionThread(NULL)
	,m_nFCBusNetworkErrorCount(0)
	,m_bExitThread(false)
	,m_nPrecision(6)
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_strErrInfo = "";
	m_nEngineIndex = 0;
}

CFCBusTcpCtrl::~CFCBusTcpCtrl(void)
{
	if (m_hsendthread != NULL)
	{
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
}

//golding ,设置发送读取命令的毫秒数
void    CFCBusTcpCtrl::SetSendCmdTimeInterval(int nTimeMs,int nPrecision)
{
	m_nSendReadCmdTimeInterval = nTimeMs;
	m_nPrecision = nPrecision;
}

// notify received package, use for sub-class derivativation
bool CFCBusTcpCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
{
	//02 0E 01 16 4D 00 00 00 00 00 00 00 00 00 00 56
	//02 06 01 0F 07 20 6D 40
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	DWORD wHeadAddr=0, wState=0, wValue=0;
	
	if(length == _FCBTCP_PD_CMD_LENTH_)			//过程数据
	{
		if(pRevData[0] == 0x02 && pRevData[1] == 0x06)
		{
			//异或运算校验
			unsigned char chXOR;
			chXOR = 0;
			for(int i=0; i<7;++i)
			{
				chXOR ^= pRevData[i];
			}

			if(pRevData[7] != (BYTE)chXOR)						//BCC为数据控制字节(异或)
				return false;

			memcpy(&wHeadAddr, pRevData+2, 1);
			char szBuff[4] = {0};
			memcpy(szBuff, pRevData+3, 4);
			wValue = HexChartoDec(szBuff);
			DataPointEntry* entry = FindEntry(wHeadAddr);
			if (!entry){					
				return false;
			}
			double dval = entry->GetMultipleReadValue((double)wValue);
			entry->SetValue(dval);
			entry->SetUpdated();
			m_oleUpdateTime = COleDateTime::GetCurrentTime();
			return true;
		}
	}
	else if(length == _FCBTCP_PC_CMD_LENTH_)	//参数通道
	{
		if(pRevData[0] == 0x02 && pRevData[1] == 0x0E)
		{
			//异或运算校验
			unsigned char chXOR;
			chXOR = 0;
			for(int i=0; i<15;++i)
			{
				chXOR ^= pRevData[i];
			}

			if(pRevData[15] != (BYTE)chXOR)						//BCC为数据控制字节(异或)
				return false;
		
			char szBuff[4] = {0};
			memcpy(szBuff, pRevData+11, 4);
			memcpy(&wHeadAddr, pRevData+2, 1);
			wValue = HexChartoDec(szBuff);
			DataPointEntry* entry = FindEntry(wHeadAddr);
			if (!entry){					
				return false;
			}
			double dval = entry->GetMultipleReadValue((double)wValue);
			entry->SetValue(dval);
			entry->SetUpdated();
			m_oleUpdateTime = COleDateTime::GetCurrentTime();
			return true;
		}
	}
	return false;
}

/***************************************************
*@brief:初始化hashmap的信息
*@param: void
*@return: bool,true设置成功，false设置失败。
*@author: ZHW 2011-02-28
****************************************************/

/***************************************************
*@brief:FCBusTcp网络连接并通讯
*@param: u_short uPort端口, string strIp网络地址
*@return: bool,true设置成功，false设置失败。
*@author: ZHW 2011-02-24
****************************************************/
bool CFCBusTcpCtrl::Init()
{
	// if there is not modbus point, thread will not be created.
	if (m_pointlist.empty())
	{
		return false;
	}

	if (!TcpIpConnectComm(m_host, m_port))
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("connect fail");
		return false;
	}

	m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	if (!m_hsendthread)
		return false;

	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	if(!m_hDataCheckRetryConnectionThread)
		return false;
	m_strErrInfo = "";
	return true;
}

void    CFCBusTcpCtrl::SetNetworkError()
{
	if(m_nFCBusNetworkErrorCount<10000)
		m_nFCBusNetworkErrorCount++;
}

void    CFCBusTcpCtrl::ClearNetworkError()
{
	m_nFCBusNetworkErrorCount = 0;
}

/***************************************************
*@brief:TCP/IP通讯发送数据[读指令]
*@param:
  WORD wAddress		//读取变频器地址
*@return: bool,true发送成功，false发送失败。
*@author: Robert 2014-11-25
****************************************************/

bool CFCBusTcpCtrl::SendReadCmd( WORD dAddress )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);

	ZeroMemory(m_SendPCBuffer, sizeof(m_SendPCBuffer));
	m_SendPCBuffer[0] = 0x02;					//起始字符 (STX)=02 Hex
	m_SendPCBuffer[1] = 0x0E;					//报文长度 (LGE)
	m_SendPCBuffer[2] = (BYTE)dAddress;			//变频器 地址 (ADR)
	m_SendPCBuffer[3] = 0x16;					//PKE占用两个字节，包括参数命令类型和参数数目;
	m_SendPCBuffer[4] = 0x4D;
	m_SendPCBuffer[5] = 0x00;					//IND为索引 在读命令中必须具有0400H的格式，在写命令中必须具有0500H的格式;
	m_SendPCBuffer[6] = 0x00;
	m_SendPCBuffer[7] = 0x00;					//PWE H
	m_SendPCBuffer[8] = 0x00;
	m_SendPCBuffer[9] = 0x00;					//PWE L
	m_SendPCBuffer[10] = 0x00;				
	m_SendPCBuffer[11] = 0x00;					//PCD1 控制字
	m_SendPCBuffer[12] = 0x00;
	m_SendPCBuffer[13] = 0x00;					//PCD2 参考值
	m_SendPCBuffer[14] = 0x00;

	//异或运算
	unsigned char chXOR;
	chXOR = 0;
	for(int i=0; i<15;++i)
	{
		chXOR ^= m_SendPCBuffer[i];
	}
	m_SendPCBuffer[15] = (BYTE)chXOR;					//BCC为数据控制字节(异或)

	//SendMessage
	return SendPackage(m_SendPCBuffer, sizeof(m_SendPCBuffer));
}

/**********************************************************************
*@brief:TCP/IP通讯发送数据[写指令]
*@param:  WORD wAddress			//写入属性的地址
, WORD wValue					//属性值
, bool bOpened);				//开关，默认为开
*@return: bool,true成功，false失败
*@author: Robert 2014-11-25
***********************************************************************/
bool CFCBusTcpCtrl::SendWriteCmd( WORD wAddress, WORD wValue, bool bOpened)
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);

	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序
	ZeroMemory(m_SendPDBuffer, sizeof(m_SendPDBuffer));
	m_SendPDBuffer[0] = 0x02;					//起始字符 (STX)=02 Hex
	m_SendPDBuffer[1] = 0x06;					//报文长度 (LGE) 06?不是长度08吗？
	m_SendPDBuffer[2] = (BYTE)wAddress;			//变频器 地址 (ADR)
	if(bOpened)
	{
		m_SendPDBuffer[3] = 0x04;					//PKE占用两个字节，包括参数命令类型和参数数目;
		m_SendPDBuffer[4] = 0x7C;
	}
	else
	{
		m_SendPDBuffer[3] = 0x16;					//PKE占用两个字节，包括参数命令类型和参数数目;  关
		m_SendPDBuffer[4] = 0x74;
	}

	memcpy(byteToEx, &wValue, 2);
	m_SendPDBuffer[5] = byteToEx[1];					//值
	m_SendPDBuffer[6] = byteToEx[0];
	//异或运算
	unsigned char chXOR;
	chXOR = 0;
	for(int i=0; i<7;++i)
	{
		chXOR ^= m_SendPDBuffer[i];
	}
	m_SendPDBuffer[7] = (BYTE)chXOR;					//BCC为数据控制字节(异或)

	//SendMessage
	return SendPackage(m_SendPDBuffer, sizeof(m_SendPDBuffer));
}

bool CFCBusTcpCtrl::SendWriteCmd( WORD wAddress ,DWORD wValue )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);

	BYTE byteToEx[4] = {0x00, 0x00,0x00, 0x00};	//内存顺序
	ZeroMemory(m_SendPDBuffer, sizeof(m_SendPDBuffer));
	m_SendPDBuffer[0] = 0x02;					//起始字符 (STX)=02 Hex
	m_SendPDBuffer[1] = 0x06;					//报文长度 (LGE) 06?不是长度08吗？
	m_SendPDBuffer[2] = (BYTE)wAddress;			//变频器 地址 (ADR)
	
	memcpy(byteToEx, &wValue, 4);
	m_SendPDBuffer[3] = byteToEx[3];			//PKE占用两个字节，包括参数命令类型和参数数目;
	m_SendPDBuffer[4] = byteToEx[2];
	m_SendPDBuffer[5] = byteToEx[1];					//值
	m_SendPDBuffer[6] = byteToEx[0];
	//异或运算
	unsigned char chXOR;
	chXOR = 0;
	for(int i=0; i<7;++i)
	{
		chXOR ^= m_SendPDBuffer[i];
	}
	m_SendPDBuffer[7] = (BYTE)chXOR;					//BCC为数据控制字节(异或)

	//SendMessage
	return SendPackage(m_SendPDBuffer, sizeof(m_SendPDBuffer));
}

void CFCBusTcpCtrl::SetHost( const string& strhost )
{
	m_host = strhost;
}

void CFCBusTcpCtrl::SetPort( u_short portnumer )
{
	m_port = portnumer;
}

UINT CFCBusTcpCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CFCBusTcpCtrl* pthis = (CFCBusTcpCtrl*)lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			pthis->SetReadCommands();	
			Sleep(2000);
		}
	}

	return 0;
}

void CFCBusTcpCtrl::TerminateSendThread()
{
	m_bExitThread = true;
	WaitForSingleObject(m_hsendthread, INFINITE);
	WaitForSingleObject(m_hDataCheckRetryConnectionThread,INFINITE);
}

void CFCBusTcpCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CFCBusTcpCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry&  modbusentry = m_pointlist[i];
		if(m_pointlist[i].GetUpdateSignal()<10)
		{
			wstring strValueSet = Project::Tools::RemoveDecimalW(modbusentry.GetValue(),m_nPrecision);
			valuelist.push_back(make_pair(modbusentry.GetShortName(), strValueSet));
		}
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_FCBUS,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
		strName.Format(_T("%s%d"),ERROR_FCBUS,m_nEngineIndex);
		//valuelist.push_back(make_pair(strName, Project::Tools::AnsiToWideChar(m_strErrInfo.c_str())));
	}
}

DataPointEntry* CFCBusTcpCtrl::FindEntry(DWORD headaddress)
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetFCPointAddress() == headaddress)
		{	
			return &m_pointlist[i];
		}
	}

	return NULL;
}

bool	CFCBusTcpCtrl::SetValue(const wstring& pointname, double fValue)
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			DWORD headaddress = entry.GetFCPointAddress();
			fValue =  entry.GetMultipleReadValue(fValue,false);			//edit by robert

			bool bSent = SendWriteCmd(headaddress,(DWORD)(fValue));
			if(!bSent)//这里才是网络错误
			{
				PrintLog(L"ERROR: FCBus write point send package failed.\r\n",false);
				SetNetworkError();
			}
			return true;
		}
	}

	return false;
}

bool CFCBusTcpCtrl::GetValue( const wstring& pointname, double& result )
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

bool CFCBusTcpCtrl::InitDataEntryValue(const wstring& pointname, double value)
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

void CFCBusTcpCtrl::AddLog( const wchar_t* loginfo )
{
	if (m_logsession){
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

void CFCBusTcpCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CFCBusTcpCtrl::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	PrintLog(L"INFO : FCBbus reconnecting...\r\n",false);
	Disconnect(); //tcpip recv thread stop, and clear socket

	if (!TcpIpConnectComm(m_host, m_port))
	{
		PrintLog(L"ERROR: FCBbus reconnecting failed!\r\n");
		m_strErrInfo = Project::Tools::OutTimeInfo("reconnect fail");
		return false;
	}
	m_strErrInfo = "";
	PrintLog(L"INFO : FCBbus reconnecting success!\r\n",false);
	return true;
}

UINT WINAPI CFCBusTcpCtrl::ThreadCheckAndRetryConnection(LPVOID lparam)
{
	CFCBusTcpCtrl* pthis = (CFCBusTcpCtrl*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(pthis->m_nFCBusNetworkErrorCount>=60)
			{
				if(pthis->ReConnect())
					pthis->ClearNetworkError();
			}
			Sleep(5000);
		}
	}

	return 0;
}

bool CFCBusTcpCtrl::Exit()
{
	m_bExitThread = true;
	Disconnect();		//关闭TCP
	if(m_hsendthread)
		WaitForSingleObject(m_hsendthread, INFINITE);
	if(m_hDataCheckRetryConnectionThread)
		WaitForSingleObject(m_hDataCheckRetryConnectionThread,INFINITE);
	return true;
}

void CFCBusTcpCtrl::SetReadCommands()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(m_sockConnected==NULL)
		return;

	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];
		bool bSent = SendReadCmd(entry.GetFCPointAddress());
		if(!bSent)
		{
			m_strErrInfo = Project::Tools::OutTimeInfo("send cmd fail");
			SetNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

DWORD CFCBusTcpCtrl::HexChartoDec( char address[4] )
{
	DWORD hex=0;
	DWORD d1 = address[3];
	DWORD d2 = address[2]*256;
	DWORD d3 = address[1]*65536;
	DWORD d4 = address[0]*256*65536;
	hex = address[3] + address[2]*256 + address[1]*65536 + address[0]*256*65536;
	return hex;
}

void CFCBusTcpCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

void CFCBusTcpCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
