#pragma once
#include "Tools/CustomTools.h"
#include "Tools/CommonDefine.h"
#include "mswsock.h"
#include <process.h>
#include <string>
#include <vector>
#include <queue>

using namespace std;
using std::vector;

#define MAX_RECEIVED_BUFFER_SIZE  2048		//每次接收数据大小

typedef LRESULT(*LPRecDataProc)(const unsigned char* pRevData, DWORD length, DWORD userdata);					//接收数据的回调函数

class CTCPDataSender
{
public:
	CTCPDataSender(wstring wstrTCPIP=L"beopdata.rnbtech.com.hk", wstring wstrTCPPort=L"9500", wstring wstrTCPName=L"", wstring wstrTCPSendFilePackageSize=L"1000", wstring wstrTCPSendFilePackageInterval=L"2000", wstring wstrTCPAES256=L"0");
	~CTCPDataSender();

	bool	Init(LPRecDataProc proc1 = NULL, DWORD userdata = NULL);											//初始化函数
	bool	Exit();																								//退出函数
	bool	ReConnect();																						//失败自动重连函数
	bool	SendOneData(DTU_DATA_INFO& data);																	//发送一个数据	
	bool	SendString(string strContent);																		//发送文本数据
	bool	IsConnected();																						//连接状态

private:
	//TCP函数
	bool	TCPConnect(const string& strHost, u_short uPort);													//TCP连接
	bool	DisConnect();																						//关闭连接
	bool	SendFile(string strFilePath);																		//发送文件
	bool	SendRealDataFile(string strFilePath, int nPointCount);												//发送实时数据文件
	bool	SendHistoryDataFile(string strFilePath, int nPointCount);											//发送历史丢失文件
	bool	GenerateLog(DTU_CMD_TYPE type, string strContent,bool bSendSuccess,string strRemark="");			//输出log

	bool	TCPSendPackageByWrap(char *pData, int nLen);														//自动添加换行符
	bool	TCPSendPackage(const char *pSrcPackage, unsigned int nSrcPackSize);									// send package	
	SOCKET  GetTCPConnectedSocket();

	bool	HandleRecData(const char* pSrcBuffer, int nSize);													//处理接收到的数据
	void	PrintLog(const wstring &strLog);																	//打印显示log
	void	SplitStringSpecial(const char* buffer, int nSize, std::vector<TCP_DATA_PACKAGE>& resultlist);		//争对;\n操作
	bool	OnRecvData(const unsigned char* pRevData, DWORD dwLength);											//
	bool	StoreRecvData(TCP_DATA_PACKAGE& data);																//存储接收到的数据
	void	SplitString(const std::string& source, const char* sep, std::vector<string>& resultlist);
	unsigned int BufferToInt(const unsigned char* buffer);

	bool	AnalyzeRecvData();																					//解析处理
	bool	AnalyzeReceive(const unsigned char* pRevData, DWORD length);										//处理收到的数据
	void	AnzlyzeString(char* pBuffer);																		//一般命令
	void	AnalyzeUpdateFile(const unsigned char* buffer);														//更新程序
	void	AnalyzeRecLogFile(const unsigned char* buffer);														//丢失文件压缩包
	void	GenerateUpdateFile();
	void	GenerateLostFile();																					//
	bool	CreateOrDeleteUpdateFileBuffer(int nSize = 1024, bool bDeelete = false);
	void	ReleaseUpdateBufferSize(int nTimeOut = 300);
	void	StringReplace(string&s1, const string&s2, const string&s3);											//替换字符串

	static UINT WINAPI RecvThread(LPVOID lpVoid);																//创建接收线程
	static UINT WINAPI AnalyzeRecvCmd(LPVOID lpVoid);															//创建解析处理线程
	static UINT WINAPI DTUThreadReleaseUpdateSize(LPVOID lparam);												//创建释放更新缓存线程

private:
	bool			m_bTCPConnectSuccess;			//TCP连接状态
	bool			m_bAES256;						//使用AES256加密

	//TCP
	wstring			m_wstrTCPIP;					//TcpSenderIP TCP的IP或域名		114.215.172.232  beopdata.rnbtech.com.hk
	wstring			m_wstrTCPPort;					//TcpSenderPort TCP端口			9500
	wstring			m_wstrTCPName;					//TcpSenderName TCP标识（11位）
	wstring			m_wstrTCPSendFilePackageSize;	//TCPSendFilePackageSize TCP包大小	1000
	wstring			m_wstrTCPSendFilePackageInterval;//TCPSendFilePackageInterval TCP包间隔 2000
	wstring			m_wstrTcpSenderEnabled;			//TcpSenderEnabled		启用

	SOCKET          m_sockTCP;      	 			//TCP客户端Socket
	HANDLE			m_hTCPReceiveHandler;			//TCP接收线程句柄
	bool			m_bExitThread;					//TCP退出线程
	int				m_nTCPErrorCount;				//TCP发送错误
	COleDateTime	m_oleTCPUpdateTime;				//TCP发送接收更新时间
	Project::Tools::Mutex	m_lockTCP;				//TCP接收锁

	HANDLE			m_hReleaseBufferHandler;		//释放缓存内存锁
	HANDLE			m_hAnalyzeHandler;				//解析处理线程句柄
	char			m_cSpiltBuffer[g_nRecMaxNum];
	char			m_cRecvBuf[g_nRecMaxNum];		//数据接收缓存
	int				m_nRecSize;						//当前数据接收缓存大小
	char			m_cAnzlyzeBuff[g_nRecMaxNum];	//数据解析缓存
	int				m_nAnzlyzeCount;				//当前数据解析缓存大小
	char			*m_szWriteBuffer;				//待发送内容
	int				m_nWriteSize;					//待发送大小
	int				m_nDTUFilePackageNumber;		//文件包编号

	LPRecDataProc	m_lpRecDataProc;				//收到数据回调
	DWORD			m_dwUserData;					//用户数据

	CRITICAL_SECTION m_csDTUDataSync;				//临界区对象 保护共享资源
	queue<TCP_DATA_PACKAGE>	m_queCmdPaket;			//存储服务器命令包

	Project::Tools::Mutex	m_lockReveive;			//处理线程锁
	Project::Tools::Mutex	m_lockReceiveBuffer;	//接收缓存锁

	hash_map<int, int>	m_mapFileUpdateState;		//文件收包情况
	string			m_strCoreFileName;				//Core文件名称
	int				m_nCoreFileCount;				//Core文件包总数
	int				m_nRecCoreFileCount;			//收到的Core文件包总数
	int				m_nLastUpdateExeSize;			//最后一个包的大小
	int				m_nUpdateFileSize;				//更新队列大小
	char**			m_cFileUpdateBuffer;			//更新缓存
	COleDateTime	m_oleUpdateFileTime;				//更新文件最近使用时间，超过10分钟清除内存
};
