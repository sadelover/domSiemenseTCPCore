#include "stdafx.h"
#include "TCPDataSender.h"
#include "PackageTypeDefine.h"
#include "Tools/zip/zlib.h"
#include "Tools/Zip/unzip.h"
#pragma comment(lib,"zlib.lib")
#include "Tools/aes/AES256.h"

CTCPDataSender::CTCPDataSender(wstring wstrTCPIP/*=L"beopdata.rnbtech.com.hk"*/, wstring wstrTCPPort/*=L"9500"*/, wstring wstrTCPName/*=L""*/, wstring wstrTCPSendFilePackageSize/*=L"1000"*/, wstring wstrTCPSendFilePackageInterval/*=L"2000"*/, wstring wstrTCPAES256)
	: m_wstrTCPIP(wstrTCPIP)
	, m_wstrTCPPort(wstrTCPPort)
	, m_wstrTCPName(wstrTCPName)
	, m_wstrTCPSendFilePackageSize(wstrTCPSendFilePackageSize)
	, m_wstrTCPSendFilePackageInterval(wstrTCPSendFilePackageInterval)
	, m_sockTCP(NULL)
	, m_hTCPReceiveHandler(NULL)
	, m_hAnalyzeHandler(NULL)
	, m_hReleaseBufferHandler(NULL)
	, m_bTCPConnectSuccess(false)
	, m_bExitThread(false)
	, m_nTCPErrorCount(0)
	, m_szWriteBuffer(NULL)
	, m_nWriteSize(0)
	, m_lpRecDataProc(NULL)
	, m_dwUserData(0)
	, m_nDTUFilePackageNumber(0)
	, m_cFileUpdateBuffer(NULL)
	, m_strCoreFileName("")
	, m_nCoreFileCount(0)
	, m_nRecCoreFileCount(0)
	, m_nLastUpdateExeSize(0)
	, m_nUpdateFileSize(MAX_UPDATE_SIZE)
	, m_bAES256(_wtoi(wstrTCPAES256.c_str()))
{
	ZeroMemory(m_cRecvBuf, g_nRecMaxNum);
	m_nRecSize = 0;
	ZeroMemory(m_cSpiltBuffer, g_nRecMaxNum);
	ZeroMemory(m_cAnzlyzeBuff, g_nRecMaxNum);
	m_nAnzlyzeCount = 0;
	m_oleTCPUpdateTime = COleDateTime::GetCurrentTime();
	m_oleUpdateFileTime = COleDateTime::GetCurrentTime();
	// 初始化临界区
	InitializeCriticalSection(&m_csDTUDataSync);
}

CTCPDataSender::~CTCPDataSender()
{

}

bool CTCPDataSender::Init(LPRecDataProc proc1, DWORD userdata)
{
	// Initialize the winsock2.
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != NO_ERROR)
	{
		PrintLog(L"ERROR: Initialize TCPDataSender Winsock 2 fail.\r\n");
		return false;
	}

	// 设置读写缓存
	if (m_szWriteBuffer != NULL)
	{
		delete[]m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	m_szWriteBuffer = new char[g_nRecMaxNum];
	m_dwUserData = userdata;
	m_lpRecDataProc = proc1;

	if (m_bAES256)
	{
		string strAESPsw = AES_PSW;
		AES256::GetInstance()->Init((char*)strAESPsw.c_str());
	}
	//
	CString strDebugInfo;
	strDebugInfo.Format(_T("INFO : Init TCPDataSender(Host:%s, Port:%d, Tag:%s).\r\n"), m_wstrTCPIP.c_str(),_wtoi(m_wstrTCPPort.c_str()),m_wstrTCPName.c_str());
	PrintLog(strDebugInfo.GetString());

	return true;
}

bool CTCPDataSender::Exit()
{
	DisConnect();
	WSACleanup();
	if (m_bAES256)
	{
		AES256::GetInstance()->Exit();
	}
	return true;
}

void CTCPDataSender::PrintLog(const wstring & strLog)
{
	_tprintf(strLog.c_str());
}

bool CTCPDataSender::ReConnect()
{
	//自己判断需不需要重连
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleTCPUpdateTime;
	if (m_bTCPConnectSuccess == false || m_nTCPErrorCount >= 5 || oleSpan.GetTotalMinutes() >= 30)				//连续错误达到5次 或者30分钟内未成功收发过一次需要重连
	{
		DisConnect();
		if (TCPConnect(Project::Tools::WideCharToAnsi(m_wstrTCPIP.c_str()), _wtoi(m_wstrTCPPort.c_str())))
		{
			PrintLog(L"INFO : TCPDataSender Connect Success.\r\n");
			return true;
		}
		else
		{
			PrintLog(L"ERROR: TCPDataSender Connect Fail.\r\n");
			return false;
		}
	}
	return false;
}

bool CTCPDataSender::DisConnect()
{
	m_bTCPConnectSuccess = false;
	m_bExitThread = true;
	if (m_hTCPReceiveHandler)
	{
		WaitForSingleObject(m_hTCPReceiveHandler, 2000);
		CloseHandle(m_hTCPReceiveHandler);
		m_hTCPReceiveHandler = NULL;
	}

	if (m_hAnalyzeHandler)
	{
		WaitForSingleObject(m_hAnalyzeHandler, 2000);
		CloseHandle(m_hAnalyzeHandler);
		m_hAnalyzeHandler = NULL;
	}

	if (m_hReleaseBufferHandler)
	{
		WaitForSingleObject(m_hReleaseBufferHandler, 2000);
		CloseHandle(m_hReleaseBufferHandler);
		m_hReleaseBufferHandler = NULL;
	}

	if (m_sockTCP)
	{
		shutdown(m_sockTCP, SD_BOTH);
		closesocket(m_sockTCP);
		m_sockTCP = NULL;
	}

	// 设置读写缓存
	if (m_szWriteBuffer != NULL)
	{
		delete[]m_szWriteBuffer;
		m_szWriteBuffer = NULL;
	}
	return true;
}

bool CTCPDataSender::SendFile(string strFilePath)
{
	if (m_bTCPConnectSuccess)
	{
		if (strFilePath.length() > 0)
		{
			if (Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strFilePath.c_str())))		//文件存在
			{
				//读文件
				gzFile gz = gzopen(strFilePath.c_str(), "rb");
				if (!gz)
				{ 
					PrintLog(L"ERROR: TCPDataSender SendFile Fail(Open File Fail)\r\n");
					return false;
				}
					
				char **cFile = new char *[FILE_SIZE];
				for (int i = 0;i < FILE_SIZE;++i)
				{
					cFile[i] = new char[ONE_FILE_SIZE];
				}

				int nLastLen = 0;
				int nCount = 0;
				//开始传送文件
				for (;;)
				{
					//一次读取BLOCKSIZE大小的文件内容
					nLastLen = gzread(gz, cFile[nCount], _wtoi(m_wstrTCPSendFilePackageSize.c_str()));
					cFile[nCount][nLastLen] = ';';			//加一个分号
					nCount++;
					if (nLastLen < _wtoi(m_wstrTCPSendFilePackageSize.c_str()))
						break;
				}
				//关闭文件
				gzclose(gz);

				int pos = strFilePath.find_last_of('/');
				string strFileName(strFilePath.substr(pos + 1));

				//开始
				bool bResult = true;
				std::ostringstream sqlstream, sqlstream_index;
				sqlstream.str("");
				sqlstream << DTU_FILE_TAG << "0|" << strFileName << ";";
				string strdata = sqlstream.str();

				bResult = TCPSendPackageByWrap((char*)strdata.data(), strdata.size());
				if(bResult)
				{
					Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));

					sqlstream.str("");
					sqlstream << DTU_FILE_TAG << "2|";
					string strSend = sqlstream.str();
					for (int i = 0;i < nCount; ++i)
					{
						char cSend[ONE_FILE_SIZE] = { 0 };
						memcpy(cSend, strSend.data(), 6);
						unsigned int length = 0;
						if (i == nCount - 1)
						{
							length = nLastLen + 1;
						}
						else
						{
							length = _wtoi(m_wstrTCPSendFilePackageSize.c_str()) + 1;
						}
						memcpy(cSend + 11, &length, 4);
						if (m_nDTUFilePackageNumber == 59)			//因为长度位置会变成; 影响解析
							m_nDTUFilePackageNumber++;
						memcpy(cSend + 6, &m_nDTUFilePackageNumber, 4);
						sqlstream_index << m_nDTUFilePackageNumber << ",";
						m_nDTUFilePackageNumber++;
						if (m_nDTUFilePackageNumber >= 1000)
						{
							m_nDTUFilePackageNumber = 0;
						}

						cSend[10] = '|';
						cSend[15] = '|';
						memcpy(cSend + 16, cFile[i], length);
						bResult = TCPSendPackageByWrap(cSend, length + 16);
						if (!bResult)
						{
							break;
						}
						Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));
					}

					if (bResult)
					{
						//结束
						char cSend1[256] = { 0 };
						sqlstream.str("");
						sqlstream << DTU_FILE_TAG << "1|";
						strSend = sqlstream.str();

						memcpy(cSend1, strSend.data(), 6);
						memcpy(cSend1 + 11, &nLastLen, 4);
						memcpy(cSend1 + 6, &nCount, 4);

						string strFileIndex = sqlstream_index.str();
						strFileIndex.erase(--strFileIndex.end());
						strFileIndex += ";";
						memcpy(cSend1 + 16, strFileIndex.data(), strFileIndex.size());
						cSend1[10] = '|';
						cSend1[15] = ';';

						bResult = TCPSendPackageByWrap(cSend1, 16 + strFileIndex.size());
					}
				}

				//释放
				for (int i = 0;i < FILE_SIZE;++i)
				{
					delete cFile[i];
					cFile[i] = NULL;
				}
				delete[ONE_FILE_SIZE]cFile;
				cFile = NULL;
				return bResult;
			}
			else
			{
				PrintLog(L"ERROR: TCPDataSender SendFile Fail(File is UnExist)\r\n");
			}
		}
		else
		{
			PrintLog(L"ERROR: TCPDataSender SendFile Fail(Path is Null)\r\n");
		}
	}
	return false;
}

bool CTCPDataSender::SendRealDataFile(string strFilePath, int nPointCount)
{
	if (m_bTCPConnectSuccess)
	{
		if (strFilePath.length() > 0)
		{
			if (Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strFilePath.c_str())))		//文件存在
			{
				//读文件
				gzFile gz = gzopen(strFilePath.c_str(), "rb");
				if (!gz)
				{
					PrintLog(L"ERROR: TCPDataSender SendRealDataFile Fail(Open File Fail)\r\n");
					return false;
				}

				char **cFile = new char *[FILE_SIZE];
				for (int i = 0;i < FILE_SIZE;++i)
				{
					cFile[i] = new char[ONE_FILE_SIZE];
				}

				int nLastLen = 0;
				int nCount = 0;
				//开始传送文件
				for (;;)
				{
					//一次读取BLOCKSIZE大小的文件内容
					nLastLen = gzread(gz, cFile[nCount], _wtoi(m_wstrTCPSendFilePackageSize.c_str()));
					cFile[nCount][nLastLen] = ';';			//加一个分号
					nCount++;
					if (nLastLen < _wtoi(m_wstrTCPSendFilePackageSize.c_str()))
						break;
				}
				//关闭文件
				gzclose(gz);

				int pos = strFilePath.find_last_of('/');
				string strFileName(strFilePath.substr(pos + 1));

				//开始
				bool bResult = true;
				std::ostringstream sqlstream, sqlstream_index;
				sqlstream.str("");
				sqlstream << DTU_FILE_REAL_TAG << "0|" << strFileName << ";" << nPointCount << ";";
				string strdata = sqlstream.str();
				bResult = TCPSendPackageByWrap((char*)strdata.data(), strdata.size());
				if (bResult)
				{
					Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));

					sqlstream.str("");
					sqlstream << DTU_FILE_REAL_TAG << "2|";
					string strSend = sqlstream.str();

					for (int i = 0;i < nCount; ++i)
					{
						char cSend[ONE_FILE_SIZE] = { 0 };
						memcpy(cSend, strSend.data(), 6);
						unsigned int length = 0;
						if (i == nCount - 1)
						{
							length = nLastLen + 1;
						}
						else
						{
							length = _wtoi(m_wstrTCPSendFilePackageSize.c_str()) + 1;
						}
						memcpy(cSend + 11, &length, 4);
						if (m_nDTUFilePackageNumber == 59)					//因为长度位置会变成; 影响解析
							m_nDTUFilePackageNumber++;
						memcpy(cSend + 6, &m_nDTUFilePackageNumber, 4);
						sqlstream_index << m_nDTUFilePackageNumber << ",";
						m_nDTUFilePackageNumber++;
						if (m_nDTUFilePackageNumber >= 1000)
						{
							m_nDTUFilePackageNumber = 0;
						}

						cSend[10] = '|';
						cSend[15] = '|';
						memcpy(cSend + 16, cFile[i], length);
						bResult = TCPSendPackageByWrap(cSend, length + 16);
						if (!bResult)
						{
							break;
						}
						Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));
					}
					if (bResult)
					{
						//结束
						char cSend1[256] = { 0 };
						sqlstream.str("");
						sqlstream << DTU_FILE_REAL_TAG << "1|";
						strSend = sqlstream.str();
						memcpy(cSend1, strSend.data(), 6);
						memcpy(cSend1 + 11, &nLastLen, 4);
						memcpy(cSend1 + 6, &nCount, 4);

						string strFileIndex = sqlstream_index.str();
						strFileIndex.erase(--strFileIndex.end());
						strFileIndex += ";";
						memcpy(cSend1 + 16, strFileIndex.data(), strFileIndex.size());
						cSend1[10] = '|';
						cSend1[15] = ';';

						bResult = TCPSendPackageByWrap(cSend1, 16 + strFileIndex.size());
						Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));
					}
				}
				//释放
				for (int i = 0;i < FILE_SIZE;++i)
				{
					delete cFile[i];
					cFile[i] = NULL;
				}
				delete[ONE_FILE_SIZE]cFile;
				cFile = NULL;
				return bResult;
			}
			else
			{
				PrintLog(L"ERROR: TCPDataSender SendRealDataFile Fail(File is UnExist)\r\n");
			}
		}
		else
		{
			PrintLog(L"ERROR: TCPDataSender SendRealDataFile Fail(Path is Null)\r\n");
		}
	}
	return false;
}

bool CTCPDataSender::SendHistoryDataFile(string strFilePath, int nPointCount)
{
	if (m_bTCPConnectSuccess)
	{
		if (strFilePath.length() > 0)
		{
			if (Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strFilePath.c_str())))		//文件存在
			{
				//读文件
				gzFile gz = gzopen(strFilePath.c_str(), "rb");
				if (!gz)
				{
					PrintLog(L"ERROR: TCPDataSender SendHistoryDataFile Fail(Open File Fail)\r\n");
					return false;
				}

				char **cFile = new char *[FILE_SIZE];
				for (int i = 0;i < FILE_SIZE;++i)
				{
					cFile[i] = new char[ONE_FILE_SIZE];
				}

				int nLastLen = 0;
				int nCount = 0;
				//开始传送文件
				for (;;)
				{
					//一次读取BLOCKSIZE大小的文件内容
					nLastLen = gzread(gz, cFile[nCount], _wtoi(m_wstrTCPSendFilePackageSize.c_str()));
					cFile[nCount][nLastLen] = ';';			//加一个分号
					nCount++;
					if (nLastLen < _wtoi(m_wstrTCPSendFilePackageSize.c_str()))
						break;
				}
				//关闭文件
				gzclose(gz);

				int pos = strFilePath.find_last_of('/');
				string strFileName(strFilePath.substr(pos + 1));

				//开始
				bool bResult = true;
				std::ostringstream sqlstream, sqlstream_index;
				sqlstream.str("");
				sqlstream << DTU_FILE_HISTORY_TAG << "0|" << strFileName << ";" << nPointCount << ";";
				string strdata = sqlstream.str();
				bResult = TCPSendPackageByWrap((char*)strdata.data(), strdata.size());
				if (bResult)
				{
					Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));

					sqlstream.str("");
					sqlstream << DTU_FILE_HISTORY_TAG << "2|";
					string strSend = sqlstream.str();

					for (int i = 0;i < nCount; ++i)
					{
						char cSend[ONE_FILE_SIZE] = { 0 };
						memcpy(cSend, strSend.data(), 6);
						unsigned int length = 0;
						if (i == nCount - 1)
						{
							length = nLastLen + 1;
						}
						else
						{
							length = _wtoi(m_wstrTCPSendFilePackageSize.c_str()) + 1;
						}
						memcpy(cSend + 11, &length, 4);
						if (m_nDTUFilePackageNumber == 59)					//因为长度位置会变成; 影响解析
							m_nDTUFilePackageNumber++;
						memcpy(cSend + 6, &m_nDTUFilePackageNumber, 4);
						sqlstream_index << m_nDTUFilePackageNumber << ",";
						m_nDTUFilePackageNumber++;
						if (m_nDTUFilePackageNumber >= 1000)
						{
							m_nDTUFilePackageNumber = 0;
						}

						cSend[10] = '|';
						cSend[15] = '|';
						memcpy(cSend + 16, cFile[i], length);
						bResult = TCPSendPackageByWrap(cSend, length + 16);
						if (!bResult)
						{
							break;
						}
						Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));
					}
					if (bResult)
					{
						//结束
						char cSend1[256] = { 0 };
						sqlstream.str("");
						sqlstream << DTU_FILE_HISTORY_TAG << "1|";
						strSend = sqlstream.str();
						memcpy(cSend1, strSend.data(), 6);
						memcpy(cSend1 + 11, &nLastLen, 4);
						memcpy(cSend1 + 6, &nCount, 4);

						string strFileIndex = sqlstream_index.str();
						strFileIndex.erase(--strFileIndex.end());
						strFileIndex += ";";
						memcpy(cSend1 + 16, strFileIndex.data(), strFileIndex.size());
						cSend1[10] = '|';
						cSend1[15] = ';';

						bResult = TCPSendPackageByWrap(cSend1, 16 + strFileIndex.size());
						Sleep(_wtoi(m_wstrTCPSendFilePackageInterval.c_str()));
					}
				}
				//释放
				for (int i = 0;i < FILE_SIZE;++i)
				{
					delete cFile[i];
					cFile[i] = NULL;
				}
				delete[ONE_FILE_SIZE]cFile;
				cFile = NULL;
				return bResult;
			}
			else
			{
				PrintLog(L"ERROR: TCPDataSender SendHistoryDataFile Fail(File is UnExist)\r\n");
			}
		}
		else
		{
			PrintLog(L"ERROR: TCPDataSender SendHistoryDataFile Fail(Path is Null)\r\n");
		}
	}
	return false;
}

bool CTCPDataSender::GenerateLog(DTU_CMD_TYPE type, string strContent, bool bSendSuccess, string strRemark)
{ 
	std::ostringstream sqlstream;
	if (bSendSuccess)
	{
		sqlstream << "SEND : ";
	}
	else
	{
		sqlstream << "ERROR: ";
	}
	switch (type)
	{
	case DTU_CMD_DEFINE:
		break;
	case DTU_CMD_REAL_DATA_SEND:
	case DTU_CMD_REAL_DATA_SYN:
		{
			sqlstream << "RealData(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_REAL_DATA_EDIT:
	case DTU_CMD_REAL_DATA_EDIT_MUL:
		{
			sqlstream << "RealData Edit(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_WARNING_DATA_SEND:
	case DTU_CMD_WARNING_DATA_SYN:
		{
			sqlstream << "WarningData(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_WARNING_OPERATION_DATA_SEND:
	case DTU_CMD_WARNING_OPERATION_DATA_SYN:
		{
			sqlstream << "WarningOperationData(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_OPERATION_SEND:
	case DTU_CMD_OPERATION_SYN:
		{
			sqlstream << "OperationData(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_UNIT_SEND:
	case DTU_CMD_UNIT_SYN:
		{
			sqlstream << "Unit01(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_UNIT_EDIT:
	case DTU_CMD_UNIT_EDIT_MUL:
		{
			sqlstream << "Unit01 Edit(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_LOG_SEND:
	case DTU_CMD_LOG_SYN:
		{
			int pos = strContent.find_last_of('/');
			sqlstream << "MysqlFile(" << strContent.substr(pos + 1) << ")" << "\r\n";
		}
		break;
	case DTU_CMD_HEART_SEND:
	case DTU_CMD_HEART_SYN:
		{
			sqlstream << "HeartBeat(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_ERR_LIST:
		{
			sqlstream << "ERROR List(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_ERR_CODE:
		{
			sqlstream << "ERROR Code(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_ERROR_SEND:;
	case DTU_CMD_ERROR_SYN:
		{
			sqlstream << "ERROR Log(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_REAL_FILE_SEND:
	case DTU_CMD_REAL_FILE_SYN:
		{
			int pos = strContent.find_last_of('/');
			sqlstream << "RealFileData(" << strContent.substr(pos + 1) << ")" << "\r\n";
		}
		break;
	case DTU_CMD_HISTORY_FILE_SEND:
	case DTU_CMD_HISTORY_FILE_SYN:
		{
			int pos = strContent.find_last_of('/');
			sqlstream << "HisoryFileData(" << strContent.substr(pos + 1) << ")";
			if (strRemark.length() > 0)
			{
				sqlstream << "(" << strRemark << ")";
			}
			sqlstream << "\r\n";
		}
		break;
	case DTU_CMD_UPDATE_EXE:
		{
			sqlstream << "Update Exe(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_UPDATE_POINT_CSV:
		{
			sqlstream << "Update Point CSV(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_UPDATE_POINT_EXCEL:
		{
			sqlstream << "Update Point Excel(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_UPDATE_DLL:
		{
			sqlstream << "Update Point DLL(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_UPDATE_S3DB:
		{
			sqlstream << "Update Point S3db(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_UPLOAD_POINT_CSV:
		{
			sqlstream << "Upload Point CSV(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_DELETE_POINT_MUL:
		{
			sqlstream << "Delete Point(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_TIME_SYN:
		{
			sqlstream << "SynTime(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_RESTART_CORE:
		{
			sqlstream << "Restart Core(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_RESTART_DOG:
		{
			sqlstream << "Restart Dog(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_RESTART_DTU:
		{
			sqlstream << "Restart DTU(" << strContent.length() << ")" << "\r\n";
		}
		break;
	case DTU_CMD_CORE_STATUS:
		{
			sqlstream << "Core Status(" << strContent << ")" << "\r\n";
		}
		break;
	case DTU_CMD_VERSION_CORE:
	case DTU_CMD_VERSION_DOG:
	case DTU_CMD_VERSION_DTU:
		{
			sqlstream << "Version(" << strContent.length() << ")" << "\r\n";
		}
		break;
	default:
		{
			sqlstream << "Undefine(" << strContent.length() << ")" << "\r\n";
		}
		break;
	}
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));
	return false;
}

bool CTCPDataSender::SendString(string strContent)
{
	return TCPSendPackageByWrap((char*)strContent.data(), strContent.size());
}

bool CTCPDataSender::IsConnected()
{
	return m_bTCPConnectSuccess;
}

bool CTCPDataSender::TCPSendPackageByWrap(char * pData, int nLen)
{
	if (m_bTCPConnectSuccess)
	{
		nLen = nLen + 2;
		if (nLen >= 4050)
		{
			return false;
		}
		memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));
		memcpy(m_szWriteBuffer, pData, nLen);
		m_szWriteBuffer[nLen - 2] = ';';			//以;;\n为包结尾
		m_szWriteBuffer[nLen - 1] = '\n';
		m_nWriteSize = nLen;
		bool bResult = TCPSendPackage(m_szWriteBuffer, nLen);
		if (!bResult)
		{
			m_nTCPErrorCount++;
		}
		else
		{
			m_nTCPErrorCount = 0;
			m_oleTCPUpdateTime = COleDateTime::GetCurrentTime();
		}
		return bResult;
	}
	return false;
}

bool CTCPDataSender::TCPSendPackage(const char * pSrcPackage, unsigned int nSrcPackSize)
{
	if (m_bTCPConnectSuccess)
	{
		BOOL bTcpNoDelay = TRUE;
		DWORD dwTimeout = 2000;

		if (nSrcPackSize != 0)
		{
			if (m_bAES256)		//加密
			{
				unsigned long cLen = 0;
				char *cipherText = 0;

				if (!AES256::GetInstance()->Encrypt(&cipherText, &cLen, (char *)pSrcPackage, nSrcPackSize))
				{
					PrintLog(L"ERROR: TCPDataSender Encryption Fail!\r\n");
					return false;
				}
				_tprintf(L"INFO : TCPDataSender Encryption Old:%d New:%d\r\n", nSrcPackSize, cLen);

				//Set TCP/IP NODELAY
				setsockopt(m_sockTCP, IPPROTO_TCP, TCP_NODELAY, (char*)&bTcpNoDelay, sizeof(bTcpNoDelay));
				// Set the send timeout.
				setsockopt(m_sockTCP, SOL_SOCKET, SO_SNDTIMEO, (char*)&dwTimeout, sizeof(dwTimeout));

				// Send datas.
				if (send(m_sockTCP, cipherText, cLen/*+1*/, 0) == SOCKET_ERROR)
				{
					SAFE_FREE(cipherText);
					return false;
				}
				SAFE_FREE(cipherText);
				return true;
			}
			else
			{
				//Set TCP/IP NODELAY
				setsockopt(m_sockTCP, IPPROTO_TCP, TCP_NODELAY, (char*)&bTcpNoDelay, sizeof(bTcpNoDelay));
				// Set the send timeout.
				setsockopt(m_sockTCP, SOL_SOCKET, SO_SNDTIMEO, (char*)&dwTimeout, sizeof(dwTimeout));

				// Send datas.
				if (send(m_sockTCP, pSrcPackage, nSrcPackSize/*+1*/, 0) == SOCKET_ERROR)
				{
					//_tprintf(L"ERROR: TCPSendPackage Send datas fail.\r\n");
					return false;
				}
				return true;
			}
		}
	}
	return false;
}

bool CTCPDataSender::TCPConnect(const string & strHost, u_short uPort)
{
	m_bExitThread = false;
	m_bTCPConnectSuccess = false;

	if (strHost.length() <= 0 || uPort <= 0)
	{
		PrintLog(L"ERROR: TCPDataSender Address invalid.\r\n");
		return false;
	}

	SOCKADDR_IN sockAddrIn;
	memset(&sockAddrIn, 0, sizeof(SOCKADDR_IN));
	if (isalpha(strHost[0]))									//判断输入的是否域名
	{
		//查找主机
		LPHOSTENT lpHost = gethostbyname(strHost.c_str());
		if (lpHost == NULL)
		{
			return FALSE;
		}
		sockAddrIn.sin_family = AF_INET;
		sockAddrIn.sin_addr.s_addr = *((u_long FAR*)(lpHost->h_addr));
		sockAddrIn.sin_port = htons(uPort);
	}
	else
	{
		// Config the socket address information.
		sockAddrIn.sin_family = AF_INET;
		sockAddrIn.sin_addr.s_addr = inet_addr(const_cast<char*>(strHost.c_str()));
		sockAddrIn.sin_port = htons(uPort);
	}

	//创建Socket
	m_sockTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sockTCP == SOCKET_ERROR)
	{
		CString strDebugInfo;
		strDebugInfo.Format(_T("ERROR: Create TCPDataSender fail(error: %d).\r\n"), WSAGetLastError());
		PrintLog(strDebugInfo.GetString());
		DisConnect();
		return false;
	}

	//允许重用本地地址和端口:这样的好处是，即使socket断了，调用前面的socket函数也不会占用另一个，而是始终就是一个端口
	int nREUSEADDR = 1;
	setsockopt(m_sockTCP, SOL_SOCKET, SO_REUSEADDR, (const char *)&nREUSEADDR, sizeof(int));
	// closesocket 调用进入“锁定”状态（等待完成），不论是否有排队数据未发送或未被确认。这种关闭方式称为“强行关闭”
	linger m_sLinger;
	m_sLinger.l_onoff = 1;  // ( 在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	m_sLinger.l_linger = 0; // ( 容许逗留的时间为0秒)
	setsockopt(m_sockTCP, SOL_SOCKET, SO_LINGER, (const char *)&m_sLinger, sizeof(linger));

	//设置非阻塞方式连接
	unsigned long ul = 1;
	int  ret = ioctlsocket(m_sockTCP, FIONBIO, (unsigned long*)&ul);
	if (ret == SOCKET_ERROR)
	{
		CString strDebugInfo;
		strDebugInfo.Format(_T("ERROR: Set TCPDataSender socket fail(error: %d).\n"), WSAGetLastError());	
		PrintLog(strDebugInfo.GetString());
		DisConnect();
		return false;
	}

	// Connect to the assign ip address.
	connect(m_sockTCP, (sockaddr*)&sockAddrIn, sizeof(sockAddrIn));
	//select 模型，即设置超时
	struct timeval timeout;
	fd_set r;
	FD_ZERO(&r);
	FD_SET(m_sockTCP, &r);
	timeout.tv_sec = 10; //连接超时10秒
	timeout.tv_usec = 0;
	ret = select(0, 0, &r, 0, &timeout);
	if (ret <= 0)
	{
		CString strDebugInfo;
		strDebugInfo.Format(_T("ERROR: TCPDataSender connect time out(%d).\n"), WSAGetLastError());
		PrintLog(strDebugInfo.GetString());
		DisConnect();
		return false;
	}

	//设回阻塞模式
	unsigned long ul1 = 0;
	ret = ioctlsocket(m_sockTCP, FIONBIO, (unsigned long*)&ul1);
	if (ret == SOCKET_ERROR)
	{
		CString strDebugInfo;
		strDebugInfo.Format(_T("ERROR: TCPDataSender Set socket fail(error: %d).\n"), WSAGetLastError());
		PrintLog(strDebugInfo.GetString());
		DisConnect();
		return false;
	}

	//连接成功
	if(m_szWriteBuffer == NULL)
		m_szWriteBuffer = new char[g_nRecMaxNum];

	m_bExitThread = false;
	m_bTCPConnectSuccess = true;
	if(m_hTCPReceiveHandler == NULL)
		m_hTCPReceiveHandler = (HANDLE)_beginthreadex(NULL, 0, RecvThread, this, 0, NULL);
	if (m_hAnalyzeHandler == NULL)
		m_hAnalyzeHandler = (HANDLE)_beginthreadex(NULL, 0, AnalyzeRecvCmd, this, 0, NULL);
	if(m_hReleaseBufferHandler == NULL)
		m_hReleaseBufferHandler = (HANDLE)_beginthreadex(NULL, 0, DTUThreadReleaseUpdateSize, this, 0, NULL);
	return true;
}

bool CTCPDataSender::SendOneData(DTU_DATA_INFO & data)
{
	bool bSendResult = false;
	if (data.strFileName.length() > 0)				//文件数据  成功删除，失败则尝试计数+1
	{
		string strJson = data.strContent;
		if (strJson.length() > 0)
		{
			StringReplace(strJson, "\\", "/");				//转换		

			Json::Reader reader;
			Json::Value value;
			if (reader.parse(strJson, value))
			{
				Value arrayObj = value;
				DTU_CMD_TYPE type = DTU_CMD_DEFINE;
				int nPointCount = 0;
				string strFilePath = "";
				string strReamrk = "";
				if (!arrayObj["id"].isNull() && arrayObj["id"].isInt())
				{
					nPointCount = (DTU_CMD_TYPE)arrayObj["id"].asInt();
				}
				if (!arrayObj["type"].isNull() && arrayObj["type"].isInt())
				{
					type = (DTU_CMD_TYPE)arrayObj["type"].asInt();
				}

				if (!arrayObj["content"].isNull() && arrayObj["content"].isString())
				{
					strFilePath = arrayObj["content"].asString();
				}
				if (!arrayObj["remark"].isNull() && arrayObj["remark"].isString())
				{
					strReamrk = arrayObj["remark"].asString();
				}
				
				switch (type)
				{
				case DTU_CMD_REAL_FILE_SEND:
				case DTU_CMD_REAL_FILE_SYN:		//实时文件
				{
					bSendResult = SendRealDataFile(strFilePath, nPointCount);
					break;
				}
				case DTU_CMD_HISTORY_FILE_SEND:
				case DTU_CMD_HISTORY_FILE_SYN:	//历史文件
					bSendResult = SendHistoryDataFile(strFilePath, nPointCount);
					break;
				default:
					bSendResult = SendFile(strFilePath);
					break;
				}
				GenerateLog(type, strFilePath, bSendResult, strReamrk);
			}
		}
	}
	else
	{
		string strJson = data.strContent;
		if (strJson.length() > 0)
		{
			StringReplace(strJson, "\\", "/");				//转换		

			Json::Reader reader;
			Json::Value value;
			if (reader.parse(strJson, value))
			{
				Value arrayObj = value;
				DTU_CMD_TYPE type = DTU_CMD_DEFINE;
				string strStrungContent = "";
				string strReamrk = "";
				if (!arrayObj["type"].isNull() && arrayObj["type"].isInt())
				{
					type = (DTU_CMD_TYPE)arrayObj["type"].asInt();
				}

				if (!arrayObj["content"].isNull() && arrayObj["content"].isString())
				{
					strStrungContent = arrayObj["content"].asString();
				}

				if (!arrayObj["remark"].isNull() && arrayObj["remark"].isString())
				{
					strReamrk = arrayObj["remark"].asString();
				}

				bSendResult = SendString(strStrungContent);
				GenerateLog(type, strStrungContent, bSendResult, strReamrk);
			}
		}
	}
	return bSendResult;
}

SOCKET CTCPDataSender::GetTCPConnectedSocket()
{
	return m_sockTCP;
}

bool CTCPDataSender::HandleRecData(const char* pSrcBuffer, int nSize)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lockTCP);
	if (pSrcBuffer == NULL)
		return false;

	int nLength = m_nRecSize + nSize;
	if (nLength <= g_nRecMaxNum)
	{
		memcpy(m_cRecvBuf + m_nRecSize, pSrcBuffer, nSize);
		m_nRecSize = nLength;
	}
	else
	{
		ZeroMemory(m_cRecvBuf, g_nRecMaxNum);
		m_nRecSize = 0;

		memcpy(m_cRecvBuf, pSrcBuffer, nSize);
		m_nRecSize = nSize;
	}

	vector<TCP_DATA_PACKAGE> vecPackage;
	SplitStringSpecial(m_cRecvBuf, m_nRecSize, vecPackage);
	int nCount = vecPackage.size();
	for (int i = 0; i < nCount; ++i)
	{
		int length = vecPackage[i].nLength;
		if (length > 0)
		{
			char* p = vecPackage[i].cData;
			if (i < nCount - 1)
			{
				OnRecvData((unsigned char *)p, length);
			}
			else
			{
				if (m_cRecvBuf[m_nRecSize - 1] == '\n')
				{
					OnRecvData((unsigned char *)p, length);
					ZeroMemory(m_cRecvBuf, g_nRecMaxNum);
					m_nRecSize = 0;
				}
				else
				{
					ZeroMemory(m_cRecvBuf, g_nRecMaxNum);
					memcpy(m_cRecvBuf, p, length);
					m_nRecSize = length;
				}
			}
		}
	}
	return true;
}

void CTCPDataSender::SplitStringSpecial(const char * buffer, int nSize, std::vector<TCP_DATA_PACKAGE>& resultlist)
{
	if (nSize <= 0)
		return;

	resultlist.clear();
	memset(m_cSpiltBuffer, 0, g_nRecMaxNum);
	memcpy(m_cSpiltBuffer, buffer, nSize);

	char cBefore1 = 0x00;
	char cBefore2 = 0x00;
	int nBefore = 0;
	int nAfter = 0;
	for (int i = 0; i < nSize; ++i)
	{
		if (buffer[i] == '\n'  && cBefore2 == ';' && cBefore1 == ';')			//以;;\n结尾
		{
			TCP_DATA_PACKAGE data;
			int length = i + 1 - nBefore;
			length = (length > g_nPackageSize) ? g_nPackageSize : length;
			memcpy(data.cData, m_cSpiltBuffer + nBefore, length);
			data.nLength = length;
			resultlist.push_back(data);
			nBefore = i + 1;
		}
		cBefore2 = cBefore1;
		cBefore1 = buffer[i];
	}

	int nLength = nSize - nBefore;
	if (nLength > 0)
	{
		TCP_DATA_PACKAGE data;
		int length = (nLength > g_nPackageSize) ? g_nPackageSize : nLength;
		memcpy(data.cData, m_cSpiltBuffer + nBefore, length);
		data.nLength = length;
		resultlist.push_back(data);
	}
}

bool CTCPDataSender::OnRecvData(const unsigned char * pRevData, DWORD dwLength)
{
	TCP_DATA_PACKAGE data;
	memset(data.cData, 0, g_nPackageSize);
	dwLength = (dwLength > g_nPackageSize) ? g_nPackageSize : dwLength;
	memcpy(data.cData, pRevData, dwLength);
	data.nLength = dwLength;
	return StoreRecvData(data);
}

bool CTCPDataSender::StoreRecvData(TCP_DATA_PACKAGE& data)
{
	// 开始进入临界区
	EnterCriticalSection(&m_csDTUDataSync);
	m_queCmdPaket.push(data);
	LeaveCriticalSection(&m_csDTUDataSync);
	return true;
}

void CTCPDataSender::SplitString(const std::string& strsource, const char* sep, std::vector<string>& resultlist)
{
	if (!sep || strsource.length()<=0)
		return;
	resultlist.clear();
	memset(m_cSpiltBuffer, 0, MAX_REC_COUNT);
	memcpy(m_cSpiltBuffer, strsource.c_str(), strsource.length());
	char* token = NULL;
	char* nexttoken = NULL;
	token = strtok_s(m_cSpiltBuffer, sep, &nexttoken);
	while (token != NULL)
	{
		string str = token;
		resultlist.push_back(str);
		token = strtok_s(NULL, sep, &nexttoken);
	}
}

unsigned int CTCPDataSender::BufferToInt(const unsigned char * buffer)
{
	return buffer[0] + buffer[1] * 256 + buffer[2] * 65536 + buffer[3] * 256 * 65536;
}

bool CTCPDataSender::AnalyzeRecvData()
{
	while (!m_queCmdPaket.empty())
	{
		// 开始进入临界区
		EnterCriticalSection(&m_csDTUDataSync);
		bool bHasData = false;
		TCP_DATA_PACKAGE data;
		if (!m_queCmdPaket.empty())
		{
			data = m_queCmdPaket.front();
			m_queCmdPaket.pop();
			bHasData = true;
		}
		LeaveCriticalSection(&m_csDTUDataSync);

		if (bHasData)
		{
			char* RXBuff = data.cData;
			if (RXBuff != NULL)
			{
				int dwBytesRead = data.nLength;
				if (m_nAnzlyzeCount + dwBytesRead >= MAX_REC_COUNT)
				{
					memset(m_cAnzlyzeBuff, 0, sizeof(m_cAnzlyzeBuff));
					m_nAnzlyzeCount = 0;
				}

				//拷贝
				memcpy(m_cAnzlyzeBuff + m_nAnzlyzeCount, RXBuff, dwBytesRead);
				m_nAnzlyzeCount = m_nAnzlyzeCount + dwBytesRead;

				if (m_nAnzlyzeCount > 3)
				{
					if (m_cAnzlyzeBuff[m_nAnzlyzeCount - 1] == '\n' && m_cAnzlyzeBuff[m_nAnzlyzeCount - 2] == ';' && m_cAnzlyzeBuff[m_nAnzlyzeCount - 3] == ';')		//;;\n结尾
					{
						AnalyzeReceive((const unsigned char *)m_cAnzlyzeBuff, m_nAnzlyzeCount);
						memset(m_cAnzlyzeBuff, 0, sizeof(m_cAnzlyzeBuff));
						m_nAnzlyzeCount = 0;
					}
				}
			}
		}
	}
	return true;
}

bool CTCPDataSender::AnalyzeReceive(const unsigned char * pRevData, DWORD length)
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lockReveive);
	if (length <= 3)
		return false;

	//;;\n结尾
	if (pRevData[length - 1] == '\n' && pRevData[length - 2] == ';')
	{
		char buffer_copy[g_nPackageSize] = { 0 };
		length = length - 3;
		length = (length > g_nPackageSize) ? g_nPackageSize : length;
		memcpy(buffer_copy, pRevData, length);

		rnbtech::CPackageType::DTU_DATATYPE type = rnbtech::CPackageType::GetPackageType(buffer_copy);
		switch (type)
		{
			case rnbtech::CPackageType::Type_DTUServerCmd:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy, length);
				AnzlyzeString(buffer_copy);
			}
			break;
			case rnbtech::CPackageType::Type_DTUServerFup:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy, length);
				//处理接收升级文件
				AnalyzeUpdateFile((unsigned char*)buffer_copy);
			}
			break;
			case rnbtech::CPackageType::Type_DTUServerFrl:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy, length);
				//处理匹配文件接收
				AnalyzeRecLogFile((unsigned char*)buffer_copy);
			}
			break;
			case rnbtech::CPackageType::Type_DTUWriteTime:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy);
				//UpdateDTUWriteTime(buffer_copy);
			}
			break;
			default:
			{
				PrintLog(L"ERROR: TCPDataSender Analyze DTUServer Cmd Fail.\r\n");
			}
			break;
		}
		return true;
	}
	return false;
}

void CTCPDataSender::AnzlyzeString(char * pBuffer)
{
	char* pReveive = strtok((char*)pBuffer, ";");
	string strReveive = pReveive;
	//创建接收记录
	Json::Value jsonBody;
	jsonBody["type"] = 0;    //文件
	jsonBody["content"] = strReveive;
	string strContent = jsonBody.toStyledString();
	if (m_lpRecDataProc)
		m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
}

void CTCPDataSender::AnalyzeUpdateFile(const unsigned char * buffer)
{
	try
	{
		if (buffer != NULL)
		{
			if (buffer[0] == '0')		//开始
			{
				m_nCoreFileCount = 0;
				m_nLastUpdateExeSize = 0;
				m_mapFileUpdateState.clear();
				char cFileName[256] = { 0 };
				memcpy(cFileName, buffer + 2, 256);
				char* p = strtok(cFileName, "|");
				if (p)
				{
					CString str;
					str.Format(_T("RECV : TCPDataSender Receive Update File Package:%s...\r\n"), Project::Tools::AnsiToWideChar(p).c_str());
					PrintLog(str.GetString());
					m_strCoreFileName = p;

					p = strtok(NULL, ";");
					if (p)
					{
						m_nCoreFileCount = ATOI(p);
						if (m_nCoreFileCount<0 || m_nCoreFileCount>MAX_UPDATE_SIZE)
							return;

						m_nUpdateFileSize = m_nCoreFileCount + 10;
						for (int i = 0; i < m_nCoreFileCount; ++i)
						{
							m_mapFileUpdateState[i] = 0;
						}
					}
				}
				m_nRecCoreFileCount = 0;
			}
			else if (buffer[0] == '1')
			{
				m_nCoreFileCount = BufferToInt(buffer + 2);
				m_nLastUpdateExeSize = BufferToInt(buffer + 7);
				if (m_nCoreFileCount == m_nRecCoreFileCount)
				{
					GenerateUpdateFile();
				}
				else		//有丢包   启动线程去像服务器要丢失的包  直到接收时间超过1h，本次更新失败
				{
					//待处理
					
				}
			}
			else if (buffer[0] == '2')
			{
				int nPackageIndex = BufferToInt(buffer + 2);
				int nLength = BufferToInt(buffer + 7);
				if (nPackageIndex >= 0 && nPackageIndex < MAX_UPDATE_SIZE && nLength <= g_nPackageSize)
				{
					m_nUpdateFileSize = (nPackageIndex >= m_nUpdateFileSize) ? nPackageIndex : m_nUpdateFileSize;
					CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize);
					memset(m_cFileUpdateBuffer[nPackageIndex], 0, sizeof(m_cFileUpdateBuffer[nPackageIndex]));
					memcpy(m_cFileUpdateBuffer[nPackageIndex], buffer + 12, nLength);
					m_mapFileUpdateState[nPackageIndex] = 1;
					m_nRecCoreFileCount++;
					CString str;
					str.Format(_T("RECV : TCPDataSender Receive Update File Package:%d/%d.\r\n"), nPackageIndex, m_nCoreFileCount);
					PrintLog(str.GetString());
				}
			}
			else if (buffer[0] == '4')		//补包后确认 待处理
			{
				
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}

void CTCPDataSender::AnalyzeRecLogFile(const unsigned char* buffer)
{
	try
	{
		if (buffer != NULL)
		{
			if (buffer[0] == '0')		//开始
			{
				m_nCoreFileCount = 0;
				m_nLastUpdateExeSize = 0;
				m_mapFileUpdateState.clear();
				char cFileName[256] = { 0 };
				memcpy(cFileName, buffer + 2, 256);
				char* p = strtok(cFileName, "|");
				if (p)
				{
					CString str;
					str.Format(_T("RECV : TCPDataSender Receive Lost File Package:%s...\r\n"), Project::Tools::AnsiToWideChar(p).c_str());
					PrintLog(str.GetString());
					m_strCoreFileName = p;

					p = strtok(NULL, ";");
					if (p)
					{
						m_nCoreFileCount = ATOI(p);
						if (m_nCoreFileCount<0 || m_nCoreFileCount>MAX_UPDATE_SIZE)
							return;

						m_nUpdateFileSize = m_nCoreFileCount + 10;
						for (int i = 0; i < m_nCoreFileCount; ++i)
						{
							m_mapFileUpdateState[i] = 0;
						}
					}
				}
				m_nRecCoreFileCount = 0;
			}
			else if (buffer[0] == '1')
			{
				m_nCoreFileCount = BufferToInt(buffer + 2);
				m_nLastUpdateExeSize = BufferToInt(buffer + 7);
				if (m_nCoreFileCount == m_nRecCoreFileCount)
				{
					GenerateLostFile();
				}
				else		//有丢包   启动线程去像服务器要丢失的包  直到接收时间超过1h，本次更新失败
				{
					//待处理

				}
			}
			else if (buffer[0] == '2')
			{
				int nPackageIndex = BufferToInt(buffer + 2);
				int nLength = BufferToInt(buffer + 7);
				if (nPackageIndex >= 0 && nPackageIndex < MAX_UPDATE_SIZE && nLength <= g_nPackageSize)
				{
					m_nUpdateFileSize = (nPackageIndex >= m_nUpdateFileSize) ? nPackageIndex : m_nUpdateFileSize;
					CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize);
					memset(m_cFileUpdateBuffer[nPackageIndex], 0, sizeof(m_cFileUpdateBuffer[nPackageIndex]));
					memcpy(m_cFileUpdateBuffer[nPackageIndex], buffer + 12, nLength);
					m_mapFileUpdateState[nPackageIndex] = 1;
					m_nRecCoreFileCount++;
					CString str;
					str.Format(_T("RECV : TCPDataSender Receive Lost File Package:%d/%d.\r\n"), nPackageIndex, m_nCoreFileCount);
					PrintLog(str.GetString());
				}
			}
			else if (buffer[0] == '4')		//补包后确认 待处理
			{

			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}

void CTCPDataSender::GenerateUpdateFile()
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\fileupdate";
	if (Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		if (m_strCoreFileName.length() <= 0 || m_nUpdateFileSize < m_nCoreFileCount || m_cFileUpdateBuffer == NULL)
			return;

		CString strFilePath;
		strFilePath.Format(_T("%s\\%s"), wstrFileFolder.c_str(), Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
		CFile file(strFilePath, CFile::modeCreate | CFile::modeWrite);
		file.Seek(0, CFile::begin);

		for (int i = 0; i < m_nCoreFileCount; i++)
		{
			if (i == m_nCoreFileCount - 1)
			{
				file.Write(m_cFileUpdateBuffer[i], m_nLastUpdateExeSize);
			}
			else
			{
				file.Write(m_cFileUpdateBuffer[i], FILE_UPDATE_LENGTH);
			}
			file.Seek(0, CFile::end);
		}
		file.Close();
		CString str;
		str.Format(_T("RECV : TCPDataSender Receive Update File Package(%s)...\r\n"), Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
		PrintLog(str.GetString());

		//创建接收记录
		string strRecevieFilePath = Project::Tools::WideCharToAnsi(strFilePath);
		Json::Value jsonBody;
		jsonBody["type"] = 1;    //文件
		jsonBody["content"] = strRecevieFilePath;
		string strContent = jsonBody.toStyledString();
		if (m_lpRecDataProc)
			m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
	}
}

void CTCPDataSender::GenerateLostFile()
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\filelog";
	if (Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		if (m_strCoreFileName.length() <= 0 || m_nUpdateFileSize < m_nCoreFileCount || m_cFileUpdateBuffer == NULL)
			return;

		CString strFilePath;
		strFilePath.Format(_T("%s\\%s"), wstrFileFolder.c_str(), Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
		CFile file(strFilePath, CFile::modeCreate | CFile::modeWrite);
		file.Seek(0, CFile::begin);

		for (int i = 0; i < m_nCoreFileCount; i++)
		{
			if (i == m_nCoreFileCount - 1)
			{
				file.Write(m_cFileUpdateBuffer[i], m_nLastUpdateExeSize);
			}
			else
			{
				file.Write(m_cFileUpdateBuffer[i], FILE_UPDATE_LENGTH);
			}
			file.Seek(0, CFile::end);
		}
		file.Close();
		CString str;
		str.Format(_T("RECV : TCPDataSender Receive Lost File Package(%s)...\r\n"), Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
		PrintLog(str.GetString());

		//解压文件 根据;划分值
		HZIP hz = OpenZip(strFilePath, 0);
		SetUnzipBaseDir(hz, wstrFileFolder.c_str());
		ZIPENTRY ze;
		GetZipItem(hz, -1, &ze);
		int numitems = ze.index;
		for (int j = 0; j < numitems; ++j)
		{
			GetZipItem(hz, j, &ze);
			ZRESULT zr = UnzipItem(hz, j, ze.name);
			if (zr == ZR_OK)
			{
				//处理解压出来的.csv文件
				CString strCSVPath;
				strCSVPath.Format(_T("%s\\%s"), wstrFileFolder.c_str(), ze.name);
				
				//创建接收记录
				string strRecevieFilePath = Project::Tools::WideCharToAnsi(strCSVPath);
				Json::Value jsonBody;
				jsonBody["type"] = 2;    //丢失数据包文件
				jsonBody["content"] = strRecevieFilePath;
				string strContent = jsonBody.toStyledString();
				if (m_lpRecDataProc)
					m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
			}
		}
		CloseZip(hz);
	}
}

bool CTCPDataSender::CreateOrDeleteUpdateFileBuffer(int nSize /*= 1024*/, bool bDeelete /*= false*/)
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lockReceiveBuffer);
	try
	{
		m_oleUpdateFileTime = COleDateTime::GetCurrentTime();
		if (bDeelete)
		{
			if (m_cFileUpdateBuffer)
			{
				for (int i = 0; i <= nSize; ++i)
				{
					delete[] m_cFileUpdateBuffer[i];
					m_cFileUpdateBuffer[i] = NULL;
				}
				delete[] m_cFileUpdateBuffer;
				m_cFileUpdateBuffer = NULL;
				return true;
			}
		}
		else
		{
			if (m_cFileUpdateBuffer == NULL)
			{
				m_cFileUpdateBuffer = new char*[nSize];
				for (int i = 0; i <= nSize; ++i)
				{
					m_cFileUpdateBuffer[i] = new char[g_nPackageSize];
				}
				return true;
			}
			else
			{
				if (nSize > m_nUpdateFileSize)
				{
					for (int i = 0; i < m_nUpdateFileSize; ++i)
					{
						delete[] m_cFileUpdateBuffer[i];
						m_cFileUpdateBuffer[i] = NULL;
					}
					delete[] m_cFileUpdateBuffer;
					m_cFileUpdateBuffer = NULL;

					m_cFileUpdateBuffer = new char*[nSize];
					for (int i = 0; i <= nSize; ++i)
					{
						m_cFileUpdateBuffer[i] = new char[g_nPackageSize];
					}

				}
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

void CTCPDataSender::ReleaseUpdateBufferSize(int nTimeOut)
{
	try
	{
		if (m_cFileUpdateBuffer)
		{
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleUpdateFileTime;
			if (oleSpan.GetTotalSeconds() >= nTimeOut)
			{
				CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize, true);
			}
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}

void CTCPDataSender::StringReplace(string & s1, const string & s2, const string & s3)
{
	string::size_type pos = 0;
	string::size_type a = s2.size();
	string::size_type b = s3.size();
	while ((pos = s1.find(s2, pos)) != string::npos)
	{
		s1.replace(pos, a, s3);
		pos += b;
	}
}

UINT CTCPDataSender::RecvThread(LPVOID lpVoid)
{
	CTCPDataSender* pTCPDataSender = reinterpret_cast<CTCPDataSender*>(lpVoid);
	if (NULL == pTCPDataSender)
	{
		return 0;
	}
	
	char szBuff[MAX_RECEIVED_BUFFER_SIZE] = { 0 };
	SOCKET sockConnected = pTCPDataSender->GetTCPConnectedSocket();
	if (sockConnected != NULL)
	{
		while (!pTCPDataSender->m_bExitThread)
		{
			//接收数据
			memset(szBuff, 0, MAX_RECEIVED_BUFFER_SIZE);
			int nLen = recv(sockConnected, szBuff, MAX_RECEIVED_BUFFER_SIZE, 0);
			if (nLen > 0)
			{
				pTCPDataSender->HandleRecData((const char*)szBuff, nLen);
			}
			else if (nLen == 0)
			{
				// The server connected closed.
				pTCPDataSender->m_bTCPConnectSuccess = false;
				return 0;
			}
			else
			{
				// Received failed.
				pTCPDataSender->m_bTCPConnectSuccess = false;
				return 0;
			}
		}
	}
	return 1;
}

UINT CTCPDataSender::AnalyzeRecvCmd(LPVOID lpVoid)
{
	CTCPDataSender* pTCPDataSender = reinterpret_cast<CTCPDataSender*>(lpVoid);
	if (NULL == pTCPDataSender)
	{
		return 0;
	}

	while (!pTCPDataSender->m_bExitThread) 
	{
		pTCPDataSender->AnalyzeRecvData();
		Sleep(1000);
	}
	return 0;
}

UINT WINAPI CTCPDataSender::DTUThreadReleaseUpdateSize(LPVOID lparam)
{
	CTCPDataSender* pthis = (CTCPDataSender*)lparam;
	if (!pthis) 
	{
		return 0;
	}

	while (!pthis->m_bExitThread)
	{
		pthis->ReleaseUpdateBufferSize();
		Sleep(1000);
	}
	return 0;
}
