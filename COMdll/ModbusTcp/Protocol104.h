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

#include "DB_BasicIO/RealTimeDataAccess.h"

//Frame帧类型
enum TCP_FRAME_TYPE
{
	PROTOCOL_TYPEID64H =0,       //总召唤
	PROTOCOL_TYPEID67H,			//主站对时  
	PROTOCOL_TYPEID2DH,    //单点遥控 
	PROTOCOL_TYPEID2EH,    //双点遥控   
	PROTOCOL_TYPEID65H,	//二进制计数器读数(BCR) -- 电度累计量   
	PROTOCOL_TYPEID01H,   //（单点YX报文）
	PROTOCOL_TYPEID0DH      //段浮点遥测
};

class CProtocol104Ctrl : public CTcpIpComm
{
public:
	CProtocol104Ctrl(void);
	virtual ~CProtocol104Ctrl(void);

protected:
	
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:
	void	SetHost(const string& strhost);
	void	SetPort(u_short portnumer);

	void	SetPointList(const vector<DataPointEntry>& pointlist);
	void	GetValueSet( vector< pair<wstring, wstring> >& valuelist );
	DataPointEntry*	FindEntry(int nFrameType, int nPointID);

	//Initialize Modbus Tcp Info
	bool Init();
	bool	Exit();

	static UINT WINAPI ThreadSendCommandsFunc(LPVOID lparam);
	static UINT WINAPI ThreadCheckAndRetryConnection(LPVOID lparam);
	void	TerminateSendThread();

	//从包里分帧解析
	void	GetFrameFromPackage(BYTE *msgbuf, int len);

	double	GetMultiple(wstring strMultiple);


	//帧解析
	void UnPack_Start68H(BYTE *msgbuf, int len);   //
	void UnPack_TypeID64H(BYTE *msgbuf,int len );		//总召唤 
	void UnPack_TypeID67H(BYTE *msgbuf,int len );     //主站对时   
	void UnPack_TypeID2DH(BYTE *msgbuf,int len );     //单点遥控   
	void UnPack_TypeID2EH(BYTE *msgbuf,int len );     //双点遥控   
	void UnPack_TypeID65H(BYTE *msgbuf,int len );    //二进制计数器读数(BCR) -- 电度累计量   
	void UnPack_TypeID01H(BYTE *msgbuf,int len );    //（单点YX报文）
	void UnPack_TypeID0DH(BYTE *msgbuf,int len );    //段浮点遥测
	void UnPack_Start68H_Unknow(BYTE *msgbuf,int len ); 

	void	SendPack_TypeID64H();					//发送总召唤
	int		GetSendInterval();

	void    SetNetworkError();
	void    ClearNetworkError();

	bool	ReConnect();
private:
	string		m_host;
	u_short		m_port;
	HANDLE		m_hsendthread;
	bool		m_bExitThread;
	vector<DataPointEntry> m_pointlist;	//点表.
	Project::Tools::Mutex	m_lock;
	BOOL		m_bLog;
	Beopdatalink::CLogDBAccess* m_logsession;
	int			m_nSendReadCmdTimeInterval;				//两次命令间隔
	
	//自动重连机制
	Project::Tools::Mutex	m_lock_connection;
	HANDLE		m_hDataCheckRetryConnectionThread;
	int			m_nFCBusNetworkErrorCount;

	COleDateTime	m_oleUpdateTime;
	string			m_strErrInfo;
	int				m_nEngineIndex;

	int			m_IRPacketNum;//I帧接收数目
	int			m_ISPacketNum;//I帧发送数目
	BYTE		m_IPacket[16];//I帧总召
	int			m_nSendInterval;
};

