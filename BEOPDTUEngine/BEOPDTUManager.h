#pragma once
#include "Tools/dtu/BEOPDTUCtrl.h"
class CBEOPDTUManager
{
public:
	CBEOPDTUManager(CDataHandler* pDataHandler);
	~CBEOPDTUManager();

	bool	Init(int nTimeOut=0);					//初始化函数
	bool	Exit();									//退出函数
	bool	GetExitThread();						//获取退出标记
	bool	CheckTimeOut(int& nTimeOut);			//检测是否超时 0:不超时  1:SendTimeOut  10:ReceiveTimeOut 100:ConnectTimeOut

	static  unsigned int WINAPI ThreadConnectDTU(LPVOID lpVoid);	//DTU连接线程
	static  unsigned int WINAPI ThreadSendDTU(LPVOID lpVoid);		//DTU发送线程
	static  unsigned int WINAPI ThreadReceiveDTU(LPVOID lpVoid);	//DTU接收处理线程
	
	bool	ConnectDTU();							//连接DTU
	bool	SendDTU();								//发送函数
	bool	ReceiveDTU();							//接收函数
	
private:
	CDataHandler*	m_pDataHandler;					//数据库处理类
	CBEOPDTUCtrl*	m_pBEOPDTUCtrl;					//DTU处理类
	HANDLE			m_hDTUConnect;					//DTU连接线程句柄
	HANDLE			m_hDTUSend;						//DTU发送线程句柄
	HANDLE			m_hDTUReceive;					//DTU接收线程句柄
	bool			m_bExitThread;					//退出线程标记
	int				m_nTryCount;					//尝试次数
	COleDateTime	m_oleHeartTime;					//心跳和注册时间

	COleDateTime	m_oleSend;						//发送时间
	COleDateTime	m_oleReceive;					//接收时间
	COleDateTime	m_oleConnect;					//连接时间
};
