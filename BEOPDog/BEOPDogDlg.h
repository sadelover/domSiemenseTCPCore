
// BEOPDogDlg.h : header file
//

#pragma once
#include "ProcessView.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <hash_map>
#include "afxwin.h"
#include "TCPCustom.h"
#include "TCPServer.h"
#include "RedundencyCtrl.h"

#define WM_SHOWTASK (WM_USER+20)	//托盘消息

// CBEOPDogDlg dialog
class CBEOPDogDlg : public CDialog
{
// Construction
public:
	CBEOPDogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BEOPDog_DIALOG };

	static  unsigned int WINAPI MasterThread(LPVOID lpVoid);						//Master主启动线程
	static  unsigned int WINAPI MasterTaskHandleThread(LPVOID lpVoid);				//Master任务处理线程

	bool	MasterMain();															//Master主处理函数
	bool	MasterTaskHandle();
	bool	MasterTask(E_RESTART_TYPE eType,E_RESTART_REASON eReason);
	bool	MasterTaskResult(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
	bool	MasterTaskLog(E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult);
	bool	MasterSlaveCloseAll();													//Master从机关闭所有程序

	bool	MasterHandle();									//处理重启等任务
	bool	CheckExeState();								//检测喂狗是否超时
	bool	CheckOherExe();
	bool	FindExeByName(CString strExeName);
	bool	OpenExeByName(CString strExeName);
	bool	OpenExeByPath(CString strExePath);
	bool	CloseExe();
	bool	CloseOherExe();
	bool	CloseExeByName(CString strExeName);

	bool	ChangeIcon();

	//////////////////////////////////////////////////////////////////////////
	bool	RestartExe(E_RESTART_TYPE eTyp,E_RESTART_REASON eReason);				//重启Exe
	bool	CloseExe(E_RESTART_TYPE eTyp);					//关闭Exe
	//////////////////////////////////////////////////////////////////////////

	void	StoreTask(int nRestartType,int nReason);
	void	RegTask(E_RESTART_REASON eRegType, char* strProgName);

	bool	ReadConfig();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private: 
	//客户端连接建立事件处理函数 
	static void CALLBACK	OnClientConnect(void* pOwner,CTCPCustom *pTcpCustom); 
	//客户端SOCKET关闭事件处理函数 
	static void  CALLBACK OnClientClose(void* pOwner,CTCPCustom*pTcpCustom); 
	//服务器端收到来自客户端的数据 
	static  void CALLBACK OnClientRead(void* pOwner,CTCPCustom* pTcpCustom,const char * buf,DWORD dwBufLen ); 
	//客户端Socket错误事件处理函数 
	static  void CALLBACK OnClientError(void* pOwner,CTCPCustom* pTcpCustom,int nErrorCode); 
	//服务器端Socket错误事件处理函数 
	static void CALLBACK OnServerError(void* pOwner,CTCPServer* pTcpServer_CE,int nErrorCode); 

	static  void CALLBACK OnClientRestartTask(void* pOwner,CTCPCustom* pTcpCustom,int nRestartType,E_RESTART_REASON eReason); 

	static void CALLBACK OnClientRegedit(void* pOwner,CTCPCustom* pTcpCustom,E_RESTART_REASON eRegType, char* strProgName); 
// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA		m_nid;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnShowTask(WPARAM wParam,LPARAM lParam); 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCloseDog();
	DECLARE_MESSAGE_MAP()

public:
	bool	OutPutLogString(CString strOut);
	bool	InitTCPDataCernter();
	bool	CloseDog();
	bool	Exit();
private:
	CProcessView		m_PrsV;
	bool                m_bFirst;

public:
	CString m_strLog;
	CString	m_strPath;
	CString m_strIniFilePath;
	CString m_strLogFilePath;

	CString	m_strCorePath;
	CString m_strLogicPath;
	CString m_strWatchPath;

	CTCPServer		m_tcpServer; 						//TCPServer
	hash_map<E_RESTART_TYPE,E_RESTART_REASON>	m_mapTaskList;
	CRITICAL_SECTION        m_criTask; 

	bool		m_bExitThread;
	HANDLE		m_hMasterTaskHandle;

	COleDateTime	m_oleRegByCore;		//Core注册时间
	COleDateTime	m_oleRegByLogic;	//Logic注册时间
	COleDateTime	m_oleRegByUpdate;	//Update注册时间
	COleDateTime	m_oleRegByDTU;		//DTU注册时间
	COleDateTime	m_oleCheckTime;		//上次检测时间

	int			m_nCheckInterval;		//多久检测一次
	int			m_nTimeOut;				//多久没收到喂狗判断超时
	bool		m_bIconOnline;
	bool		m_bCloseDlg;

	bool		m_bEnableCore;			//启用Core喂狗
	bool		m_bEnableUpdate;		//启用Update喂狗
	bool		m_bEnableLogic;			//启用Logic喂狗
	bool		m_bEnableDTUEngine;		//启用DTUEngine喂狗
	wstring		m_wstrRedundencyIP;		//冗余IP
	wstring		m_strMasterVersion;		//版本号
	int			m_nMasterTCPPort;		//Master TCP端口
	int			m_nRedundencyPort;		//冗余端口
	int			m_nRedundencyTimeOut;	//冗余喂狗超时
	int			m_nMasterFeedLimitPerHour;//喂狗每小时重启限制
	wstring		m_wstrOtherCheckExeName;//其他待维护Exe
	wstring		m_wstrOtherCheckExePath;//其他待维护Exe

	hash_map<COleDateTime,E_RESTART_REASON>	m_mapCoreUnFeedRestart;		//core喂狗超时重启记录
	hash_map<COleDateTime,E_RESTART_REASON>	m_mapLogicUnFeedRestart;	//Logic喂狗超时重启记录
	hash_map<COleDateTime,E_RESTART_REASON>	m_mapUpdateUnFeedRestart;	//Update喂狗超时重启记录
	hash_map<COleDateTime,E_RESTART_REASON>	m_mapDTUEngineUnFeedRestart;//DTUEngine喂狗超时重启记录
	hash_map<string,COleDateTime>			m_mapThirdRegTime;			//第三方注册时间

	Project::Tools::Mutex m_lock;
	Project::Tools::Mutex m_dog_lock;
};
