#include "StdAfx.h"
#include "SqliteLink.h"
#include "../COMdll/SqliteCtrl.h"
#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CSqliteEngine::CSqliteEngine( CBEOPDataLinkManager* datalinker )
	:m_datalinker(datalinker)
	,m_sqlitesession(new CSqliteCtrl())
{

}

CSqliteEngine::~CSqliteEngine( void )
{
	if (m_sqlitesession){
		delete m_sqlitesession;
		m_sqlitesession = NULL;
	}
}

void CSqliteEngine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CSqliteEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	m_sqlitesession->GetValueSet(valuelist);
}

bool CSqliteEngine::Init()
{
	if (m_pointlist.empty()){
		return true;
	}
	m_sqlitesession->SetPointList(m_pointlist);
	return m_sqlitesession->Init();
}

void CSqliteEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CSqliteEngine::Exit()
{
	m_sqlitesession->Exit();
	return true;
}

bool CSqliteEngine::SetValue( wstring strName, wstring strValue )
{
	return m_sqlitesession->SetValue(strName,strValue);
}

void CSqliteEngine::SetIndex( int nIndex )
{
	if(m_sqlitesession)
	{
		m_sqlitesession->SetIndex(nIndex);
	}
}
