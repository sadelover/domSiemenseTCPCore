// TCPCustom.h: interface for the CTCPCustom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPCUSTOM_CE_H__0E8B4A18_8A99_438E_B5F6_B5985FFC117D__INCLUDED_)
#define AFX_TCPCUSTOM_CE_H__0E8B4A18_8A99_438E_B5F6_B5985FFC117D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock.h>
#include "TCPServer.h"
#include <queue>
#include "ProcessView.h"

const int g_nRecMaxNum = 8192;	  //4096*2
const int g_nRecAveNum = 4096;    //4096
const int g_nPackageSize = 4096;	//每个数据包长度

struct datapackage				//分包一个包
{
	datapackage()
	{
		nLength = 0;
		memset(cData,0,g_nPackageSize);
	}
	char   cData[g_nPackageSize];				//内容
	int	   nLength;					//长度
};

class __declspec(dllexport) CTCPCustom  
{
public:
    CTCPCustom();
    virtual ~CTCPCustom();
public:
    CTCPServer* m_pTCPServer;		//引用TCP服务端监听Socket
    CString		m_RemoteHost;		//远程主机IP地址
    DWORD		m_RemotePort;		//远程主机端口号
    SOCKET		m_socket;			//通讯Socket句柄
	COleDateTime m_oleLastReceive;	//上一次收到数据时间
	char		m_cRecvBuf[g_nRecMaxNum];
	int			m_nRecSize;
	char		m_cSpiltBuffer[g_nRecMaxNum];			//数据解析缓存
	bool		m_bOK;				//成功标志
	bool		m_bOutRec;			//输入接收LOG
	bool		m_bexitthread;
	CProcessView		m_PrsV;
	E_RESTART_REASON		m_eType;	//客户类别
	string	m_strThirdName;				//第三方程序名称
	Project::Tools::Mutex m_lock;
	Project::Tools::Mutex m_reveivelock;
	Project::Tools::Mutex m_sendlock;
	CRITICAL_SECTION m_csTCPSocketSync;			//临界区对象 保护Socket资源

	queue<datapackage>	m_queDataPaket;					//存储数据包
	CRITICAL_SECTION m_csDTUDataSync;			//临界区对象 保护共享资源
	HANDLE m_event;
private:
    HANDLE m_exitThreadEvent;  //通讯线程退出事件句柄
    HANDLE m_tcpThreadHandle;  //通讯线程句柄
	HANDLE m_datathread;
private:
    //通讯线程函数
    static DWORD WINAPI SocketThreadFunc(PVOID lparam);
	static UINT WINAPI ThreadFunc(LPVOID lparam);				//处理收到数据
public:
    //打开socket，创建通讯线程
    bool Open(CTCPServer *pTCPServer);
    
    //关闭socket，关闭线程，释放Socket资源
    bool Close();

    //向客户端发送数据
    bool  SendData(const char * buf , int len);
	
	bool	IsTimeOut(int nSeconds = 600);		//10分钟未收到数据时间
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<datapackage>& resultlist);		//争对;\n操作
	bool	HandleRecData(const char* buffer,int nSize);

	bool	StoreRecData(const char* buffer,int nSize);
	bool	AnalyzeReceive(const char* pRevData, DWORD length);	
	void	SetDataEvent();
	void	HandleData();

	bool	AckReg();
	bool	SendAck(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
	bool	SendAck(int nCmdType,int nResult,int nRestartType = -1);	// nCmdType 0:心跳包 1：重启Core 2：重启Watch    nResult 0:失败 nRestartType:重启方式
};

#endif // !defined(AFX_TCPCUSTOM_CE_H__0E8B4A18_8A99_438E_B5F6_B5985FFC117D__INCLUDED_)
