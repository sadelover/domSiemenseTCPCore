#pragma once

#include "TcpClient.h"
#include "Tools/CustomTools/CustomTools.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

const int g_nDogRecMaxNum = 8192;
const int g_nDogPackageSize = 4096;	//每个数据包长度

struct dogpackage				//分包一个包
{
	dogpackage()
	{
		nLength = 0;
		memset(cData,0,g_nDogPackageSize);
	}
	char   cData[g_nDogPackageSize];				//内容
	int	   nLength;					//长度
};

enum E_SLAVE_MODE
{
	E_SLAVE_CORE =1,
	E_SLAVE_LOGIC,
	E_SLAVE_UPDATE,
	E_SLAVE_MASTER,
	E_SLAVE_DTUENGINE,
};

enum E_SLAVE_REASON
{
	E_REASON_NULL = 0,
	E_REASON_REMOTE,		//远程命令
	E_REASON_UPDATE,		//更新升级

};

class CDogTcpCtrl : public CTcpClient
{
public:
	CDogTcpCtrl();
	virtual ~CDogTcpCtrl();

protected:
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:

	static CDogTcpCtrl* GetInstance(bool bConnected = true);  // get the singleton instance of assign object
	static void DestroyInstance();                   // destroy the singleton instance of assign objec

	//Initialize Tcp Info
	bool	CloseTcp();
	bool	Init(bool bConnected = true);
	bool	Exit();
	bool	GetConnectSuccess();
	bool	GetDogRestartFunctonOK();				//版本号支持 连接成功
	bool	SetDogMode(E_SLAVE_MODE eMode);				
	bool	SendActiveSignal();
	bool	SendRestart(int nRestartExe, int nRestartType=-1);		//重启 1：Core  2:Watch
	bool	SendRestart(E_SLAVE_MODE nRestartExe, E_SLAVE_REASON nReason);	

	bool	GetRestartSuccess(bool& bRestartCore,bool& bRestartWatch);
	bool	GetRestartSuccess(bool& bRestartCore,bool& bRestartLogic,bool& bRestartUpdate,bool& bRestartMaster);
	bool	SetRestartCoreSuccess(bool bRestartCore);
	bool	SetRestartLogicSuccess(bool bRestartLogic);
	bool	SetRestartUpdateSuccess(bool bRestartUpdate);
	bool	SetRestartMasterSuccess(bool bRestartMaster);

	bool	SetRestartWatchSuccess(bool bRestartWatch);

protected:
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	bool	ReConnect();
	void    ClearNetworkError();

private:
	bool	AnalyzeReceive(const unsigned char* pRevData, DWORD length);	
	bool	WriteToPort(char *pData,int nLen);
	bool	WriteToPortByWrap(char *pData,int nLen);			//自动添加换行符
	bool	HandleRecData(const char* buffer,int nSize);
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<dogpackage>& resultlist);		//争对;\n操作
	string GetHostIP();
private:
	string					m_host;
	u_short					m_port;
	Project::Tools::Mutex	m_reveivelock;
	Project::Tools::Mutex	m_lock;
	//自动重连机制
	Project::Tools::Mutex	m_lock_connection;
	Project::Tools::Mutex	m_lock_send;
	HANDLE					m_hDataCheckRetryConnectionThread;
	int						m_nModbusNetworkErrorCount;
	bool					m_bExitThread;
	bool					m_bConnectSuccess;			//连接成功标志位
	bool					m_bRestartCoreSuccess;		//重启Core成功
	bool					m_bRestartLogicSuccess;		//重启Core成功
	bool					m_bRestartUpdateSuccess;		//重启Core成功
	bool					m_bRestartMasterSuccess;		//重启Core成功

	bool					m_bRestartWatchSuccess;		//重启Watch成功

	char					*m_szWriteBuffer;			//待发送内容
	int						m_nWriteSize;				//待发送大小
	int						m_nReConnectInterval;		//重连间隔
	E_SLAVE_MODE			m_nMode;					
	
	char					m_cSpiltBuffer[g_nDogRecMaxNum];			//数据解析缓存
	char					m_cRecvBuf[g_nDogRecMaxNum];
	int						m_nRecSize;

	static CDogTcpCtrl*     m_csDogCommObj;     // singleton object
	static Mutex            m_mutexLock;                    // mutex object

	wstring					m_strMasterVersion;
};

