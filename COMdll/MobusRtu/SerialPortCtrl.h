//_ **********************************************************
//_ 
//_ Name: SerialPort.h : header file
//_ brief	: Serial Port communication
//			
//_ Purpose: 
//_ Author: Zhu Huawei  z_hw66@sohu.com
//-
//_ Copyright (c) 2009 
//_ 
//_ **********************************************************
#ifndef __SERIALPORTCTRL_H__
#define __SERIALPORTCTRL_H__
#pragma  once

#include "../ComInterface.h"

#define WM_COMM_BREAK_DETECTED		WM_USER+1	// A break was detected on input.
#define WM_COMM_CTS_DETECTED		WM_USER+2	// The CTS (clear-to-send) signal changed state. 
#define WM_COMM_DSR_DETECTED		WM_USER+3	// The DSR (data-set-ready) signal changed state. 
#define WM_COMM_ERR_DETECTED		WM_USER+4	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define WM_COMM_RING_DETECTED		WM_USER+5	// A ring indicator was detected. 
#define WM_COMM_RLSD_DETECTED		WM_USER+6	// The RLSD (receive-line-signal-detect) signal changed state. 
#define WM_COMM_RXCHAR				WM_USER+7	// A character was received and placed in the input buffer. 
#define WM_COMM_RXFLAG_DETECTED		WM_USER+8	// The event character was received and placed in the input buffer.  
#define WM_COMM_TXEMPTY_DETECTED	WM_USER+9	// The last character in the output buffer was sent.  

class CSerialPortCtrl:public CComBase
{
public:
	// contruction and destruction
	CSerialPortCtrl();
	virtual	~CSerialPortCtrl();

protected:
	// protected memberfunctions
	void ProcessErrorMessage(char* ErrorText);
	//void ProcessErrorMessage(CString strErrorText);
	static UINT WINAPI CommThread(LPVOID pParam);
	static void	ReceiveChar(CSerialPortCtrl *port, COMSTAT comstat);
	static void	WriteChar(CSerialPortCtrl *port);

public:
	/*BOOL InitPort(CWnd *pPortOwner, UINT portnr = 1, UINT baud = 19200, char parity = 'E', UINT databits = 8
		, UINT stopbits = 1, DWORD dwCommEvents = EV_RXCHAR, UINT writebuffersize = 1024);*/
	BOOL InitPort(UINT portnr = 1, UINT baud = 19200, char parity = 'N', UINT databits = 8
		, UINT stopbits = 1, DWORD dwCommEvents = EV_RXCHAR, UINT writebuffersize = 1024);

	void ClosePort(int nCode=0);

	// start/stop comm watching
	BOOL	StartMonitoring();
	BOOL	RestartMonitoring();
	BOOL	StopMonitoring();

	DWORD	GetWriteBufferSize();
	DWORD	GetCommEvents();
	DCB		GetDCB();

	void	WriteToPort(char *pData);
	void	WriteToPort(char *pData,int nLen);
	void	WriteToPort(LPCTSTR string);
	void	WriteToPort(LPCTSTR string, int nLen);

	bool	GetReceiveEvent();
	bool	SetReceiveEvent(bool bReceived);			//设置是否读串口内容
	 virtual bool OnCommunication(const unsigned char* pRevData, DWORD length);

	bool	ReConnect();
	int		GetErrorCount();
private:
	// thread
	HANDLE   m_hThread;

	// Event array. 
	// One element is used for each event. There are two event handles for each port.
	// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
	// There is a general shutdown when the port is closed. 
	HANDLE		m_hEventArray[3];
	// handles
	HANDLE		m_hWriteEvent;
	HANDLE		m_hShutdownEvent;

	// synchronization objects
	CRITICAL_SECTION m_csCommunicationSync;
	BOOL			 m_bThreadAlive;
	bool			m_bReceived;					//接收字符串

	// structures
	OVERLAPPED		 m_ov;
	COMMTIMEOUTS	 m_CommTimeouts;
	DCB				 m_dcb;

	// misc
	DWORD			 m_dwCommEvents;
	DWORD			 m_nWriteBufferSize;
	char			 *m_szWriteBuffer;
	int				 m_nWriteSize; 

public:
	// port initialization											
	HANDLE			 m_hComm;
	bool			 m_bConnectRTUOK;	
	int				 m_nErrorCount;
	UINT			 m_nPortNr;
	UINT			 m_nBaud;
	UINT			 m_nDatabits;
	UINT			 m_nStopbits;
	char			 m_cParity;
};
#endif //__SERIALPORTCTRL_H__


