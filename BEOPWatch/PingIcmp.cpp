// PingIcmp.cpp: implementation of the CPingIcmp class.

#include "stdafx.h"
#include "PingIcmp.h"


// Default constructor.
CPingIcmp::CPingIcmp() 
: m_sockPingIcmp(NULL)
{
}

// Default destructor.
CPingIcmp::~CPingIcmp()
{
	if (m_sockPingIcmp != NULL)
	{
		closesocket(m_sockPingIcmp);
		m_sockPingIcmp = NULL;
	}
}

// Ping the assign host.
bool CPingIcmp::Ping(const string& strIpAddr, unsigned int nPackageSiz /*= 0*/)
{
	if (m_sockPingIcmp == NULL)
	{
		WSADATA wsaData;
		// Initialize the winsock2.
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			TRACE("WSAStartup failed: %d.  \r\n", GetLastError());
			return false;
		}

		// Create the socket.
		// Note: the user must set the lash parameter dwFlags as WSA_FLAG_OVERLAPPED
		//       for the response of recvfrom() function, when the recvfrom() function timeout;
		//       otherwise, the recvfrom() function will be block.
		m_sockPingIcmp = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_sockPingIcmp == INVALID_SOCKET) 
		{
			TRACE("WSASocket() failed: %d.  \r\n", WSAGetLastError());
			return false;
		}
	}

	// Get the host ip address by the local host name.
	struct sockaddr_in dest;
	unsigned int addr = 0;
	memset(&dest ,0, sizeof(dest));
	struct hostent *hp = gethostbyname(strIpAddr.c_str());
	if (!hp)
	{
		addr = inet_addr(strIpAddr.c_str());
	}
	if ((!hp) && (addr == INADDR_NONE)) 
	{
		TRACE("Unable to resolve %s.  \r\n", strIpAddr.c_str());
		return false;
	}

	if (hp != NULL)
	{
		memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
	}
	else
	{
		dest.sin_addr.s_addr = addr;
	}

	if (hp)
	{
		dest.sin_family = hp->h_addrtype;
	}
	else
	{
		dest.sin_family = AF_INET;
	}

	char *dest_ip = inet_ntoa(dest.sin_addr);

	int datasize;
	if (nPackageSiz == 0) 
	{
		datasize = DEF_PACKET_SIZE;
	}
	else
	{
		datasize = nPackageSiz;
	}
	datasize += sizeof(IcmpHeader);

	// Allocate the buffer.
	char *icmp_data = reinterpret_cast<char*>(xmalloc(MAX_PACKET));
	char *recvbuf = reinterpret_cast<char*>(xmalloc(MAX_PACKET));
	if (!icmp_data || !recvbuf) 
	{
		TRACE("HeapAlloc failed %d.  \r\n", GetLastError());
		return false;
	}

	// Fill the icmp data.
	memset(icmp_data, 0, MAX_PACKET);
	FillIcmpData(icmp_data, datasize);

	// Set the socket receive timeout.
	int nRecvTimeoOut = 1000;
	int bread = setsockopt(m_sockPingIcmp, SOL_SOCKET, SO_RCVTIMEO,(char*)&nRecvTimeoOut, sizeof(nRecvTimeoOut));
	if (bread == SOCKET_ERROR) 
	{
		TRACE("failed to set recv timeout: %d.  \r\n", WSAGetLastError());
		return false;
	}

	USHORT seq_no = 0;
	bool bResult = false;
	int repeattime = 2;
	while(repeattime-- > 0) 
	{
		int bwrote;
		// Write the tick count into the icmp package.
		((IcmpHeader*)icmp_data)->i_cksum = 0;
		((IcmpHeader*)icmp_data)->timestamp = GetTickCount();

		// Write the checksum into the icmp package.
		((IcmpHeader*)icmp_data)->i_seq = seq_no++;
		((IcmpHeader*)icmp_data)->i_cksum = CheckSum((USHORT*)icmp_data, datasize);

		// 发送icmp包
		bwrote = sendto(m_sockPingIcmp, icmp_data, datasize, 0, (struct sockaddr*)&dest, sizeof(dest));
		if (bwrote == SOCKET_ERROR){
			//发送超时的话重新发送
			if (WSAGetLastError() == WSAETIMEDOUT) {
				Sleep(200);
				continue;  // again
			}

			TRACE("sendto failed: %d.  \r\n", WSAGetLastError());
			bResult = false;
			break;
		}
		
		// Receive the icmp package.
		int bread = recvfrom(m_sockPingIcmp, recvbuf, MAX_PACKET, 0, NULL, NULL);
		if (bread == SOCKET_ERROR){
			//接收超时
			if (WSAGetLastError() == WSAETIMEDOUT){
				Sleep(200);
				continue;  // again
			}
			
			// Receive error.
			TRACE("recvfrom failed: %d.  \r\n", WSAGetLastError());
			bResult = false;
			break;
		}

		// Decode the response datas.
		if (!DecodeResp(recvbuf, bread)){
			// Receive error.
			TRACE("recvfrom failed: %d.  \r\n", WSAGetLastError());
			bResult = false;
			break;
		}

		// Receive and ping success.
		bResult = true;
		break;
	}

	// Free the allocated buffer.
	xfree(icmp_data);
	xfree(recvbuf);

	return bResult;
}

// The response is an IP packet. 
// We must decode the IP header to locate the ICMP data.
void CPingIcmp::DecodeResp(char *buf, int bytes,struct sockaddr_in *from) 
{
	IpHeader *iphdr;
	IcmpHeader *icmphdr;
	unsigned short iphdrlen;

	iphdr = (IpHeader *)buf;

	iphdrlen = iphdr->h_len * 4 ;  // number of 32-bit words *4 = bytes

	if (bytes < iphdrlen + ICMP_MIN) 
	{
		printf("Too few bytes from %s.  \r\n", inet_ntoa(from->sin_addr));
	}

	icmphdr = (IcmpHeader*)(buf + iphdrlen);

	if (icmphdr->i_type != ICMP_ECHOREPLY) 
	{
		fprintf(stderr, "non-echo type %d recvd.  \r\n", icmphdr->i_type);
		return ;
	}
	if (icmphdr->i_id != (USHORT)GetCurrentProcessId()) 
	{
		fprintf(stderr, "someone else's packet!  \r\n");
		return ;
	}

	printf("%d bytes from %s:  \r\n", bytes, inet_ntoa(from->sin_addr));
	printf(" icmp_seq = %d.  \r\n", icmphdr->i_seq);
	printf(" time: %d ms.  \r\n", GetTickCount()-icmphdr->timestamp);
}

bool CPingIcmp::DecodeResp(char *buf, int bytes)
{
	IpHeader *iphdr;
	IcmpHeader *icmphdr;
	unsigned short iphdrlen;
	CToolsFunction toolsFunction;

	iphdr = (IpHeader *)buf;

	iphdrlen = iphdr->h_len * 4 ;  // number of 32-bit words *4 = bytes

	if (bytes < iphdrlen + ICMP_MIN) 
	{
		static bool bOnly = true;
		if (bOnly)
		{
			toolsFunction.SetLogFilePath(TEXT("LogFile.dat"));
			toolsFunction.AppendLog2File("Received too few bytes in icmp ping.  \r\n");
			bOnly = false;
		}
		TRACE("Received too few bytes in icmp ping.  \r\n");
		return false;
	}

	icmphdr = (IcmpHeader*)(buf + iphdrlen);
	if (icmphdr->i_type != ICMP_ECHOREPLY) 
	{
		static bool bOnly = true;
		if (bOnly)
		{
			char szFormat[MAX_PATH] = {0};
			sprintf_s(szFormat, 
				MAX_PATH, 
				"non-echo type %d recvd. GetLastError() return %d.  \r\n", 
				icmphdr->i_type,
				GetLastError());
			toolsFunction.SetLogFilePath(TEXT("LogFile.dat"));
			toolsFunction.AppendLog2File(szFormat);
			bOnly = false;
		}
		TRACE("non-echo type %d recvd.  \r\n", icmphdr->i_type);
		return false;
	}
// 	if (icmphdr->i_id != (USHORT)GetCurrentProcessId()) 
// 	{
// 		TRACE("someone else's packet!  \r\n");
// 		return false;
// 	}

	TRACE("Icmp ping success.  \r\n");
	return true;
}

// Check the ICMP package sum.
USHORT CPingIcmp::CheckSum(USHORT *buffer, int size) 
{
// 校验和的计算:  
// 以16位的字为单位将缓冲区的内容相加,如果缓冲区长度为奇数,  
// 则再加上一个字节.它们的和存入一个32位的双字中.

	unsigned long cksum = 0;

	while(size > 1) 
	{
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}

	// 如果是奇数 
	if (size) 
	{
		cksum += *(UCHAR*)buffer;
	}

	// 将32位的cksum高16位和低16位相加，然后取反
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (USHORT)(~cksum);
}

// Helper function to fill in various stuff in our ICMP request.
void CPingIcmp::FillIcmpData(char * icmp_data, int datasize)
{
	IcmpHeader *icmp_hdr;
	char *datapart;

	icmp_hdr = (IcmpHeader*)icmp_data;

	// Fill the initial icmp header.
	icmp_hdr->i_type = ICMP_ECHO;
	icmp_hdr->i_code = 0;
	icmp_hdr->i_id = (USHORT)GetCurrentProcessId();
	icmp_hdr->i_cksum = 0;
	icmp_hdr->i_seq = 0;

	datapart = icmp_data + sizeof(IcmpHeader);

	// Place some junk in the buffer.
	memset(datapart, 'E', datasize - sizeof(IcmpHeader));
} 

// Usage information.
void CPingIcmp::Usage(char *progname)
{
	fprintf(stderr, "Usage:  \r\n");
	fprintf(stderr, "%s [data_size].  \r\n",progname);
	fprintf(stderr, "datasize can be up to 1Kb.  \r\n");

	return ;
}

bool CPingIcmp::PingServer(const string& strIpAddr, unsigned int nPackageSiz /*= 0*/ )
{
	if (m_sockPingIcmp == NULL)
	{
		WSADATA wsaData;
		// Initialize the winsock2.
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			TRACE("WSAStartup failed: %d.  \r\n", GetLastError());
			return true;
		}

		// Create the socket.
		// Note: the user must set the lash parameter dwFlags as WSA_FLAG_OVERLAPPED
		//       for the response of recvfrom() function, when the recvfrom() function timeout;
		//       otherwise, the recvfrom() function will be block.
		m_sockPingIcmp = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_sockPingIcmp == INVALID_SOCKET) 
		{
			TRACE("WSASocket() failed: %d.  \r\n", WSAGetLastError());
			return true;					//创建不了也算成功 特殊
		}
	}

	// Get the host ip address by the local host name.
	struct sockaddr_in dest;
	unsigned int addr = 0;
	memset(&dest ,0, sizeof(dest));
	struct hostent *hp = gethostbyname(strIpAddr.c_str());
	if (!hp)
	{
		addr = inet_addr(strIpAddr.c_str());
	}
	if ((!hp) && (addr == INADDR_NONE)) 
	{
		TRACE("Unable to resolve %s.  \r\n", strIpAddr.c_str());
		return false;
	}

	if (hp != NULL)
	{
		memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
	}
	else
	{
		dest.sin_addr.s_addr = addr;
	}

	if (hp)
	{
		dest.sin_family = hp->h_addrtype;
	}
	else
	{
		dest.sin_family = AF_INET;
	}

	char *dest_ip = inet_ntoa(dest.sin_addr);

	int datasize;
	if (nPackageSiz == 0) 
	{
		datasize = DEF_PACKET_SIZE;
	}
	else
	{
		datasize = nPackageSiz;
	}
	datasize += sizeof(IcmpHeader);

	// Allocate the buffer.
	char *icmp_data = reinterpret_cast<char*>(xmalloc(MAX_PACKET));
	char *recvbuf = reinterpret_cast<char*>(xmalloc(MAX_PACKET));
	if (!icmp_data || !recvbuf) 
	{
		TRACE("HeapAlloc failed %d.  \r\n", GetLastError());
		return false;
	}

	// Fill the icmp data.
	memset(icmp_data, 0, MAX_PACKET);
	FillIcmpData(icmp_data, datasize);

	// Set the socket receive timeout.
	int nRecvTimeoOut = 1000;
	int bread = setsockopt(m_sockPingIcmp, SOL_SOCKET, SO_RCVTIMEO,(char*)&nRecvTimeoOut, sizeof(nRecvTimeoOut));
	if (bread == SOCKET_ERROR) 
	{
		TRACE("failed to set recv timeout: %d.  \r\n", WSAGetLastError());
		return false;
	}

	USHORT seq_no = 0;
	bool bResult = false;
	int repeattime = 2;
	while(repeattime-- > 0) 
	{
		int bwrote;
		// Write the tick count into the icmp package.
		((IcmpHeader*)icmp_data)->i_cksum = 0;
		((IcmpHeader*)icmp_data)->timestamp = GetTickCount();

		// Write the checksum into the icmp package.
		((IcmpHeader*)icmp_data)->i_seq = seq_no++;
		((IcmpHeader*)icmp_data)->i_cksum = CheckSum((USHORT*)icmp_data, datasize);

		// 发送icmp包
		bwrote = sendto(m_sockPingIcmp, icmp_data, datasize, 0, (struct sockaddr*)&dest, sizeof(dest));
		if (bwrote == SOCKET_ERROR){
			//发送超时的话重新发送
			if (WSAGetLastError() == WSAETIMEDOUT) {
				Sleep(200);
				continue;  // again
			}

			TRACE("sendto failed: %d.  \r\n", WSAGetLastError());
			bResult = false;
			break;
		}

		// Receive the icmp package.
		int bread = recvfrom(m_sockPingIcmp, recvbuf, MAX_PACKET, 0, NULL, NULL);
		if (bread == SOCKET_ERROR){
			//接收超时
			if (WSAGetLastError() == WSAETIMEDOUT){
				Sleep(200);
				continue;  // again
			}

			// Receive error.
			TRACE("recvfrom failed: %d.  \r\n", WSAGetLastError());
			bResult = false;
			break;
		}

		// Decode the response datas.
		if (!DecodeResp(recvbuf, bread)){
			// Receive error.
			TRACE("recvfrom failed: %d.  \r\n", WSAGetLastError());
			bResult = false;
			break;
		}

		// Receive and ping success.
		bResult = true;
		break;
	}

	// Free the allocated buffer.
	xfree(icmp_data);
	xfree(recvbuf);

	return bResult;
}
