#include "StdAfx.h"
#include "CO3PLink.h"
#include "../COMdll/ModbusTcp/CKECO3PTcpCtrl.h"
#include "Tools/CustomTools/CustomTools.h"
#include "BEOPDataLinkManager.h"

CCO3PEngine::CCO3PEngine(CBEOPDataLinkManager* datalinker)
	:m_datalinker(datalinker)
	,m_co3psession(new CCKECO3PTcpCtrl())
	,m_bTCPMode(true)
	,m_strServerhost(L"")
	,m_strServerCom(L"")
	,m_strServerBaud(L"9600")
	,m_strServerParity(L"N")
	,m_strServerPort(L"502")
	,m_strServerIP(L"")
{
}


CCO3PEngine::~CCO3PEngine(void)
{
	if (m_co3psession)
	{
		delete m_co3psession;
		m_co3psession = NULL;
	}
}

void CCO3PEngine::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
	//查找启用模式
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		m_strServerhost = entry.GetParam(1);
		if (!m_strServerhost.empty()){
			break;
		}
	}

	m_strServerIP = m_strServerhost;
	if (!m_strServerhost.empty())
	{
		//判断启用TCP还是RTU
		vector<wstring> vecParam;
		Project::Tools::SplitStringByChar(m_strServerhost.c_str(),L'/',vecParam);
		if(vecParam.size() == 2)			//端口/波特率
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

void CCO3PEngine::SetSendCmdTimeInterval( int nTimeMs,int nRollInterval/*=2*/,int nTimeOut/*=5000*/,int nPrecision)
{
	if(m_co3psession)
	{
		m_co3psession->SetSendCmdTimeInterval(nTimeMs,nRollInterval,nTimeOut,nPrecision);
	}
}

bool CCO3PEngine::Init()
{
	if (m_pointlist.empty()){
		return true;
	}

	if(m_bTCPMode)
	{
		if (m_strServerIP.empty())
		{
			Project::Tools::PrintLog(L"ERROR: CO3P TCP Host Empty!\r\n");
			return false;
		}

		string serverhost_ansi = Project::Tools::WideCharToAnsi(m_strServerIP.c_str());
		m_co3psession->SetHost(serverhost_ansi);
		m_co3psession->SetPort(_wtoi(m_strServerPort.c_str()));
		m_co3psession->SetPointList(m_pointlist);
		return m_co3psession->Init();
	}
	else
	{
		return false;
	}
	return false;
}

void CCO3PEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	if(m_bTCPMode)
	{
		if(m_co3psession)
		{
			m_co3psession->GetValueSet(valuelist);
		}
	}
	else
	{
		
	}
}

void CCO3PEngine::SetValue( const wstring& pointname, double fValue )
{
	if(m_bTCPMode)
	{
		
	}
	else
	{
		
	}
}

void CCO3PEngine::ExitThread()
{
	if (m_co3psession)
	{
		m_co3psession->Disconnect();
		m_co3psession->TerminateSendThread();
	}
}

bool CCO3PEngine::GetValue( const wstring& pointname, double& result )
{
	if(m_bTCPMode)
	{
		if (m_co3psession)
		{
			return m_co3psession->GetValue(pointname, result);
		}
	}
	else
	{
		
	}
	return false;
}


bool CCO3PEngine::InitDataEntryValue(const wstring& pointname, double value)
{
	return false;
}

void CCO3PEngine::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CCO3PEngine::AddLog( const wchar_t* loginfo )
{
	if (m_logsession)
	{
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

bool CCO3PEngine::Exit()
{
	if(m_bTCPMode)
	{
		if (m_co3psession)
		{
			return m_co3psession->Exit();
		}
	}
	else
	{
	}
	return true;
}

void CCO3PEngine::SetIndex( int nIndex )
{
	if(m_bTCPMode)
	{
		if (m_co3psession)
		{
			m_co3psession->SetIndex(nIndex);
		}
	}
	else
	{
		
	}
}

void CCO3PEngine::SetDebug( int nDebug )
{
	
}
