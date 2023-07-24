// TCPServer.cpp: implementation of the CTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TCPServer.h"
#include "TCPCustom.h"
//#include "../include/vld.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <afxtempl.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPtrList m_ListClientSocket;   

//构造函数
CTCPServer::CTCPServer()
{
	//创建线程退出事件句柄
	m_exitThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_bexitthread = false;
	m_serverCloseHandle = NULL;

	//客户端连接建立事件，回调函数   
	OnClientConnect = NULL;   
	//客户端连接断开事件，回调函数   
	OnClientClose = NULL;   
	//客户端接收数据事件，回调函数   
	OnClientRead = NULL;   
	//客户端发生错误事件，回调函数   
	OnClientError = NULL;   
	//服务器端发生错误事件,回调函数   
	OnServerError = NULL;   

	OnServerRestartTask = NULL;

	OnClientRegedit = NULL;
}

//析构函数
CTCPServer::~CTCPServer()
{
	//关闭线程退出事件句柄
	m_bexitthread = true;
	CloseHandle(m_exitThreadEvent);
}

/*--------------------------------------------------------------------
【函数介绍】:  此线程用于检测监听套接字事件。
【入口参数】:  lparam:无类型指针，可以通过此参数，向线程中传入需要用到的资源。
			   在这里我们将CTCPServer类实例指针传进来
【出口参数】:  (无)
【返回  值】:  返回值没有特别的意义，在此我们将返回值设为0。
---------------------------------------------------------------------*/
DWORD WINAPI CTCPServer::SocketThreadFunc(PVOID lparam)
{
	CTCPServer *pSocket;
	//得到CTCPServer实例指针
	pSocket = (CTCPServer*)lparam;
	//定义读事件集合
	fd_set fdRead;
	int ret;
	TIMEVAL	aTime;
	aTime.tv_sec = 1;
	aTime.tv_usec = 1;
	while (TRUE)
	{
        //收到退出事件，结束线程
		if (WaitForSingleObject(pSocket->m_exitThreadEvent,0) == WAIT_OBJECT_0)
		{
			break;
		}
		
		FD_ZERO(&fdRead);
		FD_SET(pSocket->m_ServerSocket,&fdRead);
		
		ret = select(0,&fdRead,NULL,NULL,&aTime);
		
		if (ret == SOCKET_ERROR)
		{
			//触发错误事件
			int iErrorCode = WSAGetLastError();
			//触发服务器socket的错误事件
			pSocket->OnServerError(pSocket->m_pOwnerWnd,pSocket,iErrorCode);
			//关闭服务器套接字 
			closesocket(pSocket->m_ServerSocket);
			break;
		}
		
		if (ret > 0)
		{
			//判断是否读事件
			if (FD_ISSET(pSocket->m_ServerSocket,&fdRead))
			{
				//如果调用了Listen，则表示触发了OnAccept事件
				
				SOCKADDR_IN clientAddr;
				CTCPCustom * pClientSocket = new CTCPCustom();
				int namelen = sizeof(clientAddr);
				//等待，创建与客户端连接的套接字
				pClientSocket->m_socket = accept(pSocket->m_ServerSocket, (struct sockaddr *)&clientAddr, &namelen);
				//接收到客户端连接
				if (pClientSocket->m_socket)
				{
					pClientSocket->m_RemoteHost = inet_ntoa(clientAddr.sin_addr);
					pClientSocket->m_RemotePort = ntohs(clientAddr.sin_port);
					
					//触发与客户端建立连接事件   
					if (pSocket->OnClientConnect)   
					{   
						pSocket->OnClientConnect(pSocket->m_pOwner,pClientSocket);   
					}   
					//打开pClientSocket服务线程   
					pClientSocket->Open(pSocket);   
					//添加到客户端连接队列中   
					m_ListClientSocket.AddTail(pClientSocket);   
				}   
				else   
				{   
					//失败，释放内存   
					delete pClientSocket;   
					pClientSocket = NULL;   
				}      
			}
		}
	}
	//   
	TRACE(L"服务器端线程退出\n");   
	return 0;
}

/*--------------------------------------------------------------------
【函数介绍】:  打开TCP服务
【入口参数】:  (无)
【出口参数】:  (无)
【返回  值】:  <=0:打开TCP服务失败; =1:打开TCP服务成功
---------------------------------------------------------------------*/
int CTCPServer::Open()
{
	WSADATA wsa;
	
	//1.初始化socket资源
	if (WSAStartup(MAKEWORD(1,1),&wsa) != 0)
	{
		return -1;//代表失败
	}
	
	//2.创建监听套接字
	if ((m_ServerSocket=socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		return -2;
	}
	
    SOCKADDR_IN  serverAddr;
	ZeroMemory((char *)&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(m_LocalPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//3.绑定监听套接字
	if (bind(m_ServerSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0)
	{
		
		return -3 ;
	}
	//4.监听套接字开始监听
	if (listen(m_ServerSocket,8)!=0)
	{
		return -3;
	}
	
	//4.设置监听套接字通讯模式为异步模式
	DWORD ul= 1;
	ioctlsocket(m_ServerSocket,FIONBIO,&ul);

	ResetEvent(m_exitThreadEvent);
	m_bexitthread = false;
	//5.创建通讯线程，在线程里，等待客户端接入
	m_serverThreadHandle = CreateThread(NULL,0,SocketThreadFunc,this,0,NULL);
	m_serverCloseHandle = CreateThread(NULL,0,CloseThreadFunc,this,0,NULL);
	if (m_serverThreadHandle == NULL)
	{
		closesocket(m_ServerSocket);
		return -1;
	}

	return 1;
}


/*--------------------------------------------------------------------
【函数介绍】:  关闭TCP服务
【入口参数】:  (无)
【出口参数】:  (无)
【返回  值】:  <=0:关闭TCP服务失败; =1:关闭TCP服务成功
---------------------------------------------------------------------*/
int CTCPServer::Close()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	//结束通讯线程   
	m_bexitthread = true;
	SetEvent(m_exitThreadEvent);   
	//等待1秒，如果读线程没有退出，则强制退出   
	if (WaitForSingleObject(m_serverThreadHandle,2000) == WAIT_TIMEOUT)   
	{   
		TerminateThread(m_serverThreadHandle,0);   
		TRACE(L"强制终止服务器端线程\n");   
	}   
	m_serverThreadHandle = NULL;   

	if(m_serverCloseHandle)
	{
		CloseHandle(m_serverCloseHandle);
		m_serverCloseHandle = NULL;
	}
	//首先，关闭与所有客户端连接   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		//得到客户端对象   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos);   
		if (!pTcpCustom->Close())   
		{   
			TRACE(L"关闭客户端socket错误");   
		}   

		//释放内存   
		delete pTcpCustom;   
		pTcpCustom = NULL;   
	}   
	m_ListClientSocket.RemoveAll();  
	//关闭Socket，释放资源   
	int err = closesocket(m_ServerSocket);   
	if (err == SOCKET_ERROR)   
	{   
		return -1;   
	}    
	WSACleanup();   
	return 1;  
}  

void CTCPServer::RemoteClient( CTCPCustom *pClient )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   

	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos);   
		if (pTcpCustom != NULL && pTcpCustom == pClient)   
		{   
			if (!pTcpCustom->Close())   
			{   
				TRACE(L"closesocket err\n");   
			} 

			//释放内存   
			delete pTcpCustom;   
			pTcpCustom = NULL;   
			m_ListClientSocket.RemoveAt(posPrior); 
			TRACE(L"RemoteClient\n");   
			break; 
		}   
	}   
}

DWORD WINAPI CTCPServer::CloseThreadFunc( PVOID lparam )
{
	CTCPServer* pthis = (CTCPServer*)lparam;
	if (pthis != NULL)
	{
		while(!pthis->GetExitThread())
		{
			pthis->RemoteInactiveClient();	
			Sleep(2000);	
		}
	}

	return 0;
}

void CTCPServer::RemoteInactiveClient()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
		if(pTcpCustom)
		{
			if(pTcpCustom->IsTimeOut())
			{
				CString strOut;
				strOut.Format(_T("RemoteinactiveClient(%s)\n"),pTcpCustom->m_RemoteHost);
				//触发与客户端端连接的Socket关闭事件   
				if (pTcpCustom->m_pTCPServer->OnClientClose)   
				{   
					pTcpCustom->m_pTCPServer->OnClientClose(pTcpCustom->m_pTCPServer->m_pOwner,pTcpCustom);   
				}  

				if (!pTcpCustom->Close())   
				{   
					TRACE(L"closesocket inactive err\n");  
				}   
				//释放内存   
				delete pTcpCustom;   
				pTcpCustom = NULL;   
				m_ListClientSocket.RemoveAt(posPrior); 
				TRACE(strOut.GetString());   
			}
		}
	}   
}

bool CTCPServer::GetExitThread() const
{
	return m_bexitthread;
}

CTCPCustom* CTCPServer::FindClient( CString strRemoteHost,DWORD nPort )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
		if(pTcpCustom)
		{
			if(pTcpCustom->m_RemoteHost == strRemoteHost && pTcpCustom->m_RemotePort == nPort)
			{
				return pTcpCustom;
			}
		}
	}
	return NULL;
}

bool CTCPServer::GetAllClientConnectInfo( vector<CString> &vecConnectInfo )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	vecConnectInfo.clear();
	POSITION posPrior;   
	POSITION pos = m_ListClientSocket.GetHeadPosition();   
	while (pos != NULL)   
	{   
		posPrior = pos;   
		CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
		if(pTcpCustom)
		{
			CString strConnectInfo;
			strConnectInfo.Format(_T("%s:%d...\n"),pTcpCustom->m_RemoteHost,pTcpCustom->m_RemotePort);
			vecConnectInfo.push_back(strConnectInfo);
		}
	}
	return true;
}

bool CTCPServer::AckRestart( E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult )
{
	switch(eReason)
	{
		case E_REASON_SUBMIT_CORE:		//core主动提交的
		case E_REASON_SUBMIT_LOGIC:		//logic主动提交的
		case E_REASON_SUBMIT_UPDATE:	//update主动提交的
			{
				Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
				POSITION posPrior;   
				POSITION pos = m_ListClientSocket.GetHeadPosition();   
				while (pos != NULL)   
				{   
					posPrior = pos;   
					CTCPCustom *pTcpCustom = (CTCPCustom*)m_ListClientSocket.GetNext(pos); 
					if(pTcpCustom && pTcpCustom->m_bOK)
					{
						if(pTcpCustom->m_eType == eReason)
						{
							return pTcpCustom->SendAck(eType,eReason,bResult);
						}
					}
				}
				return false;
			}
		default:
			return false;
	}
	return false;
}
