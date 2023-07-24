#pragma once
#include "StdAfx.h"
#include "IOSServerCtrl.h"
#include "atlstr.h"

#define IOSSERVER_PORT	8888
#define E_NUM_CMD_PREFIX	9

const char* LOGIN_CMD_PREFIX =		"logincmd:";
const char* PRJNAME_CMD_PREFIX =	"prjnaack:";
const char* PAGE_CMD_PREFIX =		"pagescmd:";
const char* POINT_CMD_PREFIX =		"pointcmd:";

CIOSServer::CIOSServer( void )
	: m_lpClientStatusProc(NULL)
	, m_lpServerDataProc(NULL)
{
	SetType(0);
}

CIOSServer::~CIOSServer( void )
{

}

bool CIOSServer::Init(ClientStatusProc proc1/*=NULL*/,ServerDataProc proc2/*=NULL*/,DWORD userdata/*=NULL*/)
{
	m_lpClientStatusProc = proc1;
	m_lpServerDataProc = proc2;
	m_dwUserData = userdata;

	if(!CreateServer(IOSSERVER_PORT,5))		//创建Server
		return false;

	StartServer(OnStatusChange,OnServerDataArrive,(DWORD)this);		//开始服务
	
	return true;
}

bool CIOSServer::Exit()
{
	m_lpClientStatusProc = NULL;
	m_lpServerDataProc = NULL;

	Disconnect();
	return true;
}

LRESULT CIOSServer::OnStatusChange(TCP_CONNECT_STATE type,
	char *data,int nNo,DWORD userdata )
{
	CIOSServer *pTcpCtrl=(CIOSServer *)userdata;
	if(!pTcpCtrl)
		return 0;

	if(pTcpCtrl->m_lpClientStatusProc != NULL)
	{
		pTcpCtrl->m_lpClientStatusProc(type,nNo,data,pTcpCtrl->m_dwUserData);
	}
	return 0;
}

LRESULT CIOSServer::OnServerDataArrive(int nNO,char *ip, char *data,int length,DWORD userdata )
{
	CIOSServer *pTcpCtrl=(CIOSServer *)userdata;
	if(!pTcpCtrl)
		return 0;

	if(pTcpCtrl->m_lpServerDataProc != NULL)
	{
		pTcpCtrl->m_lpServerDataProc(ip,TCP_DATA_RECEIVE,data,pTcpCtrl->m_dwUserData);
	}

	//解析数据
	if(length>E_NUM_CMD_PREFIX)
	{
		char buffer_cmd_prefix[E_NUM_CMD_PREFIX+1];
		memcpy(buffer_cmd_prefix, data, E_NUM_CMD_PREFIX);
		buffer_cmd_prefix[E_NUM_CMD_PREFIX] = '\0';

		if(strcmp(buffer_cmd_prefix, LOGIN_CMD_PREFIX) == 0)			// 登录
		{
			char content[256];
			memcpy(content, data+E_NUM_CMD_PREFIX, length-E_NUM_CMD_PREFIX);

			char* name;
			char* psw;
			name = strtok(content, ","); 
			psw = strtok(NULL, ";");
			psw = strtok(psw, ";");		
		}
		else if(strcmp(buffer_cmd_prefix, PAGE_CMD_PREFIX) == 0)			// 页面：点名
		{
			char source[1024];
			memcpy(source, data+E_NUM_CMD_PREFIX, length-E_NUM_CMD_PREFIX);

			char* strcopy = strtok(source, ";"); 
			char* token = NULL;
			char* nexttoken = NULL;
			token = strtok_s(strcopy, ",", &nexttoken);

			vector<char*> vecPointName;
			while(token != NULL)
			{
				vecPointName.push_back(token);
				token = strtok_s(NULL, ",", &nexttoken);
			}
		}
	}

	//回复数据

	return 0;
}

bool CIOSServer::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	return true;
}
