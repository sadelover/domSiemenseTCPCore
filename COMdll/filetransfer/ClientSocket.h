#if !defined(AFX_CLIENTSOCKET_H__E3382998_1502_46F5_B0EA_C52542E7F4F5__INCLUDED_)
#define AFX_CLIENTSOCKET_H__E3382998_1502_46F5_B0EA_C52542E7F4F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClientSocket.h : header file
//

#include "Message.h"
#include "afxsock.h"

/////////////////////////////////////////////////////////////////////////////
// CClientSocket command target
class CFileTransfer;
class CClientSocket : public CSocket
{
// Attributes
public:

// Operations
public:
	CClientSocket(CFileTransfer *pFileTransfer);
	virtual ~CClientSocket();

// Overrides
public:
	CSocketFile* m_pFile;
	CArchive* m_pArchiveIn;
	CArchive* m_pArchiveOut;

	void Init();
	void Abort();

	BOOL SendMsg(CMessage* pMsg);
	void ReceiveMsg(CMessage* pMsg);
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientSocket)
	public:
	virtual void OnReceive(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CClientSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:

	CFileTransfer * m_pFileTransfer;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTSOCKET_H__E3382998_1502_46F5_B0EA_C52542E7F4F5__INCLUDED_)
