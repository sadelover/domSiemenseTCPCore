#pragma once
#include "afxcmn.h"
#include "DTUServerHandlerPool.h"

// CPointWatchDlg dialog

class CPointWatchDlg : public CDialog
{
	DECLARE_DYNAMIC(CPointWatchDlg)

public:
	CPointWatchDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPointWatchDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_POINT };

	void SetPointInfoByDTUName(string strName);

	void SetDTUServerHandlerPool(DTUServerHandlerPool handlelist);

	void RefreshByDTUName(string strName);

	void InitPointByDTUName(string strName);

	void StartThreads();

	static DWORD WINAPI ThreadFuncPoint(LPVOID lpParameter);
	void  ExitThreads();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ListAll;
	CString m_strNameSearch;
	int m_nCurSearchIndex;
	string	m_strDTUName;
	DTUServerHandlerPool m_handlelist;
	vector<spointwatch> m_vecEntrylist;
	HANDLE  m_hThreadPoint;
	BOOL    m_bExitThread;

	Project::Tools::Mutex m_lock;

	afx_msg void OnBnClickedButtonSearchPrev();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnLvnItemchangedListAll(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListAll(NMHDR *pNMHDR, LRESULT *pResult);
};
