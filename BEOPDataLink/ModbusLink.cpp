#include "StdAfx.h"
#include "ModbusLink.h"


#include "../COMdll/ModbusTcp/ModbusTcpCtrl.h"
#include "../COMdll/MobusRtu/ModbusRTUCtrl.h"
#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CModbusEngine::CModbusEngine(CBEOPDataLinkManager* datalinker)
	:m_datalinker(datalinker)
	,m_modbussession(new CModbusTcpCtrl())
	,m_modbusRtusession(new CModbusRTUCtrl())
	,m_bTCPMode(true)
	,m_strServerhost(L"")
	,m_strServerCom(L"")
	,m_strServerBaud(L"9600")
	,m_strServerParity(L"N")
	,m_strServerPort(L"502")
	,m_strServerIP(L"")
{
}


CModbusEngine::~CModbusEngine(void)
{
	if (m_modbussession)
	{
		delete m_modbussession;
		m_modbussession = NULL;
	}

	if(m_modbusRtusession)
	{
		delete m_modbusRtusession;
		m_modbusRtusession = NULL;
	}
}

void CModbusEngine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
	//查找启用模式
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		m_strServerhost = entry.GetServerAddress();
		if (!m_strServerhost.empty())
		{
			break;
		}
	}

	m_strServerIP = m_strServerhost;
	if (!m_strServerhost.empty())
	{
		//判断启用TCP还是RTU
		vector<wstring> vecParam;
		Project::Tools::SplitStringByChar(m_strServerhost.c_str(),L'/',vecParam);

		if(vecParam.size() == 1)
		{
			CString strParam = vecParam[0].c_str();
			bool bIPMode = false;
			if(Project::Tools::IsValidIP(Project::Tools::WideCharToAnsi(strParam).c_str()))
			{
				bIPMode = true;
			}

			if(bIPMode)
			{
				m_strServerIP = vecParam[0];
				m_strServerPort = L"502";
				m_bTCPMode = true;
			}
			else
			{
				m_strServerCom = vecParam[0];
				m_strServerBaud = 9600;
				m_bTCPMode = false;
			}
		}
		else if(vecParam.size() == 2)			//端口/波特率
		{
			CString strParam = vecParam[0].c_str();
			bool bIPMode = false;
			if(Project::Tools::IsValidIP(Project::Tools::WideCharToAnsi(strParam).c_str()))
			{
				bIPMode = true;
			}

			if(bIPMode)
			{
				m_strServerIP = vecParam[0];
				m_strServerPort = vecParam[1];
				m_bTCPMode = true;
			}
			else
			{
				m_strServerCom = vecParam[0];
				m_strServerBaud = vecParam[1];
				m_bTCPMode = false;
			}
		}
		else if(vecParam.size() == 3)			//端口/波特率/听制服
		{
			m_strServerCom = vecParam[0];
			m_strServerBaud = vecParam[1];
			m_strServerParity = vecParam[2];
			m_bTCPMode = false;
		}
	}
}

void CModbusEngine::SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount,int nIDInterval,int nTimeOut,int nPollInterval,int nPrecision,bool bDisSingleRead,bool bSaveErrLog)
{
	m_modbussession->SetSendCmdTimeInterval(nTimeMs,nMutilReadCount,nIDInterval,nTimeOut,nPollInterval,nPrecision,bDisSingleRead);
	m_modbusRtusession->SetSendCmdTimeInterval(nTimeMs,nMutilReadCount,nIDInterval,nTimeOut,nPollInterval,nPrecision,bDisSingleRead,bSaveErrLog);
}

bool CModbusEngine::Init()
{
	if (m_pointlist.empty()){
		return true;
	}

	if(m_bTCPMode)
	{
		if (m_strServerIP.empty())
		{
			Project::Tools::PrintLog(L"ERROR: Modbus TCP Host Empty!\r\n");
			return false;
		}

		string serverhost_ansi = Project::Tools::WideCharToAnsi(m_strServerIP.c_str());
		m_modbussession->SetHost(serverhost_ansi);
		m_modbussession->SetPort(_wtoi(m_strServerPort.c_str()));
		m_modbussession->SetPointList(m_pointlist);
		m_modbussession->SetLogSession(m_logsession);
		return m_modbussession->Init();
	}
	else
	{
		if (m_strServerCom.empty())
		{
			Project::Tools::PrintLog(L"ERROR: Modbus RTU Com Empty!\r\n");
			return false;
		}
		if(m_strServerParity.length()>0)
		{
			m_modbusRtusession->SetPortAndBaud(_wtoi(m_strServerCom.c_str()),_wtoi(m_strServerBaud.c_str()),m_strServerParity[0]);
		}
		else
		{
			m_modbusRtusession->SetPortAndBaud(_wtoi(m_strServerCom.c_str()),_wtoi(m_strServerBaud.c_str()));
		}
		m_modbusRtusession->SetPointList(m_pointlist);
		m_modbusRtusession->SetLogSession(m_logsession);
		return m_modbusRtusession->Init();
	}

}

void CModbusEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	if(m_bTCPMode)
	{
		if(m_modbussession)
		{
			m_modbussession->GetValueSet(valuelist);
		}
	}
	else
	{
		if(m_modbusRtusession)
		{
			m_modbusRtusession->GetValueSet(valuelist);
		}
	}
}

void CModbusEngine::SetValue( const wstring& pointname, double fValue )
{
	if(m_bTCPMode)
	{
		if(m_modbussession)
		{
			m_modbussession->SetValue(pointname, fValue);
		}
	}
	else
	{
		if(m_modbusRtusession)
		{
			m_modbusRtusession->SetValue(pointname, fValue);
		}
	}
}

void CModbusEngine::ExitThread()
{
	if (m_modbussession)
	{
		m_modbussession->Disconnect();
		m_modbussession->TerminateSendThread();
	}
}

bool CModbusEngine::GetValue( const wstring& pointname, double& result )
{
	if(m_bTCPMode)
	{
		if (m_modbussession)
		{
			return m_modbussession->GetValue(pointname, result);
		}
	}
	else
	{
		if (m_modbusRtusession){
			return m_modbusRtusession->GetValue(pointname, result);
		}
	}
	return false;
}


bool CModbusEngine::InitDataEntryValue(const wstring& pointname, double value)
{
	if(m_bTCPMode)
	{
		if (m_modbussession)
		{
			return m_modbussession->InitDataEntryValue(pointname, value);
		}
	}
	else
	{
		if (m_modbusRtusession)
		{
			return m_modbusRtusession->InitDataEntryValue(pointname, value);
		}
	}
	return false;
}

void CModbusEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CModbusEngine::AddLog( const wchar_t* loginfo )
{
	if (m_logsession)
	{
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

bool CModbusEngine::Exit()
{
	if(m_bTCPMode)
	{
		if (m_modbussession)
		{
			return m_modbussession->Exit();
		}
	}
	else
	{
		if (m_modbusRtusession)
		{
			return m_modbusRtusession->Exit();
		}
	}
	return true;
}

void CModbusEngine::SetReadOneByOneMode( bool bReadOneByOneMode )
{
	if(m_bTCPMode)
	{
		if (m_modbussession)
		{
			m_modbussession->SetReadOneByOneMode(bReadOneByOneMode);
		}
	}
	else
	{
		if (m_modbusRtusession)
		{
			m_modbusRtusession->SetReadOneByOneMode(bReadOneByOneMode);
		}
	}
}

void CModbusEngine::SetIndex( int nIndex )
{
	if(m_bTCPMode)
	{
		if (m_modbussession)
		{
			m_modbussession->SetIndex(nIndex);
		}
	}
	else
	{
		if (m_modbusRtusession)
		{
			m_modbusRtusession->SetIndex(nIndex);
		}
	}
}

void CModbusEngine::SetDebug( int nDebug )
{
	if(m_bTCPMode)
	{
		if (m_modbussession)
		{
			m_modbussession->SetDebug(nDebug);
		}
	}
	else
	{
		if (m_modbusRtusession)
		{
			m_modbusRtusession->SetDebug(nDebug);
		}
	}
}

vector<DataPointEntry> & CModbusEngine::GetPointList()
{
	return m_pointlist;
}
