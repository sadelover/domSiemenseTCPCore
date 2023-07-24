#ifndef __NET_COMM_BASE_H__
#define __NET_COMM_BASE_H__



#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define CONNECT_PORT                      8303                             	  // c/s connect port
#define MAX_BUFFER_SIZE                   8192                             	  // max buffer size to transmit datas


class CNetCommBase
{
public:
	CNetCommBase();          	// default constructor
	virtual ~CNetCommBase();  	// default destructor

	void InitIpAddr_Port(__in const char* pszIpAddr, __in int nPort);       // initialize the ip address and port

	virtual SOCKET CreateUdpSock() = 0;                                	    // create the udp socket
	virtual SOCKET CreateUdpBroadcastSock(__in const char* pszIpAddr) = 0;  // create the udp broadcast socket

	virtual void SendUdpPackage(__in SOCKET sock, __in const char* pszBuffer) = 0;  // send the udp package
	virtual int RecvUdpPackage(__in SOCKET sock, __out char* pszBuffer, __in int nMaxBuffer) = 0;  // receive the udp package
	
	virtual void SendUdpBroadcastPackage(__in SOCKET sock, __in const char* pszBuffer) = 0;

private:
	bool InitSockLib();			// initialize parameters

public:
	char m_pszIpAddr[MAX_PATH];
	int m_nPort;
};

#endif  // __NET_COMM_BASE_H__