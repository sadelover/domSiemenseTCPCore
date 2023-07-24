#ifndef __NET_COMM_H__
#define __NET_COMM_H__

#include "NetCommBase.h"
#include "../ToolsFunction/ToolsFunction.h"


class CNetComm : public CNetCommBase
{
public:
	CNetComm();          		// default constructor
	virtual ~CNetComm();  		// default destructor

public:
	virtual SOCKET CreateUdpSock();                                     // create the udp socket
	virtual SOCKET CreateUdpBroadcastSock(__in const char* pszIpAddr);  // create the udp broadcast socket

	virtual void SendUdpPackage(__in SOCKET sock, __in const char* pszBuffer);  // send the udp package
	virtual int RecvUdpPackage(__in SOCKET sock, __out char* pszBuffer, __in int nMaxBuffer);  // receive the udp package

	virtual void SendUdpBroadcastPackage(__in SOCKET sock, __in const char* pszBuffer);

private:
	void InitParams();			// initialize parameters

private:
	CToolsFunction m_toolsFunction;

	SOCKET m_sockUdp;			// the udp socket handle
	SOCKET m_sockUdpBroadcast;  // the udp broadcast socket handle
};

#endif  // __NET_COMM_H__