#pragma once
#include "afxcmn.h"
#include "DTUAddressMap.h"

// CDTUProjectDlg dialog

class CDTUProjectDlg : public CDialog
{
	DECLARE_DYNAMIC(CDTUProjectDlg)

public:
	CDTUProjectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDTUProjectDlg();
	virtual BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_DIALOG_PROJECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:	
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonSave();

	bool	Read(const std::wstring& filename);
	bool	Save(const std::wstring& filename);

	bool	InitList();
	bool	RefreshList();

public:
	CListCtrl m_listProject;
	vector<mapentry>	m_vecProject;
	CString				m_strFilePath;
	string			m_strDTUServerIP;
	afx_msg void OnBnClickedButtonCreateDb();
};
