// ClientSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ClientSocket.h"
#include "../FileTransfer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket(CFileTransfer *pFileTransfer)
	:m_pFileTransfer(pFileTransfer)
{

	m_pFile = NULL;
	m_pArchiveIn = NULL;
	m_pArchiveOut = NULL;
}

CClientSocket::~CClientSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CSocket)
	//{{AFX_MSG_MAP(CClientSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions

//串行化的初始化
void CClientSocket::Init()
{
	m_pFile = new CSocketFile(this);
	m_pArchiveIn = new CArchive(m_pFile,CArchive::load);
	m_pArchiveOut = new CArchive(m_pFile,CArchive::store);
}

void CClientSocket::Abort()
{
	if (m_pArchiveOut != NULL)
	{
		m_pArchiveOut->Abort();
		delete m_pArchiveOut;
		m_pArchiveOut = NULL;
	}
}

BOOL CClientSocket::SendMsg(CMessage *pMsg)
{
	if (m_pArchiveOut != NULL)
	{
		TRY
		{
			pMsg->Serialize(*m_pArchiveOut);
			m_pArchiveOut->Flush();

			return TRUE;
		}
		CATCH(CFileException, e)
		{
			m_pArchiveOut->Abort();
			delete m_pArchiveOut;
			m_pArchiveOut = NULL;
		}
		END_CATCH
	}
	
	return FALSE;
}

void CClientSocket::ReceiveMsg(CMessage *pMsg)
{
	//pMsg->Serialize(*m_pArchiveIn);
}

void CClientSocket::OnReceive(int nErrorCode) 
{
//	m_pFileTransfer->ProcessReceive(this);
	CSocket::OnReceive(nErrorCode);
}