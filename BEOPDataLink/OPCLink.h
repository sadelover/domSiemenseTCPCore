#pragma once




/*
 **
 **
 **
 ** this class defines the opc engine.\
 ** it may include more than one opc ctrls for reading or writing files. 
 **	
 **
 **
 **/

#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

#include "DB_BasicIO/RealTimeDataAccess.h"
#include "BEOPDataPoint/DataPointEntry.h"


class COPCConnection_AutoBrowse;
class CLogDBAccess;
class CBEOPDataLinkManager;


class COPCEngine
{
public:
	COPCEngine(CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	dbsession_history, wstring strOPCServerIP,int nSleep = 5,int nMutilCount =1,bool bUseThread = false,int nSleepFromDevice = 50,int nPollSleep = 5,int nPollSleepFromDevice = 60,bool bDisableCheckQuality=false,int nPrecision=6,bool bAsync20Mode=false,int nRefresh20Interval = 60, int nLanguageID=0, int nMultilAddItems =1000, int nDebugOPC=0, int nUpdateRate =500);
	~COPCEngine(void);

	void	SetOpcPointList(const vector<DataPointEntry>& pointlist);
	void	SetMultiMode(bool bMultiMode);
	void	SetDebug(int nDebug,int nOPCCheckReconnectInterval=5,int nOPCReconnectInertval=10,int nReconnectMinute = 0,bool bEnableSecurity=false,int nOPCMainThreadOPCPollSleep = 2);
	bool	Init(int nOPCClientIndex = -1);
	bool	Exit();

	void	GetValueSet(vector< pair<wstring, wstring> >& valuelist );

	static UINT WINAPI ThreadUpdateOPCData(LPVOID lparam);
	static UINT WINAPI ThreadConnectOPCData(LPVOID lparam);
	bool	ReConnectOPC();
	bool	ReLoadItemHandle();					//因为线程安全的缘故 只能在主进程去做
	bool	UpdateMutilOPCData();
	bool	UpdateSingleOPCData();
	bool	UpdateOPCDataBuffer();				//跟新OPC点缓存

	vector<DataPointEntry> & GetPointList();


	//////////////////////////////////////////////////////////////////////////
	//20180926
	void	GetValueListV1(vector< pair<wstring, wstring> >& valuelist );	//从pointlist中返回值
	bool	UpdateMutilOPCDataV1();
	bool	UpdateSingleOPCDataV1();
	bool	UpdateOPCDataBufferV1();				//更新新OPC点缓存
	bool	ReReadAsync20ItemV1();			//异步写值后 每次拿值时候 先批量读取更新内存中异步回调写成功的点位
	bool	GetValueFromOPCItemV1();				//20180926 从OPCitem中把值更新到pointlist
	bool	GetMutilOPCDataV1(int nStart, int nEnd, vector<DataPointEntry>& pointlist, vector<wstring> & strBadPointList);
	//////////////////////////////////////////////////////////////////////////

	bool	Refresh20(bool bDeviceRead = false);		//异步刷新全部数据
	bool	GetMutilOPCData(int nStart, int nEnd, vector<DataPointEntry>& pointlist);
	double	GetBitFromFloat(float dValue, int nIndex);			//从第一位开始计数
	int		GetBitFromInt(int nValue, int nIndex);
	wstring	GetBitFromString(wstring strValue, int nIndex);
	bool	ReReadAsync20Item();			//异步写值后 每次拿值时候 先批量读取更新内存中异步回调写成功的点位

	double	SetBitToFloat(double nData, int nIndex,double nValue);
	wstring	SetBitToString(double nData, int nIndex,wstring strValue);

	void	ClearResource();
	static COPCConnection_AutoBrowse* CreateOPCObject(OPCSERVER_TYPE type);

	//从opc server中读值
	bool	SetValue(const wstring& pointname, double value);
	bool	SetValue(const wstring& pointname, wstring strvalue);
	bool	GetValue(const wstring& pointname, wstring& strvalue);
	bool	GetValue(const wstring& pointname, bool& strvalue);
	bool	GetValue(const wstring& pointname, int& strvalue);
	bool	GetValue(const wstring& pointname, double& strvalue);

	//初始化值
	bool InitDataEntryValue(const wstring& pointname, double value);

	//直接从plc读值
	bool	GetValueFromDevice(const wstring& pointname, wstring& strvalue);
	bool	GetValueFromDevice(const wstring& pointname, bool& strvalue);
	bool	GetValueFromDevice(const wstring& pointname, int& strvalue);
	bool	GetValueFromDevice(const wstring& pointname, double& strvalue);

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);

	void	OutErrLog(wstring strLog);
	void	SumReadAndResponse();						//每次发送前统计上一次的收发情况
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	vector<DataPointEntry>  m_pointlist;
	COPCConnection_AutoBrowse* m_opcsession;		// opc engine will own the object.
	Beopdatalink::CLogDBAccess* m_logsession;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
	CBEOPDataLinkManager*	m_datalinker;

	wstring m_strOPCServerIP;
	bool	m_bMultiClientMode;
	bool	m_bConnectOK;
	HANDLE  m_hDataReadThread;
	HANDLE  m_hOPCConnectThread;
	HANDLE  m_hDataReadDeviceThread;
	bool	m_bExitThread;
	bool	m_bUseThread;			//使用线程去读取
	int		m_nMutilCount;			//批量读取个数
	int		m_nSleep;				//OPC读点间隔（从缓存）
	int		m_nReSponse;
	int		m_nReadCount;					//OPC读数量(缓存)
	int		m_nReadDeviceCount;				//OPC读数量(设备)
	int		m_nReSponseDevice;				//OPC反馈(设备)
	int		m_nOPCClientIndex;
	int		m_nOPCCmdSleepFromDevice;		//OPC读点间隔(从设备)
	int		m_nOPCPollSleep;				//OPC轮询间隔（从缓存）
	int		m_nOPCPollSleepFromDevice;		//OPC轮询间隔(从设备)
	int		m_nOPCCheckReconnectInterval;	//OPC检测重连间隔（min）
	int		m_nOPCReconnectInertval;		//OPC N分钟数据未更新重连(min)
	bool	m_bEnableSecurity;				//开启进程安全设置项
	bool	m_bDisableCheckQuality;			//开启禁用品质数据检测
	int		m_nOPCMainThreadOPCPollSleep;	//OPC轮询间隔s（主进程从缓存）
	int		m_nMainUpdateIndex;				//OPC主进程更新次数

	COleDateTime m_oleDeviceTime;
	COleDateTime m_oleUpdateTime;			//更新时间,连接成功后，若10分钟内未更新成功一次，即重连
	COleDateTime m_oleReConnectTime;		//重连后N分钟内不存储数据
	int			 m_nReconnectMinute;		//重连后N分钟
	bool		 m_bReconnect;				//重连标志位
	Project::Tools::Mutex		m_lock;		//用于操作连接
	int			 m_nSingleReadCount;
	int			 m_nReloadErrItem;			//主进程获取数据N次时候重新加载无效的Item
	hash_map<wstring, double>	m_PositionValuemap;		//映射取位的点名和值的关联
	int			m_nPrecision;				//数据精度 默认6
	bool		m_bAsync20Mode;				//异步读写模式
	COleDateTime m_oleRefresh20;			//异步刷新时间
	int			m_nRefresh20Interval;		//异步刷新间隔
	int			m_nLanguageID;				//语言ID
	int			m_nMultiAddItems;			//批量添加点
	int			m_nDebugOPC;				//调试OPC
	int			m_nUpdateRate;			//向服务器发送请求的刷新率
	string m_strUpdateLog;
};

