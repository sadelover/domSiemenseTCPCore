// TCPCustom_CE.cpp: implementation of the CTCPCustom class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TCPCustom.h"
#include <direct.h>
#include <io.h>
#include <string>
#include <sstream>
#include <algorithm>
//#include "../include/vld.h"

#define ACCESS _access
#define MKDIR(a) _mkdir((a))

const int default_portnumber = 9500;
const int min_length = 7;
const char* REGCMD_PREFIX = "regcmd:";
const char* RESCMD_PREFIX = "rescmd:";
const char* REGACK_PREFIX = "regack:";
const char* RESACK_PREFIX = "resack:";
const int DTU_ID_LENGTH = 128;						//DTU项目标识

const CString g_NAME_Core = _T("domcore.exe");
const CString g_NAME_Watch = _T("BEOPWatch.exe");

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//构造函数
CTCPCustom::CTCPCustom()
{
	m_eType = E_REASON_NULL;
	m_RemoteHost = _T("");
	m_RemotePort = 0;
	m_bOutRec = false;
	m_oleLastReceive = COleDateTime::GetCurrentTime();
	ZeroMemory(m_cRecvBuf,g_nRecMaxNum);
	m_nRecSize = 0;
	m_bOK = true;
	m_bexitthread = false;
	m_pTCPServer = NULL;
	m_datathread = NULL;
	m_exitThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL); 
	m_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	// 初始化临界区
	InitializeCriticalSection(&m_csTCPSocketSync);
	InitializeCriticalSection(&m_csDTUDataSync);
}

//析构函数
CTCPCustom::~CTCPCustom()
{
 
}

/*--------------------------------------------------------------------
【函数介绍】:  此线程用于监听与客户端连接的socket通讯的事件，例如当接收到数据、
			   连接断开和通讯过程发生错误等事件
【入口参数】:  lparam:无类型指针，可以通过此参数，向线程中传入需要用到的资源。
			   在这里我们将CTCPCustom类实例指针传进来
【出口参数】:  (无)
【返回  值】:  返回值没有特别的意义，在此我们将返回值设为0。
---------------------------------------------------------------------*/
DWORD WINAPI CTCPCustom::SocketThreadFunc(PVOID lparam)
{
	CTCPCustom *pSocket;
	//得到CTCPCustom类实例指针
	pSocket = (CTCPCustom*)lparam;
	//定义读事件集合
	fd_set fdRead;  
	int ret;
	TIMEVAL	aTime;
	aTime.tv_sec = 1;
	aTime.tv_usec = 0;
	while (TRUE)
	{
        //收到退出事件，结束线程
		if (WaitForSingleObject(pSocket->m_exitThreadEvent,0) == WAIT_OBJECT_0)
		{
			break;
		}
		//置空读事件集合
		FD_ZERO(&fdRead);
		//给pSocket设置读事件
		FD_SET(pSocket->m_socket,&fdRead);
		//调用select函数，判断是否有读事件发生
		ret = select(0,&fdRead,NULL,NULL,&aTime);

		if(pSocket->m_bOK == false)
			break;
		
		if (ret == SOCKET_ERROR)
		{
			pSocket->m_bOK = false;
			int nErrorCode = WSAGetLastError();   
			CString strErr;
			strErr.Format(_T("socket select err:%d\n"),nErrorCode);
			TRACE(strErr);
			break;   
		}
		
		if (ret > 0)
		{
			//判断是否读事件
			if (FD_ISSET(pSocket->m_socket,&fdRead))
			{
				char recvBuf[g_nRecAveNum];
				int recvLen;
				ZeroMemory(recvBuf,g_nRecAveNum);
				EnterCriticalSection(&(pSocket->m_csTCPSocketSync));
				recvLen = recv(pSocket->m_socket,recvBuf, g_nRecAveNum,0); 
				LeaveCriticalSection(&(pSocket->m_csTCPSocketSync));
				if (recvLen == SOCKET_ERROR)
				{
					pSocket->m_bOK = false;
					int nErrorCode = WSAGetLastError();   
					CString strErr;
					strErr.Format(_T("socket recv err:%d\n"),nErrorCode);
					TRACE(strErr);
					break;   
				}
				//表示连接已经从容关闭
				else if (recvLen == 0)
				{
					pSocket->m_bOK = false;
					TRACE(L"socket close good\n");
					break;   
				}
				else
				{
					if(pSocket->m_bOK)
					{						
						pSocket->HandleRecData(recvBuf,recvLen);
						if(recvLen > 1)
							pSocket->m_oleLastReceive = COleDateTime::GetCurrentTime();
					}
				}
			}
		}
	}
	return 0;
}

/*--------------------------------------------------------------------
【函数介绍】: 打开socket，创建通讯线程
【入口参数】:  pTCPServer指向服务器端监听socket
【出口参数】:  (无)
【返回  值】:  TRUE:打开成功;FALSE:打开失败
---------------------------------------------------------------------*/
bool CTCPCustom::Open(CTCPServer *pTCPServer)
{
	CString strEvent;   
	strEvent.Format(L"EVENT_CLIENT_THREAD_EXIT %d",m_socket);   
	//创建线程退出事件    
	ResetEvent(m_exitThreadEvent);
	m_oleLastReceive = COleDateTime::GetCurrentTime();

	//设置通讯模式为阻塞模式   否则发送时候容易出现10035的错误 
	DWORD ul= 0;   
	ioctlsocket(m_socket,FIONBIO,&ul);   
	m_pTCPServer = pTCPServer; 

	//创建通讯线程   
	m_tcpThreadHandle = CreateThread(NULL,0,SocketThreadFunc,this,0,NULL);   
	m_datathread = (HANDLE)_beginthreadex(NULL, 0 , ThreadFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	if (m_tcpThreadHandle == NULL)   
	{   
		m_bOK = false;
		closesocket(m_socket);   
		return FALSE;   
	}   
	m_bOK = true;
	return TRUE;   
}

/*--------------------------------------------------------------------
【函数介绍】: 关闭socket，关闭线程，释放Socket资源
【入口参数】:  (无)
【出口参数】:  (无)
【返回  值】:  TRUE:成功关闭;FALSE:关闭失败
---------------------------------------------------------------------*/
//bool CTCPCustom::Close()
//{
//	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
//
//	m_bOK = false;
//	//发送通讯线程结束事件   
//	SetEvent(m_exitThreadEvent);   
//
//	//触发与客户端端连接的Socket关闭事件   
//	if (m_pTCPServer && m_pTCPServer->OnClientClose)   
//	{   
//		m_pTCPServer->OnClientClose(m_pTCPServer->m_pOwner,this);   
//	}   
//
//	m_pTCPServer = NULL;
//
//	//等待1秒，如果读线程没有退出，则强制退出   
//	if (WaitForSingleObject(m_tcpThreadHandle,1000) == WAIT_TIMEOUT)   
//	{   
//		TerminateThread(m_tcpThreadHandle,0);   
//		TRACE(L"强制终止客户端线程\n");   
//	}   
//	m_tcpThreadHandle = NULL;   
//	//关闭Socket，释放资源   
//	int err = closesocket(m_socket);   
//	if (err == SOCKET_ERROR)   
//	{   
//		return FALSE;   
//	}   
//
//	TRACE(L"客户端对象被成功关闭\n");   
//	return TRUE;   
//}

//20150417 先要退出接收线程 再发关闭事件  再从CPtrList中移除
bool CTCPCustom::Close()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	m_bexitthread = true;
	m_bOK = false;
	//发送通讯线程结束事件   
	SetEvent(m_exitThreadEvent);   

	//等待1秒，如果读线程没有退出，则强制退出   
	if (WaitForSingleObject(m_tcpThreadHandle,2000) == WAIT_TIMEOUT)   
	{   
		if(m_tcpThreadHandle)
		{
			CloseHandle(m_tcpThreadHandle);
			m_tcpThreadHandle = NULL;   
		}
	}   

	if(m_exitThreadEvent)
	{
		CloseHandle(m_exitThreadEvent);
		m_exitThreadEvent = NULL;
	}

	//关闭Socket，释放资源 
	if(m_socket)
	{
		EnterCriticalSection(&m_csTCPSocketSync);
		int err = closesocket(m_socket);   
		LeaveCriticalSection(&m_csTCPSocketSync);
		if (err == SOCKET_ERROR)   
		{   
			return FALSE;   
		} 
	}  

	m_pTCPServer = NULL;
	return TRUE;   
}

/*-----------------------------------------------------------------
【函数介绍】: 向客户端发送数据
【入口参数】: buf:待发送的数据
              len:待发送的数据长度
【出口参数】: (无)
【返回  值】: TRUE:发送数据成功;FALSE:发送数据失败
------------------------------------------------------------------*/
bool CTCPCustom::SendData(const char * buf , int len)
{
	if(!m_bOK)
		return false;

	int nBytes = 0;   
	int nSendBytes=0;   

	while (nSendBytes < len)   
	{   
		nBytes = send(m_socket,buf+nSendBytes,len-nSendBytes,0);   
		if (nBytes==SOCKET_ERROR )   
		{   
			int iErrorCode = WSAGetLastError();   
			if(iErrorCode == 10035)
			{
				Sleep(1000);
				SendData(buf,len);
				break;
			}
			else
			{
				m_bOK = false;
			}
			return FALSE;   
		}   

		nSendBytes = nSendBytes + nBytes;   

		if (nSendBytes < len)   
		{   
			Sleep(1000);   
		}   
	}    
	return TRUE; 
}

bool CTCPCustom::IsTimeOut( int nSeconds )
{
	if(m_bOK == false)
		return true;

	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan oleSpan = oleNow - m_oleLastReceive;
	int nTotalSecond = oleSpan.GetTotalSeconds();
	if(nTotalSecond >= nSeconds)
	{
		m_bOK = false;
		return true;
	}
	return false;
}

//20150415 在类里直接解析好
bool CTCPCustom::HandleRecData( const char* buffer,int nSize )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	if(buffer == NULL || nSize <= 0 || nSize > g_nRecMaxNum)
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
				StoreRecData(p,length);
			}
			else
			{
				if(m_cRecvBuf[m_nRecSize-1] == '\n')
				{
					StoreRecData(p,length);
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

void CTCPCustom::SplitStringSpecial( const char* buffer,int nSize, std::vector<datapackage>& resultlist )
{
	if(nSize <= 0 || nSize>g_nRecMaxNum)
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

bool CTCPCustom::AnalyzeReceive( const char* pRevData, DWORD length )
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

		if(strcmp(buffer_prefix, REGCMD_PREFIX) == 0)		//心跳包
		{
			char cDTUMark[256] = {0};
			memcpy(cDTUMark,pRevData+min_length, DTU_ID_LENGTH);
			for(int i=0; i<DTU_ID_LENGTH; ++i)
			{
				if(cDTUMark[i] == ';')
				{
					cDTUMark[i] = '\0';
					break;
				}
			}
			m_strThirdName = "";
			string strClientMark = cDTUMark;
			if(strClientMark == "core")
			{
				m_eType = E_REASON_SUBMIT_CORE;
			}
			else if(strClientMark == "logic")
			{
				m_eType = E_REASON_SUBMIT_LOGIC;
			}
			else if(strClientMark == "update")
			{
				m_eType = E_REASON_SUBMIT_UPDATE;
			}
			else if(strClientMark == "dtuengine")
			{
				m_eType = E_REASON_SUBMIT_DTUENGINE;
			}
			else
			{
				m_strThirdName = strClientMark;
				m_eType = E_REASON_THIRD;
			}
			if(m_pTCPServer && m_pTCPServer->OnClientRegedit)
			{
				m_pTCPServer->OnClientRegedit(m_pTCPServer->m_pOwner,this,m_eType,(char*)m_strThirdName.c_str());
			}
		}
		if(strcmp(buffer_prefix, RESCMD_PREFIX) == 0)		//重启
		{
			char buffer_content[g_nRecMaxNum] = {0};
			int nLength = ((length-min_length)>g_nRecMaxNum)?g_nRecMaxNum:(length-min_length);
			memcpy(buffer_content, pRevData+min_length, nLength);
			char* pContent = strtok((char*)buffer_content, ";");
			vector<string>	vecCmd;
			SplitString(pContent,"|",vecCmd);			
			if(vecCmd.size() == 2)
			{
				if(m_pTCPServer && m_pTCPServer->OnServerRestartTask)
				{
					if(vecCmd[0] == "1")					//重启Core成功
					{
						m_pTCPServer->OnServerRestartTask(m_pTCPServer->m_pOwner,this,E_RESTART_CORE,m_eType);
					}
					else if(vecCmd[0] == "2")				//重启Logic成功
					{
						m_pTCPServer->OnServerRestartTask(m_pTCPServer->m_pOwner,this,E_RESTART_LOGIC,m_eType);
					}
					else if(vecCmd[0] == "3")				//重启Update成功
					{
						m_pTCPServer->OnServerRestartTask(m_pTCPServer->m_pOwner,this,E_RESTART_UPDATE,m_eType);
					}
					else if(vecCmd[0] == "9")				//重启Update成功
					{
						m_pTCPServer->OnServerRestartTask(m_pTCPServer->m_pOwner,this,E_RESTART_DTUENGINE,m_eType);
					}
				}
			}
			return true;
		}
		return true;
	}
	return false;
}

bool CTCPCustom::StoreRecData( const char* rawbuffer,int rawsize )
{
	datapackage data;
	int length = (rawsize>=g_nPackageSize)?g_nPackageSize:rawsize;
	memset(data.cData,0,g_nPackageSize);
	memcpy(data.cData,rawbuffer,length);
	data.nLength = length;
	// 开始进入临界区
	EnterCriticalSection(&m_csDTUDataSync);
	m_queDataPaket.push(data);
	LeaveCriticalSection(&m_csDTUDataSync);
	SetDataEvent();
	return true;
}

void CTCPCustom::SetDataEvent()
{
	SetEvent(m_event);
}

void CTCPCustom::HandleData()
{
	while(!m_bexitthread)
	{
		DWORD waitflag = WaitForSingleObject(m_event, INFINITE);
		if (waitflag == WAIT_OBJECT_0  && !m_bexitthread)
		{
			while(!m_queDataPaket.empty())
			{
				// 开始进入临界区
				EnterCriticalSection(&m_csDTUDataSync);
				bool bHasData = false;
				datapackage data;
				if(!m_queDataPaket.empty())
				{
					data = m_queDataPaket.front();
					m_queDataPaket.pop();
					bHasData = true;
				}
				LeaveCriticalSection(&m_csDTUDataSync);

				if(bHasData)
				{
					AnalyzeReceive(data.cData,data.nLength);
				}
			}
		}
	}
}

UINT WINAPI CTCPCustom::ThreadFunc( LPVOID lparam )
{
	CTCPCustom* pthis = (CTCPCustom*)lparam;
	if (pthis != NULL)
	{
		pthis->HandleData();
	}

	return 0;
}

bool CTCPCustom::AckReg()
{
	return SendAck(0,1);
}

bool CTCPCustom::SendAck( int nCmdType,int nResult,int nRestartType )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_sendlock);
	std::ostringstream sqlstream;
	if(nCmdType == 0)
	{
		sqlstream << REGACK_PREFIX<< nCmdType << "," << nResult  << "," << nRestartType << ";";
	}
	else
	{
		sqlstream << RESACK_PREFIX<< nCmdType << "," << nResult << "," << nRestartType << ";";
	}
	string strdata = sqlstream.str();
	char strbuf[1024] = {0};
	int len = strdata.length();
	len = (len>=1021)?1021:len;
	memcpy(strbuf,strdata.data(),len);
	strbuf[len] = ';';			//以;;\n为包结尾
	strbuf[len+1] = '\n';
	return SendData((const char *)strbuf, len+2);
}

bool CTCPCustom::SendAck( E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_sendlock);
	std::ostringstream sqlstream;
	sqlstream << RESACK_PREFIX << eType << "," << eReason  << "," << bResult << ";";
	string strdata = sqlstream.str();
	char strbuf[1024] = {0};
	int len = strdata.length();
	len = (len>=1021)?1021:len;
	memcpy(strbuf,strdata.data(),len);
	strbuf[len] = ';';			//以;;\n为包结尾
	strbuf[len+1] = '\n';
	return SendData((const char *)strbuf, len+2);
}
