#pragma once

#include "Tools/CustomTools/CustomTools.h"
#include "Tools/EngineInfoDefine.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include <hash_map>

class CBEOPDataLinkManager;
class CDiagnoseLink
{
public:
	CDiagnoseLink(CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	pDBsession);
	~CDiagnoseLink(void);

	bool	Init();
	bool    Exit();

	//return the value set.
	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );

private:
	static UINT WINAPI ThreadDiagnose(LPVOID lparam);
	bool	Diagnose();

	bool	GetCPUTemp();

private:
	CBEOPDataLinkManager*	m_pDataLinker;
	Beopdatalink::CCommonDBAccess*	m_pDBsession;
	HANDLE					m_hDiagnoseThread;
	bool					m_bExitThread;
	hash_map<string,string>	m_mapDiagnoseValue;
};