#pragma once


// CValueChangeDlg dialog

class CValueChangeDlg : public CDialog
{
	DECLARE_DYNAMIC(CValueChangeDlg)

public:
	CValueChangeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CValueChangeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_VALUECHANGE };

	void	SetName(CString strName);
	void	SetValue(CString strValue);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strValue;
	CString m_strName;
	afx_msg void OnBnClickedOk();
};
