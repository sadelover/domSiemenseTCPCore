
// BEOPOPCClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BEOPOPCClient.h"
#include "BEOPOPCClientDlg.h"
#include "afxdialogex.h"
#include "BEOPOPCLink.h"
#include "CsvManager.h"

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
	m_nSleep(600000)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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

	// TODO: Add extra initialization here
	InitConfig();

	//初始化系统托盘图标
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
	m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));//加载图标
	wcscpy(m_nid.szTip, _T("DOMOPCSeed V1.0.1"));//信息提示
	Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标
	ShowWindow(SW_HIDE);//隐藏窗体  

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
	if(wParam!=IDR_MAINFRAME) 
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
			menu.AppendMenu(MF_STRING,WM_DESTROY,_T("Close(&C)"));
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
	ShowWindow(SW_HIDE);//隐藏窗体
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
		WriteLog(L"Init OPC Engine...");
		wstring strOPCServerIP = L"localhost";
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
					COPCEngine* opcengine = new COPCEngine(strOPCSIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality);
					opcengine->SetOpcPointList(opcpointlist_);
					opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
					m_mapOPCEngine[nOPCIndex] = opcengine;
					if (!InitOPC(opcengine,nOPCIndex))
					{
						CString strErr;
						strErr.Format(_T("ERROR: OPC Engine %d(Mutil:%s-%s) Start Failed!\r\n"),nOPCIndex,strOPCSIP.c_str(),opcpointlist_[0].GetParam(3).c_str());
						WriteLog(strErr);
					}
					//////////////////////////////////////////////////////////////////////////
				}
			}
		}
		else
		{
			//增加大批量OPC时分连接读取
			int nSize = opcpointlist.size();
			if(nSize <= m_nOPCClientMaxPoint)
			{
				m_opcengine = new COPCEngine(strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality);
				m_opcengine->SetOpcPointList(opcpointlist);
				m_opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
				if (!InitOPC(m_opcengine,0))
				{
					WriteLog(L"ERROR: OPC Engine Start Failed!");
					return false;
				}

				WriteLog(L"OPC Engine Connected Success.");
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
						COPCEngine* opcengine = new COPCEngine(strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality);
						opcengine->SetOpcPointList(opcpointlist_);
						opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
						m_mapOPCEngine[nOPCIndex] = opcengine;
						if (!InitOPC(opcengine,nOPCIndex))
						{
							CString strErr;
							strErr.Format(_T("ERROR: OPC Engine %d(Mutil) Start Failed!\r\n"),nOPCIndex);
							WriteLog(strErr);
							return false;
						}
						nOPCIndex++;
						opcpointlist_.clear();
					}
					opcpointlist_.push_back(opcpointlist[i]);
				}

				if(opcpointlist_.size() > 0)
				{
					COPCEngine* opcengine = new COPCEngine(strOPCServerIP,m_nOPCCmdSleep,m_nMutilCount,m_bMutilOPCClientUseThread,m_nOPCCmdSleepFromDevice,m_nOPCPollSleep,m_nOPCPollSleepFromDevice,m_bDisableCheckQuality);
					opcengine->SetOpcPointList(opcpointlist_);
					opcengine->SetDebug(m_nDebugOPC,m_nOPCCheckReconnectInterval,m_nOPCReconnectInertval,m_nReconnectMinute,m_bEnableSecurity,m_nOPCMainThreadOPCPollSleep);
					m_mapOPCEngine[nOPCIndex] = opcengine;
					if (!InitOPC(opcengine,nOPCIndex))
					{
						CString strErr;
						strErr.Format(_T("ERROR: OPC Engine %d(Mutil) Start Failed!\r\n"),nOPCIndex);
						WriteLog(strErr);
						return false;
					}
				}
			}
			WriteLog(L"OPC Engine(Mutil) Connected Success.");
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
	//初始化参数


	//初始化点表
	CCsvManager dlg(m_strExeDir + _T("opvValue.csv"));
	if (!dlg.GetFileInfo(m_opcpointlist))
	{
		return false;
	}


	//初始化COM 启用进程安全 因此不能线程读取OPC点
	wstring wstrEnableSecurity = m_strEnableSecurity;
	if(wstrEnableSecurity == L"1")
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

	//
	InitOPC();


	// create thread for receive
	HANDLE hThread = NULL;
	unsigned threadID = 0;
	hThread = (HANDLE)_beginthreadex(NULL, 0, &GetOpcValueThread, this, 0, &threadID);
	Sleep(0);
	CloseHandle(hThread);


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

unsigned __stdcall CBEOPOPCClientDlg::GetOpcValueThread(void* pArguments)
{
	CBEOPOPCClientDlg* pDlg = (CBEOPOPCClientDlg*)pArguments;
	ASSERT(pDlg != NULL);
	if (NULL == pDlg)
	{
		return 0;
	}

	vector<pair<wstring, wstring> > vecOpcVal;
	while (TRUE)
	{
		// get OPC value
		vecOpcVal.clear();
		pDlg->GetOPCValueSets(vecOpcVal);

		// set into mysql
		pDlg->WriteIntoRealTimeData(vecOpcVal);

		Sleep(pDlg->m_nSleep);
	}

	_endthreadex(0);
	return 0;
}

void CBEOPOPCClientDlg::WriteIntoRealTimeData(const vector<pair<wstring, wstring> >& vecOpc)
{
	// generate sql
	USES_CONVERSION;
	vector<pair<wstring, wstring> >::const_iterator	iter;
	CString	strSql(_T("REPLACE INTO `realtimedata_output`(pointname, pointvalue) VALUES"));
	for (iter=vecOpc.begin(); iter!=vecOpc.end(); iter++)
	{
		CString	strTemp;
		strTemp.Format(_T("('%s', '%s'),"), (iter->first).c_str(), (iter->second).c_str());
		strSql += strTemp;
	}
	strSql.TrimRight(_T(","));
	strSql += _T(";");

	try
	{
		if (NULL == mysql_init(&m_mysql))
		{
			WriteLog(_T("Init MySql Error !"));
			return;
		}
		if (NULL == mysql_real_connect(&m_mysql, T2A((LPCTSTR)m_stWriteInfo.strIp), T2A((LPCTSTR)m_stWriteInfo.strUserName), T2A((LPCTSTR)m_stWriteInfo.strPwd), T2A((LPCTSTR)m_stWriteInfo.strDbName), m_stWriteInfo.nPort, NULL, 0))
		{
			WriteLog(_T("Connect MySql Error !"));
			return;
		}
		WriteLog(_T("Run Sql: ") + strSql);

		char szBuf[300000] ={0};
		strcpy_s(szBuf, 300000, T2A(strSql.GetBuffer()));
		if (0 != mysql_real_query(&m_mysql, szBuf, strlen(szBuf)))
		{
			// error
			WriteLog(_T("Update MySql Error !"));
		}
		else
		{
			WriteLog(_T("Update MySql Success !"));
		}
		mysql_close(&m_mysql);
		strSql.ReleaseBuffer();
	}
	catch (...)
	{
		WriteLog(_T("MySql operate error throw !"));
		return;
	}

}

void CBEOPOPCClientDlg::InitConfig(void)
{
	TCHAR szAppDir[MAX_PATH] = {0};
	TCHAR szConfigDir[MAX_PATH] = {0};

	GetModuleFileName(NULL, szAppDir, MAX_PATH);
	PathRemoveFileSpec(szAppDir);
	PathCombine(szConfigDir, szAppDir, _T("BeopOpcCfg.ini"));

	m_strExeDir.Format(_T("%s\\"), szAppDir);
	m_strCfgPathName.Format(_T("%s"), szConfigDir);
	m_strLogDir.Format(_T("%s\\log\\"), szAppDir);
	InitConfigFile(m_strCfgPathName);
	ReadConfigInfo(m_strCfgPathName);
	CreateDir(m_strLogDir);
}

void CBEOPOPCClientDlg::InitConfigFile(const CString strPath)
{
	CFileFind file;
	if (!file.FindFile(strPath))
	{
		CFile fileNew;
		if (!fileNew.Open(strPath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite|CFile::shareDenyRead))
		{
			return;
		}
		try {
			char pbufWrite[] = {"\
[MySql]\r\n\
Ip=127.0.0.1\r\n\
Port=3306\r\n\
UserName=root\r\n\
Password=RNB.beop-2013\r\n\
DbName=beopdata\r\n\
Sleep=60\r\n\
\r\n\
[Opc]\r\n\
MaxPoint=10000\r\n\
MutilCount=20\r\n\
CmdSleep=5\r\n\
CmdSleepDev=50\r\n\
PollSleep=5\r\n\
PollSleepDev=60\r\n\
ReconnectMin=0\r\n\
SendRemoteSet=0\r\n\
MutilOpcClient=0\r\n\
MutilOpcClientThread=0\r\n\
StoreHistory=0\r\n\
EnableSecurity=0\r\n\
CheckQuality=0\r\n\
OpcPollSleep=2\r\n\
OpcCheckInterval=5\r\n\
OpcReconnectInterval=10\r\n\
DebugOpc=0\r\n\
strEnableSecurity=0\r\n\
"};
			int len = strlen(pbufWrite);
			fileNew.Write(pbufWrite, len);         
			fileNew.Flush();
			fileNew.Close();
		}
		catch (...)
		{
			return;
		}
	}
}

void CBEOPOPCClientDlg::ReadConfigInfo(const CString strPath)
{
	CString strAppName(_T("MySql"));
	CString strIp;
	int		nPort = 0;
	CString strUser;
	CString strPwd;
	CString strDbName;

	TCHAR	szTemp[MAX_PATH] = {0};
	memset(szTemp, 0, sizeof(szTemp));
	GetPrivateProfileString(strAppName, _T("Ip"), NULL, szTemp, MAX_PATH, strPath);
	strIp.Format(_T("%s"), szTemp);

	nPort = GetPrivateProfileInt(strAppName, _T("Port"), NULL, strPath);

	memset(szTemp, 0, sizeof(szTemp));
	GetPrivateProfileString(strAppName, _T("UserName"), NULL, szTemp, MAX_PATH, strPath);
	strUser.Format(_T("%s"), szTemp);

	memset(szTemp, 0, sizeof(szTemp));
	GetPrivateProfileString(strAppName, _T("Password"), NULL, szTemp, MAX_PATH, strPath);
	strPwd.Format(_T("%s"), szTemp);

	memset(szTemp, 0, sizeof(szTemp));
	GetPrivateProfileString(strAppName, _T("DbName"), NULL, szTemp, MAX_PATH, strPath);
	strDbName.Format(_T("%s"), szTemp);

	m_stWriteInfo.strIp = strIp;
	m_stWriteInfo.nPort = nPort;
	m_stWriteInfo.strUserName = strUser;
	m_stWriteInfo.strPwd = strPwd;
	m_stWriteInfo.strDbName = strDbName;

	m_nSleep = GetPrivateProfileInt(strAppName, _T("Sleep"), NULL, strPath);
	m_nSleep *= 1000;


	// Opc part
	strAppName = _T("Opc");
	m_nOPCClientMaxPoint = GetPrivateProfileInt(strAppName, _T("MaxPoint"), NULL, strPath);
	m_nMutilCount = GetPrivateProfileInt(strAppName, _T("MutilCount"), NULL, strPath);
	m_nOPCCmdSleep = GetPrivateProfileInt(strAppName, _T("CmdSleep"), NULL, strPath);
	m_nOPCCmdSleepFromDevice = GetPrivateProfileInt(strAppName, _T("CmdSleepDev"), NULL, strPath);
	m_nOPCPollSleep = GetPrivateProfileInt(strAppName, _T("PollSleep"), NULL, strPath);
	m_nOPCPollSleepFromDevice = GetPrivateProfileInt(strAppName, _T("PollSleepDev"), NULL, strPath);
	m_nReconnectMinute = GetPrivateProfileInt(strAppName, _T("ReconnectMin"), NULL, strPath);
	m_nSendRemoteSet = GetPrivateProfileInt(strAppName, _T("SendRemoteSet"), NULL, strPath);
	m_bMutilOPCClientMode = GetPrivateProfileInt(strAppName, _T("MutilOpcClient"), NULL, strPath);
	m_bMutilOPCClientUseThread = GetPrivateProfileInt(strAppName, _T("MutilOpcClientThread"), NULL, strPath);
	m_bStoreHistory = GetPrivateProfileInt(strAppName, _T("StoreHistory"), NULL, strPath);
	m_bEnableSecurity = GetPrivateProfileInt(strAppName, _T("EnableSecurity"), NULL, strPath);
	m_bDisableCheckQuality = GetPrivateProfileInt(strAppName, _T("CheckQuality"), NULL, strPath);
	m_nOPCMainThreadOPCPollSleep = GetPrivateProfileInt(strAppName, _T("OpcPollSleep"), NULL, strPath);
	m_nOPCCheckReconnectInterval = GetPrivateProfileInt(strAppName, _T("OpcCheckInterval"), NULL, strPath);
	m_nOPCReconnectInertval = GetPrivateProfileInt(strAppName, _T("OpcReconnectInterval"), NULL, strPath);
	m_nDebugOPC = GetPrivateProfileInt(strAppName, _T("DebugOpc"), NULL, strPath);

	memset(szTemp, 0, sizeof(szTemp));
	GetPrivateProfileString(strAppName, _T("strEnableSecurity"), NULL, szTemp, MAX_PATH, strPath);
	m_strEnableSecurity = szTemp;
}

int CBEOPOPCClientDlg::CreateDir(CString strFolderPath)
{
	HANDLE			hFile;
	WIN32_FIND_DATA	fileinfo;
	CStringArray m_arr;// To hold Directory Structures
	BOOL			bRet = FALSE;
	int				nIndex = 0;
	CString			strTemp;


	hFile = FindFirstFile(strFolderPath, &fileinfo);

	// if the file exists and it is a directory
	if(fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(hFile);
		return 2;
	}
	FindClose(hFile);

	// Parse the supplied CString Directory String
	m_arr.RemoveAll();
	int nLength = strFolderPath.GetLength();

	for(nIndex = 0; nIndex < nLength; nIndex++)
	{
		// If the character is not a '\', add it to csTemp
		if(strFolderPath.GetAt(nIndex) != '\\')
		{
			strTemp += strFolderPath.GetAt(nIndex);
		}
		else
		{
			m_arr.Add(strTemp);
			strTemp += '\\';
		}
		// If we reached the end of the file add the remaining string
		if(nIndex == (nLength - 1))
		{
			m_arr.Add(strTemp);
		}
	}

	for(nIndex = 1; nIndex < m_arr.GetSize(); nIndex++)
	{
		strTemp = m_arr.GetAt(nIndex);

		bRet = CreateDirectory(strTemp, NULL);

		if(TRUE == bRet)
		{
			SetFileAttributes(strTemp, FILE_ATTRIBUTE_NORMAL);
		}
	}
	m_arr.RemoveAll();

	hFile = FindFirstFile(strFolderPath, &fileinfo);

	if(fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(hFile);
		return 1;
	}
	else
	{
		FindClose(hFile);
		return 0;
	}
}

void CBEOPOPCClientDlg::WriteLog(CString strLog)
{
	SYSTEMTIME	sys; 
	TCHAR		szTime[MAX_PATH] = {};
	FILE		*stream;

	GetLocalTime(&sys);
	_stprintf_s(szTime, MAX_PATH, _T("%s%d-%d-%d.txt"), (LPTSTR)(LPCTSTR)m_strLogDir, sys.wYear, sys.wMonth, sys.wDay);
	if (0 != _tfopen_s(&stream, szTime, _T("a+")))
	{
		return;
	}

	CString strTimeNow;
	strTimeNow.Format(_T("%02d:%02d:%02d"), sys.wHour, sys.wMinute, sys.wSecond);
	_ftprintf_s(stream, _T("%s  "), strTimeNow);
	_ftprintf_s(stream, _T("%s\n"), strLog);

	fclose(stream);
}
