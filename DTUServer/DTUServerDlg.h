
// DTUServerDlg.h : header file
//
#include "LogDlg.h"
#include "PointWatchDlg.h"
#include "SettingDlg.h"
#include "DTUProjectDlg.h"

#include "DTUServerHandlerPool.h"
#include "DTUServerHandler.h"
#include "ContentCache.h"

#pragma once


// CDTUServerDlg dialog
class CDTUServerDlg : public CDialog
{
// Construction
public:
	CDTUServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DTUSERVER_DIALOG };

	//更新当前在线的DTU
	void UpdateActiveDTUList();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void	CreateDlg();

	void SendData();

	bool	LoadConfig(vector<mapentry>& entrylist,bool bSendCmd);
	bool	Read(vector<mapentry>& entrylist,const std::wstring& filename);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonLog();
	afx_msg void OnBnClickedButtonPoint();
	afx_msg void OnBnClickedButtonSetting();
	afx_msg void OnNMThemeChangedListDtu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLbnSelchangeListDtu();
	afx_msg LRESULT ProcessData(WPARAM wparam, LPARAM lparam);

public:
	CSettingDlg		m_SettingDlg;
	CPointWatchDlg	m_PointWatchDlg;
	CLogDlg			m_LogDlg;
	CDTUProjectDlg	m_DTUProjectDlg;

	CListBox m_listactive_dtu;

	DTUServerHandlerPool m_dtuserver;
	CContentCache m_contentcache;

	int				m_nType;		//0:无 1:Log 2:Point 3:设置 
	bool			m_bSendCmd;
	string			m_strDTUServerIP;
	
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonTest();
};
