

/*
 * Data Engine运行时的动态参数类.
 * 可以动态修改的参数.
 * 程序结束时会写入到ini文件.启动时从ini文件中load.
 *
 * 这个类将被实现为singleton
 */

#pragma once

#include <Tools/CustomTools/CustomTools.h>
#include <string>
using std::wstring;

struct MDODBInfo
{
	wstring strHost;
	wstring strUser;
	wstring strPwd;
	wstring strSchema;
	int		nPort;
};

class CDataEngineAppConfig
{
private:
	CDataEngineAppConfig(void);

public:
	static CDataEngineAppConfig* GetInstance();
	static void DestroyInstance();

	~CDataEngineAppConfig(void);

	int	GetModbusSendCommandInternal() ;
	void SetModbusSendCommandInternal(int sendinternal);

	wstring GetAnotherServerIP() ;
	void SetOtherServerIP(const wstring& ip_other);

	wstring GetGateWayIP();
	void SetGateWayIP(const wstring& ip_gateway);

	int GetMasterSlaveEnable();
	void SetMasterSlaveEnable(int enablevalue);

	wstring	GetIniFileFullPath();

private:
	int		m_modbus_sendcommand_internal;			//modbus命令发送时间间隔
	int		m_heartbeat_checkinternal_seconds;		//心跳包发送时间间隔
	int		m_master_deadjudgement_seconds;			//主机掉线的持续时间,如果判断主机掉线超过这一时间设定,则认为主机掉线.
	int		m_masterslave_enable;	//是否启用master，slave冗余模式。
	std::wstring m_ip_other;	//另一台服务器的ip
	std::wstring m_ip_gateway;	//网关的ip

	Project::Tools::Mutex	m_lock;

	static CDataEngineAppConfig* m_pconfiginstance;
	static Project::Tools::Mutex m_singleton_lock;
};

