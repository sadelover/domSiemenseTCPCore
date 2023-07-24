#pragma once


// CAddDTUProjectDlg dialog

class CAddDTUProjectDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddDTUProjectDlg)

public:
	CAddDTUProjectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddDTUProjectDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DTUPROJECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strDTUName;
	CString m_strDTURemark;
	CString m_strHost;
	CString m_strUser;
	CString m_strPsw;
	CString m_strDataBase;
	afx_msg void OnBnClickedOk();
	BOOL m_bSendCmd;
};
