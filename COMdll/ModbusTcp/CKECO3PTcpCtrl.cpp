#pragma once
#include "StdAfx.h"
#include "CKECO3PTcpCtrl.h"
#include "atlstr.h"

#include "Tools/Maths/MathCalcs.h"
#include "Tools/EngineInfoDefine.h"
//#include "Tools/vld.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../LAN_WANComm/PingIcmp.h"
#include "../COMdll//DebugLog.h"

CCKECO3PTcpCtrl::CCKECO3PTcpCtrl(void)
	:m_host("")
	,m_port(21)
	,m_wAddr1(-1)
	,m_wAddr2(-1)
	,m_wCmd(-1)
	,m_wType(-1)
	,m_nNetworkErrorCount(0)
	,m_bExitThread(false)
	,m_hsendthread(NULL)
	,m_nRollInterval(2)
	,m_hDataCheckRetryConnectionThread(NULL)
	,m_nSendReadCmdTimeInterval(30)
	,m_bReSponseSuccess(false)
	,m_nRecevieTimeOut(5000)
	,m_nPrecision(6)
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_nEngineIndex = 0;
	m_vecPointList.clear();
	m_ReadUnitList.clear();
}

CCKECO3PTcpCtrl::~CCKECO3PTcpCtrl(void)
{
	
}

bool CCKECO3PTcpCtrl::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(pRevData == NULL || length < 4)
		return false;
	m_bReSponseSuccess = true;
	if(pRevData[1] != 0x55 )	//55 代表收到并正确执行
	{
		PrintLog(L"ERROR: CKECO3P Cmd Excuse Fail!\r\n");
		return false;
	}

	if(!SumValid(pRevData,1,length-2))	//校验不通过,采用累加和校验,即 STATUS+RDATA+RCRC1+RCRC2=0；55 代表收到并正确执行
	{
		PrintLog(L"ERROR: CKECO3P Response Verification Fail!\r\n",false);
		return false;
	}

	_CKECO3PReadUnit* pReadUnit = FindCKECO3PReadUnit(m_wAddr1,m_wAddr2,m_wCmd,m_wType);
	if(pReadUnit)
	{
		m_oleUpdateTime = COleDateTime::GetCurrentTime();
		pReadUnit->nLength = length;
		pReadUnit->bResponseSuccess = true;
		memcpy(pReadUnit->cRecBuffer,pRevData,length);
	}
	return true;
}

void CCKECO3PTcpCtrl::SetHost( const string& strhost )
{
	m_host = strhost;
}

void CCKECO3PTcpCtrl::SetPort( u_short portnumer )
{
	m_port = portnumer;
}

void CCKECO3PTcpCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

void CCKECO3PTcpCtrl::SetSendCmdTimeInterval( int nTimeMs,int nRollInterval/*=2*/,int nTimeOut/*=5000*/ ,int nPrecision)
{
	m_nSendReadCmdTimeInterval = nTimeMs;
	m_nRollInterval = nRollInterval;
	m_nRecevieTimeOut = nTimeOut;
	m_nPrecision = nPrecision;
}

bool CCKECO3PTcpCtrl::Init()
{
	if (m_vecPointList.empty())
	{
		return false;
	}

	//生成读取序列
	for(int i=0;i<m_vecPointList.size();i++)
	{
		_CKECO3PReadUnit munit;
		munit.wAddr1 = _wtoi(m_vecPointList[i].GetParam(E_CO3P_ADDR1).c_str());
		munit.wAddr2 = _wtoi(m_vecPointList[i].GetParam(E_CO3P_ADDR2).c_str());
		munit.wType = _wtoi(m_vecPointList[i].GetParam(E_CO3P_TYPE).c_str());
		munit.wCmd = _wtoi(m_vecPointList[i].GetParam(E_CO3P_CMD).c_str());
		munit.nReadSuccessCount = 100;
		if(!CombineReadUnit(munit))
		{
			m_ReadUnitList.push_back(munit);
		}
	}

	if (!TcpIpConnectComm(m_host, m_port))
	{
		m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
		m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
		return false;
	}
	m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);

	return true;
}

bool CCKECO3PTcpCtrl::Exit()
{
	m_bExitThread = true;
	m_bReSponseSuccess = false;
	Disconnect();		//关闭TCP
	if(m_hsendthread)
		WaitForSingleObject(m_hsendthread, INFINITE);
	if(m_hDataCheckRetryConnectionThread)
		WaitForSingleObject(m_hDataCheckRetryConnectionThread,INFINITE);
	return true;
}

bool CCKECO3PTcpCtrl::SumValid( const unsigned char* pRevData,int nStartIndex,int len )
{
	if(pRevData == NULL || nStartIndex>=len || len <=0)
		return false;
	unsigned char crc8 =0;
	pRevData += nStartIndex;
	while(len--)
		crc8 = crc8 + (*pRevData++);
	return (crc8 == 0);
}

unsigned char CCKECO3PTcpCtrl::Sum__( const unsigned char* pRevData,int nStartIndex,int len )
{
	unsigned  char crc8 = 0x00;
	if(pRevData == NULL || nStartIndex>=len || len <=0)
		return crc8;
	pRevData += nStartIndex;
	while(len--)
		crc8 = crc8 + (*pRevData++);
	crc8 = ~crc8 +1;
	return crc8;
}

bool CCKECO3PTcpCtrl::SendReadCmd( WORD wAddr1 /*管理器地址, 采用二进制表示，长度为 1Byte，寻址范围为 00—7FH， */ , 
									WORD wAddr2 /*终端机地址 ，长度为 1Byte，寻址范围为 00—7FH，FFH 为特殊地址，采用广播方式时，使用该地址 */ , 
									WORD wType /*受控终端机的类型符，长度为 1Byte, 采用压缩 BCD 码表示。 */ ,
									WORD wCmd )
{
	m_wAddr1 = wAddr1;   //先清掉接收地址
	m_wAddr2 = wAddr2; 
	m_wCmd = wType;
	m_wType = wCmd;
	m_bReSponseSuccess = false;
	ZeroMemory(m_SendBuffer, sizeof(m_SendBuffer));
	m_SendBuffer[0] = 0xDB;
	m_SendBuffer[1] = (BYTE)wAddr1;
	m_SendBuffer[2] = (BYTE)wAddr2;
	m_SendBuffer[3] = (BYTE)wType;
	m_SendBuffer[4] = (BYTE)wCmd;

	unsigned char bCRC = Sum__(m_SendBuffer,1,4);
	m_SendBuffer[5] = bCRC;
	m_SendBuffer[6] = 0x00;
	m_SendBuffer[7] = 0xDD;
	return SendPackage((const char*)m_SendBuffer, sizeof(m_SendBuffer));
}

void CCKECO3PTcpCtrl::SendReadCommands()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode())
	{
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(m_sockConnected==NULL)
		return;

	int i = 0;
	bool bConnectionExist = false; //连接是否正常
	//set command
	for(i=0;i<m_vecPointList.size();i++)
	{
		m_vecPointList[i].SetToUpdate();
	}

	for (i = 0; i < m_ReadUnitList.size(); i++)
	{
		_CKECO3PReadUnit & mmUnit = m_ReadUnitList[i];
		if(SendReadMutil_(mmUnit))
			bConnectionExist = true;
	}

	if(bConnectionExist)//只要读到成功过一个数据，那么就认为连接是正常的
	{
		ClearNetworkError();
		//
	}
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read CO3P data Un-Recved, error %d times.\r\n", m_nNetworkErrorCount);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_CO3P_UN_REC);
		PrintLog(loginfo.GetString(),false);
	}

	UpdateValue();
}

bool CCKECO3PTcpCtrl::SendReadMutil_( _CKECO3PReadUnit& mmUnit )
{
	if(mmUnit.wAddr1 >0x7F || mmUnit.wAddr2 > 0x7F)
	{
		CString loginfo;
		loginfo.Format(L"ERROR: CO3P entry %x%x params not valid, skip!\r\n", mmUnit.wAddr1,mmUnit.wAddr2);
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_CO3P_,ERROR_CUSTOM_MODBUS_INVALID);
		return false;
	}

	bool bSent = SendReadCmd(mmUnit.wAddr1, mmUnit.wAddr2, mmUnit.wType, mmUnit.wCmd);
	mmUnit.bResponseSuccess = false;
	if(!bSent)
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: CO3P entry %s send fail.\r\n", mmUnit.strPointName.c_str());
		PrintLog(loginfo.GetString(),false);
		return false;
	}

	//等待回应的机制，是半双工网络
	//10秒
	int nWaitingCount = m_nRecevieTimeOut; 
	while(!m_bReSponseSuccess  && nWaitingCount>0)
	{
		Sleep(1);
		nWaitingCount--;
	}
	//等待不到回应，如果之前有过回应，那么重试一次！
	if(nWaitingCount<=0)
	{
		if(mmUnit.nReadSuccessCount>0)
		{
			//曾经读成功过的，就再发一次
			SendReadCmd(mmUnit.wAddr1, mmUnit.wAddr2, mmUnit.wType, mmUnit.wCmd);
			nWaitingCount = m_nRecevieTimeOut; 
			while(!m_bReSponseSuccess  && nWaitingCount>0)
			{
				Sleep(1);
				nWaitingCount--;
			}

			if(nWaitingCount<=0)
			{
				CString loginfo;
				loginfo.Format(L"ERROR: CO3P (ADDR: %u %u) no response!\r\n", mmUnit.wAddr1,mmUnit.wAddr2);
				PrintLog(loginfo.GetString(),false);
				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_CO3P_NO_RESPONSE);
				mmUnit.nReadSuccessCount = 0; //读两次都失败，下次只读一次。
			}
		}
	}
	else
	{//只要有回应，就认为成功（部分数据回不来不代表网络出了问题）
		if(mmUnit.nReadSuccessCount<10000)
			mmUnit.nReadSuccessCount++;
		Sleep(m_nSendReadCmdTimeInterval); 
		return true;
	}

	Sleep(m_nSendReadCmdTimeInterval); 
	return false;
}

UINT WINAPI CCKECO3PTcpCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CCKECO3PTcpCtrl* pthis = (CCKECO3PTcpCtrl*)lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			pthis->SendReadCommands();
			int nSleep = pthis->m_nRollInterval;
			while (!pthis->m_bExitThread)
			{
				if(nSleep <= 0)
				{
					break;
				}
				nSleep--;
				Sleep(1000);
			}
		}
	}

	return 0;
}

UINT WINAPI CCKECO3PTcpCtrl::ThreadCheckAndRetryConnection( LPVOID lparam )
{
	CCKECO3PTcpCtrl* pthis = (CCKECO3PTcpCtrl*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(pthis->m_nNetworkErrorCount>=30 || !pthis->m_bConnectOK)
			{
				if(pthis->ReConnect())
					pthis->ClearNetworkError();
			}

			int nSleep = 5;
			while (!pthis->m_bExitThread)
			{
				if(nSleep <= 0)
				{
					break;
				}
				nSleep--;
				Sleep(1000);
			}
		}
	}
	return 0;
}

void CCKECO3PTcpCtrl::TerminateSendThread()
{
	m_bExitThread = true;
	WaitForSingleObject(m_hsendthread, INFINITE);
	WaitForSingleObject(m_hDataCheckRetryConnectionThread,INFINITE);
}

_CKECO3PReadUnit* CCKECO3PTcpCtrl::FindCKECO3PReadUnit( WORD wAddr1,WORD wAddr2,WORD wType,WORD wCmd )
{
	for(int i=0; i<m_ReadUnitList.size();  ++i)
	{
		if(m_ReadUnitList[i].wAddr1 == wAddr1 && m_ReadUnitList[i].wAddr2 == wAddr2 && m_ReadUnitList[i].wType == wType && m_ReadUnitList[i].wCmd == wCmd)
			return &m_ReadUnitList[i];
	}
	return NULL;
}

void CCKECO3PTcpCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_vecPointList = pointlist;
}

bool CCKECO3PTcpCtrl::CombineReadUnit( const _CKECO3PReadUnit & mrUnit )
{
	if(m_ReadUnitList.size()<=0)
		return false;

	int i =0;
	for(i=m_ReadUnitList.size()-1;i>=0;i--)
	{
		if(m_ReadUnitList[i].wAddr1==mrUnit.wAddr1 && m_ReadUnitList[i].wAddr2==mrUnit.wAddr2 && m_ReadUnitList[i].wCmd==mrUnit.wCmd && m_ReadUnitList[i].wType==mrUnit.wType)
		{ 
			return true;
		}
	}
	return false;
}

bool CCKECO3PTcpCtrl::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	PrintLog(L"INFO : CKECO3P reconnecting...\r\n",false);
	Disconnect(); //tcpip recv thread stop, and clear socket

	if (!TcpIpConnectComm(m_host, m_port))
	{
		PrintLog(L"ERROR: CKECO3P reconnecting failed!\r\n");
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_RECONNECT_CKECO3P);
		return false;
	}
	PrintLog(L"INFO : CKECO3P reconnecting success!\r\n",false);
	return true;
}

void CCKECO3PTcpCtrl::SetNetworkError()
{
	if(m_nNetworkErrorCount<10000)
		m_nNetworkErrorCount++;
}

void CCKECO3PTcpCtrl::ClearNetworkError()
{
	m_nNetworkErrorCount = 0;
}

bool CCKECO3PTcpCtrl::GetValue( const wstring& pointname, double& result )
{
	for (unsigned int i = 0; i < m_vecPointList.size(); i++)
	{
		const DataPointEntry& entry = m_vecPointList[i];
		if (entry.GetShortName() == pointname)
		{
			result =  entry.GetValue();
			return true;
		}
	}

	return false;
}

bool CCKECO3PTcpCtrl::SetValue( const wstring& pointname, double fValue )
{
	return true;
}

void CCKECO3PTcpCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	for (unsigned int i = 0; i < m_vecPointList.size(); i++)
	{
		DataPointEntry&  co3pentery = m_vecPointList[i];
		if(co3pentery.GetUpdateSignal()<10)
		{
			valuelist.push_back(make_pair(co3pentery.GetShortName(), co3pentery.GetSValue()));
		}
	}

	if(m_vecPointList.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_CO3P,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
	}
}

bool CCKECO3PTcpCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,wstring& wstrValue )
{
	_CKECO3PReadUnit* PUnit = FindCKECO3PReadUnit(_wtoi(entry.GetParam(E_CO3P_ADDR1).c_str()),_wtoi(entry.GetParam(E_CO3P_ADDR2).c_str()),_wtoi(entry.GetParam(E_CO3P_TYPE).c_str()),_wtoi(entry.GetParam(E_CO3P_CMD).c_str()));
	if(PUnit && PUnit->bResponseSuccess && PUnit->nLength>0)
	{
		int		nLength = PUnit->nLength;
		char	cRecBuffer[1024] = {0};
		nLength = (nLength>1024)?1024:nLength;
		memcpy(cRecBuffer,PUnit->cRecBuffer,nLength);
		wstring strInteger = entry.GetParam(E_CO3P_INTEGER);			//整数部分
		wstring strDecimal = entry.GetParam(E_CO3P_DECIMAL);			//小数部分
		std::ostringstream sqlstream;
		//有两种  根据小数部分是否有/来判断是否取位的 其他默认为小数部分
		if(strDecimal.find_first_of(L"_",0)!=wstring::npos)		
		{
			vector<wstring> vecInteger,vecDecimal;
			Project::Tools::SplitStringByChar(strInteger.c_str(),L'.',vecInteger);
			Project::Tools::SplitStringByChar(strDecimal.c_str(),L'_',vecDecimal);
			if(vecInteger.size() == 2)
			{
				int nStartPos = _wtoi(vecInteger[0].c_str());
				int nRangee = _wtoi(vecInteger[1].c_str());
				CString strValue = _T("");
				if(nStartPos>=0 && nRangee <nLength && nStartPos+nRangee<=nLength)
				{
					for(int nPos = nStartPos; nPos <nStartPos+nRangee; ++nPos)
					{
						strValue.Format(_T("%02d"),(WORD)cRecBuffer[nPos]);
						sqlstream << Project::Tools::WideCharToAnsi(strValue);
					}
					WORD dwValue = _wtoi(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());

					if(vecDecimal.size() == 2)
					{
						int nStartPos = _wtoi(vecDecimal[0].c_str());
						int nRangee = _wtoi(vecDecimal[1].c_str());
						if(nStartPos>=0 && nStartPos <16)
						{
							int nValue = GetBitFromWord(dwValue,nStartPos);
							strValue.Format(_T("%d"),nValue);
							wstrValue = strValue.GetString();
							return true;
						}
					}
				}
			}
		}
		else
		{
			vector<wstring> vecInteger,vecDecimal;
			Project::Tools::SplitStringByChar(strInteger.c_str(),L'.',vecInteger);
			Project::Tools::SplitStringByChar(strDecimal.c_str(),L'.',vecDecimal);
			sqlstream << "0";
			if(vecInteger.size() == 2)
			{
				int nStartPos = _wtoi(vecInteger[0].c_str());
				int nRangee = _wtoi(vecInteger[1].c_str());
				CString strValue = _T("");
				if(nStartPos>=0 && nRangee <nLength && nStartPos+nRangee<=nLength)
				{
					for(int nPos = nStartPos; nPos <nStartPos+nRangee; ++nPos)
					{
						strValue.Format(_T("%02d"),(WORD)cRecBuffer[nPos]);
						sqlstream << Project::Tools::WideCharToAnsi(strValue);
					}
				}
			}

			if(vecDecimal.size() == 2)
			{
				int nStartPos = _wtoi(vecDecimal[0].c_str());
				int nRangee = _wtoi(vecDecimal[1].c_str());
				if(nStartPos>=0 && nRangee <nLength && nStartPos+nRangee<=nLength)
				{
					sqlstream << ".";
					CString strValue = _T("");
					for(int nPos = nStartPos; nPos <nStartPos+nRangee; ++nPos)
					{
						strValue.Format(_T("%02d"),(WORD)cRecBuffer[nPos]);
						sqlstream << Project::Tools::WideCharToAnsi(strValue);
					}
				}
			}
			double dValue = _wtof(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			wstrValue = Project::Tools::RemoveDecimalW(dValue,m_nPrecision);
			return true;
		}
	}
	return false;
}

int CCKECO3PTcpCtrl::GetBitFromWord( WORD dwValue, int nIndex )
{
	WORD dwTemp = dwValue<<(16-nIndex-1);
	int nTemp = dwTemp>> 15;
	return nTemp;
}

bool CCKECO3PTcpCtrl::UpdateValue()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	for (unsigned int i = 0; i < m_vecPointList.size(); i++)
	{
		DataPointEntry&  co3pentery = m_vecPointList[i];
		wstring strValue = L"";
		if(GetValueByDataTypeAndRange(co3pentery,strValue))
		{
			co3pentery.SetSValue(strValue);
			co3pentery.SetUpdated();
		}
		else
		{
			co3pentery.SetToUpdate();
		}
	}
	return true;
}

void CCKECO3PTcpCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}

