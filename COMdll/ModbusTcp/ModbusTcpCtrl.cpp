#pragma once
#include "StdAfx.h"
#include "ModbusTcpCtrl.h"
#include "atlstr.h"

#include "Tools/Maths/MathCalcs.h"
#include "Tools/EngineInfoDefine.h"
//#include "Tools/vld.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../LAN_WANComm/PingIcmp.h"
#include "../COMdll//DebugLog.h"

const int default_portnumber = 502;

CModbusTcpCtrl::CModbusTcpCtrl(void)
	:m_host("")
	,m_port(default_portnumber)
	,m_hsendthread(NULL)
	,m_nSendReadCmdTimeInterval(30)
	,m_nRecevieTimeOut(5000)
	,m_nRecvedSlave(-1)
	,m_hDataCheckRetryConnectionThread(NULL)
	,m_nModbusNetworkErrorCount(0)
	,m_bExitThread(false)
	,m_bSortPoint(false)
	,m_nSendCmdCount(0)
	,m_bReadOneByOneMode(false)
	,m_nMutilReadCount(99)
	,m_nIDInterval(500)
	,m_nPollSendInterval(2)
	,m_nOutPut(0)
	,m_nPrecision(6)
	,m_bDisSingleRead(false)
	,m_nCmdCount(0)
	,m_nResponseCount(0)
	,m_nUpdatePointCount(0)
	,m_strUpdateLog("")
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_strErrInfo = "";
	m_nEngineIndex = 0;
	m_vecPointList.clear();
	m_ModbusValue.clear();
	m_sendCmdEvent = ::CreateEvent(NULL, false, false, L"");
}

CModbusTcpCtrl::~CModbusTcpCtrl(void)
{
	if (m_hsendthread != NULL)
	{
		::CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
}

//golding ,设置发送读取命令的毫秒数
void    CModbusTcpCtrl::SetSendCmdTimeInterval(int nTimeMs,int nMutilReadCount,int nIDInterval,int nTimeOut,int nPollInterval,int nPrecision,bool bDisSingleRead)
{
	m_nSendReadCmdTimeInterval = nTimeMs;
	m_nMutilReadCount = nMutilReadCount;
	m_nIDInterval = nIDInterval;
	m_nRecevieTimeOut = nTimeOut;
	m_nPollSendInterval = nPollInterval;
	m_nPrecision = nPrecision;
	m_bDisSingleRead = bDisSingleRead;
}

/*
 * the modbus tcp protocal
	The request and response are prefixed by six bytes as follows

	byte 0:	transaction identifier - copied by server - usually 0
	byte 1:	transaction identifier - copied by server - usually 0
	byte 2:	protocol identifier = 0
	byte 3:	protocol identifier = 0
	byte 4:	length field (upper byte) = 0 (since all messages are smaller than 256)
	byte 5:	length field (lower byte) = number of bytes following
	byte 6:	unit identifier (previously ‘slave address’)
	byte 7:	MODBUS function code
	byte 8 on:	data bytes.
	byte 9...  the real datas. each data is two bytes.

	So an example transaction ‘read 1 register at offset 4 from UI 9’ returning a value of 5 would be
	request:  00 00 00 00 00 06 09 03 00 04 00 01
	response: 00 00 00 00 00 05 09 03 02 00 05
*/

// notify received package, use for sub-class derivativation
//bool CModbusTcpCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
//{
//	//20 00 00 00 00 2b 01 03 28 05 2a ba a4 00 00 03 01 03 00 00 00 00 00 00 00 00 00 00 00 00 
//	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
//	WORD wHeadAddr=0, wValue=0;
//    
//    unsigned int  slaveid = *(pRevData+6);
//
//	
//	m_nRecvedSlave = slaveid;  //收到即可保存，收到后即可继续下发.
//	m_oleUpdateTime = COleDateTime::GetCurrentTime();
//    memcpy(&wHeadAddr, pRevData, 2);
//	WORD funccode = pRevData[7];
//	int datanum = 0;
//	if (funccode == 1 || funccode == 2)		//读线圈  独离散输入
//	{
//		datanum = *(pRevData+8);
//		for(int nIndex=0; nIndex < datanum; ++nIndex)
//		{
//			wValue = pRevData[9+nIndex];
//			for(int i=0; i<8; ++i)
//			{
//				DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//				if (!entry){					
//					continue;
//				}
//				double dval =(wValue>>i) & 1 ;
//				dval = dval/entry->GetMultiple();
//
//				entry->SetValue(dval);
//				entry->SetUpdated();
//				wHeadAddr++;
//			}
//		}
//	}
//	else if(funccode == 3)					//读保持寄存器
//	{
//		datanum = *(pRevData+8)/2;
//		for (int nIndex=0; nIndex < datanum; nIndex++)
//		{
//			DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//			if (!entry){
//				CString strTemp;
//				strTemp.Format(L"ERROR: modbus recv ErrorData: %d,%d,%d\r\n",slaveid, (int)wHeadAddr, (int)funccode);
//				PrintLog(strTemp.GetString());
//				continue;
//			}
//
//			wValue = pRevData[9+nIndex*2];
//			wValue = (wValue&0x00ff)<<8;
//			wValue |= (pRevData[9+nIndex*2+1]&0x00ff);
//
//			int nFlag = (pRevData[9+nIndex*2]>>7) & 1 ;			//第一位是否为1 若为1则为负数
//			int nValue = wValue;
//			if(nFlag >0 )		//负数
//			{
//				nValue = nValue-65536;
//			}
//			double dval = nValue/entry->GetMultiple();
//			entry->SetValue(dval);
//			entry->SetUpdated();
//
//			wHeadAddr++;
//		}
//	}
//	else if(funccode == 4)			//读输入寄存器
//	{
//		datanum = *(pRevData+8)/2;
//		for (int nIndex=0; nIndex < datanum; nIndex++)
//		{
//			DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//			if (!entry){
//				CString strTemp;
//				strTemp.Format(L"ERROR: modbus recv ErrorData: %d,%d,%d\r\n",slaveid, (int)wHeadAddr, (int)funccode);
//				PrintLog(strTemp.GetString());
//				continue;
//			}
//
//			wValue = pRevData[9+nIndex*2];
//			wValue = (wValue&0x00ff)<<8;
//			wValue |= (pRevData[9+nIndex*2+1]&0x00ff);
//
//			int nFlag = (pRevData[9+nIndex*2]>>7) & 1 ;			//第一位是否为1 若为1则为负数
//			int nValue = wValue;
//			if(nFlag >0 )		//负数
//			{
//				nValue = nValue-65536;
//			}
//			double dval = nValue/entry->GetMultiple();
//
//			entry->SetValue(dval);
//			entry->SetUpdated();
//
//			wHeadAddr++;
//		}
//	}
//	else if(funccode == 5)			//写单个线圈 相应
//	{		
//		wHeadAddr = pRevData[8];
//		wHeadAddr = (wHeadAddr&0x00ff)<<8;
//		wHeadAddr |= (pRevData[9]&0x00ff);
//		wValue =(pRevData[10]>>7) & 1 ;
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if (!entry)
//		{
//			return false;
//		}
//		double dval = wValue/entry->GetMultiple();
//		entry->SetValue(dval);
//		entry->SetUpdated();
//	}
//	else if(funccode == 6)			//写单个寄存器
//	{
//		wHeadAddr = pRevData[8];
//		wHeadAddr = (wHeadAddr&0x00ff)<<8;
//		wHeadAddr |= (pRevData[9]&0x00ff);
//		
//		wValue = pRevData[10];
//		wValue = (wValue&0x00ff)<<8;
//		wValue |= (pRevData[11]&0x00ff);
//		
//
//		wHeadAddr++;		//地址需加1
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, 6);
//		if (!entry)
//		{
//			return false;
//		}
//
//		int nFlag = (pRevData[10]>>7) & 1 ;			//第一位是否为1 若为1则为负数
//		int nValue = wValue;
//		if(nFlag >0 )		//负数
//		{
//			nValue = nValue-65536;
//		}
//		double dval = nValue/entry->GetMultiple();
//
//		entry->SetValue(dval);
//		entry->SetUpdated();
//	}
//	else if(funccode == 0x81)		//错误码
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 01 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x82)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 02 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x83)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 03 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x84)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 04 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x85)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 05 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	else if(funccode == 0x86)
//	{
//		WORD errccode = pRevData[8];
//		CString strTemp;
//		strTemp.Format(L"ERROR: modbus fun 06 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
//		PrintLog(strTemp.GetString());
//		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
//		DataPointEntry* entry = FindEntry(slaveid, wHeadAddr, funccode);
//		if(entry)
//		{
//			m_mapErrPoint[entry->GetShortName()] = entry;
//		}
//	}
//	return true;
//}

//edit by robert 20151113
bool CModbusTcpCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
{
	//20 00 00 00 00 2b 01 03 28 05 2a ba a4 00 00 03 01 03 00 00 00 00 00 00 00 00 00 00 00 00 
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(length <= 8)
		return false;

	if(m_nOutPut>0)
	{
		CString strTemp;
		strTemp.Format(L"DEBUG: Modbus Recv[%s]\r\n",Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)pRevData,9).c_str()).c_str());
		PrintLog(strTemp.GetString(),false);
	}

	SetSendCmdEvent();

	WORD wLenth = 0;
	wLenth = pRevData[4];
	wLenth = (wLenth&0x00ff)<<8;
	wLenth |= (pRevData[5]&0x00ff);

	if (length < wLenth+6)
		return false;

	OutPutModbusReceive(pRevData,length);

	WORD wHeadAddr=0, wValue=0;
	unsigned int  slaveid = *(pRevData+6);

	m_nRecvedSlave = slaveid;  //收到即可保存，收到后即可继续下发.
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	memcpy(&wHeadAddr, pRevData, 2);
	WORD funccode = pRevData[7];
	int datanum = 0;
	if (funccode == 1 || funccode == 2)		//读线圈  独离散输入
	{
		datanum = *(pRevData+8);
		int nUpdateNum = 0;
		if(!m_bReadOneByOneMode)
		{
			nUpdateNum = GetMultiReadCount(slaveid,wHeadAddr,funccode);
		}
		for(int nIndex=0; nIndex < datanum; ++nIndex)
		{
			wValue = pRevData[9+nIndex];
			if(m_bReadOneByOneMode)				//单点读取模式
			{
				m_nResponseCount++;
				DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
				if (!entry)
				{	
					wHeadAddr++;
					continue;
				}
				else
				{
					WORD dval = wValue & 1 ;
					entry->SetWORD(dval);
					entry->SetUpdated();
					wHeadAddr++;
					m_nUpdatePointCount++;
				}
			}
			else
			{
				for(int i=0; i<8; ++i)
				{
					nUpdateNum--;
					if(nUpdateNum<0)
						break;
					m_nResponseCount++;
					DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
					if (!entry)
					{	
						wHeadAddr++;
						continue;
					}
					else
					{
						WORD dval =(wValue>>i) & 1 ;
						entry->SetWORD(dval);
						entry->SetUpdated();
						wHeadAddr++;
						m_nUpdatePointCount++;
					}
				}
			}
		}
	}
	else if(funccode == 3)					//读保持寄存器
	{
		datanum = *(pRevData+8)/2;
		for (int nIndex=0; nIndex < datanum; nIndex++)
		{
			m_nResponseCount++;
			DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
			if (!entry)
			{
				wHeadAddr++;
				continue;
			}
			else
			{
				wValue = pRevData[9+nIndex*2];
				wValue = (wValue&0x00ff)<<8;
				wValue |= (pRevData[9+nIndex*2+1]&0x00ff);

				entry->SetWORD(wValue);
				entry->SetUpdated();
				wHeadAddr++;
				m_nUpdatePointCount++;
			}
		}
	}
	else if(funccode == 4)			//读输入寄存器
	{
		datanum = *(pRevData+8)/2;
		for (int nIndex=0; nIndex < datanum; nIndex++)
		{
			m_nResponseCount++;
			DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, funccode);
			if (!entry)
			{
				wHeadAddr++;
				continue;
			}
			else
			{
				wValue = pRevData[9+nIndex*2];
				wValue = (wValue&0x00ff)<<8;
				wValue |= (pRevData[9+nIndex*2+1]&0x00ff);

				entry->SetWORD(wValue);
				entry->SetUpdated();
				wHeadAddr++;
				m_nUpdatePointCount++;
			}
		}
	}
	else if(funccode == 5)			//写单个线圈 相应
	{		
		wHeadAddr = pRevData[8];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[9]&0x00ff);
		wValue =(pRevData[10]>>7) & 1 ;
		wHeadAddr++;		//地址需加1

	}
	else if(funccode == 6)			//写单个寄存器
	{
		wHeadAddr = pRevData[8];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[9]&0x00ff);
			
		wValue = pRevData[10];
		wValue = (wValue&0x00ff)<<8;
		wValue |= (pRevData[11]&0x00ff);
			
	}
	else if(funccode == 0x10)			//写多个寄存器
	{
		wHeadAddr = pRevData[8];
		wHeadAddr = (wHeadAddr&0x00ff)<<8;
		wHeadAddr |= (pRevData[9]&0x00ff);

		wValue = pRevData[10];
		wValue = (wValue&0x00ff)<<8;
		wValue |= (pRevData[11]&0x00ff);

		wHeadAddr++;		//地址需加1		
	}
	else if(funccode == 0x81)		//错误码
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 01 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
		PrintLog(strTemp.GetString(),false);

		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,1,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 1);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_01);
		}
	}
	else if(funccode == 0x82)
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 02 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,2,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 2);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_02);
		}
	}
	else if(funccode == 0x83)
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 03 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,3,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 3);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_03);
		}
	}
	else if(funccode == 0x84)
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 04 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,4,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 4);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_04);
		}
	}
	else if(funccode == 0x85)
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 05 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,5,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 5);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_05);
		}
	}
	else if(funccode == 0x86)
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 06 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,6,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 6);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_06);
		}
	}
	else if(funccode == 0x90)
	{
		WORD errccode = pRevData[8];
		CString strTemp;
		strTemp.Format(L"ERROR: modbus fun 16 Error:id-%d,addr-%d, errcode-%d\r\n",slaveid, (int)wHeadAddr, (int)errccode);
		PrintLog(strTemp.GetString(),false);
		m_strErrInfo = Project::Tools::OutTimeInfo(Project::Tools::WideCharToAnsi(strTemp));
		SetModbusReadUnitErrFlag(slaveid,16,wHeadAddr);
		DataPointEntry* entry = FindEntry_(slaveid, wHeadAddr, 16);
		if(entry)
		{
			m_mapErrPoint[entry->GetShortName()] = entry;
			CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_MODBUS_16);
		}
	}
	return true;
}

/***************************************************
*@brief:初始化hashmap的信息
*@param: void
*@return: bool,true设置成功，false设置失败。
*@author: ZHW 2011-02-28
****************************************************/

/***************************************************
*@brief:ModbusTcp网络连接并通讯
*@param: u_short uPort端口, string strIp网络地址
*@return: bool,true设置成功，false设置失败。
*@author: ZHW 2011-02-24
****************************************************/
bool CModbusTcpCtrl::Init()
{
	// if there is not modbus point, thread will not be created.
	if (m_pointlist.empty())
	{
		return false;
	}

	//Sort Point
	//SortPointByDevice_();

	if (!TcpIpConnectComm(m_host, m_port))
	{
		m_strErrInfo = Project::Tools::OutTimeInfo("connect fail");
		m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
		m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
		return false;
	}
	m_hsendthread = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hDataCheckRetryConnectionThread = (HANDLE) _beginthreadex(NULL, 0, ThreadCheckAndRetryConnection, this, NORMAL_PRIORITY_CLASS, NULL);
	
	return true;
}

void CModbusTcpCtrl::SortPointByDevice()
{
	//按设备排序（先设备地址、后点地址）
	int i=0, j=0;
	for(i=0;(i+1)<m_pointlist.size();i++)
	{
		for(j=i+1; j<m_pointlist.size();j++)
		{
			DataPointEntry entryi = m_pointlist[i];
			DataPointEntry entryj = m_pointlist[j];
			int nDeviceAdd1 = _wtoi(entryi.GetParam(1).c_str());
			int nDeviceAdd2 = _wtoi(entryj.GetParam(1).c_str());

			if(nDeviceAdd1>nDeviceAdd2)
			{
				m_pointlist[i] = entryj;
				m_pointlist[j] = entryi;
			}
			else if(nDeviceAdd1==nDeviceAdd2)
			{
				int nPointAdd1 = _wtoi(entryi.GetParam(2).c_str());
				int nPointAdd2 = _wtoi(entryj.GetParam(2).c_str());
				if(nPointAdd1> nPointAdd2)
				{
					m_pointlist[i] = entryj;
					m_pointlist[j] = entryi;
				}
			}
		}
	}


	//生成读取序列
	for(i=0;i<m_pointlist.size();i++)
	{
		_ModbusReadUnit munit;
		munit.strPointName = m_pointlist[i].GetShortName();
		munit.nSlaveId = _wtoi(m_pointlist[i].GetParam(1).c_str());
		munit.dwAddFrom = _wtoi(m_pointlist[i].GetParam(2).c_str());
		munit.dwAddTo = munit.dwAddFrom;
		munit.dwFuncCode = _wtoi(m_pointlist[i].GetParam(3).c_str());
		munit.nReadSuccessCount = 100;
		if(!CombineReadUnit(munit))
		{
			m_ReadUnitList.push_back(munit);
		}
	}
}

void    CModbusTcpCtrl::SetNetworkError()
{
	if(m_nModbusNetworkErrorCount<10000)
		m_nModbusNetworkErrorCount++;
}
void    CModbusTcpCtrl::ClearNetworkError()
{
	m_nModbusNetworkErrorCount = 0;
}

bool CModbusTcpCtrl::CombineReadUnit(const _ModbusReadUnit &  mrUnit,bool bModbusEnd)
{
	if(m_ReadUnitList.size()<=0)
		return false;

	//find unit
	bool bFound = false;
	int i =0;
	for(i=m_ReadUnitList.size()-1;i>=0;i--)
	{
		if(m_ReadUnitList[i].nSlaveId==mrUnit.nSlaveId && m_ReadUnitList[i].dwFuncCode==mrUnit.dwFuncCode)
		{ //必须function也要相同，否则读回来的只是相应function代码的点
			bFound = true;
			break;
		}
	}

	if(bFound)
	{
		if(mrUnit.dwAddFrom > (m_ReadUnitList[i].dwAddTo+1))
		{
			return false;
		}
		else if(m_ReadUnitList[i].dwAddTo -m_ReadUnitList[i].dwAddFrom >= m_nMutilReadCount && m_ReadUnitList[i].bModbusEnd == true)		//个数大于后 且是完整的连续点位了
		{
			return false;
		}
		else if(mrUnit.dwAddFrom >= m_ReadUnitList[i].dwAddFrom && mrUnit.dwAddFrom>=m_ReadUnitList[i].dwAddTo)
		{
			m_ReadUnitList[i].dwAddTo = mrUnit.dwAddFrom;
			m_ReadUnitList[i].bModbusEnd = bModbusEnd;
		}

		return true; //combined
	}
	else
	{
		return false;//not combined
	}

	return false;
}

/***************************************************
*@brief:TCP/IP通讯发送数据[读指令]
*@param: unsigned int nSlaveId //设备的Slave ID 号
, WORD wAddrFrom		//读取属性的起始地址
, WORD wAddrTo			//读取属性的终止地址
, WORD wFunc);			//设备通讯类型
*@return: bool,true发送成功，false发送失败。
*@author: ZHW 2011-01-07
****************************************************/
bool CModbusTcpCtrl:: SendReadCmd(unsigned int nSlaveId, WORD wAddrFrom
								  , WORD wAddrTo, WORD wFunc)

{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);

	if(wFunc==0x06 || wFunc==0x10 || wFunc==0x17)
		wFunc = 0x03;
	else if(wFunc==0x05)
		wFunc = 0x01;
	else if(wFunc==0x0F)
		wFunc = 0x01;

	m_nRecvedSlave = -1; //先清掉接收Slave
		
	ZeroMemory(m_SendBuffer, sizeof(m_SendBuffer));
	memcpy(m_SendBuffer, &wAddrFrom, sizeof(WORD));

	m_SendBuffer[5] = 0x06;
	m_SendBuffer[6] = (BYTE)nSlaveId;
	m_SendBuffer[7] = (BYTE)wFunc;
	m_SendBuffer[8] = ((wAddrFrom-1)&0xFF00)>>8;
	m_SendBuffer[9] = ((wAddrFrom-1)&0x00FF);
	m_SendBuffer[10] = ((wAddrTo-wAddrFrom+1)&0xFF00)>>8;
	m_SendBuffer[11] = ((wAddrTo-wAddrFrom+1)&0x00FF);

	//SendMessage
	if(m_nOutPut>0)
	{
		CString strTemp;
		strTemp.Format(L"DEBUG: Modbus Send[%s]\r\n",Project::Tools::AnsiToWideChar(charToHexStr((unsigned char*)m_SendBuffer,sizeof(m_SendBuffer)).c_str()).c_str());
		PrintLog(strTemp.GetString(),false);
	}
	return SendPackage(m_SendBuffer, sizeof(m_SendBuffer));

}

/**********************************************************************
*@brief:TCP/IP通讯发送数据[写指令]
*@param: unsigned int nSlaveId   //设备的Slave ID 号
, WORD wAddress			//写入属性的地址
, WORD wValue			//属性值
, WORD wFunc);			//设备通讯类型
*@return: bool,true成功，false失败
*@author: ZHW 2011-01-24
***********************************************************************/
bool CModbusTcpCtrl::SendWriteCmd(unsigned int nSlaveId, WORD wAddress
								  , WORD wValue, WORD wFunc)

{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);
	if(wFunc == 16)
	{
		char  cSendBuffer[256] = {0};
		ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
		memcpy(cSendBuffer, &wAddress, sizeof(WORD));

		//////////////////////////////////////////////////////////////////////////
		cSendBuffer[6] = (BYTE)nSlaveId;
		cSendBuffer[7] = (BYTE)wFunc;
		cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
		cSendBuffer[9] = ((wAddress-1)&0x00FF);
		WORD wNum = 1;
		WORD wRange = 2;
		cSendBuffer[10] = (wNum&0xFF00)>>8;
		cSendBuffer[11] = (wNum&0x00FF);
		cSendBuffer[12] = (BYTE)wRange;
		BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序
		int nCount = 13;
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress, 16);
		if (entry)
		{
			entry->SetWORD(wValue);
		}
		memcpy(byteToEx, &wValue, 2);
		nCount = 13;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
		WORD wCount = nCount-5;
		cSendBuffer[5] = (BYTE)wCount;
		return SendPackage(cSendBuffer, nCount+1);
	}
	else
	{
		BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序
		ZeroMemory(m_SendBuffer, sizeof(m_SendBuffer));
		memcpy(m_SendBuffer, &wAddress, sizeof(WORD));

		if (wFunc == 06)
		{
			m_SendBuffer[5] = 0x06;
			m_SendBuffer[6] = (BYTE)nSlaveId;
			m_SendBuffer[7] = (BYTE)wFunc;
			m_SendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
			m_SendBuffer[9] = ((wAddress-1)&0x00FF);
			memcpy(byteToEx, &wValue, 2);
			m_SendBuffer[10] = byteToEx[1];
			m_SendBuffer[11] = byteToEx[0];
		}
		else
		{
			m_SendBuffer[5] = 0x06;
			m_SendBuffer[6] = (BYTE)nSlaveId;
			m_SendBuffer[7] = 0x05;
			m_SendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
			m_SendBuffer[9] = ((wAddress-1)&0x00FF);
			memcpy(byteToEx, &wValue, 2);

			if (wValue > 0)
			{
				m_SendBuffer[10] = 0xFF;
				m_SendBuffer[11] = 0;
			}
			else
			{
				m_SendBuffer[10] = 0;
				m_SendBuffer[11] = 0;
			}	
		}
		//SendMessage
		return SendPackage(m_SendBuffer, sizeof(m_SendBuffer));
	}
	return false;
}

void CModbusTcpCtrl::SetHost( const string& strhost )
{
	m_host = strhost;
}

void CModbusTcpCtrl::SetPort( u_short portnumer )
{
	m_port = portnumer;
}

UINT CModbusTcpCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CModbusTcpCtrl* pthis = (CModbusTcpCtrl*)lparam;
	if (pthis != NULL)
	{
		pthis->SortPointByDevice_();

		while (!pthis->m_bExitThread)
		{
			if(!pthis->m_bReadOneByOneMode)
			{
				pthis->SendReadCommandsByActive();
			}
			else
			{
				pthis->SendOneByOneByActive();			//一个点一个点发送
			}

			int nPollSleep = pthis->m_nPollSendInterval;
			while (!pthis->m_bExitThread)
			{
				if(nPollSleep <= 0)
				{
					break;
				}
				nPollSleep--;
				Sleep(1000);
			}
		}
	}

	return 0;
}

void CModbusTcpCtrl::TerminateSendThread()
{
	m_bExitThread = true;
	WaitForSingleObject(m_hsendthread, INFINITE);
	WaitForSingleObject(m_hDataCheckRetryConnectionThread,INFINITE);
}

void CModbusTcpCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

void CModbusTcpCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry&  modbusentry = m_pointlist[i];
		GetValueByDataTypeAndRange(modbusentry,valuelist);
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_MODBUS,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
		strName.Format(_T("%s%d"),LOG_MODBUS,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str())));
	}
}

DataPointEntry* CModbusTcpCtrl::FindEntry(DWORD slaveid, DWORD headaddress, DWORD funccode)
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if ((entry.GetSlaveID() == slaveid) && (entry.GetHeadAddress() == headaddress)){		//edit by robert 20140715 写值失败 取消功能吗匹配
			return &m_pointlist[i];
		}
	}

	return NULL;
}

//bool	CModbusTcpCtrl::SetValue(const wstring& pointname, double fValue)
//{
//	for (unsigned int i = 0; i < m_pointlist.size(); i++)
//	{
//		const DataPointEntry& entry = m_pointlist[i];
//		if (entry.GetShortName() == pointname)
//		{
//
//			DWORD slaveid = entry.GetSlaveID();
//			DWORD headaddress = entry.GetHeadAddress();
//			DWORD funccode = entry.GetFuncCode();
//
//			if (slaveid == 0 || headaddress == 0 || funccode == 0)
//			{
//				PrintLog(L"ERROR: modbus write point params invalid!\r\n");
//
//				return false;
//			}
//
//			//golding: 注意下面这个陷阱, modbus写值可能会出问题
//			if (funccode == 03){
//				//防止老项目配置没有配置对。做的折中方案。
//				funccode = 06;
//			}
//
//			fValue = fValue*entry.GetMultiple();			//edit by robert
//
//			bool bSent = SendWriteCmd(slaveid, headaddress, (WORD)(fValue), funccode);
//			if(!bSent)//这里才是网络错误
//			{
//				PrintLog(L"ERROR: modbus write point send package failed.\r\n");
//				SetNetworkError();
//			}
//			return true;
//		}
//	}
//
//	return false;
//}

bool	CModbusTcpCtrl::SetValue(const wstring& pointname, double fValue)
{
	if(!m_bConnectOK)
		return false;

	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			fValue = entry.GetMultipleReadValue(fValue,false);			//edit by robert
			bool bSent = SendWriteCmd(entry,fValue);
			if(!bSent)//这里才是网络错误
			{
				PrintLog(L"ERROR: modbus write point send package failed.\r\n",false);
				CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
				SetNetworkError();
			}
			return true;
		}
	}

	return false;
}

bool CModbusTcpCtrl::GetValue( const wstring& pointname, double& result )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
			result =  entry.GetMultipleReadValue(entry.GetValue());
			return true;
		}
	}

	return false;
}

bool CModbusTcpCtrl::InitDataEntryValue(const wstring& pointname, double value)
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		DataPointEntry& entry = m_pointlist[i];
		if (entry.GetShortName() == pointname)
		{
			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
			entry.SetValue(value);
			return true;
		}
	}

	return false;
}

void CModbusTcpCtrl::AddLog( const wchar_t* loginfo )
{
	if (m_logsession){
		m_logsession->InsertLog(loginfo);
		Sleep(50);
	}
}

void CModbusTcpCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}

bool CModbusTcpCtrl::ReConnect()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	PrintLog(L"INFO : Modbus reconnecting...\r\n",false);
	Disconnect(); //tcpip recv thread stop, and clear socket

	if (!TcpIpConnectComm(m_host, m_port))
	{
		PrintLog(L"ERROR: Modbus reconnecting failed!\r\n");
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_RECONNECT_MODBUS);
		return false;
	}
	m_strErrInfo = "";
	PrintLog(L"INFO : Modbus reconnecting success!\r\n",false);
	return true;
}

UINT WINAPI CModbusTcpCtrl::ThreadCheckAndRetryConnection(LPVOID lparam)
{
	CModbusTcpCtrl* pthis = (CModbusTcpCtrl*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(pthis->m_nModbusNetworkErrorCount>=60 || !pthis->m_bConnectOK)
			{
				if(pthis->ReConnect())
					pthis->ClearNetworkError();
			}

			int nPollSleep = 30;
			while (!pthis->m_bExitThread)
			{
				if(nPollSleep <= 0)
				{
					break;
				}
				nPollSleep--;
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool CModbusTcpCtrl::Exit()
{
	m_bExitThread = true;
	Disconnect();		//关闭TCP

	if(m_hDataCheckRetryConnectionThread)
	{
		WaitForSingleObject(m_hDataCheckRetryConnectionThread,5000);
		CloseHandle(m_hDataCheckRetryConnectionThread);
		m_hDataCheckRetryConnectionThread = NULL;
	}

	if(m_hsendthread)
	{
		WaitForSingleObject(m_hsendthread, 5000);
		CloseHandle(m_hsendthread);
		m_hsendthread = NULL;
	}
	
	return true;
}

void CModbusTcpCtrl::SendReadCommandOneByOne()
{
	m_bSortPoint = true;
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode());
		if(!bSent)
		{
			SetNetworkError();
			CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_SEND_ONE_MODBUS);
		}
		Sleep(10);
	}

	//5秒
	Sleep(5000);

	m_backuppointlist.clear();
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		wstring strPointName = m_pointlist[i].GetShortName();
		map<wstring,DataPointEntry*>::iterator iter = m_mapErrPoint.find(strPointName);
		if(iter != m_mapErrPoint.end())
		{
			continue;
		}
		m_backuppointlist.push_back(m_pointlist[i]);
	}

	SortBackupPointByDevice();
	m_mapErrPoint.clear();
	m_bSortPoint = false;
}

void CModbusTcpCtrl::SortBackupPointByDevice()
{
	//按设备排序（先设备地址、后点地址）
	int i=0, j=0;
	for(i=0;(i+1)<m_backuppointlist.size();i++)
	{
		for(j=i+1; j<m_backuppointlist.size();j++)
		{
			DataPointEntry entryi = m_backuppointlist[i];
			DataPointEntry entryj = m_backuppointlist[j];
			int nDeviceAdd1 = _wtoi(entryi.GetParam(1).c_str());
			int nDeviceAdd2 = _wtoi(entryj.GetParam(1).c_str());

			if(nDeviceAdd1>nDeviceAdd2)
			{
				m_backuppointlist[i] = entryj;
				m_backuppointlist[j] = entryi;
			}
			else if(nDeviceAdd1==nDeviceAdd2)
			{
				int nPointAdd1 = _wtoi(entryi.GetParam(2).c_str());
				int nPointAdd2 = _wtoi(entryj.GetParam(2).c_str());
				if(nPointAdd1> nPointAdd2)
				{
					m_backuppointlist[i] = entryj;
					m_backuppointlist[j] = entryi;
				}
			}
		}
	}


	//生成读取序列
	m_ReadUnitList.clear();
	for(i=0;i<m_backuppointlist.size();i++)
	{
		_ModbusReadUnit munit;
		munit.strPointName = m_backuppointlist[i].GetShortName();
		munit.nSlaveId = _wtoi(m_backuppointlist[i].GetParam(1).c_str());
		munit.dwAddFrom = _wtoi(m_backuppointlist[i].GetParam(2).c_str());
		munit.dwAddTo = munit.dwAddFrom;
		munit.dwFuncCode = _wtoi(m_backuppointlist[i].GetParam(3).c_str());
		munit.nReadSuccessCount = 100;
		munit.bHasErrPoint = false;
		if(!CombineReadUnit(munit))
		{
			m_ReadUnitList.push_back(munit);
		}
	}
}

void CModbusTcpCtrl::SetBackupReadCommands()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	if(m_bSortPoint)		//重排点表 不发命令
		return;

	if(m_nSendCmdCount >= 100)			//发送100次后，若点表有误 则重排一次
	{
		m_nSendCmdCount = 0;
		if(m_pointlist.size() != m_backuppointlist.size())
		{
			SendReadCommandOneByOne();		//重排
			return;
		}
	}

	if(m_mapErrPoint.size()>0)
	{
		SendReadCommandOneByOne();		//重排
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	int i = 0;
	bool bConnectionExist = false; //连接是否正常
	//set command
	for(i=0;i<m_backuppointlist.size();i++)
	{
		m_backuppointlist[i].SetToUpdate();
	}

	for (i = 0; i < m_ReadUnitList.size(); i++)
	{
		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];

		int slaveid = mmUnit.nSlaveId;
		DWORD headaddressFrom = mmUnit.dwAddFrom;
		DWORD headaddressTo = mmUnit.dwAddTo;
		DWORD funccode = mmUnit.dwFuncCode;
		if (slaveid < 0 || headaddressFrom < 0 || headaddressTo<headaddressFrom || funccode == 0 || funccode > 17)
		{
			// this point entry is not valid.
			CString loginfo;
			loginfo.Format(L"ERROR: modbus entry %s params not valid, skip!\r\n", mmUnit.strPointName.c_str());
			PrintLog(loginfo.GetString(),false);
			CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_MODBUS_,ERROR_CUSTOM_MODBUS_INVALID);
			continue;
		}

		bool bSent = SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
		if(!bSent)
		{
			SetNetworkError();
			continue;
		}

		//等待回应的机制，是半双工网络
		//10秒
		int nWaitingCount = m_nRecevieTimeOut; 
		while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
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
				SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
				nWaitingCount = m_nRecevieTimeOut; 
				while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
				{
					Sleep(1);
					nWaitingCount--;
				}

				if(nWaitingCount<=0)
				{
					CString loginfo;
					loginfo.Format(L"ERROR: modbus slave(ADDR: %d) no response!\r\n", mmUnit.nSlaveId);
					PrintLog(loginfo.GetString(),false);
					CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
					mmUnit.nReadSuccessCount = 0; //读两次都失败，下次只读一次。
				}
			}
		}
		else
		{//只要有回应，就认为成功（部分数据回不来不代表网络出了问题）
			if(mmUnit.nReadSuccessCount<10000)
				mmUnit.nReadSuccessCount++;

			bConnectionExist = true;
		}

		Sleep(100); 

	}

	if(bConnectionExist)//只要读到成功过一个数据，那么就认为连接是正常的
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved, error %d times.\r\n", m_nModbusNetworkErrorCount);
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
	}

	m_nSendCmdCount++;
}

void CModbusTcpCtrl::SetReadOneByOneMode( bool bReadOneByOneMode )
{
	m_bReadOneByOneMode = bReadOneByOneMode;
}

void CModbusTcpCtrl::SendOneByOne()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode());
		if(!bSent)
		{
			SetNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

void CModbusTcpCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

void CModbusTcpCtrl::SortPointByDevice_()
{
	//先按类型和位数新建出点
	m_vecPointList.clear();
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		DataPointEntry entry = m_pointlist[i];
		CreateEntryByDataTypeAndRange(entry,m_vecPointList);
	}

	//按设备排序（先设备地址、后点地址）
	int i=0, j=0;
	for(i=0;(i+1)<m_vecPointList.size();i++)
	{
		for(j=i+1; j<m_vecPointList.size();j++)
		{
			DataPointEntry entryi = m_vecPointList[i];
			DataPointEntry entryj = m_vecPointList[j];
			int nDeviceAdd1 = _wtoi(entryi.GetParam(1).c_str());
			int nDeviceAdd2 = _wtoi(entryj.GetParam(1).c_str());

			if(nDeviceAdd1>nDeviceAdd2)
			{
				m_vecPointList[i] = entryj;
				m_vecPointList[j] = entryi;
			}
			else if(nDeviceAdd1==nDeviceAdd2)
			{
				int nFunCode1 = _wtoi(entryi.GetParam(3).c_str());
				int nFunCode2 = _wtoi(entryj.GetParam(3).c_str());
				if(nFunCode1>nFunCode2)
				{
					m_vecPointList[i] = entryj;
					m_vecPointList[j] = entryi;
				}
				else if(nFunCode1 == nFunCode2)
				{
					int nPointAdd1 = _wtoi(entryi.GetParam(2).c_str());
					int nPointAdd2 = _wtoi(entryj.GetParam(2).c_str());
					if(nPointAdd1> nPointAdd2)
					{
						m_vecPointList[i] = entryj;
						m_vecPointList[j] = entryi;
					}
				}
			}
		}
	}

	//生成读取序列
	for(i=0;i<m_vecPointList.size();i++)
	{
		_ModbusReadUnit munit;
		munit.strPointName = m_vecPointList[i].GetShortName();
		munit.nSlaveId = _wtoi(m_vecPointList[i].GetParam(1).c_str());
		munit.dwAddFrom = _wtoi(m_vecPointList[i].GetParam(2).c_str());
		munit.dwAddTo = munit.dwAddFrom;
		munit.dwFuncCode = _wtoi(m_vecPointList[i].GetParam(3).c_str());
		munit.nReadSuccessCount = 100;
		munit.bHasErrPoint = false;
		if(!CombineReadUnit(munit,m_vecPointList[i].GetModbusMutileEnd()))
		{
			m_ReadUnitList.push_back(munit);
		}
	}
}

void CModbusTcpCtrl::CreateEntryByDataTypeAndRange( DataPointEntry entry,vector<DataPointEntry>& vecModbus )
{
	if(entry.GetValueType() == E_MODBUS_BITE)			//Byte 0-15  合并成一个点位读取
	{
		bool bExist = false;
		for(int i=0; i<vecModbus.size(); ++i)
		{
			if(vecModbus[i].GetParam(1) == entry.GetParam(1) && vecModbus[i].GetParam(2) == entry.GetParam(2))
			{
				bExist = true;
				break;
			}
		}
		if(!bExist)
			vecModbus.push_back(entry);
	}
	else if(entry.GetValueType() == E_MODBUS_POWERLINK)			//只读前两位
	{
		int nDataRange = 2;
		for(int i=0; i<nDataRange; ++i)
		{
			DWORD nAddress = entry.GetHeadAddress();
			CString strNewParm2;
			strNewParm2.Format(_T("%d"),nAddress+i);
			DataPointEntry entryCopy = entry;
			entryCopy.SetParam(2,strNewParm2.GetString());
			if(i < nDataRange-1)
				entryCopy.SetModbusMutileEnd(false);
			vecModbus.push_back(entryCopy);
		}
	}
	else
	{
		int nDataRange =entry.GetValueRange();
		if(nDataRange <= 1)
		{
			vecModbus.push_back(entry);
		}
		else
		{
			for(int i=0; i<nDataRange; ++i)
			{
				DWORD nAddress = entry.GetHeadAddress();
				CString strNewParm2;
				strNewParm2.Format(_T("%d"),nAddress+i);
				DataPointEntry entryCopy = entry;
				entryCopy.SetParam(2,strNewParm2.GetString());
				if(i < nDataRange-1)
					entryCopy.SetModbusMutileEnd(false);
				vecModbus.push_back(entryCopy);
			}
		}
	}
}

void CModbusTcpCtrl::SetReadCommands_()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
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
		if(!m_bConnectOK)
			return;

		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];
		if(m_nRecvedSlave != mmUnit.nSlaveId)
		{
			Sleep(m_nIDInterval);			//两个ID之间切换时
		}

		if(mmUnit.bHasErrPoint == false || m_bDisSingleRead == true)
		{
			if(SendReadMutil_(mmUnit))
				bConnectionExist = true;
		}
		else
		{
			if(SendOneByOne_(mmUnit))
				bConnectionExist = true;
		}
	}

	if(bConnectionExist)//只要读到成功过一个数据，那么就认为连接是正常的
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved, error %d times.\r\n", m_nModbusNetworkErrorCount);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
		PrintLog(loginfo.GetString(),false);
	}
}

DataPointEntry* CModbusTcpCtrl::FindEntry_( DWORD slaveid, DWORD headaddress, DWORD funccode )
{
	for (unsigned int i = 0; i < m_vecPointList.size(); i++)
	{
		const DataPointEntry& entry = m_vecPointList[i];
		if(entry.GetSlaveID() == slaveid && entry.GetHeadAddress() == headaddress)
		{
			WORD wFunc = entry.GetFuncCode();
			if(wFunc==0x06 || wFunc==0x10 || wFunc==0x17)
				wFunc = 0x03;
			else if(wFunc==0x05)
				wFunc = 0x01;
			else if(wFunc==0x0F)
				wFunc = 0x01;

			if(funccode==0x06 || funccode==0x10 || funccode==0x17)
				funccode = 0x03;
			else if(funccode==0x05)
				funccode = 0x01;
			else if(wFunc==0x0F)
				funccode = 0x01;

			if(wFunc == funccode)
				return &m_vecPointList[i];
		}
	}

	return NULL;
}

int CModbusTcpCtrl::GetBitFromWord( WORD dwValue, int nIndex )
{
	WORD dwTemp = dwValue<<(16-nIndex-1);
	int nTemp = dwTemp>> 15;

	return nTemp;
}

DWORD CModbusTcpCtrl::MergeTwoWORD( WORD wGao,WORD wDi,bool bOrder /*= true*/ )
{
	if(!bOrder)
	{
		DWORD dwHourTotal;
		dwHourTotal = wGao;
		dwHourTotal = dwHourTotal<< 16;
		dwHourTotal = dwHourTotal | (wDi&0x0000FFFF);
		return dwHourTotal;
	}
	else
	{
		DWORD dwHourTotal;
		dwHourTotal = wDi;
		dwHourTotal = dwHourTotal<< 16;
		dwHourTotal = dwHourTotal | (wGao&0x0000FFFF);
		return dwHourTotal;
	}
}

void CModbusTcpCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,vector< pair<wstring, wstring> >& valuelist )
{	
	if(entry.GetValueType() == E_MODBUS_BITE)		//BYTE
	{
		vector<WORD> vecValue;
		bool bHasFault = false;
		for(int i=0; i<1; ++i)
		{
			DataPointEntry* entryFind = FindEntry_(entry.GetSlaveID(), entry.GetHeadAddress()+i, entry.GetFuncCode());
			if(entryFind)
			{
				if(entryFind->GetUpdateSignal()<10)
				{
					vecValue.push_back(entryFind->GetWORD());
				}
				else
				{
					bHasFault = true;
					break;
				}
			}
		}

		if(!bHasFault)
		{
			GetValueByDataTypeAndRange(entry,E_MODBUS_VALUE_TYPE(entry.GetValueType()),vecValue,valuelist);
		}
	}
	else if(entry.GetValueType() == E_MODBUS_POWERLINK)		//E_MODBUS_POWERLINK
	{
		int nDataRange = 2;
		vector<WORD> vecValue;
		bool bHasFault = false;
		nDataRange = (nDataRange<=0)?1:nDataRange;
		for(int i=0; i<nDataRange; ++i)
		{
			DataPointEntry* entryFind = FindEntry_(entry.GetSlaveID(), entry.GetHeadAddress()+i, entry.GetFuncCode());
			if(entryFind)
			{
				if(entryFind->GetUpdateSignal()<10)
				{
					vecValue.push_back(entryFind->GetWORD());
				}
				else
				{
					bHasFault = true;
					break;
				}
			}
		}

		if(!bHasFault)
		{
			GetValueByDataTypeAndRange(entry,E_MODBUS_VALUE_TYPE(entry.GetValueType()),vecValue,valuelist);
		}
	}
	else
	{
		int nDataRange = entry.GetValueRange();
		vector<WORD> vecValue;
		bool bHasFault = false;
		nDataRange = (nDataRange<=0)?1:nDataRange;
		int nAddress = 0;
		for(int i=0; i<nDataRange; ++i)
		{
			DataPointEntry* entryFind = FindEntry_(entry.GetSlaveID(), entry.GetHeadAddress()+i, entry.GetFuncCode());
			if(entryFind)
			{
				if(entryFind->GetUpdateSignal()<10)
				{
					vecValue.push_back(entryFind->GetWORD());
				}
				else
				{
					nAddress = entry.GetHeadAddress()+i;
					bHasFault = true;
					break;
				}
			}
		}

		if(!bHasFault)
		{
			GetValueByDataTypeAndRange(entry,E_MODBUS_VALUE_TYPE(entry.GetValueType()),vecValue,valuelist);
		}
		else
		{
			
		}
	}
}

void CModbusTcpCtrl::GetValueByDataTypeAndRange(DataPointEntry entry, E_MODBUS_VALUE_TYPE type,vector<WORD>& vecValue,vector< pair<wstring, wstring> >& valuelist )
{
	if(vecValue.size() <= 0)
		return;

	switch(type)
	{
	case E_MODBUS_SIGNED:
		{
			WORD wValue = vecValue[0];
			int nFlag = (wValue>>15) & 1 ;			//第一位是否为1 若为1则为负数
			int nValue = wValue;
			if(nFlag >0 )		//负数
			{
				nValue = nValue-65536;
			}
			double dval = entry.GetMultipleReadValue((double)nValue);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_UNSIGNED:
		{
			WORD wValue = vecValue[0];
			int nValue = wValue;
			double dval = entry.GetMultipleReadValue((double)nValue);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_BITE:
		{
			WORD wValue = vecValue[0];
			int nDataRange = entry.GetValueRange();
			int nFlag = GetBitFromWord(wValue,nDataRange);
			double dval = entry.GetMultipleReadValue((double)nFlag);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_LONG:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0],false);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.lVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_LONG_INVERSE:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0]);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue((double)vt.lVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_FLOAT:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0],false);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue(vt.fltVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_FLOAT_INVERSE:
		{
			if(vecValue.size() < 2)
				break;
			DWORD dwValue = MergeTwoWORD(vecValue[1],vecValue[0]);
			VARIANT vt;
			vt.ulVal = dwValue;
			double dval = entry.GetMultipleReadValue(vt.fltVal);
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_DOUBLE:
		{
			if(vecValue.size() < 4)
				break;
			double dval = entry.GetMultipleReadValue(MergeFourDouble(vecValue[3],vecValue[2],vecValue[1],vecValue[0],false));
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_DOUBLE_INVERSE:
		{
			if(vecValue.size() < 4)
				break;
			double dval = entry.GetMultipleReadValue(MergeFourDouble(vecValue[3],vecValue[2],vecValue[1],vecValue[0]));
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_STRING:
		{
			char cWarnig[256] = {0};
			for(int i=0; i<vecValue.size(); ++i)
			{
				memcpy(cWarnig+2*i,&vecValue[i],2);
			}
			wstring strValueSet = Project::Tools::AnsiToWideChar(cWarnig);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_STRING_INVERSE:
		{
			char cWarnig[256] = {0};
			BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序
			for(int i=0; i<vecValue.size(); ++i)
			{
				memcpy(byteToEx, &vecValue[i], 2);
				memcpy(cWarnig+2*i,&byteToEx[1],1);
				memcpy(cWarnig+2*i+1,&byteToEx[0],1);
			}
			wstring strValueSet = Project::Tools::AnsiToWideChar(cWarnig);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	case E_MODBUS_POWERLINK:
		{
			if(vecValue.size() < 2)
				break;
			double dval = entry.GetMultipleReadValue((double)MergeTwoWORD(vecValue[1],vecValue[0],false));
			wstring strValueSet = Project::Tools::RemoveDecimalW(dval,m_nPrecision);
			valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
		}
		break;
	default:
		break;
	}
}

void CModbusTcpCtrl::GetValueByDataTypeAndRange( DataPointEntry entry,double dValue,vector<WORD>& vecCmd ,bool bOrder)
{
	vecCmd.clear();
	switch(entry.GetValueType())
	{
	case E_MODBUS_SIGNED:			//2016-03-18 测试
		{
			VARIANT vt;
			vt.lVal = dValue;
			WORD dwValue = vt.uintVal;
			vecCmd.push_back(dwValue);
		}
		break;
	case E_MODBUS_UNSIGNED:			//2016-03-18 测试
		{
			VARIANT vt;
			vt.lVal = dValue;
			WORD dwValue = vt.uintVal;
			vecCmd.push_back(dwValue);
		}
		break;
	case E_MODBUS_LONG:
		{
			VARIANT vt;
			vt.lVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_FLOAT:
		{
			VARIANT vt;
			vt.fltVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_FLOAT_INVERSE:
		{
			VARIANT vt;
			vt.fltVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(!bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_LONG_INVERSE:
		{
			VARIANT vt;
			vt.lVal = dValue;
			DWORD dwValue = vt.ulVal;
			WORD wLow = LOWORD(dwValue);
			WORD wHigh = HIWORD(dwValue);
			if(!bOrder)
			{
				vecCmd.push_back(wHigh);
				vecCmd.push_back(wLow);
			}
			else
			{
				vecCmd.push_back(wLow);
				vecCmd.push_back(wHigh);
			}
		}
		break;
	case E_MODBUS_DOUBLE:
		{
			VARIANT vt;
			vt.dblVal = dValue;
			ULONGLONG ullVal = vt.ullVal;
			DWORD dwHigh =  ((DWORD)((((LONG64)(ullVal)) >> 32) & 0xffffffff));
			WORD wHLow = LOWORD(dwHigh);
			WORD wHHigh = HIWORD(dwHigh);
			DWORD dwLow =  ((DWORD)(((LONG64)(ullVal)) & 0xffffffff));
			WORD wLLow = LOWORD(dwLow);
			WORD wLHigh = HIWORD(dwLow);
			if(bOrder)
			{
				vecCmd.push_back(wHHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wLLow);
			}
			else
			{
				vecCmd.push_back(wLLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wHHigh);
			}
		}
		break;
	case E_MODBUS_DOUBLE_INVERSE:
		{
			VARIANT vt;
			vt.dblVal = dValue;
			ULONGLONG ullVal = vt.ullVal;
			DWORD dwHigh =  ((DWORD)((((LONG64)(ullVal)) >> 32) & 0xffffffff));
			WORD wHLow = LOWORD(dwHigh);
			WORD wHHigh = HIWORD(dwHigh);
			DWORD dwLow =  ((DWORD)(((LONG64)(ullVal)) & 0xffffffff));
			WORD wLLow = LOWORD(dwLow);
			WORD wLHigh = HIWORD(dwLow);
			if(!bOrder)
			{
				vecCmd.push_back(wHHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wLLow);
			}
			else
			{
				vecCmd.push_back(wLLow);
				vecCmd.push_back(wLHigh);
				vecCmd.push_back(wHLow);
				vecCmd.push_back(wHHigh);
			}
		}
		break;
	case E_MODBUS_POWERLINK:			//double值 整数部分为 设置 小数部分为功能码  2位小数
		{
			DWORD nInt = dValue;
			CString strValue;
			strValue.Format(_T("%.2f"),dValue);
			WORD nDecimal = _wtof(strValue)*100-nInt*100;
			WORD wHLow = LOWORD(nInt);
			WORD wHHigh = HIWORD(nInt);
			vecCmd.push_back(wHLow);
			vecCmd.push_back(wHHigh);
			vecCmd.push_back(nDecimal);
		}
		break;
	default:
		break;
	}
}

double CModbusTcpCtrl::MergeFourDouble( WORD wGaoGao,WORD wGao,WORD wDi,WORD wDiDi,bool bOrder /*= true*/ )
{
	if(!bOrder)
	{
		wstring strValue;
		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDiDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGaoGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		return strtodbl(strValue.c_str());
	}
	else
	{
		wstring strValue;
		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGaoGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wGao,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		for(int i=0; i<16; ++i)
		{
			if(GetBitFromWord(wDiDi,i) == 1)
				strValue = L"1"+ strValue;
			else
				strValue = L"0"+ strValue;
		}

		return strtodbl(strValue.c_str());
	}
}

double CModbusTcpCtrl::strtodbl( const wchar_t * str )
{
	long long dbl = 0;  
	for(int i=0;i<63;i++)
	{  
		dbl += (str[i]-'0');  
		dbl <<= 1;  
	}  
	dbl +=(str[63]-'0');  
	double* db = (double*)&dbl;  
	return *db;  
}

bool CModbusTcpCtrl::SendWriteCmd( DataPointEntry entry,double fValue )
{
	Project::Tools::Scoped_Lock<Mutex> guardlock(m_lock);
	if(entry.GetValueType() == E_MODBUS_BITE)
	{
		return SendWriteBYTECmd(entry,fValue);
	}
	else
	{
		if(entry.GetValueRange() == 0 || entry.GetValueRange() == 1)				//xie
		{
			//调用原来的
			return SendWriteWORDCmd(entry,fValue);
		}
		else if(entry.GetValueRange() > 1)
		{
			return SendWriteMutilCmd(entry,fValue);
		}
	}
	return false;
}

bool CModbusTcpCtrl::SendWriteMutilCmd( DataPointEntry entry,double fValue )
{
	DWORD nSlaveId = entry.GetSlaveID();
	DWORD wAddress = entry.GetHeadAddress();
	DWORD wFunc = entry.GetFuncCode();
	if (nSlaveId == 0 || wAddress == 0 || wFunc == 0)
	{
		PrintLog(L"ERROR: modbus write point params invalid!\r\n",false);
		CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
		return false;
	}

	if (wFunc == 03){
		wFunc = 0x10;
	}

	vector<WORD> vecCmd;
	GetValueByDataTypeAndRange(entry,fValue,vecCmd,false);

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = entry.GetValueRange();
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序
	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	if(SendPackage(cSendBuffer, nCount+1))
	{
		//发送成功更新值
		for(int i=0; i<vecCmd.size(); ++i)
		{
			WORD value = vecCmd[i];
			DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
			if (entry)
			{
				entry->SetWORD(value);
			}
		}
	}
	return false;
}

void CModbusTcpCtrl::SendOneByOne_()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode()){
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;
	
	SumReadAndResponse();
	for(int i=0; i<m_vecPointList.size(); ++i)
	{
		DataPointEntry entry = m_vecPointList[i];
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode());
		if(!bSent)
		{
			SetNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

bool CModbusTcpCtrl::SendOneByOne_( _ModbusReadUnit& unit )
{
	unit.bHasErrPoint = false;
	unit.bMultiRead = false;
	bool bConnectionExist = false;
	for(int i=unit.dwAddFrom; i<= unit.dwAddTo; ++i)
	{
		if(!m_bConnectOK)
			return false;

		bool bSent = SendReadCmd(unit.nSlaveId,i,i,unit.dwFuncCode);
		if(!bSent)
		{
			SetNetworkError();
		}
		else
		{
			bConnectionExist = true;
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
	return bConnectionExist;
}

bool CModbusTcpCtrl::MutilReadHasErr()
{
	if(m_mapErrPoint.size() <=0)
	{
		return false;
	}
	m_mapErrPoint.clear();
	return true;
}

void CModbusTcpCtrl::SetModbusReadUnitErrFlag( unsigned int nSlaveId,DWORD dwFuncCode,DWORD dwAdd,bool bHasErrPoint/*=true*/ )
{
	for(int i=0; i<m_ReadUnitList.size(); ++i)
	{
		if(m_ReadUnitList[i].nSlaveId==nSlaveId && m_ReadUnitList[i].dwFuncCode==dwFuncCode)
		{
			if(m_ReadUnitList[i].dwAddFrom <= dwAdd && dwAdd <= m_ReadUnitList[i].dwAddTo)
			{
				if(m_ReadUnitList[i].bMultiRead)
					m_ReadUnitList[i].bHasErrPoint = bHasErrPoint;
				else
					m_ReadUnitList[i].bHasErrPoint = false;
				return;
			}
		}
	}
}

//bool CModbusTcpCtrl::SendReadMutil_( _ModbusReadUnit& mmUnit )
//{
//	mmUnit.bHasErrPoint = false;
//	mmUnit.bMultiRead = true;
//	int slaveid = mmUnit.nSlaveId;
//	DWORD headaddressFrom = mmUnit.dwAddFrom;
//	DWORD headaddressTo = mmUnit.dwAddTo;
//	DWORD funccode = mmUnit.dwFuncCode;
//	if (slaveid < 0 || headaddressFrom < 0 || headaddressTo<headaddressFrom || funccode == 0 || funccode > 17)
//	{
//		// this point entry is not valid.
//		CString loginfo;
//		loginfo.Format(L"ERROR: modbus entry %s params not valid, skip!\r\n", mmUnit.strPointName.c_str());
//		PrintLog(loginfo.GetString());
//		CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_MODBUS_,ERROR_CUSTOM_MODBUS_INVALID);
//		return false;
//	}
//
//	bool bSent = SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
//	if(!bSent)
//	{
//		SetNetworkError();
//		//找到所有点
//		for(int i= mmUnit.dwAddFrom; i<mmUnit.dwAddTo; ++i)
//		{
//			DataPointEntry* entry = FindEntry_(mmUnit.nSlaveId, i, mmUnit.dwFuncCode);
//			if(entry)
//			{		
//				CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_SEND_MODBUS);
//			}
//		}
//		CString loginfo;
//		loginfo.Format(L"ERROR: modbus entry %s send fail.\r\n", mmUnit.strPointName.c_str());
//		PrintLog(loginfo.GetString());
//		return false;
//	}
//
//	//等待回应的机制，是半双工网络
//	//10秒
//	int nWaitingCount = m_nRecevieTimeOut; 
//	while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
//	{
//		Sleep(1);
//		nWaitingCount--;
//	}
//	//等待不到回应，如果之前有过回应，那么重试一次！
//	if(nWaitingCount<=0)
//	{
//		if(mmUnit.nReadSuccessCount>0)
//		{
//			//曾经读成功过的，就再发一次
//			SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
//			nWaitingCount = m_nRecevieTimeOut; 
//			while(m_nRecvedSlave!= mmUnit.nSlaveId && nWaitingCount>0)
//			{
//				Sleep(1);
//				nWaitingCount--;
//			}
//
//			if(nWaitingCount<=0)
//			{
//				CString loginfo;
//				loginfo.Format(L"ERROR: modbus slave(ADDR: %d) no response!\r\n", mmUnit.nSlaveId);
//				PrintLog(loginfo.GetString());
//				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
//				mmUnit.nReadSuccessCount = 0; //读两次都失败，下次只读一次。
//				mmUnit.bHasErrPoint = true;
//			}
//		}
//	}
//	else
//	{//只要有回应，就认为成功（部分数据回不来不代表网络出了问题）
//		if(mmUnit.nReadSuccessCount<10000)
//			mmUnit.nReadSuccessCount++;
//		Sleep(m_nSendReadCmdTimeInterval); 
//		return true;
//	}
//
//	Sleep(m_nSendReadCmdTimeInterval); 
//	return false;
//}

bool CModbusTcpCtrl::SendReadMutil_( _ModbusReadUnit& mmUnit )
{
	mmUnit.bHasErrPoint = false;
	mmUnit.bMultiRead = true;
	int slaveid = mmUnit.nSlaveId;
	DWORD headaddressFrom = mmUnit.dwAddFrom;
	DWORD headaddressTo = mmUnit.dwAddTo;
	DWORD funccode = mmUnit.dwFuncCode;
	if (slaveid < 0 || headaddressFrom < 0 || headaddressTo<headaddressFrom || funccode == 0 || funccode > 17)
	{
		// this point entry is not valid.
		CString loginfo;
		loginfo.Format(L"ERROR: modbus entry %s params not valid, skip!\r\n", mmUnit.strPointName.c_str());
		PrintLog(loginfo.GetString(),false);
		CDebugLog::GetInstance()->SetErrPoint(mmUnit.strPointName,POINTTYPE_MODBUS_,ERROR_CUSTOM_MODBUS_INVALID);
		return false;
	}

	bool bSent = SendReadCmd(mmUnit.nSlaveId, mmUnit.dwAddFrom, mmUnit.dwAddTo, mmUnit.dwFuncCode);
	if(!bSent)
	{
		SetNetworkError();
		//找到所有点
		for(int i= mmUnit.dwAddFrom; i<mmUnit.dwAddTo; ++i)
		{
			DataPointEntry* entry = FindEntry_(mmUnit.nSlaveId, i, mmUnit.dwFuncCode);
			if(entry)
			{		
				CDebugLog::GetInstance()->SetErrPoint(*entry,ERROR_CUSTOM_SEND_MODBUS);
			}
		}
		CString loginfo;
		loginfo.Format(L"ERROR: modbus entry %s send fail.\r\n", mmUnit.strPointName.c_str());
		PrintLog(loginfo.GetString(),false);
		return false;
	}
	return bSent;
}

unsigned short CModbusTcpCtrl::CalcCrcFast( unsigned char*puchMsg,unsigned short usDataLen )
{
	unsigned char uchCRCHi=0xFF ;
	unsigned char uchCRCLo=0xFF ;
	unsigned short uIndex ;

	while(usDataLen--)
	{
		uIndex=uchCRCHi^*puchMsg++;
		uchCRCHi=uchCRCLo^m_auchCRCHi[uIndex];
		uchCRCLo=m_auchCRCLo[uIndex];
	}
	return(uchCRCHi<<8|uchCRCLo);
}

bool CModbusTcpCtrl::SendWriteBYTECmd( DataPointEntry entry,double fValue )
{
	//调用原来的
	DWORD slaveid = entry.GetSlaveID();
	DWORD headaddress = entry.GetHeadAddress();
	DWORD funccode = entry.GetFuncCode();
	if (slaveid == 0 || headaddress == 0 || funccode == 0)
	{
		PrintLog(L"ERROR: modbus write point params invalid!\r\n",false);
		CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
		return false;
	}

	//golding: 注意下面这个陷阱, modbus写值可能会出问题
	if (funccode == 03){
		//防止老项目配置没有配置对。做的折中方案。
		funccode = 06;
	}

	//找到该点值
	DataPointEntry* entry_ = FindEntry_(slaveid, headaddress, 6);
	if (entry_)
	{
		WORD wValue = entry_->GetWORD();
		wValue = SetBitToWord(wValue,entry.GetValueRange(),(int)fValue);
		bool bSent = SendWriteCmd(slaveid, headaddress,wValue, funccode);
		if(bSent)
		{
			entry_->SetWORD(wValue);
		}
		return bSent;
	}
	return false;
}

bool CModbusTcpCtrl::SendWriteWORDCmd( DataPointEntry entry,double fValue )
{
	//调用原来的
	DWORD slaveid = entry.GetSlaveID();
	DWORD headaddress = entry.GetHeadAddress();
	DWORD funccode = entry.GetFuncCode();
	if (slaveid == 0 || headaddress == 0 || funccode == 0)
	{
		PrintLog(L"ERROR: modbus write point params invalid!\r\n",false);
		CDebugLog::GetInstance()->SetErrPoint(entry,ERROR_CUSTOM_WRITE_MODBUS_VALID);
		return false;
	}

	//golding: 注意下面这个陷阱, modbus写值可能会出问题
	if (funccode == 03){
		//防止老项目配置没有配置对。做的折中方案。
		funccode = 06;
	}

	bool bSent = SendWriteCmd(slaveid, headaddress, (WORD)(fValue), funccode);
	if(bSent)
	{
		DataPointEntry* entry_ = FindEntry_(slaveid, headaddress, 6);
		if (entry_)
		{
			entry_->SetWORD((WORD)(fValue));
		}
	}
	return bSent;
}

WORD CModbusTcpCtrl::SetBitToWord( WORD nData, int nIndex ,int nValue)
{
	nData &= ~(1<<nIndex); //将*num的第pos位设置为0
	nData |= nValue<<nIndex;
	return nData;
}

bool CModbusTcpCtrl::SendTestWORDCmd( )
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序
	
	vector<WORD> vecCmd;
	WORD wwValue = 0x01FE;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	return SendPackage(cSendBuffer, nCount+1);
}

bool CModbusTcpCtrl::SendTestCloseWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序

	vector<WORD> vecCmd;
	WORD wwValue = 0x02FD;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	return SendPackage(cSendBuffer, nCount+1);
}

bool CModbusTcpCtrl::SendTestHornResetWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序

	vector<WORD> vecCmd;
	WORD wwValue = 0x04FB;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	return SendPackage(cSendBuffer, nCount+1);
}

bool CModbusTcpCtrl::SendTestFaultResetWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序

	vector<WORD> vecCmd;
	WORD wwValue = 0x08F7;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0001;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	return SendPackage(cSendBuffer, nCount+1);
}

bool CModbusTcpCtrl::SendTestOPenCloseWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序

	vector<WORD> vecCmd;
	WORD wwValue = 0x11EE;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0002;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	return SendPackage(cSendBuffer, nCount+1);
}

bool CModbusTcpCtrl::SendTestCloseOPenWORDCmd()
{
	DWORD nSlaveId = 1;
	DWORD wAddress = 6359;
	DWORD wFunc = 3;

	if (wFunc == 03){
		wFunc = 0x10;
	}

	char  cSendBuffer[256] = {0};
	ZeroMemory(cSendBuffer, sizeof(cSendBuffer));
	memcpy(cSendBuffer, &wAddress, sizeof(WORD));

	//////////////////////////////////////////////////////////////////////////
	cSendBuffer[6] = (BYTE)nSlaveId;
	cSendBuffer[7] = (BYTE)wFunc;
	cSendBuffer[8] = ((wAddress-1)&0xFF00)>>8;
	cSendBuffer[9] = ((wAddress-1)&0x00FF);
	WORD wNum = 3;
	WORD wRange = wNum*2;
	cSendBuffer[10] = (wNum&0xFF00)>>8;
	cSendBuffer[11] = (wNum&0x00FF);
	cSendBuffer[12] = (BYTE)wRange;
	BYTE byteToEx[2] = {0x00, 0x00};	//内存顺序

	vector<WORD> vecCmd;
	WORD wwValue = 0x11EE;
	vecCmd.push_back(wwValue);
	wwValue = 0x0000;
	vecCmd.push_back(wwValue);
	wwValue = 0x0002;
	vecCmd.push_back(wwValue);

	int nCount = 13;
	for(int i=0; i<vecCmd.size(); ++i)
	{
		WORD value = vecCmd[i];
		DataPointEntry* entry = FindEntry_(nSlaveId, wAddress+i, 16);
		if (entry)
		{
			entry->SetWORD(value);
		}
		memcpy(byteToEx, &value, 2);
		nCount = 13+2*i;
		cSendBuffer[nCount] =  byteToEx[1];
		nCount++;
		cSendBuffer[nCount] = byteToEx[0];
	}

	WORD wCount = nCount-5;
	cSendBuffer[5] = (BYTE)wCount;
	return SendPackage(cSendBuffer, nCount+1);
}

bool CModbusTcpCtrl::OutPutModbusReceive( const unsigned char* pRevData, DWORD length )
{
	if(m_nOutPut>0)
	{
		try
		{
			wstring strPath;
			GetSysPath(strPath);
			strPath += L"\\log";
			if(Project::Tools::FindOrCreateFolder(strPath))
			{
				COleDateTime oleNow = COleDateTime::GetCurrentTime();
				CString strLogPath;
				strLogPath.Format(_T("%s\\modbus_debug_%d_%02d_%02d.log"),strPath.c_str(),oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());

				CString strLog;
				std::ostringstream sqlstream;
				sqlstream << oleNow.GetHour() << ":" << oleNow.GetMinute() << ":" << oleNow.GetSecond() << " [" << charToHexStr((unsigned char*)pRevData,length) << "]\n";
				strLog = Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str();
				char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
				setlocale( LC_ALL, "chs" );
				//记录Log
				CStdioFile	ff;
				if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
				{
					ff.Seek(0,CFile::end);
					ff.WriteString(strLog);
					ff.Close();
				}
				setlocale( LC_CTYPE, old_locale ); 
				free( old_locale ); 	
			}
		}
		catch (CMemoryException* e)
		{

		}
		catch (CFileException* e)
		{
		}
		catch (CException* e)
		{
		}
		return true;
	}
	return false;
}

bool CModbusTcpCtrl::SetDebug( int nDebug )
{
	m_nOutPut = nDebug;
	return true;
}

string* CModbusTcpCtrl::byteToHexStr( unsigned char* byte_arr, int arr_len )
{
	string* hexstr=new string();  
	for (int i=0;i<arr_len;i++)  
	{  
		char hex1;  
		char hex2;  
		/*借助C++支持的unsigned和int的强制转换，把unsigned char赋值给int的值，那么系统就会自动完成强制转换*/  
		int value=byte_arr[i];  
		int S=value/16;  
		int Y=value % 16;  
		//将C++中unsigned char和int的强制转换得到的商转成字母  
		if (S>=0&&S<=9)  
			hex1=(char)(48+S);  
		else  
			hex1=(char)(55+S);  
		//将C++中unsigned char和int的强制转换得到的余数转成字母  
		if (Y>=0&&Y<=9)  
			hex2=(char)(48+Y);  
		else  
			hex2=(char)(55+Y);  
		//最后一步的代码实现，将所得到的两个字母连接成字符串达到目的  
		*hexstr=*hexstr+hex1+hex2;  
	}  
	return hexstr;  
}

string CModbusTcpCtrl::charToHexStr( unsigned char* byte_arr, int arr_len )
{
	std::ostringstream sqlstream;
	for (int i=0;i<arr_len;i++)  
	{  
		char hex1;  
		char hex2;  
		/*借助C++支持的unsigned和int的强制转换，把unsigned char赋值给int的值，那么系统就会自动完成强制转换*/  
		int value=byte_arr[i];  
		int S=value/16;  
		int Y=value % 16;  
		//将C++中unsigned char和int的强制转换得到的商转成字母  
		if (S>=0&&S<=9)  
			hex1=(char)(48+S);  
		else  
			hex1=(char)(55+S);  
		//将C++中unsigned char和int的强制转换得到的余数转成字母  
		if (Y>=0&&Y<=9)  
			hex2=(char)(48+Y);  
		else  
			hex2=(char)(55+Y);  
		//最后一步的代码实现，将所得到的两个字母连接成字符串达到目的  
		sqlstream << hex1 << hex2 << " ";
	}  
	return sqlstream.str().c_str();  
}

int CModbusTcpCtrl::GetMultiReadCount( DWORD slaveid, DWORD headaddress, DWORD funccode )
{
	for(int i=0; i<m_ReadUnitList.size(); ++i)
	{
		if(m_ReadUnitList[i].nSlaveId == slaveid && m_ReadUnitList[i].dwFuncCode == funccode)
		{
			if(m_ReadUnitList[i].dwAddFrom <= headaddress && m_ReadUnitList[i].dwAddTo >= headaddress)
				return m_ReadUnitList[i].dwAddTo-m_ReadUnitList[i].dwAddFrom+1;
		}
	}
	return 0;
}

void CModbusTcpCtrl::SumReadAndResponse()
{
	if(m_nCmdCount > 0)			//统计信息
	{
		std::ostringstream sqlstream;
		m_nResponseCount = (m_nResponseCount>m_nCmdCount)?m_nUpdatePointCount:m_nResponseCount;
		sqlstream << "Read(" << m_nCmdCount << ") Response(" << m_nResponseCount << ") UpdatePoint(" << m_nUpdatePointCount << ")";
		m_strUpdateLog = sqlstream.str();

		CString strOut;
		strOut.Format(_T("INFO : Modbus TCP(%d):%s.\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str()).c_str());
		PrintLog(strOut.GetString(),false);
	}
	m_nCmdCount = 0;
	m_nResponseCount = 0;
	m_nUpdatePointCount = 0;
}

void CModbusTcpCtrl::SendReadCommandsByActive()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode())
	{
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	bool bConnectionExist = false; //连接是否正常
	SumReadAndResponse();
	for(int i=0;i<m_vecPointList.size();i++)
	{
		m_vecPointList[i].SetToUpdate();
	}

	for (int i = 0; i < m_ReadUnitList.size(); i++)
	{
		if(!m_bConnectOK)
			return;

		_ModbusReadUnit & mmUnit = m_ReadUnitList[i];
		if(m_lastReadUnit.nSlaveId != mmUnit.nSlaveId)
		{
			Sleep(m_nIDInterval);			//两个ID之间切换时
		}

		if(mmUnit.bHasErrPoint == false  || m_bDisSingleRead == true)
		{
			m_lastReadUnit = mmUnit;
			if(SendReadMutil_(mmUnit))
			{
				//使用激活模式
				DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
				if(dwObject == WAIT_TIMEOUT)		// 超时
				{
					CString loginfo;
					loginfo.Format(L"ERROR in SendReadCommandsByActive: Modbus TimeOut(IP:%s:%d ID:%d Fun:%02d From:%d To:%d) Timeout:%d!\r\n", 
						Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),
						m_port,m_lastReadUnit.nSlaveId,m_lastReadUnit.dwFuncCode,m_lastReadUnit.dwAddFrom, m_lastReadUnit.dwAddTo,m_nRecevieTimeOut);
					PrintLog(loginfo.GetString(),false);
					CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
					m_lastReadUnit.nReadSuccessCount = 0; //读两次都失败，下次只读一次。
					m_lastReadUnit.bHasErrPoint = true;
				}
				else if(dwObject == WAIT_OBJECT_0)
				{
					bConnectionExist = true;
				}
			}
		}
		else
		{
			if(SendOneByOneByActive(mmUnit))
				bConnectionExist = true;
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}

	if(bConnectionExist)//只要读到成功过一个数据，那么就认为连接是正常的
		ClearNetworkError();
	else
	{
		SetNetworkError();
		CString loginfo;
		loginfo.Format(L"ERROR: Read modbus data Un-Recved(IP:%s:%d).\r\n",Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_UN_REC);
		PrintLog(loginfo.GetString(),false);
	}
}

void CModbusTcpCtrl::SetSendCmdEvent()
{
	SetEvent(m_sendCmdEvent);
}

HANDLE CModbusTcpCtrl::GetSendCmdEvent()
{
	return m_sendCmdEvent;
}

void CModbusTcpCtrl::SendOneByOneByActive()
{
	if (!CNetworkComm::GetInstance()->IsMainServerRunningMode())
	{
		return;
	}

	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_connection);

	if(!m_bConnectOK)
		return;

	SumReadAndResponse();
	for(int i=0; i<m_vecPointList.size(); ++i)
	{
		DataPointEntry entry = m_vecPointList[i];
		m_vecPointList[i].SetToUpdate();
		bool bSent = SendReadCmd(entry.GetSlaveID(),entry.GetHeadAddress(),entry.GetHeadAddress(),entry.GetFuncCode());
		if(!bSent)
		{
			SetNetworkError();
		}
		else
		{
			//使用激活模式
			DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
			if(dwObject == WAIT_TIMEOUT)		// 超时
			{
				CString loginfo;
				loginfo.Format(L"ERROR in SendOneByOneByActive: Modbus TimeOut(IP:%s:%d ID:%d Fun:%02d From:%d To:%d) Timeout:%d!\r\n",
					Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port,m_lastReadEntry.GetSlaveID(),
					m_lastReadEntry.GetFuncCode(),m_lastReadEntry.GetHeadAddress(),m_lastReadEntry.GetHeadAddress(), m_nRecevieTimeOut);
				PrintLog(loginfo.GetString(),false);
				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
			}

			ClearNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
}

bool CModbusTcpCtrl::SendOneByOneByActive( _ModbusReadUnit& unit )
{
	unit.bHasErrPoint = false;
	unit.bMultiRead = false;
	bool bConnectionExist = false;

	for(int i=unit.dwAddFrom; i<= unit.dwAddTo; ++i)
	{
		if(!m_bConnectOK)
			return false;

		m_lastReadUnit = unit;
		bool bSent = SendReadCmd(unit.nSlaveId,i,i,unit.dwFuncCode);
		if(!bSent)
		{
			SetNetworkError();
		}
		else
		{
			//使用激活模式
			DWORD dwObject = WaitForSingleObject(GetSendCmdEvent(), m_nRecevieTimeOut);
			if(dwObject == WAIT_TIMEOUT)		// 超时
			{
				CString loginfo;
				loginfo.Format(L"ERROR in SendOneByOneByActive: Modbus TimeOut(IP:%s:%d ID:%d Fun:%02d From:%d To:%d) Timeout:%d!\r\n", 
					Project::Tools::AnsiToWideChar(m_host.c_str()).c_str(),m_port,m_lastReadEntry.GetSlaveID(),m_lastReadEntry.GetFuncCode(),
					m_lastReadEntry.GetHeadAddress(),m_lastReadEntry.GetHeadAddress(), m_nRecevieTimeOut);
				PrintLog(loginfo.GetString(),false);
				CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_MODBUS_NO_RESPONSE);
			}

			bConnectionExist = true;
			ClearNetworkError();
		}
		Sleep(m_nSendReadCmdTimeInterval);
	}
	return bConnectionExist;
}

void CModbusTcpCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
