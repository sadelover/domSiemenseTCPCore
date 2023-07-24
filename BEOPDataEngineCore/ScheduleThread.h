#pragma once

#include "VECTOR"
#include "DataBaseSetting.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "../DB_BasicIO/RealTimeDataAccess.h"
#include <hash_map>
class CLogicBase;
using namespace std;

class  CScheduleThread
{
public:
	CScheduleThread(CBEOPDataAccess* pDataAccess);
	~CScheduleThread(void);

	bool	Init();
	bool	Exit();

	//////////////////////////////////////////////////////////////////////////
	bool	CreateTable();			//创建表schedule_list,schedule_info_weeky

	//////////////////////////////////////////////////////////////////////////

	static  UINT WINAPI ThreadFunc(LPVOID lparam);
	bool	HandleScheduleTask();
	bool	CheckScehduleResult();

public:
	CBEOPDataAccess *m_pDataAccess;
	bool			m_bCreateTable;
	bool			m_bExitThread;
	HANDLE			m_hSchedulehandle;
	hash_map<int,Beopdatalink::ScheduleInfo>	m_mapScheduleTask;		//待确认结果
};

