#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "nodaveinclude/nodave.h"
#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

/*
B是Byte的意思，没有默认为X
VB,VW,VD  字节，字，双字寻址
输入映像寄存器				(I0.0~I15.7）按位操作			位
输出映像寄存器				（Q0.0~Q15.7）					位	
变量存储器V					(VB0.0~VB5119.7)  			位，字节，字，双字
内部标志位M(中间寄存器)		（M0.0~M31.7）		位，字节，字，双字
顺序控制继电器S				（S0.0~S31.7）			位，字节，字，双字
特殊标志位	SM				(SM0.0~SSM179.7)				位
局部存储器	L				（LB0.0~LB63.7）
定时器 T					（T0~T255）
计数器 C					(C0~C255)
模拟量输入/输出映像寄存器	（AI/AQ 0~62） 起始地址偶数字节地址 1个字长
累加器	AC					（AC0~AC3）		字节B,字W,双字D
高速计数器 HC				(HC0~HC5)		双子长的符号整数

*/

class __declspec(dllexport) CS7UDPCtrl
{
public:
	CS7UDPCtrl(wstring strPLCIPAddress, int nSlack, int nSlot);
	~CS7UDPCtrl(void);

	bool	Init();
	bool    Exit();
	void	AddLog( const wchar_t* loginfo );
	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	DataPointEntry*	FindEntry(int nFrameType, int nPointID);


	bool	InitPLCConnection();
	bool	ExitPLCConnection();

	bool	ReadOneByOneAndRemoveErrPoint(int nFromIndex, int nToIndex);		//一个个点发送,并移除出错点
	int    ReadOneDataByIndex(int nIndex);
	bool	GetReadOneByOneSuccess();

	bool	ReadDataOnce();
	bool    ReadDataIndex(int nFromIndex, int nToIndex);

	bool	SetValue(const wstring& pointname, double fValue);
	void	InitDataEntryValue(const wstring& pointname, double fvalue); //不发送通信指令, add by golding

	bool	GetValue(const wstring& pointname, double& result);

	bool	GetConnectSuccess();
	void    SetConnectSuccess(bool bSuccess);

	void	EnableLog(BOOL bEnable);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);

	void    SetRebootPointNameValueCondition(vector<CString> strNameList, vector<double> fValueList);
	void	SetPLCError();
	void	ClrPLCError();
	int		GetErrCount();

	wstring GetIP();


	VARENUM GetPointDBPos(wstring strPLCAddress, int &nPos, int &nOffset, int &nBit,int &nBlock,VARENUM varType = VT_INT);
	VARENUM GetPointDBPos_200(wstring strPLCAddress, int &nPos, int &nOffset, int &nBit,int &nBlock,VARENUM varType = VT_INT);
	VARENUM GetPointPos_200(wstring strPLCAddress,  int &nOffset, int &nBit,VARENUM varType = VT_INT);			//单首字母
	VARENUM GetPointPos_200_(wstring strPLCAddress,  int &nOffset, int &nBit,VARENUM varType = VT_INT);			//双首字母


	//错误log
	string	GetExecuteOrderLog();
	SYSTEMTIME	m_sExecuteTime;
	string		m_strExecuteLog;

	bool	OutPutLogString(wstring strOut);

	void	SetIndex(int nIndex);
	void	SetHistoryDbCon(Beopdatalink::CCommonDBAccess* phisdbcon);
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);

public:
	wstring	m_strCPUAddress;
private:
	Mutex						m_mutex;
	Project::Tools::Mutex		m_lock;		//用于操作连接
	
	int m_nSlack;
	int m_nSlot;

	vector<DataPointEntry> m_pointlist;	//点表.
	map<wstring, int> m_mapNameIndex; //名称与索引的关系map
	
	vector<VARENUM> m_pointVarTypeList; //DB区地址
	vector<int> m_pointDBIndexList; //DB区地址
	vector<int> m_pointOffsetIndexList; //DB偏移量
	vector<int> m_pointOffsetBitList; //Bit
	vector<int>	m_pointBlockArea;		//寄存器块


	Beopdatalink::CLogDBAccess* m_logsession;

	//PLC Connection by DAVE
	daveInterface * m_pPLCInterface;
	daveConnection * m_pPLCConnection;
	bool			m_bPLCConnectionSuccess;		//连接标志位

	int				m_nPLCReadErrorCount;
	HANDLE			m_hTCPSocket;

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;

	bool            m_bFirstRunReadOnce;


	vector<CString> m_strRebootPointNameList; //触发重启程序的点名
	vector<double> m_fRebootPointValueList; //触发重启程序的点值
};

