// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DTUServer.h"
#include "LogDlg.h"
#include "afxdialogex.h"


// CLogDlg dialog

IMPLEMENT_DYNAMIC(CLogDlg, CDialog)

CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
	, m_strLog(_T(""))
	, m_bShowLog(false)
	, m_strDTUName("")
	, m_bExitThread(false)
{

}

CLogDlg::~CLogDlg()
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_eLog);
	DDX_Text(pDX, IDC_EDIT_LOG, m_strLog);
	DDX_Control(pDX, IDC_CHECK_AUTOSCROLL, m_CheckAlwaysLog);
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_AUTOSCROLL, &CLogDlg::OnBnClickedCheckAutoscroll)
END_MESSAGE_MAP()


// CLogDlg message handlers

void CLogDlg::OnBnClickedCheckAutoscroll()
{
	// TODO: Add your control notification handler code here
	if(m_CheckAlwaysLog.GetCheck())
	{
		m_bShowLog = true;
	}
	else
	{
		m_bShowLog = false;
	}
}

DWORD WINAPI CLogDlg::ThreadFuncLog( LPVOID lpParameter )
{
	CLogDlg* pDlg = (CLogDlg*)lpParameter;
	while(TRUE)
	{
		Sleep(1000);

		if (pDlg->m_bExitThread)
		{
			return 0;
		}

		if(pDlg->IsWindowVisible()==false)
		{
			continue;
		}

		pDlg->RefreshLog();
	}
	return 0;
}

void CLogDlg::ExitThreads()
{

	m_bExitThread = true;
}

void CLogDlg::UpdateLog()
{

	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	if(m_strDTUName == "")
		return;

	CDTUServerHandler* phandle = m_handlelist.GetHandlerByDTUName(m_strDTUName);
	if(phandle)
	{
		m_strLog = phandle->GetLog();
		m_eLog.SetWindowTextW(m_strLog);
		UpdateData(FALSE);
	}
}

BOOL CLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_hThreadLog = CreateThread(NULL, 0, ThreadFuncLog, this, 0, NULL); 

	return TRUE;
}

void CLogDlg::SetLogInfoByDTUName( string strName )
{
	m_strDTUName = strName;
	InitLog();
}

void CLogDlg::SetDTUServerHandlerPool( DTUServerHandlerPool handlelist )
{
	m_handlelist = handlelist;
}

bool CLogDlg::RefreshLog()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	if(m_bShowLog)
	{		
		if(m_strDTUName == "")
			return false;

		CDTUServerHandler* phandle = m_handlelist.GetHandlerByDTUName(m_strDTUName);
		if(phandle)
		{
			m_strLog = phandle->GetLog();
			m_eLog.SetWindowTextW(m_strLog);
			//UpdateData(FALSE);
		}
	}

	return true;
}

bool CLogDlg::InitLog()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	m_eLog.SetWindowTextW(_T(""));
		
	if(m_strDTUName == "")
			return false;

	CDTUServerHandler* phandle = m_handlelist.GetHandlerByDTUName(m_strDTUName);
	if(phandle)
	{
		m_strLog = phandle->GetLog();
		m_eLog.SetWindowTextW(m_strLog);
	}
	
	return true;
}
