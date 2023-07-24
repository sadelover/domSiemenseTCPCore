#pragma once

#include "TcpClient.h"
#include "Tools/CustomTools/CustomTools.h"

#include <vector>
#include <utility>
#include <string>
using std::vector;
using std::pair;
using std::wstring;

class CTelnetTcpCtrl : public CTcpClient
{
public:
	CTelnetTcpCtrl(void);
	virtual ~CTelnetTcpCtrl(void);

protected:
	
	//解析通讯回来的包.
	virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

public:
	bool	Init(const string& strhost);
	bool	Exit();

	bool	SendPsw();
	bool	SendShowCmd();
	bool	SaveInformation(const char* pInformation,int nLength);
	bool	GetSaveFlag();
private:
	string		m_host;
	u_short		m_port;
	bool		m_bSave;
	Project::Tools::Mutex	m_lock;
};

