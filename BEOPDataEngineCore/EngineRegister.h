#pragma once

#include "DB_BasicIO/RealTimeDataAccess.h"


class Beopdatalink::CCommonDBAccess;

class CEngineRegister
{
public:
	CEngineRegister(Beopdatalink::CCommonDBAccess* pdbcon_user);
	~CEngineRegister(void);

	bool	Init();

	static UINT WINAPI	ThreadFunc(LPVOID lpvoid);

	bool	RegisterDataEngine();
	
	bool	GetThreadExit();
	void		SetThreadExit();

	bool	IsEngineOnline();

	void	UpdatePLCConnectionStatus();
private:
	Beopdatalink::CCommonDBAccess*	m_dbcon_common;
	
	HANDLE	m_threadhandle;
	bool	m_bexitthread;

	string m_strOpcIP;
};

