// NetComm.h implement

#include "stdafx.h"
#include "NetComm.h"


CNetComm::CNetComm()
{
	InitParams();
}

CNetComm::~CNetComm()
{
	if (m_sockUdp != NULL)
	{
		closesocket(m_sockUdp);
		m_sockUdp = NULL;
	}
	if (m_sockUdpBroadcast != NULL)
	{
		closesocket(m_sockUdpBroadcast);
		m_sockUdpBroadcast = NULL;
	}
}

// Initialize parameters.
void CNetComm::InitParams()
{
	m_sockUdp = NULL;
	m_sockUdpBroadcast = NULL;
}

// Create the udp socket.
SOCKET CNetComm::CreateUdpSock()
{
	ASSERT(strlen(m_pszIpAddr) != 0 || m_nPort != 0);

	if (m_sockUdp == NULL)
	{
		// Create the udp socket.
		m_sockUdp = socket(AF_INET, SOCK_DGRAM, 0);

		// Config the  bind struct SOCKADDR_IN.
		SOCKADDR_IN sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(m_nPort);
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);  
		  
		if (bind(m_sockUdp, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != 0)    
		{    
			// Bind failed.
			m_toolsFunction.AppendLog2File("Can't bind socket to local port! Program stop.  \r\n");
			closesocket(m_sockUdp);
			m_sockUdp = NULL;    
		}

		// Set the bind socket address reuseable.
		BOOL bReuseAddr = TRUE;
		setsockopt(m_sockUdp, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuseAddr, sizeof(bReuseAddr));
	}

	return m_sockUdp;
}

// Create the udp broadcast socket.
SOCKET CNetComm::CreateUdpBroadcastSock(const char* pszIpAddr)
{
	ASSERT(pszIpAddr != NULL);

	if (m_sockUdpBroadcast == NULL)
	{
		// Create socket.
		m_sockUdpBroadcast = socket(AF_INET, SOCK_DGRAM, 0);

		// Set socket broadcast option.
		BOOL bBroadcast = TRUE; 
		setsockopt(m_sockUdpBroadcast, SOL_SOCKET, SO_BROADCAST, (char *)&bBroadcast, sizeof(BOOL)); 

		// Config the bind struct SOCKADDR_IN.
		SOCKADDR_IN sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(0); 
		if (strcmp(pszIpAddr, "") == 0)
		{
			// Bind socket with default ethernet card.
			sockAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
		}
		else 
		{
			// Bind socket with assigned ethernet card.
			sockAddr.sin_addr.s_addr = inet_addr(pszIpAddr);
		}

		if (bind(m_sockUdpBroadcast, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != 0)    
		{    
			// Bind failed.
			m_toolsFunction.AppendLog2File("Can't bind socket to local port! Program stop.  \r\n");
			closesocket(m_sockUdpBroadcast);
			m_sockUdpBroadcast = NULL;    
		}
	}

	return m_sockUdpBroadcast;
}

// Send the udp package.
void CNetComm::SendUdpPackage(SOCKET sock, const char* pszBuffer)
{
 	// Config ip address and communication port.
 	SOCKADDR_IN addrSend;
 	addrSend.sin_addr.S_un.S_addr = inet_addr(m_pszIpAddr);
 	addrSend.sin_family = AF_INET;
 	addrSend.sin_port = htons(m_nPort);
 	int len = sizeof(SOCKADDR);

	// Send the udp package.
	int nSendResult = sendto(sock, pszBuffer, static_cast<int>(strlen(pszBuffer))+1, 0, (SOCKADDR*)&addrSend, static_cast<int>(sizeof(SOCKADDR_IN)));
	if (nSendResult == SOCKET_ERROR)
	{
		TRACE("Send udp package failed. WSAGetLastError() return %d.  \r\n", WSAGetLastError());
	}
}

// Receive the udp package.
int CNetComm::RecvUdpPackage(SOCKET sock, char* pszBuffer, int nMaxBuffer)
{
	char szBuff[MAX_BUFFER_SIZE] = {0};
	int nRecvSize = 0;

	if ((nRecvSize = recvfrom(sock, szBuff, nMaxBuffer, 0, NULL, NULL)) == SOCKET_ERROR)
	{
		int nError = WSAGetLastError();
		if (nError != WSAETIMEDOUT)
		{
			TRACE("Received udp package failed. WSAGetLastError() return %d.  \r\n", WSAGetLastError());
			return SOCKET_ERROR;
		}

		return WSAETIMEDOUT;
	}

	memcpy(pszBuffer, szBuff, nRecvSize);

	return nRecvSize;
}

void CNetComm::SendUdpBroadcastPackage(SOCKET sock, const char* pszBuffer)
{
	// Setting sendto struct SOCKADDR_IN.
	SOCKADDR_IN sockBroadcast;
	sockBroadcast.sin_family = AF_INET;    
	sockBroadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);  // broadcast address    
	sockBroadcast.sin_port = htons(m_nPort);                  // connect port 

	// Send the udp broadcast package.
	int nSendResult = sendto(sock, pszBuffer, static_cast<int>(strlen(pszBuffer)), 0, (SOCKADDR*)&sockBroadcast, static_cast<int>(sizeof(SOCKADDR_IN)));
	if (nSendResult == SOCKET_ERROR)
	{
		TRACE("Send datas failed. WSAGetLastError() return %d.  \r\n", WSAGetLastError());
	}
}