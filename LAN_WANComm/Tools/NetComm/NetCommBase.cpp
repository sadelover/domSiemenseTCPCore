// NetCommBase.h implement

#include "stdafx.h"
#include "NetCommBase.h"


CNetCommBase::CNetCommBase() : m_nPort(0)
{
	InitSockLib();

	memset(m_pszIpAddr, NULL, MAX_PATH);
}

CNetCommBase::~CNetCommBase()
{
	WSACleanup();
}

// Initialize parameters.
bool CNetCommBase::InitSockLib()
{
	WSADATA wsaData;
	memset(&wsaData, 0, sizeof(WSADATA));

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		TRACE("Winsock2 library start up failed. GetLastError() return: %d", WSAGetLastError());
		WSACleanup();
		return false;
	}

	return true;
}

// Initialize the ip address and port.
void CNetCommBase::InitIpAddr_Port(const char* pszIpAddr, int nPort)
{
	ASSERT(pszIpAddr != NULL);

	strcpy_s(m_pszIpAddr, MAX_PATH, pszIpAddr);
	m_nPort = nPort;
}