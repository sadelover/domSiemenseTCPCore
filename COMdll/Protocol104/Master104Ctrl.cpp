#pragma once
#include "StdAfx.h"
#include "Master104Ctrl.h"
#include "atlstr.h"

#include "Tools/Maths/MathCalcs.h"
#include "Tools/EngineInfoDefine.h"
//#include "Tools/vld.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../LAN_WANComm/PingIcmp.h"

const int default_portnumber = 2404;

CMaster104Tcp::CMaster104Tcp(void)
	:m_host("")
	,m_port(default_portnumber)
	,m_IRPacketNum(0)
	,m_ISPacketNum(0)
	,m_nSendInterval(0)
	,m_bExitThread(false)
	,m_hsendthread(NULL)
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
}

CMaster104Tcp::~CMaster104Tcp(void)
{
	if(m_hsendthread)
	{
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
}

void CMaster104Tcp::SetHost( const string& strhost )
{
	m_host = strhost;
}

void CMaster104Tcp::SetPort( u_short portnumer )
{
	m_port = portnumer;
}

bool CMaster104Tcp::Exit()
{
	m_bExitThread = true;
	if(m_hsendthread)
		WaitForSingleObject(m_hsendthread,INFINITE);
	StopReceiving();
	return true;
}

bool CMaster104Tcp::Init()
{
	if (m_pointlist.empty()){
		return false;
	}
	wstring strInterval = m_pointlist[0].GetParam(5);
	m_nSendInterval = _wtoi(strInterval.c_str());

	CPingIcmp ping;
	if(m_host != "localhost" && !ping.Ping(m_host))
		return false;

	if (!Connect(m_host, m_port)){
#ifdef DEBUG		
#else
		return false;
#endif // _DEBUG
	}
	m_ISPacketNum = 0;
	m_IRPacketNum = 0;
	SendPack_TypeID64H();
	StartReceiving(OnStatusChange,NULL,(DWORD)this);
 
	if(m_nSendInterval>0)
		m_hsendthread = (HANDLE)_beginthreadex(NULL, 0, DTUThreadSendFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	return true;
}

void CMaster104Tcp::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

DataPointEntry* CMaster104Tcp::FindEntry( int nFrameType, int nPointID )
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

bool CMaster104Tcp::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	if(length>=10)
	{
		BYTE *data=(BYTE*)pRevData;
		GetFrameFromPackage(data,length);
		m_oleUpdateTime = COleDateTime::GetCurrentTime();
	}
	
	return 0;
}

void CMaster104Tcp::UnPack_Start68H( BYTE *msgbuf, int len )
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

void CMaster104Tcp::UnPack_TypeID64H( BYTE *msgbuf,int len )
{
	short nType = *(msgbuf+6);
	short nLength = *(msgbuf+1);
	if(nType==100)  //0x16
	{

	}
}

void CMaster104Tcp::UnPack_TypeID67H( BYTE *msgbuf,int len )
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

void CMaster104Tcp::UnPack_TypeID2DH( BYTE *msgbuf,int len )
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

void CMaster104Tcp::UnPack_TypeID2EH( BYTE *msgbuf,int len )
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

void CMaster104Tcp::UnPack_TypeID65H( BYTE *msgbuf,int len )
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

void CMaster104Tcp::UnPack_TypeID01H( BYTE *msgbuf,int len )
{
	short nType = *(msgbuf+6);
	short nLength = *(msgbuf+1);
	if(nType==1)  //0x01
	{

	}
}

void CMaster104Tcp::UnPack_TypeID0DH( BYTE *msgbuf,int len )
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

void CMaster104Tcp::UnPack_Start68H_Unknow( BYTE *msgbuf,int len )
{
	
}

void CMaster104Tcp::AddLog( const wchar_t* loginfo )
{
	if (m_logsession){
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

void CMaster104Tcp::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

void CMaster104Tcp::EnableLog( BOOL bEnable )
{

}

LRESULT CMaster104Tcp::OnStatusChange(TCP_CONNECT_STATE type, char *data,int length,DWORD userdata )
{
	CMaster104Tcp *pTcpCtrl=(CMaster104Tcp *)userdata;
	//断开重连
	pTcpCtrl->Reconnect();
	return 0;
}

bool CMaster104Tcp::Reconnect()
{
	Disconnect();
	if(m_hsendthread != NULL)
	{
		CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	if(!Init())
		return false;
	return true;
}

void CMaster104Tcp::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
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

void CMaster104Tcp::SendPack_TypeID64H()
{
	// 修改 接受 发送序列
	m_IPacket[2]=(BYTE)((LOWORD(m_ISPacketNum))*2);	//发送序列
	m_IPacket[3]=(BYTE)(HIWORD(m_ISPacketNum));
	m_IPacket[4]=(BYTE)((LOWORD(m_IRPacketNum))*2);  //接收序列
	m_IPacket[5]=(BYTE)(HIWORD(m_IRPacketNum));
	
	char* pSend = (char*)m_IPacket;
	int nRet = SendClient(pSend,16);
	if(nRet>=1)
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("send 64h fail");
		m_ISPacketNum = m_ISPacketNum+1;
	}

}

UINT WINAPI CMaster104Tcp::DTUThreadSendFunc( LPVOID lparam )
{
	CMaster104Tcp* pthis = (CMaster104Tcp*)lparam;
	if (!pthis){
		return 0;
	}
	int nInterval = pthis->GetSendInterval();
	while (true)
	{
		if(pthis->m_bExitThread)
			break;

		Sleep(nInterval*1000);
		pthis->SendPack_TypeID64H();
	}

	return 0;
}

int CMaster104Tcp::GetSendInterval()
{
	return m_nSendInterval;
}

void CMaster104Tcp::GetFrameFromPackage( BYTE *msgbuf, int len )
{
	Scoped_Lock<Mutex> mut(m_mutex);
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

double CMaster104Tcp::GetMultiple( wstring strMultiple )
{
	if (strMultiple.empty()){
		return 1;
	}
	else
		return _ttof(strMultiple.c_str());
}
