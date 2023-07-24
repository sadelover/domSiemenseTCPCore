#pragma once
#include "StdAfx.h"
#include "DogTcpCtrl.h"
#include "atlstr.h"

#include <iostream>
#include <sstream>

const int default_portnumber = 9500;
const int min_length = 7;
const char* REGCMD_PREFIX = "regcmd:";
const char* RESCMD_PREFIX = "rescmd:";
const char* REGACK_PREFIX = "regack:";
const char* RESACK_PREFIX = "resack:";

CDogTcpCtrl* CDogTcpCtrl::m_csDogCommObj = NULL;
Mutex CDogTcpCtrl::m_mutexLock;

CDogTcpCtrl::CDogTcpCtrl()
	:m_host("")
	,m_port(default_portnumber)
	,m_hDataCheckRetryConnectionThread(NULL)
	,m_nModbusNetworkErrorCount(0)
	,m_bExitThread(false)
	,m_bConnectSuccess(false)
	,m_nWriteSize(0)
	,m_nReConnectInterval(10)
	,m_szWriteBuffer(NULL)
	,m_bRestartCoreSuccess(false)
	,m_bRestartWatchSuccess(false)
	,m_bRestartLogicSuccess(false)
	,m_bRestartUpdateSuccess(false)
	,m_bRestartMasterSuccess(false)
	,m_nMode(E_SLAVE_CORE)
{
	ZeroMemory(m_cRecvBuf,g_nDogRecMaxNum);
	m_nRecSize = 0;
	ZeroMemory(m_cSpiltBuffer,g_nDogRecMaxNum);
	m_host = GetHostIP();

	//读取服务器数据库
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\domcore.ini";

	wchar_t charLocalPort[1024];
	GetPrivateProfileString(L"master", L"MasterTCPPort", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrLocalPort = charLocalPort;
	if(wstrLocalPort == L"" || wstrLocalPort == L"0")
		wstrLocalPort = L"9502";
	WritePrivateProfileString(L"master", L"MasterTCPPort",wstrLocalPort.c_str(),exepath.c_str());
	m_port = _wtoi(wstrLocalPort.c_str());   

	GetPrivateProfileString(L"master", L"MasterVersion", L"", charLocalPort, 1024, exepath.c_str());
	m_strMasterVersion = charLocalPort;
	WritePrivateProfileString(L"master", L"MasterVersion",m_strMasterVersion.c_str(),exepath.c_str());
}

CDogTcpCtrl::~CDogTcpCtrl()
{
	m_bExitThread = true;
	m_bConnectSuccess = false;
	if (m_hDataCheckRetryConnectionThread != NULL)
	{
		::CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
}

bool CDogTcpCtrl::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	HandleRecData((char*)pRevData,length);
	return true;
}

bool CDogTcpCtrl::Init(bool bConnected)
{
	// 设置读写缓存
	if (m_szWriteBuffer != NULL)
	{
		delete []m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	m_szWriteBuffer = new char[g_nDogRecMaxNum];

	if(bConnected)
	{
		if (!TcpIpConnectComm(m_host, m_port))
		{
			m_bConnectSuccess = false;
		}
	}

	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	m_bConnectSuccess = true;
	//SendActiveSignal();
	return true;
}

bool CDogTcpCtrl::Exit()
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
	if (m_szWriteBuffer != NULL)
	{
		delete []m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	return true;
}

UINT WINAPI CDogTcpCtrl::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CDogTcpCtrl* pthis = (CDogTcpCtrl*) lparam;
	if (pthis != NULL)
	{
		int nSleepInterval = pthis->m_nReConnectInterval;
		while (!pthis->m_bExitThread)
		{
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

bool CDogTcpCtrl::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(m_bConnectSuccess)
	{
		if(!CloseTcp())
			return false;
	}

	if (!TcpIpConnectComm(m_host, m_port))
	{
		return false;
	}
	
	m_bConnectSuccess = true;
	SendActiveSignal();
	return true;
}

void CDogTcpCtrl::ClearNetworkError()
{
	m_nModbusNetworkErrorCount = 0;
}

bool CDogTcpCtrl::CloseTcp()
{
	m_bConnectSuccess = false;
	m_nModbusNetworkErrorCount = 0;
	Disconnect();		//关闭TCP
	return true;
}

bool CDogTcpCtrl::WriteToPort( char *pData,int nLen )
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
	if(!bResult)
	{
		m_nModbusNetworkErrorCount++;
	}
	else
	{
		m_nModbusNetworkErrorCount = 0;
	}
	return bResult;
}

bool CDogTcpCtrl::WriteToPortByWrap( char *pData,int nLen )
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
	if(!bResult)
	{
		m_nModbusNetworkErrorCount++;
	}
	else
	{
		m_nModbusNetworkErrorCount = 0;
	}
	return bResult;
}

bool CDogTcpCtrl::HandleRecData( const char* buffer,int nSize )
{
	if(buffer == NULL)
		return false;
	int nLength  = m_nRecSize + nSize;
	if(nLength <= g_nDogRecMaxNum)
	{
		memcpy(m_cRecvBuf+m_nRecSize,buffer,nSize);
		m_nRecSize = nLength;
	}
	else
	{
		ZeroMemory(m_cRecvBuf,g_nDogRecMaxNum);
		m_nRecSize = 0;

		memcpy(m_cRecvBuf,buffer,nSize);
		m_nRecSize = nSize;
	}

	vector<dogpackage> vecPackage;
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
				AnalyzeReceive((unsigned char *)p,length);
			}
			else
			{
				if(m_cRecvBuf[m_nRecSize-1] == '\n')
				{
					AnalyzeReceive((unsigned char *)p,length);
					ZeroMemory(m_cRecvBuf,g_nDogRecMaxNum);
					m_nRecSize = 0;
				}
				else
				{
					ZeroMemory(m_cRecvBuf,g_nDogRecMaxNum);
					memcpy(m_cRecvBuf,p,length);
					m_nRecSize = length;
				}
			}
		}
	}
	return true;
}

void CDogTcpCtrl::SplitStringSpecial( const char* buffer,int nSize, std::vector<dogpackage>& resultlist )
{
	if(nSize <= 0)
		return;

	resultlist.clear();
	memset(m_cSpiltBuffer,0,g_nDogRecMaxNum);
	memcpy(m_cSpiltBuffer,buffer,nSize);

	char cBefore1 = 0x00;
	char cBefore2 = 0x00;
	int nBefore = 0;
	int nAfter = 0;
	for(int i=0; i<nSize; ++i)
	{
		if(buffer[i] == '\n'  && cBefore2 == ';' && cBefore1 == ';')			//以;;\n结尾
		{
			dogpackage data;
			int length = i+1-nBefore;
			length = (length>g_nDogPackageSize)?g_nDogPackageSize:length;
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
		dogpackage data;
		int length = (nLength>g_nDogPackageSize)?g_nDogPackageSize:nLength;
		memcpy(data.cData,m_cSpiltBuffer+nBefore,length);	
		data.nLength = length;
		resultlist.push_back(data);
	}
}

CDogTcpCtrl* CDogTcpCtrl::GetInstance(bool bConnected)
{
	if (m_csDogCommObj == NULL)
	{
		Scoped_Lock<Mutex> mut(m_mutexLock);
		m_csDogCommObj = new CDogTcpCtrl();
		if(m_csDogCommObj != NULL)
		{
			m_csDogCommObj->Init(bConnected);
		}
		atexit(DestroyInstance);
	}

	return m_csDogCommObj;
}

void CDogTcpCtrl::DestroyInstance()
{
	if (m_csDogCommObj != NULL)
	{
		m_csDogCommObj->Exit();
		delete m_csDogCommObj;
		m_csDogCommObj = NULL;
	}
}

bool CDogTcpCtrl::SendActiveSignal()
{
	std::ostringstream sqlstream;
	switch(m_nMode)
	{
	case E_SLAVE_CORE:
		{
			sqlstream << "regcmd:core;";
			string strdata = sqlstream.str();
			return  WriteToPortByWrap((char*)strdata.data(), strdata.size()+2);
		}
		break;
	case E_SLAVE_LOGIC:
		{
			sqlstream << "regcmd:logic;";
			string strdata = sqlstream.str();
			return  WriteToPortByWrap((char*)strdata.data(), strdata.size()+2);
		}
		break;
	case E_SLAVE_UPDATE:
		{
			sqlstream << "regcmd:update;";
			string strdata = sqlstream.str();
			return  WriteToPortByWrap((char*)strdata.data(), strdata.size()+2);
		}
		break;
	case E_SLAVE_DTUENGINE:
		{
			sqlstream << "regcmd:dtuengine;";
			string strdata = sqlstream.str();
			return  WriteToPortByWrap((char*)strdata.data(), strdata.size()+2);
		}
		break;
	default:
		return false;
	}
	return false;
}

bool CDogTcpCtrl::SendRestart( int nRestartExe , int nRestartType)
{
	std::ostringstream sqlstream;
	sqlstream << "rescmd:" << nRestartExe << "|" << nRestartType << ";";
	string strdata = sqlstream.str();
	return WriteToPortByWrap((char*)strdata.data(), strdata.size()+2);
}

bool CDogTcpCtrl::SendRestart( E_SLAVE_MODE nRestartExe, E_SLAVE_REASON nReason )
{
	std::ostringstream sqlstream;
	sqlstream << "rescmd:" << nRestartExe << "|" << nReason << ";";
	string strdata = sqlstream.str();
	return WriteToPortByWrap((char*)strdata.data(), strdata.size()+2);
}

bool CDogTcpCtrl::AnalyzeReceive( const unsigned char* pRevData, DWORD length )
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_reveivelock);
	if(length <= min_length)
		return false;

	//;;\n结尾
	if(pRevData[length-1] == '\n' && pRevData[length-2] == ';')
	{
		char buffer_prefix[min_length+1];
		memcpy(buffer_prefix, pRevData, min_length);
		buffer_prefix[min_length] = '\0';

		if(strcmp(buffer_prefix, REGACK_PREFIX) == 0)		//心跳包
		{
			return true;
		}
		if(strcmp(buffer_prefix, RESACK_PREFIX) == 0)		//重启
		{
			char buffer_content[g_nDogRecMaxNum] = {0};
			int nLength = ((length-min_length)>g_nDogRecMaxNum)?g_nDogRecMaxNum:(length-min_length);
			memcpy(buffer_content, pRevData+min_length, nLength);
			char* pContent = strtok((char*)buffer_content, ";");
			vector<string>	vecCmd;
			SplitString(pContent,",",vecCmd);			
			if(vecCmd.size() == 3)
			{
				if(vecCmd[0] == "1")					//重启Core成功
				{
					if(vecCmd[2] == "1")
					{
						m_bRestartCoreSuccess = true;
					}
					else
					{
						m_bRestartCoreSuccess = false;
					}
				}
				else if(vecCmd[0] == "2")				//重启Logic成功
				{
					if(vecCmd[2] == "1")
					{
						m_bRestartLogicSuccess = true;
					}
					else
					{
						m_bRestartLogicSuccess = false;
					}
				}
				else if(vecCmd[0] == "3")				//重启Update成功
				{
					if(vecCmd[2] == "1")
					{
						m_bRestartUpdateSuccess = true;
					}
					else
					{
						m_bRestartUpdateSuccess = false;
					}
				}
				else if(vecCmd[0] == "3")				//重启Master成功
				{
					if(vecCmd[2] == "1")
					{
						m_bRestartMasterSuccess = true;
					}
					else
					{
						m_bRestartMasterSuccess = false;
					}
				}
			}
			return true;
		}
		return true;
	}
	return false;
}

bool CDogTcpCtrl::GetRestartSuccess( bool& bRestartCore,bool& bRestartWatch )
{
	bRestartCore = m_bRestartCoreSuccess;
	bRestartWatch = m_bRestartWatchSuccess;
	return true;
}

bool CDogTcpCtrl::GetRestartSuccess( bool& bRestartCore,bool& bRestartLogic,bool& bRestartUpdate,bool& bRestartMaster )
{
	bRestartCore = m_bRestartCoreSuccess;
	bRestartLogic = m_bRestartLogicSuccess;
	bRestartUpdate = m_bRestartUpdateSuccess;
	bRestartMaster = m_bRestartMasterSuccess;
	return true;
}

bool CDogTcpCtrl::SetRestartCoreSuccess( bool bRestartCore )
{
	m_bRestartCoreSuccess = bRestartCore;
	return true;
}

bool CDogTcpCtrl::SetRestartWatchSuccess( bool bRestartWatch )
{
	m_bRestartWatchSuccess = bRestartWatch;
	return true;
}

string CDogTcpCtrl::GetHostIP()
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

bool CDogTcpCtrl::SetDogMode( E_SLAVE_MODE nMode)
{
	m_nMode = nMode;
	return true;
}

bool CDogTcpCtrl::SetRestartLogicSuccess( bool bRestartLogic )
{
	m_bRestartLogicSuccess = bRestartLogic;
	return true;
}

bool CDogTcpCtrl::SetRestartUpdateSuccess( bool bRestartUpdate )
{
	m_bRestartUpdateSuccess = bRestartUpdate;
	return true;
}

bool CDogTcpCtrl::SetRestartMasterSuccess( bool bRestartMaster )
{
	m_bRestartMasterSuccess = bRestartMaster;
	return true;
}

bool CDogTcpCtrl::GetConnectSuccess()
{
	return m_bConnectSuccess;
}

bool CDogTcpCtrl::GetDogRestartFunctonOK()
{
	if(m_bConnectSuccess && m_strMasterVersion >= L"V3.0.1")
		return true;
	return false;
}

