// TCPServer_CE.h: interface for the CTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSERVER_CE_H__711FE909_4A87_4123_95F8_45160691659D__INCLUDED_)
#define AFX_TCPSERVER_CE_H__711FE909_4A87_4123_95F8_45160691659D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock.h>
#include <map>
#include <string>
#include "../Tools/CustomTools/CustomTools.h"
using namespace std;

class CTCPCustom;
class CTCPServer;

enum E_RESTART_TYPE
{
	E_RESTART_NULL = 0,
	E_RESTART_CORE,				//重启Core
	E_RESTART_LOGIC,
	E_RESTART_UPDATE,
	E_RESTART_MASTER,
	E_CLOSE_CORE,				//关闭Core
	E_CLOSE_LOGIC,
	E_CLOSE_UPDATE,
	E_CLOSE_MASTER,
	E_RESTART_DTUENGINE,
	E_CLOSE_DTUENGINE,
};

enum E_RESTART_REASON
{
	E_REASON_NULL = 0,
	E_REASON_UNFEED_CORE,			//Core没有喂狗
	E_REASON_UNFEED_LOGIC,			//Logic没有喂狗
	E_REASON_UNFEED_UPDATE,			//Update没有喂狗
	E_REASON_SUBMIT_CORE,			//Core主动提交
	E_REASON_SUBMIT_LOGIC,
	E_REASON_SUBMIT_UPDATE,
	E_REASON_UNEXIST_MASTER,		//master判断不存在
	E_REASON_FEED,					//主动喂狗
	E_REASON_THIRD,					//第三方数据包
	E_REASON_UNFEED_DTUENGINE,		//DTUEngine没有喂狗
	E_REASON_SUBMIT_DTUENGINE,			//
};


//定义客户端连接建立事件 
typedef void (CALLBACK* ONCLIENTCONNECT)(void* pOwner,CTCPCustom*); 
//定义客户端SOCKET关闭事件 
typedef void (CALLBACK* ONCLIENTCLOSE)(void* pOwner,CTCPCustom*); 
//定义客户端当有数据接收事件 
typedef void (CALLBACK* ONCLIENTREAD)(void* pOwner,CTCPCustom*,const char * buf,DWORD dwBufLen ); 
//定义客户端Socket错误事件 
typedef void (CALLBACK* ONCLIENTERROR)(void* pOwner,CTCPCustom*,int nErrorCode); 
//定义服务器端Socket错误事件 
typedef void (CALLBACK* ONSERVERERROR)(void* pOwner,CTCPServer*,int nErrorCode); 
//定义服务器端任务时间
typedef void (CALLBACK* ONSERVERERESTARTTASK)(void* pOwner,CTCPCustom*,int nRestartType,E_RESTART_REASON eReason); 

//定义服务器端注册
typedef void (CALLBACK* ONSERVERERREGEDIT)(void* pOwner,CTCPCustom*,E_RESTART_REASON eRegType, char* strProgName); 

class __declspec(dllexport) CTCPServer  
{
public:
    CTCPServer();
    virtual ~CTCPServer();

public:
  int Open(); //打开TCP服务
  int Close(); //关闭TCP服务 
  //BOOL SendData(CTCPCustom* pCustomCE, const char * buf , DWORD dwBufLen); 
  //bool	SendData(string strClientMark,const char * buf , DWORD dwBufLen);
  //删除客户端对象 
  void RemoteClient(CTCPCustom *pClient /*客户端对象*/); 

  void	RemoteInactiveClient();
  bool	GetExitThread() const;
  CTCPCustom* FindClient(CString strRemoteHost,DWORD nPort);  
  bool	GetAllClientConnectInfo(vector<CString> &vecConnectInfo);

  bool	AckRestart(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
private:
    //线程处理函数
    static DWORD WINAPI SocketThreadFunc(PVOID lparam);

	//关闭长时间未收到数据的连接
	static DWORD WINAPI CloseThreadFunc(PVOID lparam);
public:
    int m_LocalPort; //设置服务端口号
	void * m_pOwner;   //父对象句柄 


	//存储客户端Socket句柄
	CTCPCustom FindClient(SOCKET m_client);
    CWnd * m_pOwnerWnd;   //父窗口句柄
	bool	m_bexitthread;
	Mutex	m_utex;
	Project::Tools::Mutex m_lock;		//移除客户端加锁
private:
    SOCKET m_ServerSocket;     //TCP服务监听socket
    HANDLE m_serverThreadHandle;  //通讯线程句柄
	HANDLE m_serverCloseHandle;  //线程句柄
    HANDLE m_exitThreadEvent;  //通讯线程退出事件句柄

public:  //定义事件
	//客户端连接建立事件，回调函数 
	ONCLIENTCONNECT    OnClientConnect; 
	//客户端连接断开事件，回调函数 
	ONCLIENTCLOSE OnClientClose; 
	//客户端接收数据事件，回调函数 
	ONCLIENTREAD       OnClientRead; 
	//客户端发生错误事件，回调函数 
	ONCLIENTERROR      OnClientError; 
	//服务器端发生错误事件,回调函数 
	ONSERVERERROR	   OnServerError; 

	ONSERVERERESTARTTASK	   OnServerRestartTask; 

	ONSERVERERREGEDIT	OnClientRegedit;
};

#endif // !defined(AFX_TCPSERVER_CE_H__711FE909_4A87_4123_95F8_45160691659D__INCLUDED_)
