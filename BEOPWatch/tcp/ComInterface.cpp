//ComInterface.cpp
#include "stdafx.h"
#include "ComInterface.h"

//////////////////////////////////////////////////////////////////////////
CComBase::CComBase()
{

}

CComBase::~CComBase()
{

}

/**********************************************************************
*@brief: 根据设备的Slave ID获取所接收的消息数据
*@param: unsigned int nSlaveId          //设备的Slave ID 号
, gDeviceRevData &RevData				//消息数据
*@return: bool,true成功，false失败
*@author: ZHW 2011-04-01
***********************************************************************/


bool CComBase::SendReadCmd( unsigned int nSlaveId, WORD wAddrFrom, WORD wAddrTo, WORD wFunc )
{
	return true;
}

bool CComBase::SendWriteCmd( unsigned int nSlaveId, WORD wAddress, WORD wValue, WORD wFunc )
{
	return true;
}
