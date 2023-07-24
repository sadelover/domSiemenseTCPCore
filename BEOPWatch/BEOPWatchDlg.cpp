
// BEOPWatchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BEOPWatch.h"
#include "BEOPWatchDlg.h"
#include "afxdialogex.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "./point/sqlite/SqliteAcess.h"
#include "DumpFile.h"
#include "ToolsFunction.h"
#include "./tcp/TelnetTcpCtrl.h"
#include "PingIcmp.h"
#include <hash_map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const CString g_NAME_Core = _T("domcore.exe");
const CString g_NAME_Dog = _T("domdog.exe");
const CString g_NAME_Logic = _T("domlogic.exe");
const CString g_NAME_Master = _T("domdog.exe");
const CString g_NAME_Update = _T("domwatch.exe");

const int	  g_Check_Interval = 2;
const int	  g_Restart_Count = 3;

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


// CBEOPWatchDlg dialog




CBEOPWatchDlg::CBEOPWatchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBEOPWatchDlg::IDD, pParent)
	, m_bFirst(TRUE)
	, m_strLog(_T(""))
	, m_nRestartType(-1)
	, m_pDataHandle(NULL)
	, m_pdtusender(NULL)
	, m_bdtuSuccess(false)
	, m_hsenddtuhandle(NULL)
	, m_hReceivedtuhandle(NULL)
	, m_hScanUpdateStatehandle(NULL)
	, m_hcheckhandle(NULL)
	, m_bexitthread_history(false)
	, m_strTCPName(L"")
	, m_strTCPPort(L"")
	, m_bVecCopying(false)
	, m_nErrCode(-99)
	, m_strSendRealFilePath("")
	, m_nSendRealFileCount(0)
	, m_strZipPath("")
	, m_strAck("")
	, m_nDTUState(1)
	, m_bChangeIcon(false)
	, m_bCloseDlg(false)
	, m_nErrorCode(0)
	, m_nOutPutMemoryInterval(30)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_vecReStartTime.clear();
	m_oleStartTime = COleDateTime::GetCurrentTime();
	m_oleLastSendReg = COleDateTime::GetCurrentTime();
	m_oleShowRouterState = COleDateTime::GetCurrentTime();
	m_oleLastDleteTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
	m_oleLastRestartTime = COleDateTime::GetCurrentTime()- COleDateTimeSpan(1,0,0,0);
	m_oleGetMemoryTime = COleDateTime::GetCurrentTime()- COleDateTimeSpan(0,0,28,0);
	m_senddtuevent = ::CreateEvent(NULL, false, false, L"");
	m_mapSendLostHistory.clear();
	GetLocalTime(&m_ackTime);
	GetSystemTimes( &m_preidleTime, &m_prekernelTime, &m_preuserTime);
}

void CBEOPWatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_eLog);
	DDX_Text(pDX, IDC_EDIT_LOG, m_strLog);
}

BEGIN_MESSAGE_MAP(CBEOPWatchDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_BUTTON_RESTART, &CBEOPWatchDlg::OnBnClickedButtonRestart)
	ON_BN_CLICKED(IDC_CHECK_TCP_CONNECTION, &CBEOPWatchDlg::OnBnClickedCheckTcpConnection)
	ON_BN_CLICKED(IDC_CLOSE_WATCH,&CBEOPWatchDlg::OnCloseWatch)
END_MESSAGE_MAP()


// CBEOPWatchDlg message handlers

BOOL CBEOPWatchDlg::OnInitDialog()
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

	OutPutLogString(_T("BEOPWatch V2.1.5 start!"));
	CDogTcpCtrl::GetInstance()->SetDogMode(E_SLAVE_UPDATE);
	FeedDog();

	wstring strPath;
	GetSysPath(strPath);
	m_strPath += strPath.c_str();
	m_strIniFilePath = m_strPath + _T("\\domcore.ini");
	m_strLogFilePath = m_strPath + _T("\\domwatch.log");
	m_strPath += _T("\\");
	m_strLogicPath = m_strPath + g_NAME_Logic;
	m_strPath += g_NAME_Core;

	//数据库配置信息
	wchar_t charContent[256];
	GetPrivateProfileString(L"core", L"defaultdb", L"", charContent, 256, m_strIniFilePath);
	wstring wstrDefaultDB = charContent;
	if(wstrDefaultDB == L"")
		wstrDefaultDB = L"beopdata";
	WritePrivateProfileString(L"core", L"defaultdb",wstrDefaultDB.c_str(),m_strIniFilePath);

	GetPrivateProfileString(L"core", L"defaultuser", L"", charContent, 256, m_strIniFilePath);
	wstring wstrDefaultUser = charContent;
	if(wstrDefaultUser == L"")
		wstrDefaultUser = L"root";
	WritePrivateProfileString(L"core", L"defaultuser",wstrDefaultUser.c_str(),m_strIniFilePath);

	GetPrivateProfileString(L"core", L"defaultpsw", L"", charContent, 256, m_strIniFilePath);
	wstring wstrDefaultPsw = charContent;
	if(wstrDefaultPsw == L"")
		wstrDefaultPsw = L"RNB.beop-2013";
	WritePrivateProfileString(L"core", L"defaultpsw",wstrDefaultPsw.c_str(),m_strIniFilePath);

	m_pDataHandle = new CDataHandler();
	if(m_pDataHandle)
	{
		if(!m_pDataHandle->Connect(L"localhost",wstrDefaultUser,wstrDefaultPsw,wstrDefaultDB,3306))
		{
			MessageBox(_T("Access to the database configuration information fails, the watchdog will exit!"));
			EndDialog(0);
			return FALSE;
		}
	}
	// TODO: Add extra initialization here
	//初始化系统托盘图标
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
	m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));//加载图标
	wcscpy(m_nid.szTip, _T("domwatch"));//信息提示
	Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标

	//SetTimer(1,g_Check_Interval*1000,NULL);
	//InitTCPSender();
	HANDLE hCreate = (HANDLE)_beginthreadex(NULL, 0, CreateTCPConnection, this, 0, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBEOPWatchDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBEOPWatchDlg::OnPaint()
{
	if (IsIconic() || m_bChangeIcon)
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
		m_bChangeIcon = false;
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
HCURSOR CBEOPWatchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBEOPWatchDlg::OnBnClickedButtonRestart()
{
	// TODO: Add your control notification handler code here
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"-1");
	}
	RestartCoreByDog(13,m_nErrorCode,true);
}

bool CBEOPWatchDlg::OutPutLogString( CString strOut )
{
	if(m_bexitthread_history)
		return false;
	int count = m_eLog.GetLineCount();
	if (count > 100){
		m_strLog = _T("");
	}

	COleDateTime tnow = COleDateTime::GetCurrentTime();
	CString strLog;
	strLog += tnow.Format(_T("%Y-%m-%d %H:%M:%S "));
	strLog += strOut;
	
	m_strLog += strLog;
	m_strLog += _T("\r\n");
	m_eLog.SetWindowTextW(m_strLog);
	int nLine=m_eLog.GetLineCount();
	m_eLog.LineScroll(nLine);
	strLog += _T("\n");

	//记录Log
	CStdioFile	ff;
	if(ff.Open(m_strLogFilePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
	{
		ff.Seek(0,CFile::end);
		ff.WriteString(strLog);
		ff.Close();
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::CloseCore()
{
	bool bResult = false;
	if (m_PrsV.FindProcessByName_other(g_NAME_Core))
	{
		if(m_PrsV.CloseApp(g_NAME_Core))
		{
			bResult = true;
		}
		else
		{
			bResult = false;
		}
	}
	return bResult;
}

bool CBEOPWatchDlg::CloseLogic()
{
	bool bResult = false;
	if (m_PrsV.FindProcessByName_other(g_NAME_Logic))
	{
		if(m_PrsV.CloseApp(g_NAME_Logic))
		{
			bResult = true;
		}
		else
		{
			bResult = false;
		}
	}
	return bResult;
}

LRESULT CBEOPWatchDlg::OnShowTask( WPARAM wParam,LPARAM lParam )
{
	if(wParam!=IDR_MAINFRAME && wParam!=IDR_ICON2) 
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
			menu.AppendMenu(MF_STRING,IDC_CLOSE_WATCH,_T("Close(&C)"));
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
			this->ShowWindow(SW_SHOW);//简单的显示主窗口完事儿
		}
		break; 
	default:
		break;
	} 
	return 0;
}

BOOL CBEOPWatchDlg::OnEraseBkgnd( CDC* pDC )
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

void CBEOPWatchDlg::OnClose()
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

void CBEOPWatchDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	//删除托盘图标
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void CBEOPWatchDlg::OnTimer( UINT_PTR nIDEvent )
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

void CBEOPWatchDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if(nType == SIZE_MINIMIZED)
	{
		ShowWindow(SW_HIDE);
	}
}

BOOL CBEOPWatchDlg::PreTranslateMessage( MSG* pMsg )
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

bool CBEOPWatchDlg::InitTCPSender()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_watch_lock);
	//写入Core的版本号
	m_pDataHandle->WriteCoreDebugItemValue(L"watchversion",WATCH_VERSION);

	wstring wstrOutputMemoryInterval = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"OutputMemoryInterval",L"30");
	if(wstrOutputMemoryInterval == L"" || wstrOutputMemoryInterval == L"0")
		wstrOutputMemoryInterval = L"30";
	m_nOutPutMemoryInterval = _wtoi(wstrOutputMemoryInterval.c_str());

	//
	if(!m_bexitthread_history)
	{
		if(m_hsenddtuhandle == NULL)
			m_hsenddtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUThread, this, 0, NULL);
		if(m_hReceivedtuhandle == NULL)
			m_hReceivedtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUReceiveHandleThread, this, 0, NULL);
		if(m_hScanUpdateStatehandle == NULL)
			m_hScanUpdateStatehandle = (HANDLE)_beginthreadex(NULL, 0, ScanUpdateStateHandleThread, this, 0, NULL);
		if(m_hcheckhandle == NULL)
			m_hcheckhandle = (HANDLE)_beginthreadex(NULL, 0, CheckExeThread, this, 0, NULL);
	}

	bool bWithTcpSender = (m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderEnabled"))==L"1";
	((CButton*)GetDlgItem(IDC_CHECK_TCP_CONNECTION))->SetCheck(bWithTcpSender);
	if(bWithTcpSender)
	{
		wstring wstrTcpSenderIP = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderIP",L"");		//读TCP IP   默认为114.215.172.232
		if(wstrTcpSenderIP == L"" || wstrTcpSenderIP == L"0")
			wstrTcpSenderIP = L"";

		wstring wstrTcpSenderport = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderPort");		//读TCP端口   默认为9501
		if(wstrTcpSenderport == L"" || wstrTcpSenderport == L"0")
			wstrTcpSenderport = L"9500";

		m_strTCPPort = wstrTcpSenderport;
		m_strTCPName = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderName");		//读TCP Name  用于标识项目
		if(m_strTCPName == L"" || m_strTCPName == L"0")
		{
			m_strTCPName = L"default2015";
			CToolsFunction toolsFunction;
			IP_ADAPTER_INFO* pIpAdapterInfo = toolsFunction.GetAllLocalMachineEthInfo();
			if(pIpAdapterInfo != NULL)
			{
				if (pIpAdapterInfo)
				{
					BYTE s[8]= {0};
					memcpy(s,pIpAdapterInfo->Address,6);  
					CString strMac;
					strMac.Format(_T("%02X%02X%02X%02X%02X%02X"),s[0],s[1],s[2],s[3],s[4],s[5]);
					m_strTCPName = strMac.GetString();
				}
			}
			m_pDataHandle->WriteCoreDebugItemValue(L"storehistory",L"0");
			m_pDataHandle->WriteCoreDebugItemValue(L"TcpSenderName",m_strTCPName);
		}

		((CEdit*)GetDlgItem(IDC_STATIC_NAME))->SetWindowText(m_strTCPName.c_str());

		if(m_pdtusender == NULL)
			m_pdtusender = new CDTUSender(Project::Tools::WideCharToAnsi(m_strTCPName.c_str()),Project::Tools::WideCharToAnsi(wstrTcpSenderIP.c_str()),_wtoi((wstrTcpSenderport.c_str()))+1,m_pDataHandle);
		if(m_pdtusender)
		{
			m_bdtuSuccess = m_pdtusender->InitDTU();
			if(!m_bdtuSuccess)
			{
				CString strErrInfo;
				strErrInfo.Format(_T("InitDTU Fail(%s:%d %s)!"),wstrTcpSenderIP.c_str(),_wtoi((wstrTcpSenderport.c_str()))+1,m_strTCPName.c_str());
				OutPutLogString(strErrInfo);
			}

			/*if(!m_bexitthread_history)
			{
				if(m_hsenddtuhandle == NULL)
					m_hsenddtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUThread, this, 0, NULL);
				if(m_hReceivedtuhandle == NULL)
					m_hReceivedtuhandle = (HANDLE)_beginthreadex(NULL, 0, SendDTUReceiveHandleThread, this, 0, NULL);
				if(m_hScanUpdateStatehandle == NULL)
					m_hScanUpdateStatehandle = (HANDLE)_beginthreadex(NULL, 0, ScanUpdateStateHandleThread, this, 0, NULL);
				if(m_hcheckhandle == NULL)
					m_hcheckhandle = (HANDLE)_beginthreadex(NULL, 0, CheckExeThread, this, 0, NULL);
			}*/
		}
		else
		{
			OutPutLogString(_T("Create DTUSender Fail!"));
		}
	}
	return m_bdtuSuccess;
}

unsigned int WINAPI CBEOPWatchDlg::SendDTUThread( LPVOID lpVoid )
{
	CBEOPWatchDlg* pDTUSendComm = reinterpret_cast<CBEOPWatchDlg*>(lpVoid);
	if (NULL == pDTUSendComm)
	{
		return 0;
	}

	while(!pDTUSendComm->GetExitThread_History())
	{
		// send Data
		DWORD waitstatus = WaitForSingleObject(pDTUSendComm->GetSendDTUEvent(), INFINITE);
		if (waitstatus == WAIT_OBJECT_0)
		{
			//发送DTU
			pDTUSendComm->SendDTUData();					   			
		}

	}

	return 1;
}

unsigned int WINAPI CBEOPWatchDlg::SendDTUReceiveHandleThread( LPVOID lpVoid )
{
	CBEOPWatchDlg* pDTUReceiveHandle = reinterpret_cast<CBEOPWatchDlg*>(lpVoid);
	if (NULL == pDTUReceiveHandle)
	{
		return 0;
	}

	while(!pDTUReceiveHandle->GetExitThread_History())
	{	
		pDTUReceiveHandle->ActiveLostSendEvent();		//不停激活待发送队列
		pDTUReceiveHandle->HandleCmdFromDTUServer();
		Sleep(1000);
	}
	return 1;
}

HANDLE CBEOPWatchDlg::GetSendDTUEvent()
{
	return m_senddtuevent;
}

bool CBEOPWatchDlg::Exit()
{	
	m_bexitthread_history = true;

	Project::Tools::Scoped_Lock<Mutex> guardlock(m_watch_lock);
	if(m_pdtusender)
	{
		m_pdtusender->Exit();
	}
	if(m_hsenddtuhandle)
	{
		SetEvent(m_senddtuevent);
		WaitForSingleObject(m_hsenddtuhandle, 2000);
	}

	if(m_hReceivedtuhandle)
		WaitForSingleObject(m_hReceivedtuhandle, 2000);

	if(m_hScanUpdateStatehandle)
		WaitForSingleObject(m_hScanUpdateStatehandle,2000);

	if(m_hcheckhandle)
		WaitForSingleObject(m_hcheckhandle,2000);

	if(m_hsenddtuhandle)
	{
		CloseHandle(m_hsenddtuhandle);
		m_hsenddtuhandle = NULL;
	}

	if(m_hReceivedtuhandle)
	{
		CloseHandle(m_hReceivedtuhandle);
		m_hReceivedtuhandle = NULL;
	}

	if(m_hScanUpdateStatehandle)
	{
		CloseHandle(m_hScanUpdateStatehandle);
		m_hScanUpdateStatehandle = NULL;
	}

	if(m_hcheckhandle)
	{
		CloseHandle(m_hcheckhandle);
		m_hcheckhandle = NULL;
	}

	if(m_pdtusender)
	{
		delete m_pdtusender;
		m_pdtusender = NULL;
	}

	if(m_pDataHandle)
	{
		delete m_pDataHandle;
		m_pDataHandle = NULL;
	}

	//dog退出
	CDogTcpCtrl::GetInstance()->Exit();
	CDogTcpCtrl::GetInstance()->DestroyInstance();

	return true;
}

bool CBEOPWatchDlg::GetExitThread_History() const
{
	return m_bexitthread_history;
}

bool CBEOPWatchDlg::SendDTUData()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_lock);
	bool bResult = true;
	//发送DTU数据
	if(m_pdtusender && m_bdtuSuccess)
	{
		if(m_vecSendBuffer.size() <= 0)
		{
			if(m_pdtusender->GetSendIdle())
			{
				//发送一个历史丢失文件
				SendOneLostHistoryData();
				SYSTEMTIME sNow;
				GetLocalTime(&sNow);
				SetSendDTUEventAndType();
			}
			return bResult;
		}

		//将全局发送vec拷贝到临时变量中
		m_bVecCopying = true;
		vector<DTUSendInfo> vecDTU;
		vecDTU = m_vecSendBuffer;
		m_vecSendBuffer.clear();
		m_bVecCopying = false;

		bool bSendDTUState = true;
		if(m_strAck.length()>0)
		{
			bSendDTUState = m_pdtusender->SendCmdState(m_ackTime,3,4,m_strAck);
			m_strAck = "";
			Sleep(1000);
		}

		if(m_strSendRealFilePath.length()>0)
		{
			bSendDTUState = m_pdtusender->SendRealDataFile(m_strSendRealFilePath,m_nSendRealFileCount);
			m_nSendRealFileCount = 0;
			m_strSendRealFilePath = "";
			Sleep(1000);
		}

		for(int i=0; i<vecDTU.size(); ++i)
		{
			//立即发送
			if(m_strSendRealFilePath.length()>0)
			{
				bSendDTUState = m_pdtusender->SendRealDataFile(m_strSendRealFilePath,m_nSendRealFileCount);
				m_nSendRealFileCount = 0;
				m_strSendRealFilePath = "";
				Sleep(1000);
			}

			DTUSendInfo sDTU = vecDTU[i];
			if(sDTU.nType == 3)		//Unit01
			{
				if(sDTU.strSendBuf.size()>0)
				{
					bSendDTUState = m_pdtusender->SendUnit01(sDTU.tSendTime,sDTU.strSendBuf);
				}

				if(sDTU.nSubType == 1)			//同步
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,1,sDTU.strPacketIndex);
				}
				else if(sDTU.nSubType == 2)		//修改
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,2,sDTU.strPacketIndex);
				}
				else if(sDTU.nSubType == 3)		//restart
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,3,sDTU.strPacketIndex);
				}
				else if(sDTU.nSubType == 5)		//sql命令
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,5,sDTU.strPacketIndex);
				}
				else if(sDTU.nSubType == 6)		//心跳包相应
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendHeartBeat(sDTU.tSendTime);
				}
				else if(sDTU.nSubType == 7)		//start
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCoreStart(Project::Tools::WideCharToAnsi(m_strTCPName.c_str()));
				}
				else if(sDTU.nSubType == 8)		//服务器状态信息
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,8,sDTU.strPacketIndex);
				}
				else if(sDTU.nSubType == 10)		//同步现场时钟
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,10,sDTU.strPacketIndex);
				}
				else if(sDTU.nSubType == 11)		//错误代码
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,11,sDTU.strPacketIndex);
				}
				else if(sDTU.nSubType == 12)		//错误日志列表
				{
					Sleep(1000);

					bSendDTUState = m_pdtusender->SendCmdState(sDTU.tSendTime,3,12,sDTU.strPacketIndex);
				}
			}	
			else if(sDTU.nType == 6)		//File
			{
				Sleep(1000);		

				bSendDTUState = m_pdtusender->SendFile(sDTU.strSendBuf);
				if(sDTU.nSubType == 1)					//删除文件
				{
					DelteFoder(Project::Tools::AnsiToWideChar(sDTU.strSendBuf.c_str()).c_str());
				}
			}
			else if(sDTU.nType == 7)		//realdata file
			{
				Sleep(1000);		

				bSendDTUState = m_pdtusender->SendRealDataFile(sDTU.strSendBuf,sDTU.nPointCount);
			}
			else if(sDTU.nType == 8)		//exe更新回执
			{
				Sleep(1000);

				bSendDTUState = m_pdtusender->SendUpdateFileAck(sDTU.nSubType,sDTU.strSendBuf);
			}
			Sleep(1000);			//DTU 延迟1s的发送间隔  
		}
	}

	return bResult;
}

bool CBEOPWatchDlg::HandleCmdFromDTUServer()
{
	if(m_pdtusender && m_bdtuSuccess)
	{		
		string strUpdateExeAck, strLostFile;
		if(m_pdtusender->ScanAndSendLostPackage(strUpdateExeAck, strLostFile))
		{
			if(strUpdateExeAck.length() > 0)
			{
				AckReceiveExeFile(strUpdateExeAck);
			}

			if(strLostFile.length() > 0)
			{
				AckLostExeFile(strLostFile);
			}
		}

		vector<vector<string>> vecCmd;	
		if(m_pdtusender->GetReceiveDTUServerEvent(vecCmd))		//扫描到命令
		{
			for(int i=0;i<vecCmd.size(); ++i)
			{
				vector<string> vec = vecCmd[i];
				if(vec.size() == 2)
				{
					if(vec[0] == "2"&& vec[1] == "0")	//同步Unit01
					{
						SynUnit01();
					}
					else if(vec[0] == "6")		//批量修改点
					{
						ChangeValuesByDTUServer(vec[1]);
					}
					else if(vec[0] == "8" && vec[1] == "0")		//心跳包请求
					{
						AckHeartBeat();
					}
					else if(vec[0] == "4")		//重发丢失实时文件包
					{
						AckReSendFileData(vec[1]);
					}	
					else if(vec[0] == "9")		//回应服务器状态
					{
						AckServerState();
					}
					else if(vec[0] == "5")	//设置Unit01
					{
						ChangeUnitsByDTUServer(vec[1]);
					}
				}
				else if(vec.size() == 3)
				{
					if(vec[0] == "1")								//历史数据
					{
						SendHistoryDataFile(vec[1],vec[2]);
					}
					else if(vec[0] == "5")	//设置Unit01
					{
						ChangeUnit01ByDTUServer(vec[1],vec[2]);
					}					
					else if(vec[0] == "10" && vec[1] =="1")		//删除点
					{
						DeleteMutilPoint(vec[2]);
					}
					else if(vec[0] == "3" && vec[1] == "0" && vec[2] == "0")		//重启Core
					{
						UpdateRestart();
					}
					else if(vec[0] == "3" && vec[1] == "1" && vec[2] == "0")		//Core版本号
					{
						AckCoreVersion();
					}
					else if(vec[0] == "3" && vec[1] == "1" && vec[2] == "1")		//同步现场点表
					{
						AckLocalPoint();
					}
					else if(vec[0] == "3" && vec[1] == "1" && vec[2] == "2")		//回应错误文件列表
					{
						AckErrFileList();
					}
					else if(vec[0] == "3" && vec[1] == "2")							//Core错误代码
					{
						AckCoreErrCode(vec[2]);
					}
					else if(vec[0] == "3" && vec[1] == "3")		//回应错误文件
					{
						AckErrFileByName(vec[2]);
					}
					else if(vec[0] == "3" && vec[1] == "4")		//回应数据库文件
					{
						AckMysqlFileByTableName(vec[2]);
					}
					else if(vec[0] == "7")			//回应丢失掉线期间数据zip
					{
						AckReSendLostZipData(vec[1],vec[2]);
					}
					else if(vec[0] == "10" && vec[1] =="0")		//增/修点表
					{
						UpdateMutilPoint(vec[2]);
					}
				}
				else if(vec.size() == 4)
				{
					if(vec[0] == "10" && vec[1] =="0")		//增/修点表
					{
						UpdateMutilPoint_(vec[2],vec[3]);
					}
				}
			}
		}
	}
	else
	{
		UpdateDTUSuccess();
	}
	return true;
}

bool CBEOPWatchDlg::UpdateDTUSuccess()
{
	if(m_pdtusender && !m_bdtuSuccess)
	{
		m_bdtuSuccess = m_pdtusender->GetDTUSuccess();
	}
	return true;
}

bool CBEOPWatchDlg::AckUpdateExeFile( string strExeName )
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	if(m_bdtuSuccess)
	{
		DTUSendInfo sDTUInfo;
		sDTUInfo.tSendTime = st;
		sDTUInfo.nType = 8;
		sDTUInfo.nSubType = 5;
		sDTUInfo.strSendBuf = strExeName;
		if(!m_bVecCopying)
			m_vecSendBuffer.push_back(sDTUInfo);
		SetSendDTUEventAndType();
	} 
	return true;
}

bool CBEOPWatchDlg::AckLostExeFile( string strLostFile )
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	if(m_bdtuSuccess)
	{
		DTUSendInfo sDTUInfo;
		sDTUInfo.tSendTime = st;
		sDTUInfo.nType = 8;
		sDTUInfo.nSubType = 2;
		sDTUInfo.strSendBuf = strLostFile;
		if(!m_bVecCopying)
			m_vecSendBuffer.push_back(sDTUInfo);
		SetSendDTUEventAndType();
	}
	return true;
}

bool CBEOPWatchDlg::SetSendDTUEventAndType()
{
	SetEvent(m_senddtuevent);
	return true;
}

unsigned int WINAPI CBEOPWatchDlg::ScanUpdateStateHandleThread( LPVOID lpVoid )
{
	CBEOPWatchDlg* pScanUpdateStateHandle = reinterpret_cast<CBEOPWatchDlg*>(lpVoid);
	if (NULL == pScanUpdateStateHandle)
	{
		return 0;
	}

	while(!pScanUpdateStateHandle->GetExitThread_History())
	{		
		pScanUpdateStateHandle->UpdateDTUInfo();
		pScanUpdateStateHandle->ScanUpdateState();
		pScanUpdateStateHandle->DeleteBackupFolderByDate();
		pScanUpdateStateHandle->DeleteTableByDiskWarning();
		pScanUpdateStateHandle->OutputMemorySize();
		pScanUpdateStateHandle->ScanAndGernerateLogFile();
		COleDateTime oleNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleSpan = oleNow - pScanUpdateStateHandle->m_oleLastSendReg;
		if (oleSpan.GetTotalSeconds() >= 90)
		{	
			pScanUpdateStateHandle->SendWatchStart();
			pScanUpdateStateHandle->m_oleLastSendReg = oleNow;
		}

		int nSleep = 10;
		while(!pScanUpdateStateHandle->GetExitThread_History())
		{
			nSleep--;
			if(nSleep <= 0)
				break;
			Sleep(1000);
		}
	}
	return 1;
}

bool CBEOPWatchDlg::ScanUpdateState()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatecore") == L"1")
		{
			if(!UpdateCore())
			{
				CString strLog;
				strLog.Format(_T("Update Core Failed"));
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
		}
		
		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatelogic") == L"1")
		{
			if(!UpdateLogic())
			{
				CString strLog;
				strLog.Format(_T("Update Logic Failed"));
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
		}

		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatedogfinish") == L"1")
		{
			//更新成功标志
			AckUpdateExeFile("BEOPWatch.exe");
			m_pDataHandle->WriteCoreDebugItemValue(L"updatedogfinish",L"0");
		}
		
		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatedll") == L"1")
		{
			if(!UpdateDll())
			{
				CString strLog;
				strLog.Format(_T("Update dll fails(%s)"),m_strUpdateXlsFile);
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
			else
			{
				CString strLog;
				strLog.Format(_T("Update dll success(%s)"),m_strUpdateXlsFile);
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
		}

		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updates3db") == L"1")
		{
			if(!UpdateS3db())
			{
				CString strLog;
				strLog.Format(_T("Update s3db fails(%s)"),m_strUpdateXlsFile);
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
		}

		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatebat") == L"1")
		{
			if(!UpdateBat())
			{
				CString strLog;
				strLog.Format(_T("Execute the bat file fails(%s)"),m_strUpdateBatFile);
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
			else
			{
				CString strLog;
				strLog.Format(_T("Execute the bat file success(%s)"),m_strUpdateBatFile);
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
		}
		
		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatexls") == L"1")
		{
			if(!UpdatePoint())
			{
				CString strLog;
				strLog.Format(_T("Update point fails(%s)"),m_strUpdateXlsFile);
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
		}
		
		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatecsv") == L"1")
		{
			if(!UpdatePoint_CSV())
			{
				CString strLog;
				strLog.Format(_T("Update point fails(%s)"),m_strUpdateXlsFile);
				OutPutUpdateLog(strLog);
				//更新成功标志
				AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
			}
		}
	}
	return false;
}

bool CBEOPWatchDlg::OutPutUpdateLog( CString strLog )
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\fileupdate";
	if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		setlocale( LC_ALL, "chs" );

		CString strOut;
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog);

		CString strLogPath;
		strLogPath.Format(_T("%s\\fileupdate.log"),wstrFileFolder.c_str());

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strOut);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
		setlocale( LC_CTYPE, old_locale ); 
		free( old_locale ); 
	}
	return false;
}

bool CBEOPWatchDlg::DelteFile( CString strDir,CString strDeleteDir )
{
	strDir += '\0';
	strDeleteDir += '\0';
	SHFILEOPSTRUCT    shFileOp;  
	memset(&shFileOp,0,sizeof(shFileOp));  
	shFileOp.wFunc    = FO_MOVE;  
	shFileOp.pFrom    = strDir;  
	shFileOp.pTo    = strDeleteDir;  
	shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION|FOF_ALLOWUNDO|FOF_NOCONFIRMMKDIR;  
	int nResult = ::SHFileOperation(&shFileOp);  
	if(nResult != 0)
	{
		CString strResult;
		strResult.Format(_T("Delete files error:%d"),nResult);
		OutPutUpdateLog(strResult);
	}
	return nResult == 0;  
}

bool CBEOPWatchDlg::DelteFile( CString strFilePath )
{
	try
	{
		strFilePath += '\0';
		SHFILEOPSTRUCT    shFileOp;  
		memset(&shFileOp,0,sizeof(shFileOp));  
		shFileOp.wFunc    = FO_DELETE;  
		shFileOp.pFrom    = strFilePath;  
		shFileOp.pTo    = NULL;  
		shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION|FOF_ALLOWUNDO|FOF_NOCONFIRMMKDIR;  
		int nResult = ::SHFileOperation(&shFileOp);  
		if(nResult != 0)
		{
			CString strResult;
			strResult.Format(_T("Delete files error:%d"),nResult);
			OutPutUpdateLog(strResult);
		}
		return nResult == 0;
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

bool CBEOPWatchDlg::FindFile( CString strFolder,CString strExten,vector<CString>& vecFileName )
{
	vecFileName.clear();
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\*.%s"),strFolder,strExten);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strFileName = tempFind.GetFileName();
				vecFileName.push_back(strFileName);
			}
		}
	}
	tempFind.Close();
	return true;
}

bool CBEOPWatchDlg::FindFile( CString strFolder,CString strName )
{
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\*.*"),strFolder);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strFileName = tempFind.GetFileName();
				if(strFileName.Find(strName)>=0)
				{
					tempFind.Close();
					return true;
				}					
			}
		}
	}
	tempFind.Close();
	return false;
}

bool CBEOPWatchDlg::UpdatePointXls( CString strExcelPath,CString strS3dbPath ,int& nErrCode)
{
	vector<DataPointEntry>	vecPoint;
	CExcelOperator excepOperator;
	if(excepOperator.ReadFromExcel(strExcelPath, vecPoint,nErrCode))
	{
		if (vecPoint.size()>0)
		{
			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
			CSqliteAcess access(strPath.c_str());
			bool bOK = false;
			int nNeedSave = 0;
			int nSaved = 0;
			access.BeginTransaction();
			access.DeleteAllPoint();
			int nCount = 0;
			CString	strPar16;
			CString	strPar17;
			CString	strPar18;
			for ( int i=0; i<vecPoint.size(); ++i )
			{	
				nNeedSave++;
				string strName;
				string strSourceType;
				string strParam1;
				string strParam2;
				string strParam3;
				string strParam4;
				string strParam5;
				string strParam6;
				string strParam7;
				string strParam8;
				string strParam9;
				string strParam10;
				string strParam11;
				string strParam12;
				string strParam13;
				string strParam14;
				string strParam15;
				string strDescription;
				string strUnit;
				const DataPointEntry& info = vecPoint[i];

				//name
				strName = Project::Tools::WideCharToAnsi(info.GetShortName().c_str());
				//SourceType
				strSourceType =  Project::Tools::WideCharToAnsi(info.GetSourceType().c_str());
				//param1 点位值
				strParam1 =  Project::Tools::WideCharToAnsi(info.GetParam(1).c_str());
				//param2 点位值
				strParam2 =  Project::Tools::WideCharToAnsi(info.GetParam(2).c_str());
				//param3 点位值
				strParam3 =  Project::Tools::WideCharToAnsi(info.GetParam(3).c_str());
				//param4 点位值
				strParam4 =  Project::Tools::WideCharToAnsi(info.GetParam(4).c_str());
				//param5 点位值
				strParam5 =  Project::Tools::WideCharToAnsi(info.GetParam(5).c_str());
				//param6 点位值
				strParam6 =  Project::Tools::WideCharToAnsi(info.GetParam(6).c_str());
				//param7 点位值
				strParam7 =  Project::Tools::WideCharToAnsi(info.GetParam(7).c_str());
				//param8 点位值
				strParam8 =  Project::Tools::WideCharToAnsi(info.GetParam(8).c_str());
				//param9 点位值
				strParam9 =  Project::Tools::WideCharToAnsi(info.GetParam(9).c_str());
				//param10 点位值
				strParam10 =  Project::Tools::WideCharToAnsi(info.GetParam(10).c_str());
				//param11 点位值
				strParam11 =  Project::Tools::WideCharToAnsi(info.GetParam(11).c_str());
				//param12 点位值
				strParam12 =  Project::Tools::WideCharToAnsi(info.GetParam(12).c_str());
				//param13 点位值
				strParam13 =  Project::Tools::WideCharToAnsi(info.GetParam(13).c_str());
				//param14 点位值
				strParam14 =  Project::Tools::WideCharToAnsi(info.GetParam(14).c_str());
				//param15 点位值
				strParam15 =  Project::Tools::WideCharToAnsi(info.GetParam(15).c_str());
				//param16 点位值
				strPar16.Format(_T("%s"), (info.GetParam(16)).c_str());
				int nParam16 = 0;
				for (int x=0; x<g_nLenPointAttrSystem; x++)
				{
					if (g_strPointAttrSystem[x] == strPar16)
					{
						nParam16 = x;
						break;
					}
				}
				//param17 点位值
				strPar17.Format(_T("%s"), (info.GetParam(17)).c_str());
				int nParam17 = 0;
				for (int y=0; y<g_nLenPointAttrDevice; y++)
				{
					if (g_strPointAttrDevice[y] == strPar17)
					{
						nParam17 = y;
						break;
					}
				}
				//param18 点位值
				strPar18.Format(_T("%s"), (info.GetParam(18)).c_str());
				int nParam18 = 0;
				for (int z=0; z<g_nLenPointAttrType; z++)
				{
					if (g_strPointAttrType[z] == strPar18)
					{
						nParam18 = z;
						break;
					}
				}

				//remark
				strDescription =  Project::Tools::WideCharToAnsi(info.GetDescription().c_str());
				//单位 unit
				strUnit =  Project::Tools::WideCharToAnsi(info.GetUnit().c_str());
				char szCycle[10] = {0};
				sprintf_s(szCycle,sizeof(szCycle),"%d",(int)info.GetStoreCycle());
				assert(strlen(szCycle)>0);
				int nIndex = 0;
				if (0 == info.GetPointIndex())
				{
					nIndex = i;
				}
				else
				{
					nIndex = info.GetPointIndex();
				}
				if(access.InsertRecordToOPCPoint(nIndex,0,strName.c_str(),strSourceType.c_str(),info.GetProperty(),strDescription.c_str(),strUnit.c_str(),
					info.GetHighAlarm(),info.GetHighHighAlarm(),info.GetLowAlarm(),info.GetLowLowAlarm(),strParam1.c_str(),
					strParam2.c_str(),strParam3.c_str(),strParam4.c_str(),strParam5.c_str(),strParam6.c_str(),strParam7.c_str(),
					strParam8.c_str(),strParam9.c_str(),strParam10.c_str(),szCycle,strParam12.c_str(),strParam13.c_str(),strParam14.c_str(),
					strParam15.c_str(),nParam16,nParam17,nParam18) == 0)
				{
					nSaved++;
				}
				else
				{
					CString strLog;
					strLog.Format(_T("Insert xls(%d) fails."),nIndex);
					OutPutUpdateLog(strLog);
					nErrCode = 31003;
				}
			}

			if (nNeedSave == nSaved)
			{
				bOK = true;
			}

			if(bOK)
			{
				access.CommitTransaction();
			}
			else
			{
				access.RollbackTransaction();
			}
			return bOK;
		}
	}
	else
	{
		CString strLog;
		strLog.Format(_T("Read xls(%s) fails."),strExcelPath);
		OutPutUpdateLog(strLog);
	}
	return false;
}

bool CBEOPWatchDlg::UpdatePoint()
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
				if(strS3dbPath.length() <= 0)
					return false;
				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//
					CString strNewPath;
					strNewPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strFilePath);
					//先备份一个原s3db到更新文件夹
					if(!CopyFile(strS3dbPath.c_str(),strNewPath,FALSE))
						return false;

					//再更新该备份s3db
					vector<CString> vecFile;
					FindFile(strUpdateFolder.c_str(),_T("xls"),vecFile);
					if(vecFile.size()>0)
					{
						//使用.xls更新备份s3db
						CString strExcelPath,strExcelBackPath;
						strExcelPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);

						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						if(UpdatePointXls(strExcelPath,strNewPath,m_nErrCode))		//更新成功
						{
							//备份当前使用s3db到备份文件夹
							if(FindOrCreateFolder(strBacFolder))
							{
								CString strNewBacPath;
								strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
								//先备份一个原s3db到更新文件夹
								if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
								{
									CString strLog;
									strLog.Format(_T("Backup s3db(%s) To Update Folder (%s) fails."),strS3dbPath.c_str(),strNewBacPath);
									OutPutUpdateLog(strLog);
									return false;
								}

								CString strLog;
								strLog.Format(_T("Update before backup s3db(%s)"),strNewBacPath);
								OutPutUpdateLog(strLog);
							}

							//覆盖
							if(DelteFile(strNewPath,strS3dbPath.c_str()))
							{
								if(m_pDataHandle && m_pDataHandle->IsConnected())
								{
									m_strUpdateS3dbPath = strS3dbPath.c_str();
									m_strUpdateXlsFile = vecFile[0];

									//重启Core
									m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"5");
								}
							}
							else
							{
								CString strLog;
								strLog.Format(_T("Copy Update s3db(%s) To the directory folder(%s) fails."),strNewPath,strS3dbPath.c_str());
								OutPutUpdateLog(strLog);
							}
						}
						else
						{
							CString strLog;
							strLog.Format(_T("Update Point s3db(%s) fails(%s)."),strExcelPath,strNewPath);
							OutPutUpdateLog(strLog);

							CString strUpdateLog;
							strUpdateLog.Format(_T("Update %s Fail."),vecFile[0]);
							//更新失败标志
							AckUpdateExeFile(Project::Tools::WideCharToAnsi(strUpdateLog));
						}

						CString strExcelName = vecFile[0];
						CString strName = strExcelName.Left(strExcelName.ReverseFind('.'));
						CString strPriName = strExcelName.Right(strExcelName.GetLength() - strExcelName.ReverseFind('.') - 1);
						//删除改.xls
						strExcelBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
						DelteFile(strExcelPath,strExcelBackPath);
					}
					else
					{
						//查找.xlsx
						FindFile(strUpdateFolder.c_str(),_T("xlsx"),vecFile);
						if(vecFile.size()>0)
						{
							//使用.xlsx更新备份s3db
							CString strExcelPath,strExcelBackPath;
							strExcelPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

							SYSTEMTIME sNow;
							GetLocalTime(&sNow);

							CString strTime;
							strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

							if(UpdatePointXls(strExcelPath,strNewPath,m_nErrCode))		//更新成功
							{
								//备份当前使用s3db到备份文件夹
								if(FindOrCreateFolder(strBacFolder))
								{
									CString strNewBacPath;
									strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
									//先备份一个原s3db到更新文件夹
									if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
									{
										CString strLog;
										strLog.Format(_T("Backup s3db(%s) To Update Folder (%s) fails."),strS3dbPath.c_str(),strNewBacPath);
										OutPutUpdateLog(strLog);
										return false;
									}

									CString strLog;
									strLog.Format(_T("Update before backup s3db(%s)"),strNewBacPath);
									OutPutUpdateLog(strLog);
								}

								//覆盖
								if(DelteFile(strNewPath,strS3dbPath.c_str()))
								{
									if(m_pDataHandle && m_pDataHandle->IsConnected())
									{
										m_strUpdateS3dbPath = strS3dbPath.c_str();
										m_strUpdateXlsFile = vecFile[0];

										//重启Core
										m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"5");
									}
								}
								else
								{
									CString strLog;
									strLog.Format(_T("Copy Update s3db(%s) To the directory folder(%s) fails."),strNewPath,strS3dbPath.c_str());
									OutPutUpdateLog(strLog);
								}
							}
							else
							{
								CString strLog;
								strLog.Format(_T("Update Point s3db(%s) fails(%s)."),strExcelPath,strNewPath);
								OutPutUpdateLog(strLog);

								CString strUpdateLog;
								strUpdateLog.Format(_T("Update %s Fail."),vecFile[0]);
								//更新失败标志
								AckUpdateExeFile(Project::Tools::WideCharToAnsi(strUpdateLog));
							}

							CString strExcelName = vecFile[0];
							CString strName = strExcelName.Left(strExcelName.ReverseFind('.'));
							CString strPriName = strExcelName.Right(strExcelName.GetLength() - strExcelName.ReverseFind('.') - 1);
							//删除改.xls
							strExcelBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
							DelteFile(strExcelPath,strExcelBackPath);
						}
						else
						{
							OutPutUpdateLog(_T("Can not find update point file！"));
						}
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"updatexls",L"0");
				}
			}
		}
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
	return true;
}

bool CBEOPWatchDlg::UpdateDll()
{
	//////////////////////////////////////////////////////////////////////////
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
				if(strS3dbPath.length() <= 0)
					return false;
				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//
					CString strNewPath;
					strNewPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strFilePath);
					//先备份一个原s3db到更新文件夹
					if(!CopyFile(strS3dbPath.c_str(),strNewPath,FALSE))
						return false;

					//再更新该备份s3db
					vector<CString> vecDLL,vecDCG;
					FindFile(strUpdateFolder.c_str(),_T("dll"),vecDLL);
					FindFile(strUpdateFolder.c_str(),_T("dcg"),vecDCG);

					SYSTEMTIME sNow;
					GetLocalTime(&sNow);
					CString strTime;
					strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

					if(vecDCG.size()>0)
					{
						//使用.dcg更新备份s3db
						CString strDCGPath,strDCGBackPath,strDCGName;
						strDCGName = vecDCG[0];
						strDCGPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strDCGName);

						int nDllSize,nDeleteParamSize,nInsertParamSize,nThreadSize;
						wstring strDllList,strParamList,strThreadList;

						wchar_t charSize[1024];
						GetPrivateProfileString(L"dll", L"dllSize", L"", charSize, 1024, strDCGPath);
						wstring strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nDllSize = _wtoi(strSize.c_str());

						//
						GetPrivateProfileString(L"dll", L"paramterDeleteSize", L"", charSize, 1024, strDCGPath);
						strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nDeleteParamSize = _wtoi(strSize.c_str());

						GetPrivateProfileString(L"dll", L"paramterInsertSize", L"", charSize, 1024, strDCGPath);
						strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nInsertParamSize = _wtoi(strSize.c_str());

						GetPrivateProfileString(L"dll", L"threadSize", L"", charSize, 1024, strDCGPath);
						strSize = charSize;
						if(strSize == L"")
							strSize = L"0";
						nThreadSize = _wtoi(strSize.c_str());

						bool bUpdateS3db = false;
						if(nDllSize>0)
						{
							wchar_t* charDllList = new wchar_t[nDllSize+1];
							GetPrivateProfileString(L"dll", L"dllList", L"", charDllList, nDllSize, strDCGPath);
							charDllList[nDllSize] = L'\0';
							strDllList = charDllList;
							delete[] charDllList;

							//插入，删除，更新dll
							vector<wstring> vecDll;
							Project::Tools::SplitStringByChar(strDllList.c_str(),L';',vecDll);
							for(int i=0; i<vecDll.size(); ++i)
							{
								wstring strDllInfo = vecDll[i];
								vector<wstring> vecDllInfo;
								Project::Tools::SplitStringByChar(strDllInfo.c_str(),L',',vecDllInfo);
								if(vecDllInfo.size() == 3)
								{
									CString strDllName = vecDllInfo[1].c_str();
									if(!UpdateDll(_wtoi(vecDllInfo[0].c_str()),strDllName,vecDllInfo[2].c_str(),strNewPath))
									{
										CString strLog;
										strLog.Format(_T("Update dll(%s) Failed."),strDllName);
										OutPutUpdateLog(strLog);
									}

									if(_wtoi(vecDllInfo[0].c_str()) == 0)
									{
										CString strDllPath,strDllBackPath;
										strDllPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strDllName);
										//删除改.dll
										CString strName = strDllName.Left(strDllName.ReverseFind('.'));
										CString strPriName = strDllName.Right(strDllName.GetLength() - strDllName.ReverseFind('.') - 1);
										strDllBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
										DelteFile(strDllPath,strDllBackPath);
									}
									bUpdateS3db = true;
								}
							}

							//
						}

						if(nDeleteParamSize>0)
						{
							wchar_t* charParamList = new wchar_t[nDeleteParamSize+1];
							GetPrivateProfileString(L"dll", L"paramterDelete", L"", charParamList, nDeleteParamSize, strDCGPath);
							charParamList[nDeleteParamSize] = L'\0';
							strParamList = charParamList;
							delete[] charParamList;

							//删除策略关系
							if(!DeleteDllParamter(strNewPath,strParamList))
							{
								CString strLog = _T("Delete dll Paramter Failed.");
								OutPutUpdateLog(strLog);
							}
							bUpdateS3db = true;
						}

						if(nInsertParamSize>0)
						{
							wchar_t* charParamList = new wchar_t[nInsertParamSize+1];
							GetPrivateProfileString(L"dll", L"paramterInsert", L"", charParamList, nInsertParamSize, strDCGPath);
							charParamList[nInsertParamSize] = L'\0';
							strParamList = charParamList;
							delete[] charParamList;

							//插入策略关系
							if(!UpdateDllParamter(strNewPath,strParamList))
							{
								CString strLog = _T("Insert dll Paramter Failed.");
								OutPutUpdateLog(strLog);
							}
							bUpdateS3db = true;
						}

						if(nThreadSize>0)
						{
							wchar_t* charThreadList = new wchar_t[nThreadSize+1];
							GetPrivateProfileString(L"dll", L"threadList", L"", charThreadList, nThreadSize, strDCGPath);
							charThreadList[nThreadSize] = L'\0';
							strThreadList = charThreadList;
							delete[] charThreadList;

							//更新线程关系
							vector<wstring> vecThread;
							Project::Tools::SplitStringByChar(strThreadList.c_str(),L';',vecThread);
							for(int i=0; i<vecThread.size(); ++i)
							{
								wstring strThreadInfo = vecThread[i];
								vector<wstring> vecThreadInfo;
								Project::Tools::SplitStringByChar(strThreadInfo.c_str(),L',',vecThreadInfo);
								if(vecThreadInfo.size() == 3)		//只需修改数据库
								{
									if(!UpdateThreadState(strNewPath.GetString(),vecThreadInfo[0],vecThreadInfo[1],vecThreadInfo[2]))
									{
										CString strLog;
										strLog.Format(_T("Update Thread(%s) Failed."),vecThreadInfo[1].c_str());
										OutPutUpdateLog(strLog);
									}

									if(_wtoi(vecThreadInfo[0].c_str()) == 1)
									{
										bUpdateS3db = true;
									}	
								}
							}
						}

						//删除配置文件
						CString strName = strDCGName.Left(strDCGName.ReverseFind('.'));
						CString strPriName = strDCGName.Right(strDCGName.GetLength() - strDCGName.ReverseFind('.') - 1);
						strDCGBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
						DelteFile(strDCGPath,strDCGBackPath);

						if(bUpdateS3db)
						{
							SYSTEMTIME sNow;
							GetLocalTime(&sNow);

							CString strTime;
							strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

							//备份当前使用s3db到备份文件夹
							if(FindOrCreateFolder(strBacFolder))
							{
								CString strNewBacPath;
								strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
								//先备份一个原s3db到更新文件夹
								if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
								{
									CString strLog;
									strLog.Format(_T("Backup s3db(%s) To Update Folder (%s) Fails."),strS3dbPath.c_str(),strNewBacPath);
									OutPutUpdateLog(strLog);
									return false;
								}

								CString strLog;
								strLog.Format(_T("Backup before update s3db(%s)."),strNewBacPath);
								OutPutUpdateLog(strLog);
							}

							//覆盖
							if(DelteFile(strNewPath,strS3dbPath.c_str()))
							{
								if(m_pDataHandle && m_pDataHandle->IsConnected())
								{							
									//重启Core
									m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"7");
								}
							}
							else
							{
								CString strLog;
								strLog.Format(_T("Copy s3db(%s) to the directory folder (%s) fails."),strNewPath,strS3dbPath.c_str());
								OutPutUpdateLog(strLog);
							}
						}
					}
				}
				m_pDataHandle->WriteCoreDebugItemValue(L"updatedll",L"0");
			}
		}
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
	return true;
}

bool CBEOPWatchDlg::UpdateDll(int nUpdateSate,CString strDllName,CString strThreadName,CString strS3dbPath)
{
	try
	{
		if(nUpdateSate == 0)		//增加或修改
		{
			if(strDllName.GetLength()<=0)
			{
				return false;
			}

			wstring strFolder;
			GetSysPath(strFolder);
			strFolder += L"\\fileupdate\\update";
			if(FindOrCreateFolder(strFolder))
			{
				if(FindFile(strFolder.c_str(),strDllName))
				{
					CString strdllPath;
					strdllPath.Format(_T("%s\\%s"),strFolder.c_str(),strDllName);
					HINSTANCE hDLL = NULL;
					hDLL = LoadLibrary(strdllPath);//加载动态链接库MyDll.dll文件；
					int nErr = GetLastError();
					if(hDLL==NULL)
					{
						return false;
					}
					pfnLogicBase pf_Dll = (pfnLogicBase)GetProcAddress(hDLL,"fnInitLogic");
					if(pf_Dll==NULL)
					{
						FreeLibrary(hDLL);
						return false;
					}

					CLogicBase *pLBDll = pf_Dll();
					CString strVersion,strDescription,strAuthor;
					if(pLBDll != NULL)
					{
						strVersion = pLBDll->GetDllLogicVersion();
						strDescription = pLBDll->GetDllDescription();
						strAuthor = pLBDll->GetDllAuthor();
					}

					if (pLBDll != NULL)
					{
						delete pLBDll;
						pLBDll = NULL;
					}
					if (hDLL != NULL)
					{
						FreeLibrary(hDLL);
						hDLL = NULL;
					}

					//读取内容
					return StoreDllToDB(strdllPath,strS3dbPath,strDllName,strThreadName,strVersion,strDescription,strAuthor);
				}
			}
		}
		else if(nUpdateSate == 1)						//删除
		{
			if(strDllName.GetLength()<=0)
			{
				return false;
			}

			return DeleteDll(strS3dbPath,strDllName,strThreadName);
		}
		else if(nUpdateSate == 2)						//更改策略线程
		{
			if(strDllName.GetLength()<=0)
			{
				return false;
			}

			return UpdateDllThread(strS3dbPath,strDllName,strThreadName);
		}
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

bool CBEOPWatchDlg::UpdateDllThread(CString strS3dbPath,CString strDllName,CString strThreadName)
{
	try
	{
		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			_bstr_t tem_des = strDllName;
			char* des = (char*)tem_des;
			std::string dllname__(des);

			tem_des = strThreadName;
			des = (char*)tem_des;
			std::string threadname__(des);

			//删除已有的同名dll
			string sql_del;
			ostringstream sqlstream_del;
			sqlstream_del << "update list_dllstore set unitproperty04 ='" << threadname__ << "' where DllName = '" << dllname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}
			access.CloseDataBase();
			return true;
		}
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

bool CBEOPWatchDlg::UpdateWatch()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"updatedogfinish") == L"1")
		{
			//更新成功标志
			AckUpdateExeFile("BEOPWatch.exe");
			m_pDataHandle->WriteCoreDebugItemValue(L"updatedogfinish",L"0");
		}
	}
	return true;
}

bool CBEOPWatchDlg::UpdateLogic()
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
				if(strS3dbPath.length() <= 0)
					return false;

				CString strFolderPath = strS3dbPath.c_str();
				strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//找到更新文件
					if(FindFile(strUpdateFolder.c_str(),_T("BEOPLogicEngine.exe")))
					{
						//拷贝更新文件到运行目录  以.bak结尾 并删除更新文件
						CString strUpdatePath;
						strUpdatePath.Format(_T("%s\\BEOPLogicEngine.exe"),strUpdateFolder.c_str());

						CString strCurrentBacPath;
						strCurrentBacPath.Format(_T("%s\\BEOPLogicEngine.exe.bak"),strFolderPath);
						if(!DelteFile(strUpdatePath,strCurrentBacPath))
							return false;

						//备份当前运行的Core文件到back文件夹
						CString strCurrentPath;
						strCurrentPath.Format(_T("%s\\BEOPLogicEngine.exe"),strFolderPath);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);
						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						if(FindOrCreateFolder(strBacFolder))
						{
							CString strBackPath;
							strBackPath.Format(_T("%s\\BEOPLogicEngine%s.exe"),strBacFolder.c_str(),strTime);

							//当前exe存在拷贝一份
							if(FindFile(strFolderPath,_T("BEOPLogicEngine.exe")))
							{
								if(!CopyFile(strCurrentPath,strBackPath,FALSE))
									return false;

								CString strLog;
								strLog.Format(_T("Backup before update Logic(%s)"),strBackPath);
								OutPutUpdateLog(strLog);
							}
						}

						//关闭当前运行Logic
						CloseLogic();

						//删除当前运行Logic
						DelteFoder(strCurrentPath);

						//重命名.bak为
						int nReslut = rename(Project::Tools::WideCharToAnsi(strCurrentBacPath).c_str(),Project::Tools::WideCharToAnsi(strCurrentPath).c_str());
						if(nReslut != 0)
						{
							int nErrCode = GetLastError();
							CString strLog;
							strLog.Format(_T("Rename (result:%d,errCode:%d)"),nReslut,nErrCode);
							OutPutUpdateLog(strLog);
						}

						//重启Logic
						if(m_pDataHandle && m_pDataHandle->IsConnected())
						{
							m_strUpdateCorePath = strUpdatePath;
							m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"6");
						}
						m_pDataHandle->WriteCoreDebugItemValue(L"updatelogic",L"0");
					}
				}
			}
		}
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
	return true;
}

bool CBEOPWatchDlg::UpdateCore()
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
				if(strS3dbPath.length() <= 0)
					return false;

				CString strFolderPath = strS3dbPath.c_str();
				strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//找到更新文件
					if(FindFile(strUpdateFolder.c_str(),_T("domcore.exe")))
					{
						//拷贝更新文件到运行目录  以.bak结尾 并删除更新文件
						CString strUpdatePath;
						strUpdatePath.Format(_T("%s\\domcore.exe"),strUpdateFolder.c_str());

						CString strCurrentBacPath;
						strCurrentBacPath.Format(_T("%s\\domcore.exe.bak"),strFolderPath);
						if(!DelteFile(strUpdatePath,strCurrentBacPath))
							return false;

						//备份当前运行的Core文件到back文件夹
						CString strCurrentPath;
						strCurrentPath.Format(_T("%s\\domcore.exe"),strFolderPath);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);
						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						if(FindOrCreateFolder(strBacFolder))
						{
							CString strBackPath;
							strBackPath.Format(_T("%s\\BEOPGatewayCore_%s.exe"),strBacFolder.c_str(),strTime);

							//当前exe存在拷贝一份
							if(FindFile(strFolderPath,_T("domcore.exe")))
							{
								if(!CopyFile(strCurrentPath,strBackPath,FALSE))
									return false;

								CString strLog;
								strLog.Format(_T("Backup before update Core(%s)"),strBackPath);
								OutPutUpdateLog(strLog);
							}
						}

						//关闭当前运行Core
						CloseCore();

						//删除当前运行Core
						DelteFoder(strCurrentPath);

						//重命名.bak为
						int nReslut = rename(Project::Tools::WideCharToAnsi(strCurrentBacPath).c_str(),Project::Tools::WideCharToAnsi(strCurrentPath).c_str());
						if(nReslut != 0)
						{
							int nErrCode = GetLastError();
							CString strLog;
							strLog.Format(_T("Rename (result:%d,errCode:%d)"),nReslut,nErrCode);
							OutPutUpdateLog(strLog);
						}

						//重启Core
						if(m_pDataHandle && m_pDataHandle->IsConnected())
						{
							m_strUpdateCorePath = strUpdatePath;
							m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"6");
						}
						m_pDataHandle->WriteCoreDebugItemValue(L"updatecore",L"0");
					}
				}
			}
		}
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
	return true;
}

bool CBEOPWatchDlg::AckReceiveExeFile( string strExeName )
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	if(m_bdtuSuccess)
	{
		DTUSendInfo sDTUInfo;
		sDTUInfo.tSendTime = st;
		sDTUInfo.nType = 8;
		sDTUInfo.nSubType = 1;		//接收
		sDTUInfo.strSendBuf = strExeName;
		if(!m_bVecCopying)
			m_vecSendBuffer.push_back(sDTUInfo);
		SetSendDTUEventAndType();
	} 
	return true;
}

unsigned int WINAPI CBEOPWatchDlg::CheckExeThread( LPVOID lpVoid )
{
	CBEOPWatchDlg* pCheckStateHandle = reinterpret_cast<CBEOPWatchDlg*>(lpVoid);
	if (NULL == pCheckStateHandle)
	{
		return 0;
	}

	while(!pCheckStateHandle->GetExitThread_History())
	{		
		pCheckStateHandle->FeedDog();
		pCheckStateHandle->CheckStateIsNeedRestart();
		pCheckStateHandle->HandleRestart();
		pCheckStateHandle->HandleRestartLogic();
		pCheckStateHandle->WriteActiveTime();
		pCheckStateHandle->ChangeIconByDTUState();
		pCheckStateHandle->ShowDTURouterStateInfo();
		int nSleep = g_Check_Interval;
		while(!pCheckStateHandle->GetExitThread_History())
		{
			nSleep--;
			if(nSleep <= 0)
				break;
			Sleep(1000);
		}
	}
	return 1;
}

bool CBEOPWatchDlg::CheckStateIsNeedRestart()
{
	//启动时候延迟1分钟判断
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleStartTime;
	if(oleSpan.GetTotalMinutes() < 1)
	{
		return false;
	}
	//先判断一下有没有更新
	if(m_pDataHandle)
	{
		if(m_pDataHandle->IsConnected())
		{
			//如果有待需要重启 也先不要继续判断了
			if(m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"restartcoretype",L"-1") != L"-1")
				return true;
				
			//检测自动重启时间   6:05
			wstring wstrCheckRestart = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"checkrestart",L"");
			if(wstrCheckRestart.length() > 0)
			{
				vector<wstring> vecTime;
				Project::Tools::SplitStringByChar(wstrCheckRestart.c_str(),L':',vecTime);
				if(vecTime.size() == 2)
				{
					COleDateTime oleNow = COleDateTime::GetCurrentTime();
					if(oleNow.GetYear() != m_oleLastRestartTime.GetYear() && oleNow.GetYear() != m_oleLastRestartTime.GetMonth() && oleNow.GetDay() != m_oleLastRestartTime.GetDay() && oleNow.GetHour() >= _wtoi(vecTime[0].c_str()) && oleNow.GetMinute() >= _wtoi(vecTime[1].c_str()))
					{
						m_oleLastRestartTime = COleDateTime::GetCurrentTime();
						m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"10");
						return true;
					}
				}
			}	
			return false;
		}
	}	
	return false;
}

bool CBEOPWatchDlg::HandleRestart()
{
	if(m_pDataHandle)
	{
		if(m_pDataHandle->IsConnected())
		{
			wstring wstrAutoStartCore = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"restartcoretype",L"-1");
			m_nRestartType = _wtoi(wstrAutoStartCore.c_str());
			switch(m_nRestartType)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				{
					if(RestartCoreByDog(m_nRestartType,m_nErrorCode))
					{
						
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"-1");
				}
				break;
			case 8:
			case 10:
			case 11:
				{
					if(RestartCoreByDog(m_nRestartType,m_nErrorCode,true))
					{
				
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"-1");
				}
				break;
			case 12:
			case 5:
				{
					if(RestartCoreByDog(m_nRestartType,m_nErrorCode,true))
					{
						CString strLog;
						strLog.Format(_T("Update s3db(%s)."),m_strUpdateS3dbPath);
						OutPutUpdateLog(_T("Update s3db."));
						//更新成功标志
						AckUpdateExeFile(Project::Tools::WideCharToAnsi(m_strUpdateXlsFile));
					}
					else
					{
						CString strLog;
						strLog.Format(_T("Update success(%s),But restart fails."),m_strUpdateXlsFile);
						OutPutUpdateLog(_T("Update s3db."));
						//更新成功标志
						AckUpdateExeFile(Project::Tools::WideCharToAnsi(strLog));
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"-1");
				}
				break;
			case 6:
				{
					if(RestartCoreByDog(m_nRestartType,m_nErrorCode,true))
					{
						

						CString strLog;
						strLog.Format(_T("Update And Restart Core(%s)."),m_strUpdateCorePath);
						OutPutUpdateLog(strLog);
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"-1");
				}
				break;
			case 7:
				{
					if(RestartCoreByDog(m_nRestartType,m_nErrorCode,true))
					{
						
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"-1");
				}
				break;
			case 9:
				{
					if(RestartCoreByDog(m_nRestartType,m_nErrorCode,true))
					{
						
						AckUpdateExeFile("Remote Server Restart Core Success.");
					}
					else
					{
						
						CString strFail;
						strFail.Format(_T("Remote Server Restart Core Fails(%d)."),m_nErrorCode);
						AckUpdateExeFile(Project::Tools::WideCharToAnsi(strFail));
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"-1");
				}
				break;
			default:
				break;
			}
			return true;
		}
	}
	return false;
}

bool CBEOPWatchDlg::SumRestartTime( int nMinute /*= 40*/ )
{
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	if(m_vecReStartTime.size() >= g_Restart_Count)
	{
		m_vecReStartTime.pop_back();
	}
	m_vecReStartTime.push_back(oleNow);
	int nSize = m_vecReStartTime.size();
	if(nSize == g_Restart_Count && g_Restart_Count>0)
	{
		COleDateTimeSpan oleSpan = oleNow - m_vecReStartTime[0];
		if(oleSpan.GetTotalMinutes() <= nMinute)
		{
			m_vecReStartTime.clear();
			int nSleep = 60;
			while(!GetExitThread_History())
			{
				nSleep--;
				if(nSleep <= 0)
					return true;
				Sleep(1000);
			}
			return true;
		}
	}
	return false;
}

//DTUServerCmd:10|0|46,A32AHU_A_42_PressSaOut,DB-MySQL,,,R,1,2,3,4,5,6,7,8,9,10,,11,12,1,2,13,1,2,3;;
bool CBEOPWatchDlg::UpdateMutilPoint( string strPoints )
{
	try
	{
		vector<wstring> vecPoint;
		Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strPoints.c_str()).c_str(),L';',vecPoint);

		vector<vector<wstring>> vecPoints;
		for(int i=0; i<vecPoint.size(); ++i)
		{
			wstring strPoints = vecPoint[i];
			vector<wstring> vecPointInfo;
			Project::Tools::SplitStringByChar(strPoints.c_str(),L',',vecPointInfo);
			if(vecPointInfo.size() < g_Point_Length)
				continue;
			vecPoints.push_back(vecPointInfo);
		}
		if(vecPoints.size()>0)
		{
			vector<DataPointEntry> vecEditPoint;
			if(GeneratePointFromInfo(vecPoints,vecEditPoint))
			{
				if(UpdateMutilPoint(vecEditPoint))
				{
					//回复

					std::ostringstream sqlstream_point;
					for(int i=0; i<vecEditPoint.size(); ++i)
					{
						sqlstream_point << Project::Tools::WideCharToAnsi(vecEditPoint[i].GetShortName().c_str()) << ",";
					}
					AckUpdatePoint(sqlstream_point.str(),0);
				}
			}
			return true;
		}
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

bool CBEOPWatchDlg::UpdateMutilPoint( vector<DataPointEntry>& vecPoint )
{
	try
	{
		if(vecPoint.size() > 0)
		{
			//
			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				wstring strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
				if(strS3dbPath.length() <= 0)
					return false;

				m_strUpdateS3dbPath = strS3dbPath.c_str();

				string strPath;
				Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
				CSqliteAcess access(strPath.c_str());
				bool bOK = false;
				int nNeedSave = 0;
				int nSaved = 0;
				access.BeginTransaction();
				int nCount = 0;
				CString	strPar16;
				CString	strPar17;
				CString	strPar18;
				for ( int i=0; i<vecPoint.size(); ++i )
				{	
					nNeedSave++;
					string strName;
					string strSourceType;
					string strParam1;
					string strParam2;
					string strParam3;
					string strParam4;
					string strParam5;
					string strParam6;
					string strParam7;
					string strParam8;
					string strParam9;
					string strParam10;
					string strParam11;
					string strParam12;
					string strParam13;
					string strParam14;
					string strParam15;
					string strDescription;
					string strUnit;
					const DataPointEntry& info = vecPoint[i];

					//name
					strName = Project::Tools::WideCharToAnsi(info.GetShortName().c_str());
					//SourceType
					strSourceType =  Project::Tools::WideCharToAnsi(info.GetSourceType().c_str());
					//param1 点位值
					strParam1 =  Project::Tools::WideCharToAnsi(info.GetParam(1).c_str());
					//param2 点位值
					strParam2 =  Project::Tools::WideCharToAnsi(info.GetParam(2).c_str());
					//param3 点位值
					strParam3 =  Project::Tools::WideCharToAnsi(info.GetParam(3).c_str());
					//param4 点位值
					strParam4 =  Project::Tools::WideCharToAnsi(info.GetParam(4).c_str());
					//param5 点位值
					strParam5 =  Project::Tools::WideCharToAnsi(info.GetParam(5).c_str());
					//param6 点位值
					strParam6 =  Project::Tools::WideCharToAnsi(info.GetParam(6).c_str());
					//param7 点位值
					strParam7 =  Project::Tools::WideCharToAnsi(info.GetParam(7).c_str());
					//param8 点位值
					strParam8 =  Project::Tools::WideCharToAnsi(info.GetParam(8).c_str());
					//param9 点位值
					strParam9 =  Project::Tools::WideCharToAnsi(info.GetParam(9).c_str());
					//param10 点位值
					strParam10 =  Project::Tools::WideCharToAnsi(info.GetParam(10).c_str());
					//param11 点位值
					strParam11 =  Project::Tools::WideCharToAnsi(info.GetParam(11).c_str());
					//param12 点位值
					strParam12 =  Project::Tools::WideCharToAnsi(info.GetParam(12).c_str());
					//param13 点位值
					strParam13 =  Project::Tools::WideCharToAnsi(info.GetParam(13).c_str());
					//param14 点位值
					strParam14 =  Project::Tools::WideCharToAnsi(info.GetParam(14).c_str());
					//param15 点位值
					strParam15 =  Project::Tools::WideCharToAnsi(info.GetParam(15).c_str());
					//param16 点位值
					strPar16.Format(_T("%s"), (info.GetParam(16)).c_str());
					int nParam16 = 0;
					for (int x=0; x<g_nLenPointAttrSystem; x++)
					{
						if (g_strPointAttrSystem[x] == strPar16)
						{
							nParam16 = x;
							break;
						}
					}
					//param17 点位值
					strPar17.Format(_T("%s"), (info.GetParam(17)).c_str());
					int nParam17 = 0;
					for (int y=0; y<g_nLenPointAttrDevice; y++)
					{
						if (g_strPointAttrDevice[y] == strPar17)
						{
							nParam17 = y;
							break;
						}
					}
					//param18 点位值
					strPar18.Format(_T("%s"), (info.GetParam(18)).c_str());
					int nParam18 = 0;
					for (int z=0; z<g_nLenPointAttrType; z++)
					{
						if (g_strPointAttrType[z] == strPar18)
						{
							nParam18 = z;
							break;
						}
					}

					//remark
					strDescription =  Project::Tools::WideCharToAnsi(info.GetDescription().c_str());
					//单位 unit
					strUnit =  Project::Tools::WideCharToAnsi(info.GetUnit().c_str());
					char szCycle[10] = {0};
					sprintf_s(szCycle,sizeof(szCycle),"%d",(int)info.GetStoreCycle());
					assert(strlen(szCycle)>0);
					int nIndex = 0;
					if (0 == info.GetPointIndex())
					{
						nIndex = i;
					}
					else
					{
						nIndex = info.GetPointIndex();
					}
					access.DeletePointByName(strName.c_str());
					if(access.InsertRecordToOPCPoint(nIndex,0,strName.c_str(),strSourceType.c_str(),info.GetProperty(),strDescription.c_str(),strUnit.c_str(),
						info.GetHighAlarm(),info.GetHighHighAlarm(),info.GetLowAlarm(),info.GetLowLowAlarm(),strParam1.c_str(),
						strParam2.c_str(),strParam3.c_str(),strParam4.c_str(),strParam5.c_str(),strParam6.c_str(),strParam7.c_str(),
						strParam8.c_str(),strParam9.c_str(),strParam10.c_str(),szCycle,strParam12.c_str(),strParam13.c_str(),strParam14.c_str(),
						strParam15.c_str(),nParam16,nParam17,nParam18) == 0)
					{
						nSaved++;
					}
					else
					{
						CString strLog;
						strLog.Format(_T("Insert xls(%d) Fails."),nIndex);
						OutPutUpdateLog(strLog);
					}
				}

				if (nNeedSave == nSaved)
				{
					bOK = true;
				}

				if(bOK)
				{
					access.CommitTransaction();
				}
				else
				{
					access.RollbackTransaction();
				}
				return bOK;
			}
		}
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

bool CBEOPWatchDlg::GeneratePointFromInfo( vector<vector<wstring>> vecPoints, vector<DataPointEntry>& vecPoint )
{
	try
	{
		if(m_pDataHandle && m_pDataHandle->IsConnected())
		{
			wstring strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
			if(strS3dbPath.length() <= 0)
				return false;

			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
			CSqliteAcess access(strPath.c_str());
			vecPoint.clear();
			for(int i=0; i<vecPoints.size(); ++i)
			{
				vector<wstring> vecPointInfo = vecPoints[i];
				//
				DataPointEntry entry;
				CString strIndex = vecPointInfo[0].c_str();
				CString strPhysicalID = vecPointInfo[1].c_str();
				CString strSource = vecPointInfo[2].c_str();
				CString strRemark = vecPointInfo[3].c_str();
				CString strUnit = vecPointInfo[4].c_str();
				CString strRWProperty = vecPointInfo[5].c_str();
				CString strparam1 = vecPointInfo[6].c_str();
				CString strparam2 = vecPointInfo[7].c_str();
				CString strparam3 = vecPointInfo[8].c_str();
				CString strparam4 = vecPointInfo[9].c_str();
				CString strparam5 = vecPointInfo[10].c_str();
				CString strparam6 = vecPointInfo[11].c_str();
				CString strparam7 = vecPointInfo[12].c_str();
				CString strparam8 = vecPointInfo[13].c_str();
				CString strparam9 = vecPointInfo[14].c_str();
				CString strparam10 = vecPointInfo[15].c_str();
				CString strparam11 = vecPointInfo[16].c_str();
				CString strparam12 = vecPointInfo[17].c_str();
				CString strparam13 = vecPointInfo[18].c_str();
				CString strparam14 = vecPointInfo[19].c_str();

				CString strparam15 = vecPointInfo[21].c_str();
				CString strparam16 = vecPointInfo[22].c_str();
				CString strparam17 = vecPointInfo[23].c_str();
				CString strparam18 = vecPointInfo[24].c_str();
				CString strStoreCycle = vecPointInfo[20].c_str();

				int nIndex = _wtoi(strIndex);
				entry.SetPointIndex(nIndex);
				entry.SetShortName(strPhysicalID.GetString());
				entry.SetSourceType(strSource.GetString());
				entry.SetDescription(strRemark.GetString());
				entry.SetUnit(strUnit.GetString());

				if(strRWProperty == L"R")
				{
					entry.SetProperty(PLC_READ);
				}
				else if(strRWProperty == L"W")
				{
					entry.SetProperty(PLC_WRITE);
				}
				else if(strRWProperty == L"R/W")
				{
					entry.SetProperty(PLC_PROP_MAX);
				}
				else
				{
					entry.SetProperty(PLC_READ);
				}
				entry.SetParam(1, strparam1.GetString());
				entry.SetParam(2, strparam2.GetString());
				entry.SetParam(3, strparam3.GetString());
				entry.SetParam(4, strparam4.GetString());
				entry.SetParam(5, strparam5.GetString());
				entry.SetParam(6, strparam6.GetString());
				entry.SetParam(7, strparam7.GetString());
				entry.SetParam(8, strparam8.GetString());
				entry.SetParam(9, strparam9.GetString());
				entry.SetParam(10, strparam10.GetString());
				entry.SetParam(11, strparam11.GetString());
				entry.SetParam(12, strparam12.GetString());
				entry.SetParam(13, strparam13.GetString());
				entry.SetParam(14, strparam14.GetString());
				entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
				entry.SetParam(15, strparam15.GetString());
				entry.SetParam(16, strparam16.GetString());
				entry.SetParam(17, strparam17.GetString());
				entry.SetParam(18, strparam18.GetString());
				if(entry.GetShortName().length() >0)
					vecPoint.push_back(entry);
			}
			return true;
		}
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

bool CBEOPWatchDlg::DeleteMutilPoint( string strPoints )
{
	try
	{
		if(m_pDataHandle && m_pDataHandle->IsConnected())
		{
			vector<wstring> vecPoint;
			Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strPoints.c_str()).c_str(),L',',vecPoint);
			if(vecPoint.size() <=0)
			{
				return false;
			}
			wstring strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
			if(strS3dbPath.length() <= 0)
				return false;

			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
			CSqliteAcess access(strPath.c_str());
			access.BeginTransaction();
			std::ostringstream sqlstream_point;
			for(int i=0; i<vecPoint.size(); ++i)
			{
				wstring strPointName = vecPoint[i];
				if(strPointName.length() <= 0)
					continue;
				string strName_Ansi = Project::Tools::WideCharToAnsi(strPointName.c_str());
				sqlstream_point << strName_Ansi << ",";
				if(access.DeletePointByName(strName_Ansi.c_str()) !=0)
				{
					access.RollbackTransaction();
					return false;
				}
			}
			access.CommitTransaction();
			AckUpdatePoint(sqlstream_point.str(),1);
		}
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

bool CBEOPWatchDlg::AckUpdatePoint( string strPointNames, int nOperation )
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	if(m_bdtuSuccess)
	{
		DTUSendInfo sDTUInfo;
		sDTUInfo.tSendTime = st;
		sDTUInfo.nType = 8;
		sDTUInfo.nSubType = 10;
		if(nOperation == 1)
			sDTUInfo.nSubType = 11;
		sDTUInfo.strSendBuf = strPointNames;
		if(!m_bVecCopying)
			m_vecSendBuffer.push_back(sDTUInfo);
		SetSendDTUEventAndType();
	} 
	return true;
}

bool CBEOPWatchDlg::UpdatePoint_CSV()
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
				if(strS3dbPath.length() <= 0)
					return false;
				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					//
					CString strNewPath;
					strNewPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),strFilePath);
					//先备份一个原s3db到更新文件夹
					if(!CopyFile(strS3dbPath.c_str(),strNewPath,FALSE))
						return false;

					//再更新该备份s3db
					vector<CString> vecFile;
					FindFile(strUpdateFolder.c_str(),_T("csv"),vecFile);
					if(vecFile.size()>0)
					{
						//使用.csv更新备份s3db
						CString strExcelPath,strExcelBackPath;
						strExcelPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);

						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						if(UpdatePointCSV(strExcelPath,strNewPath,m_nErrCode))		//更新成功
						{
							//备份当前使用s3db到备份文件夹
							if(FindOrCreateFolder(strBacFolder))
							{
								CString strNewBacPath;
								strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
								//先备份一个原s3db到更新文件夹
								if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
								{
									CString strLog;
									strLog.Format(_T("Backup s3db(%s) To Update Folder (%s) Fails."),strS3dbPath.c_str(),strNewBacPath);
									OutPutUpdateLog(strLog);
									return false;
								}

								CString strLog;
								strLog.Format(_T("Backup before update s3db(%s)."),strNewBacPath);
								OutPutUpdateLog(strLog);
							}

							//覆盖
							if(DelteFile(strNewPath,strS3dbPath.c_str()))
							{
								if(m_pDataHandle && m_pDataHandle->IsConnected())
								{
									m_strUpdateS3dbPath = strS3dbPath.c_str();
									m_strUpdateXlsFile = vecFile[0];

									//重启Core
									m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"5");
								}
							}
							else
							{
								CString strLog;
								strLog.Format(_T("Copy s3db(%s) to the directory folder (%s) fails."),strNewPath,strS3dbPath.c_str());
								OutPutUpdateLog(strLog);
							}
						}
						else
						{
							CString strLog;
							strLog.Format(_T("Update s3db(%s) fails(%s)."),strExcelPath,strNewPath);
							OutPutUpdateLog(strLog);

							CString strUpdateLog;
							strUpdateLog.Format(_T("Update %s Fail."),vecFile[0]);
							//更新失败标志
							AckUpdateExeFile(Project::Tools::WideCharToAnsi(strUpdateLog));
						}

						CString strExcelName = vecFile[0];
						CString strName = strExcelName.Left(strExcelName.ReverseFind('.'));
						CString strPriName = strExcelName.Right(strExcelName.GetLength() - strExcelName.ReverseFind('.') - 1);
						//删除改.csv
						strExcelBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
						DelteFile(strExcelPath,strExcelBackPath);
					}
					else
					{
						OutPutUpdateLog(_T("Can not find update point file！"));
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"updatecsv",L"0");
				}
			}
		}
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
	return true;
}

bool CBEOPWatchDlg::UpdatePointCSV( CString strCSVPath,CString strS3dbPath,int& nErrCode )
{
	try
	{
		vector<DataPointEntry>	vecPoint;
		CExcelOperator excepOperator;
		if(excepOperator.ReadFromCSV_Comma(strCSVPath, vecPoint,nErrCode))
		{
			if (vecPoint.size()>0)
			{
				string strPath;
				Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
				CSqliteAcess access(strPath.c_str());
				bool bOK = false;
				int nNeedSave = 0;
				int nSaved = 0;
				access.BeginTransaction();
				access.DeleteAllPoint();
				int nCount = 0;
				CString	strPar16;
				CString	strPar17;
				CString	strPar18;
				for ( int i=0; i<vecPoint.size(); ++i )
				{	
					nNeedSave++;
					string strName;
					string strSourceType;
					string strParam1;
					string strParam2;
					string strParam3;
					string strParam4;
					string strParam5;
					string strParam6;
					string strParam7;
					string strParam8;
					string strParam9;
					string strParam10;
					string strParam11;
					string strParam12;
					string strParam13;
					string strParam14;
					string strParam15;
					string strDescription;
					string strUnit;
					const DataPointEntry& info = vecPoint[i];

					//name
					strName = Project::Tools::WideCharToAnsi(info.GetShortName().c_str());
					//SourceType
					strSourceType =  Project::Tools::WideCharToAnsi(info.GetSourceType().c_str());
					//param1 点位值
					strParam1 =  Project::Tools::WideCharToAnsi(info.GetParam(1).c_str());
					//param2 点位值
					strParam2 =  Project::Tools::WideCharToAnsi(info.GetParam(2).c_str());
					//param3 点位值
					strParam3 =  Project::Tools::WideCharToAnsi(info.GetParam(3).c_str());
					//param4 点位值
					strParam4 =  Project::Tools::WideCharToAnsi(info.GetParam(4).c_str());
					//param5 点位值
					strParam5 =  Project::Tools::WideCharToAnsi(info.GetParam(5).c_str());
					//param6 点位值
					strParam6 =  Project::Tools::WideCharToAnsi(info.GetParam(6).c_str());
					//param7 点位值
					strParam7 =  Project::Tools::WideCharToAnsi(info.GetParam(7).c_str());
					//param8 点位值
					strParam8 =  Project::Tools::WideCharToAnsi(info.GetParam(8).c_str());
					//param9 点位值
					strParam9 =  Project::Tools::WideCharToAnsi(info.GetParam(9).c_str());
					//param10 点位值
					strParam10 =  Project::Tools::WideCharToAnsi(info.GetParam(10).c_str());
					//param11 点位值
					strParam11 =  Project::Tools::WideCharToAnsi(info.GetParam(11).c_str());
					//param12 点位值
					strParam12 =  Project::Tools::WideCharToAnsi(info.GetParam(12).c_str());
					//param13 点位值
					strParam13 =  Project::Tools::WideCharToAnsi(info.GetParam(13).c_str());
					//param14 点位值
					strParam14 =  Project::Tools::WideCharToAnsi(info.GetParam(14).c_str());
					//param15 点位值
					strParam15 =  Project::Tools::WideCharToAnsi(info.GetParam(15).c_str());
					//param16 点位值
					strPar16.Format(_T("%s"), (info.GetParam(16)).c_str());
					int nParam16 = 0;
					for (int x=0; x<g_nLenPointAttrSystem; x++)
					{
						if (g_strPointAttrSystem[x] == strPar16)
						{
							nParam16 = x;
							break;
						}
					}
					//param17 点位值
					strPar17.Format(_T("%s"), (info.GetParam(17)).c_str());
					int nParam17 = 0;
					for (int y=0; y<g_nLenPointAttrDevice; y++)
					{
						if (g_strPointAttrDevice[y] == strPar17)
						{
							nParam17 = y;
							break;
						}
					}
					//param18 点位值
					strPar18.Format(_T("%s"), (info.GetParam(18)).c_str());
					int nParam18 = 0;
					for (int z=0; z<g_nLenPointAttrType; z++)
					{
						if (g_strPointAttrType[z] == strPar18)
						{
							nParam18 = z;
							break;
						}
					}

					//remark
					strDescription =  Project::Tools::WideCharToAnsi(info.GetDescription().c_str());
					//单位 unit
					strUnit =  Project::Tools::WideCharToAnsi(info.GetUnit().c_str());
					char szCycle[10] = {0};
					sprintf_s(szCycle,sizeof(szCycle),"%d",(int)info.GetStoreCycle());
					assert(strlen(szCycle)>0);
					int nIndex = 0;
					if (0 == info.GetPointIndex())
					{
						nIndex = i;
					}
					else
					{
						nIndex = info.GetPointIndex();
					}
					if(access.InsertRecordToOPCPoint(nIndex,0,strName.c_str(),strSourceType.c_str(),info.GetProperty(),strDescription.c_str(),strUnit.c_str(),
						info.GetHighAlarm(),info.GetHighHighAlarm(),info.GetLowAlarm(),info.GetLowLowAlarm(),strParam1.c_str(),
						strParam2.c_str(),strParam3.c_str(),strParam4.c_str(),strParam5.c_str(),strParam6.c_str(),strParam7.c_str(),
						strParam8.c_str(),strParam9.c_str(),strParam10.c_str(),szCycle,strParam12.c_str(),strParam13.c_str(),strParam14.c_str(),
						strParam15.c_str(),nParam16,nParam17,nParam18) == 0)
					{
						nSaved++;
					}
					else
					{
						CString strLog;
						strLog.Format(_T("Insert s3db(%d) fails."),nIndex);
						OutPutUpdateLog(strLog);
						nErrCode = 31003;
					}
				}

				if (nNeedSave == nSaved)
				{
					bOK = true;
				}

				if(bOK)
				{
					access.CommitTransaction();
				}
				else
				{
					access.RollbackTransaction();
				}
				return bOK;
			}
		}
		else
		{
			CString strLog;
			strLog.Format(_T("Read csv(%s) fails."),strCSVPath);
			OutPutUpdateLog(strLog);
		}
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

bool CBEOPWatchDlg::AckRestartCore()
{
	return true;
}

bool CBEOPWatchDlg::UpdateRestart()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"9");
		m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"9");
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::DeleteBackupFolderByDate()
{
	try
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleLastDleteTime;
		if(oleSpan.GetTotalHours() >= 1)
		{
			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				wstring wstrBackupDay = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"backupday",L"90");
				int nSaveDay = _wtoi(wstrBackupDay.c_str());

				wstring strPath,strFolder;
				GetSysPath(strFolder);
				strPath = strFolder;
				strPath += L"\\backup";
				if(Project::Tools::FindOrCreateFolder(strPath))
				{
					COleDateTime oleDeadDate = COleDateTime::GetCurrentTime() - COleDateTimeSpan(nSaveDay,0,0,0);
					CString strDeadFolder;
					strDeadFolder.Format(_T("%04d%02d%02d"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteBackupDirectory(strPath.c_str(),strDeadFolder);			
				}

				//删除err目录下的log
				strPath = strFolder;
				strPath += L"\\log";
				if(Project::Tools::FindOrCreateFolder(strPath))
				{
					COleDateTime oleDeadDate = COleDateTime::GetCurrentTime() - COleDateTimeSpan(nSaveDay,0,0,0);
					//err file
					CString strDeadFolder;
					strDeadFolder.Format(_T("err_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("err_"),_T(".log"),strDeadFolder);

					//opc err file
					CString strOPCErrFile;
					strOPCErrFile.Format(_T("opc_err_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("opc_err_"),_T(".log"),strOPCErrFile);

					//s7udpctrl err file
					CString strS7UdpCtrlErrFile;
					strS7UdpCtrlErrFile.Format(_T("s7udpctrl_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("s7udpctrl_"),_T(".log"),strS7UdpCtrlErrFile);

					//dtucmderr_ err file
					CString strDTUCmdErrFile;
					strDTUCmdErrFile.Format(_T("dtucmderr_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("dtucmderr_"),_T(".log"),strDTUCmdErrFile);
				}

				//删除log目录下的log
				strPath = strFolder;
				strPath += L"\\log";
				if(Project::Tools::FindOrCreateFolder(strPath))
				{
					COleDateTime oleDeadDate = COleDateTime::GetCurrentTime() - COleDateTimeSpan(nSaveDay,0,0,0);
					//opc_debug file
					CString strOPCFolder;
					strOPCFolder.Format(_T("opc_debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("opc_debug_"),_T(".log"),strOPCFolder);

					//bacnet_debug_ file
					CString strBacnetFile;
					strBacnetFile.Format(_T("bacnet_debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("bacnet_debug_"),_T(".log"),strBacnetFile);

					//modbus_debug_ err file
					CString strModbusFile;
					strModbusFile.Format(_T("modbus_debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("modbus_debug_"),_T(".log"),strModbusFile);

					//debug_ err file
					CString strDebugFile;
					strDebugFile.Format(_T("debug_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("debug_"),_T(".log"),strDebugFile);

					//dtuservercmd_ err file
					CString strDTUCmdFile;
					strDTUCmdFile.Format(_T("dtuservercmd_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("dtuservercmd_"),_T(".log"),strDTUCmdFile);

					//router err file
					CString strRouterErrFile;
					strRouterErrFile.Format(_T("routerstate_%d_%02d_%02d.log"),oleDeadDate.GetYear(),oleDeadDate.GetMonth(),oleDeadDate.GetDay());
					FindAndDeleteErrFile(strPath.c_str(),_T("routerstate_"),_T(".log"),strRouterErrFile);
				}

			}
			m_oleLastDleteTime = COleDateTime::GetCurrentTime();
		}
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
	return true;
}

bool CBEOPWatchDlg::FindAndDeleteBackupDirectory( CString strDir,CString strDeadFolder )
{
	try
	{
		CFileFind tempFind;
		CString strFind;
		strFind.Format(_T("%s\\*.*"),strDir);
		BOOL IsFinded = tempFind.FindFile(strFind);
		CString strBackupPath;
		while (IsFinded)
		{
			IsFinded = tempFind.FindNextFile();
			if (!tempFind.IsDots()) 
			{
				if (tempFind.IsDirectory())
				{
					CString strDirName = tempFind.GetFileName();
					CString strDirPath = tempFind.GetFilePath();
					if(strDirName <= strDeadFolder)
					{
						DelteFoder(strDirPath);
					}
					else
					{
						FindAndDeleteBackupDirectory(strDirPath,strDeadFolder);
					}
				}
			}
		}

		tempFind.Close();
		return true;
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

bool CBEOPWatchDlg::DelteFoder( CString strDir )
{
	try
	{
		strDir += '\0';
		SHFILEOPSTRUCT    shFileOp;  
		memset(&shFileOp,0,sizeof(shFileOp));  
		shFileOp.wFunc    = FO_DELETE;  
		shFileOp.pFrom    = strDir;  
		shFileOp.pTo    = NULL;  
		shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;  
		int nResult = ::SHFileOperation(&shFileOp);  
		if(nResult != 0)
		{
			CString strResult;
			strResult.Format(_T("Delete File Error:%d"),nResult);
		}
		return nResult == 0;
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

bool CBEOPWatchDlg::SynUnit01()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		vector<unitStateInfo> unitlist;
		if(m_pDataHandle->ReadRealTimeData_Unit01(unitlist))
		{
			if(unitlist.size()>0)
			{
				SYSTEMTIME st;
				GetLocalTime(&st);

				string strSendBuf = BuildDTUSendUnit01(unitlist);
				if(strSendBuf.size()>0)
				{		
					if(m_bdtuSuccess)
					{
						DTUSendInfo sDTUInfo;
						sDTUInfo.tSendTime = st;
						sDTUInfo.nType = 3;
						sDTUInfo.nSubType = 1;
						sDTUInfo.strSendBuf = strSendBuf;
						sDTUInfo.strPacketIndex = "10";
						if(!m_bVecCopying)
							m_vecSendBuffer.push_back(sDTUInfo);
					}
				}

				SetSendDTUEventAndType();
			}
			return true;
		}
	}
	return false;
}

bool CBEOPWatchDlg::ChangeValuesByDTUServer( string strReceive )
{
	return true;
}

bool CBEOPWatchDlg::AckHeartBeat()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	if(m_bdtuSuccess)
	{
		DTUSendInfo sDTUInfo;
		sDTUInfo.tSendTime = st;
		sDTUInfo.nType = 3;
		sDTUInfo.nSubType = 6;
		sDTUInfo.strSendBuf = "";
		sDTUInfo.strPacketIndex = "13";
		if(!m_bVecCopying)
			m_vecSendBuffer.push_back(sDTUInfo);
		SetSendDTUEventAndType();
	}
	return true;
}

bool CBEOPWatchDlg::AckReSendFileData( string strFileName )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	//发送DTU数据 
	if(m_bdtuSuccess)
	{
		if(strFileName.length() <= 0)
			return false;

		wstring strFolder;
		Project::Tools::GetSysPath(strFolder);
		strFolder += L"\\backup";
		std::ostringstream sqlstream_path;

		//判断文件是否存在的
		sqlstream_path.str("");
		sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str())
			<< "\\" << strFileName.substr(11,4)  << strFileName.substr(15,2) 
			<< strFileName.substr(17,2) << "\\" << strFileName;
		string strPath = sqlstream_path.str();
		if(Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strPath.c_str())))
		{
			//Ack
			m_strSendRealFilePath = strPath;	
			m_nSendRealFileCount = GetFileInfoPointCont(strPath,strFileName);

			SYSTEMTIME st;
			GetLocalTime(&st);
			SetSendDTUEventAndType();
		}
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::SendHistoryDataFile( string strStart,string strEnd )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	//发送DTU数据 
	if(m_bdtuSuccess)
	{
		wstring strPath;
		GetSysPath(strPath);
		strPath += L"\\backup";	 
		if(Project::Tools::FindOrCreateFolder(strPath))
		{
			//将全部数据文件打包成一个压缩包
			COleDateTime oleStart,oleEnd;
			COleDateTimeSpan oleSpan(0,0,1,0);
			Project::Tools::String_To_OleTime(Project::Tools::AnsiToWideChar(strStart.c_str()),oleStart);
			Project::Tools::String_To_OleTime(Project::Tools::AnsiToWideChar(strEnd.c_str()),oleEnd);

			CString strZipPath;
			strZipPath.Format(_T("%s\\history_%d%02d%02d.zip"),strPath.c_str(),oleStart.GetYear(),oleStart.GetMonth(),oleStart.GetDay());
			HZIP hz = CreateZip(strZipPath,NULL);
			bool bFileExist = false;
			while(oleStart <= oleEnd)
			{
				if(oleStart.GetMinute() %5 != 0)
				{
					oleStart = oleStart + oleSpan;
					continue;
				}
				CString strFilePath,strFileName;
				strFileName.Format(_T("databackup_%d%02d%02d%02d%02d.zip"),oleStart.GetYear(),oleStart.GetMonth(),oleStart.GetDay(),oleStart.GetHour(),oleStart.GetMinute());
				strFilePath.Format(_T("%s\\%d%02d%02d\\%s"),strPath.c_str(),oleStart.GetYear(),oleStart.GetMonth(),oleStart.GetDay(),strFileName);
				if(Project::Tools::FindFileExist(strFilePath.GetString()))
				{
					bFileExist = true;
					ZipAdd(hz,strFileName,strFilePath);
				}
				oleStart = oleStart + oleSpan;
			}
			CloseZip(hz);

			SYSTEMTIME st;
			GetLocalTime(&st);

			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 6;
			m_strZipPath = Project::Tools::WideCharToAnsi(strZipPath);
			if(bFileExist)
			{
				sDTUInfo.strSendBuf = m_strZipPath;
			}
			else
			{
				sDTUInfo.strSendBuf = m_strZipPath;
			}
			if(m_bdtuSuccess)
			{
				if(!m_bVecCopying)
					m_vecSendBuffer.push_back(sDTUInfo);

				SetSendDTUEventAndType();
			}
			return true;
		}
	}
	return false;
}

bool CBEOPWatchDlg::AckReSendLostZipData( string strMinutes,string strFileNames )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	//发送DTU数据 
	if(m_bdtuSuccess)
	{
		vector<string> vec;
		Project::Tools::SplitString(strFileNames.c_str(),",",vec);

		wstring strFolder;
		Project::Tools::GetSysPath(strFolder);
		strFolder += L"\\backup";
		std::ostringstream sqlstream_path;

		for(int i=0; i<vec.size(); ++i)
		{
			string strFileName = vec[i];
			if(strFileName.length() < 19)
				continue;

			//判断文件是否存在的
			sqlstream_path.str("");
			sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str())
				<< "\\" << strFileName.substr(11,4)  << strFileName.substr(15,2) 
				<< strFileName.substr(17,2);
			string strFolder_ = sqlstream_path.str();
			sqlstream_path  << "\\" << strFileName;
			string strPath = sqlstream_path.str();

			if(Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strPath.c_str())))
			{
				m_mapSendLostHistory[vec[i]] = 0;
			}
		}

		//Ack
		if(m_bdtuSuccess)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 8;
			sDTUInfo.nSubType = 4;
			sDTUInfo.strSendBuf = strMinutes;
			if(!m_bVecCopying)
				m_vecSendBuffer.push_back(sDTUInfo);
			SetSendDTUEventAndType();
		}
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::ChangeUnit01ByDTUServer( string strParamName, string strParamValue )
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		wstring strName = Project::Tools::AnsiToWideChar(strParamName.c_str());
		wstring strValue = Project::Tools::AnsiToWideChar(strParamValue.c_str());
		if(m_pDataHandle->WriteCoreDebugItemValue(strName,strValue))
		{
			if(strName == L"restartwatch" && strValue == L"1")
			{
				ResetTCPInfo();
			}

			SYSTEMTIME st;
			GetLocalTime(&st);
			if(m_bdtuSuccess)
			{
				DTUSendInfo sDTUInfo;
				sDTUInfo.tSendTime = st;
				sDTUInfo.nType = 3;
				sDTUInfo.nSubType = 2;
				sDTUInfo.strSendBuf = "";
				sDTUInfo.strPacketIndex = "11";
				if(!m_bVecCopying)
					m_vecSendBuffer.push_back(sDTUInfo);
				SetSendDTUEventAndType();
			}
			return true;
		}
	}
	return false;
}

string CBEOPWatchDlg::BuildDTUSendUnit01( vector<unitStateInfo> unitlist )
{
	if (unitlist.empty()){
		return "";
	}

	string m_sendbuffer;
	std::ostringstream sqlstream;
	//pointtype,pointname,pointvalue;
	for (unsigned int i = 0; i < unitlist.size(); i++)
	{
		sqlstream <<  unitlist[i].unitproperty01 << "," << unitlist[i].unitproperty02 << ";";
	}
	m_sendbuffer = sqlstream.str();
	return m_sendbuffer;
}

int CBEOPWatchDlg::GetFileInfoPointCont( string strPath,string strFileName )
{
	if(strPath.length() <= 0 || strFileName.length() <=0 )
		return 0;

	int pos = strPath.find_last_of('\\');
	ostringstream strFilePath_;
	strFilePath_ << strPath.substr(0,pos) << "\\FilePoint.ini";
	string strFilePath = strFilePath_.str(); 
	int nPointCount = 0;
	CString strName = Project::Tools::AnsiToWideChar(strFileName.c_str()).c_str();
	wchar_t charPointCount[256];
	GetPrivateProfileString(L"file", strName.GetString(), L"", charPointCount, 256, Project::Tools::AnsiToWideChar(strFilePath.c_str()).c_str());
	wstring wstrPointCount = charPointCount;
	if(wstrPointCount == L"")
		wstrPointCount = L"0";
	nPointCount = _wtoi(wstrPointCount.c_str());
	return nPointCount;
}

bool CBEOPWatchDlg::SendOneLostHistoryData()
{
	map<string,int>::iterator iter = m_mapSendLostHistory.begin();
	if(iter != m_mapSendLostHistory.end())
	{
		string strFileName = iter->first;
		if(strFileName.length() <= 0)
			return false;

		wstring strFolder;
		Project::Tools::GetSysPath(strFolder);

		std::ostringstream sqlstream_path;
		sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str()) << "\\backup" 
			<< "\\" << strFileName.substr(11,4) << strFileName.substr(15,2) 
			<< strFileName.substr(17,2) << "\\" << strFileName;

		string strPath = sqlstream_path.str();
		int nPointCount = GetFileInfoPointCont(strPath,strFileName);
		m_pdtusender->SendHistoryDataFile(strPath,nPointCount);
		m_mapSendLostHistory.erase(iter);
		Sleep(1000);
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::UpdateMutilPoint_( string strPropertyCount,string strPoints )
{
	if(strPropertyCount.length() <=0 || strPoints.length() <= 0)
		return false;

	int nPropertyCount = atoi(strPropertyCount.c_str());
	if(nPropertyCount<=0)
		return false;

	vector<wstring> vecPointInfo;
	Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strPoints.c_str()).c_str(),L',',vecPointInfo);

	int nUpdatePoint = vecPointInfo.size()/nPropertyCount;
	if(vecPointInfo.size()%nPropertyCount != 0 || nUpdatePoint <= 1)
		return false;

	//header
	std::map<std::string, int> mapColumn;
	for(int i=0; i<nPropertyCount; ++i)
	{
		string ColumnName = Project::Tools::WideCharToAnsi(vecPointInfo[i].c_str());
		mapColumn[ColumnName] = i;
	}

	//point
	vector<DataPointEntry> vecEditPoint;
	for(int i=1; i<nUpdatePoint; ++i)
	{
		DataPointEntry entry;
		CString strIndex = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"pointindex")].c_str();
		CString strPhysicalID = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"physicalid")].c_str();
		CString strSource = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"source")].c_str();
		CString strRemark = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"remark")].c_str();
		CString strUnit = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"Unit")].c_str();
		CString strRWProperty = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"RWProperty")].c_str();
		CString strparam1 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param1")].c_str();
		CString strparam2 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param2")].c_str();
		CString strparam3 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param3")].c_str();
		CString strparam4 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param4")].c_str();
		CString strparam5 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param5")].c_str();
		CString strparam6 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param6")].c_str();
		CString strparam7 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param7")].c_str();
		CString strparam8 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param8")].c_str();
		CString strparam9 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param9")].c_str();
		CString strparam10 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param10")].c_str();

		CString strparam15 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"customName")].c_str();
		CString strparam16 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"system")].c_str();
		CString strparam17 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"device")].c_str();
		CString strparam18 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"type")].c_str();

		CString strparam11 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param11")].c_str();
		CString strparam12 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param12")].c_str();
		CString strparam13 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param13")].c_str();
		CString strparam14 = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"param14")].c_str();
		CString strStoreCycle = vecPointInfo[nPropertyCount*i+GetColumnIndex(mapColumn,"storecycle")].c_str();

		int nIndex = _wtoi(strIndex);
		entry.SetPointIndex(nIndex);
		entry.SetShortName(strPhysicalID.GetString());
		entry.SetSourceType(strSource.GetString());
		entry.SetDescription(strRemark.GetString());
		entry.SetUnit(strUnit.GetString());

		if(strRWProperty == L"R")
		{
			entry.SetProperty(PLC_READ);
		}
		else if(strRWProperty == L"W")
		{
			entry.SetProperty(PLC_WRITE);
		}
		else if(strRWProperty == L"R/W")
		{
			entry.SetProperty(PLC_PROP_MAX);
		}
		else
		{
			entry.SetProperty(PLC_READ);
		}
		entry.SetParam(1, strparam1.GetString());
		entry.SetParam(2, strparam2.GetString());
		entry.SetParam(3, strparam3.GetString());
		entry.SetParam(4, strparam4.GetString());
		entry.SetParam(5, strparam5.GetString());
		entry.SetParam(6, strparam6.GetString());
		entry.SetParam(7, strparam7.GetString());
		entry.SetParam(8, strparam8.GetString());
		entry.SetParam(9, strparam9.GetString());
		entry.SetParam(10, strparam10.GetString());
		entry.SetParam(11, strparam11.GetString());
		entry.SetParam(12, strparam12.GetString());
		entry.SetParam(13, strparam13.GetString());
		entry.SetParam(14, strparam14.GetString());
		entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
		entry.SetParam(15, strparam15.GetString());
		entry.SetParam(16, strparam16.GetString());
		entry.SetParam(17, strparam17.GetString());
		entry.SetParam(18, strparam18.GetString());
		if(entry.GetShortName().length() >0)
			vecEditPoint.push_back(entry);
	}

	if(UpdateMutilPoint(vecEditPoint))
	{
		//回复
		std::ostringstream sqlstream_point;
		for(int i=0; i<vecEditPoint.size(); ++i)
		{
			sqlstream_point << Project::Tools::WideCharToAnsi(vecEditPoint[i].GetShortName().c_str()) << ",";
		}

		if(m_pDataHandle && m_pDataHandle->IsConnected())
		{
			m_strUpdateXlsFile = Project::Tools::AnsiToWideChar(sqlstream_point.str().c_str()).c_str();

			//重启Core
			m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"5");
		}
		return true;
	}
	return false;
}

int CBEOPWatchDlg::GetColumnIndex( std::map<std::string, int> map, string strColumnName )
{
	int nIndex = 0;
	std::map<std::string, int>::iterator iter = map.find(strColumnName);
	if(iter != map.end())
	{
		return iter->second;
	}
	return nIndex;
}

bool CBEOPWatchDlg::ActiveLostSendEvent()
{
	if(!m_mapSendLostHistory.empty())
	{
		SetSendDTUEventAndType();
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::AckServerState()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	if(m_bdtuSuccess)
	{
		std::ostringstream sqlstream;
		if(m_pDataHandle && m_pDataHandle->IsConnected())
		{
			wstring strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
			if(strS3dbPath.length() <= 0)
				return false;

			string strDisk = Project::Tools::WideCharToAnsi(strS3dbPath.c_str());
			strDisk = strDisk.substr(0,2);

			//获取硬盘空间,内存,CPU利用率
			string strDiskInfo;
			if(CheckDiskFreeSpace(strDisk,strDiskInfo))
			{

			}
			sqlstream << strDiskInfo << "|";
			sqlstream << GetMemoryPervent() << "|" << GetCPUUseage();
		}

		DTUSendInfo sDTUInfo;
		sDTUInfo.tSendTime = st;
		sDTUInfo.nType = 3;
		sDTUInfo.nSubType = 8;
		sDTUInfo.strPacketIndex = sqlstream.str().c_str();
		if(!m_bVecCopying)
			m_vecSendBuffer.push_back(sDTUInfo);
		SetSendDTUEventAndType();
	} 
	return true;
}

bool CBEOPWatchDlg::CheckDiskFreeSpace( string strDisk,string& strDiskInfo )
{
	strDiskInfo = "";
	unsigned __int64 i64FreeBytesToCaller;  
	unsigned __int64 i64TotalBytes;  
	unsigned __int64 i64FreeBytes;  

	bool fResult = ::GetDiskFreeSpaceEx (  
		Project::Tools::AnsiToWideChar(strDisk.c_str()).c_str(),  
		(PULARGE_INTEGER)&i64FreeBytesToCaller,  
		(PULARGE_INTEGER)&i64TotalBytes,  
		(PULARGE_INTEGER)&i64FreeBytes);  
	//GetDiskFreeSpaceEx函数，可以获取驱动器磁盘的空间状态,函数返回的是个BOOL类型数据  
	if(fResult)
	{
		int nCount = (float)i64TotalBytes/1024/1024;
		int nSpace = (float)i64FreeBytesToCaller/1024/1024;
		std::ostringstream sqlstream;
		sqlstream << nSpace << "/" << nCount;
		strDiskInfo = sqlstream.str().c_str();
		return true;
	}
	return false;
}

double CBEOPWatchDlg::CpuUseage()
{
	if(GetSystemTimes( &m_preidleTime, &m_prekernelTime, &m_preuserTime))
	{
		Sleep(1000);

		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;
		GetSystemTimes( &idleTime, &kernelTime, &userTime );

		int idle = CompareFileTime( m_preidleTime,idleTime);
		int kernel = CompareFileTime( m_prekernelTime, kernelTime);
		int user = CompareFileTime(m_preuserTime, userTime);

		if(kernel+user == 0)
			return 0.0;
		//（总的时间-空闲时间）/总的时间=占用cpu的时间就是使用率
		double cpu = (kernel +user - idle) *100/(kernel+user);

		m_preidleTime = idleTime;
		m_prekernelTime = kernelTime;
		m_preuserTime = userTime ;
		return cpu;
	}
	return 0;
}

__int64 CBEOPWatchDlg::CompareFileTime( FILETIME time1, FILETIME time2 )
{
	__int64 a = time1.dwHighDateTime << 32 | time1.dwLowDateTime ;
	__int64 b = time2.dwHighDateTime << 32 | time2.dwLowDateTime ;
	return   (b - a);
}

int CBEOPWatchDlg::GetCPUUseage()
{
	FILETIME ftIdle, ftKernel, ftUser;
	double fOldCPUIdleTime = 0;
	double fOldCPUKernelTime = 0;
	double fOldCPUUserTime = 0;
	int nCPUUseRate = 0;
	if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser))
	{
		fOldCPUIdleTime = FileTimeToDouble(ftIdle);
		fOldCPUKernelTime = FileTimeToDouble(ftKernel);
		fOldCPUUserTime = FileTimeToDouble(ftUser);

		Sleep(1000);

		if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser))
		{
			double fCPUIdleTime = FileTimeToDouble(ftIdle);
			double fCPUKernelTime = FileTimeToDouble(ftKernel);
			double fCPUUserTime = FileTimeToDouble(ftUser);
			nCPUUseRate= (int)(100.0 - (fCPUIdleTime - fOldCPUIdleTime) 
				/ (fCPUKernelTime - fOldCPUKernelTime + fCPUUserTime - fOldCPUUserTime) 
				*100.0);
		}
	}	
	return nCPUUseRate;
}

double CBEOPWatchDlg::FileTimeToDouble( FILETIME &filetime )
{
	return (double)(filetime.dwHighDateTime * 4.294967296E9) + (double)filetime.dwLowDateTime;
}

int CBEOPWatchDlg::GetMemoryPervent()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	return statex.dwMemoryLoad;
}

bool CBEOPWatchDlg::AckCoreVersion()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		wstring strVersion =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"version");

		SYSTEMTIME st;
		GetLocalTime(&st);
		if(m_bdtuSuccess)
		{
			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 3;
			sDTUInfo.nSubType = 10;
			sDTUInfo.strSendBuf = "";
			sDTUInfo.strPacketIndex = Project::Tools::WideCharToAnsi(strVersion.c_str());
			if(!m_bVecCopying)
				m_vecSendBuffer.push_back(sDTUInfo);
			SetSendDTUEventAndType();
		}
	}
	return true;
}

bool CBEOPWatchDlg::AckCoreErrCode( string nMintue )
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		// 以|划分
		string strErrCodes =  m_pDataHandle->ReadErrCodeRecentMinute(nMintue);

		SYSTEMTIME st;
		GetLocalTime(&st);
		if(m_bdtuSuccess)
		{
			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 3;
			sDTUInfo.nSubType = 11;
			sDTUInfo.strSendBuf = "";
			sDTUInfo.strPacketIndex = strErrCodes;
			if(!m_bVecCopying)
				m_vecSendBuffer.push_back(sDTUInfo);
			SetSendDTUEventAndType();
		}
	}
	return true;
}

bool CBEOPWatchDlg::UpdateBat()
{
	bool bResult = false;
	wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
	GetSysPath(strFolder);
	strFolder += L"\\fileupdate";
	if(FindOrCreateFolder(strFolder))
	{
		strUpdateFolder = strFolder + L"\\update";
		strBacFolder = strFolder + L"\\back";

		if(m_pDataHandle && m_pDataHandle->IsConnected())
		{
			strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
			if(strS3dbPath.length() <= 0)
				return false;

			CString strFolderPath = strS3dbPath.c_str();
			strFolderPath = strFolderPath.Left(strFolderPath.ReverseFind('\\'));
			if(FindOrCreateFolder(strUpdateFolder))
			{
				//再更新该备份s3db
				vector<CString> vecFile;
				FindFile(strUpdateFolder.c_str(),_T("bat"),vecFile);
				if(vecFile.size()>0)
				{
					//使用.csv更新备份s3db
					CString strBatPath,strBatBackPath;
					strBatPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);
					m_strUpdateBatFile = vecFile[0];
					SHELLEXECUTEINFO ShExecInfo = {0};
					ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
					ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					ShExecInfo.hwnd = NULL;
					ShExecInfo.lpVerb = NULL;
					ShExecInfo.lpFile = strBatPath;	
					ShExecInfo.lpParameters = _T("");	
					ShExecInfo.lpDirectory = _T("");
					ShExecInfo.nShow = SW_HIDE;
					ShExecInfo.hInstApp = NULL;

					int nResult = ShellExecuteEx(&ShExecInfo);
					WaitForSingleObject(ShExecInfo.hProcess, INFINITE); 
					DWORD dwExitCode;
					bResult = GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);

					SYSTEMTIME sNow;
					GetLocalTime(&sNow);
					CString strTime;
					strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);
				
					CString strBatName = vecFile[0];
					CString strName = strBatName.Left(strBatName.ReverseFind('.'));
					CString strPriName = strBatName.Right(strBatName.GetLength() - strBatName.ReverseFind('.') - 1);
					//删除改.csv
					strBatBackPath.Format(_T("%s\\%s_%s.%s"),strBacFolder.c_str(),strName,strTime,strPriName);
					DelteFile(strBatPath,strBatBackPath);
				}
			}
			m_pDataHandle->WriteCoreDebugItemValue(L"updatebat",L"0");
		}
	}
	return bResult;
}

bool CBEOPWatchDlg::WriteActiveTime()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		SYSTEMTIME sNow;
		GetLocalTime(&sNow);
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		return m_pDataHandle->WriteCoreDebugItemValue(L"watchtime",strTime);
	}
	return false;
}

bool CBEOPWatchDlg::AckLocalPoint()
{
	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\fileupdate";	 
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		wstring strLocalZip;
		if(GenerateLocalPointCSV(strPath,strLocalZip))
		{
			return true;
		}
	}
	return false;
}

bool CBEOPWatchDlg::GenerateLocalPointCSV( wstring strFolder,wstring& strLocalZip )
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		wstring strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
		if(strS3dbPath.length() <= 0)
			return false;

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
		vector<DataPointEntry> vecPoint;
		ReadDataPoint(strPath,vecPoint);

		wstring strCSVPath;
		CString strZipPath;
		SYSTEMTIME st;
		GetLocalTime(&st);
		CString strFileName;
		strFileName.Format(_T("local_point_%d%02d%02d"),st.wYear,st.wMonth,st.wDay);
		strZipPath.Format(_T("%s\\%s.zip"),strFolder.c_str(),strFileName);
		if(GeneratePointCSV(vecPoint,strFolder,strFileName,strCSVPath))
		{	
			HZIP hz = CreateZip(strZipPath,NULL);
			ZipAdd(hz,strFileName,strCSVPath.c_str());
			CloseZip(hz);
			DelteFoder(strCSVPath.c_str());
			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 6;
			sDTUInfo.nSubType = 1;
			string strZipPath_Ansi = Project::Tools::WideCharToAnsi(strZipPath);
			sDTUInfo.strSendBuf = strZipPath_Ansi;
			if(m_bdtuSuccess)
			{
				if(!m_bVecCopying)
					m_vecSendBuffer.push_back(sDTUInfo);
				SetSendDTUEventAndType();
			}
		}
	}
	return true;
}

bool CBEOPWatchDlg::ChangeIconByDTUState()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		wstring wstrDTUSendTime = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"dtusendtime",L"");
		int nDTUState = 0;
		if(wstrDTUSendTime.length() > 0)
		{
			COleDateTime oleSendTime;
			Project::Tools::String_To_OleTime(wstrDTUSendTime,oleSendTime);
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - oleSendTime;
			if(oleSpan.GetTotalSeconds() <= 90)
			{
				nDTUState = 1;
			}
		}
		if(m_nDTUState != nDTUState)			//状态不一致时候
		{
			m_nDTUState = nDTUState;
			if(m_nDTUState == 1)
			{
				ChangeIcon(true);
			}
			else
			{
				ChangeIcon(false);
			}
			return true;
		}
	}
	return false;
}

bool CBEOPWatchDlg::ChangeIcon( bool bOK )
{
	Shell_NotifyIcon(NIM_DELETE, &m_nid);//在托盘区添加图标
	if(bOK)
	{
		m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		SetIcon(m_hIcon, TRUE);			// Set big icon
		SetIcon(m_hIcon, FALSE);		// Set small icon
		m_nid.uID = IDR_MAINFRAME;
		m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));//加载图标
	}
	else
	{
		m_hIcon  = AfxGetApp()->LoadIcon(IDR_ICON2);
		SetIcon(m_hIcon, TRUE);			// Set big icon
		SetIcon(m_hIcon, FALSE);		// Set small icon
		m_nid.uID = IDR_ICON2;
		m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_ICON2));//加载图标
	}
	Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标
	return true;
}

bool CBEOPWatchDlg::SendWatchStart()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	if(m_pdtusender && m_bdtuSuccess)
	{
		DTUSendInfo sDTUInfo;
		sDTUInfo.tSendTime = st;
		sDTUInfo.nType = 3;
		sDTUInfo.nSubType = 7;
		sDTUInfo.strSendBuf = "";
		if(!m_bVecCopying)
			m_vecSendBuffer.push_back(sDTUInfo);
		SetSendDTUEventAndType();
		return true;
	} 
	return false;
}

bool CBEOPWatchDlg::ReadDataPoint( string strS3dbPath,vector<DataPointEntry> &vecPoint )
{
	vecPoint.clear();
	int tempInt = 0;
	DataPointEntry entry;
	CString cstr;
	const char* pchar = NULL;
	CString strParam3;
	CString strSourceType;
	int i = 1;
	CSqliteAcess access(strS3dbPath.c_str());
	ostringstream sqlstream;
	sqlstream << "select * from list_point order by id;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	int nReadWrite = 0;
	const char szFlag[MAX_PATH] = {'\'', '"', ' ', '"', '\''};
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		entry.Clear();  //reset entry
		//OPC id
		int pointindex = access.getColumn_Int(0);
		if (pointindex == 0)
		{
			entry.SetPointIndex(i++);
		}
		else
		{
			entry.SetPointIndex(pointindex);
		}
		// get shortname
		if (access.getColumn_Text(1) !=NULL)
		{
			string   Physicalid_temp(const_cast<char*>(access.getColumn_Text(1)));
			wstring strBuf = Project::Tools::AnsiToWideChar(Physicalid_temp.c_str());
			entry.SetShortName(strBuf);
		}
		//get point source type
		if (access.getColumn_Text(24) !=NULL)
		{
			string   st_temp(const_cast<char*>(access.getColumn_Text(24)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp.c_str());
			entry.SetSourceType(strBuf);
		}

		//plcid name
		if (access.getColumn_Text(14) !=NULL)
		{
			string   st_temp1(const_cast<char*>(access.getColumn_Text(14)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp1.c_str());
			entry.SetParam(1, strBuf) ;
		}

		//param2
		if (access.getColumn_Text(15) !=NULL)
		{
			string   st_temp2(const_cast<char*>(access.getColumn_Text(15)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp2.c_str());
			entry.SetParam(2,strBuf) ;
		}

		//param3
		if (access.getColumn_Text(16) != NULL)
		{
			string   st_temp3(const_cast<char*>(access.getColumn_Text(16)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp3.c_str());
			entry.SetParam(3, strBuf) ;
		}

		//param4
		if (access.getColumn_Text(17) != NULL)
		{
			string   st_temp4(const_cast<char*>(access.getColumn_Text(17)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp4.c_str());
			entry.SetParam(4, strBuf) ;
		}

		//param5
		if (access.getColumn_Text(18) !=NULL)
		{
			string   st_temp5(const_cast<char*>(access.getColumn_Text(18)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp5.c_str());
			entry.SetParam(5, strBuf) ;
		}

		//param6
		if (access.getColumn_Text(19) != NULL)
		{
			string   st_temp6(const_cast<char*>(access.getColumn_Text(19)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp6.c_str());
			entry.SetParam(6, strBuf) ;
		}

		//param7
		if (access.getColumn_Text(20) != NULL)
		{
			string   st_temp7(const_cast<char*>(access.getColumn_Text(20)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp7.c_str());
			entry.SetParam(7, strBuf) ;
		}

		//param8
		if (access.getColumn_Text(21) != NULL)
		{
			string   st_temp8(const_cast<char*>(access.getColumn_Text(21)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp8.c_str());
			entry.SetParam(8, strBuf) ;
		}

		//param9
		if (access.getColumn_Text(22) !=NULL)
		{
			string   st_temp9(const_cast<char*>(access.getColumn_Text(22)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp9.c_str());
			entry.SetParam(9, strBuf) ;
		}

		//param10
		if (access.getColumn_Text(23) != NULL)
		{
			string   st_temp10(const_cast<char*>(access.getColumn_Text(23)));
			wstring strBuf = Project::Tools::AnsiToWideChar(st_temp10.c_str());
			entry.SetParam(10,strBuf) ;
		}

		//read write
		nReadWrite = access.getColumn_Int(10);
		entry.SetProperty((PLCVALUEPROPERTY)nReadWrite);
		//remark
		if (access.getColumn_Text(8) != NULL)
		{
			string   st_remarkp(const_cast<char*>(access.getColumn_Text(8)));

			wstring strBuf = Project::Tools::AnsiToWideChar(st_remarkp.c_str());
			entry.SetDescription(strBuf);
		}

		//unit
		pchar = access.getColumn_Text(7);
		string str;
		if(pchar != NULL)
		{
			str = pchar;  
		}
		else
		{
			str = "";            
		}

		entry.SetUnit(Project::Tools::AnsiToWideChar(str.c_str()));
		//high
		if (access.getColumn_Text(25) != NULL)
		{
			string   st_high(const_cast<char*>(access.getColumn_Text(25)));
			str = st_high;
			if (str.size()>0)
			{
				entry.SetHighAlarm(atof(str.c_str()));
			}
		}
		//highhigh -- maxValue
		if (access.getColumn_Text(26) != NULL)
		{
			string   strMaxValue(const_cast<char*>(access.getColumn_Text(26)));
			entry.SetParam(12, Project::Tools::AnsiToWideChar(strMaxValue.c_str()));
		}
		//low -- minValue
		if (access.getColumn_Text(27) != NULL)
		{
			string   strMinValue(const_cast<char*>(access.getColumn_Text(27)));
			entry.SetParam(13, Project::Tools::AnsiToWideChar(strMinValue.c_str()));
		}
		//lowlow -- isVisited
		if (access.getColumn_Text(28) != NULL)
		{
			string   strIsVisited(const_cast<char*>(access.getColumn_Text(28)));
			int nIsVisited = atoi(strIsVisited.c_str());
			stringstream ss;
			ss << nIsVisited;
			ss >> str;
			entry.SetParam(14, Project::Tools::AnsiToWideChar(str.c_str()));
		}
		if (access.getColumn_Text(29))
		{
			POINT_STORE_CYCLE cycle = (POINT_STORE_CYCLE)atoi(access.getColumn_Text(29));
			entry.SetStoreCycle(cycle);
		}
		//
		const char* pParam15 = access.getColumn_Text(33);
		if (pParam15 != NULL)
		{
			string strParam15;
			if (0 == strcmp(szFlag, pParam15))
			{
				strParam15 = "";
			}
			else
			{
				strParam15 = const_cast<char*>(pParam15);
			}
			entry.SetParam(15, Project::Tools::AnsiToWideChar(strParam15.c_str()));
		}
		const char* pParam16 = access.getColumn_Text(34);
		if (pParam16 != NULL)
		{
			int nNum = atoi(pParam16);
			entry.SetParam(16, g_strPointAttrSystem[nNum].GetString());
		}
		const char* pParam17 = access.getColumn_Text(35);
		if (pParam17 != NULL)
		{
			int nNum = atoi(pParam17);
			entry.SetParam(17, g_strPointAttrDevice[nNum].GetString());
		}
		const char* pParam18 = access.getColumn_Text(36);
		if (pParam18 != NULL)
		{
			int nNum = atoi(pParam18);
			entry.SetParam(18, g_strPointAttrType[nNum].GetString());
		}

		entry.SetUnit(Project::Tools::AnsiToWideChar(str.c_str()));
		vecPoint.push_back(entry);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	return true;
}

bool CBEOPWatchDlg::GeneratePointCSV( vector<DataPointEntry> &vecPoint,wstring strFolder,CString& strCSVName,wstring & strCSVPath )
{
	CString strFilePath,strCSVName_;
	strCSVName_.Format(_T("%s.csv"),strCSVName);
	strFilePath.Format(_T("%s\\%s"),strFolder.c_str(),strCSVName_);
	char* old_locale=_strdup( setlocale(LC_CTYPE,NULL) ); 
	setlocale( LC_CTYPE,"chs");

	CStdioFile file;
	if(file.Open(strFilePath, CFile::modeCreate|CFile::modeWrite, NULL,NULL))
	{
		CString str = _T("pointindex,physicalid,source,remark,Unit,RWProperty,param1,param2,param3,param4,param5,param6,param7,param8,param9,param10,param11,param12,param13,param14,storecycle,customName,system,device,type\n");
		file.Seek(0,CFile::begin);
		file.WriteString(str);
		for(int i=0; i<vecPoint.size(); ++i)
		{
			DataPointEntry entry = vecPoint[i];
			string strProperty;
			if(entry.GetProperty() == PLC_READ)
			{
				strProperty = "R";
			}
			else if(entry.GetProperty() == PLC_WRITE)
			{
				strProperty = "W";
			}
			else if(entry.GetProperty() == PLC_PROP_MAX)
			{
				strProperty = "R/W";
			}
			std::ostringstream sqlstream_point;
			sqlstream_point << entry.GetPointIndex() << "," << Project::Tools::WideCharToAnsi(entry.GetShortName().c_str()) << ","  << Project::Tools::WideCharToAnsi(entry.GetSourceType().c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetDescription().c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetUnit().c_str()) << "," << strProperty << "," << Project::Tools::WideCharToAnsi(entry.GetParam(1).c_str()) << "," << Project::Tools::WideCharToAnsi(entry.GetParam(2).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(3).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(4).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(5).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(6).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(7).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(8).c_str()) 
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(9).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(10).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(11).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(12).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(13).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(14).c_str())  << "," << (int)entry.GetStoreCycle()  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(15).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(16).c_str())  << "," << Project::Tools::WideCharToAnsi(entry.GetParam(17).c_str())
				<< "," << Project::Tools::WideCharToAnsi(entry.GetParam(18).c_str()) << "\n";
			str = Project::Tools::AnsiToWideChar(sqlstream_point.str().c_str()).c_str();
			file.WriteString(str);
		}
		file.Close();
	}
	setlocale( LC_CTYPE, old_locale );
	free( old_locale );
	strCSVPath = strFilePath;
	strCSVName = strCSVName_;
	return true;
}

void CBEOPWatchDlg::OnBnClickedCheckTcpConnection()
{
	// TODO: Add your control notification handler code here
	bool bCheck = ((CButton*)GetDlgItem(IDC_CHECK_TCP_CONNECTION))->GetCheck();
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		if(bCheck)
		{
			m_pDataHandle->WriteCoreDebugItemValue(L"TcpSenderEnabled", L"1");
			InitTCPSender();
			////启用时候需要设置默认参数
			//m_strTCPName = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderName");		//读TCP Name  用于标识项目
			//((CEdit*)GetDlgItem(IDC_STATIC_NAME))->SetWindowText(m_strTCPName.c_str());
			//if(m_strTCPName == L"" || m_strTCPName == L"0")
			//{
			//	m_strTCPName = L"default2015";
			//	CToolsFunction toolsFunction;
			//	IP_ADAPTER_INFO* pIpAdapterInfo = toolsFunction.GetAllLocalMachineEthInfo();
			//	if(pIpAdapterInfo != NULL)
			//	{
			//		if (pIpAdapterInfo)
			//		{
			//			BYTE s[8]= {0};
			//			memcpy(s,pIpAdapterInfo->Address,6);  
			//			CString strMac;
			//			strMac.Format(_T("%02X%02X%02X%02X%02X%02X"),s[0],s[1],s[2],s[3],s[4],s[5]);
			//			m_strTCPName = strMac.GetString();
			//		}
			//	}
			//	//TcpSenderName storehistory
			//	m_pDataHandle->WriteCoreDebugItemValue(L"TcpSenderPort",m_strTCPPort);
			//	m_pDataHandle->WriteCoreDebugItemValue(L"TcpSenderName",m_strTCPName);
			//	m_pDataHandle->WriteCoreDebugItemValue(L"storehistory",L"0");
			//	ResetTCPInfo();
			//}

			CString strInfo;
			strInfo.Format(_T("Set Enable DTU Success,Do you want to automatic restart Core(ProjectID:%s)?"),m_strTCPName.c_str());
			//确认对话框
			int nResult = ::MessageBox(NULL,strInfo,TEXT("Warning"), MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST);
			if (nResult == IDYES)
			{
				m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"11");
			}
		}
		else
		{
			m_pDataHandle->WriteCoreDebugItemValue(L"TcpSenderEnabled", L"0");
			ExitCPSender();

			CString strInfo;
			strInfo.Format(_T("Set Disable DTU Success,Do you want to automatic restart Core(ProjectID:%s)?"),m_strTCPName.c_str());
			//确认对话框
			int nResult = ::MessageBox(NULL,strInfo,TEXT("Warning"), MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST);
			if (nResult == IDYES)
			{
				m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"11");
			}
		}
	}
}

bool CBEOPWatchDlg::UpdateDTUInfo()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		bool bWithTcpSender = (m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderEnabled"))==L"1";
		((CButton*)GetDlgItem(IDC_CHECK_TCP_CONNECTION))->SetCheck(bWithTcpSender);
		m_strTCPName = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderName");		//读TCP Name  用于标识项目
		if(m_pdtusender)
		{
			m_pdtusender->SetName(Project::Tools::WideCharToAnsi(m_strTCPName.c_str()));
		}
		((CEdit*)GetDlgItem(IDC_STATIC_NAME))->SetWindowText(m_strTCPName.c_str());
	}
	return true;
}

bool CBEOPWatchDlg::ChangeUnitsByDTUServer( string strReceive )
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		vector<wstring> vecCmd;
		Project::Tools::SplitStringByChar(Project::Tools::AnsiToWideChar(strReceive.c_str()).c_str(),L',',vecCmd);

		int nPointSize = vecCmd.size()/2;
		if(nPointSize<=0)
			return false;

		bool bRestart = false;
		for (unsigned int i = 0; i < nPointSize; i++)
		{
			wstring strName = vecCmd[2*i];		 
			wstring strValue = vecCmd[2*i+1];
			m_pDataHandle->WriteCoreDebugItemValue(strName,strValue);

			if(strName == L"restartwatch" && strValue == L"1")
			{
				bRestart = true;
			}
		}

		if(bRestart)
		{
			ResetTCPInfo();
		}

		SYSTEMTIME st;
		GetLocalTime(&st);
		if(m_bdtuSuccess)
		{
			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 3;
			sDTUInfo.nSubType = 2;
			sDTUInfo.strSendBuf = "";
			sDTUInfo.strPacketIndex = "11";
			if(!m_bVecCopying)
				m_vecSendBuffer.push_back(sDTUInfo);
			SetSendDTUEventAndType();
		}
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::ResetTCPInfo()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		if(m_pdtusender == NULL)
		{
			InitTCPSender();
		}
		else
		{
			if(m_pdtusender)
			{
				wstring wstrTcpSenderIP = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderIP",L"");		//读TCP IP   默认为114.215.172.232
				if(wstrTcpSenderIP == L"" || wstrTcpSenderIP == L"0")
					wstrTcpSenderIP = L"";
				wstring wstrTcpSenderport = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderPort");		//读TCP端口   默认为9501
				if(wstrTcpSenderport == L"" || wstrTcpSenderport == L"0")
					wstrTcpSenderport = L"9500";
				m_strTCPName = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"TcpSenderName");		//读TCP Name  用于标识项目
				m_pdtusender->ResetTcpInfo(Project::Tools::WideCharToAnsi(wstrTcpSenderIP.c_str()),_wtoi(wstrTcpSenderport.c_str())+1,Project::Tools::WideCharToAnsi(m_strTCPName.c_str()));
			}
		}
		m_pDataHandle->WriteCoreDebugItemValue(L"restartwatch",L"0");
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::AckErrFileList()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	//发送DTU数据 
	wstring strFolder;
	GetSysPath(strFolder);
	strFolder += L"\\log";	 
	if(Project::Tools::FindOrCreateFolder(strFolder))
	{
		CString strFilePath;
		strFilePath.Format(_T("%s\\*.log"),strFolder.c_str());
		std::ostringstream sqlstream_file;
		CFileFind fileFinder;
		BOOL bWorking = fileFinder.FindFile(strFilePath);
		bool bHasLogFile = false;
		while (bWorking)
		{   
			bWorking = fileFinder.FindNextFile();
			if (fileFinder.IsDots())
			{
				continue;
			}
			CString strFileName = fileFinder.GetFileName();
			CString strFilePath = fileFinder.GetFilePath();

			CFileStatus rStatus;
			CFile::GetStatus(strFilePath,rStatus);
			if(strFileName.GetLength() > 0)
				sqlstream_file << Project::Tools::WideCharToAnsi(strFileName) << "|" << rStatus.m_size  << ",";
		}
		fileFinder.Close();

		if(m_bdtuSuccess)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			DTUSendInfo sDTUInfo;
			sDTUInfo.tSendTime = st;
			sDTUInfo.nType = 3;
			sDTUInfo.nSubType = 12;
			sDTUInfo.strSendBuf = "";
			sDTUInfo.strPacketIndex = sqlstream_file.str();
			if(!m_bVecCopying)
				m_vecSendBuffer.push_back(sDTUInfo);
			SetSendDTUEventAndType();
		}
	}
	return false;
}

bool CBEOPWatchDlg::AckErrFileByName( string strErrName )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_dtu_ack_lock);
	//发送DTU数据 
	wstring strFolder;
	GetSysPath(strFolder);
	strFolder += L"\\log";	 
	if(Project::Tools::FindOrCreateFolder(strFolder))
	{
		std::ostringstream sqlstream_path;
		sqlstream_path << Project::Tools::WideCharToAnsi(strFolder.c_str()) << "\\" << strErrName;
		string strPath = sqlstream_path.str();
		wstring strFilePath = Project::Tools::AnsiToWideChar(strPath.c_str());
		if(Project::Tools::FindFileExist(strFilePath))
		{
			CString strZipPath;
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString strFileName = Project::Tools::AnsiToWideChar(strErrName.c_str()).c_str();
			strFileName = strFileName.Left(strFileName.ReverseFind('.'));
			strZipPath.Format(_T("%s\\%s.zip"),strFolder.c_str(),strFileName);

			HZIP hz = CreateZip(strZipPath,NULL);
			ZipAdd(hz,strFileName,strFilePath.c_str());
			CloseZip(hz);

			if(m_bdtuSuccess)
			{
				DTUSendInfo sDTUInfo;
				sDTUInfo.tSendTime = st;
				sDTUInfo.nType = 6;
				sDTUInfo.nSubType = 1;
				string strZipPath_Ansi = Project::Tools::WideCharToAnsi(strZipPath);
				sDTUInfo.strSendBuf = strZipPath_Ansi;

				if(!m_bVecCopying)
					m_vecSendBuffer.push_back(sDTUInfo);
				SetSendDTUEventAndType();
			}
		}
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::FindAndDeleteErrFile( CString strDir,CString strDeadFileName )
{
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\*.*"),strDir);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strDirName = tempFind.GetFileName();
				CString strDirPath = tempFind.GetFilePath();
				if(strDirName == strDeadFileName)
				{
					DelteFoder(strDirPath);
				}
			}
		}
	}
	tempFind.Close();
	return true;
}

bool CBEOPWatchDlg::FindAndDeleteErrFile( CString strDir,CString strFilterName,CString strFilterPri,CString strDeadFileName )
{
	CFileFind tempFind;
	CString strFind;
	strFind.Format(_T("%s\\%s*%s"),strDir,strFilterName,strFilterPri);
	BOOL IsFinded = tempFind.FindFile(strFind);
	CString strBackupPath;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();
		if (!tempFind.IsDots()) 
		{
			if (!tempFind.IsDirectory())
			{
				CString strDirName = tempFind.GetFileName();
				CString strDirPath = tempFind.GetFilePath();
				if(strDirName <= strDeadFileName)
				{
					DelteFile(strDirPath);
				}
			}
		}
	}
	tempFind.Close();
	return true;
}

bool CBEOPWatchDlg::OpenDog()
{
	bool bResult = false;
	if (!m_PrsV.FindProcessByName_other(g_NAME_Dog))
	{
		wstring strPath;
		GetSysPath(strPath);
		CString strWatchPath;
		strWatchPath.Format(_T("%s\\%s"),strPath.c_str(),g_NAME_Dog);
		if(m_PrsV.OpenApp(strWatchPath))
		{
			bResult = true;
		}
		else
		{
			bResult = false;
		}
	}
	return bResult;
}

bool CBEOPWatchDlg::RestartCoreByDog( int nType,int& nErrorCode,bool bRestart_/*=false*/ )
{
	if(nType <= 4)
		SumRestartTime();

	CString strType;
	if(nType == 0)
	{
		strType = _T("(Manual)");
	}
	else if(nType == 1)
	{
		strType = _T("(Core not run)");
	}
	else if(nType == 2)
	{
		strType = _T("(servertime not update 10m)");
	}
	else if(nType == 3)
	{
		strType = _T("(s7udp not update 5m)");
	}
	else if(nType == 4)
	{
		strType = _T("(dtu not update 60m)");
	}
	else if(nType == 5)
	{
		strType = _T("(update point xls)");
	}
	else if(nType == 6)
	{
		strType = _T("(update exe)");
	}
	else if(nType == 7)
	{
		strType = _T("(update dll)");
	}
	else if(nType == 9)
	{
		strType = _T("(remote restart)");
	}
	else if(nType == 10)
	{
		strType = _T("(timing restart)");
	}
	else if(nType == 11)
	{
		strType = _T("(setting restart)");
	}
	else if(nType == 12)
	{
		strType = _T("(update s3db restart)");
	}
	else if(nType == 13)
	{
		strType = _T("(Manual Watch)");
	}

	bool bRestart = false;
	if (CDogTcpCtrl::GetInstance()->GetDogRestartFunctonOK())		//dog连接成功 且版本号支持
	{
		if(CDogTcpCtrl::GetInstance()->SendRestart(E_SLAVE_CORE,E_REASON_UPDATE))
		{
			//等待回应的机制 5秒
			int nWaitingCount = 50; 
			bool bRestartUpdate = false;
			bool bRestartCore = false;
			bool bRestartLogic = false;
			bool bRestartMaster = false;
			while(nWaitingCount>=0)
			{
				CDogTcpCtrl::GetInstance()->GetRestartSuccess(bRestartCore,bRestartLogic,bRestartUpdate,bRestartMaster);
				if(bRestartCore)
				{
					CDogTcpCtrl::GetInstance()->SetRestartCoreSuccess(false);
					bRestart = true;
					break;
				}
				Sleep(100);
				nWaitingCount--;
			}
		}
	}
	else
	{
		m_PrsV.CloseApp(g_NAME_Core);
		Sleep(1000);
		return m_PrsV.OpenApp(m_strPath);
	}
	
	if(bRestart)
	{
		CString str;
		str = g_NAME_Core + _T(" auto restart success") + strType;
		OutPutLogString(str);
	}
	else
	{
		CString str;
		str = g_NAME_Core + _T(" auto restart fail") + strType;
		OutPutLogString(str);
	}
	return bRestart;
}

bool CBEOPWatchDlg::TestNetWork()
{
	CPingIcmp ping1;
	bool ret = false;
	for (int count = 1; count <= 3; count++) 
	{
		ret = true;//ping1.PingServer("114.215.172.232");
		if (ret) {
			return true;
		}
		Sleep(500);
	}
	return false;
}

bool CBEOPWatchDlg::ImproveProcPriv()
{
	HANDLE token;
	//提升权限
	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&token))
	{
		return FALSE;
	}
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	::LookupPrivilegeValue(NULL,SE_SYSTEMTIME_NAME,&tkp.Privileges[0].Luid);
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if(!AdjustTokenPrivileges(token,FALSE,&tkp,sizeof(tkp),NULL,NULL))
	{
		return FALSE;
	}
	CloseHandle(token);
	return TRUE;
}

void CBEOPWatchDlg::OnCloseWatch()
{
	if(CloseWatch())
	{
		m_bCloseDlg = true;
		SendMessage(WM_CLOSE);
	}
}

bool CBEOPWatchDlg::CloseWatch()
{
	//
	Exit();
	return true;
}

bool CBEOPWatchDlg::OutputMemorySize()
{
	try
	{
		COleDateTime oleCurrentTime = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleSpan = oleCurrentTime - m_oleGetMemoryTime;
		if(oleSpan.GetTotalMinutes() >= m_nOutPutMemoryInterval)
		{
			wstring strFolder;
			GetSysPath(strFolder);
			strFolder += L"\\memory";
			if(FindOrCreateFolder(strFolder))
			{
				//获取内存大小
				float nLogicSize = m_PrsV.GetProessCPU(g_NAME_Logic);
				float nCoreSize = m_PrsV.GetProessCPU(g_NAME_Core);

				CString strLogicSize,strCoreSize;
				strLogicSize.Format(_T("%.2f"),nLogicSize);
				strCoreSize.Format(_T("%.2f"),nCoreSize);
				if(m_pDataHandle && m_pDataHandle->IsConnected())
				{
					m_pDataHandle->WriteCoreDebugItemValue(L"corememory",strCoreSize.GetString());
					m_pDataHandle->WriteCoreDebugItemValue(L"logicmemory",strLogicSize.GetString());
				}

				//记录Log
				CStdioFile	ff;
				CString strLogFilePath,strLog;
				strLogFilePath.Format(_T("%s\\memory_%04d_%02d.log"),strFolder.c_str(),oleCurrentTime.GetYear(),oleCurrentTime.GetMonth());
				if(ff.Open(strLogFilePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
				{
					wstring strTime;
					Project::Tools::OleTime_To_String(oleCurrentTime,strTime);
					strLog.Format(_T("%s => Core:%s, Logice:%s.\n"),strTime.c_str(),strCoreSize,strLogicSize);
					ff.Seek(0,CFile::end);
					ff.WriteString(strLog);
					ff.Close();
				}
			}
			m_oleGetMemoryTime = oleCurrentTime;
			return true;
		}
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

bool CBEOPWatchDlg::ScanAndGernerateLogFile()
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		vector<unitLogFileInfo> unitlist;
		if(m_pDataHandle->ReadLogFile_Unit06(unitlist))
		{
			for(int i=0; i<unitlist.size(); ++i)
			{
				ScanFile(unitlist[i]);
			}
			return true;
		}
	}
	return false;
}

bool CBEOPWatchDlg::ScanFile( unitLogFileInfo logFile )
{
	try
	{
		if(logFile.strStartTime.length() >0 && logFile.strEndTime.length()>0 && logFile.strEndTime >= logFile.strStartTime && logFile.strFileName.length() >0)
		{
			wstring strPath,strFolder;
			GetSysPath(strFolder);
			hash_map<wstring,wstring> mapLogFile;		//遍历所有log的文件名和地址
			if(logFile.nType == 0)					//err
			{
				strPath = strFolder;
				strPath += L"\\log";
				CFileFind tempFind;
				CString strFind;
				strFind.Format(_T("%s\\*.log"),strPath.c_str());
				BOOL IsFinded = tempFind.FindFile(strFind);
				CString strBackupPath;
				while (IsFinded)
				{
					IsFinded = tempFind.FindNextFile();
					if (!tempFind.IsDots()) 
					{
						if(!tempFind.IsDirectory())
						{
							CString strDirName = tempFind.GetFileName();
							CString strDirPath = tempFind.GetFilePath();
							mapLogFile[strDirName.GetString()] = strDirPath.GetString();
						}
					}
				}
				tempFind.Close();
			}
			else if(logFile.nType == 1)				//log
			{
				strPath = strFolder;
				strPath += L"\\log";
				CFileFind tempFind;
				CString strFind;
				strFind.Format(_T("%s\\*.log"),strPath.c_str());
				BOOL IsFinded = tempFind.FindFile(strFind);
				CString strBackupPath;
				while (IsFinded)
				{
					IsFinded = tempFind.FindNextFile();
					if (!tempFind.IsDots()) 
					{
						if(!tempFind.IsDirectory())
						{
							CString strDirName = tempFind.GetFileName();
							CString strDirPath = tempFind.GetFilePath();
							mapLogFile[strDirName.GetString()] = strDirPath.GetString();
						}
					}
				}
				tempFind.Close();
			}
			else if(logFile.nType == 2)				//both
			{
				strPath = strFolder;
				strPath += L"\\log";
				CFileFind tempFind;
				CString strFind;
				strFind.Format(_T("%s\\*.log"),strPath.c_str());
				BOOL IsFinded = tempFind.FindFile(strFind);
				CString strBackupPath;
				while (IsFinded)
				{
					IsFinded = tempFind.FindNextFile();
					if (!tempFind.IsDots()) 
					{
						if(!tempFind.IsDirectory())
						{
							CString strDirName = tempFind.GetFileName();
							CString strDirPath = tempFind.GetFilePath();
							mapLogFile[strDirName.GetString()] = strDirPath.GetString();
						}
					}
				}
				tempFind.Close();

				strPath = strFolder;
				strPath += L"\\log";
				strFind.Format(_T("%s\\*.log"),strPath.c_str());
				IsFinded = tempFind.FindFile(strFind);
				while (IsFinded)
				{
					IsFinded = tempFind.FindNextFile();
					if (!tempFind.IsDots()) 
					{
						if(!tempFind.IsDirectory())
						{
							CString strDirName = tempFind.GetFileName();
							CString strDirPath = tempFind.GetFilePath();
							mapLogFile[strDirName.GetString()] = strDirPath.GetString();
						}
					}
				}
				tempFind.Close();
			}

			//根据筛选条件遍历
			hash_map<wstring,wstring> mapLogFilterFile;		//遍历所有log的文件名和地址
			if(logFile.strFilter.length() >0)
			{
				vector<wstring> vecFilter;
				Project::Tools::SplitStringByChar(logFile.strFilter.c_str(),L',',vecFilter);
				hash_map<wstring,wstring>::iterator iterFile = mapLogFile.begin();
				while(iterFile != mapLogFile.end())
				{
					CString strFileName = (*iterFile).first.c_str();
					bool bFind = false;
					for(int i=0; i<vecFilter.size(); ++i)
					{
						if(strFileName.Find(vecFilter[i].c_str()) >= 0)
						{
							bFind = true;
							break;
						}
					}
					if(bFind)
					{
						mapLogFilterFile[(*iterFile).first] = (*iterFile).second;
					}
					++iterFile;
				}
			}
			else
			{
				mapLogFilterFile = mapLogFile;
			}

			//根据时间遍历
			hash_map<wstring,wstring> mapLogTimeFile;		//遍历所有log的文件名和地址
			if(logFile.strStartTime.length() >0 && logFile.strEndTime.length() >0)
			{
				COleDateTime oleStart,oleEnd;
				Project::Tools::String_To_OleTime(logFile.strStartTime,oleStart);
				Project::Tools::String_To_OleTime(logFile.strEndTime,oleEnd);
				vector<wstring> vecTimeFilter;
				for(oleStart;oleStart<=oleEnd;oleStart+=COleDateTimeSpan(1,0,0,0))
				{
					CString strTime;
					strTime.Format(_T("_%d_%02d_%02d"),oleStart.GetYear(),oleStart.GetMonth(),oleStart.GetDay());
					vecTimeFilter.push_back(strTime.GetString());
				}

				hash_map<wstring,wstring>::iterator iterFile = mapLogFilterFile.begin();
				while(iterFile != mapLogFilterFile.end())
				{
					CString strFileName = (*iterFile).first.c_str();
					bool bFind = false;
					for(int i=0; i<vecTimeFilter.size(); ++i)
					{
						if(strFileName.Find(vecTimeFilter[i].c_str()) >= 0)
						{
							bFind = true;
							break;
						}
					}
					if(bFind)
					{
						mapLogTimeFile[(*iterFile).first] = (*iterFile).second;
					}
					++iterFile;
				}
			}
			
			//打包
			CString strZipPath;
			strZipPath.Format(_T("%s\\%s"),strFolder.c_str(),logFile.strFileName.c_str());
			HZIP hz = CreateZip(strZipPath,NULL);
			hash_map<wstring,wstring>::iterator iterZipFile = mapLogTimeFile.begin();
			while(iterZipFile != mapLogTimeFile.end())
			{
				ZipAdd(hz,(*iterZipFile).first.c_str(),(*iterZipFile).second.c_str());
				++iterZipFile;
			}
			CloseZip(hz);

			//存入到filestore
			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				m_pDataHandle->UpdateLibInsertIntoFilestorage(strZipPath.GetString(),logFile.strFileName,logFile.strFileName);
				m_pDataHandle->UpdateLogFile_Unit06(logFile);
				DeleteFile(strZipPath);
				return true;
			}
		}
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

unsigned int WINAPI CBEOPWatchDlg::CreateTCPConnection( LPVOID lpVoid )
{
	CBEOPWatchDlg* pCreateTCPConnection = reinterpret_cast<CBEOPWatchDlg*>(lpVoid);
	if (NULL == pCreateTCPConnection)
	{
		return 0;
	}
	return pCreateTCPConnection->InitTCPSender();
}

bool CBEOPWatchDlg::ShowDTURouterStateInfo()
{
	wstring wstrDTURouterIP = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"DTURouterIP");
	if(_wtoi(wstrDTURouterIP.c_str()) != 0)
	{
		COleDateTime oleNow = COleDateTime::GetCurrentTime();
		if(oleNow.GetHour() != m_oleShowRouterState.GetHour())
		{
			//记录信息
			m_oleShowRouterState = oleNow;
			return ShowDTURouterState(Project::Tools::WideCharToAnsi(wstrDTURouterIP.c_str()));
		}

		if(m_pDataHandle && m_pDataHandle->IsConnected())
		{
			wstring wstrRunDtu = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"rundtu");
			if(_wtoi(wstrRunDtu.c_str()) == 1)
			{
				wstring wstrDTUSendTime = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"dtusendtime");
				COleDateTime oleSendTime;
				Project::Tools::String_To_OleTime(wstrDTUSendTime,oleSendTime);
				COleDateTimeSpan oleSpan_ = oleNow - oleSendTime;
				COleDateTimeSpan oleSpan1 = oleNow - m_oleShowRouterState;
				if(abs(oleSpan_.GetTotalMinutes())>=30 && abs(oleSpan1.GetTotalMinutes())>=30)
				{
					//输出路由器信息
					m_oleShowRouterState = oleNow;
					return ShowDTURouterState(Project::Tools::WideCharToAnsi(wstrDTURouterIP.c_str()));
				}
			}
		}
	}
	return false;
}

bool CBEOPWatchDlg::ShowDTURouterState(string strRouterIP)
{
	try
	{
		CTelnetTcpCtrl pTelnet;
		if(pTelnet.Init(strRouterIP))
		{
			int nWait = 60;
			while(nWait>=0)
			{
				if(pTelnet.GetSaveFlag())
					return true;
				nWait--;
				Sleep(1000);
			}
		}
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

bool CBEOPWatchDlg::StoreDllToDB(CString strDllPath,CString strS3dbPath,CString strDllName,CString strThreadName,CString strVersion,CString strDescription,CString strAuthor)
{
	try
	{
		FILE *fp;
		long filesize = 0;
		char * ffile;
		_bstr_t tem_des = strDllPath;

		char* des = (char*)tem_des;
		fopen_s(&fp,des, "rb");
		if(fp != NULL)
		{
			//计算文件的大小
			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			//读取文件
			ffile = new char[filesize+1];
			size_t sz = fread(ffile, sizeof(char), filesize, fp);
			fclose(fp);
		}

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			char szSQL[1024] = {0};
			memset(szSQL, 0, sizeof(szSQL));
			int rc = 0;
			int nID = 1;

			SYSTEMTIME sysTime;
			CString importtime;
			::GetLocalTime(&sysTime);
			importtime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), sysTime.wYear, sysTime.wMonth,sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	
			_bstr_t tem_des = importtime;
			char* des = (char*)tem_des;
			std::string importtime__(des);

			tem_des = strDllName;
			des = (char*)tem_des;
			std::string dllname__(des);

			tem_des = strAuthor;
			des = (char*)tem_des;
			std::string author__(des);

			//dll版本
			tem_des = strVersion;
			des = (char*)tem_des;
			std::string version__(des);

			//dll描述
			tem_des = strDescription;
			des = (char*)tem_des;
			std::string description__(des);

			//////////////////////////////////////////////////////////////////////////
			//查找是否存在
			ostringstream sqlstream;
			sqlstream << "select * from list_dllstore;";
			string sql_ = sqlstream.str();
			char *ex_sql = const_cast<char*>(sql_.c_str());
			int re = access.SqlQuery(ex_sql);

			//
			int timespan = 20;
			while(true)
			{
				if(SQLITE_ROW != access.SqlNext())
				{
					break;
				}
				timespan = access.getColumn_Int(4);
			}
			access.SqlFinalize();

			//
			tem_des = strThreadName;
			des = (char*)tem_des;
			std::string threadname__(des);

			//删除已有的同名dll
			string sql_del;
			ostringstream sqlstream_del;
			sqlstream_del << "delete from list_dllstore where DllName = "<< "'" << dllname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}
			//////////////////////////////////////////////////////////////////////////
			int runstatus = 1;
			sqlstream.str("");
			sqlstream << "insert into list_dllstore(id,DllName,importtime,author,periodicity,dllcontent,runstatus,unitproperty01,unitproperty02,unitproperty03,unitproperty04,unitproperty05)  values ('" << nID<< "','" << dllname__ << "','" \
				<< importtime__ << "','" << author__ << "','" << timespan << "',?,'1','"<<version__ <<"','"<<description__<<"','"<<dllname__<<"','"<<threadname__<<"','');";
			sql_ = sqlstream.str();

			bool bInsertOK = false;
			if (SQLITE_OK == access.SqlQuery(sql_.c_str()))
			{
				if (access.m_stmt)
				{
					rc = access.SqlBindBlob(ffile,1,filesize);
					rc = access.SqlNext();
					rc = access.SqlFinalize();
				}
			}
			access.CloseDataBase();
		}
		
		if (ffile != NULL)
		{
			delete [] ffile;
			ffile = NULL;
		}
		return true;
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

bool CBEOPWatchDlg::DeleteDll(CString strS3dbPath,CString strDllName,CString strThreadName)
{
	try
	{
		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			_bstr_t tem_des = strDllName;
			char* des = (char*)tem_des;
			std::string dllname__(des);

			tem_des = strThreadName;
			des = (char*)tem_des;
			std::string threadname__(des);

			//删除已有的同名dll
			string sql_del;
			ostringstream sqlstream_del;
			sqlstream_del << "delete from list_dllstore where DllName = "<< "'" << dllname__ << "' and unitproperty04='" << threadname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}

			//删除dll策略关系
			sqlstream_del.str("");
			sqlstream_del << "delete from list_paramterConfig where DllName = "<< "'" << dllname__ << "';"; 
			sql_del = sqlstream_del.str();
			if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
			{
				;
			}

			access.CloseDataBase();
		}
		return true;
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

bool CBEOPWatchDlg::UpdateDllParamter(CString strS3dbPath,wstring strParamSql)
{
	try
	{
		if(strParamSql.length() <= 0)
			return false;
		
		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			string sql = "";
			sql = Project::Tools::WideCharToAnsi(strParamSql.c_str());
			vector<string> vecParamSql;
			SplitStringSpecial(sql.c_str(),sql.length(),vecParamSql);
			int nResult = 0;
			if(vecParamSql.size()>0)
			{
				access.BeginTransaction();
				for(int i=0; i<vecParamSql.size(); ++i)
				{
					string sql_insert;
					ostringstream sqlstream_insert;
					sqlstream_insert << "insert into list_paramterConfig values " << vecParamSql[i] << ");";
					sql_insert = sqlstream_insert.str();
					nResult = access.SqlExe(sql_insert.c_str());
				}
				access.CommitTransaction();
			}
			access.CloseDataBase();
			if(nResult == 0)
				return true;
		}
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

bool CBEOPWatchDlg::UpdateThreadState(wstring strS3dbPath,wstring strThreadName,wstring strState,int nPeriod/*=-1*/)
{
	try
	{
		if(strThreadName.length() <= 0)
			return false;
		//修改数据库
		m_pDataHandle->WriteCoreDebugItemValue(strThreadName,strState);

		if(nPeriod != -1)		//只需修改数据库
		{
			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
			CSqliteAcess access(strPath.c_str());
			if (access.GetOpenState())
			{
				_bstr_t tem_des = strThreadName.c_str();
				char* des = (char*)tem_des;
				std::string threadname(des);

				ostringstream sqlstream_del;
				sqlstream_del << "update list_dllstore set periodicity ="<< nPeriod <<" where unitproperty04 = "<< "'" << threadname << "';"; 
				string sql_del = sqlstream_del.str();
				access.SqlExe(sql_del.c_str());
				access.CloseDataBase();
			}
		}
		return true;
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

bool CBEOPWatchDlg::UpdateThreadState(wstring strS3dbPath,wstring strState,wstring strThreadName,wstring strValue)
{
	try
	{
		if(strThreadName.length() <= 0)
			return false;

		if(_wtoi(strState.c_str()) == 0)
		{
			//修改数据库
			m_pDataHandle->WriteCoreDebugItemValue(strThreadName,strValue);
		}
		else
		{
			string strPath;
			Project::Tools::WideCharToUtf8(strS3dbPath,strPath);
			CSqliteAcess access(strPath.c_str());
			if (access.GetOpenState())
			{
				_bstr_t tem_des = strThreadName.c_str();
				char* des = (char*)tem_des;
				std::string threadname(des);

				ostringstream sqlstream_del;
				sqlstream_del << "update list_dllstore set periodicity ="<< _wtoi(strValue.c_str()) <<" where unitproperty04 = "<< "'" << threadname << "';"; 
				string sql_del = sqlstream_del.str();
				access.SqlExe(sql_del.c_str());
				access.CloseDataBase();
			}
		}
		return true;
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

bool CBEOPWatchDlg::UpdateS3db()
{
	try
	{
		wstring strFolder,strUpdateFolder,strBacFolder,strS3dbPath,strS3dbName;
		GetSysPath(strFolder);
		strFolder += L"\\fileupdate";
		if(FindOrCreateFolder(strFolder))
		{
			strUpdateFolder = strFolder + L"\\update";
			strBacFolder = strFolder + L"\\back";

			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				strS3dbPath =  m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"corefilepath");
				if(strS3dbPath.length() <= 0)
					return false;
				CString strFilePath = strS3dbPath.c_str();
				strFilePath = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('\\')-1);
				strS3dbName = strFilePath.Left(strFilePath.ReverseFind('.')).GetString();
				if(FindOrCreateFolder(strUpdateFolder))
				{
					vector<CString> vecFile;
					FindFile(strUpdateFolder.c_str(),_T("s3db"),vecFile);
					if(vecFile.size()>0)
					{
						//使用.csv更新备份s3db
						CString strUpdateS3dbPath,strUpdateS3dbBackPath;
						strUpdateS3dbPath.Format(_T("%s\\%s"),strUpdateFolder.c_str(),vecFile[0]);

						SYSTEMTIME sNow;
						GetLocalTime(&sNow);

						CString strTime;
						strTime.Format(_T("%d%02d%02d%02d%02d"),sNow.wYear,sNow.wMonth,sNow.wDay,sNow.wHour,sNow.wMinute);

						//备份当前使用s3db到备份文件夹
						if(FindOrCreateFolder(strBacFolder))
						{
							CString strNewBacPath;
							strNewBacPath.Format(_T("%s\\%s_%s.s3db"),strBacFolder.c_str(),strS3dbName.c_str(),strTime);
							//先备份一个原s3db到备份文件夹
							if(!CopyFile(strS3dbPath.c_str(),strNewBacPath,FALSE))
							{
								CString strLog;
								strLog.Format(_T("Backup s3db(%s) To Update Folder (%s) Fails."),strS3dbPath.c_str(),strNewBacPath);
								OutPutUpdateLog(strLog);
								return false;
							}

							CString strLog;
							strLog.Format(_T("Backup before update s3db(%s)."),strNewBacPath);
							OutPutUpdateLog(strLog);
						}

						//覆盖
						if(DelteFile(strUpdateS3dbPath,strS3dbPath.c_str()))
						{
							if(m_pDataHandle && m_pDataHandle->IsConnected())
							{
								m_strUpdateS3dbPath = strS3dbPath.c_str();
								m_strUpdateXlsFile = vecFile[0];

								//重启Core
								m_pDataHandle->WriteCoreDebugItemValue(L"restartcoretype",L"12");
								m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"12");
							}
						}
						else
						{
							CString strLog;
							strLog.Format(_T("Copy s3db(%s) to the directory folder (%s) fails."),strUpdateS3dbPath,strS3dbPath.c_str());
							OutPutUpdateLog(strLog);
						}
					}
					else
					{
						OutPutUpdateLog(_T("Can not find update point file！"));
					}
				}
				m_pDataHandle->WriteCoreDebugItemValue(L"updates3db",L"0");
			}
		}
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
	return true;
}

bool CBEOPWatchDlg::DeleteDllParamter(CString strS3dbPath,wstring strParamSql)
{
	try
	{
		if(strParamSql.length() <= 0)
			return false;

		string strPath;
		Project::Tools::WideCharToUtf8(strS3dbPath.GetString(),strPath);
		CSqliteAcess access(strPath.c_str());
		if (access.GetOpenState())
		{
			string sql = "";
			sql = Project::Tools::WideCharToAnsi(strParamSql.c_str());
			int nResult = access.SqlExe(sql.c_str());
			access.CloseDataBase();
			if(nResult == 0)
				return true;
		}
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

void CBEOPWatchDlg::SplitStringSpecial(const char* buffer,int nSize, std::vector<string>& resultlist)
{
	try
	{
		if(nSize <= 0)
			return;

		resultlist.clear();
		char* cSpiltBufferSpilt = new char[nSize+1];
		memcpy(cSpiltBufferSpilt,buffer,nSize);

		char cBefore = 0x00;
		int nBefore = 0;
		int nAfter = 0;
		for(int i=0; i<nSize; ++i)
		{
			if(buffer[i] == ',' && cBefore == ')')
			{
				char* cBuffer = new char[1024];
				memset(cBuffer,0,1024);
				int length = i-1-nBefore;
				length = (length>1024)?1024:length;
				memcpy(cBuffer,cSpiltBufferSpilt+nBefore,length);		
				resultlist.push_back(cBuffer);
				nBefore = i+1;
			}
			cBefore = buffer[i];
		}

		int nLength = nSize - nBefore;
		if(nLength>0)
		{
			char* cBuffer = new char[1024];
			memset(cBuffer,0,1024);
			int length = (nLength>1024)?1024:nLength;
			memcpy(cBuffer,cSpiltBufferSpilt+nBefore,length);
			resultlist.push_back(cBuffer);
		}

		delete []cSpiltBufferSpilt;
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
}

bool CBEOPWatchDlg::FeedDog( bool bFirst /*= true*/ )
{
	return CDogTcpCtrl::GetInstance()->SendActiveSignal();
}

bool CBEOPWatchDlg::HandleRestartLogic()
{
	if(m_pDataHandle)
	{
		if(m_pDataHandle->IsConnected())
		{
			wstring wstrAutoStartCore = m_pDataHandle->ReadOrCreateCoreDebugItemValue(L"restartlogictype",L"-1");
			int nRestartType = _wtoi(wstrAutoStartCore.c_str());
			switch(nRestartType)
			{
			case 0:
			case 1:
			case 2:
				{
					if(RestartLogicByDog(nRestartType,m_nErrorCode))
					{
						
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"-1");
				}
				break;
			case 12:
				{
					if(RestartLogicByDog(nRestartType,m_nErrorCode,true))
					{
						
					}
					else
					{
											
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"-1");
				}
				break;
			case 6:
				{
					if(RestartLogicByDog(nRestartType,m_nErrorCode,true))
					{
						CString strLog;
						strLog.Format(_T("Update And Restart Logic(%s)."),m_strUpdateCorePath);
						OutPutUpdateLog(strLog);
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"-1");
				}
				break;
			case 7:
				{
					if(RestartLogicByDog(nRestartType,m_nErrorCode,true))
					{
						
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"-1");
				}
				break;
			case 9:
				{
					if(RestartLogicByDog(nRestartType,m_nErrorCode,true))
					{			
						AckUpdateExeFile("Remote Server Restart Logic Success.");
					}
					else
					{
						CString strFail;
						strFail.Format(_T("Remote Server Restart Logic Fails(%d)."),m_nErrorCode);
						AckUpdateExeFile(Project::Tools::WideCharToAnsi(strFail));
					}
					m_pDataHandle->WriteCoreDebugItemValue(L"restartlogictype",L"-1");
				}
				break;
			default:
				break;
			}
			return true;
		}
	}
	return false;
}

bool CBEOPWatchDlg::RestartLogicByDog( int nType,int& nErrorCode,bool bRestart_/*=false*/ )
{
	CString strType;
	if(nType == 0)
	{
		strType = _T("(Manual)");
	}
	else if(nType == 1)
	{
		strType = _T("(Logic not run)");
	}
	else if(nType == 2)
	{
		strType = _T("(servertime not update 10m)");
	}
	else if(nType == 6)
	{
		strType = _T("(update exe)");
	}
	else if(nType == 7)
	{
		strType = _T("(update dll)");
	}
	else if(nType == 9)
	{
		strType = _T("(remote restart)");
	}
	else if(nType == 12)
	{
		strType = _T("(update s3db restart)");
	}
	
	bool bRestart = false;
	if (CDogTcpCtrl::GetInstance()->GetDogRestartFunctonOK())		//dog连接成功 且版本号支持
	{
		if(CDogTcpCtrl::GetInstance()->SendRestart(E_SLAVE_LOGIC,E_REASON_UPDATE))
		{
			//等待回应的机制 5秒
			int nWaitingCount = 50; 
			bool bRestartUpdate = false;
			bool bRestartCore = false;
			bool bRestartLogic = false;
			bool bRestartMaster = false;
			while(nWaitingCount>=0)
			{
				CDogTcpCtrl::GetInstance()->GetRestartSuccess(bRestartCore,bRestartLogic,bRestartUpdate,bRestartMaster);
				if(bRestartLogic)
				{
					CDogTcpCtrl::GetInstance()->SetRestartLogicSuccess(false);
					bRestart = true;
					break;
				}
				Sleep(100);
				nWaitingCount--;
			}
		}
	}
	else
	{
		m_PrsV.CloseApp(g_NAME_Logic);
		Sleep(1000);
		return m_PrsV.OpenApp(m_strLogicPath);
	}

	if(bRestart)
	{
		CString str;
		str = g_NAME_Logic + _T(" auto restart success") + strType;
		OutPutLogString(str);
	}
	else
	{
		CString str;
		str = g_NAME_Logic + _T(" auto restart fail") + strType;
		OutPutLogString(str);
	}
	return bRestart;
}

bool CBEOPWatchDlg::ExitCPSender()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_watch_lock);
	if(m_pdtusender)
	{
		m_pdtusender->Exit();
		delete m_pdtusender;
		m_pdtusender = NULL;
	}
	return true;
}

bool CBEOPWatchDlg::AckMysqlFileByTableName_( string strTableName )
{
	//导出生成sql文件
	if(m_pDataHandle)
	{
		string strBaseDir = m_pDataHandle->ReadMysqlVariable("basedir");
		if(strBaseDir.length() > 0)
		{
			wstring strFolder;
			GetSysPath(strFolder);
			strFolder += L"\\Temp";	 
			if(Project::Tools::FindOrCreateFolder(strFolder))
			{
				//生成命令
				Database_Config  dbparam = m_pDataHandle->GetDatabaseParam();
				std::ostringstream sqlstream;
				string baseDir(strBaseDir.substr(0,2));
				sqlstream << baseDir << "\n";
				sqlstream << "cd " << strBaseDir << "bin\n";
				sqlstream << "mysqldump -h" << dbparam.strDBIP << " -u" << dbparam.strUserName << " -p" << dbparam.strPsw << " " << dbparam.strDBName << " " << strTableName << " >"
					<< Project::Tools::WideCharToAnsi(strFolder.c_str()) << "/" << strTableName << ".sql";

				CString strConfigPath,strFilePath;
				strConfigPath.Format(_T("%s\\%s.bat"),strFolder.c_str(),Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());
				strFilePath.Format(_T("%s\\%s.sql"),strFolder.c_str(),Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());

				CStdioFile	ff;
				char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
				setlocale( LC_ALL, "chs" );
				if(ff.Open(strConfigPath,CFile::modeCreate|CFile::modeWrite))
				{
					ff.WriteString(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
					ff.Close();
					setlocale( LC_ALL, old_locale );
					free(old_locale);

					SHELLEXECUTEINFO ShExecInfo = {0};
					ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
					ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
					ShExecInfo.hwnd = NULL;
					ShExecInfo.lpVerb = NULL;
					ShExecInfo.lpFile = strConfigPath;	
					ShExecInfo.lpParameters = _T("");	
					ShExecInfo.lpDirectory = _T("");
					ShExecInfo.nShow = SW_HIDE;
					ShExecInfo.hInstApp = NULL;

					int nResult = ShellExecuteEx(&ShExecInfo);
					WaitForSingleObject(ShExecInfo.hProcess, 600); 

					DWORD dwExitCode;
					if(GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode))			//成功导出
					{
						CString strZipPath,strFileName;					
						strZipPath.Format(_T("%s\\%s.zip"),strFolder.c_str(),Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());
						strFileName.Format(_T("%s.sql"), Project::Tools::AnsiToWideChar(strTableName.c_str()).c_str());

						HZIP hz = CreateZip(strZipPath,NULL);
						ZipAdd(hz,strFileName,strFilePath);
						CloseZip(hz);

						if(m_bdtuSuccess)
						{
							DTUSendInfo sDTUInfo;
							SYSTEMTIME st;
							GetLocalTime(&st);
							sDTUInfo.tSendTime = st;
							sDTUInfo.nType = 6;
							sDTUInfo.nSubType = 1;
							string strZipPath_Ansi = Project::Tools::WideCharToAnsi(strZipPath);
							sDTUInfo.strSendBuf = strZipPath_Ansi;

							if(!m_bVecCopying)
								m_vecSendBuffer.push_back(sDTUInfo);
							SetSendDTUEventAndType();
						}
					}
				}
				else
				{
					setlocale( LC_ALL, old_locale );
					free(old_locale);
				}

				//删除.sql 和 .bat
				DelteFile(strConfigPath);
				DelteFile(strFilePath);
			}
		}
	}
	return true;
}

void CBEOPWatchDlg::AckMysqlFileByTableName(string strTableName)
{
	m_strAckMysqlTableName = strTableName;
	HANDLE hCreate = (HANDLE)_beginthreadex(NULL, 0, AckMysqlFileByTableNameThread, this, 0, NULL);
}

unsigned int WINAPI CBEOPWatchDlg::AckMysqlFileByTableNameThread( LPVOID lpVoid )
{
	CBEOPWatchDlg* pAckMysqlFileByTableName = reinterpret_cast<CBEOPWatchDlg*>(lpVoid);
	if (NULL == pAckMysqlFileByTableName)
	{
		return 0;
	}
	return pAckMysqlFileByTableName->AckMysqlFileByTableName_(pAckMysqlFileByTableName->m_strAckMysqlTableName);
}

double CBEOPWatchDlg::CheckDiskFreeSpace( string strDisk )
{
	unsigned __int64 i64FreeBytesToCaller;  
	unsigned __int64 i64TotalBytes;  
	unsigned __int64 i64FreeBytes;  

	double dUse = 0.0;
	bool fResult = ::GetDiskFreeSpaceEx (  
		Project::Tools::AnsiToWideChar(strDisk.c_str()).c_str(),  
		(PULARGE_INTEGER)&i64FreeBytesToCaller,  
		(PULARGE_INTEGER)&i64TotalBytes,  
		(PULARGE_INTEGER)&i64FreeBytes);  
	//GetDiskFreeSpaceEx函数，可以获取驱动器磁盘的空间状态,函数返回的是个BOOL类型数据  
	if(fResult)
	{
		int nCount = (float)i64TotalBytes/1024/1024;
		int nSpace = (float)i64FreeBytesToCaller/1024/1024;
		dUse = 1-(double)nSpace/(double) nCount;
	}
	return dUse;
}

bool CBEOPWatchDlg::DeleteTableByDiskWarning()
{
	//查找数据库磁盘
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		string strDataDir = m_pDataHandle->ReadMysqlVariable("datadir");
		if(strDataDir.length() > 0)
		{
			//获取磁盘空间大小
			double dUseDisk = CheckDiskFreeSpace(strDataDir);
			wstring wstrSaveLogTable = L"1";			//Log表总开关配置项
			if(dUseDisk >= 0.95)		//删除1个月的数据  同时关闭log开关
			{
				wstrSaveLogTable = L"0";
				DeleteTableByDate("historydata_5second_",1);
				DeleteTableByDate("historydata_minute_",1);
				DeleteTableByDate("historydata_5minute_",1);
				DeleteTableByDate("log_",1);
			}
			else if(dUseDisk >= 0.90)		//删除3天的数据
			{
				DeleteTableByDate("historydata_5second_",3);
				DeleteTableByDate("historydata_minute_",3);
				DeleteTableByDate("historydata_5minute_",3);
				DeleteTableByDate("log_",3);
			}
			else if(dUseDisk >= 0.8)		//删除1天的数据
			{
				DeleteTableByDate("historydata_5second_",30);
				DeleteTableByDate("historydata_minute_",30);
				DeleteTableByDate("historydata_5minute_",30);
				DeleteTableByDate("log_",30);
			}
			else							//常规动作 删除截止日期前的数据
			{
				DeleteTableByDeadDate("historydata_5second_",730);
				DeleteTableByDeadDate("historydata_minute_",730);
				DeleteTableByDeadDate("historydata_5minute_",730);
				DeleteTableByDeadDate("log_",730);
			}
			m_pDataHandle->WriteCoreDebugItemValue(L"SaveLogTable",wstrSaveLogTable);
		}
		return true;
	}
	return false;
}

bool CBEOPWatchDlg::DeleteTableByDate( string strTablePrefix, int nDate )
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		vector<string> vecDropTable;
		if(m_pDataHandle->ReadTableNameByDate(vecDropTable,strTablePrefix,nDate))
		{
			return m_pDataHandle->DropTables(vecDropTable);
		}
	}
	return false;
}

bool CBEOPWatchDlg::DeleteTableByDeadDate( string strTablePrefix, int nDate )
{
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		vector<string> vecDropTable;
		if(m_pDataHandle->ReadTableNameByDeadDate(vecDropTable,strTablePrefix,nDate))
		{
			return m_pDataHandle->DropTables(vecDropTable);
		}
	}
	return false;
}
