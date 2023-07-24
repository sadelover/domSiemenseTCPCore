#pragma once

#include "./Protocol104/TCPSocket.h"
#include "Tools/CustomTools/CustomTools.h"

#include <vector>
#include <utility>
#include <string>
#include <map>
using std::vector;
using std::pair;
using std::wstring;

#define WM_STATUS_CHANGED WM_USER+5
#define WM_DATA_ARRIVED WM_USER+6

typedef LRESULT (*ClientStatusProc)(TCP_CONNECT_STATE type,int nNo,char *ip,DWORD userdata); //客户端连接状态的回调函数
typedef LRESULT (*ServerDataProc)(char *ip,TCP_DATA_TYPE type,char *data,DWORD userdata); //服务器数据收发的回调函数

enum CMD_DATATYPE
{
	CMD_TYPE_LOGIN = 0,				//logincmd:
	CMD_TYPE_PRJNAME,				//prjnacmd:
	CMD_TYPE_PAGE,					//pagescmd:
	CMD_TYPE_POINT					//pointcmd:
};

class __declspec(dllexport) CIOSServer : public CTCPSocket
{
public:
	CIOSServer(void);
	virtual ~CIOSServer(void);

protected:
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:

	//状态回调
	static LRESULT OnStatusChange(TCP_CONNECT_STATE type,char *data,int nNo,DWORD userdata);
	static LRESULT OnServerDataArrive(int nNO,char *ip,char *data,int length,DWORD userdata);

	//Initialize Modbus Tcp Info
	bool	Init(ClientStatusProc proc1=NULL,ServerDataProc proc2=NULL,DWORD userdata=NULL);
	bool    Exit();

	ClientStatusProc  m_lpClientStatusProc;
	ServerDataProc	  m_lpServerDataProc;
	DWORD			  m_dwUserData;
	//map<char* ip, 数据库连接>      //find 没找到创建个连接     断开时  删除

private:
	Mutex						m_mutex;

};