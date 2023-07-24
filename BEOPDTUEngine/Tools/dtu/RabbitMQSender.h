#pragma once
#include "Tools/CustomTools.h"
#include <stdio.h>
#include <stdlib.h>
#include "./Tools/CommonDefine.h"
#include "./Tools/db/DataHandler.h"

/*
发送：
文件数据：数据压缩包，日志文件，错误日志
文本数据：配置表,报警数据，报警操作，反馈，丢失文件数据包

接收：
文件数据：更新文件地址，丢失文件包，点表文件
文本数据：命令（重启，同步/修改unit01,修改点值）

文件数据格式：（发送）
1：按250K拆分发送
2：消息体是二进制读取内容
3：附加属性：
dtunam:		dtuname
type:		0:文本命令  1:Json文本 2:压缩文件（zip）-不分包  3：压缩文件（zip）-分包    (必备)
cmdtype：	命令类型																	(必备)
cmdid:		命令编号																	(必备)
size:		内容大小																	(必备)
name:		文件名
index:		当前包/总的包数

文本数据格式：（发送）
1：不超过
2：消息体是 Json
3：附加属性：
type:		0:文本命令  1:Json文本 2:压缩文件（zip）-不分包  3：压缩文件（zip）-分包  (必备)
cmdtype：	命令类型																(必备)
size:		内容大小																	(必备)
id:			命令ID

Json:（发送）
{
id:反馈ID		//如果是服务器命令反馈，反馈不为0
state: 0:失败 1：成功   //如果不是服务器命令，默认true
type: int     // 0:保留  1：unit数据  2：报警数据  3：报警操作 4:log数据
msg:''		//失败原因
data:''
}

data0
{
unitNameList:[]
unitValueList:[]
}
data1
{
warningData:[{},{}]
}
data2:
{
logData:[{},{}]
}

Json:（接收）
{
id:				//命令ID
time:			//命令时间
type: int		// 0:保留  1：实时数据   2：unit数据  3:log数据  4：服务器时间 5:重启 6:更新 7:补数
data:''
}

data1:
{
type:  0:同步全部  1:同步指定  1：修改
pointNameList:[]
pointValueList:[]
}

data2
{
type:  0:同步全部  1:同步指定  1：修改
unitNameList:[]
unitValueList:[]
}

data3
{
timeStart:
timeEnd:
like:		//关键字
}

data4
{
type:  0：同步 :1：修改
time:''
}

data5
{
type: 0:重启Core  1：重启Logic  2:重启Update
}

data6
{
type: 0:更新Core  1：更新Logic  2:更新Update 3:更新点表  4：更新dll
fileurl:''
}

data7
{
dataFileList:[]				文件列表
}

//////////////////////////////////////////////////////////////////////////
发送队列      编号，时间，备注，文件名，内容（文本为Json  文件为路径地址）
接收队列      编号，时间，内容（文本为Json）  不直接传输文件

//
*/

typedef LRESULT(*LPRecDataProc)(const unsigned char* pRevData, DWORD length, DWORD userdata);					//接收数据的回调函数

class CRabbitMQSender
{
public:
	CRabbitMQSender(string strRabbitMQHost="", string strRabbitMQUser = "", string strRabbitMQPsw = "", string strRabbitMQSend = "", string strRabbitMQReceive = "", bool bRabbitMQSSL=false, CDataHandler*	pDataHandler=NULL);
	~CRabbitMQSender();

	bool	Init(LPRecDataProc proc1 = NULL, DWORD userdata = NULL);											//初始化函数
	bool	Exit();																								//退出函数
	bool	ReConnect();																						//失败自动重连函数
	bool	SendOneData(DTU_DATA_INFO& data);																	//发送一个数据	
	bool	IsConnected();																						//连接状态  不仅仅是连接的且是空闲的状态
	bool	IsBusy();																							//忙碌状态
	bool	SetConnectStatus(bool bConnectStatus);

private:
	bool	DisConnect();																						//关闭连接
	bool	RabbitMQConnect();																					//连接Rabbitmq中心

																												//
	bool	SendString(string strContent, DTU_CMD_TYPE nCmdType, string strRemark, DTU_DATA_INFO& data, string strCmdID ="");			//发送文本数据
	bool	SendFile(string strFilePath, DTU_CMD_TYPE nCmdType, DTU_DATA_INFO& data,string strCmdID ="");							//发送文件	

	static UINT WINAPI RecvThread(LPVOID lpVoid);																//创建接收线程
	bool	RecvRabbitMQ();
	void	HandlerReceive(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//处理收到的数据
	void	HandlerText(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//处理收到的文本数据
	void	HandlerFile(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//处理收到的文件数据
	void	HandlerFilePackage(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//处理收到的文件分包数据

	//
	bool	ClearFileBuffer();																					//释放文件缓存
	void	PrintLog(const wstring &strLog);																	//打印显示log
	void	StringReplace(string&s1, const string&s2, const string&s3);											//替换字符串
	bool	GenerateLog(DTU_CMD_TYPE type, string strContent, bool bSendSuccess, string strRemark = "");			//输出log
	//////////////////////////////////////////////////////////////////////////
	void	die(const char *fmt, ...);
	bool	die_on_error(int x, char const *context);
	bool	die_on_amqp_error(amqp_rpc_reply_t x, char const *context);
	void	amqp_dump(void const *buffer, size_t len);
	void	dump_row(long count, int numinrow, int *chs);
	int		rows_eq(int *a, int *b);

private:
	amqp_socket_t*				m_pSocketRabbitMQ;
	amqp_connection_state_t		m_conRabbitMQ;
	bool						m_bConnectSuccess;

	int							m_nRabbitMQStatus;
	int							m_nRabbitMQPort;
	int							m_nConnectTimeOut;
	string						m_strRabbitMQHost;
	string						m_strRabbitMQUser;
	string						m_strRabbitMQPsw;
	string						m_strRabbitMQSend;
	string						m_strRabbitMQReveive;
	string						m_strRabbitDirect;
	string						m_strRabbitDirectType;
	string						m_strRabbitBindKey;				//路由key
	int							m_nRabbitHeartbeat;				//心跳时间
	int							m_nRabbitDurable;				//持久化
	int							m_nRabbitNoAck;					//自动确认

	HANDLE						m_hRabbitReceiveHandler;		//Rabbit接收线程句柄
	bool						m_bExitThread;					//TRabbit退出线程
	LPRecDataProc				m_lpRecDataProc;				//收到数据回调
	DWORD						m_dwUserData;					//用户数据
	CDataHandler*				m_pDataHandler;					//数据库处理类
	COleDateTime				m_oleRabbitMQUpdateTime;		//Rabbit发送接收更新时间(成功)
	COleDateTime				m_oleSendTime;					//Rabbit发送事件
	int							m_nRabbitMQErrorCount;			//RabbitMQ发送错误

	char*						m_cFileBuffer;					//文件缓存
	int							m_nFileBuffer;					//总文件大小
	int							m_nCurrentFileBuffer;			//当前文件大小
	string						m_strFileName;					//文件名
	bool						m_bBusy;						//忙碌状态

	int							m_nChannelSend;					//发送通道
	int							m_nChannelReceive;				//接收通道
};