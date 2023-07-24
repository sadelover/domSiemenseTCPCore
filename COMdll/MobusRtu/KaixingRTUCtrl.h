#pragma once

#include "ModbusRTU.h"
#include "SerialPortCtrl.h"
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


//hash_map 4251
#pragma warning(disable:4251)

#define  _CMD_REV_LENTH_  0x100   //接收数据的长度
#define _CMD_KAIXING_LENTH_	 256	//协议长度

class CKaixingRTUCtrl : public CSerialPortCtrl
{
public:
	CKaixingRTUCtrl(void);
	virtual ~CKaixingRTUCtrl(void);

	bool	Init();
	bool	Exit();
	bool	ReInitPort();
	bool	InitCOMPort(UINT nPortnr, UINT bBaud, char parity = 'N');
	void	SetPortAndBaud(UINT nPortnr, UINT bBaud, char parity = 'N');
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);
	bool	InitDataEntryValue(const wstring& pointname, double value);

	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);

	void    SetNetworkError();
	void    ClearNetworkError();

	void    SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount=99,int nIDInterval = 500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false,bool bSaveErrLog = false);
	void	SetPointList(const vector<DataPointEntry>& pointlist);

	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	SetIndex(int nIndex);

	//
	bool	SetDebug(int nDebug);
	bool	OutPutModbusReceive(const unsigned char* pRevData, DWORD length,WORD wStart);
	string* byteToHexStr(unsigned char* byte_arr, int arr_len);  
	string  charToHexStr(unsigned char* byte_arr, int arr_len);  

	//////////////////////////////////////////////////////////////////////////
	void	SendReadCommandsByActive();
	HANDLE	GetSendCmdEvent();							//获取激活事件
	static void	SetSendCmdEvent();						//设置激活事件
		
	bool	SendConnectFirstStep();						//连接第一步
	bool	SendConnectSecondStep();					//连接第二步
	void	SendCmd(char *pData, BYTE bLength);			//
	BOOL	Is_DataValid(BYTE *pData, WORD dwLength);	//累加和 异或和校验
		
protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);


private:
	char *m_pSendData;					//the send package buffer
	CModbusRTU					m_Modbus;
	CRITICAL_SECTION			m_CriticalSection; //临界区保护
	
	WORD						m_StartAddress;		//起始地址 读命令。
	WORD						m_WriteStartAddress;		//起始地址 写命令。
	int							m_nReadNum;			//读取数量


public:
	HANDLE						m_hsendthread;
	HANDLE						m_hDataCheckRetryConnectionThread;
	bool						m_bExitThread;
	vector<DataPointEntry>		m_vecPointList;
	vector<_ModbusReadUnit>		m_ReadUnitList; //读取数据包单元，需合并点从而加速读取
	vector<DataPointEntry>		m_pointlist;	//点表.
	int							m_nSendReadCmdTimeInterval;				//两次命令间隔
	int							m_nRecevieTimeOut;						//接收超时
	int							m_nPollSendInterval;				//轮询间隔
	int							m_nRecvedSlave;
	int							m_nMutilReadCount;		//批量连读个数 默认99
	bool						m_bReadOneByOneMode;
	COleDateTime				m_oleUpdateTime;
	string						m_strErrInfo;
	int							m_nEngineIndex;
	int							m_nIDInterval;			//不同ID之间的间隔 默认500ms
	
	Project::Tools::Mutex		m_lock;
	Project::Tools::Mutex	m_lock_connection;
	Project::Tools::Mutex		m_lock_log;				//log锁
	map<wstring,DataPointEntry*>	m_mapErrPoint;		//读取错误点
	map<wstring,COleDateTime>	m_mapErrLog;			//log日志，五分钟存一次
	COleDateTime				m_oleLastUpdateLog;		//上一次保存log时间
	bool						m_bSaveErrLog;			//是否存储错误日志

	UINT						m_nPortnr;
	UINT						m_nBaud;
	char						m_cParity;
	bool						m_bInitPortSuccess;		//连接端口成功
	int							m_nModbusNetworkErrorCount;
	Beopdatalink::CLogDBAccess* m_logsession;
	int							m_nOutPut;				//输出接收到的cmd
	int							m_nPrecision;				//数据精度 默认6
	bool						m_bDisSingleRead;		//禁止连读失败后改为单点
	hash_map<wstring,DataPointEntry*> m_mapPointQuery;		//查找点位指针
	CRITICAL_SECTION			m_CriticalPointSection; //临界区保护

	//////////////////////////////////////////////////////////////////////////
	static HANDLE		m_sendCmdEvent;
	int					m_nEvent;								//事件编号 0:默认
	int					m_nReadTimeOut;							//超时时间
	bool				m_bNeedActive;							//需要激活事件
	static COleDateTime	m_oleSendCmdTime;						//发送命令时间
	bool				m_bConnectFirstSuccess;					//连接集中器成功
	BYTE				m_bSendCmd[_CMD_KAIXING_LENTH_];		//发送数据
	int					m_nConcentratorType;					//集中器类型
};