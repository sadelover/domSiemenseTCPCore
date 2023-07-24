#include "StdAfx.h"
#include "MysqlLink.h"

#include "../COMdll/MysqlCtrl.h"

#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CMysqlEngine::CMysqlEngine( CBEOPDataLinkManager* datalinker )
	:m_datalinker(datalinker)
	,m_mysqlsession(new CMysqlCtrl())
{

}

CMysqlEngine::~CMysqlEngine( void )
{
	if (m_mysqlsession){
		delete m_mysqlsession;
		m_mysqlsession = NULL;
	}
}

void CMysqlEngine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CMysqlEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	m_mysqlsession->GetValueSet(valuelist);
}

bool CMysqlEngine::Init()
{
	if (m_pointlist.empty()){
		return true;
	}
	m_mysqlsession->SetPointList(m_pointlist);
	m_mysqlsession->SetLogSession(m_logsession);
	return m_mysqlsession->Init();
}

void CMysqlEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CMysqlEngine::Exit()
{
	m_mysqlsession->Exit();
	return true;
}

bool CMysqlEngine::SetValue( wstring strName, wstring strValue )
{
	return m_mysqlsession->SetValue(strName,strValue);
}

void CMysqlEngine::SetIndex( int nIndex )
{
	if(m_mysqlsession)
	{
		m_mysqlsession->SetIndex(nIndex);
	}
}
