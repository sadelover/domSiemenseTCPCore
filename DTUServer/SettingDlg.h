#pragma once

// CSettingDlg dialog
#include <vector>

class CSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTING };

	void LoadNewPointCongig(CString strNewDBFileName);

	void SetDTUInfoByDTUName(string strName);

	void WriteS3db(string strName);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonCancle();

private:
	CString m_strDTUName;
	CString m_strDTURemark;
	CString m_strIP;
	CString m_strPsw;
	CString m_strUser;
	CString m_strSchema;
	CString m_strDTUConfigTableName;

	string  m_strDTU;
	vector<sPoint> m_vecPoint;
	BOOL	m_bEdit;
};
