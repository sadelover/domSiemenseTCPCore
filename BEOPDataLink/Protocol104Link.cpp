#include "StdAfx.h"
#include "Protocol104Link.h"


//#include "../COMdll/Protocol104/Master104Ctrl.h"
#include "../COMdll/ModbusTcp/Protocol104.h"
#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CProtocol104Engine::CProtocol104Engine(CBEOPDataLinkManager* datalinker)
	:m_datalinker(datalinker)
	,m_master104session(new CProtocol104Ctrl())
{
}

CProtocol104Engine::~CProtocol104Engine(void)
{
	if (m_master104session){
		delete m_master104session;
		m_master104session = NULL;
	}
}

void CProtocol104Engine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

bool CProtocol104Engine::Exit()
{
	return m_master104session->Exit();
}

bool CProtocol104Engine::Init()
{
	if (m_pointlist.empty()){
		return true;
	}

	wstring serverhost;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		serverhost = entry.GetParam(1);
		if (!serverhost.empty()){
			break;
		}
	}
	if (serverhost.empty()){
		return false;
	}

	string serverhost_ansi = Project::Tools::WideCharToAnsi(serverhost.c_str());
	m_master104session->SetHost(serverhost_ansi);
	m_master104session->SetPointList(m_pointlist);
	return m_master104session->Init();
}

void CProtocol104Engine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CProtocol104Engine::AddLog( const wchar_t* loginfo )
{
	if (m_logsession){
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

void CProtocol104Engine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	m_master104session->GetValueSet(valuelist);
}
