// TCPSocket.cpp: implementation of the CTCPSocket class.
//  把Sleep(1)为Sleep(5)
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TCPSocket.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPSocket::CTCPSocket(int nType)
{
	m_nType=nType;
	m_sSocket=NULL;
	m_bAvailable=Initwinsock();
	m_bCreated=FALSE;
	m_bAuto=FALSE;
	m_dwUserData=0;
	error=0;

	m_nPort=-1;
	m_hServerThread=NULL;
	for(int i=0;i<MAX_CONNECTION;i++)
	{
		m_hServerDataThread[i]=NULL;
		m_sServer[i]=NULL;
		ZeroMemory(m_cIp[i],16);
		m_bConnected[i]=FALSE;
	}
	m_nConnections=0;
	m_nCurrent=0;
	m_lpServerStatusProc=NULL;
	m_lpServerDataArriveProc=NULL;

	m_hClientThread=NULL;
	m_lpClientDataArriveProc=NULL;
	m_lpClientStatusProc=NULL;
}

CTCPSocket::~CTCPSocket()
{
	if(m_hServerThread)
	{
		CloseHandle(m_hServerThread);
		m_hServerThread = NULL;
	}
	Disconnect();
}

int CTCPSocket::GetError()
{
	return error;
}

SOCKET CTCPSocket::GetSocket()
{
	return m_sSocket;
}

int CTCPSocket::GetType()
{
	return m_nType;
}

BOOL CTCPSocket::IsConnected(SOCKET s)
{
	int nRet=0;
	struct fd_set Fd_Recv;
	struct timeval Time_Recv;

	memset(&Fd_Recv,0,sizeof(struct fd_set));
	FD_CLR(s,&Fd_Recv);
	FD_SET(s,&Fd_Recv);
	Time_Recv.tv_sec=0;
	Time_Recv.tv_usec=0;

	nRet=select(s,&Fd_Recv,NULL,NULL,&Time_Recv);

	return (nRet==0);
}

BOOL CTCPSocket::CreateServer(int nPort,int backlog)
{
	m_bAvailable=Initwinsock();
	if(!m_bAvailable)
	{
		return FALSE;
	}
	if(m_nType!=TCP_SOCKET_SERVER)
	{
		return FALSE;
	}
	if(m_bCreated)
	{
		return FALSE;
	}

	struct sockaddr_in local;

	m_sSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(m_sSocket==SOCKET_ERROR)
	{
		error=WSAGetLastError();
		return FALSE;
	}

	local.sin_addr.s_addr=htonl(INADDR_ANY);
	local.sin_family=AF_INET;
	local.sin_port=htons(nPort);

	if(bind(m_sSocket,(struct sockaddr*)&local,sizeof(local))==SOCKET_ERROR)
	{
		error=WSAGetLastError();
		closesocket(m_sSocket);
		return FALSE;
	}

	if(listen(m_sSocket,backlog)!=0)
	{
		error=WSAGetLastError();
		closesocket(m_sSocket);
		return FALSE;
	}

	m_nPort=nPort;
	m_bCreated=TRUE;

	return TRUE;
}

BOOL CTCPSocket::StartServer(LPStatusProc proc1,ServerDataArriveProc proc2,DWORD userdata)
{
	if(!m_bAvailable)
	{
		return FALSE;
	}
	if(m_nType!=TCP_SOCKET_SERVER)
	{
		return FALSE;
	}
	if(!m_bCreated)
	{
		return FALSE;
	}
	if(m_bAuto)
	{
		return FALSE;
	}

	m_lpServerStatusProc=proc1;
	m_lpServerDataArriveProc=proc2;
	m_dwUserData=userdata;

	DWORD dwThreadId;

	m_bAuto=TRUE;
	m_hServerThread=CreateThread(NULL,0,ServerThread,this,0,&dwThreadId);

	if(m_hServerThread==NULL)
	{
		m_bAuto=FALSE;
		error=WSAGetLastError();
		return FALSE;
	}
	
	return TRUE;
}

BOOL CTCPSocket::StopServer()
{
	if(!m_bAvailable)
	{
		return FALSE;
	}
	if(m_nType!=TCP_SOCKET_SERVER)
	{
		return FALSE;
	}
	if(!m_bCreated)
	{
		return FALSE;
	}
	if(!m_bAuto)
	{
		return FALSE;
	}

	DWORD exitcode;
	m_bAuto=FALSE;

	//回调清空
	m_lpClientStatusProc = NULL;
	m_lpClientDataArriveProc = NULL;


	//停止监听线程
	WaitForSingleObject(m_hServerThread,500);
	if(!GetExitCodeThread(m_hServerThread,&exitcode))
	{
		TerminateThread(m_hServerThread,exitcode);
	}
	CloseHandle(m_hServerThread);
	m_hServerThread=NULL;
	shutdown(m_sSocket,SD_RECEIVE);
	closesocket(m_sSocket);
	m_sSocket=NULL;

	//停止所有收发数据线程
	int i;
		
	for(i=0;i<MAX_CONNECTION;i++)
	{
		if(m_bConnected[i])
		{
			m_bConnected[i]=FALSE;
			WaitForSingleObject(m_hServerDataThread[i],50);
			if(!GetExitCodeThread(m_hServerDataThread[i],&exitcode))
			{
				TerminateThread(m_hServerDataThread[i],exitcode);
			}
			shutdown(m_sServer[i],SD_RECEIVE);
			closesocket(m_sServer[i]);
			m_sServer[i]=NULL;
			CloseHandle(m_hServerDataThread[i]);
			m_hServerDataThread[i]=NULL;
		}
	}
	m_nConnections=0;
	
	return TRUE;
}

SOCKET CTCPSocket::Listen(char* ClientIP)
{
	if(!m_bAvailable)
	{
		return -1;
	}
	if(m_nType!=TCP_SOCKET_SERVER)
	{
		return -1;
	}
	if(!m_bCreated)
	{
		return -1;
	}

	SOCKET sClient;
	int iAddrSize;
	struct sockaddr_in addr;

	iAddrSize=sizeof(addr);

	sClient=accept(m_sSocket,(struct sockaddr*)&addr,&iAddrSize);

	if(sClient==SOCKET_ERROR)
	{
		error=WSAGetLastError();
		closesocket(sClient);
		return SOCKET_ERROR;
	}

	if(ClientIP!=NULL)
	{
		sprintf(ClientIP,"%3d.%3d.%3d.%3d",addr.sin_addr.S_un.S_un_b.s_b1,addr.sin_addr.S_un.S_un_b.s_b2,addr.sin_addr.S_un.S_un_b.s_b3,addr.sin_addr.S_un.S_un_b.s_b4);
	}

	return sClient;
}

int CTCPSocket::ReceiveServer(int nNo,char* data,int length,int timeout)
{
	if(!m_bConnected[nNo])
	{
		return -2;
	}

	HANDLE hThread;
	DWORD dwThreadId;

	TimeOutParameter TimeOut;

	TimeOut.bExit=FALSE;
	TimeOut.bFinished=FALSE;
	TimeOut.EndTime=timeout;
	TimeOut.nNo=nNo;
	TimeOut.pbConnected=&(m_bConnected[nNo]);
	TimeOut.phDataThread=&(m_hServerDataThread[nNo]);
	TimeOut.pnConnections=&m_nConnections;
	TimeOut.s=m_sServer[nNo];

	hThread=CreateThread(NULL,0,TimeOutControl,(LPVOID)&TimeOut,0,&dwThreadId);

	if(hThread==NULL)
	{
		return -3;
	}

	int nRet=recv(m_sServer[nNo],data,length,0);
	if(nRet==SOCKET_ERROR)
	{
		error=WSAGetLastError();
	}
	TimeOut.bFinished=TRUE;

	while(!TimeOut.bExit)
	{
		Sleep(50);
	}

	return nRet;
}

int CTCPSocket::SendServer(int nNo,char *data,int length)
{
	if(!m_bConnected[nNo])
	{
		return -2;
	}

	int nRet=send(m_sServer[nNo],data,length,0);
	if(nRet==SOCKET_ERROR)
	{
		error=WSAGetLastError();
	}
	return nRet;
}

void CTCPSocket::Disconnect(int nNo)
{
	if(!m_bConnected[nNo])
	{
		return;
	}

	//断开服务器上第nNo个连接
	DWORD exitcode;
	m_bConnected[nNo]=FALSE;
	WaitForSingleObject(m_hServerDataThread[nNo],50);
	if(!GetExitCodeThread(m_hServerDataThread[nNo],&exitcode))
	{
		TerminateThread(m_hServerDataThread[nNo],exitcode);
	}
	shutdown(m_sServer[nNo],SD_RECEIVE);
	closesocket(m_sServer[nNo]);
	m_sServer[nNo]=NULL;
	CloseHandle(m_hServerDataThread[nNo]);
	m_hServerDataThread[nNo]=NULL;
	m_nConnections--;

}

BOOL CTCPSocket::Connect(CString pstrHost,int nPort)
{
	m_bAvailable=Initwinsock();
	if(!m_bAvailable)
	{
		return FALSE;
	}
	if(m_nType==TCP_SOCKET_SERVER)
	{
		return FALSE;
	}
	if(m_bCreated)
	{
		return FALSE;
	}

	LPHOSTENT lpHost;
	struct sockaddr_in server;

	//查找主机
	lpHost=gethostbyname(ConvertWideCharToMultiByte(pstrHost.GetString()).c_str());
	if(lpHost==NULL)
	{
		return FALSE;
	}

	server.sin_family=AF_INET;
	server.sin_addr.s_addr=*((u_long FAR*)(lpHost->h_addr));
	server.sin_port=htons(nPort);

	m_sSocket=socket(AF_INET,SOCK_STREAM,0);

	if(m_sSocket<=0)
	{
		error=WSAGetLastError();
		return FALSE;
	}
	
	if(connect(m_sSocket,(LPSOCKADDR)&server,sizeof(SOCKADDR))==SOCKET_ERROR) 
	{
		error=WSAGetLastError();
		closesocket(m_sSocket);
		m_sSocket=NULL;
		return FALSE;
	}

	m_bCreated=TRUE;

	return TRUE;
}

BOOL CTCPSocket::Connect( string pstrHost, int nPort )
{
	m_bAvailable=Initwinsock();
	if(!m_bAvailable)
	{
		return FALSE;
	}
	if(m_nType==TCP_SOCKET_SERVER)
	{
		return FALSE;
	}
	if(m_bCreated)
	{
		return FALSE;
	}

	LPHOSTENT lpHost;
	struct sockaddr_in server;

	//查找主机
	lpHost=gethostbyname(pstrHost.c_str());
	if(lpHost==NULL)
	{
		return FALSE;
	}

	server.sin_family=AF_INET;
	server.sin_addr.s_addr=*((u_long FAR*)(lpHost->h_addr));
	server.sin_port=htons(nPort);

	m_sSocket=socket(AF_INET,SOCK_STREAM,0);

	if(m_sSocket<=0)
	{
		error=WSAGetLastError();
		return FALSE;
	}

	if(connect(m_sSocket,(LPSOCKADDR)&server,sizeof(SOCKADDR))==SOCKET_ERROR) 
	{
		error=WSAGetLastError();
		closesocket(m_sSocket);
		m_sSocket=NULL;
		return FALSE;
	}

	m_bCreated=TRUE;

	return TRUE;
}

BOOL CTCPSocket::StartReceiving(LPStatusProc proc1,LPDataArriveProc proc2,DWORD userdata)
{
	if(!m_bAvailable)
	{
		return FALSE;
	}
	if(m_nType==TCP_SOCKET_SERVER)
	{
		return FALSE;
	}
	if(!m_bCreated)
	{
		return FALSE;
	}
	if(m_bAuto)
	{
		return FALSE;
	}

	//开始自动接收
	m_lpClientStatusProc=proc1;
	m_lpClientDataArriveProc=proc2;
	m_dwUserData=userdata;
	m_bAuto=TRUE;

	DWORD dwThreadId;

	m_hClientThread=CreateThread(NULL,0,ClientThread,this,0,&dwThreadId);

	if(m_hClientThread==NULL)
	{
		m_bAuto=FALSE;
		error=WSAGetLastError();
		return FALSE;
	}
	
	return TRUE;
}

BOOL CTCPSocket::StopReceiving()
{
	if(!m_bAvailable)
	{
		return FALSE;
	}
	if(m_nType==TCP_SOCKET_SERVER)
	{
		return FALSE;
	}
	if(!m_bCreated)
	{
		return FALSE;
	}
	if(!m_bAuto)
	{
		return FALSE;
	}

	DWORD exitcode;
	m_bAuto=FALSE;
	//停止接收线程
	//WaitForSingleObject(m_hClientThread,500);

	m_lpClientStatusProc = NULL;
	m_lpClientDataArriveProc = NULL;
	TerminateThread(m_hClientThread,0);

	CloseHandle(m_hClientThread);
	m_hClientThread=NULL;

	return TRUE;
}

int CTCPSocket::ReceiveClient(char* data, int length,int timeout)
{
	if(m_nType==TCP_SOCKET_SERVER)
	{
		return -3;
	}
	if(!m_bCreated)
	{
		return -2;
	}

	HANDLE hThread;
	DWORD dwThreadId;

	TimeOutParameter TimeOut;

	TimeOut.bExit=FALSE;
	TimeOut.bFinished=FALSE;
	TimeOut.EndTime=timeout;
	TimeOut.nNo=-1;
	TimeOut.pbConnected=&(m_bAuto);
	TimeOut.phDataThread=&(m_hClientThread);
	TimeOut.pnConnections=&(m_bCreated);
	TimeOut.s=m_sSocket;

	hThread=CreateThread(NULL,0,TimeOutControl,(LPVOID)&TimeOut,0,&dwThreadId);

	if(hThread==NULL)
	{
		return -3;
	}

	int nRet=recv(m_sSocket,data,length,0);
	if(nRet==SOCKET_ERROR)
	{
		error=WSAGetLastError();
	}
	TimeOut.bFinished=TRUE;

	while(!TimeOut.bExit)
	{
		Sleep(50);
	}

	return nRet;
}

int CTCPSocket::SendClient(char* data,int length)
{
	if(m_nType==TCP_SOCKET_SERVER)
	{
		return -3;
	}
	if(!m_bCreated)
	{
		return -2;
	}

	int nRet=send(m_sSocket,data,length,0);
	if(nRet==SOCKET_ERROR)
	{
		error=WSAGetLastError();
	}

	return nRet;
}

void CTCPSocket::Disconnect()
{
	if(m_nType==TCP_SOCKET_SERVER)
	{
		StopServer();
		m_bCreated=FALSE;
	}
	else
	{
		StopReceiving();
		shutdown(m_sSocket,SD_RECEIVE);
		closesocket(m_sSocket);
		m_sSocket=NULL;
		m_bCreated=FALSE;
	}
	WSACleanup();
}

/////////////////////////////////////////////////////////////////////////////
//Protected Functions

BOOL CTCPSocket::Initwinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	
	wVersionRequested=MAKEWORD(2,2);
	
	if(WSAStartup(wVersionRequested,&wsaData)==0)
	{
		return TRUE;
	}
	else
	{
		WSACleanup();
		return FALSE;
	}
}

BOOL CTCPSocket::NewConnect(int nNo)
{
	//建立一个接收数据的线程
	m_bConnected[nNo]=TRUE;
	m_hServerDataThread[nNo]=CreateThread(NULL,0,DataThread,this,0,NULL);
	if(m_hServerDataThread==NULL)
	{
		m_bConnected[nNo]=FALSE;
		closesocket(m_sServer[nNo]);
		return FALSE;
	}
	m_nConnections++;

	return TRUE;
}

DWORD WINAPI CTCPSocket::ServerThread(LPVOID lpParameter)
{
	CTCPSocket* m_pTCP=(CTCPSocket*)lpParameter;
	if(!m_pTCP)
		return 0;

	SOCKET sClient;
	int iAddrSize;
	struct sockaddr_in addr;
	int i;

	iAddrSize=sizeof(addr);

	//不断监听连接
	while(m_pTCP->m_bAuto)
	{
		sClient=accept(m_pTCP->m_sSocket,(struct sockaddr*)&addr,&iAddrSize);
		if(sClient==SOCKET_ERROR)
		{
			continue;
		}
		//判断是否达到最大连接数
		if(m_pTCP->m_nConnections>=MAX_CONNECTION)
		{
			closesocket(sClient);
			continue;
		}

		//没有则开始一个线程处理这个的收发
		for(i=0;i<MAX_CONNECTION;i++)
		{
			if(!(m_pTCP->m_bConnected[i]))
			{
				break;
			}
		}
		sprintf(m_pTCP->m_cIp[i],"%3d.%3d.%3d.%3d",addr.sin_addr.S_un.S_un_b.s_b1,addr.sin_addr.S_un.S_un_b.s_b2,addr.sin_addr.S_un.S_un_b.s_b3,addr.sin_addr.S_un.S_un_b.s_b4);
		TRACE("%s Connected! Index:%d\n",m_pTCP->m_cIp[i],i);

		//回调处理
		if(m_pTCP->m_lpServerStatusProc!=NULL)
		{		
			m_pTCP->m_lpServerStatusProc(TCP_STATE_CONNECT,m_pTCP->m_cIp[i],i,m_pTCP->m_dwUserData);
		}

		m_pTCP->m_sServer[i]=sClient;
		m_pTCP->m_nCurrent=i;
		m_pTCP->NewConnect(i);
	}

	return 0;
}

DWORD WINAPI CTCPSocket::DataThread(LPVOID lpParameter)
{
	CTCPSocket* m_pTCP=(CTCPSocket*)lpParameter;
	int MyNumber=m_pTCP->m_nCurrent;

	int nRet;
	char buf[512];
	
	timeval tv={0,5000};
	fd_set fs;

	//不断接收每个已经连接的客户的数据
	while(m_pTCP->m_bConnected[MyNumber])
	{
		FD_ZERO(&fs);
		FD_SET(m_pTCP->m_sServer[MyNumber],&fs);
		if(select(1,&fs,NULL,NULL,&tv)==1)
		{
			nRet=recv(m_pTCP->m_sServer[MyNumber],buf,1024,0);
			
			if(nRet==SOCKET_ERROR)
			{
				//出错断开(例如客户端执行了超时操作导致自己断开)
				m_pTCP->error=WSAGetLastError();
				closesocket(m_pTCP->m_sServer[MyNumber]);
				m_pTCP->m_bConnected[MyNumber]=FALSE;
				m_pTCP->m_nConnections--;
				TRACE("%s DisConnect(Error)! Index%d\n",m_pTCP->m_cIp[MyNumber],MyNumber);

				//回调处理
				if(m_pTCP->m_lpServerStatusProc!=NULL)
				{
					m_pTCP->m_lpServerStatusProc(TCP_STATE_BAD_DISCONNECT,m_pTCP->m_cIp[MyNumber],MyNumber,m_pTCP->m_dwUserData);
				}

				break;
			}
			
			if(nRet>0)
			{
				//收到新的数据
				TRACE("Receive %s Data %d Byte! Index%d\n",m_pTCP->m_cIp[MyNumber],nRet,MyNumber);

				//数据回调处理
				if(m_pTCP->m_lpServerDataArriveProc!=NULL)
				{			
					m_pTCP->m_lpServerDataArriveProc(MyNumber,m_pTCP->m_cIp[MyNumber],buf,nRet,m_pTCP->m_dwUserData);
				}

				continue;
			}
			
			if(nRet==0)
			{
				//客户断开
				TRACE("%s DisConnect(OK)! Index %d\n",m_pTCP->m_cIp[MyNumber],MyNumber);

				//回调处理
				if(m_pTCP->m_lpServerStatusProc!=NULL)
				{
					m_pTCP->m_lpServerStatusProc(TCP_STATE_GOOD_DISCONNECT,m_pTCP->m_cIp[MyNumber],MyNumber,m_pTCP->m_dwUserData);
				}

				closesocket(m_pTCP->m_sServer[MyNumber]);
				m_pTCP->m_bConnected[MyNumber]=FALSE;
				m_pTCP->m_nConnections--;
				break;
			}
		}
		Sleep(50);//hxf 添加
	}

	return 0;
}

DWORD WINAPI CTCPSocket::ClientThread(LPVOID lpParameter)
{
	CTCPSocket* m_pTCP=(CTCPSocket*)lpParameter;
	if(!m_pTCP)
		return 0;

	int nRet =0 ;
	char buf[1024];
	
	timeval tv={0,5000};
	fd_set fs;

	//不断接收服务器发来数据
	while(m_pTCP->m_bAuto)
	{
		FD_ZERO(&fs);
		FD_SET(m_pTCP->m_sSocket,&fs);
		if(select(1,&fs,NULL,NULL,&tv)==1)
		{
			nRet=recv(m_pTCP->m_sSocket,buf,1024,0);
			
			if(nRet==SOCKET_ERROR)
			{
				//出错断开(例如服务器关闭)
				m_pTCP->error=WSAGetLastError();
				closesocket(m_pTCP->m_sSocket);
				m_pTCP->m_bAuto=FALSE;
				TRACE("Client DisConnect(Error)! \n");

				//回调处理
				if(m_pTCP->m_lpClientStatusProc!=NULL)
				{
					m_pTCP->m_lpClientStatusProc(TCP_STATE_BAD_DISCONNECT,buf,nRet,m_pTCP->m_dwUserData);
				}

				break;
			}
			
			if(nRet>0)
			{
				//收到新的数据
				TRACE("Client Receive %d Byte! \n",nRet);

				//数据回调处理
				if(m_pTCP->m_lpClientDataArriveProc!=NULL)
				{
					m_pTCP->m_lpClientDataArriveProc(buf,nRet,m_pTCP->m_dwUserData);
				}

				m_pTCP->OnCommunication((unsigned char*)buf, nRet);

				continue;
			}
			
			if(nRet==0)
			{
				//服务器正常断开
				TRACE("Client DisConnect(OK)! \n");

				//回调处理
				if(m_pTCP->m_lpClientStatusProc!=NULL)
				{			
					m_pTCP->m_lpClientStatusProc(TCP_STATE_GOOD_DISCONNECT,buf,nRet,m_pTCP->m_dwUserData);
				}
				closesocket(m_pTCP->m_sSocket);
				m_pTCP->m_bAuto=FALSE;

				break;
			}
		}
		Sleep(50);//hxf 添加
	}

	return 0;
}

DWORD WINAPI CTCPSocket::TimeOutControl(LPVOID lpParameter)
{
	TimeOutParameter* m_pTimeOut=(TimeOutParameter*)lpParameter;

	time_t starttime,endtime;
	BOOL bTimeOut=FALSE;

	starttime=time(NULL);
	while(!bTimeOut)
	{
		if(m_pTimeOut->bFinished)
		{
			m_pTimeOut->bExit=TRUE;
			return 1;
		}
		Sleep(50);
		endtime=time(NULL);
		if((endtime-starttime)>m_pTimeOut->EndTime)
		{
			//超时
			bTimeOut=TRUE;
		}
	}

	//断开对应连接
	DWORD exitcode;
	if(m_pTimeOut->bFinished)
	{
		return 1;
	}
	if(m_pTimeOut->s!=NULL)
	{
		if(m_pTimeOut->nNo>=0)
		{
			//服务器socket
			//停止该接收线程
			*(m_pTimeOut->pbConnected)=FALSE;
			WaitForSingleObject(*(m_pTimeOut->phDataThread),50);
			if(!GetExitCodeThread(*(m_pTimeOut->phDataThread),&exitcode))
			{
				TerminateThread(*(m_pTimeOut->phDataThread),exitcode);
			}
			shutdown(m_pTimeOut->s,SD_RECEIVE);
			closesocket(m_pTimeOut->s);
			m_pTimeOut->s=NULL;
			CloseHandle(*(m_pTimeOut->phDataThread));
			*(m_pTimeOut->phDataThread)=NULL;
			*(m_pTimeOut->pnConnections)--;
			
		}
		else
		{
			//停止客户端接收线程
			if(*(m_pTimeOut->pbConnected))
			{
				*(m_pTimeOut->pbConnected)=FALSE;
				WaitForSingleObject(*(m_pTimeOut->phDataThread),50);
				if(!GetExitCodeThread(*(m_pTimeOut->phDataThread),&exitcode))
				{
					TerminateThread(*(m_pTimeOut->phDataThread),exitcode);
				}
			}
			shutdown(m_pTimeOut->s,SD_RECEIVE);
			closesocket(m_pTimeOut->s);
			m_pTimeOut->s=NULL;
			CloseHandle(*(m_pTimeOut->phDataThread));
			*(m_pTimeOut->phDataThread)=NULL;
			*(m_pTimeOut->pnConnections)=FALSE;
		}
	}

	m_pTimeOut->bExit=TRUE;
	return 0;
}

std::string CTCPSocket::ConvertWideCharToMultiByte( const std::wstring& wstrSrc )
{
	const TCHAR* pWStr = wstrSrc.c_str();
	const int nLen = (int)wstrSrc.length();
	string ret;
	LPSTR pStr=0;
	int len=WideCharToMultiByte(CP_ACP,0,pWStr,nLen+1,0,0,0,0);
	if(!len)
		return ret;		

	pStr=(LPSTR)malloc(len);

	len=WideCharToMultiByte(CP_ACP,0,pWStr,nLen+1,pStr,len,0,0);
	if(!len)
	{
		free(pStr);
		return ret;		
	}
	ret = pStr;
	free(pStr);

	return ret;
}

void CTCPSocket::SetType( int nType )
{
	m_nType = nType;
}
