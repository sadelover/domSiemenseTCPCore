#pragma once
#include "StdAfx.h"
#include "Protocol104.h"
#include "atlstr.h"

#include "Tools/Maths/MathCalcs.h"
#include "Tools/EngineInfoDefine.h"
const int default_portnumber = 2404;

CProtocol104Ctrl::CProtocol104Ctrl(void)
	:m_host("")
	,m_port(default_portnumber)
	,m_hsendthread(NULL)
	,m_nSendReadCmdTimeInterval(30)
	,m_hDataCheckRetryConnectionThread(NULL)
	,m_nFCBusNetworkErrorCount(0)
	,m_nSendInterval(10)
	,m_bExitThread(false)
	,m_IRPacketNum(0)
	,m_ISPacketNum(0)
{

	m_IPacket[0]=(BYTE)0x68;
	//APDU长度
	m_IPacket[1]=(BYTE)0x0E;
	//以下是控制组1，2，3，4
	m_IPacket[2]=(BYTE)((LOWORD(m_ISPacketNum))*2);	//发送序列
	m_IPacket[3]=(BYTE)(HIWORD(m_ISPacketNum));
	m_IPacket[4]=(BYTE)((LOWORD(m_IRPacketNum))*2);  //接收序列
	m_IPacket[5]=(BYTE)(HIWORD(m_IRPacketNum));
	//ASDU类型标识，16进制0d,带品质描述的浮点值，每个遥测值占5个字节
	m_IPacket[6]=(BYTE)0x64;
	//信息体个数(可变结构限定词)
	m_IPacket[7]=(BYTE)0x01;
	//传输原因
	m_IPacket[8]=(BYTE)0x06;
	m_IPacket[9]=(BYTE)0x00;
	//ASDU公共地址
	m_IPacket[10]=(BYTE)0x01;
	m_IPacket[11]=(BYTE)0x00;

	m_IPacket[12]=(BYTE)0x00;
	m_IPacket[13]=(BYTE)0x00;
	m_IPacket[14]=(BYTE)0x00;
	m_IPacket[15]=(BYTE)0x14;

	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_strErrInfo = "";
	m_nEngineIndex = 0;
}

CProtocol104Ctrl::~CProtocol104Ctrl(void)
{
	if (m_hsendthread != NULL)
	{
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	if (m_hDataCheckRetryConnectionThread != NULL)
	{
		::CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}
}

bool CProtocol104Ctrl::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(length>=10)
	{
		BYTE *data=(BYTE*)pRevData;
		GetFrameFromPackage(data,length);
		m_oleUpdateTime = COleDateTime::GetCurrentTime();
	}

	return 0;
}

void CProtocol104Ctrl::SetHost( const string& strhost )
{
	m_host = strhost;
}

void CProtocol104Ctrl::SetPort( u_short portnumer )
{
	m_port = portnumer;
}

void CProtocol104Ctrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CProtocol104Ctrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const wstring& pointname = m_pointlist[i].GetShortName();
		wstring strValue = Project::Tools::RemoveDecimalW(m_pointlist[i].GetValue());
		valuelist.push_back(make_pair(pointname, strValue));		
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);
		valuelist.push_back(make_pair(TIME_UPDATE_M104, strUpdateTime));
		//valuelist.push_back(make_pair(ERROR_M104, Project::Tools::AnsiToWideChar(m_strErrInfo.c_str())));
	}
}

bool CProtocol104Ctrl::Init()
{
	if (m_pointlist.empty())
	{
		return false;
	}

	wstring strInterval = m_pointlist[0].GetParam(5);
	m_nSendInterval = _wtoi(strInterval.c_str());

	m_ISPacketNum = 0;
	m_IRPacketNum = 0;

	bool bConnectOK = false;
	if (TcpIpConnectComm(m_host, m_port))
	{
		bConnectOK = true;
	}
	
	SendPack_TypeID64H();
	
	if(m_nSendInterval>0)
		m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	m_strErrInfo = "";
	return bConnectOK;
}

bool CProtocol104Ctrl::Exit()
{
	m_bExitThread = true;
	Disconnect();		//关闭TCP
	if(m_hsendthread)
		WaitForSingleObject(m_hsendthread, INFINITE);
	if(m_hDataCheckRetryConnectionThread)
		WaitForSingleObject(m_hDataCheckRetryConnectionThread,INFINITE);
	return true;
}

double CProtocol104Ctrl::GetMultiple( wstring strMultiple )
{
	if (strMultiple.empty()){
		return 1;
	}
	else
		return _ttof(strMultiple.c_str());
}

void CProtocol104Ctrl::UnPack_Start68H( BYTE *msgbuf, int len )
{
	BYTE packetType = *(msgbuf+6);
	switch( packetType )
	{
	case 0x64:								//总召唤   
		UnPack_TypeID64H(msgbuf,len);   
		break;   

	case 0x67:   
		UnPack_TypeID67H(msgbuf,len);     //主站对时   
		break;   

	case 0x2d:       
		UnPack_TypeID2DH(msgbuf,len);     //单点遥控   
		break;   

	case 0x2e:   
		UnPack_TypeID2EH(msgbuf,len);     //双点遥控   
		break;   

	case 0x65:                      
		UnPack_TypeID65H(msgbuf,len);    //二进制计数器读数(BCR) -- 电度累计量   
		break;   

	case 0x01:
		UnPack_TypeID01H(msgbuf,len);    //（单点YX报文）
		break;

	case  0x0D:
		UnPack_TypeID0DH(msgbuf,len);     //段浮点遥测
		break;

	default:   
		UnPack_Start68H_Unknow(msgbuf,len);   
		break;  
	}
}

void CProtocol104Ctrl::UnPack_TypeID64H( BYTE *msgbuf,int len )
{
	short nType = *(msgbuf+6);
	short nLength = *(msgbuf+1);
	if(nType==100)  //0x16
	{

	}
}

void CProtocol104Ctrl::UnPack_TypeID67H( BYTE *msgbuf,int len )
{
	BYTE nYear, nMonth, nDay, nHour, nMin; //nSec, nSec_m   
	WORD wSec;   
	float fSec;   

	nYear  = msgbuf[msgbuf[7]+9];   
	nMonth = msgbuf[msgbuf[7]+8];   
	nDay   = msgbuf[msgbuf[7]+7] & 0x1f;   
	nHour  = msgbuf[msgbuf[7]];   
	nMin   = msgbuf[msgbuf[7]-7];   
	wSec   = (msgbuf[msgbuf[7]-8]<<8) | msgbuf[msgbuf[7]-9];   
	fSec   = (float)wSec/1000;   

	DWORD dwBytesSended = 0;   
	CString s1 = _T("");   
	if (nYear < 0x0a)   
		s1.Format(_T("Syn Time 200%d-%d-%d %d:%d:%2.3f"),nYear,nMonth,nDay,nHour,nMin,fSec);//%2.3f 保留三位小数点   
	else   
		s1.Format(_T("Syn Time 20%d-%d-%d %d:%d:%2.3f"),nYear,nMonth,nDay,nHour,nMin,fSec);   
	TRACE(s1);
}

void CProtocol104Ctrl::UnPack_TypeID2DH( BYTE *msgbuf,int len )
{
	CString s1 = _T("");   

	if ((msgbuf[msgbuf[7]+9] & 0x80) == 0x80)   //遥控选择   
		s1 = _T("<Rec>: 101 Main--Single Point Command:Remote Selection");    
	else   
		s1 = _T("<Rec>: 101 Main--Single Point Command:Remote Execution");    

	if ((msgbuf[msgbuf[7]+9] & 0x01) == 0x01)    //合闸   
		s1 = s1 + _T("(Colse)");   
	else   
		s1 = s1 + _T("(Open)");  
	TRACE(s1);
}

void CProtocol104Ctrl::UnPack_TypeID2EH( BYTE *msgbuf,int len )
{ 
	CString s1 = _T("");   

	if ((msgbuf[msgbuf[7]+9] & 0x80) == 0x80)   //遥控选择   
		s1 = _T("<Rec>: 101 Main--Two Point Command:Remote Selection");    
	else   
		s1 = _T("<Rec>: 101 Main--Two Point Command:Remote Execution");    

	if ((msgbuf[msgbuf[7]+9] & 0x03) == 0x02)         //合闸   
		s1 = s1 + _T("(Colse)");   
	else if ((msgbuf[msgbuf[7]+9] & 0x03) == 0x01)    //分闸   
		s1 = s1 + _T("(Open)");   
	else   
	{   
		s1 = s1 + _T("(Allowed)");       
		return;   
	}  
	TRACE(s1);
}

void CProtocol104Ctrl::UnPack_TypeID65H( BYTE *msgbuf,int len )
{
	CString s1 = _T("");  
	CString s2 = _T("");   
	if ((msgbuf[msgbuf[7]+9] & 0x07) < 0x04)   //计数量召唤命令限定词(QCC)->RQT  仅取低3位   
		s1.Format(_T("QCC：%d"),msgbuf[msgbuf[7]+9] & 0x07);   
	else if((msgbuf[msgbuf[7]+9] & 0x07) == 0x05)   
		s1 = _T("QCC：Total Request");   
	else   
		s1 = _T("QCC：Unknown Command (Reserved)");   

	switch (msgbuf[msgbuf[7]+9] & 0xc0)        //计数量召唤命令限定词(QCC)->FRZ bit7~8   
	{   
	case 0x00:   
		s2 =  _T("(Read Command)");   
		break;   

	case 0x40:   
		s2 =  _T("(Freeze Without Reset)");   
		break;   

	case 0x80:   
		s2 =  _T("(Freeze With Reset)");   
		break;   

	case 0xc0:   
		s2 =  _T("(Reset Command)");   
		break;   

	default:   
		s2 =  _T("(Read Command)");   
		break;   
	}   

	TRACE(s1);
}

void CProtocol104Ctrl::UnPack_TypeID01H( BYTE *msgbuf,int len )
{
	short nType = *(msgbuf+6);
	short nLength = *(msgbuf+1);
	if(nType==1)  //0x01
	{

	}
}

void CProtocol104Ctrl::UnPack_TypeID0DH( BYTE *msgbuf,int len )
{
	short nType = *(msgbuf+6);
	if(nType==13)  //0D
	{
		short nReason = *(msgbuf+8);
		if(nReason == 3)  //float yc
		{
			short nSize = *(msgbuf+7);
			int nNext = 0;
			for(int i=0; i<nSize; i++)
			{
				int DotNuma = *(msgbuf+13+nNext)*256 +*(msgbuf+12+nNext);
				float* nValue = (float *)(msgbuf+15+nNext);

				DataPointEntry* entry = FindEntry(PROTOCOL_TYPEID0DH,DotNuma);
				if (entry){
					Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
					entry->SetValue(*nValue/GetMultiple(entry->GetParam(6)));
				}
				nNext = (i+1)*8;
			}
		}
		else if(nReason == 20)  //总召
		{
			short nLength = *(msgbuf+1);
			//63+2-15
			short nSize = (nLength - 13)/5;
			int nNext = 0;
			int DotNumaBase = *(msgbuf+13)*256 +*(msgbuf+12);
			for(int i=0; i<nSize; i++)
			{
				int DotNuma = DotNumaBase+i;
				float* nValue = (float *)(msgbuf+15+nNext);

				DataPointEntry* entry = FindEntry(PROTOCOL_TYPEID0DH,DotNuma);
				if (entry){
					Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
					entry->SetValue(*nValue/GetMultiple(entry->GetParam(6)));
				}
				nNext = (i+1)*5;
			}
		}
	}	
}

void CProtocol104Ctrl::UnPack_Start68H_Unknow( BYTE *msgbuf,int len )
{

}

void CProtocol104Ctrl::GetFrameFromPackage( BYTE *msgbuf, int len )
{
	m_IRPacketNum++;

	short nType = *(msgbuf+6);
	short nLength = *(msgbuf+1);
	int nFlag = 0;
	while(nLength+2<=len&&nFlag<len)
	{
		BYTE* pBuf = msgbuf+nFlag;
		nLength = *(pBuf+1);	
		UnPack_Start68H(pBuf,nLength+2);
		nFlag = nFlag+nLength+2;
	}
}

DataPointEntry* CProtocol104Ctrl::FindEntry( int nFrameType, int nPointID )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];

		if ((_wtoi(entry.GetParam(3).c_str()) == nFrameType) && (_wtoi(entry.GetParam(4).c_str()) == nPointID)){
			return &m_pointlist[i];
		}
	}
	return NULL;
}

void CProtocol104Ctrl::SendPack_TypeID64H()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);
	if(m_sockConnected==NULL)
		return;

	if(!m_bConnectOK)
		return;

	// 修改 接受 发送序列
	m_IPacket[2]=(BYTE)((LOWORD(m_ISPacketNum))*2);	//发送序列
	m_IPacket[3]=(BYTE)(HIWORD(m_ISPacketNum));
	m_IPacket[4]=(BYTE)((LOWORD(m_IRPacketNum))*2);  //接收序列
	m_IPacket[5]=(BYTE)(HIWORD(m_IRPacketNum));

	char* pSend = (char*)m_IPacket;
	if(SendPackage(pSend, sizeof(m_IPacket)))
	{
		m_ISPacketNum = m_ISPacketNum+1;
	}
	else
	{
		SetNetworkError();
	}
}

UINT WINAPI CProtocol104Ctrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CProtocol104Ctrl* pthis = (CProtocol104Ctrl*)lparam;
	if (!pthis){
		return 0;
	}
	int nInterval = pthis->GetSendInterval();
	while (!pthis->m_bExitThread)
	{
		if(pthis->m_bExitThread)
			break;

		int nSleep = nInterval;
		while(!pthis->m_bExitThread)
		{
			if(nSleep <= 0)
			{
				break;
			}
			nSleep--;
			Sleep(1000);
		}

		pthis->SendPack_TypeID64H();
	}

	return 0;
}

UINT WINAPI CProtocol104Ctrl::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CProtocol104Ctrl* pthis = (CProtocol104Ctrl*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(pthis->m_nFCBusNetworkErrorCount>=5 || !pthis->m_bConnectOK)
			{
				if(pthis->ReConnect())
					pthis->ClearNetworkError();
			}
			Sleep(5000);
		}
	}

	return 0;
}

int CProtocol104Ctrl::GetSendInterval()
{
	return m_nSendInterval;
}

void    CProtocol104Ctrl::SetNetworkError()
{
	if(m_nFCBusNetworkErrorCount<10000)
		m_nFCBusNetworkErrorCount++;
}

void    CProtocol104Ctrl::ClearNetworkError()
{
	m_nFCBusNetworkErrorCount = 0;
}

bool CProtocol104Ctrl::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	_tprintf(L"INFO : Protocol104 reconnecting...\r\n");
	Disconnect(); //tcpip recv thread stop, and clear socket

	if (!TcpIpConnectComm(m_host, m_port))
	{
		_tprintf(L"ERROR: Protocol104 reconnecting failed!\r\n");
		return false;
	}
	m_strErrInfo = "";
	_tprintf(L"INFO : Protocol104 reconnecting success!\r\n");
	return true;
}
