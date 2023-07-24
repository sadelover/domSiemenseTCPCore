#include "StdAfx.h"
#include "SqlServerLink.h"

#include "../COMdll/SqlServerCtrl.h"

#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CSqlServerEngine::CSqlServerEngine( CBEOPDataLinkManager* datalinker )
	:m_datalinker(datalinker)
	,m_SqlServersession(new CSqlServerCtrl())
{

}

CSqlServerEngine::~CSqlServerEngine( void )
{
	if (m_SqlServersession){
		delete m_SqlServersession;
		m_SqlServersession = NULL;
	}
}

void CSqlServerEngine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CSqlServerEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	m_SqlServersession->GetValueSet(valuelist);
}

bool CSqlServerEngine::Init()
{
	if (m_pointlist.empty()){
		return true;
	}
	m_SqlServersession->SetPointList(m_pointlist);
	m_SqlServersession->SetLogSession(m_logsession);
	return m_SqlServersession->Init();
}

void CSqlServerEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CSqlServerEngine::Exit()
{
	m_SqlServersession->Exit();
	return true;
}

bool CSqlServerEngine::SetValue( wstring strName, wstring strValue )
{
	return m_SqlServersession->SetValue(strName,strValue);
}

void CSqlServerEngine::SetIndex( int nIndex )
{
	if(m_SqlServersession)
	{
		m_SqlServersession->SetIndex(nIndex);
	}
}
