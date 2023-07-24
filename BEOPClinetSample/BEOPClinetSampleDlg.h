
// BEOPClinetSampleDlg.h : header file
//

#pragma once
#include "DataPointEntry.h"
#include "./db/DataHandler.h"
#define WM_SHOWTASK (WM_USER+20)	//托盘消息

// CBEOPClinetSampleDlg dialog
class CBEOPClinetSampleDlg : public CDialog
{
// Construction
public:
	CBEOPClinetSampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BEOPCLINETSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA	m_nid;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnShowTask(WPARAM wParam,LPARAM lParam); 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCloseClient();
	DECLARE_MESSAGE_MAP()

public:
	bool	Init();							//初始化信息
	bool	OutPutLogString(wstring strOut);			//输出log
	bool	Exit();
	bool	CloseSampleClient();

protected:
	wstring PreInitFileDirectory();
	bool	ReadFromCSV_Comma(CString sExcelFile, vector<ClientDataPointEntry>& vecPoint,int& nErrCode);		//处理值里带逗号的CSV
	static  unsigned int WINAPI UpdateMysqlThread(LPVOID lpVoid);						//更新数据库output表
	bool	UpdateMysqlValue();
	void	GetSampleValueSets(vector<pair<wstring, wstring> >& opcvaluesets);

public:
	bool						m_bFirst;
	vector<ClientDataPointEntry>	m_pointlist;					//点表
	bool						m_bExitThread;					//退出标志
	HANDLE						m_hupdatehandle;				//线程句柄
	bool						m_bCloseDlg;					//关闭标志
	int							m_nUpdateInterval;				//更新数据库间隔
	CDataHandler*				m_pDataHandle;				//数据库连接
};
