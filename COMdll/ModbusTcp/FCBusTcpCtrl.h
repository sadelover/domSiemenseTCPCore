#pragma once

#include "TcpIpComm.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"

#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

#include "DB_BasicIO/RealTimeDataAccess.h"

#define _FCBTCP_PD_CMD_LENTH_		  0x08     //FCBusTcp 过程数据命令长度
#define _FCBTCP_PC_CMD_LENTH_		  0x10    //FCBusTcp 参数通道命令长度

class CFCBusTcpCtrl : public CTcpIpComm
{
public:
	CFCBusTcpCtrl(void);
	virtual ~CFCBusTcpCtrl(void);

protected:
	
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:
	void	SetHost(const string& strhost);
	void	SetPort(u_short portnumer);

	//Initialize Modbus Tcp Info
	bool Init();
	bool	Exit();

	//Send Read Command
	virtual bool SendReadCmd(WORD dAddress);

	//Send Write Command
	virtual bool SendWriteCmd( WORD wAddress			//写入属性的地址
							, WORD wValue				//属性值
							, bool bOpened = true);		//默认写值时候为开

	virtual bool SendWriteCmd( WORD wAddress			//写入属性的地址
		, DWORD wValue);			//属性值

	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	void	TerminateSendThread();

	void	SetReadCommands();
	void    SetSendCmdTimeInterval(int nTimeMs,int nPrecision=6);
	void	SetPointList(const vector<DataPointEntry>& pointlist);
	
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );

	DataPointEntry*	FindEntry(DWORD headaddress);

	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);

	bool	InitDataEntryValue(const wstring& pointname, double value);

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	EnableLog(BOOL bEnable);
	bool	ReConnect();

	void    SetNetworkError();
	void    ClearNetworkError();

	DWORD HexChartoDec(char address[4]);
	void	SetIndex(int nIndex);
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	string		m_host;
	u_short		m_port;
	HANDLE		m_hsendthread;
	bool		m_bExitThread;
	vector<DataPointEntry> m_pointlist;	//点表.
	Project::Tools::Mutex	m_lock;
	BOOL		m_bLog;
	Beopdatalink::CLogDBAccess* m_logsession;
	int			m_nSendReadCmdTimeInterval;				//两次命令间隔

	char        m_SendPDBuffer[_FCBTCP_PD_CMD_LENTH_];      // 过程数据命令
	char        m_SendPCBuffer[_FCBTCP_PC_CMD_LENTH_];      // 参数通道命令

	//自动重连机制
	Project::Tools::Mutex	m_lock_connection;
	HANDLE		m_hDataCheckRetryConnectionThread;
	int			m_nFCBusNetworkErrorCount;

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;
	int				m_nPrecision;				//数据精度 默认6
};

