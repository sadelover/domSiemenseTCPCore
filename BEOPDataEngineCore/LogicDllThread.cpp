#include "StdAfx.h"
#include "LogicDllThread.h"
#include "LogicPipeline.h"
#include "DLLObject.h"


CLogicDllThread::CLogicDllThread(wstring strThreadName)
	:m_strThreadName(strThreadName)
{
	m_hThread_dll			= NULL;
	m_hThreadId				= 0;
	//初始处于非激发状态，在变成激发状态后，不重置为非激发状态
	m_hEventClose   = CreateEvent(NULL,TRUE,FALSE,NULL);

	m_bRunning = false;

	m_bNeedInit = true;
}


CLogicDllThread::~CLogicDllThread(void)
{
	if (m_hEventClose != NULL)
	{
		CloseHandle(m_hEventClose);
		m_hEventClose =NULL;
	}

	if (m_hThread_dll != NULL)
	{
		CloseHandle(m_hThread_dll);
		m_hThread_dll =NULL;
	}
}


void CLogicDllThread::AddDll(CDLLObject *pDLLObject)
{
	m_vecImportDLLList.push_back(pDLLObject);
}

int CLogicDllThread::GetDllCount()
{
	return m_vecImportDLLList.size();
}

int CLogicDllThread::GetPipelineCount()
{
	return m_pLogicPipelineList.size();
}

CLogicPipeline * CLogicDllThread::GetPipeline(int nIndex)
{
	if(nIndex>= m_pLogicPipelineList.size())
		return NULL;

	if(nIndex<0)
		return NULL;

	return m_pLogicPipelineList[nIndex];
}


CDLLObject * CLogicDllThread::GetDllObject(int iIndex)
{
	if(iIndex<0 || iIndex>=m_vecImportDLLList.size())
		return NULL;

	return m_vecImportDLLList[iIndex];
}

wstring CLogicDllThread::GetName()
{
	return m_strThreadName;
}

wstring CLogicDllThread::GetStructureString()
{
	CString strAll;
	strAll.Format(L"Thread Name: %s\r\n", GetName().c_str());
	for(int mLine =0; mLine<m_pLogicPipelineList.size();mLine++)
	{
		CLogicPipeline *pLine =  m_pLogicPipelineList[mLine];
		CString strTemp = L"    |----";
		for(int j=0;j<pLine->GetLogicDllCount();j++)
		{
			CDLLObject *pObject = pLine->GetLogicObject(j);
			strTemp += pObject->GetDllName().c_str();
			if(j<pLine->GetLogicDllCount()-1)
				strTemp+=L"--->";
			else
				strTemp+=L"----|";
		}

		strAll +=strTemp;
		strAll +=L"\r\n";
	}
	wstring wstrAll = strAll.GetString();
	return wstrAll;
}



UINT WINAPI CLogicDllThread::ThreadFunc(LPVOID lparam)
{
	CLogicDllThread * pThisDll = (CLogicDllThread*)lparam;

	//循环调用线程主内容
	pThisDll->ThreadMerberFunc();


	return 0;
}

HANDLE CLogicDllThread::GetThreadHandle()
{
	return m_hThread_dll;
}


bool CLogicDllThread::SetThreadHandle(HANDLE hThread)
{
	m_hThread_dll = hThread;
	return true;
}

bool CLogicDllThread::GeneratePipelines()
{
	for(int mLine =0; mLine<m_pLogicPipelineList.size();mLine++)
	{
		if(m_pLogicPipelineList[mLine])
		{
			delete(m_pLogicPipelineList[mLine]);
			m_pLogicPipelineList[mLine] = NULL;
		}
	}
	m_pLogicPipelineList.clear();

	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		CLogicPipeline *pLine = new CLogicPipeline;
		pLine->PushLogicObject(m_vecImportDLLList[i]);
		m_pLogicPipelineList.push_back(pLine);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
bool CLogicDllThread::StartThread()
{
	m_hThread_dll = (HANDLE)_beginthreadex(NULL,
		0,
		ThreadFunc, 
		(LPVOID)this,
		CREATE_SUSPENDED,
		&m_hThreadId);

	if(m_hThread_dll == NULL)
	{
		int nErrCode = GetLastError();
		CString str;
		str.Format(_T("ERROR: _beginthreadex failed( Threadname:%s,errCode:%d)"),GetName().c_str(),nErrCode);
		Project::Tools::PrintLog(str.GetString());
		return false;
	}

	::ResumeThread(m_hThread_dll);
	return true;
}



UINT CLogicDllThread::ThreadMerberFunc()
{

	int nClockCount = 0;
	int nClockMS = 500;

	int i=0;
	vector<int> nClockCountList;
	for(i=0;i<m_pLogicPipelineList.size();i++)
	{
		nClockCountList.push_back(0);
	}
	
	ResetEvent(m_hEventClose);

	while (TRUE)
	{	
		Sleep(nClockMS);
		if (WaitForSingleObject(m_hEventClose,0) == WAIT_OBJECT_0)
			break;

		if(!m_bRunning)
			continue;

		if(m_bNeedInit)
		{//当暂停后继续回来，需要初始化
			for(i=0;i<m_pLogicPipelineList.size();i++)
				m_pLogicPipelineList[i]->Init();

			m_bNeedInit = false;
		}

		for(i=0;i<m_pLogicPipelineList.size();i++)
		{
			nClockCountList[i]++;
			double fspan = m_pLogicPipelineList[i]->GetTimeSpanSeconds();
			int nClockActiveCount = (int)(fspan*1000.0)/nClockMS;

			if (nClockCountList[i]<nClockActiveCount)
			{
				continue;
			}
			nClockCountList[i] = 0; //执行一次就清零
			m_pLogicPipelineList[i]->ActLogic();
		}
			
	}

	return 0;
}


bool CLogicDllThread::Exit()
{
	SetEvent(m_hEventClose);

	DWORD WaitFlag = WaitForSingleObject(m_hThread_dll,INFINITE);

	if (m_hThread_dll != NULL)
	{
		CloseHandle(m_hThread_dll);
		m_hThread_dll = NULL;
	}

	bool bSuccess = true;
	unsigned int i=0;
	for(i=0;i<m_pLogicPipelineList.size();i++)
	{
		if(!m_pLogicPipelineList[i]->Exit())
			bSuccess = false;
	}

	for(i=0;i<m_pLogicPipelineList.size();i++)
	{
		SAFE_DELETE(m_pLogicPipelineList[i]);
	}

	m_pLogicPipelineList.clear();

	return bSuccess;
}


bool CLogicDllThread::GetRunStatus()
{
	return m_bRunning;
}


bool CLogicDllThread::SetRunStatus(bool runstatus)
{
	if(runstatus && !m_bRunning)
	{
		m_bNeedInit = true;
	}

	m_bRunning = runstatus ;
	return true;
}