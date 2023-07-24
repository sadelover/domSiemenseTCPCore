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

class CModbusRTUCtrl : public CSerialPortCtrl
{
public:
	CModbusRTUCtrl(void);
	virtual ~CModbusRTUCtrl(void);

	bool	Init();
	bool	Exit();
	bool	ReInitPort(int nType=0);
	void	SetPortAndBaud(UINT nPortnr, UINT bBaud, char parity = 'N');
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	void	GetValueByDataTypeAndRange(DataPointEntry entry,vector< pair<wstring, wstring> >& valuelist);	
	void	GetValueByDataTypeAndRange(DataPointEntry entry,E_MODBUS_VALUE_TYPE type,vector<WORD>& vecValue,vector< pair<wstring, wstring> >& valuelist);
	void	GetValueByDataTypeAndRange(DataPointEntry entry,double dValue,vector<WORD>& vecCmd,bool bOrder);		//ture默认从高字节往低排

	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);
	bool	InitDataEntryValue(const wstring& pointname, double value);

	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);

	////使用激活命令模式//////////////////////////////////////////////////////////////////////
	void	SendReadCommandsByActive();				//回复激活模式
	void	SetSendCmdEvent();						//设置激活事件
	HANDLE	GetSendCmdEvent();						//获取激活事件
	void	SendOneByOneByActive();
	bool	SendOneByOneByActive(_ModbusReadUnit& unit);
	bool	SendReadMutilByActive(_ModbusReadUnit& unit);		//修复写值顺序错乱 20190132
	bool	SetValueByActive(const wstring& pointname, double fValue);
	//////////////////////////////////////////////////////////////////////////

	void	SetReadCommands_();
	void	SendOneByOne_();
	bool	SendOneByOne_(_ModbusReadUnit& unit);
	bool	SendReadMutil_(_ModbusReadUnit& unit);
	bool	SendWriteCmd(DataPointEntry entry,double fValue);			//设备通讯类型
	bool	SendWriteMutilCmd(DataPointEntry entry,double fValue);			//设备通讯类型
	bool	SendWriteBYTECmd(DataPointEntry entry,double fValue);			//设备通讯类型
	bool	SendWriteWORDCmd(DataPointEntry entry,double fValue);			//设备通讯类型
	void    SetNetworkError();
	void    ClearNetworkError();

	void    SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount=99,int nIDInterval = 500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false,bool bSaveErrLog = false);
	void	SetReadOneByOneMode(bool bReadOneByOneMode);
	void	SetPointList(const vector<DataPointEntry>& pointlist);

	DataPointEntry*	FindEntry_(DWORD slaveid, DWORD headaddress, DWORD funccode);
	DataPointEntry*	FindEntryFromQueryCache_(DWORD slaveid, DWORD headaddress, DWORD funccode);
	void	SetModbusReadUnitErrFlag(unsigned int nSlaveId,DWORD dwFuncCode,DWORD dwAdd,bool bHasErrPoint=true);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	SetIndex(int nIndex);

	void    SortPointByDevice_();
	bool	CombineReadUnit(const _ModbusReadUnit &  mrUnit,bool bModbusEnd=true);
	void	CreateEntryByDataTypeAndRange(DataPointEntry entry,vector<DataPointEntry>& vecModbus);

	//转换函数
	int		GetBitFromWord(WORD nData, int nIndex);
	WORD	SetBitToWord(WORD nData, int nIndex,int nValue);
	DWORD	MergeTwoWORD(WORD wGao,WORD wDi,bool bOrder = true);		//
	double	MergeFourDouble(WORD wGaoGao,WORD wGao,WORD wDi,WORD wDiDi,bool bOrder = true);		//
	double	strtodbl(const wchar_t * str);

	//
	bool	SetDebug(int nDebug);
	bool	OutPutModbusReceive(const unsigned char* pRevData, DWORD length,WORD wStart);
	string* byteToHexStr(unsigned char* byte_arr, int arr_len);  
	string  charToHexStr(unsigned char* byte_arr, int arr_len);  

	int		GetMultiReadCount(DWORD slaveid, DWORD headaddress, DWORD funccode);

	bool	InsertLog(const wstring& loginfo, COleDateTime oleTime);
	bool	SaveLog();
	void	SumReadAndResponse();	
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

	//Initialize hash_map Info
	bool Init_HashMap(void);

	void SendCmd(char *pData, BYTE bLength,WORD nStartAddress = 0,bool bWrited=false);
	BOOL Is_DataValid(BYTE *pData, WORD dwLength);

public:
	//Initialize COMPort Info
	bool InitCOMPort(UINT nPortnr, UINT bBaud, char parity = 'N');

	//Send Read Command
	virtual bool SendReadCmd(unsigned int nSlaveId   //设备的Slave ID 号
		, WORD wAddrFrom		//读取属性的起始地址
		, WORD wAddrTo			//读取属性的终止地址
		, WORD wFunc);			//设备通讯类型

	//Send Write Command
	virtual bool SendWriteCmd(unsigned int nSlaveId   //设备的Slave ID 号
		, WORD wAddress			//写入属性的地址
		, WORD wValue			//属性值
		, WORD wFunc);			//设备通讯类型

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
	int							m_nCmdCount;						//命令个数
	int							m_nResponseCount;					//请求回复个数
	int							m_nUpdatePointCount;				//更新点
	string						m_strUpdateLog;						//更新log

	_ModbusReadUnit				m_lastReadUnit;						//上一次读取结构
	DataPointEntry				m_lastReadEntry;
	HANDLE						m_sendCmdEvent;

	Project::Tools::Mutex		m_lock_send_log;				//发送锁
};