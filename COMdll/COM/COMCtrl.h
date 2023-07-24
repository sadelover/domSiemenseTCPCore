#pragma once

#include "Modbus.h"
#include "SerialPort.h"

//hash_map 4251
#pragma warning(disable:4251)

#define  _CMD_REV_LENTH_  0x100   //接收数据的长度

class CCOMCtrl : public CSerialPort
{
public:
	CCOMCtrl(void);
	virtual ~CCOMCtrl(void);

protected:
    virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

	//Initialize hash_map Info
	bool Init_HashMap(void);

	void SendCmd(char *pData, BYTE bLength);
	BOOL Is_DataValid(BYTE *pData, WORD dwLength);

public:
	//Initialize COMPort Info
	bool InitCOMPort(UINT nPortnr, UINT bBaud, char parity = 'N');

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

private:
	char *m_pSendData;					//the send package buffer
	CModbus m_Modbus;
	CRITICAL_SECTION m_CriticalSection; //临界区保护
	
	WORD	m_StartAddress;		//起始地址。
};