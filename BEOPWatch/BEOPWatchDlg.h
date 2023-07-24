
// BEOPWatchDlg.h : header file
//

#pragma once
#include "ProcessView.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include "afxwin.h"
#include "./db/DataHandler.h"
#include "./tcp/DTUSender.h"
#include "./tcp/ExcelOperator.h"
#include "./tcp/DogTcpCtrl.h"
#include "Tools/Zip/zip.h"
#include "Tools/Zip/unzip.h"
#include "LogicBase.h"
#define WM_SHOWTASK (WM_USER+20)	//托盘消息

typedef CLogicBase* (*pfnLogicBase)();
/*			远程DTU命令

2|0			//同步Unit01
4|			//重发丢失实时文件包
5|			//设置Unit01
6|			//批量修改点
8|0			//心跳包请求
9|			//回应服务器状态

//////////////////////////////////////////////////////////////////////////
1|起始|结束			//历史数据
3|0|0				//重启Core
3|1|0				//Core版本号
3|1|1				//同步现场点表
3|1|2				//回应错误文件列表
3|2|时间			//Core错误代码
3|3|文件名			//回应错误文件
3|4|表名			//回应数据库文件

//////////////////////////////////////////////////////////////////////////
5|参数|值					//设置Unit01
7|类型|文件名				//回应丢失掉线期间数据zip
10|0|				//增/修点表
10|1|点名			//删除点


//////////////////////////////////////////////////////////////////////////
10|0|属性|点名		//增/修点表

*/

/*		DTU发送

3|		//Unit01
3|1		//同步
3|2		//修改
3|3		//restart
3|5		//sql命令
3|6		//心跳包相应
3|7		//start core
3|8		//服务器状态信息
3|10	//同步现场时钟
3|11	//错误代码
3|12	//错误日志列表

6|1		//删除文件

*/
// CBEOPWatchDlg dialog
class CBEOPWatchDlg : public CDialog
{
// Construction
public:
	CBEOPWatchDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BEOPWATCH_DIALOG };

	static  unsigned int WINAPI CheckExeThread(LPVOID lpVoid);						//检查Core是否在运行
	static  unsigned int WINAPI SendDTUThread(LPVOID lpVoid);						//发送数据
	static  unsigned int WINAPI SendDTUReceiveHandleThread(LPVOID lpVoid);			//处理来自服务器的命令
	static  unsigned int WINAPI ScanUpdateStateHandleThread(LPVOID lpVoid);			//扫描数据库中更新状态字
	static  unsigned int WINAPI CreateTCPConnection(LPVOID lpVoid);					//线程创建TCP连接
	static  unsigned int WINAPI AckMysqlFileByTableNameThread(LPVOID lpVoid);		//回复数据库文件线程

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA		m_nid;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonRestart();
	afx_msg LRESULT OnShowTask(WPARAM wParam,LPARAM lParam); 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCloseWatch();
	DECLARE_MESSAGE_MAP()

public:
	bool	OutPutLogString(CString strOut);
	
	bool	CheckStateIsNeedRestart();						//检测一下是否需要重启
	bool	HandleRestart();								//处理重启操作
	bool	HandleRestartLogic();							//处理重启Logic操作
	bool	RestartCoreByDog(int nType,int& nErrorCode,bool bRestart=false);//根据dog存在与否来决定是否通过dog
	bool	RestartLogicByDog(int nType,int& nErrorCode,bool bRestart=false);//根据dog存在与否来决定是否通过dog
	bool	WriteActiveTime();								//在线活动时间
	bool	SendWatchStart();								//定时发送项目信息
	bool	CloseWatch();

	bool	ScanFile(unitLogFileInfo logFile);				//遍历.log文件 不遍历backup/filelog文件夹

	//////////////////////////////////////////////////////////////////////////
	bool	ScanUpdateState();								//扫描数据库中更新状态字
	bool	UpdateDTUInfo();								//扫描DTU状态
	bool	OutputMemorySize();								//输出内存大小
	bool	DeleteBackupFolderByDate();						//根据日期删除备份文件
	bool	ScanAndGernerateLogFile();						//扫描输出文件log
	bool	DeleteTableByDiskWarning();						//删除数据库表根据磁盘报警

	//辅助
	double	CheckDiskFreeSpace(string strDisk);				//返回磁盘使用率
	bool	DeleteTableByDate(string strTablePrefix, int nDate);//删除表
	bool	DeleteTableByDeadDate(string strTablePrefix, int nDate);//删除截止日期以前表
	//////////////////////////////////////////////////////////////////////////

	bool	CloseCore();
	bool	CloseLogic();
	bool	SumRestartTime(int nMinute = 40);					//统计时间内自动重启3次以上 再重启需要延迟1分钟

	bool	InitTCPSender();
	bool	ExitCPSender();
	HANDLE	GetSendDTUEvent();
	bool	SendDTUData();
	bool	ActiveLostSendEvent();								//有待发送丢失队列则激活
	bool	HandleCmdFromDTUServer();							//处理来自DTUServer的命令
	bool	UpdateDTUSuccess();

	bool	AckReceiveExeFile(string strExeName);								//回应接收文件成功标志
	bool	AckUpdateExeFile(string strExeName);								//回应更新文件成功标志
	bool	AckLostExeFile(string strLostFile);								//回应更新文件成功标志
	bool	AckUpdatePoint(string strPointNames, int nOperation);							//回应更新点标志
	bool	AckRestartCore();
	bool	AckCoreVersion();								//Core版本号
	bool	AckCoreErrCode(string nMintue);					//Core错误代码
	bool	SetSendDTUEventAndType();

	bool	DelteFile(CString strDir,CString strBacDir);						//删除文件另外备份
	bool	DelteFile(CString strFilePath);						//删除文件另外备份

	bool	OutPutUpdateLog(CString strLog);		//记录更新Log
	bool	FindFile(CString strFolder,CString strExten,vector<CString>& vecFileName);		//在指定目录查找后缀名为XX的文件
	bool	FindFile(CString strFolder,CString strName);
	
	//////////////////////////////////////////////////////////////////////////
	bool	SynUnit01();		//同步Unit01
	bool	ChangeValuesByDTUServer(string strReceive);
	bool	ChangeUnitsByDTUServer(string strReceive);
	bool	AckHeartBeat();						//回应服务器心跳包
	bool	AckServerState();						//回应服务器状态
	bool	AckErrFileList();						//回应错误文件列表
	bool	AckErrFileByName(string strErrName);	//回应错误文件列表
	void	AckMysqlFileByTableName(string strTableName);						//回应数据库文件
	bool	AckMysqlFileByTableName_(string strTableName);	//回应数据库文件
	bool	AckReSendFileData(string strFileName);					//回应丢失重发数据
	bool	SendHistoryDataFile(string strStart,string strEnd);
	bool	AckReSendLostZipData(string strMinutes,string strFileNames);			//回应丢失掉线期间数据
	bool	ChangeUnit01ByDTUServer(string strParamName, string strValue);
	string BuildDTUSendUnit01(vector<unitStateInfo> unitlist);
	int		GetFileInfoPointCont(string strPath,string strFileName);
	bool	SendOneLostHistoryData();							//发送一个待发送的历史丢失数据
	int		GetColumnIndex(std::map<std::string, int> map, string strColumnName);
	bool	CheckDiskFreeSpace(string strDisk,string& strDiskInfo);
	double	CpuUseage();
	int	GetCPUUseage();
	__int64 CompareFileTime ( FILETIME time1, FILETIME time2 );
	double FileTimeToDouble(FILETIME &filetime);
	int		GetMemoryPervent();
	//////////////////////////////////////////////////////////////////////////20151213
	//同步现场点表
	bool	AckLocalPoint();								//同步现场点表
	bool	GenerateLocalPointCSV(wstring strFolder,wstring& strLocalZip);	
	bool	ReadDataPoint(string strS3dbPath,vector<DataPointEntry> &vecPoint);
	bool	GeneratePointCSV(vector<DataPointEntry> &vecPoint,wstring strFolder,CString& strCSVName,wstring & strCSVPath);
	bool	ChangeIconByDTUState();
	bool	ChangeIcon(bool bOK);
	bool	ShowDTURouterStateInfo();			//DTU 30分钟未发数据时候检测信号
	bool	ShowDTURouterState(string strRouterIP="192.168.1.241");
	//////////////////////////////////////////////////////////////////////////
	//
	bool	UpdatePoint();		//更新点表
	bool	UpdatePoint_CSV();		//更新点表
	bool	DeleteMutilPoint(string strPoints);
	bool	UpdateMutilPoint(string strPoints);
	bool	UpdateMutilPoint(vector<DataPointEntry>& vecPoint);

	bool	UpdateMutilPoint_(string strPropertyCount,string strPoints);
	bool	GeneratePointFromInfo(vector<vector<wstring>> vecPoints, vector<DataPointEntry>& vecPoint);
	bool	UpdateDll();			//更新策略
	bool	UpdateS3db();			//更新S3db
	bool	UpdateCore();			//更新Core
	bool	UpdateWatch();		//更新Watch
	bool	UpdateLogic();			//更新Logic
	bool	UpdateRestart();
	bool	UpdateBat();			//执行批处理文件

	bool	UpdatePointXls(CString strExcelPath,CString strS3dbPath,int& nErrCode);
	bool	UpdatePointCSV(CString strCSVPath,CString strS3dbPath,int& nErrCode);

	bool	Exit();
	bool	GetExitThread_History() const;

	bool	FindAndDeleteBackupDirectory(CString strDir,CString strDeadFolder);
	bool	FindAndDeleteErrFile(CString strDir,CString strDeadFileName);
	bool	FindAndDeleteErrFile(CString strDir,CString strFilterName,CString strFilterPri,CString strDeadFileName);
	bool	DelteFoder(CString strDir);

	bool	ResetTCPInfo();

	bool	OpenDog();
	bool	ImproveProcPriv();
	bool	TestNetWork();

	bool	FeedDog(bool bFirst = true);				//喂狗

	//////////////////////////////////////////////////////////////////////////
	bool	UpdateDll(int nUpdateSate,CString strDllName,CString strThreadName,CString strS3dbPath);
	bool	StoreDllToDB(CString strDllPath,CString strS3dbPath,CString strDllName,CString strThreadName,CString strVersion,CString strDescription,CString strAuthor);
	bool	DeleteDll(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllThread(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	DeleteDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strThreadName,wstring strState,int nPeriod=-1);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strState,wstring strThreadName,wstring strValue);

	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<string>& resultlist);		//争对),操作

private:
	CProcessView		m_PrsV;
	bool                m_bFirst;
	bool				m_bChangeIcon;
public:
	CEdit	m_eLog;
	CString m_strLog;
	CString	m_strPath;
	CString	m_strLogicPath;
	CString m_strIniFilePath;
	CString m_strLogFilePath;
	CString m_strMemoryLogFilePath;
	int		m_nErrorCode;

	//
	CString	m_strUpdateS3dbPath;
	CString	m_strUpdateXlsFile;
	CString	m_strUpdateCorePath;
	CString	m_strUpdateBatFile;

	int		m_nRestartType;					//重启方式：-1: 无需重启 0:手动重启 1 Core不存在; 2:servertime10分钟未更新 3：西门子5分钟未更新 4:dtu60分钟未更新 5:更新点表 6：更新core 7：更新dll 8：更新dog 9：远程重启 10:定时重启 11:重新设置dtu重启 12更新s3db
	int		m_nRestartLogicType;			//重启方式：-1: 无需重启 0:手动重启 1 Logic不存在; 2:servertime10分钟未更新 6：更新logic 7：更新dll  9：远程重启 12更新s3db(实际只写数据库，具体由core负责重启)
	HANDLE		m_hcheckhandle;
	int		m_nErrCode;

	vector<COleDateTime> m_vecReStartTime;			//自动重启时间
	CDataHandler* m_pDataHandle;	//数据库连接

	CDTUSender*	m_pdtusender;
	bool		m_bdtuSuccess;
	bool		m_bexitthread_history;
	HANDLE		m_senddtuevent;
	HANDLE		m_hsenddtuhandle;
	HANDLE		m_hReceivedtuhandle;
	HANDLE		m_hScanUpdateStatehandle;
	wstring		m_strTCPName;						//用于标识TCP项目
	wstring		m_strTCPPort;						//用于标识TCP项目

	vector<DTUSendInfo>	m_vecSendBuffer;		//DTU发送缓存
	bool		m_bVecCopying;					//正在copy不能添加
	int			m_nDTUState;					//Core DTU连接状态  0失败  1成功
	int			m_nOutPutMemoryInterval;		//输出内存间隔
	COleDateTime	m_oleLastDleteTime;
	COleDateTime	m_oleLastRestartTime;			//上一次自动重启时间
	COleDateTime	m_oleLastSendReg;				//上一次发送注册信息
	COleDateTime	m_oleStartTime;					//启动时间
	COleDateTime	m_oleShowRouterState;			//查询DTU状态
	COleDateTime	m_oleGetMemoryTime;				//查询内存大小 30分钟记录一次
	map<string,int>					m_mapSendLostHistory;
	string      m_strSendRealFilePath;				//需要立即发送的实时包地址
	int			m_nSendRealFileCount;				//需要立即发送的实时包点数
	string		m_strAck;							//修改点ACK回复内容合并发送
	SYSTEMTIME m_ackTime;							//回复时间
	string		m_strZipPath;
	string		m_strAckMysqlTableName;				//
	Project::Tools::Mutex m_dtu_lock;
	Project::Tools::Mutex m_dtu_ack_lock;
	Project::Tools::Mutex m_watch_lock;

	FILETIME m_preidleTime;
	FILETIME m_prekernelTime;
	FILETIME m_preuserTime;
	bool			m_bCloseDlg;
	afx_msg void OnBnClickedCheckTcpConnection();
};
