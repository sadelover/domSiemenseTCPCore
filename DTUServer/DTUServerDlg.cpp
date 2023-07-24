
// DTUServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DTUServer.h"
#include "DTUServerDlg.h"
#include "afxdialogex.h"

#include "wcomm_dll.h"
#include "RestartAutoRun.h"
#include "DTUAddressMap.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../BEOPDataPoint/sqlite/SqliteAcess.h"
#include <iostream>
#include <sstream>
#include "TinyXml/tinyxml.h"
#include "TinyXml/tinystr.h"

#pragma comment(lib,"wcomm_dll.lib")

using std::string;
using std::wstring;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  DTU_MSG WM_USER+100
#define DTU_OFFLINE_INTERNAL 120
#define UPDATE_DTU_ONLINE_TIMER 1
#define UPDATE_DTU_ONLINE_TIMER_INTERNAL 2*1000		//2秒更新一次。

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


// CDTUServerDlg dialog




CDTUServerDlg::CDTUServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDTUServerDlg::IDD, pParent)
	, m_nType(0)
	, m_bSendCmd(true)
	, m_strDTUServerIP("")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDTUServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DTU, m_listactive_dtu);
}

BEGIN_MESSAGE_MAP(CDTUServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOG, &CDTUServerDlg::OnBnClickedButtonLog)
	ON_BN_CLICKED(IDC_BUTTON_Point, &CDTUServerDlg::OnBnClickedButtonPoint)
	ON_BN_CLICKED(IDC_BUTTON_SETTING, &CDTUServerDlg::OnBnClickedButtonSetting)
	ON_NOTIFY(NM_THEMECHANGED, IDC_LIST_DTU, &CDTUServerDlg::OnNMThemeChangedListDtu)
	ON_LBN_SELCHANGE(IDC_LIST_DTU, &CDTUServerDlg::OnLbnSelchangeListDtu)
	ON_MESSAGE(DTU_MSG, ProcessData)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CDTUServerDlg::OnBnClickedButtonTest)
END_MESSAGE_MAP()


// CDTUServerDlg message handlers

BOOL CDTUServerDlg::OnInitDialog()
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
	CString cstrFile,strPath,strConfigPath;
	Project::Tools::GetSysPath(strPath);
	cstrFile = strPath + L"\\beopgateway.ini";
	wchar_t charServerIP[1024];
	GetPrivateProfileString(L"dtuserver", L"ip", L"", charServerIP, 1024, cstrFile.GetString());
	wstring wstrServerIP = charServerIP;

	wchar_t charSendcmd[1024];
	GetPrivateProfileString(L"dtuserver", L"sendcmd", L"", charSendcmd, 1024, cstrFile.GetString());
	wstring wstrSendcmd = charSendcmd;
	if(wstrSendcmd == L"" || wstrSendcmd == L"0")
		m_bSendCmd = false;

	vector<mapentry> entrylist;
	//LoadConfig(entrylist,m_bSendCmd);
	strConfigPath = strPath + L"\\DTUServerConfig.dsc";
	Read(entrylist,strConfigPath.GetString());

	DTUAddressMap::GetInstance()->SetConfigList(entrylist);
	//创建 处理handle
	for(int i=0; i<entrylist.size(); i++)
	{
		m_dtuserver.CreateDTUServerHandler(entrylist[i]);
	}

	SetWorkMode(2);
	////选择UDP协议
	SelectProtocol(0);
	////设置IP地址
	
	if(m_strDTUServerIP.length() >7)
	{
		SetCustomIP(inet_addr(m_strDTUServerIP.c_str()));	
	}

	char mess[512];
	int result = start_net_service(m_hWnd, DTU_MSG, 9500, mess);
	//m_dtuserver.Init();
	SetTimer(UPDATE_DTU_ONLINE_TIMER, UPDATE_DTU_ONLINE_TIMER_INTERNAL, NULL);

	CreateDlg();

	TCHAR exepath[MAX_PATH];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CRestartAutoRun::SetAutoRun(exepath);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDTUServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDTUServerDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDTUServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDTUServerDlg::OnBnClickedButtonLog()
{
	// TODO: Add your control notification handler code here
	m_DTUProjectDlg.ShowWindow(FALSE);
	m_PointWatchDlg.ShowWindow(FALSE);
	m_LogDlg.ShowWindow(TRUE);
	m_nType = 1;
}

void CDTUServerDlg::OnBnClickedButtonPoint()
{
	// TODO: Add your control notification handler code here
	m_DTUProjectDlg.ShowWindow(FALSE);
	m_PointWatchDlg.ShowWindow(TRUE);
	m_LogDlg.ShowWindow(FALSE);
	m_nType = 2;
}

void CDTUServerDlg::OnBnClickedButtonSetting()
{
	// TODO: Add your control notification handler code here
	m_DTUProjectDlg.ShowWindow(TRUE);
	m_PointWatchDlg.ShowWindow(FALSE);
	m_LogDlg.ShowWindow(FALSE);
	m_nType = 3;
}

void CDTUServerDlg::CreateDlg()
{
	CRect rc;
	GetClientRect(rc);
	rc.top+=90;
	rc.left+=245;
	rc.right-=15;
	rc.bottom-=5;

	m_DTUProjectDlg.Create(IDD_DIALOG_PROJECT, this);
	m_DTUProjectDlg.MoveWindow(rc);
	m_DTUProjectDlg.ShowWindow(FALSE);

	m_PointWatchDlg.Create(IDD_DIALOG_POINT,this);
	m_PointWatchDlg.SetDTUServerHandlerPool(m_dtuserver);
	m_PointWatchDlg.MoveWindow(rc);
	m_PointWatchDlg.ShowWindow(FALSE);

	m_LogDlg.Create(IDD_DIALOG_LOG, this);
	m_LogDlg.SetDTUServerHandlerPool(m_dtuserver);
	m_LogDlg.MoveWindow(rc);
	m_LogDlg.ShowWindow(FALSE);
}

void CDTUServerDlg::UpdateActiveDTUList()
{
	int i = 0;
	int maxdtuamount = 0;
	user_info ui;
	time_t t_now, t_update;

	vector<string> activelist;
	maxdtuamount = get_max_user_amount();
	for (i = 0; i < maxdtuamount; i++)
	{
		memset(&ui, 0x00, sizeof(user_info));
		get_user_at(i, &ui);
		if (1 == ui.m_status){
			t_now = time(NULL);
			t_update = *(time_t*)ui.m_update_time;
			int diffinternal = t_now - t_update;
			CString str_debug;
			str_debug.Format(_T("%d"), diffinternal);
			//ADEBUG(str_debug);
			if (diffinternal >= DTU_OFFLINE_INTERNAL){
				do_close_one_user((unsigned char*)ui.m_userid, NULL);
			}

			activelist.push_back(ui.m_userid);
		}
	}

	m_listactive_dtu.ResetContent();
	for (unsigned int i = 0; i < activelist.size(); i++){
		wstring str_unicode = Project::Tools::AnsiToWideChar(activelist[i].c_str());
		m_listactive_dtu.AddString(str_unicode.c_str());
	}
}

void CDTUServerDlg::OnNMThemeChangedListDtu(NMHDR *pNMHDR, LRESULT *pResult)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.
	// TODO: Add your control notification handler code here


	*pResult = 0;
}

void CDTUServerDlg::OnLbnSelchangeListDtu()
{
	// TODO: Add your control notification handler code here
	switch(m_nType)
	{
	case 1:		//log
		{
			CString str;
			m_listactive_dtu.GetText(m_listactive_dtu.GetCurSel(),str);
			m_LogDlg.SetLogInfoByDTUName(Project::Tools::WideCharToAnsi(str.GetString()));
			break;
		}		
	case 2:		//point
		{
			CString str;
			m_listactive_dtu.GetText(m_listactive_dtu.GetCurSel(),str);
			m_PointWatchDlg.SetPointInfoByDTUName(Project::Tools::WideCharToAnsi(str.GetString()));
			break;
		}		
	case 3:		//setting
		{
			
		}
		break;
	default:
		break;
	}
}

void CDTUServerDlg::OnTimer( UINT_PTR nIDEvent )
{
	if (nIDEvent == UPDATE_DTU_ONLINE_TIMER)
	{
		UpdateActiveDTUList();
	}

	CDialog::OnTimer(nIDEvent);
}

void CDTUServerDlg::SendData()
{
	unsigned char strbuf[1024];
	char userid[12];
	int	sendlen;

	////获取DTU标识符
	//GetUserId(userid);
	////设置需要发送的数据
	//sendlen = SetSendData(strbuf);
	//执行发送
	do_send_user_data((unsigned char *)userid, strbuf, sendlen, NULL);
}

LRESULT CDTUServerDlg::ProcessData( WPARAM wparam, LPARAM lparam )
{
	data_record record;
	int ret = do_read_proc(&record, NULL, false);
	if (ret == 0){
		switch(record.m_data_type){
		case 0x09:
			{
				// get what project the data are from.
				CDTUServerHandler* phandle = m_dtuserver.GetHandlerByDTUName(record.m_userid);
				// this should be wrapped to the class dtuhandlerserver.
				if (phandle != NULL)
				{
					phandle->SetData(record.m_data_buf, record.m_data_len, record.m_userid);
				}
			}
			break;
		default:
			break;
		}
	}

	return 0;
}

bool CDTUServerDlg::LoadConfig(vector<mapentry>& entrylist,bool bSendCmd)
{
	entrylist.clear();

	CString strPath = _T("");
	TCHAR szExePath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szExePath,MAX_PATH);

	strPath = szExePath;
	strPath = strPath.Left(strPath.ReverseFind('\\'));
	strPath = strPath + _T("\\beop.s3db");

	string strUtf8;
	Project::Tools::WideCharToUtf8(strPath.GetString(), strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	string sql_;
	char *ex_sql;

	//Read dtu_config
	sqlstream.str("");
	sqlstream << "select * from dtu_config;";
	sql_ = sqlstream.str();
	ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		mapentry entry;
		entry.bSendData = bSendCmd;
		entry.dtuid = access.getColumn_Int(0);
		if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
		{
			string   name_temp(const_cast<char*>(access.getColumn_Text(1)));
			entry.dtuname = name_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(2)) !=NULL)
		{
			string   t_temp(const_cast<char*>(access.getColumn_Text(2)));
			entry.remark = t_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(3)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(3)));
			entry.ip = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(4)) !=NULL)
		{
			string   r_temp(const_cast<char*>(access.getColumn_Text(4)));
			entry.user = r_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(5)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(5)));
			entry.psw = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(6)) !=NULL)
		{
			string   r_temp(const_cast<char*>(access.getColumn_Text(6)));
			entry.databasename = r_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(7)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(7)));
			entry.configpointtable = d_temp;
		}
		entrylist.push_back(entry);
	}
	access.SqlFinalize();	
	return true;
}

void CDTUServerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_PointWatchDlg.ExitThreads();

	CDialog::OnClose();
}

void CDTUServerDlg::OnBnClickedButtonTest()
{
	// TODO: Add your control notification handler code here
	CDTUServerHandler* phandle = new CDTUServerHandler();
	if(phandle)
	{
		char* newbuffer = "2014-05-26 17:00:00;1,point1,22;1,point2,33;2,point3,33;";

		char* cc = _strdup(newbuffer);
		phandle->AnalyzeRealData(cc);
	}



}

bool CDTUServerDlg::Read( vector<mapentry>& entrylist,const std::wstring& filename )
{
	entrylist.clear();
	TiXmlDocument xmlDoc;
	const string strFileName = Project::Tools::WideCharToAnsi(filename.c_str());;
	const bool ifload = xmlDoc.LoadFile(strFileName.c_str() );
	if(!ifload)
		return false;

	TiXmlElement* pElemRoot = xmlDoc.RootElement();  //project
	ASSERT(pElemRoot);
	if(!pElemRoot)
		return false;

	m_strDTUServerIP = 	pElemRoot->Attribute("dscip");

	TiXmlElement* pElemProject = pElemRoot->FirstChildElement("project");
	while(pElemProject)
	{
		string strDTUName,strDTUReamark,strPsw,strUser,strDBName,strHost;
		int nPort = 3306;
		int nSendData = 0;
		mapentry project;
		project.dtuname = pElemProject->Attribute("dtuname");
		project.ip = pElemProject->Attribute("dbip");
		project.user = pElemProject->Attribute("dbuser");
		project.databasename = pElemProject->Attribute("dbname");
		project.psw = pElemProject->Attribute("dbpsw");
		project.remark = pElemProject->Attribute("dtuRemark");
		pElemProject->Attribute("port",&nPort);
		pElemProject->Attribute("bSendData",&nSendData);
		project.bSendData = nSendData;
		entrylist.push_back(project);
		pElemProject = pElemProject->NextSiblingElement();
	}

	return true;
}
