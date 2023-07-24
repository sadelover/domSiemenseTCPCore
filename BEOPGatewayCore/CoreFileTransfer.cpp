#include "StdAfx.h"
#include "CoreFileTransfer.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/Util/UtilsFile.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "../BEOPDataPoint/sqlite/SqliteAcess.h"
#include "sstream"


CCoreFileTransfer::CCoreFileTransfer(CBEOPDataAccess *pAccess, int nPort)
	:m_nPort(nPort)
	,m_pDataAccess(pAccess)
{
	m_bIsTransmitting = FALSE;
	m_bIsStop = FALSE;

	m_hThreadListen = NULL;
	m_hThreadSend = NULL;

	m_bExitThread = false;

	m_bNewFileRecved = false;
	m_bFileSent = false;

	m_bRecvS3DB = true;
}



CCoreFileTransfer::~CCoreFileTransfer(void)
{
}


void CCoreFileTransfer::PrintLog(const wstring &strLog)
{
	_tprintf(strLog.c_str());
	
	if(m_pDataAccess)
		m_pDataAccess->InsertLog(strLog);
}


void CCoreFileTransfer::SendFile(CSocket &senSo)
{
	m_bIsTransmitting = TRUE;

	//打开要发送的文件
	CFile file;
	if(!file.Open(m_strPath, CFile::modeRead | CFile::typeBinary))
	{
		PrintLog(L"ERROR: CCoreFileTransfer::SendFile Failed.");

		senSo.Close();

		return ;
	}


	int nSize = 0, nLen = 0;
	DWORD dwCount = 0;
	char buf[BLOCKSIZE] = {0};
	file.Seek(0, CFile::begin);
	//开始传送文件
	for(;;)
	{
		//一次读取BLOCKSIZE大小的文件内容
		nLen = file.Read(buf, BLOCKSIZE);
		if(nLen == 0)
			break;

		//发送文件内容
		nSize = senSo.Send(buf, nLen);

		dwCount += nSize;
		CString strTransfersSize;
		strTransfersSize.Format(L"%ld BYTE", dwCount);


		if(m_bIsStop)
		{
			m_bIsStop = FALSE;
			break;
		}

		if(nSize == SOCKET_ERROR)
			break;
	}
	
	//关闭文件
	file.Close();
	//关闭套接字
	senSo.Close();

	m_pDataAccess->WriteCoreDebugItemValue(L"filerecved", L"finish");
	m_pDataAccess->WriteCoreDebugItemValue(L"pointupload", L"0");

	m_bIsTransmitting = FALSE;	
	m_bFileSent = true;
}


bool CCoreFileTransfer::ReceiveFile(CSocket &recSo)//文件接收处理
{
	m_bIsTransmitting = TRUE;
	PrintLog(L"Starting DB File Download...\r\n");
	int nSize = 0;
	DWORD dwCount = 0;
	char buf[BLOCKSIZE] = {0};	

	wstring strDir;
	Project::Tools::GetSysPath(strDir);
	
	CString strNewFileName, strNewFilePath;
	if(m_bRecvS3DB)
	{
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		strNewFileName.Format(L"BEOPDATA%s.s3db",tnow.Format(L"%Y%m%d%H%M%S"));
		strNewFilePath.Format(L"%s\\DBFileVersion\\%s",strDir.c_str(), strNewFileName);

	}
	else
		strNewFilePath =m_strPath;

	CFile file(strNewFilePath, CFile::modeCreate|CFile::modeWrite);
	//开始接收文件
	CString strTemp;
	bool bfailed = false;
	for(int i = 0;;i++)
	{
		//每次接收BLOCKSIZE大小的文件内容
		nSize = recSo.Receive(buf, BLOCKSIZE);
		if(nSize == 0)
			break;

		if(dwCount>1024*256*1000 || i>=20000)
		{
			bfailed = true;
			break;
		}
		//将接收到的文件写到新建的文件中去
		file.Write(buf, nSize);
		dwCount += nSize;

		//用户是否要停止接收
		if(m_bIsStop)
		{
			bfailed = true;
			m_bIsStop = FALSE;
			break;
		}
		
	//	strTemp.Format(L"接收到%d字节\r\n", dwCount);

		if(i%10==0 && m_pDataAccess)
			m_pDataAccess->WriteCoreDebugItemValue(L"filerecved", to_wstring((long long)dwCount));

	}
	//关闭文件
	file.Close(); 
	//关闭套接字
	recSo.Close();


	if(!bfailed)
	{
		if(TestDBFile(strNewFilePath.GetString()))
		{
			strTemp.Format(L"Download Success(File Size:%.2f MB)\r\n", dwCount/1000.f/1000.f);
			PrintLog(strTemp.GetString());
			//文件拷贝
			CString strSourcePath = strNewFilePath;
			CString strDisPath;
			strDisPath.Format(L"%s\\%s",strDir.c_str(), strNewFileName);

			
			if(CopyFile(strSourcePath.GetString(),strDisPath.GetString(),FALSE))
			{
				PrintLog(L"DB File Copy Success...\r\n");
			}
			else
			{
				PrintLog(L"DB File Copy Failed ERROR!!!\r\n");

			}
			Sleep(3000);

			m_bIsTransmitting = FALSE;	
			m_bNewFileRecved = true;
			return true;
		}
		else
		{
			PrintLog(L"Download Failed (File Check failed) ERROR!!!\r\n");
			m_bIsTransmitting = FALSE;
			m_bNewFileRecved = false;
		}
	}
	else
	{
		PrintLog(L"Download Failed (Timeout)\r\n");
		m_bIsTransmitting = FALSE;
		m_bNewFileRecved = false;
	}

	return false;

}

bool CCoreFileTransfer::Exit()
{
	m_bExitThread = true;
	
	::WaitForSingleObject(m_hThreadListen, INFINITE);

	return true;
}

void CCoreFileTransfer::StartAsSender(CString strIPConnectTo, int nPortConnectTo)
{
	m_strIPConnectTo = strIPConnectTo;
	m_nPortConnectTo = nPortConnectTo;
	m_hThreadListen = CreateThread(NULL, 0, _SendThread, this, 0, NULL); 
}


void CCoreFileTransfer::StartAsReceiver()
{
	m_hThreadSend = CreateThread(NULL, 0, _ListenThread, this, 0, NULL); 
}


//监听接收文件线程
DWORD WINAPI CCoreFileTransfer::_ListenThread( LPVOID lpParameter )
{
	CCoreFileTransfer *pTransfer = (CCoreFileTransfer *) lpParameter;
	//创建套接字
	while(!pTransfer->m_bExitThread)
	{

		CSocket sockSrvr;
		if(!sockSrvr.Create(pTransfer->m_nPort))
		{
			return -1;
		}

		//开始监听
		if(!sockSrvr.Listen())
		{
			return -1;
		}

		//接受连接
		CSocket recSo;
		if(!sockSrvr.Accept(recSo))
		{
			return -1;
		}

		sockSrvr.Close();

		if(pTransfer->ReceiveFile(recSo))
			return 0;

		Sleep(2000);

	}


	return 0;
}


DWORD WINAPI CCoreFileTransfer::_SendThread( LPVOID lpParameter )
{
	CCoreFileTransfer *pTransfer = (CCoreFileTransfer *) lpParameter;

	//创建客户端套接字
	CSocket sockClient;
	if(!sockClient.Create())
	{
		return -1;
	}
	//与服务器建立连接

	if(!sockClient.Connect(pTransfer->m_strIPConnectTo, pTransfer->m_nPortConnectTo))
	{
		return -1;
	}

	pTransfer->SendFile(sockClient);

	return 0;
}


bool CCoreFileTransfer::TestDBFile(wstring strFilePath)
{

	string strUtf8;
	Project::Tools::WideCharToUtf8(strFilePath, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	sqlstream << "select count(*) from list_point;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	int nCount = 0;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		nCount = access.getColumn_Int(0);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	if(nCount > 0)
		return true;
	return false;
}
