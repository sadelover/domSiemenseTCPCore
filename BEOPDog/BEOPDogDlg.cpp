
// BEOPDogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BEOPDog.h"
#include "BEOPDogDlg.h"
#include "afxdialogex.h"
#include "DumpFile.h"
#include "../Tools/CustomTools/CustomTools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const CString g_NAME_Core = _T("domcore.exe");
const CString g_NAME_Logic = _T("BEOPLogicEngine.exe");
const CString g_NAME_Watch = _T("BEOPWatch.exe");
const CString g_NAME_DTU = _T("BEOPDTUEngine.exe");
const int	  g_Check_Interval = 30;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBEOPDogDlg dialog




CBEOPDogDlg::CBEOPDogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBEOPDogDlg::IDD, pParent)
	, m_bFirst(TRUE)
	, m_strLog(_T(""))
	, m_bExitThread(false)
	, m_strPath(_T(""))
	, m_strIniFilePath(_T(""))
	, m_strLogFilePath(_T(""))
	, m_strCorePath(_T(""))
	, m_strLogicPath(_T(""))
	, m_strWatchPath(_T(""))
	, m_hMasterTaskHandle(NULL)
	, m_nCheckInterval(1)
	, m_nTimeOut(60)
	, m_bCloseDlg(false)
	, m_bEnableCore(true)
	, m_bEnableUpdate(false)
	, m_bEnableLogic(false)
	, m_bEnableDTUEngine(false)
	, m_wstrRedundencyIP(L"")
	, m_strMasterVersion(L"")
	, m_nMasterTCPPort(9502)
	, m_nRedundencyPort(8303)
	, m_nRedundencyTimeOut(60)
	, m_wstrOtherCheckExeName(L"")
	, m_wstrOtherCheckExePath(L"")
	, m_bIconOnline(true)
	, m_nMasterFeedLimitPerHour(4)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_oleRegByCore = COleDateTime::GetCurrentTime();
	m_oleRegByLogic = COleDateTime::GetCurrentTime();
	m_oleRegByUpdate = COleDateTime::GetCurrentTime();
	m_oleRegByDTU = COleDateTime::GetCurrentTime();
	m_oleCheckTime  = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0,1,0,0);
	m_mapCoreUnFeedRestart.clear();
	m_mapLogicUnFeedRestart.clear();
	m_mapUpdateUnFeedRestart.clear();
}

void CBEOPDogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBEOPDogDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_BUTTON_CLOSE,&CBEOPDogDlg::OnCloseDog)
END_MESSAGE_MAP()


// CBEOPDogDlg message handlers

BOOL CBEOPDogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//初始化Dump文件
	DeclareDumpFile();

	OutPutLogString(_T("BEOPDog V3.1.1 start!"));

	// TODO: Add extra initialization here
	//初始化系统托盘图标
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
	m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));//加载图标
	wcscpy(m_nid.szTip, _T("BEOPDog V3.1.1"));//信息提示
	Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标

	//创建处理线程
	HANDLE hMasterMain = (HANDLE)_beginthreadex(NULL, 0, MasterThread, this, 0, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBEOPDogDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if(nID==SC_MINIMIZE)
	{
		ShowWindow(SW_HIDE);//隐藏窗体
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBEOPDogDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);

	}
	else
	{
		if(m_bFirst)
		{
			ShowWindow(SW_HIDE);
			m_bFirst = false;
		}
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBEOPDogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CBEOPDogDlg::OutPutLogString( CString strLog )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);
	try
	{
		wstring wstrFileFolder;
		Project::Tools::GetSysPath(wstrFileFolder);
		SYSTEMTIME sNow;
		GetLocalTime(&sNow);
		wstrFileFolder += L"\\log";
		if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
		{
			CString strOut;
			wstring strTime;
			Project::Tools::SysTime_To_String(sNow,strTime);
			strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog);

			CString strLogPath;
			strLogPath.Format(_T("%s\\beopmaster_%d_%02d.log"),wstrFileFolder.c_str(),sNow.wYear,sNow.wMonth);

			//记录Log
			CStdioFile	ff;
			if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			{
				int nErrCode = GetLastError();
				int n = 1;
				ff.SeekToEnd();
				ff.WriteString(strOut);
				ff.Close();
				nErrCode = GetLastError();
				int n1 = 1;
				return true;
			}
		}
		return false;
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

LRESULT CBEOPDogDlg::OnShowTask( WPARAM wParam,LPARAM lParam )
{
	if(wParam!=IDR_MAINFRAME && wParam!=IDI_ICON1) 
		return 1; 

	switch(lParam) 
	{ 
	case WM_RBUTTONUP://右键起来时弹出快捷菜单，这里只有一个“关闭” 
		{ 
			LPPOINT lpoint=new tagPOINT; 
			::GetCursorPos(lpoint);//得到鼠标位置 
			CMenu menu;
			menu.CreatePopupMenu();//声明一个弹出式菜单 
			//增加菜单项“关闭”，点击则发送消息WM_DESTROY给主窗口（已//隐藏），将程序结束。 
			menu.AppendMenu(MF_STRING,ID_BUTTON_CLOSE,_T("Close(&C)"));
			//确定弹出式菜单的位置 
			menu.TrackPopupMenu(TPM_LEFTALIGN,lpoint->x,lpoint->y,this);
			//资源回收 
			HMENU hmenu=menu.Detach();
			menu.DestroyMenu();
			delete lpoint;
		} 
		break;
	case WM_LBUTTONDBLCLK://双击左键的处理 
		{
			//this->ShowWindow(SW_SHOW);//简单的显示主窗口完事儿
		}
		break; 
	default:
		break;
	} 
	return 0;
}

BOOL CBEOPDogDlg::OnEraseBkgnd( CDC* pDC )
{
	static bool bFirst=true;
	if(bFirst)
	{
		bFirst=false;
		ShowWindow(SW_HIDE);
		return TRUE;
	}

	return CDialog::OnEraseBkgnd(pDC);
}

void CBEOPDogDlg::OnClose()
{
	if(m_bCloseDlg)
	{
		m_bCloseDlg = false;
		CDialog::OnClose();
	}
	else
	{
		ShowWindow(SW_HIDE);//隐藏窗体
	}
}

void CBEOPDogDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	//删除托盘图标
	m_bExitThread = true;
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void CBEOPDogDlg::OnTimer( UINT_PTR nIDEvent )
{
	BOOL bRet	= FALSE;
	switch (nIDEvent)
	{
	case 1:
		{
			break;
		}
	default:
		break;
	}
	CDialog::OnTimer(nIDEvent);
}

void CBEOPDogDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if(nType == SIZE_MINIMIZED)
	{
		ShowWindow(SW_HIDE);
	}
}

BOOL CBEOPDogDlg::PreTranslateMessage( MSG* pMsg )
{
	DWORD msg = pMsg->message;
	if (msg == WM_KEYDOWN)
	{
		DWORD keytype = pMsg->wParam;
		if (keytype == VK_ESCAPE || keytype == VK_RETURN){
			return true;
		}
	}
	if (msg == WM_CLOSE)
	{
		OnClose();
	}
	return CDialog::PreTranslateMessage(pMsg);
}

bool CBEOPDogDlg::InitTCPDataCernter()
{
	//设置m_tcpServer属性 
	m_tcpServer.m_LocalPort = m_nMasterTCPPort;   
	m_tcpServer.m_pOwner = this;   
	m_tcpServer.OnClientConnect = OnClientConnect;   
	m_tcpServer.OnClientClose = OnClientClose;   
	m_tcpServer.OnClientRead = OnClientRead;   
	m_tcpServer.OnClientError = OnClientError;   
	m_tcpServer.OnServerError = OnServerError;   
	m_tcpServer.OnClientRegedit = OnClientRegedit;
	m_tcpServer.OnServerRestartTask = OnClientRestartTask;
	if (m_tcpServer.Open() <= 0)   
	{   
		CString strLog;
		strLog.Format(_T("TCPServer Start Fail(Port:%d)!"),m_nMasterTCPPort);
		OutPutLogString(strLog);
		return false;   
	}   
	return true;
}

//客户端连接建立事件处理函数   
void CALLBACK  CBEOPDogDlg::OnClientConnect(void* pOwner,CTCPCustom* pTcpCustom)   
{   
	
}   

//客户端SOCKET关闭事件处理函数   
void  CALLBACK CBEOPDogDlg::OnClientClose(void* pOwner,CTCPCustom* pTcpCustom)   
{   
	
}   

//服务器端收到来自客户端的数据   
void CALLBACK CBEOPDogDlg::OnClientRead(void* pOwner,CTCPCustom* pTcpCustom,const char * buf,DWORD dwBufLen )   
{   
	
}   

//客户端Socket错误事件处理函数   
void CALLBACK CBEOPDogDlg::OnClientError(void* pOwner,CTCPCustom* pTcpCustom,int nErrorCode)   
{   

}   

//服务器端Socket错误事件处理函数   
void CALLBACK CBEOPDogDlg::OnServerError(void* pOwner,CTCPServer* pTcpServer_CE,int nErrorCode)   
{   

}   

void CALLBACK CBEOPDogDlg::OnClientRestartTask( void* pOwner,CTCPCustom* pTcpCustom,int nRestartType,E_RESTART_REASON eReason )
{
	CBEOPDogDlg *pThis = (CBEOPDogDlg*)pOwner;   
	if(pThis)
	{
		pThis->StoreTask(nRestartType,eReason);
	}  
}

void CALLBACK CBEOPDogDlg::OnClientRegedit( void* pOwner,CTCPCustom* pTcpCustom,E_RESTART_REASON eRegType , char* strProgName)
{
	CBEOPDogDlg *pThis = (CBEOPDogDlg*)pOwner;   
	if(pThis)
	{
		pThis->RegTask(eRegType,strProgName);
	}  
}

unsigned int WINAPI CBEOPDogDlg::MasterThread( LPVOID lpVoid )
{
	CBEOPDogDlg* pMasterHandle = reinterpret_cast<CBEOPDogDlg*>(lpVoid);
	if (NULL == pMasterHandle)
	{
		return 0;
	}

	if(!pMasterHandle->m_bExitThread)
		pMasterHandle->MasterMain();

	return 1;
}

unsigned int WINAPI CBEOPDogDlg::MasterTaskHandleThread( LPVOID lpVoid )
{
	CBEOPDogDlg* pMasterHandle = reinterpret_cast<CBEOPDogDlg*>(lpVoid);
	if (NULL == pMasterHandle)
	{
		return 0;
	}

	while(!pMasterHandle->m_bExitThread)
	{		
		if(!pMasterHandle->m_bExitThread)
		{
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - pMasterHandle->m_oleCheckTime;
			if(oleSpan.GetTotalSeconds() >= pMasterHandle->m_nCheckInterval)
			{
				pMasterHandle->ChangeIcon();
				pMasterHandle->ReadConfig();
				pMasterHandle->MasterHandle();		//检测生成重启任务
				pMasterHandle->m_oleCheckTime = COleDateTime::GetCurrentTime();
			}
		}
	
		if(!pMasterHandle->m_bExitThread)
			pMasterHandle->MasterTaskHandle();	 //处理重启任务
		
		Sleep(1000);
	}
	return 1;
}

bool CBEOPDogDlg::MasterMain()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dog_lock);
	//初始化
	InitializeCriticalSection(&m_criTask);

	//读配置文件
	if(!m_bExitThread)
		ReadConfig();

	if(!m_bExitThread)
	{
		CRedundencyCtrl::GetInstance()->SetRedundencyInfo(m_wstrRedundencyIP,m_wstrRedundencyIP,m_nRedundencyPort,m_nRedundencyTimeOut);
		CRedundencyCtrl::GetInstance()->StartUdpComm();
	}

	//初始化InitTCPDataCernter
	if(!m_bExitThread)
		InitTCPDataCernter();

	//创建处理线程
	if(!m_bExitThread)
		m_hMasterTaskHandle = (HANDLE)_beginthreadex(NULL, 0, MasterTaskHandleThread, this, 0, NULL);

	return true;
}

bool CBEOPDogDlg::MasterTaskHandle()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dog_lock);
	if(CRedundencyCtrl::GetInstance()->IsMainServerRunningMode())			//主机模式
	{
		hash_map<E_RESTART_TYPE,E_RESTART_REASON>::iterator iterTask =	m_mapTaskList.begin();
		while(iterTask != m_mapTaskList.end())
		{
			bool bResult = MasterTask((*iterTask).first,(*iterTask).second);
			MasterTaskResult((*iterTask).first,(*iterTask).second,bResult);
			EnterCriticalSection(&m_criTask);
			iterTask = m_mapTaskList.erase(iterTask);
			LeaveCriticalSection(&m_criTask);
		}
	}
	else																	//待机模式
	{
		EnterCriticalSection(&m_criTask);
		m_mapTaskList.clear();
		LeaveCriticalSection(&m_criTask);

		//关闭所有
		MasterSlaveCloseAll();
	}
	return true;
}

bool CBEOPDogDlg::MasterTask( E_RESTART_TYPE eType,E_RESTART_REASON eReason )
{
	if(eReason == E_REASON_UNFEED_CORE)		//core没喂狗
	{
		if(m_nMasterFeedLimitPerHour >0 && m_mapCoreUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
		{
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertBegin = m_mapCoreUnFeedRestart.begin();
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertEnd = m_mapCoreUnFeedRestart.begin();
			itertEnd--;
			COleDateTimeSpan oleSpan = (*itertEnd).first - (*itertBegin).first;
			if(oleSpan.GetTotalMinutes() <= 60)
			{
				OutPutLogString(_T("Core UnFeed Restart Too Frequent."));
				return false;
			}
		}
	}
	else if(eReason == E_REASON_UNFEED_LOGIC)		//Logic没喂狗
	{
		if(m_nMasterFeedLimitPerHour >0 && m_mapLogicUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
		{
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertBegin = m_mapLogicUnFeedRestart.begin();
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertEnd = m_mapLogicUnFeedRestart.end();
			itertEnd--;
			COleDateTimeSpan oleSpan = (*itertEnd).first - (*itertBegin).first;
			if(oleSpan.GetTotalMinutes() <= 60)
			{
				OutPutLogString(_T("Logic UnFeed Restart Too Frequent."));
				return false;
			}
		}
	}
	else if(eReason == E_REASON_UNFEED_UPDATE)		//update没喂狗
	{
		if(m_nMasterFeedLimitPerHour >0 && m_mapUpdateUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
		{
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertBegin = m_mapUpdateUnFeedRestart.begin();
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertEnd = m_mapUpdateUnFeedRestart.end();
			itertEnd--;
			COleDateTimeSpan oleSpan = (*itertEnd).first - (*itertBegin).first;
			if(oleSpan.GetTotalMinutes() <= 60)
			{
				OutPutLogString(_T("Update UnFeed Restart Too Frequent."));
				return false;
			}
		}
	}
	else if(eReason == E_REASON_UNFEED_DTUENGINE)		//update没喂狗
	{
		if(m_nMasterFeedLimitPerHour >0 && m_mapDTUEngineUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
		{
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertBegin = m_mapDTUEngineUnFeedRestart.begin();
			hash_map<COleDateTime,E_RESTART_REASON>::iterator itertEnd = m_mapDTUEngineUnFeedRestart.end();
			itertEnd--;
			COleDateTimeSpan oleSpan = (*itertEnd).first - (*itertBegin).first;
			if(oleSpan.GetTotalMinutes() <= 60)
			{
				OutPutLogString(_T("DTUEngine UnFeed Restart Too Frequent."));
				return false;
			}
		}
	}

	switch(eType)
	{
	case E_RESTART_CORE:
	case E_RESTART_LOGIC:
	case E_RESTART_UPDATE:
	case E_RESTART_DTUENGINE:
		{
			return RestartExe(eType,eReason);
		}
	case E_CLOSE_CORE:
	case E_CLOSE_LOGIC:
	case E_CLOSE_UPDATE:
	case E_CLOSE_DTUENGINE:
		{
			return CloseExe(eType);
		} 
	}
	return false;
}

bool CBEOPDogDlg::MasterTaskResult( E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult )
{
	//生成操作记录
	MasterTaskLog(eType,eReason,bResult);

	//生成操作反馈
	switch(eReason)
	{
	case E_REASON_SUBMIT_CORE:
	case E_REASON_SUBMIT_LOGIC:
	case E_REASON_SUBMIT_UPDATE:
	case E_REASON_UNEXIST_MASTER:
	case E_REASON_SUBMIT_DTUENGINE:
		{
			return m_tcpServer.AckRestart(eType,eReason,bResult);
		}
	}
	return false;
}

bool CBEOPDogDlg::MasterSlaveCloseAll()
{
	//关闭自身绑定的exe
	CloseExe();

	//关闭其他绑定的exe
	CloseOherExe();

	return true;
}

bool CBEOPDogDlg::MasterHandle()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dog_lock);

	if(CRedundencyCtrl::GetInstance()->IsMainServerRunningMode())			//主机模式
	{
		CheckExeState();

		CheckOherExe();
	}
	return false;
}

bool CBEOPDogDlg::CheckExeState()
{
	if(CRedundencyCtrl::GetInstance()->IsMainServerRunningMode())			//主机模式
	{
		//检测Core
		if(m_bEnableCore)
		{
			if(FindExeByName(g_NAME_Core))		//Core存在的话 判断喂狗是否超时
			{
				COleDateTimeSpan opeSpan = COleDateTime::GetCurrentTime() - m_oleRegByCore;
				if(abs(opeSpan.GetTotalSeconds()) >= m_nTimeOut)
				{
					EnterCriticalSection(&m_criTask);
					m_mapTaskList[E_RESTART_CORE] = E_REASON_UNFEED_CORE;
					LeaveCriticalSection(&m_criTask);
				}
			}
			else								//不存在立即启动
			{
				EnterCriticalSection(&m_criTask);
				m_mapTaskList[E_RESTART_CORE] = E_REASON_UNEXIST_MASTER;
				LeaveCriticalSection(&m_criTask);
			}
		}

		//检测Logic
		if(m_bEnableLogic)
		{
			if(FindExeByName(g_NAME_Logic))		//Logic存在的话 判断喂狗是否超时
			{
				COleDateTimeSpan opeSpan = COleDateTime::GetCurrentTime() - m_oleRegByLogic;
				if(abs(opeSpan.GetTotalSeconds()) >= m_nTimeOut)
				{
					EnterCriticalSection(&m_criTask);
					m_mapTaskList[E_RESTART_LOGIC] = E_REASON_UNFEED_LOGIC;
					LeaveCriticalSection(&m_criTask);
				}
			}
			else								//不存在立即启动
			{
				EnterCriticalSection(&m_criTask);
				m_mapTaskList[E_RESTART_LOGIC] = E_REASON_UNEXIST_MASTER;
				LeaveCriticalSection(&m_criTask);
			}
		}

		//检测Update
		if(m_bEnableUpdate)
		{
			if(FindExeByName(g_NAME_Watch))		//Update存在的话 判断喂狗是否超时
			{
				COleDateTimeSpan opeSpan = COleDateTime::GetCurrentTime() - m_oleRegByUpdate;
				if(abs(opeSpan.GetTotalSeconds()) >= m_nTimeOut)
				{
					EnterCriticalSection(&m_criTask);
					m_mapTaskList[E_RESTART_UPDATE] = E_REASON_UNFEED_UPDATE;
					LeaveCriticalSection(&m_criTask);
				}
			}
			else								//不存在立即启动
			{
				EnterCriticalSection(&m_criTask);
				m_mapTaskList[E_RESTART_UPDATE] = E_REASON_UNEXIST_MASTER;
				LeaveCriticalSection(&m_criTask);
			}
		}

		//检测DTUEngine
		if(m_bEnableDTUEngine)
		{
			if(FindExeByName(g_NAME_DTU))		//DTUEngine存在的话 判断喂狗是否超时
			{
				COleDateTimeSpan opeSpan = COleDateTime::GetCurrentTime() - m_oleRegByDTU;
				if(abs(opeSpan.GetTotalSeconds()) >= m_nTimeOut)
				{
					EnterCriticalSection(&m_criTask);
					m_mapTaskList[E_RESTART_DTUENGINE] = E_REASON_UNFEED_DTUENGINE;
					LeaveCriticalSection(&m_criTask);
				}
			}
			else								//不存在立即启动
			{
				EnterCriticalSection(&m_criTask);
				m_mapTaskList[E_RESTART_DTUENGINE] = E_REASON_UNEXIST_MASTER;
				LeaveCriticalSection(&m_criTask);
			}
		}
		
	}
	return false;
}

bool CBEOPDogDlg::CheckOherExe()
{
	if(CRedundencyCtrl::GetInstance()->IsMainServerRunningMode())			//主机模式
	{
		if(m_wstrOtherCheckExeName.length() > 0)
		{
			vector<wstring> vecExeName;
			Project::Tools::SplitStringByChar(m_wstrOtherCheckExeName.c_str(),L';',vecExeName);
			for(int i=0; i<vecExeName.size(); ++i)
			{
				if(vecExeName[i].length() >0 && !FindExeByName(vecExeName[i].c_str()))		//不存在的话
				{
					bool bResult = OpenExeByName(vecExeName[i].c_str());

					CString strLog,strResult;
					strResult = (bResult==true)?_T("Success"):_T("Fail");
					strLog.Format(_T("Open %s %s."),vecExeName[i].c_str(),strResult);
					OutPutLogString(strLog);
				}
			}
		}

		if(m_wstrOtherCheckExePath.length() > 0)
		{
			vector<wstring> vecExePath;
			Project::Tools::SplitStringByChar(m_wstrOtherCheckExePath.c_str(),L';',vecExePath);
			for(int i=0; i<vecExePath.size(); ++i)
			{
				CString strExePath = vecExePath[i].c_str();
				CString strExeName;
				strExeName = strExePath.Right(strExePath.GetLength()-strExePath.ReverseFind('\\')-1);
				if(strExeName.GetLength() >0 && !FindExeByName(strExeName))		//不存在的话
				{
					bool bResult = OpenExeByPath(strExePath);
					CString strLog,strResult;
					strResult = (bResult==true)?_T("Success"):_T("Fail");
					strLog.Format(_T("Open %s %s."),strExeName,strResult);
					OutPutLogString(strLog);
				}
			}
		}

		//解析第三方注册包
		hash_map<string,COleDateTime>::iterator iterThird =	m_mapThirdRegTime.begin();
		while(iterThird != m_mapThirdRegTime.end())
		{
			COleDateTimeSpan opeSpan = COleDateTime::GetCurrentTime() - (*iterThird).second;
			if(abs(opeSpan.GetTotalSeconds()) >= m_nTimeOut)		//超时重启
			{	
				string strsqlThirdRegName = (*iterThird).first + ".exe";
				CString strThirdName = Project::Tools::AnsiToWideChar(strsqlThirdRegName.c_str()).c_str();
				if(FindExeByName(strThirdName))
				{
					CloseExeByName(strThirdName);
				}

				//启动时候在路径中找到 则从路径重启
				CString strOtherCheckExePath = m_wstrOtherCheckExePath.c_str();
				if(strOtherCheckExePath.Find(strThirdName) > 0)
				{
					vector<wstring> vecExePath;
					Project::Tools::SplitStringByChar(m_wstrOtherCheckExePath.c_str(),L';',vecExePath);
					for(int i=0; i<vecExePath.size(); ++i)
					{
						CString strExePath = vecExePath[i].c_str();
						CString strExeName;
						strExeName = strExePath.Right(strExePath.GetLength()-strExePath.ReverseFind('\\')-1);
						if(strExeName == strThirdName)		//不存在的话
						{
							bool bResult = OpenExeByPath(strExePath);
							CString strLog,strResult;
							strResult = (bResult==true)?_T("Success"):_T("Fail");
							strLog.Format(_T("Open %s %s."),strExeName,strResult);
							OutPutLogString(strLog);
						}
					}
				}
				else
				{
					bool bResult = OpenExeByName(strThirdName);
					CString strLog,strResult;
					strResult = (bResult==true)?_T("Success"):_T("Fail");
					strLog.Format(_T("Open %s %s."),strThirdName,strResult);
					OutPutLogString(strLog);
				}
			}
			iterThird++;
		}

	}
	return false;
}

bool CBEOPDogDlg::FindExeByName( CString strExeName )
{
	return m_PrsV.FindProcessByName_other(strExeName);
}

bool CBEOPDogDlg::OpenExeByName( CString strExeName )
{
	CString strExePath;
	strExePath.Format(_T("%s\\%s"),m_strPath,strExeName);
	if(Project::Tools::FindFileExist(strExePath.GetString()))
		return m_PrsV.OpenApp(strExePath);
	return false;
}

bool CBEOPDogDlg::OpenExeByPath( CString strExePath )
{
	return m_PrsV.OpenApp(strExePath);
}

bool CBEOPDogDlg::CloseOherExe()
{
	if(!CRedundencyCtrl::GetInstance()->IsMainServerRunningMode())			//待机模式
	{
		if(m_wstrOtherCheckExeName.length() > 0)
		{
			vector<wstring> vecExeName;
			Project::Tools::SplitStringByChar(m_wstrOtherCheckExeName.c_str(),L';',vecExeName);
			for(int i=0; i<vecExeName.size(); ++i)
			{
				if(vecExeName[i].length() >0 && FindExeByName(vecExeName[i].c_str()))	
				{
					CloseExeByName(vecExeName[i].c_str());
				}
			}
		}

		if(m_wstrOtherCheckExePath.length() > 0)
		{
			vector<wstring> vecExePath;
			Project::Tools::SplitStringByChar(m_wstrOtherCheckExePath.c_str(),L';',vecExePath);
			for(int i=0; i<vecExePath.size(); ++i)
			{
				CString strExePath = vecExePath[i].c_str();
				CString strExeName;
				strExeName = strExePath.Right(strExePath.GetLength()-strExePath.ReverseFind('\\')-1);
				if(strExeName.GetLength() >0 && FindExeByName(strExeName))		//Core存在的话 判断喂狗是否超时
				{
					CloseExeByName(strExeName);
				}
			}
		}
		return true;
	}
	return false;
}

bool CBEOPDogDlg::CloseExeByName( CString strExeName )
{
	return m_PrsV.CloseApp(strExeName);
}

bool CBEOPDogDlg::ChangeIcon()
{
	bool bIconOnline = CRedundencyCtrl::GetInstance()->IsMainServerRunningMode();
	if(bIconOnline != m_bIconOnline)
	{
		m_bIconOnline = bIconOnline;
		Shell_NotifyIcon(NIM_DELETE, &m_nid);//在托盘区添加图标
		if(bIconOnline)
		{
			m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
			SetIcon(m_hIcon, TRUE);			// Set big icon
			SetIcon(m_hIcon, FALSE);		// Set small icon
			m_nid.uID = IDR_MAINFRAME;
			m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));//加载图标
		}
		else
		{
			m_hIcon  = AfxGetApp()->LoadIcon(IDI_ICON1);
			SetIcon(m_hIcon, TRUE);			// Set big icon
			SetIcon(m_hIcon, FALSE);		// Set small icon
			m_nid.uID = IDI_ICON1;
			m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON1));//加载图标
		}
		Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标
	}
	return true;
}

bool CBEOPDogDlg::RestartExe( E_RESTART_TYPE eTyp, E_RESTART_REASON eReason )
{
	switch(eTyp)
	{
	case E_RESTART_CORE:
		{
			m_oleRegByCore = COleDateTime::GetCurrentTime();
			if(eReason == E_REASON_UNFEED_CORE)
			{
				if(m_nMasterFeedLimitPerHour >0 && m_mapCoreUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
					m_mapCoreUnFeedRestart.erase(m_mapCoreUnFeedRestart.begin());
				m_mapCoreUnFeedRestart[m_oleRegByCore] = E_REASON_UNFEED_CORE;
			}
			if(FindExeByName(g_NAME_Core))
			{
				CloseExeByName(g_NAME_Core);
			}
			return OpenExeByName(g_NAME_Core);
		}
	case E_RESTART_LOGIC:
		{
			m_oleRegByLogic = COleDateTime::GetCurrentTime();
			if(eReason == E_REASON_UNFEED_LOGIC)
			{
				if(m_nMasterFeedLimitPerHour >0 && m_mapLogicUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
					m_mapLogicUnFeedRestart.erase(m_mapLogicUnFeedRestart.begin());
				m_mapLogicUnFeedRestart[m_oleRegByLogic] = E_REASON_UNFEED_LOGIC;
			}
			if(FindExeByName(g_NAME_Logic))
			{
				CloseExeByName(g_NAME_Logic);
			}
			return OpenExeByName(g_NAME_Logic);
		}
	case E_RESTART_UPDATE:
		{
			m_oleRegByUpdate = COleDateTime::GetCurrentTime();
			if(eReason == E_REASON_UNFEED_UPDATE)
			{
				if(m_nMasterFeedLimitPerHour >0 && m_mapUpdateUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
					m_mapUpdateUnFeedRestart.erase(m_mapUpdateUnFeedRestart.begin());
				m_mapUpdateUnFeedRestart[m_oleRegByUpdate] = E_REASON_UNFEED_UPDATE;
			}
			if(FindExeByName(g_NAME_Watch))
			{
				CloseExeByName(g_NAME_Watch);
			}
			return OpenExeByName(g_NAME_Watch);
		}
	case E_RESTART_DTUENGINE:
		{
			m_oleRegByDTU = COleDateTime::GetCurrentTime();
			if(eReason == E_REASON_UNFEED_DTUENGINE)
			{
				if(m_nMasterFeedLimitPerHour >0 && m_mapDTUEngineUnFeedRestart.size() >= m_nMasterFeedLimitPerHour)
					m_mapDTUEngineUnFeedRestart.erase(m_mapDTUEngineUnFeedRestart.begin());
				m_mapDTUEngineUnFeedRestart[m_oleRegByDTU] = E_REASON_UNFEED_DTUENGINE;
			}
			if(FindExeByName(g_NAME_DTU))
			{
				CloseExeByName(g_NAME_DTU);
			}
			return OpenExeByName(g_NAME_DTU);
		}
	default:
		return false;
	}
	return false;
}

bool CBEOPDogDlg::CloseExe( E_RESTART_TYPE eTyp )
{
	switch(eTyp)
	{
	case E_CLOSE_CORE:
		{
			return CloseExeByName(g_NAME_Core);
		}
	case E_CLOSE_LOGIC:
		{
			return CloseExeByName(g_NAME_Logic);
		}
	case E_CLOSE_UPDATE:
		{
			return CloseExeByName(g_NAME_Watch);
		}
	case E_CLOSE_DTUENGINE:
		{
			return CloseExeByName(g_NAME_DTU);
		}
	default:
		return false;
	}
	return false;
}

bool CBEOPDogDlg::CloseExe()
{
	if(!CRedundencyCtrl::GetInstance()->IsMainServerRunningMode())			//待机模式
	{
		if(FindExeByName(g_NAME_Core))		//存在Core
		{
			CloseExeByName(g_NAME_Core);
		}
		
		if(FindExeByName(g_NAME_Logic))		//存在Logic
		{
			CloseExeByName(g_NAME_Logic);
		}
		
		if(FindExeByName(g_NAME_Watch))		//存在Watch
		{
			CloseExeByName(g_NAME_Watch);
		}

		if(FindExeByName(g_NAME_DTU))		//存在DTUEngine
		{
			CloseExeByName(g_NAME_DTU);
		}
		return true;
	}
	return false;
}

void CBEOPDogDlg::StoreTask( int nRestartType,int nReason )
{
	EnterCriticalSection(&m_criTask);
	m_mapTaskList[(E_RESTART_TYPE)nRestartType] = (E_RESTART_REASON)nReason;
	LeaveCriticalSection(&m_criTask);
}

void CBEOPDogDlg::RegTask( E_RESTART_REASON eRegType , char* strProgName)
{
	switch(eRegType)
	{
	case E_REASON_SUBMIT_CORE:
		{
			m_oleRegByCore = COleDateTime::GetCurrentTime();
			break;
		}
	case E_REASON_SUBMIT_LOGIC:
		{
			m_oleRegByLogic = COleDateTime::GetCurrentTime();
			break;
		}
	case E_REASON_SUBMIT_UPDATE:
		{
			m_oleRegByUpdate = COleDateTime::GetCurrentTime();
			break;
		}
	case E_REASON_SUBMIT_DTUENGINE:
		{
			m_oleRegByDTU = COleDateTime::GetCurrentTime();
			break;
		}
	default:
		{
			string strThirdName = strProgName;
			if(strThirdName.length() > 0)
				m_mapThirdRegTime[strThirdName] = COleDateTime::GetCurrentTime();
		}
		break;
	}
}

bool CBEOPDogDlg::ReadConfig()
{
	//读取服务器数据库
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\domcore.ini";

	m_strPath = cstrFile.c_str();
	m_strIniFilePath = exepath.c_str();
	m_strLogFilePath = m_strPath + _T("\\domdog.log");

	wchar_t charLocalPort[1024];

	//TCP 端口
	GetPrivateProfileString(L"master", L"MasterTCPPort", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrLocalPort = charLocalPort;
	if(wstrLocalPort == L"" || wstrLocalPort == L"0")
		wstrLocalPort = L"9502";
	WritePrivateProfileString(L"master", L"MasterTCPPort",wstrLocalPort.c_str(),exepath.c_str());
	m_nMasterTCPPort = _wtoi(wstrLocalPort.c_str());   

	//Master版本号,V3.0.1以上才满足分离功能
	GetPrivateProfileString(L"master", L"MasterVersion", L"", charLocalPort, 1024, exepath.c_str());
	m_strMasterVersion = charLocalPort;
	if(m_strMasterVersion == L"" || m_strMasterVersion == L"0")
		m_strMasterVersion = L"V3.0.1";
	WritePrivateProfileString(L"master", L"MasterVersion",m_strMasterVersion.c_str(),exepath.c_str());

	//冗余IP
	GetPrivateProfileString(L"master", L"RedundencyIP", L"", charLocalPort, 1024, exepath.c_str());
	m_wstrRedundencyIP = charLocalPort;
	WritePrivateProfileString(L"master", L"RedundencyIP",m_wstrRedundencyIP.c_str(),exepath.c_str());

	//冗余端口
	GetPrivateProfileString(L"master", L"RedundencyPort", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrRedundencyPort = charLocalPort;
	if(wstrRedundencyPort == L"" || wstrRedundencyPort == L"0")
		wstrRedundencyPort = L"8303";
	WritePrivateProfileString(L"master", L"RedundencyPort",wstrRedundencyPort.c_str(),exepath.c_str());
	m_nRedundencyPort = _wtoi(wstrRedundencyPort.c_str());   

	//冗余喂狗超时
	GetPrivateProfileString(L"master", L"RedundencyTimeOut", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrRedundencyTimeOut = charLocalPort;
	if(wstrRedundencyTimeOut == L"" || wstrRedundencyTimeOut == L"0")
		wstrRedundencyTimeOut = L"60";
	WritePrivateProfileString(L"master", L"RedundencyTimeOut",wstrRedundencyTimeOut.c_str(),exepath.c_str());
	m_nRedundencyTimeOut = _wtoi(wstrRedundencyTimeOut.c_str());   

	//检测间隔 1s
	GetPrivateProfileString(L"master", L"MasterCheckInterval", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrMasterCheckInterval = charLocalPort;
	if(wstrMasterCheckInterval == L"" || wstrMasterCheckInterval == L"0")
		wstrMasterCheckInterval = L"60";
	WritePrivateProfileString(L"master", L"MasterCheckInterval",wstrMasterCheckInterval.c_str(),exepath.c_str());
	m_nCheckInterval = _wtoi(wstrMasterCheckInterval.c_str());   

	//多久没收到喂狗判断超时
	GetPrivateProfileString(L"master", L"MasterFeedTimeOut", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrMasterFeedTimeOut = charLocalPort;
	if(wstrMasterFeedTimeOut == L"" || wstrMasterFeedTimeOut == L"0")
		wstrMasterFeedTimeOut = L"60";
	WritePrivateProfileString(L"master", L"MasterFeedTimeOut",wstrMasterFeedTimeOut.c_str(),exepath.c_str());
	m_nTimeOut = _wtoi(wstrMasterFeedTimeOut.c_str());   

	//喂狗每小时重启限制
	GetPrivateProfileString(L"master", L"MasterFeedLimitPerHour", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrMasterFeedLimitPerHour = charLocalPort;
	if(wstrMasterFeedLimitPerHour == L"" || wstrMasterFeedLimitPerHour == L"0")
		wstrMasterFeedLimitPerHour = L"4";
	WritePrivateProfileString(L"beopmaster", L"MasterFeedLimitPerHour",wstrMasterFeedLimitPerHour.c_str(),exepath.c_str());
	m_nMasterFeedLimitPerHour = _wtoi(wstrMasterFeedLimitPerHour.c_str());   

	//其他待维护Exe名称
	GetPrivateProfileString(L"master", L"OtherCheckExeName", L"", charLocalPort, 1024, exepath.c_str());
	m_wstrOtherCheckExeName = charLocalPort;
	WritePrivateProfileString(L"master", L"OtherCheckExeName",m_wstrOtherCheckExeName.c_str(),exepath.c_str());

	//其他待维护Exe路径
	GetPrivateProfileString(L"master", L"OtherCheckExePath", L"", charLocalPort, 1024, exepath.c_str());
	m_wstrOtherCheckExePath = charLocalPort;
	WritePrivateProfileString(L"master", L"OtherCheckExePath",m_wstrOtherCheckExePath.c_str(),exepath.c_str());

	//启用Core
	GetPrivateProfileString(L"master", L"EnableCheckCore", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrEnableCheckCore = charLocalPort;
	if(wstrEnableCheckCore == L"")
		wstrEnableCheckCore = L"1";
	WritePrivateProfileString(L"master", L"EnableCheckCore",wstrEnableCheckCore.c_str(),exepath.c_str());
	m_bEnableCore = _wtoi(wstrEnableCheckCore.c_str());   

	//启用Update
	GetPrivateProfileString(L"master", L"EnableCheckWatch", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrEnableCheckWatch = charLocalPort;
	if(wstrEnableCheckWatch == L"")
		wstrEnableCheckWatch = L"0";
	WritePrivateProfileString(L"master", L"EnableCheckWatch",wstrEnableCheckWatch.c_str(),exepath.c_str());
	m_bEnableUpdate = _wtoi(wstrEnableCheckWatch.c_str());   

	//启用Logic
	GetPrivateProfileString(L"master", L"EnableCheckLogic", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrEnableCheckLogic = charLocalPort;
	if(wstrEnableCheckLogic == L"")
		wstrEnableCheckLogic = L"0";
	WritePrivateProfileString(L"master", L"EnableCheckLogic",wstrEnableCheckLogic.c_str(),exepath.c_str());
	m_bEnableLogic = _wtoi(wstrEnableCheckLogic.c_str());   

	//启用DTUEngine
	GetPrivateProfileString(L"master", L"EnableCheckDTUEngine", L"", charLocalPort, 1024, exepath.c_str());
	wstring wstrEnableCheckDTUEngine = charLocalPort;
	if(wstrEnableCheckDTUEngine == L"")
		wstrEnableCheckDTUEngine = L"0";
	WritePrivateProfileString(L"master", L"EnableCheckDTUEngine",wstrEnableCheckDTUEngine.c_str(),exepath.c_str());
	m_bEnableDTUEngine = _wtoi(wstrEnableCheckDTUEngine.c_str());   

	return true;
}

bool CBEOPDogDlg::CloseDog()
{
	return Exit();
}

bool CBEOPDogDlg::Exit()
{
	m_bExitThread = true;
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dog_lock);
	m_tcpServer.Close();

	if(m_hMasterTaskHandle)
	{
		WaitForSingleObject(m_hMasterTaskHandle, 2000);
	}

	if(m_hMasterTaskHandle)
	{
		CloseHandle(m_hMasterTaskHandle);
		m_hMasterTaskHandle = NULL;
	}

	return true;
}

void CBEOPDogDlg::OnCloseDog()
{
	if(CloseDog())
	{
		m_bCloseDlg = true;
		SendMessage(WM_CLOSE);
	}
}

bool CBEOPDogDlg::MasterTaskLog( E_RESTART_TYPE eType,E_RESTART_REASON eReason,bool bResult )
{
	CString strOperation,strReason,strResult;
	strResult = _T("Success");
	if(!bResult)
		strResult = _T("Fail");

	switch(eType)
	{
	case E_RESTART_CORE:
		{
			strOperation = _T("Restart Core ");
			break;
		}
	case E_RESTART_LOGIC:
		{
			strOperation = _T("Restart Logic ");
			break;
		}
	case E_RESTART_UPDATE:
		{
			strOperation = _T("Restart Update ");
			break;
		}
	case E_RESTART_MASTER:
		{
			strOperation = _T("Restart Master ");
			break;
		}
	case E_RESTART_DTUENGINE:
		{
			strOperation = _T("Restart DTUEngine ");
			break;
		}
	case E_CLOSE_CORE:
		{
			strOperation = _T("Close Core ");
			break;
		}
	case E_CLOSE_LOGIC:
		{
			strOperation = _T("Close Logic ");
			break;
		}
	case E_CLOSE_UPDATE:
		{
			strOperation = _T("Close Update ");
			break;
		}
	case E_CLOSE_MASTER:
		{
			strOperation = _T("Close Master ");
			break;
		}
	case E_CLOSE_DTUENGINE:
		{
			strOperation = _T("Close DTUEngine ");
			break;
		}
	}

	switch(eReason)
	{
	case E_REASON_UNFEED_CORE:
		{
			strReason = _T("(Core Feed TimeOut)");
			break;
		}
	case E_REASON_UNFEED_LOGIC:
		{
			strReason = _T("(Logic Feed TimeOut)");
			break;
		}
	case E_REASON_UNFEED_UPDATE:
		{
			strReason = _T("(Update Feed TimeOut)");
			break;
		}
	case E_REASON_UNFEED_DTUENGINE:
		{
			strReason = _T("(DTUEngine Feed TimeOut)");
			break;
		}
	case E_REASON_SUBMIT_CORE:
		{
			strReason = _T("(Core Submit)");
			break;
		}
	case E_REASON_SUBMIT_LOGIC:
		{
			strReason = _T("(Logic Submit)");
			break;
		}
	case E_REASON_SUBMIT_UPDATE:
		{
			strReason = _T("(Update Submit)");
			break;
		}
	case E_REASON_UNEXIST_MASTER:
		{
			strReason = _T("(Master Check Unexist)");
			break;
		}
	case E_REASON_SUBMIT_DTUENGINE:
		{
			strReason = _T("(DTUEngine Submit)");
			break;
		}
	}

	CString strLog;
	strLog.Format(_T("%s%s%s"),strOperation,strResult,strReason);
	return OutPutLogString(strLog);
}
