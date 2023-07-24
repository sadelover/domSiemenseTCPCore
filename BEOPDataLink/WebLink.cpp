#include "StdAfx.h"
#include "WebLink.h"
#include "COMdll/WebAPI/WebHttpCtrl.h"

#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CWebEngine::CWebEngine( CBEOPDataLinkManager* datalinker ,Beopdatalink::CCommonDBAccess*	dbsession_history)
	:m_datalinker(datalinker)
	,m_dbsession_history(dbsession_history)
{
	//¶ÁÅäÖÃ
	wstring wstrDurationInMinutes = L"0";
	wstring wstrUpdateInterval = L"60";
	wstring wstrActiveMinutes = L"30";
	wstring wstrSubscriptionType= L"ValueItemChanged";	
	wstring wstrChangesOnly = L"1";
	wstring wstrDeleteSubcription = L"1";
	wstring wstrSubcriptions = L"";

	if(m_dbsession_history)
	{
		wstrDurationInMinutes = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"webDurationInMinutes",L"0");
		wstrUpdateInterval = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"webUpdateInterval",L"60");
		wstrActiveMinutes = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"webActiveMinutes",L"30");
		wstrSubscriptionType = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"webSubscriptionType",L"ValueItemChanged");
		wstrChangesOnly = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"webChangesOnly",L"1");
		wstrDeleteSubcription = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"webDeleteSubcription",L"1");
		wstrSubcriptions = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"webSubcriptions",L"");
	}
	m_pWebHttpCtrl = new CWebHttpCtrl(m_dbsession_history,_wtoi(wstrDurationInMinutes.c_str()),_wtoi(wstrUpdateInterval.c_str()),_wtoi(wstrActiveMinutes.c_str()),Project::Tools::WideCharToUtf8(wstrSubscriptionType.c_str()),_wtoi(wstrChangesOnly.c_str()),_wtoi(wstrDeleteSubcription.c_str()),wstrSubcriptions);
}

CWebEngine::~CWebEngine( void )
{
	if (m_pWebHttpCtrl)
	{
		delete m_pWebHttpCtrl;
		m_pWebHttpCtrl = NULL;
	}
}

void CWebEngine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	if(m_pWebHttpCtrl)
		m_pWebHttpCtrl->SetPointList(pointlist);
}

void CWebEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	if(m_pWebHttpCtrl)
		m_pWebHttpCtrl->GetValueSet(valuelist);
}

bool CWebEngine::Init()
{
	return m_pWebHttpCtrl->Init();
}

bool CWebEngine::Exit()
{
	return m_pWebHttpCtrl->Exit();
}

void CWebEngine::AddLog( const wchar_t* loginfo )
{

}

void CWebEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CWebEngine::SetIndex( int nIndex )
{
	if(m_pWebHttpCtrl)
	{
		m_pWebHttpCtrl->SetIndex(nIndex);
	}
}
