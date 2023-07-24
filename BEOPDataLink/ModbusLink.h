#pragma once


/*
 *
 * the modbus engine will take care of the modubs communication.
 *
 *it will take use of the old modbus com class.
 *
 */

#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

#include <vector>
#include <utility>
using std::vector;
using std::pair;


class CModbusTcpCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;
class CModbusRTUCtrl;
class CModbusEngine
{
public:
	CModbusEngine(CBEOPDataLinkManager* datalinker);	

	~CModbusEngine(void);

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void    SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount=99,int nIDInterval = 500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false,bool bSaveErrLog = false);//golding
	void	SetReadOneByOneMode(bool bReadOneByOneMode);
	bool	Init();
	bool	Exit();

	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );

	void	SetValue(const wstring& pointname, double fValue);
	void	SetIndex(int nIndex);
	bool	GetValue(const wstring& pointname, double& result);
	bool    InitDataEntryValue(const wstring& pointname, double value);
	void	ExitThread();

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);

	void	SetDebug(int nDebug);

	vector<DataPointEntry> & GetPointList();

private:
	vector<DataPointEntry>  m_pointlist;
	bool					m_bTCPMode;		//默认启用TCPMode
	CModbusTcpCtrl*			m_modbussession;
	CModbusRTUCtrl*			m_modbusRtusession;
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*		m_datalinker;
	wstring					m_strServerhost;
	wstring					m_strServerCom;			//RTU Com
	wstring					m_strServerBaud;		//RTU 波特率
	wstring					m_strServerParity;		//RTU 奇偶性
	wstring					m_strServerPort;		//TCP 端口
	wstring					m_strServerIP;			//TCP IP
};

