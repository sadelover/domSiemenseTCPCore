#include "StdAfx.h"
#include "RedundencyManager.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"



#define  E_NUM_UPDATE_INTERVAL 90  //最后一次更新在1分钟以内即使用还原
#define  E_STR_BACKUPFILENAME  _T("\\backup.bat")   //备份文件名
#define  E_STR_RESTOREFILENAME  _T("\\restore.bat")   //还原文件名
#define  E_STR_MYSQL_PATH	_T("\"C:\\Program Files\\MySQL\\MySQL Server 5.1\\bin\\\"")     
#define  E_STR_SQLPATH	_T("d:\RealTimeDataBase.sql")


CRedundencyManager::CRedundencyManager(void)
	: m_Backupthreadhandle(NULL)
	,m_nMainDropInterval(1000)
	,m_nActiveDelay(60)
{

	m_hEventActiveStandby = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CRedundencyManager::~CRedundencyManager(void)
{
	if(m_Backupthreadhandle != NULL)
	{
		CloseHandle(m_Backupthreadhandle);
		m_Backupthreadhandle = NULL;
	}

}



UINT WINAPI CRedundencyManager::CheckIfNeedActiveSlaveThread( LPVOID lpVoid )
{
	CRedundencyManager* pManager = reinterpret_cast<CRedundencyManager*>(lpVoid);
	if (NULL == pManager)
	{
		return 0;
	}

	//判断连续30s收到激活状态
	int nTotal = pManager->m_nActiveDelay;
	while(1)
	{
		// Analyze Data
		bool bActive = pManager->CheckIfNeedActiveSlave();
		if(bActive)
		{
			nTotal--;
			if(nTotal < 0)
			{
				pManager->ActiveSlaveEvent();
				nTotal = pManager->m_nActiveDelay;
				break;
			}
		}
		Sleep(1000);
	}

	return 1;
}


bool CRedundencyManager::WaitForActiveCommand(int nActiveDelay)
{
	//启动一个线程检测
	HANDLE threadhandle = (HANDLE)_beginthreadex(NULL, 0, CheckIfNeedActiveSlaveThread, this, 0, NULL);

	
	//	pDlg->SetWindowTextW(L"绑定PC已运行有主机模式的BEOPgateway，本机转为冗余热备状态，请勿关闭.");
	if(nActiveDelay <= 0)
		nActiveDelay = 60;

	m_nActiveDelay = nActiveDelay;
	DWORD waitstatus = WaitForSingleObject(m_hEventActiveStandby, INFINITE);
	if (waitstatus == WAIT_OBJECT_0)
	{
		CloseHandle(threadhandle);
		threadhandle = NULL;

		return true;
	}

	return false;
}


bool CRedundencyManager::CheckIfNeedActiveSlave()
{
	return CNetworkComm::GetInstance()->IsMainServerRunningMode();
}


bool CRedundencyManager::ActiveSlaveEvent()
{
	SetEvent(m_hEventActiveStandby);
	return true;
}

bool CRedundencyManager::Init(int nMainDropInterval, CString strIP, CString strIP2)
{

	m_strIP = strIP;
	m_strIP2 = strIP2;
	//设置参数
	CDataEngineAppConfig::GetInstance()->SetGateWayIP(strIP.GetString());
	CDataEngineAppConfig::GetInstance()->SetOtherServerIP(strIP2.GetString());


	//启动一个网络心跳包  StartUdpComm
	//CNetworkComm::GetInstance()->SetUpperMachineMode(DOUBLE_MODE);
	//CNetworkComm::GetInstance()->StartUdpComm();
	CNetworkComm::GetInstance()->SetMachineModes(true,true,false);
	return true;
}

void CRedundencyManager::InitWatchDog( int nInterval )
{
	//_asm
	//{
	//	//进入配置模式
	//	mov dx,0x4E
	//	mov al,055h
	//	out dx,al
	//	out dx,al

	//	//
	//	mov dx,0x4E
	//	mov al,7h
	//	out dx,al
	//	inc dx
	//	mov al,oah
	//	out dx,al

	//	//获取运行时寄存器基地址
	//	mov dx,0x4E
	//	mov al,60h
	//	out dx,al

	//	inc dx
	//	in al,dx
	//	mov bh,al

	//	mov dx,0x4E
	//	mov al,61h
	//	out dx,al

	//	inc dx
	//	in al,dx
	//	mov bl,al

	//	//配置GP60为看门狗定时器
	//}
}