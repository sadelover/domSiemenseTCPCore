#pragma once
#include "StdAfx.h"
#include "TCPDataSender.h"
#include "atlstr.h"

#include <iostream>
#include <sstream>

const int default_portnumber = 9500;

const int SEND_PACKAGE_INTERNAL =100;	//连续发包的间隔时间
const int SEND_UNIT01_INTERNAL = 2*1000;	//连续发包的间隔时间(间隔时间过短会崩溃)
const int PACKAGE_SIZE = 4000;				//分包时最大包长。
const int MAX_POINT_LENGTH = 100;			//点名最大长度

CTCPDataSender::CTCPDataSender()
	:m_host("")
	,m_port(default_portnumber)
	,m_hDataCheckRetryConnectionThread(NULL)
	,m_nModbusNetworkErrorCount(0)
	,m_bExitThread(false)
	,m_bConnectSuccess(false)
	,m_bSendTcpNameSuccess(false)
	,m_strSendSuccessBuffer("")
	,m_strTcpName("")
	,m_nWriteSize(0)
	,m_nReConnectInterval(60)
	,m_szWriteBuffer(NULL)
{
	ZeroMemory(m_cRecvBuf,g_nRecMaxNum);
	m_nRecSize = 0;
	ZeroMemory(m_cSpiltBuffer,g_nRecMaxNum);
	m_oleLastWriteTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
	m_oleActiveTime = COleDateTime::GetCurrentTime();
}

CTCPDataSender::~CTCPDataSender()
{
	m_bExitThread = true;
	m_bConnectSuccess = false;
	if (m_hDataCheckRetryConnectionThread != NULL)
	{
		::CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
}

bool CTCPDataSender::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	HandleRecData((char*)pRevData,length);
	m_oleActiveTime = COleDateTime::GetCurrentTime();
	return true;
}

void CTCPDataSender::SetHost(string strhost )
{
	m_host = strhost;
}

void CTCPDataSender::SetPort( u_short portnumer )
{
	m_port = portnumer;
}

void CTCPDataSender::SetName( string strname )
{
	m_strTcpName = strname;
}

bool CTCPDataSender::Init()
{
	if (!TcpIpConnectComm(m_host, m_port))
	{
		_tprintf(L"TCPDataSender connecting failed!\r\n");
		return false;
	}

	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	if(!m_hDataCheckRetryConnectionThread)
		return false;

	m_bConnectSuccess = true;
	SendProjectName();
	return true;
}

bool CTCPDataSender::Exit()
{
	m_bExitThread = true;
	m_bConnectSuccess = false;
	Disconnect();		//关闭TCP
	if(m_hDataCheckRetryConnectionThread)
	{
		WaitForSingleObject(m_hDataCheckRetryConnectionThread,2000);

		CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
	return true;
}

UINT WINAPI CTCPDataSender::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CTCPDataSender* pthis = (CTCPDataSender*) lparam;
	if (pthis != NULL)
	{
		int nSleepInterval = pthis->m_nReConnectInterval;
		while (!pthis->m_bExitThread)
		{
			if(pthis->m_bConnectSuccess && pthis->m_bConnectOK)			//连接上后 5分钟未收发数据重连
			{
				COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - pthis->m_oleActiveTime;
				if(oleSpan.GetTotalMinutes() >= 5)
				{
					if(pthis->ReConnect())
						pthis->ClearNetworkError();

					int nSleep = nSleepInterval;
					while(!pthis->m_bExitThread)
					{
						if(nSleep <= 0)
						{
							break;
						}
						nSleep--;
						Sleep(1000);
					}
				}			
			}

			if(!pthis->m_bConnectSuccess || !pthis->m_bConnectOK ||pthis->m_nModbusNetworkErrorCount>=5)
			{
				if(pthis->ReConnect())
					pthis->ClearNetworkError();
			}

			int nSleep = nSleepInterval;
			while(!pthis->m_bExitThread)
			{
				if(nSleep <= 0)
				{
					break;
				}
				nSleep--;
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool CTCPDataSender::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(m_bConnectSuccess)
	{
		if(!CloseTcp())
			return false;
	}

	if (!TcpIpConnectComm(m_host, m_port))
	{
		_tprintf(L"ERROR:TCP Engine reconnecting failed!\r\n");
		return false;
	}
	m_bConnectSuccess = true;
	SendProjectName();
	_tprintf(L"TCP Engine reconnecting success!\r\n");
	return true;
}

void CTCPDataSender::SetNetworkError()
{
	if(m_nModbusNetworkErrorCount<10000)
		m_nModbusNetworkErrorCount++;
}

void CTCPDataSender::ClearNetworkError()
{
	m_nModbusNetworkErrorCount = 0;
}

bool CTCPDataSender::InitTcp( string strHost,u_short portnumer,string strTCPName,int nReConnectInterval /*= 60*/ ,UINT writebuffersize/*= 1024*/)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	m_strTcpName = strTCPName;
	m_port = portnumer;
	m_host = strHost;
	m_nReConnectInterval = nReConnectInterval;
	m_nModbusNetworkErrorCount = 0;
	if(m_bConnectSuccess)
	{
		if(!CloseTcp())
			return false;
	}

	// 设置读写缓存
	if (m_szWriteBuffer != NULL)
	{
		delete []m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	m_szWriteBuffer = new char[g_nRecMaxNum];
	m_nWriteBufferSize = writebuffersize;

	if (!TcpIpConnectComm(m_host, m_port))
	{
		_tprintf(L"TCPDataSender connecting failed!\r\n");
		return false;
	}
	m_bConnectSuccess = true;
	SendProjectName();
	return true;
}

bool CTCPDataSender::CloseTcp()
{
	m_bConnectSuccess = false;
	m_nModbusNetworkErrorCount = 0;
	Disconnect();		//关闭TCP
	return true;
}

BOOL CTCPDataSender::StartMonitoring( LPRecDataProc proc1/*=NULL*/,DWORD userdata/*=NULL*/ )
{
	m_lpRecDataProc = proc1;
	m_dwUserData = userdata;

	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	if(!m_hDataCheckRetryConnectionThread)
		return FALSE;
	TRACE("Thread started\n");  
	return TRUE;
}

BOOL CTCPDataSender::RestartMonitoring()
{
	printf("Thread resumed\n");
	::ResumeThread(m_hDataCheckRetryConnectionThread);
	return TRUE;	
}

BOOL CTCPDataSender::StopMonitoring()
{
	printf("Thread suspended\n");
	::SuspendThread(m_hDataCheckRetryConnectionThread); 
	return TRUE;	
}

bool CTCPDataSender::WriteToPort( char *pData,int nLen )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_send);
	if(!m_bConnectSuccess)
		return false;

	if(nLen >= 4050)
		nLen = 4050;

	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, pData, nLen);
	m_nWriteSize = nLen;
	bool bResult = SendPackage(m_szWriteBuffer,nLen);
	m_oleLastWriteTime = COleDateTime::GetCurrentTime();
	if(!bResult)
	{
		m_nModbusNetworkErrorCount++;
	}
	else
	{
		m_strSendSuccessBuffer = m_szWriteBuffer;
		m_nModbusNetworkErrorCount = 0;
		m_oleActiveTime = COleDateTime::GetCurrentTime();
	}
	return bResult;
}

bool CTCPDataSender::WriteToPortByWrap( char *pData,int nLen )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_send);
	if(!m_bConnectSuccess)
		return false;

	if(nLen >= 4050)
		nLen = 4050;

	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, pData, nLen);
	m_szWriteBuffer[nLen-2] = ';';			//以;;\n为包结尾
	m_szWriteBuffer[nLen-1] = '\n';
	m_nWriteSize = nLen;
	bool bResult = SendPackage(m_szWriteBuffer,nLen);
	m_oleLastWriteTime = COleDateTime::GetCurrentTime();
	if(!bResult)
	{
		m_nModbusNetworkErrorCount++;
	}
	else
	{
		m_strSendSuccessBuffer = m_szWriteBuffer;
		m_nModbusNetworkErrorCount = 0;
		m_oleActiveTime = COleDateTime::GetCurrentTime();
	}
	return bResult;
}

bool CTCPDataSender::HandleRecData( const char* buffer,int nSize )
{
	if(buffer == NULL)
		return false;
	int nLength  = m_nRecSize + nSize;
	if(nLength <= g_nRecMaxNum)
	{
		memcpy(m_cRecvBuf+m_nRecSize,buffer,nSize);
		m_nRecSize = nLength;
	}
	else
	{
		ZeroMemory(m_cRecvBuf,g_nRecMaxNum);
		m_nRecSize = 0;

		memcpy(m_cRecvBuf,buffer,nSize);
		m_nRecSize = nSize;
	}

	vector<datapackage> vecPackage;
	SplitStringSpecial(m_cRecvBuf,m_nRecSize,vecPackage);
	int nCount = vecPackage.size();
	for(int i=0; i<nCount; ++i)
	{
		int length = vecPackage[i].nLength;
		if(length > 0)
		{
			char* p = vecPackage[i].cData; 
			if(i<nCount-1)
			{
				m_lpRecDataProc((unsigned char *)p,length,m_dwUserData);
			}
			else
			{
				if(m_cRecvBuf[m_nRecSize-1] == '\n')
				{
					m_lpRecDataProc((unsigned char *)p,length,m_dwUserData);
					ZeroMemory(m_cRecvBuf,g_nRecMaxNum);
					m_nRecSize = 0;
				}
				else
				{
					ZeroMemory(m_cRecvBuf,g_nRecMaxNum);
					memcpy(m_cRecvBuf,p,length);
					m_nRecSize = length;
				}
			}
		}
	}
	return true;
}

void CTCPDataSender::SplitStringSpecial( const char* buffer,int nSize, std::vector<datapackage>& resultlist )
{
	if(nSize <= 0)
		return;

	resultlist.clear();
	memset(m_cSpiltBuffer,0,g_nRecMaxNum);
	memcpy(m_cSpiltBuffer,buffer,nSize);

	char cBefore1 = 0x00;
	char cBefore2 = 0x00;
	int nBefore = 0;
	int nAfter = 0;
	for(int i=0; i<nSize; ++i)
	{
		if(buffer[i] == '\n'  && cBefore2 == ';' && cBefore1 == ';')			//以;;\n结尾
		{
			datapackage data;
			int length = i+1-nBefore;
			length = (length>g_nPackageSize)?g_nPackageSize:length;
			memcpy(data.cData,m_cSpiltBuffer+nBefore,length);		
			data.nLength = length;
			resultlist.push_back(data);
			nBefore = i+1;
		}
		cBefore2 = cBefore1;
		cBefore1 = buffer[i];
	}

	int nLength = nSize - nBefore;
	if(nLength>0)
	{
		datapackage data;
		int length = (nLength>g_nPackageSize)?g_nPackageSize:nLength;
		memcpy(data.cData,m_cSpiltBuffer+nBefore,length);	
		data.nLength = length;
		resultlist.push_back(data);
	}
}

bool CTCPDataSender::GetDTUIdleState( int nSeconds /*= 4*/ )
{
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleLastWriteTime;
	if(oleSpan.GetTotalSeconds() >= nSeconds)
		return true;
	return false;
}

std::string CTCPDataSender::GetSuccessBuffer()
{
	return m_strSendSuccessBuffer;
}

bool CTCPDataSender::GetTCPConnectOK()
{
	return m_bConnectSuccess;
}

bool CTCPDataSender::SendProjectName()
{
	bool bResult = true;
	string strdata = "reg:";
	strdata += m_strTcpName;
	strdata += ";";
	bResult = WriteToPortByWrap((char*)strdata.data(), strdata.size()+2);
	return bResult;
}

void CTCPDataSender::ReSet( string strHost,u_short portnumer,string strTCPName,int nReConnectInterval /*= 30*/,UINT writebuffersize /*= 1024*/ )
{
	m_host = strHost;
	m_port = portnumer;
	m_strTcpName = strTCPName;
	ReConnect();
}
