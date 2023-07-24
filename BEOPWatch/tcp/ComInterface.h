//ComInterface.h
#pragma once
#pragma warning(disable:4251)

//////////////////////////////////////////////////////////////////////////
class CComBase
{
public:
    CComBase();
    virtual ~CComBase();

public:
    //Send Read Command
    virtual bool SendReadCmd(unsigned int nSlaveId   //设备的Slave ID 号
							 , WORD wAddrFrom		//读取属性的起始地址
							 , WORD wAddrTo			//读取属性的终止地址
							 , WORD wFunc);			//设备通讯类型

    //Send Write Command
    virtual bool SendWriteCmd(unsigned int nSlaveId   //设备的Slave ID 号
							, WORD wAddress			//写入属性的地址
							, WORD wValue			//属性值
							, WORD wFunc);			//设备通讯类型

    //Get Revdatas with nSlaveID

protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length) = 0;
};
