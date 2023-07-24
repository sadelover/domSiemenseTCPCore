#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

#include <vector>
#include <utility>
using std::vector;
using std::pair;


//class CMaster104Tcp;
class CLogDBAccess;
class CBEOPDataLinkManager;
class CProtocol104Ctrl;
class CProtocol104Engine
{
public:
	CProtocol104Engine(CBEOPDataLinkManager* datalinker);	

	~CProtocol104Engine(void);

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );

	bool	Init();
	bool	Exit();

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);	

private:
	vector<DataPointEntry>  m_pointlist;

	CProtocol104Ctrl* m_master104session;			//edit 20151022 104规约重连问题
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*	m_datalinker;
};