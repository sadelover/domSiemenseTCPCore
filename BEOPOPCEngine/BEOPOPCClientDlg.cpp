
// BEOPOPCClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BEOPOPCClient.h"
#include "BEOPOPCClientDlg.h"
#include "afxdialogex.h"
#include "BEOPOPCLink.h"
#include "CSVParser.h"
#include "RestartAutoRun.h"
#include "DumpFile.h"
using namespace CSVParser;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CBEOPOPCClientDlg dialog




CBEOPOPCClientDlg::CBEOPOPCClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBEOPOPCClientDlg::IDD, pParent)
	, m_bFirst(TRUE),
	m_opcengine(NULL),
	m_nOPCClientMaxPoint(10000),
	m_bMutilOPCClientMode(false),
	m_bMutilOPCClientUseThread(false),
	m_nOPCCmdSleep(5),
	m_nMutilCount(20),
	m_nOPCCmdSleepFromDevice(50),
	m_nOPCPollSleep(5),
	m_nOPCPollSleepFromDevice(60),
	m_nOPCCheckReconnectInterval(5),
	m_nOPCReconnectInertval(10),
	m_nDebugOPC(0),
	m_nReconnectMinute(0),
	m_nOPCMainThreadOPCPollSleep(2),
	m_bEnableSecurity(false),
	m_bDisableCheckQuality(false),
	m_strEnableSecurity(L"0"),
	m_pDataHandle(NULL),
	m_bExitThread(false),
	m_hupdatehandle(NULL),
	m_bCloseDlg(false),
	m_strOPCServerIP(L"localhost"),
	m_nUpdateInterval(2),
	m_bIconOnline(true),
	m_nPrecision(6),
	m_bAsync20Mode(false),
	m_nRefresh20Interval(60)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON3);
}

void CBEOPOPCClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBEOPOPCClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SHOWTASK,OnShowTask)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CLOSE_WATCH,&CBEOPOPCClientDlg::OnCloseWatch)
END_MESSAGE_MAP()


// CBEOPOPCClientDlg message handlers

BOOL CBEOPOPCClientDlg::OnInitDialog()
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

	// TODO: Add extra initialization here
	//初始化系统托盘图标
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDI_ICON3;
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
	m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON3));//加载图标
	wcscpy(m_nid.szTip, _T("BEOPOPCEngine V1.0.1"));//信息提示
	Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标
	ShowWindow(SW_HIDE);//隐藏窗体  

	//写注册表 开机启动
	TCHAR exepath[MAX_PATH];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CRestartAutoRun::SetAutoRun(exepath);

	InitConfigParams();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBEOPOPCClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBEOPOPCClientDlg::OnPaint()
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
HCURSOR CBEOPOPCClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


LRESULT CBEOPOPCClientDlg::OnShowTask( WPARAM wParam,LPARAM lParam )
{
	if(wParam!=IDI_ICON3 && wParam!=IDI_ICON2) 
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
			//this->ShowWindow(SW_SHOW);//简单的显示主窗口完事儿
		}
		break; 
	default:
		break;
	} 
	return 0;
}

BOOL CBEOPOPCClientDlg::OnEraseBkgnd( CDC* pDC )
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

void CBEOPOPCClientDlg::OnClose()
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

void CBEOPOPCClientDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	//删除托盘图标
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void CBEOPOPCClientDlg::OnTimer( UINT_PTR nIDEvent )
{
	CDialog::OnTimer(nIDEvent);
}

void CBEOPOPCClientDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if(nType == SIZE_MINIMIZED)
	{
		ShowWindow(SW_HIDE);
	}
}

BOOL CBEOPOPCClientDlg::PreTranslateMessage( MSG* pMsg )
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

bool CBEOPOPCClientDlg::InitOPC()
{
	vector<OPCDataPointEntry> opcpointlist = m_opcpointlist;
	if (!opcpointlist.empty())
	{
		//SendLogMsg(L"OPC", L"Init OPC Engine...\r\n");
		OutPutLogString(L"Init OPC Engine...");
		wstring strOPCServerIP = m_strOPCServerIP;
		if(opcpointlist[0].GetSourceType() == L"OPC")		//OPC类型的点 若opcpointlist[0].GetOPCServerIP()不为空 需要根据ip创建连接 否则使用默认配置的IP
		{
			m_bMutilOPCClientMode = true;
			vector<vector<OPCDataPointEntry>> vecServerList = GetOPCServer(opcpointlist,m_nOPCClientMaxPoint,strOPCServerIP);
			for(int i=0; i<vecServerList.size(); i++)
			{
				vector<OPCDataPointEntry> opcpointlist_ = vecServerList[i];
				int nOPCIndex = i;
				if(!opcpointlist_.empty())
				{
					//////////////////////////////////////////////////////////////////////////
					wstring strOPCSIP = opcpointlist_[0].GetOPCServerIP();
					if(strOPCSIP.length() <= 0)
						strOPCSIP = strOPCServerIP;
					COPCEngine* opcengine = new COPCEngine(strOPCSIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval);
					opcengine->SetOpcPointList(opcpointlist_);
					opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
					m_mapOPCEngine[nOPCIndex] = opcengine;
					if (!InitOPC(opcengine,nOPCIndex))
					{
						CString strErr;
						strErr.Format(_T("ERROR: OPC Engine %d(Mutil:%s-%s) Start Failed!"),nOPCIndex,strOPCSIP.c_str(),opcpointlist_[0].GetParam(3).c_str());
						//SendLogMsg(L"OPC",strErr);
						OutPutLogString(strErr.GetString());
						return false;
					}
					//////////////////////////////////////////////////////////////////////////
				}
			}
			return true;
		}
		else
		{
			//增加大批量OPC时分连接读取
			int nSize = opcpointlist.size();
			if(nSize <= m_nOPCClientMaxPoint)
			{
				m_opcengine = new COPCEngine(strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval);
				m_opcengine->SetOpcPointList(opcpointlist);
				m_opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
				if (!InitOPC(m_opcengine,0))
				{
					//SendLogMsg(L"OPC",L"ERROR: OPC Engine Start Failed!\r\n");
					OutPutLogString(L"ERROR: OPC Engine Start Failed!");
					return false;
				}

				//SendLogMsg(L"OPC",L"OPC Engine Connected Success.\r\n");
				OutPutLogString(L"OPC Engine Connected Success.");
				return true;
			}
			else
			{
				m_bMutilOPCClientMode = true;
				int nOPCIndex = 0;
				vector<OPCDataPointEntry> opcpointlist_;		//单个引擎
				for(int i=0; i< opcpointlist.size(); ++i)
				{
					if(i%m_nOPCClientMaxPoint == 0 && i != 0)
					{
						COPCEngine* opcengine = new COPCEngine(strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval);
						opcengine->SetOpcPointList(opcpointlist_);
						opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
						m_mapOPCEngine[nOPCIndex] = opcengine;
						if (!InitOPC(opcengine,nOPCIndex))
						{
							CString strErr;
							strErr.Format(_T("ERROR: OPC Engine %d(Mutil) Start Failed!"),nOPCIndex);
							//SendLogMsg(L"OPC",strErr);
							OutPutLogString(strErr.GetString());
							return false;
						}
						nOPCIndex++;
						opcpointlist_.clear();
					}
					opcpointlist_.push_back(opcpointlist[i]);
				}

				if(opcpointlist_.size() > 0)
				{
					COPCEngine* opcengine = new COPCEngine(strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality,m_nPrecision,m_bAsync20Mode,m_nRefresh20Interval);
					opcengine->SetOpcPointList(opcpointlist_);
					opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
					m_mapOPCEngine[nOPCIndex] = opcengine;
					if (!InitOPC(opcengine,nOPCIndex))
					{
						CString strErr;
						strErr.Format(_T("ERROR: OPC Engine %d(Mutil) Start Failed!"),nOPCIndex);
						//SendLogMsg(L"OPC",strErr);
						OutPutLogString(strErr.GetString());
						return false;
					}
				}
			}
			//SendLogMsg(L"OPC",L"OPC Engine(Mutil) Connected Success.\r\n");
			OutPutLogString(L"OPC Engine(Mutil) Connected Success.");
			return true;
		}
	}
	return false;
}

bool CBEOPOPCClientDlg::InitOPC( COPCEngine* opcengine,int nOPCClientIndex /*= -1*/ )
{
	if (!opcengine){
		return false;
	}

#ifdef DEBUG
	return opcengine->Init(nOPCClientIndex);
#else
	int repeattime = 2;
	while(repeattime--){
		if (opcengine->Init(nOPCClientIndex)){
			return true;
		}else{
			Sleep(5*1000);
		}
	}
#endif
	return false;
}

vector<vector<OPCDataPointEntry>> CBEOPOPCClientDlg::GetOPCServer( vector<OPCDataPointEntry> opcpoint,int nMaxSize,wstring strDefaultIP )
{
	vector<vector<OPCDataPointEntry>> vecOPCPList;
	m_vecOPCSer.clear();
	for(int i=0; i<opcpoint.size(); i++)
	{
		//
		wstring strIP = opcpoint[i].GetParam(6);
		wstring strProgName = opcpoint[i].GetParam(3);
		if(strProgName.empty())
			continue;
		if(strIP.empty())
			strIP = strDefaultIP;
		int nPos = IsExistInVec(strIP,strProgName,m_vecOPCSer,nMaxSize);
		if(nPos==m_vecOPCSer.size())	//不存在
		{
			OPCServer pServer;
			pServer.strIP = strIP;
			pServer.strServerName = strProgName;
			pServer.nCount = 1;
			m_vecOPCSer.push_back(pServer);

			vector<OPCDataPointEntry> vecPList;
			vecOPCPList.push_back(vecPList);
		}
		vecOPCPList[nPos].push_back(opcpoint[i]);		
	}
	return vecOPCPList;
}

int CBEOPOPCClientDlg::IsExistInVec( wstring strSer, wstring strProgram,vector<OPCServer>& vecSer,int nMaxSize )
{
	int i=0;
	for(i=0; i<vecSer.size(); i++)
	{
		if(strSer == vecSer[i].strIP && strProgram == vecSer[i].strServerName && vecSer[i].nCount < nMaxSize)
		{
			vecSer[i].nCount++;
			return i;
		}
	}
	return i;
}

bool CBEOPOPCClientDlg::InitConfigParams()
{
	wstring strPath;
	GetSysPath(strPath);
	CString strIniFilePath;
	strIniFilePath.Format(_T("%s\\beopopcclient.ini"),strPath.c_str());
	
	//初始化数据库参数
	wchar_t charContent[256];
	GetPrivateProfileString(L"db", L"defaulathost", L"", charContent, 256, strIniFilePath);
	wstring wstrDefaultHost = charContent;
	if(wstrDefaultHost == L"")
		wstrDefaultHost = L"localhost";
	WritePrivateProfileString(L"db",L"defaulathost",wstrDefaultHost.c_str(),strIniFilePath);

	GetPrivateProfileString(L"db", L"defaultdb", L"", charContent, 256, strIniFilePath);
	wstring wstrDefaultDB = charContent;
	if(wstrDefaultDB == L"")
		wstrDefaultDB = L"beopdata";
	WritePrivateProfileString(L"db",L"defaultdb",wstrDefaultDB.c_str(),strIniFilePath);

	GetPrivateProfileString(L"db", L"defaultuser", L"", charContent, 256, strIniFilePath);
	wstring wstrDefaultUser = charContent;
	if(wstrDefaultUser == L"")
		wstrDefaultUser = L"root";
	WritePrivateProfileString(L"db",L"defaultuser",wstrDefaultUser.c_str(),strIniFilePath);

	GetPrivateProfileString(L"db", L"defaultpsw", L"", charContent, 256, strIniFilePath);
	wstring wstrDefaultPsw = charContent;
	if(wstrDefaultPsw == L"")
		wstrDefaultPsw = L"RNB.beop-2013";
	WritePrivateProfileString(L"db",L"defaultpsw",wstrDefaultPsw.c_str(),strIniFilePath);

	//初始化OPC参数
	wstring wstrOPCClientMaxPoint,wstrOPCClientThread,wstrStoreHistory,wstrOPCSleep,wstrOPCMutilCount,wstrMutilReadCount,wstrAsync20Mode,wstrRefresh20Interval;

	GetPrivateProfileString(L"opc", L"opcserverip", L"", charContent, 256, strIniFilePath);
	m_strOPCServerIP = charContent;
	if(m_strOPCServerIP == L"")
		m_strOPCServerIP = L"localhost";
	WritePrivateProfileString(L"opc",L"opcserverip",m_strOPCServerIP.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcclientmaxpoint", L"", charContent, 256, strIniFilePath);
	wstrOPCClientMaxPoint = charContent;
	if(wstrOPCClientMaxPoint == L"" || wstrOPCClientMaxPoint == L"0")
		wstrOPCClientMaxPoint = L"10000";
	m_nOPCClientMaxPoint = _wtoi(wstrOPCClientMaxPoint.c_str());
	WritePrivateProfileString(L"opc",L"opcclientmaxpoint",wstrOPCClientMaxPoint.c_str(),strIniFilePath);

	//OPC Thread
	GetPrivateProfileString(L"opc", L"opcclientthread", L"", charContent, 256, strIniFilePath);
	wstrOPCClientThread = charContent;
	if(wstrOPCClientThread == L"")
		wstrOPCClientThread = L"1";
	m_bMutilOPCClientUseThread = _wtoi(wstrOPCClientThread.c_str());
	WritePrivateProfileString(L"opc",L"opcclientthread",wstrOPCClientThread.c_str(),strIniFilePath);

	//
	GetPrivateProfileString(L"opc", L"opcmutilcount", L"", charContent, 256, strIniFilePath);
	wstrOPCMutilCount = charContent;
	if(wstrOPCMutilCount == L"")
		wstrOPCMutilCount = L"20";
	m_nMutilCount = _wtoi(wstrOPCMutilCount.c_str());
	WritePrivateProfileString(L"opc",L"opcmutilcount",wstrOPCMutilCount.c_str(),strIniFilePath);

	//
	GetPrivateProfileString(L"opc", L"opccmdsleep", L"", charContent, 256, strIniFilePath);
	wstrOPCSleep = charContent;
	if(wstrOPCSleep == L"")
		wstrOPCSleep = L"1000";
	m_nOPCCmdSleep = _wtoi(wstrOPCSleep.c_str());
	WritePrivateProfileString(L"opc",L"opccmdsleep",wstrOPCSleep.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcasync20mode", L"", charContent, 256, strIniFilePath);
	wstrAsync20Mode = charContent;
	if(wstrAsync20Mode == L"")
		wstrAsync20Mode = L"0";
	m_bAsync20Mode = _wtoi(wstrAsync20Mode.c_str());
	WritePrivateProfileString(L"opc",L"opcasync20mode",wstrAsync20Mode.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcasync20interval", L"", charContent, 256, strIniFilePath);
	wstrRefresh20Interval = charContent;
	if(wstrRefresh20Interval == L"")
		wstrRefresh20Interval = L"60";
	m_nRefresh20Interval = _wtoi(wstrRefresh20Interval.c_str());
	WritePrivateProfileString(L"opc",L"opcasync20interval",wstrRefresh20Interval.c_str(),strIniFilePath);

	//数据精度
	GetPrivateProfileString(L"opc", L"precision", L"", charContent, 256, strIniFilePath);
	wstring wstrPrecision = charContent;
	if(wstrPrecision == L"")
		wstrPrecision = L"2";
	m_nPrecision = _wtoi(wstrPrecision.c_str());
	WritePrivateProfileString(L"opc",L"precision",wstrPrecision.c_str(),strIniFilePath);
	//////////////////////////////////////////////////////////////////////////
	wstring wstrUpdateMysql,wstrOPCCmdSleepFromDevice,wstrOPCPollSleep,wstrOPCPollSleepFromDevice,wstrReconnectMinute,wstrIDInterval,wstrEnableSecurity,wstrOPCMainThreadOPCPollSleep,wstrDisableCheckQuality,wstrModbusTimeout;
	GetPrivateProfileString(L"opc", L"opccmdsleepfromdevice", L"", charContent, 256, strIniFilePath);
	wstrOPCCmdSleepFromDevice = charContent;
	if(wstrOPCCmdSleepFromDevice == L"")
		wstrOPCCmdSleepFromDevice = L"1000";
	m_nOPCCmdSleepFromDevice = _wtoi(wstrOPCCmdSleepFromDevice.c_str());
	WritePrivateProfileString(L"opc",L"opccmdsleepfromdevice",wstrOPCCmdSleepFromDevice.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcpollsleep", L"", charContent, 256, strIniFilePath);
	wstrOPCPollSleep = charContent;
	if(wstrOPCPollSleep == L"")
		wstrOPCPollSleep = L"60";
	m_nOPCPollSleep = _wtoi(wstrOPCPollSleep.c_str());
	WritePrivateProfileString(L"opc",L"opcpollsleep",wstrOPCPollSleep.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcpollsleepfromdevice", L"", charContent, 256, strIniFilePath);
	wstrOPCPollSleepFromDevice = charContent;
	if(wstrOPCPollSleepFromDevice == L"")
		wstrOPCPollSleepFromDevice = L"60";
	m_nOPCPollSleepFromDevice = _wtoi(wstrOPCPollSleepFromDevice.c_str());
	WritePrivateProfileString(L"opc",L"opcpollsleepfromdevice",wstrOPCPollSleepFromDevice.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcreconnectignore", L"", charContent, 256, strIniFilePath);
	wstrReconnectMinute = charContent;
	if(wstrReconnectMinute == L"")
		wstrReconnectMinute = L"0";
	m_nReconnectMinute = _wtoi(wstrReconnectMinute.c_str());
	WritePrivateProfileString(L"opc",L"opcreconnectignore",wstrReconnectMinute.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"enableSecurity", L"", charContent, 256, strIniFilePath);
	wstrEnableSecurity = charContent;
	if(wstrEnableSecurity == L"")
		wstrEnableSecurity = L"0";
	m_bEnableSecurity = _wtoi(wstrEnableSecurity.c_str());
	WritePrivateProfileString(L"opc",L"enableSecurity",wstrEnableSecurity.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"disableCheckQuality", L"", charContent, 256, strIniFilePath);
	wstrDisableCheckQuality = charContent;
	if(wstrDisableCheckQuality == L"")
		wstrDisableCheckQuality = L"0";
	m_bDisableCheckQuality = _wtoi(wstrDisableCheckQuality.c_str());
	WritePrivateProfileString(L"opc",L"disableCheckQuality",wstrDisableCheckQuality.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcmainpollsleep", L"", charContent, 256, strIniFilePath);
	wstrOPCMainThreadOPCPollSleep = charContent;
	if(wstrOPCMainThreadOPCPollSleep == L"")
		wstrOPCMainThreadOPCPollSleep = L"60";
	m_nOPCMainThreadOPCPollSleep = _wtoi(wstrOPCMainThreadOPCPollSleep.c_str());
	WritePrivateProfileString(L"opc",L"opcmainpollsleep",wstrOPCMainThreadOPCPollSleep.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcreconnect", L"", charContent, 256, strIniFilePath);
	wstring strOPCReconnectInterval = charContent;
	if(strOPCReconnectInterval==L"")
		strOPCReconnectInterval = L"30";
	m_nOPCReconnectInertval = _wtoi(strOPCReconnectInterval.c_str());
	WritePrivateProfileString(L"opc",L"opcreconnect",strOPCReconnectInterval.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opccheckreconnect", L"", charContent, 256, strIniFilePath);
	wstring strOPCCheckReconnect = charContent;
	if(strOPCCheckReconnect==L"")
		strOPCCheckReconnect = L"5";
	m_nOPCCheckReconnectInterval = _wtoi(strOPCCheckReconnect.c_str());
	WritePrivateProfileString(L"opc",L"opccheckreconnect",strOPCCheckReconnect.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"updatemysql", L"", charContent, 256, strIniFilePath);
	wstrUpdateMysql = charContent;
	if(wstrUpdateMysql == L"")
		wstrUpdateMysql = L"60";
	m_nUpdateInterval = _wtoi(wstrUpdateMysql.c_str());
	WritePrivateProfileString(L"opc",L"updatemysql",wstrUpdateMysql.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"debugopc", L"", charContent, 256, strIniFilePath);
	wstring wstrDebugOPC = charContent;
	if(wstrDebugOPC == L"")
		wstrDebugOPC = L"0";
	m_nDebugOPC = _wtoi(wstrDebugOPC.c_str());
	WritePrivateProfileString(L"opc",L"debugopc",wstrDebugOPC.c_str(),strIniFilePath);

	//初始化点表
	wstring strCSVPath = PreInitFileDirectory();
	if(strCSVPath.length() <= 0)
	{
		OutPutLogString(L"ERROR:Can't find Point CSV!");
		MessageBox(_T("ERROR: Can't find Point CSV!"));
		EndDialog(0);
		return 0;
	}

	int nErrCode = 0;
	if(!ReadFromCSV_Comma(strCSVPath.c_str(),m_opcpointlist,nErrCode))
	{
		OutPutLogString(L"ERROR:Read Point CSV Fail!");
		MessageBox(_T("ERROR: Read Point CSV Fail!"));
		EndDialog(0);
		return 0;
	}

	//初始化数据库
	m_pDataHandle = new CDataHandler();
	if(m_pDataHandle)
	{
		if(!m_pDataHandle->Connect(wstrDefaultHost,wstrDefaultUser,wstrDefaultPsw,wstrDefaultDB,3306))
		{
			OutPutLogString(L"ERROR: Connect DB Fail!");
			MessageBox(_T("ERROR: Connect DB Fail!"));
			EndDialog(0);
			return 0;
		}
	}

	//初始化COM 启用进程安全 因此不能线程读取OPC点
	if(m_bEnableSecurity)
	{
		HRESULT hrinitcode = CoInitialize(NULL);
		if(hrinitcode != RPC_E_CHANGED_MODE)
		{
			hrinitcode = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		}

		hrinitcode = CoInitializeSecurity(NULL, -1, NULL, NULL,
			RPC_C_AUTHN_LEVEL_NONE,
			RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL);

		if(FAILED(hrinitcode))
		{
			//PrintLog(L"ERROR:CoInitializeSecurity...\r\n");
		}
	}

	//初始化OPC
	if(InitOPC())
	{
		OutPutLogString(L"Init OPC Engine Success...");
	}
	else
	{
		OutPutLogString(L"ERROR:Init OPC Engine Fail...");
	}

	//启动线程
	m_hupdatehandle = (HANDLE)_beginthreadex(NULL, 0, UpdateMysqlThread, this, 0, NULL);
	return true;
}

bool CBEOPOPCClientDlg::ExitOPC()
{
	//opc 退出 大批量
	map<int,COPCEngine*>::iterator iter3 = m_mapOPCEngine.begin();
	while(iter3 != m_mapOPCEngine.end())
	{
		iter3->second->Exit();
		iter3++;
	}
	return true;
}

bool CBEOPOPCClientDlg::ReadFromCSV_Comma( CString sExcelFile, vector<OPCDataPointEntry>& vecPoint,int& nErrCode )
{
	try
	{
		Csv csv = Csv(Project::Tools::WideCharToAnsi(sExcelFile).c_str());
		//
		if(csv.getRowCount() >0 && csv.getColumnCount() ==5)
		{
			std::map<CString,int> mapPointName;
			for(int i=0; i<csv.getRowCount(); ++i)
			{
				//每一行
				vector<wstring> vecPointInfo;
				OPCDataPointEntry entry;
				CString strPhysicalID = Project::Tools::AnsiToWideChar(csv[i][0].c_str()).c_str();
				CString strparam1 = Project::Tools::AnsiToWideChar(csv[i][1].c_str()).c_str();
				CString strparam2 = Project::Tools::AnsiToWideChar(csv[i][2].c_str()).c_str();
				CString strparam3 = Project::Tools::AnsiToWideChar(csv[i][3].c_str()).c_str();
				CString strparam4 = Project::Tools::AnsiToWideChar(csv[i][4].c_str()).c_str();
				if(strparam2.Find(_T("VT_")) < 0)
				{
					CString strError;
					strError.Format(_T("ERROR:Point Param2 Invalid(%s)."),strPhysicalID);
					OutPutLogString(strError.GetString());
				}
				else
				{
					entry.SetShortName(strPhysicalID.GetString());
					entry.SetSourceType(L"OPC");

					entry.SetParam(1, strparam1.GetString());
					entry.SetParam(2, strparam2.GetString());
					entry.SetParam(3, strparam3.GetString());
					entry.SetParam(4, strparam4.GetString());
					if(entry.GetShortName().length() >0)
						vecPoint.push_back(entry);
				}
			}
			return true;
		}
		else
		{
			OutPutLogString(L"ERROR:Point File Invalid.");
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

wstring CBEOPOPCClientDlg::PreInitFileDirectory()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);

	wstring strDBFileName;


	CFileFind s3dbfilefinder;
	wstring strSearchPath = cstrFile + L"\\*.csv";
	wstring filename;
	BOOL bfind = s3dbfilefinder.FindFile(strSearchPath.c_str());
	wstring SourcePath, DisPath;
	vector<wstring> strAvailableFileNameList;
	while (bfind)
	{
		bfind = s3dbfilefinder.FindNextFile();
		filename = s3dbfilefinder.GetFileName();

		strAvailableFileNameList.push_back(filename);
	}


	if(strAvailableFileNameList.size()>0)
	{
		strDBFileName = cstrFile + L"\\" + strAvailableFileNameList[0];
		for(int nFileIndex=1; nFileIndex<strAvailableFileNameList.size();nFileIndex++)
		{
			wstring strFileNameToDelete = cstrFile + L"\\" + strAvailableFileNameList[nFileIndex];
			if(!DeleteFile(strFileNameToDelete.c_str()))
			{
				_tprintf(L"ERROR: Delete UnNeccesary DB Files Failed!");
			}
		}
	}
	return strDBFileName;
}

unsigned int WINAPI CBEOPOPCClientDlg::UpdateMysqlThread( LPVOID lpVoid )
{
	CBEOPOPCClientDlg* pUpdateValueHandle = reinterpret_cast<CBEOPOPCClientDlg*>(lpVoid);
	if (NULL == pUpdateValueHandle)
	{
		return 0;
	}

	while(!pUpdateValueHandle->m_bExitThread)
	{		
		pUpdateValueHandle->ChangeIcon();
		pUpdateValueHandle->UpdateMysqlValue();
		pUpdateValueHandle->UpdateDebug();
		int nSleep = pUpdateValueHandle->m_nUpdateInterval;
		while(!pUpdateValueHandle->m_bExitThread)
		{
			nSleep--;
			if(nSleep < 0)
				break;
			Sleep(1000);
		}
	}
	return 1;
}

bool CBEOPOPCClientDlg::UpdateMysqlValue()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex);
	vector<pair<wstring, wstring> >	valuesets;
	GetOPCValueSets(valuesets);
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		return m_pDataHandle->UpdateOutPutValue(valuesets);
	}
	return false;
}

void CBEOPOPCClientDlg::GetOPCValueSets( vector<pair<wstring, wstring> >& opcvaluesets )
{
	if(m_bMutilOPCClientMode)
	{
		map<int,COPCEngine*>::iterator iter = m_mapOPCEngine.begin();
		while(iter != m_mapOPCEngine.end())
		{
			iter->second->GetValueSet(opcvaluesets);
			iter++;
		}
	}
	else
	{
		if (m_opcengine)
		{
			m_opcengine->GetValueSet(opcvaluesets);
		}
	}
}

bool CBEOPOPCClientDlg::OutPutLogString( wstring strLog ,bool bDaily)
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\log";
	if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		setlocale( LC_ALL, "chs" );

		CString strOut;
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog.c_str());

		CString strLogPath;
		strLogPath.Format(_T("%s\\beopopcclient_%d_%02d.log"),wstrFileFolder.c_str(),sNow.wYear,sNow.wMonth);

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

void CBEOPOPCClientDlg::OnCloseWatch()
{
	if(CloseOPCClient())
	{
		OutPutLogString(L"Exit OPC Engine...");
		m_bCloseDlg = true;
		SendMessage(WM_CLOSE);
	}
}

bool CBEOPOPCClientDlg::CloseOPCClient()
{
	return Exit();
}

bool CBEOPOPCClientDlg::Exit()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex);
	m_bExitThread = true;
	ExitOPC();
	if(m_hupdatehandle)
		WaitForSingleObject(m_hupdatehandle, 2000);
	if(m_hupdatehandle)
	{
		CloseHandle(m_hupdatehandle);
		m_hupdatehandle = NULL;
	}

	if(m_pDataHandle)
	{
		delete m_pDataHandle;
		m_pDataHandle = NULL;
	}

	return true;
}

bool CBEOPOPCClientDlg::ChangeIcon()
{
	bool bIconOnline = true;
	map<int,COPCEngine*>::iterator iterOPC = m_mapOPCEngine.begin();
	while(iterOPC != m_mapOPCEngine.end())
	{
		bIconOnline = bIconOnline && (*iterOPC).second->GetOPCConnectState();
		iterOPC++;
	}
	if(bIconOnline != m_bIconOnline)
	{
		m_bIconOnline = bIconOnline;
		Shell_NotifyIcon(NIM_DELETE, &m_nid);//在托盘区添加图标
		if(bIconOnline)
		{
			m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON3);
			SetIcon(m_hIcon, TRUE);			// Set big icon
			SetIcon(m_hIcon, FALSE);		// Set small icon
			m_nid.uID = IDI_ICON3;
			m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON3));//加载图标
		}
		else
		{
			m_hIcon  = AfxGetApp()->LoadIcon(IDI_ICON2);
			SetIcon(m_hIcon, TRUE);			// Set big icon
			SetIcon(m_hIcon, FALSE);		// Set small icon
			m_nid.uID = IDI_ICON2;
			m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON2));//加载图标
		}
		Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标
	}
	return true;
}

bool CBEOPOPCClientDlg::UpdateDebug()
{
	wstring strPath;
	GetSysPath(strPath);
	CString strIniFilePath;
	strIniFilePath.Format(_T("%s\\beopopcclient.ini"),strPath.c_str());


	//初始化OPC参数
	wchar_t charContent[256];
	//////////////////////////////////////////////////////////////////////////
	wstring wstrUpdateMysql,wstrOPCCmdSleepFromDevice,wstrOPCPollSleep,wstrOPCPollSleepFromDevice,wstrReconnectMinute,wstrIDInterval,wstrEnableSecurity,wstrOPCMainThreadOPCPollSleep,wstrDisableCheckQuality,wstrModbusTimeout;
	GetPrivateProfileString(L"opc", L"opccmdsleepfromdevice", L"", charContent, 256, strIniFilePath);
	wstrOPCCmdSleepFromDevice = charContent;
	if(wstrOPCCmdSleepFromDevice == L"")
		wstrOPCCmdSleepFromDevice = L"1000";
	m_nOPCCmdSleepFromDevice = _wtoi(wstrOPCCmdSleepFromDevice.c_str());
	WritePrivateProfileString(L"opc",L"opccmdsleepfromdevice",wstrOPCCmdSleepFromDevice.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcpollsleep", L"", charContent, 256, strIniFilePath);
	wstrOPCPollSleep = charContent;
	if(wstrOPCPollSleep == L"")
		wstrOPCPollSleep = L"60";
	m_nOPCPollSleep = _wtoi(wstrOPCPollSleep.c_str());
	WritePrivateProfileString(L"opc",L"opcpollsleep",wstrOPCPollSleep.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcpollsleepfromdevice", L"", charContent, 256, strIniFilePath);
	wstrOPCPollSleepFromDevice = charContent;
	if(wstrOPCPollSleepFromDevice == L"")
		wstrOPCPollSleepFromDevice = L"60";
	m_nOPCPollSleepFromDevice = _wtoi(wstrOPCPollSleepFromDevice.c_str());
	WritePrivateProfileString(L"opc",L"opcpollsleepfromdevice",wstrOPCPollSleepFromDevice.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcreconnectignore", L"", charContent, 256, strIniFilePath);
	wstrReconnectMinute = charContent;
	if(wstrReconnectMinute == L"")
		wstrReconnectMinute = L"0";
	m_nReconnectMinute = _wtoi(wstrReconnectMinute.c_str());
	WritePrivateProfileString(L"opc",L"opcreconnectignore",wstrReconnectMinute.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"enableSecurity", L"", charContent, 256, strIniFilePath);
	wstrEnableSecurity = charContent;
	if(wstrEnableSecurity == L"")
		wstrEnableSecurity = L"0";
	m_bEnableSecurity = _wtoi(wstrEnableSecurity.c_str());
	WritePrivateProfileString(L"opc",L"enableSecurity",wstrEnableSecurity.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcmainpollsleep", L"", charContent, 256, strIniFilePath);
	wstrOPCMainThreadOPCPollSleep = charContent;
	if(wstrOPCMainThreadOPCPollSleep == L"")
		wstrOPCMainThreadOPCPollSleep = L"60";
	m_nOPCMainThreadOPCPollSleep = _wtoi(wstrOPCMainThreadOPCPollSleep.c_str());
	WritePrivateProfileString(L"opc",L"opcmainpollsleep",wstrOPCMainThreadOPCPollSleep.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opcreconnect", L"", charContent, 256, strIniFilePath);
	wstring strOPCReconnectInterval = charContent;
	if(strOPCReconnectInterval==L"")
		strOPCReconnectInterval = L"30";
	m_nOPCReconnectInertval = _wtoi(strOPCReconnectInterval.c_str());
	WritePrivateProfileString(L"opc",L"opcreconnect",strOPCReconnectInterval.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"opccheckreconnect", L"", charContent, 256, strIniFilePath);
	wstring strOPCCheckReconnect = charContent;
	if(strOPCCheckReconnect==L"")
		strOPCCheckReconnect = L"5";
	m_nOPCCheckReconnectInterval = _wtoi(strOPCCheckReconnect.c_str());
	WritePrivateProfileString(L"opc",L"opccheckreconnect",strOPCCheckReconnect.c_str(),strIniFilePath);

	GetPrivateProfileString(L"opc", L"debugopc", L"", charContent, 256, strIniFilePath);
	wstring wstrDebugOPC = charContent;
	if(wstrDebugOPC == L"")
		wstrDebugOPC = L"0";
	m_nDebugOPC = _wtoi(wstrDebugOPC.c_str());
	WritePrivateProfileString(L"opc",L"debugopc",wstrDebugOPC.c_str(),strIniFilePath);


	if(m_opcengine)
	{
		m_opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
	}

	map<int,COPCEngine*>::iterator iter = m_mapOPCEngine.begin();
	while(iter != m_mapOPCEngine.end())
	{
		iter->second->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
		++iter;
	}
	return true;
}
