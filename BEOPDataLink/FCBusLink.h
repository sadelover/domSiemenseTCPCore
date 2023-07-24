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


class CFCBusTcpCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;
class CFCBusEngine
{
public:
	CFCBusEngine(CBEOPDataLinkManager* datalinker);	

	~CFCBusEngine(void);

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void    SetSendCmdTimeInterval(int nTimeMs,int nPrecision=6);//golding
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

private:
	vector<DataPointEntry>  m_pointlist;

	CFCBusTcpCtrl* m_fcbussession;
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*	m_datalinker;
};

