#pragma once

#include "VECTOR"
#include "DataBaseSetting.h"

class CDLLObject;
class CLogicPipeline;

using namespace std;

class  CLogicDllThread
{
public:
	CLogicDllThread(wstring strThreadName);
	~CLogicDllThread(void);


	//创建线程
	bool    GeneratePipelines();
	bool	StartThread();
	bool    Exit();
	static  UINT WINAPI ThreadFunc(LPVOID lparam);

	HANDLE		GetThreadHandle();
	bool		SetThreadHandle(HANDLE hThread);

	bool		GetRunStatus();
	bool		SetRunStatus(bool runstatus);

	void AddDll(CDLLObject *pDLLObject);
	int  GetDllCount();
	int GetPipelineCount();
	CLogicPipeline * GetPipeline(int nIndex);
	CDLLObject * GetDllObject(int iIndex);
	wstring GetName();
	wstring GetStructureString();

protected:
	virtual UINT ThreadMerberFunc();
	HANDLE						m_hThread_dll;
	UINT						m_hThreadId;
	HANDLE			m_hEventClose;


private:
	std::vector<CDLLObject*> m_vecImportDLLList;
	wstring m_strThreadName;
	bool m_bRunning;
	bool m_bNeedInit;

	std::vector<CLogicPipeline *> m_pLogicPipelineList;
};

