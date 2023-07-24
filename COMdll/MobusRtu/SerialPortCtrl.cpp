
#pragma once
#include "stdafx.h"
#include <assert.h>
#include <stdio.h>
#include <process.h>

#include "SerialPortCtrl.h"
#include "Tools/CustomTools/CustomTools.h"
//
// Constructor
CSerialPortCtrl::CSerialPortCtrl()
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
	m_bReceived = true;
	m_bConnectRTUOK = false;
	m_nErrorCount = 0;

	m_nBaud = 19200;
	m_nDatabits = 8;
	m_nStopbits = 1;
	m_dwCommEvents = EV_RXCHAR;
	m_cParity = 'N';
	
	ZeroMemory(m_hEventArray, 0x00);
}

//
// Delete dynamic memory
CSerialPortCtrl::~CSerialPortCtrl()
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
	if (NULL != m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	printf("Thread ended\n");
	delete []m_szWriteBuffer;
}

//
// If there is a error, give the right message
void CSerialPortCtrl::ProcessErrorMessage(char* ErrorText)
{
	m_nErrorCount++;
	_tprintf(L"ERROR: Modbus RTU(Port%d) Error:%s.\r\n", m_nPortNr,Project::Tools::AnsiToWideChar(ErrorText).c_str());
}
//
// Character received. Inform the owner
void CSerialPortCtrl::ReceiveChar(CSerialPortCtrl *port, COMSTAT comstat)
{
	BOOL  bRead = TRUE; 
	BOOL  bResult = TRUE;
	DWORD dwError = 0;
	DWORD dwBytesRead = 0;
	unsigned char RXBuff[256]={0};

	for (;;) 
	{ 
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
		if (comstat.cbInQue == 0)
		{
			// break out when all bytes have been read
			break;
		}
		EnterCriticalSection(&port->m_csCommunicationSync);

		/*COMSTAT szComStat;
		ClearCommError(port->m_hComm, &dwError, &szComStat);
		if (!szComStat.cbInQue)
			return;*/
	
		if (bRead)
		{
			bResult = ReadFile(port->m_hComm // Handle to COMM port 
				,RXBuff			    // RX Buffer Pointer
				,sizeof(RXBuff) 	// Read one byte
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
		// notify parent that a byte was received
		port->OnCommunication(RXBuff, dwBytesRead);
	}
}

//
// Write a character.
void CSerialPortCtrl::WriteChar(CSerialPortCtrl* port)
{
	//ADEBUG(_T("write to port"));
	BOOL bResult = TRUE;
	DWORD BytesSent = 0;

	ResetEvent(port->m_hWriteEvent);
	// Gain ownership of the critical section
	EnterCriticalSection(&port->m_csCommunicationSync);
	// initialize variables
	port->m_ov.Offset = 0;
	port->m_ov.OffsetHigh = 0;


	bResult = WriteFile(port->m_hComm	// Handle to COMM Port
						,port->m_szWriteBuffer			// Pointer to message buffer in calling function
						,port->m_nWriteSize				// Length of message to send
						,&BytesSent						// Where to store the number of bytes sent
						,&port->m_ov);					// Overlapped structure

		// deal with any error codes
	if (!bResult) {
		DWORD dwError = GetLastError();
		switch (dwError)
		{
		case ERROR_IO_PENDING:
				// continue to GetOverlappedResults()
				bResult = GetOverlappedResult(port->m_hComm,	// Handle to COMM port 
											 &port->m_ov,		// Overlapped structure
											&BytesSent,		// Stores number of bytes sent
											TRUE); 			// Wait flag
											BytesSent = 0;
					break;
			}
	}

	PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	LeaveCriticalSection(&port->m_csCommunicationSync);
}

BOOL CSerialPortCtrl::InitPort( UINT  portnr		// port number (1..4)
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

	m_nBaud = baud;
	m_nDatabits = databits;
	m_nStopbits = stopbits;
	m_dwCommEvents = dwCommEvents;
	m_cParity = parity;

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

	// initialize the event objects
	m_hEventArray[0] = m_hShutdownEvent;	// highest priority
	m_hEventArray[1] = m_ov.hEvent;
	m_hEventArray[2] = m_hWriteEvent;

	// initialize critical section
	InitializeCriticalSection(&m_csCommunicationSync);

	// set buffersize for writing and save the owner
	if (m_szWriteBuffer != NULL)
		delete []m_szWriteBuffer;
	m_szWriteBuffer = new char[writebuffersize];

	m_nPortNr = portnr;
	m_nWriteBufferSize = writebuffersize;
	m_dwCommEvents = dwCommEvents;

	// now it critical!
	EnterCriticalSection(&m_csCommunicationSync);

	// if the port is already opened: close it
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

	// get a handle to the port
	m_hComm = CreateFile(szPort			// communication port string (COMX)
						,GENERIC_READ | GENERIC_WRITE	// read/write types
						,0								// comm devices must be opened with exclusive access
						,NULL							// no security attributes
						,OPEN_EXISTING					// comm devices must use OPEN_EXISTING
						,FILE_FLAG_OVERLAPPED|FILE_ATTRIBUTE_NORMAL	 // Async I/O
						,NULL);							// template must be 0 for comm devices

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		// port not found
		LeaveCriticalSection(&m_csCommunicationSync);
		int nError = GetLastError();
		return FALSE;
	}

	// set the timeout values
	m_CommTimeouts.ReadIntervalTimeout = 1000;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;

	// configure
	if (SetCommTimeouts(m_hComm, &m_CommTimeouts))
	{						   
		if (SetCommMask(m_hComm, dwCommEvents)){
			if (GetCommState(m_hComm, &m_dcb)){
				m_dcb.EvtChar = 'q';
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;		// set RTS bit high!
				if (BuildCommDCB(szBaud, &m_dcb)){
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
	// flush the port
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	// release critical section
	LeaveCriticalSection(&m_csCommunicationSync);
	//printf("Initialisation for communicationport %d completed.\nUse Startmonitor to communicate.\n", portnr);
	m_bConnectRTUOK = true;
	return TRUE;
}

void CSerialPortCtrl::ClosePort(int nCode)
{
	_tprintf(L"DEBUG: Modbus Closing Port(Com:%d,Code:%d)...\r\n",m_nPortNr,nCode);
	m_bConnectRTUOK = false;
	m_nErrorCount = 0;
	SetEvent(m_hShutdownEvent);

	if(m_hThread != NULL)
	{
		CloseHandle(m_hThread);  
		m_hThread = NULL;  
	}
	if (m_hComm != NULL)  
	{  
		CloseHandle(m_hComm);  
		m_hComm = NULL;  
	}  	
}

//
// Restart the comm thread
BOOL CSerialPortCtrl::RestartMonitoring()
{
	printf("Thread resumed\n");
	::ResumeThread(m_hThread);
	return TRUE;	
}

//
// Suspend the comm thread
BOOL CSerialPortCtrl::StopMonitoring()
{
	printf("Thread suspended\n");
	::SuspendThread(m_hThread); 
	return TRUE;	
}

//
// Return the output buffer size
DWORD CSerialPortCtrl::GetWriteBufferSize()
{
	return m_nWriteBufferSize;
}

//
// Return the communication event masks
DWORD CSerialPortCtrl::GetCommEvents()
{
	return m_dwCommEvents;
}

//
// Return the device control block
DCB CSerialPortCtrl::GetDCB()
{
	return m_dcb;
}

//
// Write a string to the port
void CSerialPortCtrl::WriteToPort(char *pData)
{		
	assert(m_hComm != 0);
	
	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	strcpy(m_szWriteBuffer, pData);
	m_nWriteSize = (int)strlen(pData);

	// set event for write
	//WriteChar(this);
	SetEvent(m_hWriteEvent);
}

void CSerialPortCtrl::WriteToPort(char *pData,int nLen)
{		
	assert(m_hComm != 0);
    if(!m_hComm)
        return;

	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, pData, nLen);
	m_nWriteSize = nLen;

	// set event for write
	//WriteChar(this);
	SetEvent(m_hWriteEvent);
}

void CSerialPortCtrl::WriteToPort(LPCTSTR string)
{		
	assert(m_hComm != 0);

	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	strcpy(m_szWriteBuffer, (LPSTR)string);
	m_nWriteSize = wcslen(string);

	// set event for write
	//WriteChar(this);
	SetEvent(m_hWriteEvent);
}

void CSerialPortCtrl::WriteToPort(LPCTSTR string, int nLen)
{		
	assert(m_hComm != 0);

	memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
	memcpy(m_szWriteBuffer, string, nLen);
	m_nWriteSize = nLen;

	// set event for write
	//WriteChar(this);
	SetEvent(m_hWriteEvent);
}

bool CSerialPortCtrl::OnCommunication( const unsigned char* pRevData, DWORD length )
{
	// did nothing.
	return true;
}

BOOL CSerialPortCtrl::StartMonitoring()
{
	if(m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	//m_hThread = AfxBeginThread(CommThread,this);
	m_hThread = (HANDLE) _beginthreadex(NULL, 0, CommThread, this, NORMAL_PRIORITY_CLASS, NULL);
	if (!m_hThread)  
		return FALSE;  
	TRACE("Thread started\n");  
	return TRUE;
}


//  
//  The CommThread Function.  
//  
UINT WINAPI CSerialPortCtrl::CommThread(LPVOID pParam)  
{  
	// Cast the void pointer passed to the thread back to  
	// a pointer of CSerialPort class  
	CSerialPortCtrl *port = (CSerialPortCtrl*)pParam;  

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
		case 1: // read event  
			{  
				if(port->GetReceiveEvent())
				{
					GetCommMask(port->m_hComm, &CommEvent);  
					if (CommEvent & EV_RXCHAR) //接收到字符，并置于输入缓冲区中   注释by robert 20140529
						ReceiveChar(port, comstat);  
				}
				break;  
			}    
		case 2: // write event  
			{  
				// Write character event from port  
				WriteChar(port);  
				break;  
			}  

		} // end switch  

	} // close forever loop  

	return 0;  
}  

bool CSerialPortCtrl::GetReceiveEvent()
{
	return m_bReceived;
}

bool CSerialPortCtrl::SetReceiveEvent( bool bReceived )
{
	m_bReceived = bReceived;
	return true;
}

bool CSerialPortCtrl::ReConnect()
{
	ClosePort(4);

	m_bThreadAlive   = FALSE;
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

	// now it critical!
	EnterCriticalSection(&m_csCommunicationSync);

	wchar_t szPort[50]={0}, szBaud[100]={0};

	// if the port is already opened: close it
	if (m_hComm != NULL){
		CloseHandle(m_hComm);
		m_hComm = NULL;
	}

	// prepare port strings
	if(m_nPortNr <10)
	{
		wsprintf(szPort, _T("COM%d"), m_nPortNr); 
	}
	else
	{
		wsprintf(szPort, _T("\\\\.\\COM%d"), m_nPortNr); 
	}
	//wsprintf(szPort, _T("COM%d"), m_nPortNr); 

	//strPort.Format(_T("COM%d"), portnr);
	wsprintf(szBaud, _T("baud=%d parity=%c data=%d stop=%d"), m_nBaud, m_cParity, m_nDatabits, m_nStopbits); 
	
	// get a handle to the port
	m_hComm = CreateFile(szPort			// communication port string (COMX)
		,GENERIC_READ | GENERIC_WRITE	// read/write types
		,0								// comm devices must be opened with exclusive access
		,NULL							// no security attributes
		,OPEN_EXISTING					// comm devices must use OPEN_EXISTING
		,FILE_FLAG_OVERLAPPED|FILE_ATTRIBUTE_NORMAL	 // Async I/O
		,NULL);							// template must be 0 for comm devices

	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		// port not found
		LeaveCriticalSection(&m_csCommunicationSync);
		return FALSE;
	}

	// set the timeout values
	m_CommTimeouts.ReadIntervalTimeout = 1000;
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;

	// configure
	if (SetCommTimeouts(m_hComm, &m_CommTimeouts))
	{						   
		if (SetCommMask(m_hComm, m_dwCommEvents)){
			if (GetCommState(m_hComm, &m_dcb)){
				m_dcb.EvtChar = 'q';
				m_dcb.fRtsControl = RTS_CONTROL_ENABLE;		// set RTS bit high!
				if (BuildCommDCB(szBaud, &m_dcb)){
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
	// flush the port
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	// release critical section
	LeaveCriticalSection(&m_csCommunicationSync);
	//printf("Initialisation for communicationport %d completed.\nUse Startmonitor to communicate.\n", portnr);
	m_bConnectRTUOK = true;
	return TRUE;
}

int CSerialPortCtrl::GetErrorCount()
{
	return m_nErrorCount;
}
