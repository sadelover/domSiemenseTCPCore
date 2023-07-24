#include "StdAfx.h"
#include "BacnetServerEngine.h"

#include "../COMdll/bacnet/BacnetServerComm.h"


CBacnetServerEngine::CBacnetServerEngine(CBEOPDataLinkManager* datalinker)
	:m_datalinker(datalinker)
{
	m_bacnet_server_session = new CBacnetServerCtrl;
}


CBacnetServerEngine::~CBacnetServerEngine(void)
{
}


void CBacnetServerEngine::SetPointList( const std::vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

bool CBacnetServerEngine::Init()
{
	m_bacnet_server_session->SetPointList(m_pointlist);
	m_bacnet_server_session->SetLogSession(m_logsession);
	return m_bacnet_server_session->StartBacnetServer(); //开始启动server在线
}

void CBacnetServerEngine::GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist )
{
	//return m_bacnet_server_session->GetValueSet(valuelist);
}

void CBacnetServerEngine::SetValue( const wstring& pointname, double value )
{
	//return m_bacnet_server_session->SetValue(pointname, value);
}

void CBacnetServerEngine::InitDataEntryValue( const wstring& pointname, double value )
{
	//return m_bacnet_server_session->InitDataEntryValue(pointname, value);

}

void CBacnetServerEngine::ExitThread()
{
	return m_bacnet_server_session->Exit();
}

bool CBacnetServerEngine::GetValue( const wstring& pointname, double& value )
{
//	if(m_bacnet_server_session)
//		return m_bacnet_server_session->GetValue(pointname, value);

	return false;
}

void CBacnetServerEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CBacnetServerEngine::Exit()
{
	m_bacnet_server_session->Exit();
	return true;
}
