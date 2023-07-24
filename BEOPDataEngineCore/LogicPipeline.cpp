#include "StdAfx.h"
#include "LogicPipeline.h"
#include "DLLObject.h"
#include "LogicBase.h"

CLogicPipeline::CLogicPipeline(void)
{
	m_fTimeSpanSeconds = 1e10;
}


CLogicPipeline::~CLogicPipeline(void)
{
}


bool CLogicPipeline::Init()
{
	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		if(m_vecImportDLLList[i]->GetLB() != NULL)
			m_vecImportDLLList[i]->GetLB()->Init();
	}
	return true;
}

bool CLogicPipeline::Exit()
{
	bool bSuccess = true;

	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		if(m_vecImportDLLList[i]->GetLB() != NULL)
			m_vecImportDLLList[i]->GetLB()->Exit();
	}

	return bSuccess;
}

bool CLogicPipeline::ActLogic()
{

	//Ö´ÐÐ²ßÂÔ
	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		m_vecImportDLLList[i]->RunOnce();

	}

	return true;
}


double CLogicPipeline::GetTimeSpanSeconds()
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock_timespan);
	return m_fTimeSpanSeconds;
}

void CLogicPipeline::SetTimeSpanSeconds(double fSeconds)
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock_timespan);
	m_fTimeSpanSeconds =  fSeconds;
}

bool CLogicPipeline::FindDllObject(CDLLObject *pObject)
{
	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		if(m_vecImportDLLList[i]->GetDllName()==pObject->GetDllName())
			return true;
	}

	return false;
}

void CLogicPipeline::PushLogicObject(CDLLObject *pObject)
{
	m_vecImportDLLList.push_back(pObject);

	if(m_fTimeSpanSeconds> pObject->GetTimeSpan())
		m_fTimeSpanSeconds = pObject->GetTimeSpan();
}

CDLLObject * CLogicPipeline::GetLogicObject(int nIndex)
{
	return m_vecImportDLLList[nIndex];
}

int CLogicPipeline::GetLogicDllCount()
{
	return m_vecImportDLLList.size();
}