#pragma once

#include "TcpIpComm.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "Tools/StructDefine.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

#define _CMD_CO3P_LENTH_	    0x08		//协议长度

/*
	param1:地址号
	param2:管理器地址
	param3:终端机地址
	param4:受控终端机的类型符
	param5:特定的操作命令
	param6:数据位置
	param7:数据格式
		5.2 整数部分长度.小数部分长度
		1/16 取位
*/


class CCKECO3PTcpCtrl : public CTcpIpComm
{
public:
	CCKECO3PTcpCtrl(void);
	virtual ~CCKECO3PTcpCtrl(void);

protected:
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:
	void	SetHost(const string& strhost);
	void	SetPort(u_short portnumer);
	void	SetIndex(int nIndex);
	void    SetSendCmdTimeInterval(int nTimeMs,int nRollInterval=2,int nTimeOut=5000,int nPrecision=6);

	//Initialize Modbus Tcp Info
	bool	Init();
	bool	Exit();

	bool	SumValid(const unsigned char* pRevData,int nStartIndex,int len);			//累加和校验
	unsigned  char Sum__(const unsigned char* pRevData,int nStartIndex,int len);		//累加和校验码

	//Send Read Command
	virtual bool SendReadCmd(WORD wAddr1			//管理器地址, 采用二进制表示，长度为 1Byte，寻址范围为 00—7FH，
							, WORD wAddr2			//终端机地址 ，长度为 1Byte，寻址范围为 00—7FH，FFH 为特殊地址，采用广播方式时，使用该地址 
							, WORD wType			//受控终端机的类型符，长度为 1Byte, 采用压缩 BCD 码表示。 
							, WORD wCmd);			//代表特定的操作命令，长度为 1Byte

	void	SendReadCommands();
	bool	SendReadMutil_(_CKECO3PReadUnit& unit);
	
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	void	TerminateSendThread();

	//////////////////////////////////////////////////////////////////////////
	_CKECO3PReadUnit* FindCKECO3PReadUnit(WORD wAddr1,WORD wAddr2,WORD wType,WORD wCmd);
	void	SetPointList(const vector<DataPointEntry>& pointlist);
	bool	CombineReadUnit(const _CKECO3PReadUnit &  mrUnit);
	
	bool	ReConnect();
	void    SetNetworkError();
	void    ClearNetworkError();

	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	bool    GetValueByDataTypeAndRange(DataPointEntry entry,wstring& strValue);
	int		GetBitFromWord(WORD nData, int nIndex);
	bool	UpdateValue();
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	Project::Tools::Mutex	m_lock;
	string					m_host;
	u_short					m_port;
	WORD					m_wAddr1;
	WORD					m_wAddr2;
	WORD					m_wCmd;
	WORD					m_wType;
	HANDLE					m_hsendthread;
	HANDLE					m_hDataCheckRetryConnectionThread;
	bool					m_bExitThread;
	bool					m_bReSponseSuccess;			//回复正常

	vector<_CKECO3PReadUnit> m_ReadUnitList;
	vector<DataPointEntry>	m_vecPointList;
	BYTE					m_SendBuffer[_CMD_CO3P_LENTH_];	//发送数据

	//自动重连机制
	Project::Tools::Mutex	m_lock_connection;
	int						m_nNetworkErrorCount;
	int						m_nRollInterval;			//轮询间隔
	int						m_nSendReadCmdTimeInterval;				//两次命令间隔
	int						m_nRecevieTimeOut;						//接收超时
	COleDateTime			m_oleUpdateTime;
	int						m_nEngineIndex;
	int						m_nPrecision;				//数据精度 默认6
};

