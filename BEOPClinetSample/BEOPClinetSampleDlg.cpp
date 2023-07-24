
// BEOPClinetSampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BEOPClinetSample.h"
#include "BEOPClinetSampleDlg.h"
#include "afxdialogex.h"
#include "CSVParser.h"
#include "RestartAutoRun.h"
using namespace CSVParser;

/*
	待修改：
		1：程序名称及配置文件,log名称
		2：点表参数及类型需要注意
		3: Init 函数里需要初始化参数及引擎
		4: GetSampleValueSets 获取引擎数据
		5: Exit 函数需要进行各种退出释放
		6：BEOPClinetSample.cpp里面的程序名
*/

#define SAMPLE_CLIENT_NAME	_T("BEOPClientSample V1.0.1")
#define SAMPLE_INI_NAME		_T("beopclientsample.ini")
#define SAMPLE_LOG_NAME		_T("beopsapmpleclient.log")


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


// CBEOPClinetSampleDlg dialog




CBEOPClinetSampleDlg::CBEOPClinetSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBEOPClinetSampleDlg::IDD, pParent)
	, m_bFirst(TRUE),
	m_bExitThread(false),
	m_hupdatehandle(NULL),
	m_bCloseDlg(false),
	m_nUpdateInterval(2),
	m_pDataHandle(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBEOPClinetSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBEOPClinetSampleDlg, CDialog)
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


// CBEOPClinetSampleDlg message handlers

BOOL CBEOPClinetSampleDlg::OnInitDialog()
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
	//初始化系统托盘图标
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称
	m_nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));//加载图标
	wcscpy(m_nid.szTip, SAMPLE_CLIENT_NAME);			//信息提示  edit
	Shell_NotifyIcon(NIM_ADD, &m_nid);//在托盘区添加图标
	ShowWindow(SW_HIDE);//隐藏窗体  

	//写注册表 开机启动
	TCHAR exepath[MAX_PATH];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CRestartAutoRun::SetAutoRun(exepath);

	//初始化
	Init();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBEOPClinetSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBEOPClinetSampleDlg::OnPaint()
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
HCURSOR CBEOPClinetSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CBEOPClinetSampleDlg::OnShowTask( WPARAM wParam,LPARAM lParam )
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

BOOL CBEOPClinetSampleDlg::OnEraseBkgnd( CDC* pDC )
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

void CBEOPClinetSampleDlg::OnClose()
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

void CBEOPClinetSampleDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	//删除托盘图标
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

void CBEOPClinetSampleDlg::OnTimer( UINT_PTR nIDEvent )
{
	CDialog::OnTimer(nIDEvent);
}

void CBEOPClinetSampleDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if(nType == SIZE_MINIMIZED)
	{
		ShowWindow(SW_HIDE);
	}
}

BOOL CBEOPClinetSampleDlg::PreTranslateMessage( MSG* pMsg )
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

bool CBEOPClinetSampleDlg::Init()
{
	wstring strPath;
	GetSysPath(strPath);
	CString strIniFilePath;
	strIniFilePath.Format(_T("%s\\%s"),strPath.c_str(),SAMPLE_INI_NAME);				//edit

	//初始化数据库参数
	/*
	wchar_t charContent[256];
	GetPrivateProfileString(L"db", L"defaulathost", L"", charContent, 256, strIniFilePath);
	wstring wstrDefaultHost = charContent;
	if(wstrDefaultHost == L"")
		wstrDefaultHost = L"localhost";
	WritePrivateProfileString(L"db",L"defaulathost",wstrDefaultHost.c_str(),strIniFilePath);
	*/

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
	if(!ReadFromCSV_Comma(strCSVPath.c_str(),m_pointlist,nErrCode))
	{
		OutPutLogString(L"ERROR:Read Point CSV Fail!");
		MessageBox(_T("ERROR: Read Point CSV Fail!"));
		EndDialog(0);
		return 0;
	}

	//初始化数据库
	wstring wstrDefaultHost,wstrDefaultUser,wstrDefaultPsw,wstrDefaultDB;
	m_pDataHandle = new CDataHandler();
	if(m_pDataHandle)
	{
		if(!m_pDataHandle->Connect(wstrDefaultHost,wstrDefaultUser,wstrDefaultPsw,wstrDefaultDB,3306))
		{
			MessageBox(_T("ERROR: Connect DB Fail!"));
			EndDialog(0);
			return 0;
		}
	}

	//初始化引擎

	//启动线程
	m_hupdatehandle = (HANDLE)_beginthreadex(NULL, 0, UpdateMysqlThread, this, 0, NULL);
	return true;
}

bool CBEOPClinetSampleDlg::ReadFromCSV_Comma( CString sExcelFile, vector<ClientDataPointEntry>& vecPoint,int& nErrCode )
{
	try
	{
		Csv csv = Csv(Project::Tools::WideCharToAnsi(sExcelFile).c_str());
		//
		if(csv.getRowCount() >0)
		{
			std::map<CString,int> mapPointName;
			for(int i=0; i<csv.getRowCount(); ++i)
			{
				//每一行
				vector<wstring> vecPointInfo;
				ClientDataPointEntry entry;
				CString strPhysicalID = Project::Tools::AnsiToWideChar(csv[i][0].c_str()).c_str();
				CString strparam1 = Project::Tools::AnsiToWideChar(csv[i][1].c_str()).c_str();
				CString strparam2 = Project::Tools::AnsiToWideChar(csv[i][2].c_str()).c_str();
				CString strparam3 = Project::Tools::AnsiToWideChar(csv[i][3].c_str()).c_str();
				CString strparam4 = Project::Tools::AnsiToWideChar(csv[i][4].c_str()).c_str();
				entry.SetShortName(strPhysicalID.GetString());			//edit
				entry.SetSourceType(L"OPC");

				entry.SetParam(1, strparam1.GetString());
				entry.SetParam(2, strparam2.GetString());
				entry.SetParam(3, strparam3.GetString());
				entry.SetParam(4, strparam4.GetString());
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

wstring CBEOPClinetSampleDlg::PreInitFileDirectory()
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

unsigned int WINAPI CBEOPClinetSampleDlg::UpdateMysqlThread( LPVOID lpVoid )
{
	CBEOPClinetSampleDlg* pUpdateValueHandle = reinterpret_cast<CBEOPClinetSampleDlg*>(lpVoid);
	if (NULL == pUpdateValueHandle)
	{
		return 0;
	}

	while(!pUpdateValueHandle->m_bExitThread)
	{		
		pUpdateValueHandle->UpdateMysqlValue();
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

bool CBEOPClinetSampleDlg::UpdateMysqlValue()
{
	vector<pair<wstring, wstring> >	valuesets;
	GetSampleValueSets(valuesets);
	if(m_pDataHandle && m_pDataHandle->IsConnected())
	{
		return m_pDataHandle->UpdateOutPutValue(valuesets);
	}
	return false;
}

void CBEOPClinetSampleDlg::GetSampleValueSets( vector<pair<wstring, wstring> >& opcvaluesets )
{
	
}

bool CBEOPClinetSampleDlg::OutPutLogString( wstring strLog )
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
		strLogPath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),SAMPLE_LOG_NAME);			//edit

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

void CBEOPClinetSampleDlg::OnCloseClient()
{
	if(CloseSampleClient())
	{
		m_bCloseDlg = true;
		SendMessage(WM_CLOSE);
	}
}

bool CBEOPClinetSampleDlg::CloseSampleClient()
{
	return Exit();
}

bool CBEOPClinetSampleDlg::Exit()
{
	m_bExitThread = true;
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