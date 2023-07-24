#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

#include <vector>
#include <utility>
using std::vector;
using std::pair;

class CWebHttpCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;

class CWebEngine
{
public:
	CWebEngine(CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	dbsession_history);	

	~CWebEngine(void);

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	bool	Init();
	bool	Exit();
	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);	
	void	SetIndex(int nIndex);
private:
	CWebHttpCtrl*				m_pWebHttpCtrl;	
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*		m_datalinker;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};