#pragma once
#include "Tools/db/DataHandler.h"
#include "Tools/dtu/TCPDataSender.h"
#include "Tools/dtu/DTUDataSender.h"
//#include "Tools/dtu/IOTDataSender.h"
#include "Tools/dtu/RabbitMQSender.h"
#include "Tools/ProcessView.h"
#include "Tools/ExcelOperator.h"
#include "Tools/LogicBase.h"

typedef CLogicBase* (*pfnLogicBase)();

class CBEOPDTUCtrl
{
public:
	CBEOPDTUCtrl(CDataHandler* pDataHandler);
	~CBEOPDTUCtrl();

	bool	Init();									//初始化函数
	bool	Exit();									//退出函数
	void	PrintLog(const wstring &strLog);		//打印显示log

	bool	IsConnected();							//连接成功

	static LRESULT OnRecData(const unsigned char* pRevData, DWORD dwLength, DWORD dwUserData);
	bool	StoreDTUCmd(DTU_RECEIVE_INFO& data);

	//发送函数
	bool	SendOneData(DTU_DATA_INFO& data);		//发送一个数据	

	//发送注册函数
	bool	SendRegData();							//发送DTU注释

	//处理收到的数据
	bool	HandlerRecCmd();						//处理收到的数据

	//连接处理函数
	bool	ReConnect();							//失败自动重连函数

private:
	bool	HandlerRecData(DTU_RECEIVE_INFO& data);	//处理收到的数据
	bool	HandlerString(DTU_RECEIVE_INFO& data);	//处理文本
	bool	HandlerFile(DTU_RECEIVE_INFO& data);	//处理文件
	bool	HandlerLostFile(DTU_RECEIVE_INFO& data);//处理丢失文件
	bool	HandlerUpdateCmd(DTU_UPDATE_INFO& data);//处理一个更新的数据

	bool	HandlerSynRealData(POINT_STORE_CYCLE nType,string strCmdID="");											//同步某种类型数据
	bool	HandlerSynUnit01(string strCmdID="");																	//同步Unit01
	bool	HandlerReadLostFileAndSendHistory(string strPath,string strCmdID="");									//根据文件重发包
	bool	HandlerChangeValues(string strReceive,string strCmdID="");												//批量修改点
	bool	HandlerAckHeartBeat(string strCmdID="");																//心跳包请求
	bool	HandlerAckReSendFileData(string strFileName,string strCmdID="");										//重发丢失实时文件包
	bool	HandlerAckServerState(string strCmdID="");																//心跳包请求
	bool	HandlerChangeUnits(string strReceive,string strCmdID="");												//批量设置Unit01
	bool	HandlerSendHistoryDataFile(string strStart, string strEnd,string strCmdID="");							//历史数据
	bool	HandlerChangeUnit(string strParamName, string strValue,string strCmdID="");								//设置Unit01
	bool	HandlerDeleteMutilPoint(string strPoints,string strCmdID="");											//删除点
	bool	HandlerUpdateRestart(string strCmdID="");																//重启Core
	bool	HandlerRestartDTUEngine(string strCmdID="");															//重启DTUEngine
	bool	HandlerAckCoreVersion(string strCmdID="");																//Core版本号
	bool	HandlerAckLocalPoint(string strCmdID="");																//同步现场点表
	bool	HandlerAckErrFileList(string strCmdID="");																//回应错误文件列表
	bool	HandlerAckCoreErrCode(string nMintue,string strCmdID="");												//Core错误代码
	bool	HandlerAckErrFileByName(string strErrName,string strCmdID="");											//回应错误文件列表
	bool	HandlerAckReSendLostZipData(string strMinutes, string strFileNames,string strCmdID="");					//回应丢失掉线期间数据
	bool	HandlerAckReSendLostData(string strMinutes, string strPacketIndex,string strCmdID="");					//回应丢失掉线期间数据
	bool	HandlerUpdateMutilPoint(string strPoints,string strCmdID="");											//增/修点表
	bool	HandlerUpdateMutilPoint_(string strPropertyCount, string strPoints,string strCmdID="");					//增/修点表
	bool	HandlerMysqlFileByTableName(string strTableName,string strCmdID="");									//回应数据库文件
	bool	HandlerAckSynSystemTime(string strSystemTime,string strCmdID="");										//回应服务器同步系统时间
	bool	HandlerAckReStartDTU(string strCmdID="");																//回应重启DTU
	bool	HandlerRestartLogic(string strCmdID="");																//重启Logic
	bool	HandlerChangeValueByDTUServer(string strPointName, string strValue,string strCmdID="");					//修改单点

	//////////////////////////////////////////////////////////////////////////
	bool	UpdateDll(DTU_UPDATE_INFO& data,string& strResultString);												//更新策略
	bool	UpdateS3db(DTU_UPDATE_INFO& data,string& strResultString);												//更新S3db
	bool	UpdateCore(DTU_UPDATE_INFO& data,string& strResultString);												//更新Core
	bool	UpdateDTU(DTU_UPDATE_INFO& data,string& strResultString);												//更新DTU
	bool	UpdateLogic(DTU_UPDATE_INFO& data,string& strResultString);												//更新Logic
	bool	UpdateBat(DTU_UPDATE_INFO& data,string& strResultString);												//执行批处理文件
	bool	UpdatePointExcel(DTU_UPDATE_INFO& data,string& strResultString);										//更新点表Excel文件
	bool	UpdatePointCSV(DTU_UPDATE_INFO& data,string& strResultString);											//更新点表CSV文件

	bool	UpdatePointXls(CString strExcelPath,CString strS3dbPath,int& nErrCode);
	bool	UpdatePointCSV(CString strCSVPath,CString strS3dbPath,int& nErrCode);
	bool	UpdateDll(int nUpdateSate,CString strDllName,CString strThreadName,CString strS3dbPath);
	bool	StoreDllToDB(CString strDllPath,CString strS3dbPath,CString strDllName,CString strThreadName,CString strVersion,CString strDescription,CString strAuthor);
	bool	DeleteDll(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllThread(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	DeleteDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strThreadName,wstring strState,int nPeriod=-1);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strState,wstring strThreadName,wstring strValue);

	bool	FindFile(CString strFolder,CString strExten,vector<CString>& vecFileName);		//在指定目录查找后缀名为XX的文件
	bool	FindFile(CString strFolder,CString strName);
	//////////////////////////////////////////////////////////////////////////

	string	SendCmdState(const string strSendTime, int nCmdType, int nCmdState, string strPacketIndex);	//命令反馈
	bool	CheckDiskFreeSpace(string strDisk, string& strDiskInfo);								//获取磁盘空间
	int		GetMemoryPervent();																		//获取内存
	int		GetCPUUseage();																			//获取CPU
	double	FileTimeToDouble(FILETIME &filetime);
	bool	ReadDataPoint(string strS3dbPath, vector<DataPointEntry> &vecPoint);					//读取点表
	bool	GeneratePointCSV(vector<DataPointEntry> &vecPoint, wstring strFolder, CString& strCSVName, wstring & strCSVPath);
	bool	GeneratePointFromInfo(vector<vector<wstring>> vecPoints, vector<DataPointEntry>& vecPoint);
	string	GetFileInfoPointCont(string strPath, string strFileName);
	bool	UpdateMutilPoint(vector<DataPointEntry>& vecPoint);
	int		GetColumnIndex(std::map<std::string, int> map, string strColumnName);
	bool	DelteFoder(wstring strDir);																//删除文件夹
	bool	ImproveProcPriv();																		//提升权限	
	bool	SetSystemTimeByBat(SYSTEMTIME st);														//设置服务器时间
	bool	RestartLogic(wstring strPath);															//重启Logic

	bool	CloseExe(CString strExeName);															//关闭程序
	bool	OpenExe(CString strExePath);															//启动程序
	bool	RestartExe(CString strExeName,CString strFolder);										//重启程序
	bool	OutPutUpdateLog(CString strLog);
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<string>& resultlist);		//争对),操作
private:
	CDataHandler*	m_pDataHandler;					//数据库处理类
	CDTUtDataSender* m_pDTUtDataSender;				//DTU发送类
	CTCPDataSender*	m_pTCPDataSender;				//TCP发送类
	//CIotDataSender*	m_pIotDataSender;				//Iot发送类
	CRabbitMQSender* m_pRabbitMQSender;				//RabbitMQ发送类

	bool			m_bDTUConnectSuccess;			//DTU连接状态
	wstring			m_wstrDTUType;					//DTUType 0:禁用  1:2gDTU(UDP+DDP)  2:2gDTU(TCP)  3:TCP  4:Iot
	wstring			m_wstrDTUMinType;				//DTUMinType	DTU最小发送类型间隔 2:1分钟点  3:5分钟 默认为3
	wstring			m_wstrDTUDisableSendAll;		//DTUDisableSendAll 是否禁用零点发送
	wstring			m_wstrDTUSendAll;				//sendall			发送全部数据

	//串口参数
	wstring			m_wstrDTUEnabled;				//DTUEnabled DTU启用
	wstring			m_wstrDTUComPort;				//DTUComPort DTUCom口   默认为串口1
	wstring			m_wstrDTUChecked;				//DTUChecked DTU检测    默认0

	//TCP
	wstring			m_wstrTCPIP;					//TcpSenderIP TCP的IP或域名		114.215.172.232  beopdata.rnbtech.com.hk
	wstring			m_wstrTCPPort;					//TcpSenderPort TCP端口			9500
	wstring			m_wstrTCPName;					//TcpSenderName TCP标识（11位）
	wstring			m_wstrTCPSendFilePackageSize;	//TCPSendFilePackageSize TCP包大小	1000
	wstring			m_wstrTCPSendFilePackageInterval;//TCPSendFilePackageInterval TCP包间隔 2000
	wstring			m_wstrTcpSenderEnabled;			//TcpSenderEnabled		启用
	wstring			m_wstrTCPAES256;				//AES256

	//Iot
	wstring			m_wstrIotEnable;				//IotEnable	启用
	wstring			m_wstrIotConnection;			//IotConnection Iot连接字符串

	//RabbitMQ
	wstring			m_wstrRabbitMQHost;				//RabbitMQ地址
	wstring			m_wstrRabbitMQUser;				//RabbitMQ用户
	wstring			m_wstrRabbitMQPsw;				//RabbitMQ密码
	wstring			m_wstrRabbitMQSend;				//RabbitMQ发送
	wstring			m_wstrRabbitMQReveive;			//RabbitMQ接收
	wstring			m_wstrRabbitMQSSL;				//SSSL/TSL连接

	//固定需要写的
	wstring			m_wstrRunDTU;					//rundtu 状态
	CRITICAL_SECTION m_csDTUDataSync;				//临界区对象 保护共享资源
	queue<DTU_RECEIVE_INFO>	m_queReceivePaket;		//存储服务器接收数据结构

	//锁
	Project::Tools::Mutex	m_lockSaveReceive;		//存储接收事件锁
	Project::Tools::Mutex	m_lockConnect;			//连接事件锁

	int				m_nDebugIndex;

	CProcessView	m_PrsView;
};
