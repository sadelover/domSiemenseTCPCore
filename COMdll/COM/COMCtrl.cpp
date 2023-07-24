#pragma once

#include "StdAfx.h"
#include "COMCtrl.h"


#include <vector>
using std::vector;

CCOMCtrl::CCOMCtrl(void)
{
	m_pSendData = NULL;
	InitializeCriticalSection(&m_CriticalSection);
}

CCOMCtrl::~CCOMCtrl(void)
{
	ClosePort();
	DeleteCriticalSection(&m_CriticalSection);
}

//static long rxdatacount=0;  //该变量用于接收字符计数
bool CCOMCtrl::OnCommunication(const unsigned char* pRevData, DWORD length)
{
	//WORD wHeadAddr=-1, wValue=-1;

	//const BYTE *pData = (BYTE*)pRevData;
	//if ( (NULL ==pData)
	//	|| (!Is_DataValid((BYTE *)pData, (WORD)length)) ){
	//		return 1;
	//}

 //   const UINT devIdx = *pData;
	//hash_map<unsigned int, gDeviceRevData>::iterator itDev = m_mapDeviceRevData.find(devIdx);
	//if (itDev != m_mapDeviceRevData.end())
	//{
	//	/*************************ZHW ADD 2011-04-01*************************/
	//	wHeadAddr = (*itDev).second.startaddress;
	//	/********************************************************************/

	//	for (int nIndex=0; nIndex<(WORD)*(pData+2)/2; nIndex++)
	//	{
	//		hash_map<WORD, gItemData>::iterator it = (*itDev).second.mapItemData.find(wHeadAddr);
	//		if (it != (*itDev).second.mapItemData.end()
	//			&& ((*it).second.wFunc == pData[1]))
	//		{
	//			hash_map<wstring, gData>::iterator itDt = (*itDev).second.mapData.find((*it).second.strPlcParam);
	//			if (itDt != (*itDev).second.mapData.end())
	//			{
	//				wValue = pData[3+nIndex*2];
	//				wValue = (wValue&0x00ff)<<8;
	//				wValue |= (pData[3+nIndex*2+1]&0x00ff);

	//				(*itDt).second.wValue = wValue;
	//			}
	//		}
	//		wHeadAddr++;
	//	}
	//}
	return 0;
}

/***************************************************
*@brief:初始化hashmap的信息
*@param: void
*@return: bool,true设置成功，false设置失败。
*@author: ZHW 2011-04-01
****************************************************/
bool CCOMCtrl::Init_HashMap(void)
{
	//vector<gDeviceComm>	szVecDeviceComm;    //设备通讯信息容器
	//if (!CDeviceCommXml::GetInstance()->GetDeviceComm(szVecDeviceComm))
	//{
	//	return false;
	//}
	////初始化hash_map信息
	//for (vector<gDeviceComm>::iterator it=szVecDeviceComm.begin(); it!=szVecDeviceComm.end(); it++)
	//{
	//	gDeviceRevData szDeviceRevData;
	//	szDeviceRevData.startaddress = it->vecData.begin()->wAddress;
	//	for (vector<gRevData>::iterator itt=it->vecData.begin(); itt!=it->vecData.end(); itt++)
	//	{
	//		gItemData szItemData;
	//		szItemData.wAddress = itt->wAddress;
	//		szItemData.wFunc    = itt->wFunc;
	//		szItemData.strPlcParam = itt->strPlcParam;
	//		szDeviceRevData.mapItemData.insert(make_pair(itt->wAddress, szItemData));

	//		gData szData;
	//		szData.fMultiple = itt->fMultiple;
	//		szData.bWriteToPlc = itt->m_bWritePlc;
	//		szDeviceRevData.mapData.insert(make_pair(itt->strPlcParam, szData));			
	//	}
	//	szDeviceRevData.wCommSlaveID = it->nId;
	//	m_mapDeviceRevData.insert(make_pair(it->nId, szDeviceRevData));
	//}

	return true;
}

/***************************************************
*@brief:发送命令(串口通讯)
*@param: BYTE *pData 接收数据, BYTE bLength 接收数据长度
*@return:void
*@author: ZHW 2010-08-30
****************************************************/
BOOL CCOMCtrl::Is_DataValid(BYTE *pData, WORD dwLength)
{
	/*成功或失败的信号*/
	WORD wCRC			= 0;
	BYTE byData[2]	    = {0};
	if (dwLength <= 2){
		return FALSE;
	}
	wCRC = m_Modbus.CRC16___(pData, dwLength-2);
	memcpy(byData, &wCRC, sizeof(WORD));

	if ((byData[0]==pData[dwLength-1])
		&& (byData[1]==pData[dwLength-2]))
	{
		return TRUE;
	}
	return FALSE;
}

/***************************************************
*@brief:发送命令(串口通讯)
*@param: char *pData 发送数据, BYTE bLength 发送数据长度
*@return:void
*@author: ZHW 2010-08-10
****************************************************/
void CCOMCtrl::SendCmd(char *pData, BYTE bLength)
{
	EnterCriticalSection(&m_CriticalSection);

	//WriteToPort(pData, bLength);

	LeaveCriticalSection(&m_CriticalSection);
}

/***************************************************
*@brief:初始化COM
*@param: UINT nPortnr,串口号  UINT bBaud波特率
*@return: BOOL, TRUE成功，FALSE失败
*@author: ZHW 2011-04-01
****************************************************/
bool CCOMCtrl::InitCOMPort(UINT nPortnr, UINT bBaud, char parity)
{
	if (!Init_HashMap()
		|| !InitPort(nPortnr, bBaud, parity))
	{
		return false;
	}
	return true;
}

/***************************************************
*@brief:TCP/IP通讯发送数据[读指令]
*@param: unsigned int nSlaveId //设备的Slave ID 号
, WORD wAddrFrom		//读取属性的起始地址
, WORD wAddrTo			//读取属性的终止地址
, WORD wFunc);			//设备通讯类型
*@return: bool,true发送成功，false发送失败。
*@author: ZHW 2011-04-01
****************************************************/
bool CCOMCtrl::SendReadCmd(unsigned int nSlaveId, WORD wAddrFrom
								  , WORD wAddrTo, WORD wFunc)

{
	BYTE bLength = 0;

	m_pSendData = (char *)m_Modbus.ReadCmd(nSlaveId, wAddrFrom, wAddrTo, wFunc, bLength);	
	if ((NULL==m_pSendData) || (0 == bLength))
	{
		return false;
	}
	SendCmd(m_pSendData, bLength);

	return true;
}

/**********************************************************************
*@brief:TCP/IP通讯发送数据[写指令]
*@param: unsigned int nSlaveId   //设备的Slave ID 号
, WORD wAddress			//写入属性的地址
, WORD wValue			//属性值
, WORD wFunc);			//设备通讯类型
*@return: bool,true成功，false失败
*@author: ZHW 2011-04-01
***********************************************************************/
bool CCOMCtrl::SendWriteCmd(unsigned int nSlaveId, WORD wAddress
								  , WORD wValue, WORD wFunc)

{
	BYTE bLength = 0;

	m_pSendData = (char *)m_Modbus.WriteCmd(nSlaveId, wAddress, wValue, wFunc, bLength);	
	if ((NULL==m_pSendData) || (0 == bLength))
	{
		return false;
	}
	SendCmd(m_pSendData, bLength);

	return true;
}