//_ **********************************************************
//_ 
//_ Name: SerialPort.cpp
//_ brief	: Serial Port communication
//			
//_ Purpose: 
//_ Author: Zhu Huawei  z_hw66@sohu.com
//-
//_ Copyright (c) 2009 
//_ 
//_ **********************************************************
#pragma once
#include "stdafx.h"
#include <assert.h>
#include <stdio.h>
#include <process.h>

#include "SerialPort.h"
#include "Tools/CustomTools/CustomTools.h"

#include <string>
#include <iostream>
#include <sstream>

//
// Constructor
CSerialPort::CSerialPort()
{
	m_nWriteSize = 1;
	m_hComm   = NULL;
	m_hThread = NULL;
	// initialize overlapped structure members to zero
	m_ov.Offset = 0;
	m_ov.OffsetHigh = 0;

	// create events
	m_ov.hEvent      = NULL;
	m_hWriteEvent    = NULL;
	m_hShutdownEvent = NULL;
	m_szWriteBuffer  = NULL;
	m_bThreadAlive   = FALSE;
	m_dbsession_history = NULL;
	m_bReceived = false;
	
	ZeroMemory(m_hEventArray, 0x00);

	m_oleLastWriteTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
	m_bRecLog = true;
	m_nReadMode = 0;

	memset(m_cRXBuff,0,sizeof(m_cRXBuff));
	m_nReceCount = 0;
}

//
// Delete dynamic memory
CSerialPort::~CSerialPort()
{
	do{
		SetEvent(m_hShutdownEvent);
	} while (m_bThreadAlive);

	// if the port is still opened: close it 
	if (m_hComm != NULL)
	{
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}
	// Close Handles  
	if(m_hShutdownEvent!=NULL)
		CloseHandle(m_hShutdownEvent); 
	if(m_ov.hEvent!=NULL)
		CloseHandle( m_ov.hEvent ); 
	if(m_hWriteEvent!=NULL)
		CloseHandle( m_hWriteEvent ); 

	//TerminateThread(m_hThread,0);

	printf("Thread ended\n");
	if(m_szWriteBuffer != NULL)
	{
		m_szWriteBuffer[m_nWriteBufferSize-1] = '\0';
		delete m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
}

//
// If there is a error, give the right message
void CSerialPort::ProcessErrorMessage(char* ErrorText)
{
	//wchar_t *pTemp = new wchar_t[200];
	//LPVOID lpMsgBuf;

	//FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
	//	,NULL,GetLastError()
	//	,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) // Default language
	//	,(LPTSTR) &lpMsgBuf, 0, NULL);

	//wsprintf(pTemp, _T("WARNING:  %s Failed with the following error: \n%s\nPort: %d\n")
	//	, (char*)ErrorText, lpMsgBuf, m_nPortNr); 
	////MessageBox(NULL, pTemp, _T("Application Error"), MB_ICONSTOP);

	//_tprintf(pTemp);

	//LocalFree(lpMsgBuf);
	//delete []pTemp;


	/*CString strOut;
	strOut.Format(_T("WARNING:%sFailed with the following error: Port: %d\r\n"), Project::Tools::AnsiToWideChar(ErrorText), m_nPortNr);
	_tprintf(strOut);*/
}
//
// Character received. Inform the owner
void CSerialPort::ReceiveChar(CSerialPort *port, COMSTAT comstat)
{
	BOOL  bRead = TRUE; 
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD dwBytesRead = 0;
	unsigned char RXBuff[1024]={0};
	DWORD dwReadLength = 0;
	DWORD dwcbInQue = 0;
	for (;;) 
	{ 
		if(WaitForSingleObject(port->m_hShutdownEvent,0)==WAIT_OBJECT_0)  
			return;  
		// Gain ownership of the comm port critical section.
		// This process guarantees no other part of this program 
		// is using the port object. 
		EnterCriticalSection(&port->m_csCommunicationSync);

		// ClearCommError() will update the COMSTAT structure and
		// clear any other errors.
		bResult = ClearCommError(port->m_hComm, &dwError, &comstat);
		LeaveCriticalSection(&port->m_csCommunicationSync);

		// start forever loop.  I use this type of loop because I
		// do not know at runtime how many loops this will have to
		// run. My solution is to start a forever loop and to
		// break out of it when I have processed all of the
		// data available.  Be careful with this approach and
		// be sure your loop will exit.
		// My reasons for this are not as clear in this sample 
		// as it is in my production code, but I have found this 
		// solutiion to be the most efficient way to do this.
		dwcbInQue = comstat.cbInQue;
		if (dwcbInQue == 0)
		{
			// break out when all bytes have been read
			break;
		}

		EnterCriticalSection(&port->m_csCommunicationSync);

	/*	COMSTAT szComStat;
		ClearCommError(port->m_hComm, &dwError, &szComStat);
		if (!szComStat.cbInQue)
			return;*/
		if(port->m_bRecLog)
			printf("DTU Reveive\n");
		
		if (bRead)
		{

			if(port->m_nReadMode == 0)
			{
				dwReadLength = 1;
			}
			else if(port->m_nReadMode == 1)
			{
				dwReadLength = (sizeof(RXBuff)<dwcbInQue)?sizeof(RXBuff):dwcbInQue;
			}
			else if(port->m_nReadMode == 2)
			{
				dwReadLength = sizeof(RXBuff);
			}

			bResult = ReadFile(port->m_hComm // Handle to COMM port 
				,RXBuff			    // RX Buffer Pointer
				,dwReadLength 	// Read one byte
				,&dwBytesRead		// Stores number of bytes read
				,&port->m_ov);		// pointer to the m_ov structure
			// deal with the error code 
			if (!bResult)  
			{ 
				switch (dwError = GetLastError()) 
				{ 
				case ERROR_IO_PENDING: 	
					{ 
						// asynchronous i/o is still in progress 
						// Proceed on to GetOverlappedResults();
						bRead = FALSE;
						break;
					}
				default:
					{
						// Another error has occured.  Process this error.
						port->ProcessErrorMessage("ReadFile()");
						break;
					} 
				}
			}
			else
			{
				// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
				bRead = TRUE;
			}
		}

		if (!bRead)
		{
			bRead = TRUE;
			bResult = GetOverlappedResult(port->m_hComm	// Handle to COMM port 
				,&port->m_ov		// Overlapped structure
				,&dwBytesRead		// Stores number of bytes read
				,TRUE); 			// Wait flag

			// deal with the error code 
			if (!bResult)  
			{
				port->ProcessErrorMessage("GetOverlappedResults() in ReadFile()");
			}	
		}
		LeaveCriticalSection(&port->m_csCommunicationSync);

		//清空接收缓存，解决串口收造成的内存越位奔溃bug   by Robert 20140801
		//PurgeComm(port->m_hComm, PURGE_RXCLEAR  | PURGE_RXABORT );

		if(dwBytesRead>0)
		{
			port->StoreRecvData((char*)RXBuff,dwBytesRead);
		}
	}
}

//
// Write a character.
bool CSerialPort::WriteChar(CSerialPort* port)
{
//	ADEBUG(_T("write to port"));
	BOOL bResult = TRUE;
	DWORD BytesSent = 0;

	ResetEvent(port->m_hWriteEvent);
	// Gain ownership of the critical section
	EnterCriticalSection(&port->m_csCommunicationSync);
	// initialize variables
	port->m_ov.Offset = 0;			//该文件的位置是从文件起始处的字节偏移量 读取或写入为0
	port->m_ov.OffsetHigh = 0;		//指定文件传送的字节偏移量的高位字

	//clear buffer  by robert 20140612
	//PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	PurgeComm(port->m_hComm, PURGE_TXABORT | PURGE_TXCLEAR );		//只清空发送缓存区

	//向串口写数据 
	bResult = WriteFile(port->m_hComm					// 串口句柄
						,port->m_szWriteBuffer			// 待写入数据的首地址 
						,port->m_nWriteSize				// 待写入数据的字节数长度 
						,&BytesSent						// 函数返回的实际写入串口的数据个数的地址，利用此变量可判断实际写入的字节数和准备写入的字节数是否相同。 
						,&port->m_ov);					// 重叠I/O结构的指针 

		// deal with any error codes
	if (!bResult) {
		DWORD dwError = GetLastError();
		switch (dwError)
		{
		case ERROR_IO_PENDING:						//正在后台处理（重叠 I/O处理成功,只是操作还没完成）			
			{
				BytesSent = 0;
				// continue to GetOverlappedResults()
				
				/*在此等待异步操作结果,直到异步操作结束时才返回.实际上此时WaitCommEvent()函数一直在等待串口监控的事件之一发生,当事件发
				生时该函数将OVERLAPPED结构中的事件句柄置为有信号状态,此时GetOverlappedResult()函数发现此事件有信号后马上返回,然后下面
				的程序马上分析WaitCommEvent()函数等到的事件是被监视的串口事件中的哪一个,然后执行相应的动作并发出相应消息. */ 

				//判断一个重叠操作当前的状态 
				bResult = GetOverlappedResult(port->m_hComm,	// Handle to COMM port 
											 &port->m_ov,		// Overlapped structure
											&BytesSent,		// Stores number of bytes sent
											TRUE); 			// Wait flag
				port->m_strSendSuccessBuffer = port->m_szWriteBuffer;
					break;
			}
		}
	}

	//PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	LeaveCriticalSection(&port->m_csCommunicationSync);

	if(!bResult)
	{
		printf("serial port writechar fail!...\r\n");
	}

	if(port->m_lpRecDataProc!=NULL)
	{
		//更新发送时间
		if(bResult)
		{
			port->UpdateDTUWriteTime();
		}
	}

	return bResult;
}

BOOL CSerialPort::InitPort(UINT  portnr		// port number (1..4)
						   ,UINT  baud			// baud rate
						   ,char  parity		// parity 
						   ,UINT  databits		// databits 
						   ,UINT  stopbits		// stopbits 
						   ,DWORD dwCommEvents 	// EV_RXCHAR, EV_CTS etc
						   ,UINT  writebuffersize)	// size to the write buffer
{
	BOOL bResult = FALSE;
	wchar_t szPort[50]={0}, szBaud[100]={0};

	//assert(portnr > 0 && portnr < 10);
	// if the thread is alive: Kill  
	if (m_bThreadAlive)  
	{  
		do  
		{  
			SetEvent(m_hShutdownEvent);  
		} while (m_bThreadAlive);  
		TRACE("Thread ended\n");  
	}  

	// create events
	if (m_ov.hEvent != NULL)
		ResetEvent(m_ov.hEvent);
	else
		m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hWriteEvent != NULL)
		ResetEvent(m_hWriteEvent);
	else
		m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hShutdownEvent != NULL)
		ResetEvent(m_hShutdownEvent);
	else
		m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// 初始化事件对象
	m_hEventArray[0] = m_hShutdownEvent;	//最高优先级
	m_hEventArray[2] = m_ov.hEvent;
	m_hEventArray[1] = m_hWriteEvent;

	// 初始化临界区
	InitializeCriticalSection(&m_csCommunicationSync);

	// 设置读写缓存
	if (m_szWriteBuffer != NULL)
	{
		delete []m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	m_szWriteBuffer = new char[writebuffersize];

	m_nPortNr = portnr;
	m_nWriteBufferSize = writebuffersize;
	m_dwCommEvents = dwCommEvents;

	// 开始进入临界区
	EnterCriticalSection(&m_csCommunicationSync);

	// 如果该端口已经打开：先关闭
	if (m_hComm != NULL){
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	// prepare port strings
	if(portnr <10)
	{
		wsprintf(szPort, _T("COM%d"), portnr); 
	}
	else
	{
		wsprintf(szPort, _T("\\\\.\\COM%d"), portnr); 
	}
	//wsprintf(szPort, _T("COM%d"), portnr); 
	//strPort.Format(_T("COM%d"), portnr);
	wsprintf(szBaud, _T("baud=%d parity=%c data=%d stop=%d"), baud, parity, databits, stopbits); 

	// 返回一个用来访问串口的句柄
	m_hComm = CreateFile(szPort			// communication port string (COMX)
						,GENERIC_READ | GENERIC_WRITE	// 访问模式 read/write types
						,0								// 共享模式  零表示不共享
						,NULL							// no security attributes  定义了文件的安全特性
						,OPEN_EXISTING					// 文件必须已经存在（如何创建）
						,FILE_FLAG_OVERLAPPED	 // 文件属性：允许对文件进行重叠操作|默认属性    异步I/O 
						,NULL);							// template must be 0 for comm devices

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		// port not found
		return FALSE;
	}

	wstring wstrReadIntervalTimeout = L"1000";
	wstring wstrReadTotalTimeoutMultiplier = L"1000";
	wstring wstrReadTotalTimeoutConstant = L"1000";
	wstring wstrRecLog = L"0";
	wstring wstrCmdTerminator = L"0";
	wstring wstrReadMode = L"0";
	if(m_dbsession_history)
	{
		wstrReadIntervalTimeout = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"ReadIntervalTimeout",L"1000");
		wstrReadTotalTimeoutMultiplier = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"ReadTotalTimeoutMultiplier",L"1000");
		wstrReadTotalTimeoutConstant = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"ReadTotalTimeoutConstant",L"1000");
		wstrRecLog = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"RecLog",L"0");
		wstrCmdTerminator = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"CmdTerminator",L"0");
		wstrReadMode = m_dbsession_history->ReadOrCreateCoreDebugItemValue_Defalut(L"SerialRMode",L"0");
	}

	m_bCmdTerminator = true;
	if(wstrCmdTerminator == L"" || wstrCmdTerminator == L"0")
		m_bCmdTerminator = false;
	m_nReadMode = _wtoi(wstrReadMode.c_str());

	if(wstrRecLog == L"0")
		m_bRecLog = false;

	// set the timeout values
	if(wstrReadIntervalTimeout == L"-1")
	{
		m_CommTimeouts.ReadIntervalTimeout = MAXDWORD;				// 两字符之间最大的延时 
	}
	else
	{
		m_CommTimeouts.ReadIntervalTimeout = _wtoi(wstrReadIntervalTimeout.c_str());				// 两字符之间最大的延时 
	}
	//m_CommTimeouts.ReadIntervalTimeout = _wtoi(wstrReadIntervalTimeout.c_str());				// 两字符之间最大的延时 
	m_CommTimeouts.ReadTotalTimeoutMultiplier = _wtoi(wstrReadTotalTimeoutMultiplier.c_str());		// 指定比例因子（毫秒），实际上是设置读取一个字节和等待下一个字节所需的时间 
	m_CommTimeouts.ReadTotalTimeoutConstant = _wtoi(wstrReadTotalTimeoutConstant.c_str());			// 一次读取串口数据的固定超时
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;		// 写入每字符间的超时
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;		// 一次写入串口数据的固定超时
	// configure
	if (SetCommTimeouts(m_hComm, &m_CommTimeouts))				//设定通讯设备读写时的超时参数
	{						   
		if (SetCommMask(m_hComm, dwCommEvents)){					//设置串口状态 串口号、波特率、奇偶校验方式、数据位
			if (GetCommState(m_hComm, &m_dcb)){							//取得串口当前状态(串口句柄,设备控制块(Device Control Block)结构地址) 
				m_dcb.EvtChar = 'q';							//当接收到此字符时,会产生一个事件
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;		// set RTS bit high!
				if (BuildCommDCB(szBaud, &m_dcb)){				//初始化串口
					if (!SetCommState(m_hComm, &m_dcb)){// normal operation... continue
						ProcessErrorMessage("SetCommState()");
					}
				}
				else{
					ProcessErrorMessage("BuildCommDCB()");
				}
			}
			else{
				ProcessErrorMessage("GetCommState()");
			}
		}
		else{
			ProcessErrorMessage("SetCommMask()");
		}
	}
	else
	{
		ProcessErrorMessage("SetCommTimeouts()");
	}
	//清空端口缓存  清除输入缓冲区|输出缓冲区|终止所有正在进行的字符输入操作|输出操作
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	// release critical section
	LeaveCriticalSection(&m_csCommunicationSync);
	//printf("Initialisation for communicationport %d completed.\r\n", portnr);

	return TRUE;
}

void CSerialPort::ClosePort()
{
	// if the port is still opened: close it 

	m_lpRecDataProc = NULL;

	SetEvent(m_hShutdownEvent);

	if (m_hComm != NULL)  
	{  
		CloseHandle(m_hComm);  
		m_hComm = NULL;  
	}  	
}

//
// Restart the comm thread
BOOL CSerialPort::RestartMonitoring()
{
	printf("Thread resumed\n");
	::ResumeThread(m_hThread);
	return TRUE;	
}

//
// Suspend the comm thread
BOOL CSerialPort::StopMonitoring()
{
	printf("Thread suspended\n");
	::SuspendThread(m_hThread); 
	return TRUE;	
}

//
// Return the output buffer size
DWORD CSerialPort::GetWriteBufferSize()
{
	return m_nWriteBufferSize;
}

//
// Return the communication event masks
DWORD CSerialPort::GetCommEvents()
{
	return m_dwCommEvents;
}

//
// Return the device control block
DCB CSerialPort::GetDCB()
{
	return m_dcb;
}

//
// Write a string to the port
bool CSerialPort::WriteToPort(char *pData)
{		
	assert(m_hComm != 0);
	
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	strcpy(m_szWriteBuffer, pData);
	m_nWriteSize = (int)strlen(pData);

	// set event for write
	SetEvent(m_hWriteEvent);
	//return WriteChar(this);
	return true;
}

bool CSerialPort::WriteToPort(char *pData,int nLen)
{		
	assert(m_hComm != 0);
    if(!m_hComm)
        return false;
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, pData, nLen);
	m_nWriteSize = nLen;

	// set event for write		edit  by robert 20140612	
	SetEvent(m_hWriteEvent);
	//return WriteChar(this);
	return true;
}


bool CSerialPort::WriteToPortByWrap( char *pData,int nLen )
{
	assert(m_hComm != 0);
	if(!m_hComm)
		return false;
	if(nLen >= 1020)
		nLen = 1020;
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, pData, nLen);
	m_szWriteBuffer[nLen-2] = ';';			//以;;\n为包结尾
	m_szWriteBuffer[nLen-1] = '\n';
	m_szWriteBuffer[nLen] = '\0';
	m_nWriteSize = nLen;
	if(m_bCmdTerminator)
		m_nWriteSize = nLen+1;

	// set event for write		edit  by robert 20140612	
	SetEvent(m_hWriteEvent);
	//return WriteChar(this);
	return true;
}

bool CSerialPort::WriteToPort(LPCTSTR string)
{		
	assert(m_hComm != 0);

	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	strcpy(m_szWriteBuffer, (LPSTR)string);
	m_nWriteSize = wcslen(string);

	// set event for write
	SetEvent(m_hWriteEvent);
	//return WriteChar(this);
	return true;
}

bool CSerialPort::WriteToPort(LPCTSTR string, int nLen)
{		
	assert(m_hComm != 0);
	if(nLen >= 1020)
		nLen = 1020;
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, string, nLen);
	m_nWriteSize = nLen;

	// set event for write
	SetEvent(m_hWriteEvent);
	//return WriteChar(this);
	return true;
}

bool CSerialPort::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	//  
	//接收数据  
	//  
	assert(m_hComm!=0);  
	memset((unsigned char*)pRevData,0,length);  
	DWORD mylen  = 0;  
	DWORD mylen2 = 0;  
	while (mylen<length)
	{  
		if(!ReadFile(m_hComm,(unsigned char*)pRevData,length,&mylen2,NULL))   
			return FALSE;  
		mylen += mylen2;  
	}  

	return true;
}

//  
//  The CommThread Function.  
//  
UINT CSerialPort::CommThread(LPVOID pParam)  
{  
	// Cast the void pointer passed to the thread back to  
	// a pointer of CSerialPort class  
	CSerialPort *port = (CSerialPort*)pParam;  

	// Set the status variable in the dialog class to  
	// TRUE to indicate the thread is running.  
	port->m_bThreadAlive = TRUE;   

	// Misc. variables  
	DWORD BytesTransfered = 0;   
	DWORD Event = 0;  
	DWORD CommEvent = 0;  
	DWORD dwError = 0;  
	static COMSTAT comstat;  
	BOOL  bResult = TRUE;  

	// Clear comm buffers at startup  
	if (port->m_hComm)       // 检测端口是否打开 清空缓存
		PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);  

	// begin forever loop.  This loop will run as long as the thread is alive.  
	for (;;)   
	{   

		// Make a call to WaitCommEvent().  This call will return immediatly  
		// because our port was created as an async port (FILE_FLAG_OVERLAPPED  
		// and an m_OverlappedStructerlapped structure specified).  This call will cause the   
		// m_OverlappedStructerlapped element m_OverlappedStruct.hEvent, which is part of the m_hEventArray to   
		// be placed in a non-signeled state if there are no bytes available to be read,  
		// or to a signeled state if there are bytes available.  If this event handle   
		// is set to the non-signeled state, it will be set to signeled when a   
		// character arrives at the port.  

		// we do this for each port!  

		bResult = WaitCommEvent(port->m_hComm, &Event, &port->m_ov);	//用来判断用SetCommMask()函数设置的串口通信事件是否已发生。 

		if (!bResult)   //如果异步操作不能立即完成的话,函数返回FALSE,
		{   
			// If WaitCommEvent() returns FALSE, process the last error to determin  
			// the reason..  
			switch (dwError = GetLastError())   
			{   
			case ERROR_IO_PENDING:		//指示异步操作正在后台进行.
				{   
					// This is a normal return value if there are no bytes  
					// to read at the port.  
					// Do nothing and continue  
					break;  
				}  
			case 87:  
				{  
					// Under Windows NT, this value is returned for some reason.  
					// I have not investigated why, but it is also a valid reply  
					// Also do nothing and continue.  
					break;  
				}  
			default:  
				{  
					// All other error codes indicate a serious error has  
					// occured.  Process this error.  
					port->ProcessErrorMessage("WaitCommEvent()");  
					break;  
				}  
			}  
		}  
		else		//函数成功，返回非零值
		{  
			// If WaitCommEvent() returns TRUE, check to be sure there are  
			// actually bytes in the buffer to read.    
			//  
			// If you are reading more than one byte at a time from the buffer   
			// (which this program does not do) you will have the situation occur   
			// where the first byte to arrive will cause the WaitForMultipleObjects()   
			// function to stop waiting.  The WaitForMultipleObjects() function   
			// resets the event handle in m_OverlappedStruct.hEvent to the non-signelead state  
			// as it returns.    
			//  
			// If in the time between the reset of this event and the call to   
			// ReadFile() more bytes arrive, the m_OverlappedStruct.hEvent handle will be set again  
			// to the signeled state. When the call to ReadFile() occurs, it will   
			// read all of the bytes from the buffer, and the program will  
			// loop back around to WaitCommEvent().  
			//   
			// At this point you will be in the situation where m_OverlappedStruct.hEvent is set,  
			// but there are no bytes available to read.  If you proceed and call  
			// ReadFile(), it will return immediatly due to the async port setup, but  
			// GetOverlappedResults() will not return until the next character arrives.  
			//  
			// It is not desirable for the GetOverlappedResults() function to be in   
			// this state.  The thread shutdown event (event 0) and the WriteFile()  
			// event (Event2) will not work if the thread is blocked by GetOverlappedResults().  
			//  
			// The solution to this is to check the buffer with a call to ClearCommError().  
			// This call will reset the event handle, and if there are no bytes to read  
			// we can loop back through WaitCommEvent() again, then proceed.  
			// If there are really bytes to read, do nothing and proceed.  

			//清除硬件的通讯错误以及获取通讯设备的当前状态
			bResult = ClearCommError(port->m_hComm, &dwError, &comstat);  

			if (comstat.cbInQue == 0)	//数据长度
				continue;  
		}   // end if bResult  

		// Main wait function.  This function will normally block the thread  
		// until one of nine events occur that require action.  

		Event = WaitForMultipleObjects(3, port->m_hEventArray, FALSE, INFINITE);  

		switch (Event)  
		{  
		case 0:  
			{  
				// Shutdown event.  This is event zero so it will be  
				// the higest priority and be serviced first.  

				port->m_bThreadAlive = FALSE;  

				// Kill this thread.  break is not needed, but makes me feel better.  
				AfxEndThread(100);  

				if (NULL != port->m_hThread)
				{
					CloseHandle(port->m_hThread);
					port->m_hThread = NULL;
				}

				break;  
			}  
		case 2: // read event  
			{  
				if(port->GetReceiveEvent())
				{
					GetCommMask(port->m_hComm, &CommEvent);  
					if (CommEvent & EV_RXCHAR) //接收到字符，并置于输入缓冲区中   注释by robert 20140529
						ReceiveChar(port, comstat);  
				}
				break;  
			}    
		case 1: // write event  
			{  
				// Write character event from port  
				WriteChar(port);  
				break;  
			}  

		} // end switch  

	} // close forever loop  

	return 0;  
}  

BOOL CSerialPort::StartMonitoring(LPRecDataProc proc1,DWORD userdata)
{
	m_lpRecDataProc = proc1;
	m_dwUserData = userdata;

	m_hThread = AfxBeginThread(CommThread,this);

	if (!m_hThread)  
		return FALSE;  
	TRACE("Thread started\n");  
	return TRUE;
}

BOOL CSerialPort::RecvData( LPTSTR lpszData, const int nSize )
{
	//
	//接收数据
	//
	assert(m_hComm!=0);
	memset(lpszData,0,nSize);
	DWORD mylen  = 0;
	DWORD mylen2 = 0;
	while (mylen<nSize) {
		if(!ReadFile(m_hComm,lpszData,nSize,&mylen2,NULL)) 
			return FALSE;
		mylen += mylen2;


	}

	return TRUE;
}

bool CSerialPort::SetReceiveEvent( bool bReceived )
{
	m_bReceived = bReceived;
	return true;
}

bool CSerialPort::GetReceiveEvent()
{
	return m_bReceived;
}

std::string CSerialPort::GetSuccessBuffer()
{
	return m_strSendSuccessBuffer;
}

bool CSerialPort::UpdateDTUWriteTime()
{
	m_oleLastWriteTime = COleDateTime::GetCurrentTime();
	wstring strItemValue;
	Project::Tools::OleTime_To_String(m_oleLastWriteTime,strItemValue);
	//更新ini文件
	if(m_dbsession_history)
	{
		m_dbsession_history->WriteCoreDebugItemValue(L"dtusendtime",strItemValue);
	}
	return true;
}

bool CSerialPort::StoreRecvData( char *pData,int nLen )
{
	memcpy(m_cRXBuff+m_nReceCount,pData,nLen);
	m_nReceCount = m_nReceCount + nLen;

	if(pData[nLen-1] == '\n')
	{
		if(m_lpRecDataProc!=NULL)
		{
			//收到字符大于1的时候，才回调处理
			m_lpRecDataProc(m_cRXBuff,m_nReceCount,m_dwUserData);
		}

		memset(m_cRXBuff,0,sizeof(m_cRXBuff));
		m_nReceCount = 0;
	}
	return true;
}

bool CSerialPort::GetDTUIdleState( int nSeconds /*= 4*/ )
{
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleLastWriteTime;
	if(oleSpan.GetTotalSeconds() >= nSeconds)
		return true;
	return false;
}

void CSerialPort::SetHistoryDbCon( Beopdatalink::CCommonDBAccess* phisdbcon )
{
	m_dbsession_history = phisdbcon;
}
