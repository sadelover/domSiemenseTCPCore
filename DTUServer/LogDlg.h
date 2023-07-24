#pragma once
#include "afxwin.h"
#include "DTUServerHandlerPool.h"

// CLogDlg dialog

class CLogDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_LOG };

	static DWORD WINAPI ThreadFuncLog(LPVOID lpParameter);
	void  ExitThreads();
	void	UpdateLog();
	bool	RefreshLog();
	bool	InitLog();

	void SetLogInfoByDTUName(string strName);
	void SetDTUServerHandlerPool(DTUServerHandlerPool handlelist);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckAutoscroll();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	bool	m_bShowLog;

	HANDLE  m_hThreadLog;
	BOOL    m_bExitThread;
	CEdit m_eLog;
	CString m_strLog;
	CButton m_CheckAlwaysLog;
	string	m_strDTUName;
	DTUServerHandlerPool m_handlelist;
	Project::Tools::Mutex m_lock;
};
