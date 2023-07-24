#pragma once


#include <vector>
#include <utility>
#include <string>
#include "BEOPDataPoint/DataPointEntry.h"
#include "DB_BasicIO/RealTimeDataAccess.h"

class CS7UDPCtrl;
class CLogDBAccess;
class CBEOPDataLinkManager;
class CCommonDBAccess;

struct _SiemensPLCProp
{
	wstring strIP;
	int nSlack;
	int nSlot;
} ;

class CSiemensLink
{
public:
	CSiemensLink(CBEOPDataLinkManager* datalinker, vector<_SiemensPLCProp> propPLCList,Beopdatalink::CCommonDBAccess*	dbsession_history);
	~CSiemensLink(void);

	void	SetPointList(const std::vector<DataPointEntry>& pointlist);

	bool	Init(CString strRebootPointValueDefine);
	bool	Exit();

	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );

	bool	SetValue(const wstring& pointname, double fValue);
	bool	SetValue(const wstring& pointname, float fValue);
	void	InitDataEntryValue(const wstring& pointname, double fValue);

	bool	GetValue(const wstring& pointname, double& fValue);

	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);

	//thread
	static UINT WINAPI ThreadReadAllPLCData(LPVOID lparam);
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	void	TerminateReadThread();

	vector<string> GetExecuteOrderLog();

	int    GetErrrConnectCount();

private:
	std::vector<DataPointEntry>  m_pointlist;
	std::map<wstring , CS7UDPCtrl*> m_mapS7Ctrl;
	vector<CS7UDPCtrl *> m_pS7CtrlList;
	Beopdatalink::CLogDBAccess* m_logsession;
	CBEOPDataLinkManager*	m_datalinker;


	Project::Tools::Mutex	m_lock;
	bool m_bExitThread;

	HANDLE m_hDataReadThread;
	HANDLE m_hDataCheckRetryConnectionThread;
	Beopdatalink::CCommonDBAccess*	m_dbsession_history;
};

