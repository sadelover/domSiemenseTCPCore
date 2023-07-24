#pragma once
#include "StdAfx.h"
#include "TelnetTcpCtrl.h"
#include "atlstr.h"

const int default_portnumber = 23;

CTelnetTcpCtrl::CTelnetTcpCtrl(void)
	:m_host("")
	,m_bSave(false)
	,m_port(default_portnumber)
{
	
}

CTelnetTcpCtrl::~CTelnetTcpCtrl(void)
{
	Exit();
}

// notify received package, use for sub-class derivativation
bool CTelnetTcpCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(strstr((const char*)pRevData,"Password: ")>0)
	{
		return SendPsw();
	}

	if(strstr((const char*)pRevData,"Router>")>0 && length <20)
	{
		return SendShowCmd();
	}

	if(strstr((const char*)pRevData,"show modem-information")>0 && length>0)
	{
		return SaveInformation((const char*)pRevData,length);
	}
	return false;
}

bool CTelnetTcpCtrl::Init(const string& strhost)
{
	m_host = strhost;
	return TcpIpConnectComm(m_host, m_port);
}

bool CTelnetTcpCtrl::Exit()
{
	Disconnect();		//¹Ø±ÕTCP
	return true;
}

bool CTelnetTcpCtrl::SendShowCmd()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);
	char cPsw[23] = {0};
	cPsw[0] = 0x73;
	cPsw[1] = 0x68;
	cPsw[2] = 0x6F;
	cPsw[3] = 0x77;
	cPsw[4] = 0x20;
	cPsw[5] = 0x6D;
	cPsw[6] = 0x6F;
	cPsw[7] = 0x64;
	cPsw[8] = 0x65;
	cPsw[9] = 0x6D;
	cPsw[10] = 0x2D;
	cPsw[11] = 0x69;
	cPsw[12] = 0x6E;
	cPsw[13] = 0x66;
	cPsw[14] = 0x6F;
	cPsw[15] = 0x72;
	cPsw[16] = 0x6D;
	cPsw[17] = 0x61;
	cPsw[18] = 0x74;
	cPsw[19] = 0x69;
	cPsw[20] = 0x6F;
	cPsw[21] = 0x6E;
	cPsw[22] = 0x0D;
	return SendPackage(cPsw, sizeof(cPsw));
}

bool CTelnetTcpCtrl::SendPsw()
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);
	char cPsw[6] = {0};
	cPsw[0] = 0x73;
	cPsw[1] = 0x75;
	cPsw[2] = 0x70;
	cPsw[3] = 0x65;
	cPsw[4] = 0x72;
	cPsw[5] = 0x0D;
	return SendPackage(cPsw, sizeof(cPsw));
}

bool CTelnetTcpCtrl::SaveInformation(const char* pInformation,int nLength)
{
	try
	{
		wstring wstrFileFolder;
		Project::Tools::GetSysPath(wstrFileFolder);
		SYSTEMTIME sNow;
		GetLocalTime(&sNow);
		wstrFileFolder += L"\\log";
		if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
		{
			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );

			CString strOut;
			wstring strTime;
			Project::Tools::SysTime_To_String(sNow,strTime);
			strOut.Format(_T("%s -->\n"),strTime.c_str());

			CString strLogPath;
			strLogPath.Format(_T("%s\\routerstate_%d_%02d_%02d.log"),wstrFileFolder.c_str(),sNow.wYear,sNow.wMonth,sNow.wDay);

			//¼ÇÂ¼Log
			CStdioFile	ff;
			if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			{
				ff.Seek(0,CFile::end);
				ff.WriteString(strOut);
				ff.Write(pInformation,nLength);
				ff.Close();
				setlocale( LC_CTYPE, old_locale ); 
				free( old_locale ); 
				m_bSave = true;
				return true;
			}
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
		}
		m_bSave = true;
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

bool CTelnetTcpCtrl::GetSaveFlag()
{
	return m_bSave;
}
