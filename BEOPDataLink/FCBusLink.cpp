#include "StdAfx.h"
#include "FCBusLink.h"


#include "../COMdll/ModbusTcp/FCBusTcpCtrl.h"
#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CFCBusEngine::CFCBusEngine(CBEOPDataLinkManager* datalinker)
	:m_datalinker(datalinker)
	,m_fcbussession(new CFCBusTcpCtrl())
{
}


CFCBusEngine::~CFCBusEngine(void)
{
	if (m_fcbussession){
		delete m_fcbussession;
		m_fcbussession = NULL;
	}
}

void CFCBusEngine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CFCBusEngine::SetSendCmdTimeInterval(int nTimeMs,int nPrecision)
{
	m_fcbussession->SetSendCmdTimeInterval(nTimeMs,nPrecision);
}

bool CFCBusEngine::Init()
{
	if (m_pointlist.empty()){
		return true;
	}

	wstring serverhost,serverport;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		serverhost = entry.GetFCServerIP();
		serverport = entry.GetFCServerPort();
		if (!serverhost.empty()||!serverport.empty()){
			break;
		}
	}
	if (serverhost.empty()||serverport.empty()){
		return false;
	}

	string serverhost_ansi = Project::Tools::WideCharToAnsi(serverhost.c_str());
	m_fcbussession->SetHost(serverhost_ansi);
	m_fcbussession->SetPort(_wtoi(serverport.c_str()));
	m_fcbussession->SetPointList(m_pointlist);
	m_fcbussession->SetLogSession(m_logsession);
	return m_fcbussession->Init();
}

void CFCBusEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	return m_fcbussession->GetValueSet(valuelist);
}

void CFCBusEngine::SetValue( const wstring& pointname, double fValue )
{
	m_fcbussession->SetValue(pointname, fValue);
	
}

void CFCBusEngine::ExitThread()
{
	if (m_fcbussession)
	{
		m_fcbussession->Disconnect();
		m_fcbussession->TerminateSendThread();
	}
}

bool CFCBusEngine::GetValue( const wstring& pointname, double& result )
{
	if (m_fcbussession){
		return m_fcbussession->GetValue(pointname, result);
	}
	return false;
}

bool CFCBusEngine::InitDataEntryValue(const wstring& pointname, double value)
{
	if (m_fcbussession){
		return m_fcbussession->InitDataEntryValue(pointname, value);
	}
	return false;
}

void CFCBusEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CFCBusEngine::AddLog( const wchar_t* loginfo )
{
	if (m_logsession){
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

bool CFCBusEngine::Exit()
{
	if (m_fcbussession)
	{
		return m_fcbussession->Exit();
	}
	return true;
}

void CFCBusEngine::SetIndex( int nIndex )
{
	if (m_fcbussession)
	{
		m_fcbussession->SetIndex(nIndex);
	}
}
