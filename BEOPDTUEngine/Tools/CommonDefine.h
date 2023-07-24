#pragma once

#include "XSTRING"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <assert.h>
#include <string>
#include <windef.h>
#include <process.h>
#include <WinBase.h>
#include <winsock2.h>
#include "./Tools/json/json.h"
#include "Tools/zip/zlib.h"
#include "Tools/Zip/unzip.h"
#include "Tools/zip/zip.h"
#include "Tools/point/DataPointEntry.h"

#pragma comment(lib,"ZLib.lib")
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#pragma comment(lib,"json_vc71_libmtd.lib")
#else
#pragma comment(lib,"json_vc71_libmt.lib")
#endif

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 1
#include <hash_map>

using std::vector;
using std::string;
using std::wstring;
using std::list;
using namespace std;
using namespace Json;

/*
	发送：
		文件数据：数据压缩包，日志文件，错误日志,点表文件
		文本数据：配置表,报警数据，报警操作，操作记录，远程命令反馈

	接收：
		文件数据：更新文件，丢失文件包，点表文件, 策略文件
		文本数据：各种命令（重启，同步/修改unit01,修改点值）

	//////////////////////////////////////////////////////////////////////////
	发送队列      编号，时间，filename, subtype(),JsonContent,尝试次数


	
	最先发送文本数据:
		 选取 filename='' 且尝试次数小于3 根据时间和subtype综合排序,成功删除，失败则尝试计数+1

	其次发送一个数据文件   选取 filename!='' subtupe = 0 根据时间排序,成功删除，失败则尝试计数+1
	再次发送一个其他类型文件数据 选取 filename!='' subtupe != 0 且尝试次数小于3的 根据时间排序,成功删除，失败则尝试计数+1
	
	实时数据压缩包(文件名，0)
	日志文件（文件名，1）
	错误文件（文件名，2）
	点表文件（文件名，3）
	策略文件（文件名，4）

//
*/

//Data Struct

#define DTU_FILE_TAG		"fdp:"				//文件包头
#define DTU_FILE_REAL_TAG	"frp:"				//实时文件包头
#define DTU_FILE_HISTORY_TAG "fhp:"				//历史文件包头
#define DTU_HEART_TAG		"hba:"				//心跳包头
#define DTU_ACK_TAG			"ack:"				//回复包头
#define DTU_UNIT_TAG		"udp:"				//unit包头
#define DTU_REG_TAG			"reg:"				//注册包头
#define DTU_SPACE			"        "			//空格

#define MAX_REC_COUNT		8192				//接收最大包
#define MAX_UPDATE_SIZE		50*1024				//最大

const int DTU_SEND_PACKAGE_INTERNAL = 2 * 1000;	//dtu连续发包的间隔时间
const int DTU_SEND_UNIT01_INTERNAL = 2 * 1000;	//dtu连续发包的间隔时间(间隔时间过短会崩溃)
const int DTU_PACKAGE_SIZE = 900;				//分包时最大包长。
const int MAX_POINT_LENGTH = 120;				//点名最大长度

const int FILE_UPDATE_LENGTH = 4000;			//文件升级固定长度为4000
const int TCP_SEND_INTERNAL = 200;				//200ms发送一帧
const int FILE_SIZE = 1024;						//小的读取文件的空间总数
const int ONE_FILE_SIZE = 4096;					//每个小的读取文件的空间

const int IOT_FILE_SIZE = 40;					//IOT大小
const int IOT_ONE_FILE_SIZE = 1024*250;			//IOT每个文件包发送大小
const int IOT_RECEIVE_ONE_FILE_SIZE = 1024 * 60;//IOT每个文件包接收大小
const int IOT_SEND_TIME_OUT = 15;				//IOT发送超时
const int IOT_EVENT_COUNT = 100;				//IOT存储10个发送事件

const int RABBIT_FILE_SIZE = 40;				//RABBIT大小
const int RABBIT_ONE_FILE_SIZE = 1024 * 250;	//RABBIT每个文件包发送大小
const int RABBIT_SEND_TIME_OUT = 15;			//RabbitMQ发送超时

const string EXE_LOGIC = "domlogic.exe";	//Logic
const string EXE_DOG = "domdog.exe";			//Dog
const string EXE_CORE = "domcore.exe";	//Core
const string EXE_DTU = "domDTUEngine.exe";		//DTUEngine

const string AES_PSW = "RNB.beop-2013";			//AES256密码

enum DTU_CMD_TYPE								//DTU命令类型
{
	DTU_CMD_DEFINE = 0,							//默认
	DTU_CMD_REAL_DATA_SEND,						//实时数据（发送）
	DTU_CMD_REAL_DATA_SYN,						//实时数据（同步）
	DTU_CMD_REAL_DATA_EDIT,						//实时数据（修改）
	DTU_CMD_REAL_DATA_EDIT_MUL,					//实时数据（批量修改）
	DTU_CMD_WARNING_DATA_SEND,					//报警数据(发送)
	DTU_CMD_WARNING_DATA_SYN,					//报警数据(同步)
	DTU_CMD_WARNING_OPERATION_DATA_SEND,		//报警操作(发送)
	DTU_CMD_WARNING_OPERATION_DATA_SYN,			//报警操作(发送)
	DTU_CMD_OPERATION_SEND,						//操作记录（发送）
	DTU_CMD_OPERATION_SYN,						//操作记录（同步）
	DTU_CMD_UNIT_SEND,							//unit01(发送)
	DTU_CMD_UNIT_SYN,							//unit01(同步)
	DTU_CMD_UNIT_EDIT,							//unit01(修改)
	DTU_CMD_UNIT_EDIT_MUL,						//unit01(批量修改)
	DTU_CMD_LOG_SEND,							//Log（发送）
	DTU_CMD_LOG_SYN,							//Log（同步）
	DTU_CMD_HEART_SEND,							//心跳包(发送)
	DTU_CMD_HEART_SYN,							//心跳包(同步)
	DTU_CMD_ERR_LIST,							//ERROR(错误列表)
	DTU_CMD_ERR_CODE,							//ERROR(错误代码)
	DTU_CMD_ERROR_SEND,							//ERROR（发送）
	DTU_CMD_ERROR_SYN,							//ERROR（同步）
	DTU_CMD_REAL_FILE_SEND,						//实时文件（发送）
	DTU_CMD_REAL_FILE_SYN,						//实时文件（同步）
	DTU_CMD_HISTORY_FILE_SEND,					//历史文件（发送）
	DTU_CMD_HISTORY_FILE_SYN,					//历史文件（同步）
	DTU_CMD_UPDATE_EXE,							//更新程序
	DTU_CMD_UPDATE_POINT_CSV,					//更新CSV点表
	DTU_CMD_UPDATE_POINT_EXCEL,					//更新EXCEL点表
	DTU_CMD_UPDATE_DLL,							//更新DLL
	DTU_CMD_UPDATE_S3DB,						//更新s3db
	DTU_CMD_UPLOAD_POINT_CSV,					//上传CSV点表
	DTU_CMD_DELETE_POINT_MUL,					//批量删除点
	DTU_CMD_TIME_SYN,							//同步时钟
	DTU_CMD_RESTART_CORE,						//重启Core
	DTU_CMD_RESTART_DOG,						//重启Dog
	DTU_CMD_RESTART_LOGIC,						//重启Logic
	DTU_CMD_RESTART_DTU,						//重启DTU
	DTU_CMD_RESTART_DTU_ENGINE,					//重启DTUEngine
	DTU_CMD_CORE_STATUS,						//获取服务器状态
	DTU_CMD_VERSION_CORE,						//Core版本号
	DTU_CMD_VERSION_DOG,						//Dog版本号
	DTU_CMD_VERSION_DTU,						//DTU版本号
	DTU_CMD_UPDATE_BAT							//更新s3db
};

//接收数据结构
typedef struct DTU_RECEIVE_INFO_TAG
{
	int				nType;			//0:文本   1：文件
	int				nID;			//ID
	int				nCmdType;		//命令类型
	string			strCmdID;		//命令编号
	string			strContent;		//内容
	string			strTime;		//时间
}DTU_RECEIVE_INFO;

//接收数据结构
typedef struct DTU_UPDATE_INFO_TAG
{
	string			strCmdID;		//命令编号
	string			strFileName;	//文件名
	string			strTime;		//时间
}DTU_UPDATE_INFO;

//DTU发送结构
typedef struct DTU_DATA_INFO_TAG
{
	DTU_DATA_INFO_TAG()
	{
		strID = "0";
		nSubType = 0;
		nTryCount = 0;
		strTime = "";
		strFileName = "";
		strContent = "";
		nCmdType = DTU_CMD_DEFINE;
	}
	string		strID;							//编号
	string		strTime;						//时间
	string		strFileName;					//文件名
	int			nSubType;						//子类型   0表示数据文件  1表示其他文件
	int			nTryCount;						//尝试次数
	string		strContent;						//文本内容  Ansi
	DTU_CMD_TYPE nCmdType;						//命令类型
} DTU_DATA_INFO;

//IOT数据结构
typedef struct IOT_DATA_INFO_TAG
{
	IOT_DATA_INFO_TAG()
	{
		nLength = 0;
		memset(cData, 0, IOT_ONE_FILE_SIZE);
	}
	char	cData[IOT_ONE_FILE_SIZE];		//内容
	int		nLength;						//长度
	int		nType;							//0:文本命令  1:Json文本 2:压缩文件（zip）-不分包  3：压缩文件（zip）-分包
	int		nCmdType;						//命令类型
}IOT_DATA_INFO;

//文件结构
typedef struct FILE_INFO_TAG
{
	string strFileName;		//文件名
	string strFilePath;		//文件路径
}FILE_INFO;

//Iot
//#include "azure_c_shared_utility/platform.h"
//#include "azure_c_shared_utility/threadapi.h"
//#include "azure_c_shared_utility/crt_abstractions.h"
//#include "iothub_client.h"
//#include "iothub_message.h"
//#include "iothubtransportamqp.h"
//
//typedef struct EVENT_INSTANCE_TAG
//{
//	IOTHUB_MESSAGE_HANDLE messageHandle;
//	string messageTrackingId;  // For tracking the messages within the user callback.
//} EVENT_INSTANCE;

//TCP

const int g_nRecMaxNum = 8192;				//接收数据缓存大小
const int g_nPackageSize = 4096;			//每个数据包大小
typedef struct TCP_DATA_PACKAGE_TAG			//分包一个包
{
	TCP_DATA_PACKAGE_TAG()
	{
		nLength = 0;
		memset(cData, 0, g_nPackageSize);
	}
	char   cData[g_nPackageSize];			//内容
	int	   nLength;							//长度
} TCP_DATA_PACKAGE;

//RabbitMQ
#include <amqp_ssl_socket.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>
#include <utils.h>

//unit01数据结构
typedef struct  UNIT01_INFO_TAG
{
	string		strProterty01;				//参数1
	string		strProterty02;
	string		strProterty03;
}UNIT01_INFO;

//Data数据结构
typedef struct  REAL_DATA_INFO_TAG
{
	string		strTime;				//时间
	string		strPointName;			//点名
	string		strPointValue;			//值(Ansi)
}REAL_DATA_INFO;
