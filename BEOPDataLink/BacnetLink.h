#pragma once

#include <vector>
#include <utility>
#include <string>
#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

class CBacnetCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;
class CBacnetEngine
{
public:
	CBacnetEngine(CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	dbsession_history);
	~CBacnetEngine(void);

	void	SetPointList(const std::vector<DataPointEntry>& pointlist);
	void	ClearPointList();
	void	SetDebug(int nDebug);
	bool	Init();
	bool	Exit();
	bool	SaveBacnetInfo();

	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );

	void	SetValue(const wstring& pointname, double value);
	void	InitDataEntryValue(const wstring& pointname, double value);
	
	bool	GetValue(const wstring& pointname, double& value);

	void	ExitThread();
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);

	string  GetExecuteOrderLog();
	void 	GetHostIPs(vector<string> & IPlist); //多IP的core的获取
private:
	std::vector<DataPointEntry>  m_pointlist;

	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
	CBacnetCtrl* m_bacnet_session;
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*	m_datalinker;
};

