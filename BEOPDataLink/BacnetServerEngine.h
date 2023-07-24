#pragma once


#include <vector>
#include <utility>
#include <string>
#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

class CBacnetServerCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;

class CBacnetServerEngine
{
public:
	CBacnetServerEngine(CBEOPDataLinkManager* datalinker);
	~CBacnetServerEngine(void);


	void	SetPointList(const std::vector<DataPointEntry>& pointlist);

	bool	Init();
	bool	Exit();

	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );

	void	SetValue(const wstring& pointname, double value);
	void	InitDataEntryValue(const wstring& pointname, double value);

	bool	GetValue(const wstring& pointname, double& value);

	void	ExitThread();
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);


private:

	std::vector<DataPointEntry>  m_pointlist;
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*	m_datalinker;
	CBacnetServerCtrl *m_bacnet_server_session;

};

