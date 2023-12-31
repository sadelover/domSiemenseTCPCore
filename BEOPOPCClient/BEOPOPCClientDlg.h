
// BEOPOPCClientDlg.h : header file
//

#pragma once
#include "./OPCCtrl/BEOPOPCCtrl.h"
#include "DataPointEntry.h"
#define WM_SHOWTASK (WM_USER+20)	//托盘消息

class COPCEngine;

struct OPCServer
{
	wstring strIP;
	wstring strServerName;
	int		nCount;		
};

typedef struct _tagWriteNetCfg
{
	CString	strIp;
	int		nPort;
	CString	strUserName;
	CString	strPwd;
	CString	strDbName;

	_tagWriteNetCfg(void)
	{
		strIp = _T("127.0.0.1");
		nPort = 3306;
		strUserName = _T("root");
		strPwd = _T("RNB.beop-2013");
		strDbName = _T("test");
	}
}WriteNetCfg;


// CBEOPOPCClientDlg dialog
class CBEOPOPCClientDlg : public CDialog
{
// Construction
public:
	CBEOPOPCClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BEOPOPCCLIENT_DIALOG };

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
	DECLARE_MESSAGE_MAP()

public:
	bool	InitConfigParams();							//初始化信息
	bool	OutPutLogString(CString strOut);			//输出log
	bool	InitOPC();									//初始化OPC
	bool	ExitOPC();									//退出释放
	void	GetOPCValueSets(vector<pair<wstring, wstring> >& opcvaluesets);
protected:
	bool	InitOPC(COPCEngine* opcengine,int nOPCClientIndex = -1);
	vector<vector<OPCDataPointEntry>> GetOPCServer(vector<OPCDataPointEntry> opcpoint,int nMaxSize,wstring strDefaultIP);	
	int	IsExistInVec(wstring strSer, wstring strProgram,vector<OPCServer>& vecSer,int nMaxSize);
private:
	void	InitConfig(void);
	void	InitConfigFile(const CString strPath);
	void	ReadConfigInfo(const CString strPath);
	int		CreateDir(CString strFolderPath);
	void	WriteLog(CString strLog);

	static unsigned __stdcall GetOpcValueThread(void* pArguments);
	void	WriteIntoRealTimeData(const vector<pair<wstring, wstring> >& vecOpc);
	MYSQL	m_mysql;
	WriteNetCfg	m_stWriteInfo;
	CString	m_strExeDir;
	CString	m_strCfgPathName;
	CString	m_strLogDir;
	int		m_nSleep;


public:
	bool						m_bFirst;
	vector<OPCDataPointEntry>	m_opcpointlist;
	vector<OPCServer>			m_vecOPCSer;
	map<int,COPCEngine*>	m_mapOPCEngine;				//index编号OPC引擎（OPC点表超过10000时候启用）
	COPCEngine* m_opcengine;

	int			m_nOPCClientMaxPoint;			//OPCClient最大点数限制
	int			m_nMutilCount;					//OPC批量读取点个数
	int			m_nOPCCmdSleep;					//OPC读点间隔（从缓存）
	int			m_nOPCCmdSleepFromDevice;		//OPC读点间隔(从设备)
	int			m_nOPCPollSleep;				//OPC轮询间隔（从缓存）
	int			m_nOPCPollSleepFromDevice;		//OPC轮询间隔(从设备)
	int			m_nReconnectMinute;				//重连后N分钟内不存储数据
	int			m_nSendRemoteSet;				//远程设置过后Ns内发送5秒数据
	bool		m_bMutilOPCClientMode;			//多OPCClient模式
	bool		m_bMutilOPCClientUseThread;		//多OPCClient模式使用线程模式
	bool		m_bStoreHistory;				//是否存储历史数据
	bool		m_bEnableSecurity;				//开启进程安全设置项
	bool		m_bDisableCheckQuality;			//开启禁用品质数据检测
	int			m_nOPCMainThreadOPCPollSleep;	//OPC轮询间隔s（主进程从缓存）
	int			m_nOPCCheckReconnectInterval;	//OPC检测重连间隔（min）
	int			m_nOPCReconnectInertval;		//OPC N分钟数据未更新重连(min)
	int			m_nDebugOPC;
	wstring		m_strEnableSecurity;			//启用进程安全

	CString		m_strCsvPathName;
};
