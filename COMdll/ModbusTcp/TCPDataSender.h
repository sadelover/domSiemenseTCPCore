#pragma once

#include "TcpClient.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

typedef LRESULT (*LPRecDataProc)(const unsigned char* pRevData, DWORD length,DWORD userdata); //接收数据的回调函数  add by robert 20150430

const int g_nRecMaxNum = 8192;
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

class CTCPDataSender : public CTcpClient
{
public:
	CTCPDataSender();
	virtual ~CTCPDataSender();

protected:
	
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);
public:
	void	SetHost(string strhost);
	void	SetPort(u_short portnumer);
	void	SetName(string strname);

	//Initialize Tcp Info
	bool	InitTcp(string strHost,u_short portnumer,string strTCPName,int nReConnectInterval = 30,UINT writebuffersize = 1024);
	bool	CloseTcp();


	bool	Init();
	bool	Exit();

	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	bool	ReConnect();

	void    SetNetworkError();
	void    ClearNetworkError();

	// start/stop comm watching
	BOOL	StartMonitoring(LPRecDataProc proc1=NULL,DWORD userdata=NULL);
	BOOL	RestartMonitoring();
	BOOL	StopMonitoring();

	bool	WriteToPort(char *pData,int nLen);
	bool	WriteToPortByWrap(char *pData,int nLen);			//自动添加换行符

	bool	UpdateDTUWriteTime();
	bool	GetDTUIdleState(int nSeconds = 4);				//若发送时间间隔4s,即表示空闲,可用于补发历史数据
	std::string	GetSuccessBuffer();

	bool	HandleRecData(const char* buffer,int nSize);
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<datapackage>& resultlist);		//争对;\n操作

	bool	GetTCPConnectOK();
	void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
private:
	string					m_host;
	u_short					m_port;
	Project::Tools::Mutex	m_lock;
	//自动重连机制
	Project::Tools::Mutex	m_lock_connection;
	Project::Tools::Mutex	m_lock_send;
	HANDLE					m_hDataCheckRetryConnectionThread;
	int						m_nModbusNetworkErrorCount;
	bool					m_bExitThread;
	bool					m_bConnectSuccess;			//连接成功标志位
	bool					m_bSendTcpNameSuccess;		//发送名称成功标志位
	string					m_strTcpName;				//名称

	LPRecDataProc			m_lpRecDataProc;			//收到数据回复回调
	DWORD					m_dwUserData;				//用户数据
	char					*m_szWriteBuffer;			//待发送内容
	DWORD					m_nWriteBufferSize;			//
	int						m_nWriteSize;				//待发送大小
	int						m_nReConnectInterval;		//重连间隔
	COleDateTime			m_oleLastWriteTime;			//上一次写DTU时间
	std::string				m_strSendSuccessBuffer;			//最后一次成功发送的字符串
	char					m_cSpiltBuffer[g_nRecMaxNum];			//数据解析缓存
	char					m_cRecvBuf[g_nRecMaxNum];
	int						m_nRecSize;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};

