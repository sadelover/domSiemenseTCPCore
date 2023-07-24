
#include "StdAfx.h"

#include "CDataEngineAppConfig.h"
#include <Tools/Scope_Ptr.h>

#define STR_MODBUS_SECTIONAME L"modbussetting"
#define STR_SENDCOMMAND_INTERNAL L"send_command_internal"

#define STR_REDUNDANCE_SECTIONNAME L"redundancesetting"
#define STR_HEARTBEAT_PACKET_CHECK_INTERNAL L"heartbeat_packet_check_internalseconds"
#define STR_MASTER_DEAD_JUDGEMENT_INTERNAL L"master_dead_judgement_internalseconds"
#define STR_IP_OTHER L"ip_other"
#define STR_IP_GATEWAY L"ip_gateway"
#define STR_MASTER_SLAVE_ENABLE L"enable"

#define STR_POINTCONFIG_NAME L"pointconfig"
#define STR_POINT_FROM_DB L"pointfromDB"
#define STR_DB_IP L"db_ip"
#define STR_DB_USER L"db_user"
#define STR_DB_PWD L"db_pwd"
#define STR_SCHEMA L"schema"
#define STR_DB_PORT L"db_port"

CDataEngineAppConfig::CDataEngineAppConfig(void)
	:m_masterslave_enable(0)
{
	m_pconfiginstance = NULL;
}

CDataEngineAppConfig::~CDataEngineAppConfig(void)
{
	}

int CDataEngineAppConfig::GetModbusSendCommandInternal()
{
	return m_modbus_sendcommand_internal;
}

void CDataEngineAppConfig::SetModbusSendCommandInternal( int sendinternal ) 
{
	m_modbus_sendcommand_internal = sendinternal;
	
}

wstring CDataEngineAppConfig::GetAnotherServerIP() 
{
	return m_ip_other;
}

void CDataEngineAppConfig::SetOtherServerIP( const wstring& ip_other )
{
	m_ip_other = ip_other;
}

CDataEngineAppConfig* CDataEngineAppConfig::GetInstance()
{
	if (!m_pconfiginstance)
	{
		Project::Tools::Scoped_Lock<Mutex> guardlock(m_singleton_lock);
		if (!m_pconfiginstance)
		{
			m_pconfiginstance = new CDataEngineAppConfig;
			if (m_pconfiginstance){
				atexit(CDataEngineAppConfig::DestroyInstance);
			}
		}

	}

	return m_pconfiginstance;
}

void CDataEngineAppConfig::DestroyInstance()
{
	if (m_pconfiginstance){
		delete m_pconfiginstance;
		m_pconfiginstance = NULL;
	}
}

wstring CDataEngineAppConfig::GetGateWayIP()
{
	return m_ip_gateway;
}

void CDataEngineAppConfig::SetGateWayIP( const wstring& ip_gateway )
{
	m_ip_gateway = ip_gateway;
}

int CDataEngineAppConfig::GetMasterSlaveEnable()
{
	return m_masterslave_enable;
}

void CDataEngineAppConfig::SetMasterSlaveEnable( int enablevalue )
{
	m_masterslave_enable = enablevalue;
}

Project::Tools::Mutex CDataEngineAppConfig::m_singleton_lock;

CDataEngineAppConfig* CDataEngineAppConfig::m_pconfiginstance = NULL;

