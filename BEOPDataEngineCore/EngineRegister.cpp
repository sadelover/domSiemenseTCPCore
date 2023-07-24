#include "StdAfx.h"
#include "EngineRegister.h"

#include "../LAN_WANComm/PingIcmp.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"


CEngineRegister::CEngineRegister(Beopdatalink::CCommonDBAccess* pdbcon_user)
	:m_dbcon_common(pdbcon_user)
	,m_threadhandle(NULL)
	,m_bexitthread(false)
{
}


CEngineRegister::~CEngineRegister(void)
{
	if (m_threadhandle){
		CloseHandle(m_threadhandle);
		m_threadhandle = NULL;
	}
}

bool CEngineRegister::RegisterDataEngine()
{
	Beopdatalink::CUserOnlineEntry entry;
	SYSTEMTIME st;
	GetLocalTime(&st);
	entry.SetTimestamp(st);
	entry.SetUserName(_T("admin"));
	entry.SetUserHost(_T("localhost"));
	entry.SetUserType(_T("dataengine"));

	bool ismainserver = false;
	int ienableredundancy = CDataEngineAppConfig::GetInstance()->GetMasterSlaveEnable();
	if (ienableredundancy != 0){
		ismainserver = CNetworkComm::GetInstance()->IsMainServerRunningMode();
	}else
		ismainserver = true;
	 
	entry.SetUserPriority(ismainserver);

	if (m_dbcon_common){
		return m_dbcon_common->RegisterUserRecord(entry);
	}
	
	return false;
}


bool CEngineRegister::IsEngineOnline()
{
	if (m_dbcon_common->IsUserOnline(_T("admin"), _T("dataengine")))
		return true;
	return false;
}

bool CEngineRegister::Init()
{
	m_threadhandle = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	return m_bexitthread != NULL;
}

UINT WINAPI CEngineRegister::ThreadFunc( LPVOID lpvoid )
{
	CEngineRegister* pengineregister = (CEngineRegister*)lpvoid;
	if (pengineregister)
	{
		while(!pengineregister->GetThreadExit())
		{
			Sleep(20*1000);
			pengineregister->RegisterDataEngine();
		//	pengineregister->UpdatePLCConnectionStatus();
		}
	}

	return 0;
}

bool CEngineRegister::GetThreadExit()
{
	return m_bexitthread;
}

void CEngineRegister::SetThreadExit()
{
	m_bexitthread = true;
	WaitForSingleObject(m_threadhandle, INFINITE);
}

void CEngineRegister::UpdatePLCConnectionStatus()
{
	
	CPingIcmp pingcmp;
	bool bresult = pingcmp.Ping(m_strOpcIP);
	if (m_dbcon_common){
		m_dbcon_common->SetPLCConnectionStatus(bresult);
	}
}

