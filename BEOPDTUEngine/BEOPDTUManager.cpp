#include "stdafx.h"
#include "BEOPDTUManager.h"

CBEOPDTUManager::CBEOPDTUManager(CDataHandler * pDataHandler)
	: m_pDataHandler(pDataHandler)
	, m_pBEOPDTUCtrl(NULL)
	, m_hDTUConnect(NULL)
	, m_hDTUSend(NULL)
	, m_hDTUReceive(NULL)
	, m_bExitThread(false)
	, m_nTryCount(3)
{
	m_oleHeartTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0, 1, 0, 0);
	m_oleSend = COleDateTime::GetCurrentTime();
	m_oleReceive = COleDateTime::GetCurrentTime();
	m_oleConnect = COleDateTime::GetCurrentTime();
}

CBEOPDTUManager::~CBEOPDTUManager()
{
	Exit();
}

/*
初始化函数：
1：读取数据库各项配置
2：创建发送/接收线程
*/
bool CBEOPDTUManager::Init(int nTimeOut)
{
	m_bExitThread = false;
	m_oleSend = COleDateTime::GetCurrentTime();
	m_oleReceive = COleDateTime::GetCurrentTime();
	m_oleConnect = COleDateTime::GetCurrentTime();

	bool bInitSuccess = false;
	if (m_pBEOPDTUCtrl == NULL)
	{
		m_pBEOPDTUCtrl = new CBEOPDTUCtrl(m_pDataHandler);
	}

	if (m_pBEOPDTUCtrl)
	{
		bInitSuccess = m_pBEOPDTUCtrl->Init();

		//创建线程
		if(m_hDTUConnect == NULL)
			m_hDTUConnect = (HANDLE)_beginthreadex(NULL, 0, ThreadConnectDTU, this, NORMAL_PRIORITY_CLASS, NULL);

		if (m_hDTUSend == NULL)
			m_hDTUSend = (HANDLE)_beginthreadex(NULL, 0, ThreadSendDTU, this, NORMAL_PRIORITY_CLASS, NULL);

		if (m_hDTUReceive == NULL)
			m_hDTUReceive = (HANDLE)_beginthreadex(NULL, 0, ThreadReceiveDTU, this, NORMAL_PRIORITY_CLASS, NULL);
	}
	return bInitSuccess;
}

bool CBEOPDTUManager::Exit()
{
	m_bExitThread = true;

	//关闭线程
	if (m_hDTUConnect)
	{
		WaitForSingleObject(m_hDTUConnect, 2000);
		CloseHandle(m_hDTUConnect);
		m_hDTUConnect = NULL;
	}

	if (m_hDTUSend)
	{
		WaitForSingleObject(m_hDTUSend, 2000);
		CloseHandle(m_hDTUSend);
		m_hDTUSend = NULL;
	}

	if (m_hDTUReceive)
	{
		WaitForSingleObject(m_hDTUReceive, 2000);
		CloseHandle(m_hDTUReceive);
		m_hDTUReceive = NULL;
	}

	//释放类
	if (m_pBEOPDTUCtrl)
	{
		m_pBEOPDTUCtrl->Exit();
		delete m_pBEOPDTUCtrl;
		m_pBEOPDTUCtrl = NULL;
	}
	return true;
}

bool CBEOPDTUManager::GetExitThread()
{
	return m_bExitThread;
}

unsigned int CBEOPDTUManager::ThreadConnectDTU(LPVOID lpVoid)
{
	CBEOPDTUManager* pDTUConnect = reinterpret_cast<CBEOPDTUManager*>(lpVoid);
	if (NULL == pDTUConnect)
	{
		return 0;
	}

	while (!pDTUConnect->GetExitThread())
	{
		pDTUConnect->ConnectDTU();

		int nWaitDTU = 60;			//等待60秒
		while (nWaitDTU >= 0 && !pDTUConnect->GetExitThread())
		{
			nWaitDTU--;
			Sleep(1000);
		}
	}
	return 1;
}

unsigned int CBEOPDTUManager::ThreadSendDTU(LPVOID lpVoid)
{
	CBEOPDTUManager* pDTUSend = reinterpret_cast<CBEOPDTUManager*>(lpVoid);
	if (NULL == pDTUSend)
	{
		return 0;
	}

	while (!pDTUSend->GetExitThread())
	{
		pDTUSend->SendDTU();
		Sleep(1000);
	}
	return 1;
}

unsigned int CBEOPDTUManager::ThreadReceiveDTU(LPVOID lpVoid)
{
	CBEOPDTUManager* pDTUReceive = reinterpret_cast<CBEOPDTUManager*>(lpVoid);
	if (NULL == pDTUReceive)
	{
		return 0;
	}

	while (!pDTUReceive->GetExitThread())
	{
		pDTUReceive->ReceiveDTU();
		Sleep(1000);
	}
	return 1;
}

bool CBEOPDTUManager::ConnectDTU()
{
	m_oleConnect = COleDateTime::GetCurrentTime();
	if (m_pBEOPDTUCtrl)
	{
		return m_pBEOPDTUCtrl->ReConnect();
	}
	return false;
}

bool CBEOPDTUManager::SendDTU()
{
	m_oleSend = COleDateTime::GetCurrentTime();
	if (m_pBEOPDTUCtrl && m_pBEOPDTUCtrl->IsConnected())
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleHeartTime;
		if (oleSpan.GetTotalSeconds() >= 60)
		{
			if (m_pBEOPDTUCtrl->IsConnected())
			{
				m_pBEOPDTUCtrl->SendRegData();
				m_oleHeartTime = COleDateTime::GetCurrentTime();
			}
		}

		//首先选取 filename = '' 且尝试次数小于3 根据时间和subtype综合排序, 成功删除，失败则尝试计数 + 1
		if (m_pBEOPDTUCtrl->IsConnected())
		{
			if (m_pDataHandler && m_pDataHandler->IsConnected())
			{
				DTU_DATA_INFO data;
				if (m_pDataHandler->GetLatestString(data, m_nTryCount))
				{
					m_pBEOPDTUCtrl->SendOneData(data);
					Sleep(1000);
				}
			}
		}

		//其次发送一个数据文件   选取 filename != '' subtupe = 0 根据时间排序, 成功删除，失败则尝试计数 + 1
		if (m_pBEOPDTUCtrl->IsConnected())
		{
			if (m_pDataHandler && m_pDataHandler->IsConnected())
			{
				DTU_DATA_INFO data;
				if (m_pDataHandler->GetOneLatestDataFile(data))
				{
					m_pBEOPDTUCtrl->SendOneData(data);
					Sleep(1000);
				}
			}
		}

		//再次发送一个其他类型文件数据 选取 filename != '' subtupe != 0 且尝试次数小于3的 根据时间排序, 成功删除，失败则尝试计数 + 1
		if (m_pBEOPDTUCtrl->IsConnected())
		{
			if (m_pDataHandler && m_pDataHandler->IsConnected())
			{
				DTU_DATA_INFO data;
				if (m_pDataHandler->GetOneLatestNormalFile(data,m_nTryCount))
				{
					m_pBEOPDTUCtrl->SendOneData(data);
					Sleep(1000);
				}
			}
		}
	}
	return true;
}

bool CBEOPDTUManager::ReceiveDTU()
{
	m_oleReceive = COleDateTime::GetCurrentTime();
	if (m_pBEOPDTUCtrl)
	{
		return m_pBEOPDTUCtrl->HandlerRecCmd();
	}
	return false;
}

bool CBEOPDTUManager::CheckTimeOut( int& nTimeOut )
{
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan oleSpanSend = oleNow - m_oleSend;
	COleDateTimeSpan oleSpanReceive = oleNow - m_oleReceive;
	COleDateTimeSpan oleSpanConnect = oleNow - m_oleConnect;

	nTimeOut = 0;											//0:不超时  1:SendTimeOut  10:ReceiveTimeOut 100:ConnectTimeOut
	if (oleSpanSend.GetTotalMinutes() >= 10)				//发送线程超时
	{
		nTimeOut += 1;
	}

	if (oleSpanReceive.GetTotalMinutes() >= 10)				//接收线程超时
	{
		nTimeOut += 10;
	}

	if (oleSpanConnect.GetTotalMinutes() >= 10)				//连接线程超时
	{
		nTimeOut += 100;
	}

	if(nTimeOut>0)
	{
		return true;
	}
	return false;
}
