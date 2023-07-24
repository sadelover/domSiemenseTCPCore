#pragma once

#include "TcpIpComm.h"
#include "BEOPDataPoint/DataPointEntry.h"
#include "Tools/CustomTools/CustomTools.h"

#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

/*
	param6:类型
	0：Signed				1		
	1: UnSigned				1
	2: Bite					0-15   （WORD取第N位）
	3: Long					2
	4: Long Inverse			2
	5: Float				2
	6: Float Inverse		2
	7: Double				4
	8: Double Inverse		4
	9：String 				N个寄存器
	10：String Inverse 		N个寄存器(每个寄存器数据取反)
	11：PowerLink		 	3  (3个寄存器 6个字节 前两位可读写值，后1位功能码只写不能读)
/*
 *
 *用于modbus命令打包发送
 *
 */

/*
MODBUS异常码
代码	名称	含义
01	非法功能	对于服务器(或从站)来说，询问中接收到的功能码是不可允许的操作。这也许是因为功能码仅仅适用于新设备而在被选单元中是不可实现的。同时，还指出服务器(或从站)在错误状态中处理这种请求，例如：因为它是未配置的，并且要求返回寄存器值。
02	非法数据地址	对于服务器(或从站)来说，询问中接收到的数据地址是不可允许的地址。特别是，参考号和传输长度的组合是无效的。对于带有100个寄存器的控制器来说，带有偏移量96和长度4的请求会成功，带有偏移量96和长度5的请求将产生异常码02。
03	非法数据值	对于服务器(或从站)来说，询问中包括的值是不可允许的值。这个值指示了组合请求剩余结构中的故障，例如：隐含长度是不正确的。并不意味着，因为MODBUS协议不知道任何特殊寄存器的任何特殊值的重要意义，寄存器中被提交存储的数据项有一个应用程序期望之外的值。
04	从站设备故障	当服务器(或从站)正在设法执行请求的操作时，产生不可重新获得的差错。
05	确认	与编程命令一起使用。服务器(或从站)已经接受请求，并切正在处理这个请求，但是需要长的持续时间进行这些操作。返回这个响应防止在客户机(或主站)中发生超时错误。客户机(或主站)可以继续发送轮询程序完成报文来确定是否完成处理。
06	从属设备忙	与编程命令一起使用。服务器(或从站)正在处理长持续时间的程序命令。张服务器(或从站)空闲时，用户(或主站)应该稍后重新传输报文。
08	存储奇偶性差错	与功能码20和21以及参考类型6一起使用，指示扩展文件区不能通过一致性校验。
服务器(或从站)设法读取记录文件，但是在存储器中发现一个奇偶校验错误。客户机(或主方)可以重新发送请求，但可以在服务器(或从站)设备上要求服务。
0A	不可用网关路径	与网关一起使用，指示网关不能为处理请求分配输入端口至输出端口的内部通信路径。通常意味着网关是错误配置的或过载的。
0B	网关目标设备响应失败	与网关一起使用，指示没有从目标设备中获得响应。通常意味着设备未在网络中。
*/

/*
错误码：
0 无错误
1 内存范围错误
2 非法波特率或校验
3 非法从属地址
4 非法Modbus参数值
5 保持寄存器与Modbus从属符号重叠
6 收到校验错误
7 收到CRC错误
8 非法功能请求／功能不受支持
9 请求中的非法内存地址
10 从属功能未启用
*/
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "Tools/StructDefine.h"

const unsigned char m_auchCRCHi[]=
{
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
	0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
	0x80,0x41,0x00,0xC1,0x81,0x40 
};

const unsigned char m_auchCRCLo[]=
{
	0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
	0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
	0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
	0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
	0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
	0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
	0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
	0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
	0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
	0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
	0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
	0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
	0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
	0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
	0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
	0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
	0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
	0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
	0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
	0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
	0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
	0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
	0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
	0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
	0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
	0x43,0x83,0x41,0x81,0x80,0x40 
};

//enum E_MODBUS_VALUE_TYPE
//{
//	E_MODBUS_SIGNED = 0,
//	E_MODBUS_UNSIGNED,
//	E_MODBUS_BITE,
//	E_MODBUS_LONG,
//	E_MODBUS_LONG_INVERSE,
//	E_MODBUS_FLOAT,
//	E_MODBUS_FLOAT_INVERSE,
//	E_MODBUS_DOUBLE,
//	E_MODBUS_DOUBLE_INVERSE,
//	E_MODBUS_STRING,
//	E_MODBUS_STRING_INVERSE,
//	E_MODBUS_POWERLINK,			//11 争对powerLink 3位寄存器，前两位可读，后一位只写
//};
//
//struct _ModbusReadUnit
//{
//	wstring strPointName;
//	unsigned int nSlaveId;
//	DWORD dwAddFrom;
//	DWORD dwAddTo;
//	DWORD dwFuncCode;
//	int nReadSuccessCount;
//	bool	bHasErrPoint;
//	bool	bMultiRead;			//true 是批量读取  false 是单点读取   批量读取错误 切换到单点读一次 再切回批量
//};
//
//struct _ModbusWriteCmd
//{
//	WORD dwAddFrom;
//	WORD dwFuncCode;
//	WORD dwValue;
//};

class CModbusTcpCtrl : public CTcpIpComm
{
public:
	CModbusTcpCtrl(void);
	virtual ~CModbusTcpCtrl(void);

protected:
	
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:
	void	SetHost(const string& strhost);
	void	SetPort(u_short portnumer);

	//Initialize Modbus Tcp Info
	bool Init();
	bool	Exit();

	//Send Read Command
	virtual bool SendReadCmd(unsigned int nSlaveId   //设备的Slave ID 号
							, WORD wAddrFrom		//读取属性的起始地址
							, WORD wAddrTo			//读取属性的终止地址
							, WORD wFunc);			//设备通讯类型

	//Send Write Command
	virtual bool SendWriteCmd(unsigned int nSlaveId   //设备的Slave ID 号
							, WORD wAddress			//写入属性的地址
							, WORD wValue			//属性值
							, WORD wFunc);			//设备通讯类型
	
	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	void	TerminateSendThread();

	
	void    SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount=99,int nIDInterval = 500,int nTimeOut=5000,int nPollInterval = 2,int nPrecision=6,bool bDisSingleRead = false);
	void	SetReadOneByOneMode(bool bReadOneByOneMode);
	void	SetPointList(const vector<DataPointEntry>& pointlist);
	
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );

	DataPointEntry*	FindEntry(DWORD slaveid, DWORD headaddress, DWORD funccode);

	bool	GetValue(const wstring& pointname, double& result);
	bool	SetValue(const wstring& pointname, double fValue);

	bool	InitDataEntryValue(const wstring& pointname, double value);

	void	AddLog(const wchar_t* loginfo);
	void	SetLogSession(Beopdatalink::CLogDBAccess* logsesssion);
	void	EnableLog(BOOL bEnable);
	void    SortPointByDevice();

	//////////////////////////////////////////////////////////////////////////
	void    SortPointByDevice_();
	void	CreateEntryByDataTypeAndRange(DataPointEntry entry,vector<DataPointEntry>& vecModbus);
	void	GetValueByDataTypeAndRange(DataPointEntry entry,vector< pair<wstring, wstring> >& valuelist);		
	void	GetValueByDataTypeAndRange(DataPointEntry entry,E_MODBUS_VALUE_TYPE type,vector<WORD>& vecValue,vector< pair<wstring, wstring> >& valuelist);
	void	GetValueByDataTypeAndRange(DataPointEntry entry,double dValue,vector<WORD>& vecCmd,bool bOrder);		//ture默认从高字节往低排
	void	SetReadCommands_();

	////使用激活命令模式//////////////////////////////////////////////////////////////////////
	void	SendReadCommandsByActive();				//回复激活模式
	void	SetSendCmdEvent();						//设置激活事件
	HANDLE	GetSendCmdEvent();						//获取激活事件
	void	SendOneByOneByActive();
	bool	SendOneByOneByActive(_ModbusReadUnit& unit);
	//////////////////////////////////////////////////////////////////////////

	void	SetReadCommandsWithError();			//若某一个modbus设备有地址错误，该部分单独读取 其他仍然连续读取
	DataPointEntry*	FindEntry_(DWORD slaveid, DWORD headaddress, DWORD funccode);
	void	SetModbusReadUnitErrFlag(unsigned int nSlaveId,DWORD dwFuncCode,DWORD dwAdd,bool bHasErrPoint=true);
	//转换函数
	int	GetBitFromWord(WORD nData, int nIndex);
	WORD	SetBitToWord(WORD nData, int nIndex,int nValue);
	DWORD MergeTwoWORD(WORD wGao,WORD wDi,bool bOrder = true);		//
	double MergeFourDouble(WORD wGaoGao,WORD wGao,WORD wDi,WORD wDiDi,bool bOrder = true);		//
	double			strtodbl(const wchar_t * str);
	bool	SendWriteCmd(DataPointEntry entry,double fValue);			//设备通讯类型
	bool	SendWriteMutilCmd(DataPointEntry entry,double fValue);			//设备通讯类型
	bool	SendWriteBYTECmd(DataPointEntry entry,double fValue);			//设备通讯类型
	bool	SendWriteWORDCmd(DataPointEntry entry,double fValue);			//设备通讯类型

	bool	SendTestWORDCmd();			//开机
	bool	SendTestCloseWORDCmd();			//关机
	bool	SendTestHornResetWORDCmd();			//Horn Reset
	bool	SendTestFaultResetWORDCmd();			//Fault Reset
	bool	SendTestOPenCloseWORDCmd();
	bool	SendTestCloseOPenWORDCmd();

	void	SendOneByOne_();
	bool	SendOneByOne_(_ModbusReadUnit& unit);
	bool	SendReadMutil_(_ModbusReadUnit& unit);
	bool	MutilReadHasErr();
	unsigned short CalcCrcFast(unsigned char*puchMsg,unsigned short usDataLen);
	//////////////////////////////////////////////////////////////////////////
	bool	CombineReadUnit(const _ModbusReadUnit &  mrUnit,bool bModbusEnd=true);

	bool	ReConnect();

	void    SetNetworkError();
	void    ClearNetworkError();

	void	SendReadCommandOneByOne();			//一个个发送
	void	SendOneByOne();
	void    SortBackupPointByDevice();
	void	SetBackupReadCommands();

	void	SetIndex(int nIndex);
	bool	SetDebug(int nDebug);
	bool	OutPutModbusReceive(const unsigned char* pRevData, DWORD length);
	string* byteToHexStr(unsigned char* byte_arr, int arr_len);  
	string  charToHexStr(unsigned char* byte_arr, int arr_len); 

	int		GetMultiReadCount(DWORD slaveid, DWORD headaddress, DWORD funccode);
	void	SumReadAndResponse();	
	void 	PrintLog(const wstring &strLog,bool bSaveLog = true);
private:
	string	m_host;
	u_short	m_port;
	HANDLE	m_hsendthread;
	bool m_bExitThread;
	vector<DataPointEntry> m_pointlist;	//点表.
	vector<DataPointEntry> m_backuppointlist;	//点表.
	vector<DataPointEntry>	m_vecPointList;
	vector<_ModbusReadUnit> m_ReadUnitList; //读取数据包单元，需合并点从而加速读取
	map<string,WORD>		m_ModbusValue;		//string 设备ID_点地址_功能码
	map<wstring,DataPointEntry*>	m_mapErrPoint;		//读取错误点
	Project::Tools::Mutex	m_lock;
	BOOL m_bLog;
	Beopdatalink::CLogDBAccess* m_logsession;
	int m_nSendReadCmdTimeInterval;				//两次命令间隔
	int							m_nRecevieTimeOut;						//接收超时
	int	m_nPollSendInterval;				//轮询间隔
	int m_nRecvedSlave;

	bool	m_bSortPoint;		//重排点表中
	int		m_nSendCmdCount;	//发送100次判断下点表是否有误，有误重排一次
	int		m_nMutilReadCount;		//批量连读个数 默认99

	bool	m_bReadOneByOneMode;
	bool	m_bDisSingleRead;
	//自动重连机制
	Project::Tools::Mutex	m_lock_connection;
	HANDLE m_hDataCheckRetryConnectionThread;
	int m_nModbusNetworkErrorCount;

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;
	int				m_nIDInterval;			//不同ID之间的间隔 默认500ms
	int				m_nOutPut;				//输出接收到的cmd
	int				m_nPrecision;				//数据精度 默认6

	int									m_nCmdCount;						//命令个数
	int									m_nResponseCount;					//请求回复个数
	int									m_nUpdatePointCount;				//更新点
	string								m_strUpdateLog;						//更新log

	_ModbusReadUnit				m_lastReadUnit;						//上一次读取结构
	DataPointEntry				m_lastReadEntry;
	HANDLE						m_sendCmdEvent;
};

