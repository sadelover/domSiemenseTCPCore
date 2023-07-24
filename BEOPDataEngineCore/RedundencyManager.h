#pragma once

#include "DataBaseSetting.h"


class  CRedundencyManager
{
public:
	CRedundencyManager(void);
	~CRedundencyManager(void);



	static UINT WINAPI CheckIfNeedActiveSlaveThread(LPVOID lpVoid);
	bool	CheckIfNeedActiveSlave();
	bool	ActiveSlaveEvent();
	bool WaitForActiveCommand(int nActiveDelay);

	//看门狗定时器
	void	InitWatchDog(int nInterval);
	bool    Init(int nMainDropInterval, CString strIP, CString strIP2);


public:
	HANDLE m_hEventActiveStandby;
	HANDLE m_Backupthreadhandle;

	int m_nMainDropInterval;
	int m_nActiveDelay;
	CString m_strIP;
	CString m_strIP2;
};

