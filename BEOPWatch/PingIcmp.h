/******************************************************************************\
* ping.c - Simple ping utility using SOCK_RAW
*
* This is a part of the Microsoft Source Code Samples.
* Copyright 1996-1997 Microsoft Corporation.
* All rights reserved.
* This source code is only intended as a supplement to
* Microsoft Development Tools and/or WinHelp documentation.
* See these sources for detailed information regarding the
* Microsoft samples programs.
\******************************************************************************/

#ifndef __PING_ICMP_H__
#define __PING_ICMP_H__


#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

#include "ToolsFunction.h"

#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0

#define ICMP_MIN 8  // minimum 8 byte icmp packet (just header)

// The IP header.
#pragma pack(push, 1)
typedef struct tagIpHdr 
{
	unsigned int h_len:4;           // length of the header
	unsigned int version:4;         // Version of IP
	unsigned char tos;              // Type of service
	unsigned short total_len;       // total length of the packet
	unsigned short ident;           // unique identifier
	unsigned short frag_and_flags;  // flags
	unsigned char ttl;
	unsigned char proto;            // protocol (TCP, UDP etc)
	unsigned short checksum;        // IP checksum

	unsigned int sourceIP;
	unsigned int destIP;

	tagIpHdr()
	{
		memset(this, 0, sizeof(*this));
	}

}IpHeader, *pIpHeader;

// ICMP header.
typedef struct tagIcmpHdr 
{
	BYTE i_type;
	BYTE i_code; /* type sub code */
	USHORT i_cksum;
	USHORT i_id;
	USHORT i_seq;
	/* This is not the std header, but we reserve space for time */
	ULONG timestamp;

	tagIcmpHdr()
	{
		memset(this, 0, sizeof(*this));
	}

}IcmpHeader, *pIcmpHeader;

#pragma pack(pop)

#define DEF_PACKET_SIZE  32
#define MAX_PACKET       1024

#define xmalloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#define xfree(p) HeapFree (GetProcessHeap(),0,(p))


class _declspec(dllexport) CPingIcmp
{
public:
	CPingIcmp();                                        // default constructor
	virtual ~CPingIcmp();                               // default destructor
	virtual bool Ping(__in const string& strIpAddr, __in unsigned int nPackageSize = 0);  // ping the assign host
	bool PingServer(__in const string& strIpAddr, __in unsigned int nPackageSize = 0);  // ping the assign host

private:
	void DecodeResp(__in char *, __in int , __in struct sockaddr_in *);  // decode the ip header to locate the icmp data
	bool DecodeResp(__in char *, __in int);                      

	void FillIcmpData(__in char *, __in int);           // helper function to fill in various stuff in our icmp request
	USHORT CheckSum(__in USHORT *, __in int);           // check the icmp package sum

	inline void Usage(__in char *progname);             // usage information

private:
	SOCKET m_sockPingIcmp;                              // the ping icmp socket
};


#endif  // __PING_ICMP_H__