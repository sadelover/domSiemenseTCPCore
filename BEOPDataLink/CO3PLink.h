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


class CCKECO3PTcpCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;
class CCO3PEngine
{
public:
	CCO3PEngine(CBEOPDataLinkManager* datalinker);	

	~CCO3PEngine(void);

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void    SetSendCmdTimeInterval(int nTimeMs,int nRollInterval=2,int nTimeOut=5000,int nPrecision=6);
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

private:
	vector<DataPointEntry>  m_pointlist;
	bool					m_bTCPMode;		//默认启用TCPMode
	CCKECO3PTcpCtrl*			m_co3psession;
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*		m_datalinker;
	wstring					m_strServerhost;
	wstring					m_strServerCom;			//RTU Com
	wstring					m_strServerBaud;		//RTU 波特率
	wstring					m_strServerParity;		//RTU 奇偶性
	wstring					m_strServerPort;		//TCP 端口
	wstring					m_strServerIP;			//TCP IP
};

