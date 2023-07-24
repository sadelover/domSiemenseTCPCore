#pragma once

#include "VECTOR"
#include "DataBaseSetting.h"
#include "../Tools/CustomTools/CustomTools.h"


class CDLLObject;



class  CLogicPipeline
{
public:
	CLogicPipeline(void);
	~CLogicPipeline(void);

	bool Init();
	bool Exit();
	bool ActLogic();
	void PushLogicObject(CDLLObject *pObject);
	bool FindDllObject(CDLLObject *pObject);

	CDLLObject * GetLogicObject(int nIndex);
	int GetLogicDllCount();

	double GetTimeSpanSeconds();
	void SetTimeSpanSeconds(double fSeconds);
	//°´Ë³ÐòÅÅµÄdll

private:
	std::vector<CDLLObject*> m_vecImportDLLList;
	Project::Tools::Mutex	m_lock_timespan;
	double m_fTimeSpanSeconds;
};

