#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

#include <vector>
#include <utility>
using std::vector;
using std::pair;

class CSqlServerCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;

class CSqlServerEngine
{
public:
	CSqlServerEngine(CBEOPDataLinkManager* datalinker);	

	~CSqlServerEngine(void);

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	bool	SetValue(wstring strName, wstring strValue);
	bool	Init();
	bool	Exit();

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);	
	void	SetIndex(int nIndex);
private:
	vector<DataPointEntry>  m_pointlist;

	CSqlServerCtrl* m_SqlServersession;	
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*	m_datalinker;
};