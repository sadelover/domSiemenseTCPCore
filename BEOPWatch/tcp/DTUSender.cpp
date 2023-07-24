#include "StdAfx.h"
#include "DTUSender.h"
#include <afx.h>
#include "../Zip/zlib.h"
#include "Tools/CustomTools/CustomTools.h"
#include "../Zip/unzip.h"
#include "ExcelOperator.h"
#pragma comment(lib,"zlib.lib") 

#include "PackageTypeDefine.h"

#include <string>
#include <iostream>
#include <sstream>
using std::wstring;
using std::string;

const int DTU_SEND_PACKAGE_INTERNAL = 2*1000;	//dtu连续发包的间隔时间
const int DTU_SEND_UNIT01_INTERNAL = 2*1000;	//dtu连续发包的间隔时间(间隔时间过短会崩溃)
const int DTU_PACKAGE_SIZE = 900;				//分包时最大包长。
const int MAX_POINT_LENGTH = 120;				//点名最大长度
const int FILE_UPDATE_LENGTH = 4000;			//文件升级固定长度为4000
const int TCP_SEND_INTERNAL = 200;				//200ms发送一帧

const int FILE_SIZE = 1024;						//小的读取文件的空间总数
const int ONE_FILE_SIZE = 4096;					//每个小的读取文件的空间

#define STR_SEP_MARK ","
#define STR_SEPGROUP_MARK ";"

const string DTU_PACKAGE_INDEX = "DTU_P_INDEX";	//实时数据发包编号
const string DTU_PACKAGE_BASE = "DTU_P_BASE";	//实时数据发包基数
const int	MAX_STORE_CONTENT = 1000;		//存储最近1000个包的内容

//#include "Tools/vld.h"

CDTUSender::CDTUSender(string strTCPName,string strHost,int nPort,CDataHandler* pDataHandle)
	:m_tcphandler(new CTCPDataSender())
	,m_nDTUPackageNumber(0)
	,m_nDTUPackageBase(1)
	,m_bRunReserverPort(false)
	,m_nRunReserverPort(0)
	,m_strDTUID("")
	,m_strCSQ("")
	,m_strDSCIP("")
	,m_strRecFileName("")
	,m_strCoreFileName("")
	,m_nRTUAnswerMode(0)
	,m_nDTUFilePackageNumber(0)
	,m_nFileCount(0)
	,m_nRecFileCount(0)
	,m_nCoreFileCount(0)
	,m_nRecCoreFileCount(0)
	,m_recthread(NULL)
	,m_bHasLostFileFlag(false)
	,m_strUpdateExeAck("")
	,m_nLastUpdateExeSize(0)
	,m_host(strHost)
	,m_port(nPort)
	,m_bdtuSuccess(false)
	,m_strTcpName(strTCPName)				//名称
	,m_pDataHandle(pDataHandle)
	,m_nUpdateFileSize(MAX_UPDATE_SIZE)
{
	m_sendthread = NULL;
	m_readthread = NULL;
	m_relesethread = NULL;
	m_cFileUpdateBuffer = NULL;
	m_bsendthreadexit = false;
	m_breadthreadexit = false;
	m_brecthreadexit = false;
	m_bDTU_To_RTU = false;
	m_bDTUConnectOK = false;
	m_bReceiveDTU = false;
	m_bReceiveDTUServerEvent = false;
	m_nReceiveDTUCmdType = 0;
	m_sendbuffer = "";
	m_oleUpdateExeTime = COleDateTime::GetCurrentTime();
	m_oleUpdateFileTime = COleDateTime::GetCurrentTime();
	memset(&m_sendtime, 0x00, sizeof(SYSTEMTIME));
	m_oleLastWriteTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
	m_vecDTUCmd.clear();
	memset(m_cRXBuff,0,sizeof(m_cRXBuff));
	m_nReceCount = 0;
}

CDTUSender::~CDTUSender(void)
{
	m_tcphandler->CloseTcp();
	//===== free thread handle.
	if (m_sendthread){
		CloseHandle(m_sendthread);
		m_sendthread = NULL;
	}

	if (m_readthread){
		CloseHandle(m_readthread);
		m_readthread = NULL;
	}

	if (m_relesethread){
		CloseHandle(m_relesethread);
		m_relesethread = NULL;
	}

	CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize,true);
}

bool CDTUSender::InitDTU()
{
	// 初始化临界区
	InitializeCriticalSection(&m_csDTUDataSync);

	m_bdtuSuccess = m_tcphandler->InitTcp(m_host,m_port,m_strTcpName);
	m_tcphandler->StartMonitoring(OnRecData,(DWORD)this);
	m_readthread = (HANDLE)_beginthreadex(NULL, 0, DTUThreadHandRecCmd, this, NORMAL_PRIORITY_CLASS, NULL);
	m_relesethread = (HANDLE)_beginthreadex(NULL, 0, DTUThreadReleaseUpdateSize, this, NORMAL_PRIORITY_CLASS, NULL);
	if (!m_bdtuSuccess){
		return m_bdtuSuccess;
	}

	if(m_bdtuSuccess)
	{
		_tprintf(_T("Init TCPDataSender Engine Success...\r\n"));
	}
	return m_bdtuSuccess;
}

bool CDTUSender::SendData(char* buffer, int buffersize)
{
	////发送之前压缩
	return m_tcphandler->WriteToPort(buffer, buffersize+1);
}

bool CDTUSender::SendDataByWrap( char* buffer, int buffersize )
{
	return m_tcphandler->WriteToPortByWrap(buffer, buffersize+2);
}

bool CDTUSender::SendData(const SYSTEMTIME tSendTime,  const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "pdp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	std::ostringstream sqlstream;
	//一批次包发个确认
	std::ostringstream sqlstream_Cfg;
	sqlstream_Cfg << "cfg:time:" << timestring << ";" << m_nDTUPackageBase << ";";
	int nCount = 0;
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		sqlstream.str("");
		sqlstream << pointdataprefix << timestring << ";" << bufferlist[i] 
					<< "0," << DTU_PACKAGE_BASE << "," << m_nDTUPackageBase			
					<< ";0," << DTU_PACKAGE_INDEX << "," << m_nDTUPackageNumber  << ";";
		sqlstream_Cfg << m_nDTUPackageNumber << ",";
		nCount++;
		if(nCount >= 20)						//100组为一回复包
		{
			string strCfg = sqlstream_Cfg.str();
			strCfg.erase(--strCfg.end());
			strCfg += ";";
			bResult = SendDataByWrap((char*)strCfg.data(), strCfg.size()) && bResult;

			nCount = 0;
			sqlstream_Cfg.str("");
			sqlstream_Cfg << "cfg:time:" << timestring << ";" << m_nDTUPackageBase << ";";
			Sleep(DTU_SEND_PACKAGE_INTERNAL);
		}

		string strdata = sqlstream.str();
		StoreSendInfo(strdata);
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}

	string strCfg = sqlstream_Cfg.str();
	strCfg.erase(--strCfg.end());
	strCfg += ";";
	bResult = SendDataByWrap((char*)strCfg.data(), strCfg.size()) && bResult;
	Sleep(DTU_SEND_PACKAGE_INTERNAL);
	return bResult;
}

void CDTUSender::BuildPackage(const string& buffer, vector<string>& resultlist)
{
	int buffersize = buffer.size();
	if (buffersize == 0)
	{
		return;
	}
	if(buffersize <= DTU_PACKAGE_SIZE)
		resultlist.push_back(buffer);
	else
	{
		string strcopy = buffer;
		while(strcopy.size() >  DTU_PACKAGE_SIZE)
		{
			int i = DTU_PACKAGE_SIZE;
			int j = 0;	//比一个点更大
			while(j < MAX_POINT_LENGTH){
				if( ((i+j) >= strcopy.size()) || strcopy[i+j] == ';')
					break;
				j++;
			}

			resultlist.push_back(strcopy.substr(0,i+j+1));
			if((i+j) < strcopy.size())
				strcopy = strcopy.substr(i+j+1);
		}
		resultlist.push_back(strcopy);
	}
}

void CDTUSender::ExitThreads()
{
	m_bsendthreadexit = true;
	m_breadthreadexit = true;

	TerminateThread(m_readthread, 0);
	TerminateThread(m_sendthread, 0);
	TerminateThread(m_relesethread, 0);
	WaitForSingleObject(m_readthread, INFINITE);
	WaitForSingleObject(m_sendthread, INFINITE);
	WaitForSingleObject(m_relesethread, INFINITE);
}

UINT WINAPI CDTUSender::DTUThreadHandRecCmd( LPVOID lparam )
{
	CDTUSender* pthis = (CDTUSender*)lparam;
	if (!pthis){
		return 0;
	}

	while(!pthis->GetReadThreadExit()){
		pthis->HandRecCmd();
		Sleep(500);
	}
	return 0;
}

bool CDTUSender::GetReadThreadExit() const
{
	return m_breadthreadexit;
}

bool CDTUSender::GetSendThreadExit() const
{
	return m_bsendthreadexit;
}

bool CDTUSender::GetRecThreadExit() const
{
	return m_brecthreadexit;
}

LRESULT CDTUSender::OnRecData( const unsigned char* pRevData, DWORD length ,DWORD userdata)
{
	//
	CDTUSender *pDTUCtrl=(CDTUSender *)userdata;
	if(pDTUCtrl != NULL)
	{
		DTUServerCmdInfo data;
		memset(data.cData,0,g_nPackageSize);
		length = (length>g_nPackageSize)?g_nPackageSize:length;
		memcpy(data.cData,pRevData,length);
		data.nLength = length;
		pDTUCtrl->StoreDTUCmd(data);
	}
	return 0;
}

bool CDTUSender::SendWarningData(const SYSTEMTIME tSendTime,  const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "wdp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::SendWarnigOperationData(const SYSTEMTIME tSendTime,  const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "wop:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::GetReceiveDTUServerEvent(vector<vector<string>>& vecCmd)
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_reveivelock);
	if(m_bReceiveDTUServerEvent)
	{
		vecCmd = m_vecDTUCmd;
		m_vecDTUCmd.clear();
		m_bReceiveDTUServerEvent = false;
		return true;
	}
	return false;
}

bool CDTUSender::SetReceiveDTUServerEvent( bool bReceiveDTUServerEvent )
{
	m_bReceiveDTUServerEvent = bReceiveDTUServerEvent;
	return true;
}

//处理中原本就不会含多个\n  20150402
bool CDTUSender::AnalyzeReceive( const unsigned char* pRevData, DWORD length )
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_reveivelock);
	if(length <= 3)
		return false;

	//;;\n结尾
	if(pRevData[length-1] == '\n' && pRevData[length-2] == ';')
	{
		char buffer_copy[g_nPackageSize] = {0};
		length = length-3;
		length = (length>g_nPackageSize)?g_nPackageSize:length;
		memcpy(buffer_copy,pRevData,length);

		rnbtech::CPackageType::DTU_DATATYPE type = rnbtech::CPackageType::GetPackageType(buffer_copy);
		switch(type)
		{
		case rnbtech::CPackageType::Type_DTUServerCmd:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy,length);
				UpdateReceiveBuffer(buffer_copy);
			}
			break;
		case rnbtech::CPackageType::Type_DTUServerFup:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy,length);
				//处理接收升级文件
				AnalyzeUpdateFile((unsigned char*)buffer_copy);
			}
			break;
		case rnbtech::CPackageType::Type_WebServerFup:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy,length);
				//处理接收升级文件
				AnalyzeWebUpdateFile((unsigned char*)buffer_copy);
			}
			break;
		case rnbtech::CPackageType::Type_DTUServerFrl:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy,length);
				//处理匹配文件接收
			}
			break;
		case rnbtech::CPackageType::Type_DTUWriteTime:
			{
				rnbtech::CPackageType::RemovePrefix(buffer_copy);
				UpdateDTUWriteTime(buffer_copy);
			}
			break;
		default:
			{
				OutPutLogErrString(buffer_copy);
				_tprintf(_T("Error: DTUServer Cmd Analyze Failed.\r\n"));
			}
			break;
		}
		return true;
	}
	return false;
}

void CDTUSender::UpdateReceiveBuffer( char* pBuffer )
{
	m_pReceiveDTUServer = strtok((char*)pBuffer, ";");
	OutPutLogString(m_pReceiveDTUServer);
	vector<string>	vecCmd;
	SplitString(m_pReceiveDTUServer,"|",vecCmd);
	m_vecDTUCmd.push_back(vecCmd);
	m_bReceiveDTUServerEvent = true;
}

void CDTUSender::SplitString( const std::string& strsource, const char* sep, std::vector<string>& resultlist )
{
	if( !sep)
		return;
	resultlist.clear();
	memset(m_cSpiltBuffer,0,MAX_REC_COUNT);
	memcpy(m_cSpiltBuffer,strsource.c_str(),strsource.length());
	char* token = NULL;
	char* nexttoken = NULL;
	token = strtok_s(m_cSpiltBuffer, sep, &nexttoken);

	while(token != NULL)
	{
		string str = token;
		resultlist.push_back(str);
		token = strtok_s(NULL, sep, &nexttoken);
	}
}

bool CDTUSender::SendUnit01( const SYSTEMTIME tSendTime, const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string dataprefix = "udp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = dataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_UNIT01_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::SendCmdState(const SYSTEMTIME tSendTime, int nCmdType, int nCmdState , string strPacketIndex)
{
	bool bResult = true;

	std::ostringstream sqlstream;
	sqlstream << "ack:time:" << Project::Tools::SystemTimeToString(tSendTime) << ";" << nCmdType << ","<< nCmdState <<"," << strPacketIndex << ";";
	string strdata = sqlstream.str();
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	return bResult;
}

bool CDTUSender::Exit()
{
	m_tcphandler->Exit();
	
	m_bsendthreadexit = true;
	m_breadthreadexit = true;

	if(m_readthread)
	{
		TerminateThread(m_readthread, 0);
		WaitForSingleObject(m_readthread, INFINITE);
	}
	if(m_sendthread)
	{
		TerminateThread(m_sendthread, 0);
		WaitForSingleObject(m_sendthread, INFINITE);
	}
	if(m_relesethread)
	{
		TerminateThread(m_relesethread, 0);
		WaitForSingleObject(m_relesethread, INFINITE);
	}
	CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize,true);
	return true;
}

std::string CDTUSender::GetSuccessBuffer()
{
	return m_tcphandler->GetSuccessBuffer();
}

void CDTUSender::UpdateDTUWriteTime( char* pBuffer )			//更新写时间
{
	char*   pTime = strtok((char*)pBuffer, ";");
	if(pTime != NULL)
	{
		COleDateTime oleNow;
		wstring strItemValue = Project::Tools::AnsiToWideChar(pTime);
		Project::Tools::String_To_OleTime(strItemValue,oleNow);
		COleDateTimeSpan oleSpan = oleNow - m_oleLastWriteTime;
		if(oleSpan.GetTotalMinutes()>=10)
		{
			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				m_pDataHandle->WriteCoreDebugItemValue(L"dtusendtime",strItemValue);
			}
			m_oleLastWriteTime = oleNow;
		}
	}
}

bool CDTUSender::SendHeartBeat( const SYSTEMTIME tSendTime )
{

	bool bResult = true;
	string dataprefix = "hba:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	string strdata = dataprefix;
	strdata += timestring;
	strdata += ";";
	strdata += timestring;
	strdata += ";";
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	Sleep(DTU_SEND_UNIT01_INTERNAL);
	return bResult;
}

bool CDTUSender::SendExecuteLog( const SYSTEMTIME tSendTime, const string& strSendBuff )
{
	bool bResult = true;
	string strdata = "log:time:";
	strdata += Project::Tools::SystemTimeToString(tSendTime);
	strdata += ";";
	strdata += strSendBuff;
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
	Sleep(DTU_SEND_PACKAGE_INTERNAL);
	return bResult;
}

bool CDTUSender::StoreSendInfo( string strSendContent )
{
	m_mapStoreSendPackageBase[m_nDTUPackageNumber] = m_nDTUPackageBase;
	if(m_nDTUPackageNumber > MAX_STORE_CONTENT)
	{
		m_nDTUPackageNumber = 0;
		m_nDTUPackageBase++;
		if(m_nDTUPackageBase > MAX_STORE_CONTENT)
			m_nDTUPackageBase = 1;
	}
	m_mapStoreSendPackage[m_nDTUPackageNumber] = strSendContent;
	m_nDTUPackageNumber++;
	return true;
}

string CDTUSender::GetStoreInfo( int nIndex ,int& nBase)
{
	nBase = 0;
	map<int, int>::iterator iter1 =  m_mapStoreSendPackageBase.find(nIndex);
	if(iter1 != m_mapStoreSendPackageBase.end())
	{
		nBase = (*iter1).second;
	}

	map<int, string>::iterator iter =  m_mapStoreSendPackage.find(nIndex);
	if(iter != m_mapStoreSendPackage.end())
	{
		return (*iter).second;
	}
	return "";
}

bool CDTUSender::ReSendRealData( string strIndex )
{
	if(strIndex.length()<=0)
		return false;

	//发确认
	SYSTEMTIME tSendTime;
	GetLocalTime(&tSendTime);
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	std::ostringstream sqlstream_Cfg;
	sqlstream_Cfg << "cfg:time:" << timestring << ";";

	vector<string> vecIndex;
	SplitString(strIndex.c_str(),",",vecIndex);

	std::ostringstream sqlstream_Cfg_;
	bool bResult = true;
	int nCount = 0;
	int nDTUPackageBase = 0;
	for(int i=0; i<vecIndex.size(); ++i)
	{
		string strSendBuf = GetStoreInfo(atoi(vecIndex[i].c_str()),nDTUPackageBase);
		if(strSendBuf.length()>0)
		{
			sqlstream_Cfg_ << vecIndex[i] << ",";
			nCount++;
			bResult = SendDataByWrap((char*)strSendBuf.data(), strSendBuf.size()) && bResult;
			Sleep(DTU_SEND_PACKAGE_INTERNAL);
		}
	}

	sqlstream_Cfg << nDTUPackageBase << ";" << sqlstream_Cfg_.str();

	if(nCount>0)
	{
		string strCfg = sqlstream_Cfg.str();
		strCfg.erase(--strCfg.end());
		strCfg += ";";
		bResult = SendDataByWrap((char*)strCfg.data(), strCfg.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::OutPutLogString( char* pOut )
{
	char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	setlocale( LC_ALL, "chs" );

	CString strLog;
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	Project::Tools::SysTime_To_String(st,strTime);
	strLog += strTime.c_str();
	strLog += _T(" - ");

	strLog += Project::Tools::AnsiToWideChar(pOut).c_str();
	strLog += _T("\n");

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		CString strLogPath;
		strLogPath.Format(_T("%s\\dtuservercmd_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
	}
	setlocale( LC_CTYPE, old_locale ); 
	free( old_locale ); 
	return false;
}

//处理队列为char*  20150403  命令以;;\n结尾
bool CDTUSender::HandRecCmd()
{
	while(!m_queCmdPaket.empty())
	{
		// 开始进入临界区
		EnterCriticalSection(&m_csDTUDataSync);
		bool bHasData = false;
		DTUServerCmdInfo data;
		if(!m_queCmdPaket.empty())
		{
			data = m_queCmdPaket.front();
			m_queCmdPaket.pop();
			bHasData = true;
		}
		LeaveCriticalSection(&m_csDTUDataSync);

		if(bHasData)
		{
			char* RXBuff = data.cData;
			if(RXBuff != NULL)
			{		
				int dwBytesRead = data.nLength;
				if(m_nReceCount + dwBytesRead >= MAX_REC_COUNT)
				{
					memset(m_cRXBuff,0,sizeof(m_cRXBuff));
					m_nReceCount = 0;
				}

				//拷贝
				memcpy(m_cRXBuff+m_nReceCount,RXBuff,dwBytesRead);
				m_nReceCount = m_nReceCount + dwBytesRead;

				if(m_nReceCount > 3)
				{
					if(m_cRXBuff[m_nReceCount-1] == '\n' && m_cRXBuff[m_nReceCount-2] == ';' && m_cRXBuff[m_nReceCount-3] == ';')		//;;\n结尾
					{
						AnalyzeReceive((const unsigned char *)m_cRXBuff,m_nReceCount);
						memset(m_cRXBuff,0,sizeof(m_cRXBuff));
						m_nReceCount = 0;
					}
				}
			}
		}
	}
	return true;
}

bool CDTUSender::OutPutLogErrString( char* pOut )
{
	char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	setlocale( LC_ALL, "chs" );

	CString strLog;
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	Project::Tools::SysTime_To_String(st,strTime);
	strLog += strTime.c_str();
	strLog += _T(" - ");

	strLog += Project::Tools::AnsiToWideChar(pOut).c_str();
	strLog += _T("\n");

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		CString strLogPath;
		strLogPath.Format(_T("%s\\dtucmderr_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);
		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
	}
	setlocale( LC_CTYPE, old_locale ); 
	free( old_locale ); 
	return false;
}

bool CDTUSender::SendCoreStart( const string strProjectName )
{
	bool bResult = true;
	string strdata = "reg:";
	strdata += strProjectName;
	strdata += ";";
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	Sleep(DTU_SEND_UNIT01_INTERNAL);
	return bResult;
}

bool CDTUSender::WriteSendFileLog( string strFileName )
{
	char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	setlocale( LC_ALL, "chs" );

	SYSTEMTIME st;
	GetLocalTime(&st);

	CString strLog;	
	strLog = Project::Tools::AnsiToWideChar(strFileName.c_str()).c_str();
	strLog += _T(",");

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\filelog";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		CString strLogPath;
		strLogPath.Format(_T("%s\\filesendlog_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
	}

	setlocale( LC_CTYPE, old_locale ); 
	free( old_locale ); 
	return false;
}

bool CDTUSender::StoreDTUCmd( DTUServerCmdInfo data )
{
	// 开始进入临界区
	EnterCriticalSection(&m_csDTUDataSync);
	m_queCmdPaket.push(data);
	LeaveCriticalSection(&m_csDTUDataSync);
	return true;
}

void CDTUSender::SplitStringSpecial( const char* buffer,int nSize, std::vector<char*>& resultlist,std::vector<int>& sizelist )
{
	if(nSize <= 0)
		return;

	resultlist.clear();
	sizelist.clear();
	memset(m_cSpiltBufferSpilt,0,MAX_REC_COUNT);
	memcpy(m_cSpiltBufferSpilt,buffer,nSize);

	char cBefore = 0x00;
	int nBefore = 0;
	int nAfter = 0;
	for(int i=0; i<nSize; ++i)
	{
		if(buffer[i] == '\n' && cBefore == ';')
		{
			char* cBuffer = new char[1024];
			memset(cBuffer,0,1024);
			int length = i-1-nBefore;
			length = (length>1024)?1024:length;
			memcpy(cBuffer,m_cSpiltBufferSpilt+nBefore,length);		
			sizelist.push_back(length);
			resultlist.push_back(cBuffer);
			nBefore = i+1;
		}
		cBefore = buffer[i];
	}

	int nLength = nSize - nBefore;
	if(nLength>0)
	{
		char* cBuffer = new char[1024];
		memset(cBuffer,0,1024);
		int length = (nLength>1024)?1024:nLength;
		memcpy(cBuffer,m_cSpiltBufferSpilt+nBefore,length);
		sizelist.push_back(length);
		resultlist.push_back(cBuffer);
	}
}

void CDTUSender::AnalyzeUpdateFile( const unsigned char* buffer )
{
	try
	{
		if(buffer != NULL)
		{
			m_oleUpdateExeTime = COleDateTime::GetCurrentTime();

			if(buffer[0] == '0')		//开始
			{
				m_nCoreFileCount = 0;
				m_nLastUpdateExeSize = 0;
				m_mapFileUpdateState.clear();
				char cFileName[256] = {0};
				memcpy(cFileName,buffer+2,256);
				char* p = strtok(cFileName, "|");
				if(p)
				{
					CString str;
					str.Format(_T("DTU:Receive Core Update File Package:%s...\r\n"),Project::Tools::AnsiToWideChar(p).c_str());
					_tprintf(str);
					m_strCoreFileName = p;

					p = strtok(NULL, ";");
					if(p)
					{
						m_nCoreFileCount = ATOI(p);
						if(m_nCoreFileCount<0 || m_nCoreFileCount>MAX_UPDATE_SIZE)
							return;

						m_nUpdateFileSize = m_nCoreFileCount+10;
						for(int i=0; i<m_nCoreFileCount; ++i)
						{
							m_mapFileUpdateState[i] = 0;
						}
					}
				}
				m_nRecCoreFileCount = 0;
			}
			else if(buffer[0] == '1')
			{
				m_nCoreFileCount = BufferToInt(buffer+2);
				m_nLastUpdateExeSize =   BufferToInt(buffer+7);
				if(m_nCoreFileCount == m_nRecCoreFileCount)
				{
					GenerateUpdateFile();
				}
				else		//有丢包   启动线程去像服务器要丢失的包  直到接收时间超过1h，本次更新失败
				{
					//
					m_bHasLostFileFlag = true;
				}
			} 
			else if(buffer[0] == '2')
			{
				int nPackageIndex = BufferToInt(buffer+2);
				int nLength =   BufferToInt(buffer+7);
				if(nPackageIndex>=0 && nPackageIndex<MAX_UPDATE_SIZE && nLength <= g_nPackageSize)
				{
					m_nUpdateFileSize = (nPackageIndex>=m_nUpdateFileSize)?nPackageIndex:m_nUpdateFileSize;
					CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize);
					memset(m_cFileUpdateBuffer[nPackageIndex],0,sizeof(m_cFileUpdateBuffer[nPackageIndex]));
					memcpy(m_cFileUpdateBuffer[nPackageIndex],buffer+12,nLength);
					m_mapFileUpdateState[nPackageIndex] = 1;
					m_nRecCoreFileCount++;
					CString str;
					str.Format(_T("DTU:Receive Core Update File Package:%d/%d.\r\n"),nPackageIndex,m_nCoreFileCount);
					_tprintf(str);
				}
			}
			else if(buffer[0] == '4')		//补包后确认
			{
				if(ScanAndSendLostPackage())
				{
					m_bHasLostFileFlag = true;
				}
				else
				{
					GenerateUpdateFile();		
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
}

unsigned int CDTUSender::BufferToInt( const unsigned char* buffer )
{
	return buffer[0] + buffer[1]*256 + buffer[2]*65536 + buffer[3]*256*65536;
}

bool CDTUSender::ScanAndSendLostPackage()
{
	map<int,int>::iterator itert = m_mapFileUpdateState.begin();
	while(itert != m_mapFileUpdateState.end())
	{
		int nState = itert->second;
		if(nState != 1)
			return true;
		++itert;
	}
	return false;
}

bool CDTUSender::ScanAndSendLostPackage( string& strUpdateExeAck,string& strLostFile )
{
	if(m_bHasLostFileFlag)
	{
		std::ostringstream sqlstream;
		vector<int> vecLost;
		map<int,int>::iterator itert = m_mapFileUpdateState.begin();
		while(itert != m_mapFileUpdateState.end())
		{
			int nState = itert->second;
			int nIndex = itert->first;
			if(nState != 1)
				sqlstream << nIndex << ",";
			++itert;
		}

		strLostFile = sqlstream.str();
		if(strLostFile.length()>0)
			strLostFile.erase(--strLostFile.end());

		strUpdateExeAck = "";
		m_bHasLostFileFlag = false;
		return true;
	}
	else
	{
		strUpdateExeAck = m_strUpdateExeAck;
		m_strUpdateExeAck = "";
		if(strUpdateExeAck.length() > 0)
			return true;
	}
	return false;
}

bool CDTUSender::SendUpdateFileAck( int nState, string strContetn )
{
	bool bResult = true;
	std::ostringstream sqlstream;
	sqlstream << "fdp:4|" << nState << "|" << strContetn << ";";
	string strdata = sqlstream.str();
	bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	return bResult;
}

bool CDTUSender::GetSendIdle( int nSeconds/*=4*/ )
{
	return m_tcphandler->GetDTUIdleState(nSeconds);
}

bool CDTUSender::SendHistoryDataFile( string strFilePath ,int nPointCount)
{
	bool bResult = true;
	std::ostringstream sqlstream,sqlstream_index;
	if(strFilePath == "")		//查找失败
	{
		sqlstream << "fhp:3;";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	}
	else
	{
		//读文件
		gzFile gz = gzopen(strFilePath.c_str(),"rb");
		if(!gz)
			return false;

		char **cFile = new char *[FILE_SIZE];
		for(int i=0;i<FILE_SIZE;++i)
		{
			cFile[i] = new char[ONE_FILE_SIZE];
		}

		int nLastLen = 0;
		int nCount = 0;
		//开始传送文件
		for(;;)
		{
			//一次读取BLOCKSIZE大小的文件内容
			nLastLen = gzread(gz,cFile[nCount], FILE_UPDATE_LENGTH);
			cFile[nCount][nLastLen] = ';';			//加一个分号
			nCount++;
			if(nLastLen < FILE_UPDATE_LENGTH)
				break;		
		}
		//关闭文件
		gzclose(gz);

		int pos = strFilePath.find_last_of('\\');
		string strFileName(strFilePath.substr(pos + 1));

		//开始
		sqlstream.str("");
		sqlstream << "fhp:0|" << strFileName << ";" << nPointCount << ";";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
		Sleep(TCP_SEND_INTERNAL);

		string strSend = "fhp:2|";
		for(int i=0;i<nCount; ++i)
		{
			char cSend[ONE_FILE_SIZE] = {0};
			memcpy(cSend,strSend.data(),6);
			unsigned int length = 0;
			if(i==nCount-1)
			{
				length = nLastLen+1;
			}
			else
			{
				length = FILE_UPDATE_LENGTH+1;
			}
			memcpy(cSend+11,&length,4);
			if(m_nDTUFilePackageNumber==59)			//因为长度位置会变成; 影响解析
				m_nDTUFilePackageNumber++;
			memcpy(cSend+6,&m_nDTUFilePackageNumber,4);
			sqlstream_index << m_nDTUFilePackageNumber << ",";
			m_nDTUFilePackageNumber++;
			if(m_nDTUFilePackageNumber>=1000)
			{
				m_nDTUFilePackageNumber = 0;
			}

			cSend[10] = '|';
			cSend[15] = '|';
			memcpy(cSend+16,cFile[i],length);
			bResult = SendDataByWrap(cSend, length+16);
			Sleep(TCP_SEND_INTERNAL);
		}

		//释放
		for(int i=0;i<FILE_SIZE;++i)  
		{  
			delete cFile[i];  
			cFile[i] = NULL;  
		}  
		delete [g_nPackageSize]cFile;  
		cFile = NULL;     
					
		//结束
		char cSend1[256] = {0};
		strSend = "fhp:1|";
		memcpy(cSend1,strSend.data(),6);
		memcpy(cSend1+11,&nLastLen,4);
		memcpy(cSend1+6,&nCount,4);

		string strFileIndex = sqlstream_index.str();
		strFileIndex.erase(--strFileIndex.end());
		strFileIndex += ";";
		memcpy(cSend1+16,strFileIndex.data(),strFileIndex.size());
		cSend1[10] = '|';
		cSend1[15] = ';';

		bResult = SendDataByWrap(cSend1,16+strFileIndex.size());
		Sleep(TCP_SEND_INTERNAL);
	}

	return bResult;
}

bool CDTUSender::GetDTUSuccess()
{
	m_bdtuSuccess = m_tcphandler->GetTCPConnectOK();
	return m_bdtuSuccess;
}

bool CDTUSender::SendUserOperationData( const SYSTEMTIME tSendTime, const string& strSendBuff )
{
	vector<string> bufferlist;
	m_lock.Lock();
	BuildPackage(strSendBuff, bufferlist);
	m_lock.UnLock();
	if (bufferlist.empty()){
		return false;
	}

	bool bResult = true;
	string pointdataprefix = "orp:time:";
	string timestring = Project::Tools::SystemTimeToString(tSendTime);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += timestring;
		strdata += ";";
		strdata += bufferlist[i];
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size()) && bResult;
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}
	return bResult;
}

bool CDTUSender::OutPutUpdateLog( CString strLog )
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\fileupdate";
	if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		setlocale( LC_ALL, "chs" );

		CString strOut;
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog);
		
		CString strLogPath;
		strLogPath.Format(_T("%s\\fileupdate.log"),wstrFileFolder.c_str());

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strOut);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
		setlocale( LC_CTYPE, old_locale ); 
		free( old_locale ); 
	}
	return false;
}

void CDTUSender::GenerateUpdateFile()
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\fileupdate";
	if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{				
		if(m_strCoreFileName.length() <= 0 || m_nUpdateFileSize<m_nCoreFileCount || m_cFileUpdateBuffer == NULL)
			return;
		CString strFilePath;
		strFilePath.Format(_T("%s\\%s"),wstrFileFolder.c_str(),Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
		CFile file(strFilePath, CFile::modeCreate|CFile::modeWrite);
		file.Seek(0,CFile::begin);

		for(int i=0; i<m_nCoreFileCount; i++)
		{						
			if(i==m_nCoreFileCount-1)
			{
				file.Write(m_cFileUpdateBuffer[i],m_nLastUpdateExeSize);
			}
			else
			{
				file.Write(m_cFileUpdateBuffer[i],FILE_UPDATE_LENGTH);
			}
			file.Seek(0,CFile::end);
			m_oleUpdateFileTime = COleDateTime::GetCurrentTime();
		}
		file.Close(); 
		CString str;
		str.Format(_T("DTU:Receive Core Update File Package(%s)...\r\n"),Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
		_tprintf(str);

		//解压文件 根据;划分值
		wstrFileFolder += L"\\update";

		//先删除update文件夹
		DelteFoder(wstrFileFolder);

		HZIP hz = OpenZip(strFilePath,0);
		SetUnzipBaseDir(hz,wstrFileFolder.c_str());
		ZIPENTRY ze;
		GetZipItem(hz,-1,&ze); 
		int numitems=ze.index;
		bool bUpdateDll = false;
		for (int j=0; j<numitems; ++j)
		{
			GetZipItem(hz,j,&ze);
			ZRESULT zr = UnzipItem(hz,j,ze.name);
			if(zr == ZR_OK)
			{
				CString strExeName = ze.name;
				CString strLog;
				if(strExeName.Find(_T(".exe"))>=0)
				{
					if(strExeName.Find(_T("domcore.exe"))>=0)
					{
						strLog.Format(_T("DTU:Receive Core Update File(%s)..."),strExeName);
						OutPutUpdateLog(strLog);

						if(m_pDataHandle && m_pDataHandle->IsConnected())
						{
							m_pDataHandle->WriteCoreDebugItemValue(L"updatecore",L"1");
						}
					}
					else if(strExeName.Find(_T("BEOPWatch.exe"))>=0)
					{
						strLog.Format(_T("DTU:Receive Watch Update File(%s)..."),strExeName);
						OutPutUpdateLog(strLog);

						if(m_pDataHandle && m_pDataHandle->IsConnected())
						{
							m_pDataHandle->WriteCoreDebugItemValue(L"updatedog",L"1");
						}
					}
					else if(strExeName.Find(_T("BEOPLogicEngine.exe"))>=0)
					{
						strLog.Format(_T("DTU:Receive LogicEngine Update File(%s)..."),strExeName);
						OutPutUpdateLog(strLog);

						if(m_pDataHandle && m_pDataHandle->IsConnected())
						{
							m_pDataHandle->WriteCoreDebugItemValue(L"updatelogic",L"1");
						}
					}
				}
				else if(strExeName.Find(_T(".dcg"))>=0)
				{
					bUpdateDll = true;
				}
				else if(strExeName.Find(_T(".xls"))>=0)
				{
					strLog.Format(_T("DTU:Receive Point Update File(%s)..."),strExeName);
					OutPutUpdateLog(strLog);

					if(m_pDataHandle && m_pDataHandle->IsConnected())
					{
						m_pDataHandle->WriteCoreDebugItemValue(L"updatexls",L"1");
					}
				}
				else if(strExeName.Find(_T(".csv"))>=0)
				{
					strLog.Format(_T("DTU:Receive Point Update File(%s)..."),strExeName);
					OutPutUpdateLog(strLog);

					if(m_pDataHandle && m_pDataHandle->IsConnected())
					{
						m_pDataHandle->WriteCoreDebugItemValue(L"updatecsv",L"1");
					}
				}
				else if(strExeName.Find(_T(".bat"))>=0)
				{
					strLog.Format(_T("DTU:Receive Bat File(%s)..."),strExeName);
					OutPutUpdateLog(strLog);

					if(m_pDataHandle && m_pDataHandle->IsConnected())
					{
						//m_pDataHandle->WriteCoreDebugItemValue(L"updatebat",L"1");
					}
				}
				else if(strExeName.Find(_T(".s3db"))>=0)
				{
					strLog.Format(_T("DTU:Receive S3db Update File(%s)..."),strExeName);
					OutPutUpdateLog(strLog);

					if(m_pDataHandle && m_pDataHandle->IsConnected())
					{
						m_pDataHandle->WriteCoreDebugItemValue(L"updates3db",L"1");
					}
				}
				//发送回执
				m_strUpdateExeAck = Project::Tools::WideCharToAnsi(ze.name);
			}
		}
		CloseZip(hz);

		if(bUpdateDll)
		{
			CString strLog;
			strLog.Format(_T("DTU:Receive Dll Update File(%s)..."),Project::Tools::AnsiToWideChar(m_strCoreFileName.c_str()).c_str());
			OutPutUpdateLog(strLog);

			if(m_pDataHandle && m_pDataHandle->IsConnected())
			{
				m_pDataHandle->WriteCoreDebugItemValue(L"updatedll",L"1");
			}
		}
	}
	m_strRecFileName = "";
}

bool CDTUSender::SendRealDataFile( string strFilePath ,int nPointCount)
{
	bool bResult = true;
	std::ostringstream sqlstream,sqlstream_index;
	if(strFilePath == "")		//查找失败
	{
		sqlstream << "frp:3;";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	}
	else
	{
		//读文件
		gzFile gz = gzopen(strFilePath.c_str(),"rb");
		if(!gz)
			return false;

		char **cFile = new char *[FILE_SIZE];
		for(int i=0;i<FILE_SIZE;++i)
		{
			cFile[i] = new char[ONE_FILE_SIZE];
		}

		int nLastLen = 0;
		int nCount = 0;
		//开始传送文件
		for(;;)
		{
			//一次读取BLOCKSIZE大小的文件内容
			nLastLen = gzread(gz,cFile[nCount], FILE_UPDATE_LENGTH);
			cFile[nCount][nLastLen] = ';';			//加一个分号
			nCount++;
			if(nLastLen < FILE_UPDATE_LENGTH)
				break;		
		}
		//关闭文件
		gzclose(gz);

		int pos = strFilePath.find_last_of('\\');
		string strFileName(strFilePath.substr(pos + 1));

		//开始
		sqlstream.str("");
		sqlstream << "frp:0|" << strFileName << ";" << nPointCount << ";";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
		Sleep(TCP_SEND_INTERNAL);

		string strSend = "frp:2|";
		for(int i=0;i<nCount; ++i)
		{
			char cSend[ONE_FILE_SIZE] = {0};
			memcpy(cSend,strSend.data(),6);
			unsigned int length = 0;
			if(i==nCount-1)
			{
				length = nLastLen+1;
			}
			else
			{
				length = FILE_UPDATE_LENGTH+1;
			}
			memcpy(cSend+11,&length,4);
			if(m_nDTUFilePackageNumber==59)			//因为长度位置会变成; 影响解析
				m_nDTUFilePackageNumber++;
			memcpy(cSend+6,&m_nDTUFilePackageNumber,4);
			sqlstream_index << m_nDTUFilePackageNumber << ",";
			m_nDTUFilePackageNumber++;
			if(m_nDTUFilePackageNumber>=1000)
			{
				m_nDTUFilePackageNumber = 0;
			}

			cSend[10] = '|';
			cSend[15] = '|';
			memcpy(cSend+16,cFile[i],length);
			bResult = SendDataByWrap(cSend, length+16);
			Sleep(TCP_SEND_INTERNAL);
		}

		//释放
		for(int i=0;i<FILE_SIZE;++i)  
		{  
			delete cFile[i];  
			cFile[i] = NULL;  
		}  
		delete [ONE_FILE_SIZE]cFile;  
		cFile = NULL;     

		//结束
		char cSend1[256] = {0};
		strSend = "frp:1|";
		memcpy(cSend1,strSend.data(),6);
		memcpy(cSend1+11,&nLastLen,4);
		memcpy(cSend1+6,&nCount,4);

		string strFileIndex = sqlstream_index.str();
		strFileIndex.erase(--strFileIndex.end());
		strFileIndex += ";";
		memcpy(cSend1+16,strFileIndex.data(),strFileIndex.size());
		cSend1[10] = '|';
		cSend1[15] = ';';

		bResult = SendDataByWrap(cSend1,16+strFileIndex.size());
		Sleep(TCP_SEND_INTERNAL);
	}

	return bResult;
}

bool CDTUSender::SendFile( string strFilePath )
{
	bool bResult = true;
	std::ostringstream sqlstream,sqlstream_index;
	if(strFilePath == "")		//查找失败
	{
		sqlstream << "fdp:3;";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
	}
	else
	{
		//读文件
		gzFile gz = gzopen(strFilePath.c_str(),"rb");
		if(!gz)
			return false;

		char **cFile = new char *[FILE_SIZE];
		for(int i=0;i<FILE_SIZE;++i)
		{
			cFile[i] = new char[ONE_FILE_SIZE];
		}

		int nLastLen = 0;
		int nCount = 0;
		//开始传送文件
		for(;;)
		{
			//一次读取BLOCKSIZE大小的文件内容
			nLastLen = gzread(gz,cFile[nCount], FILE_UPDATE_LENGTH);
			cFile[nCount][nLastLen] = ';';			//加一个分号
			nCount++;
			if(nLastLen < FILE_UPDATE_LENGTH)
				break;		
		}
		//关闭文件
		gzclose(gz);

		int pos = strFilePath.find_last_of('\\');
		string strFileName(strFilePath.substr(pos + 1));

		//开始
		sqlstream.str("");
		sqlstream << "fdp:0|" << strFileName << ";";
		string strdata = sqlstream.str();
		bResult = SendDataByWrap((char*)strdata.data(), strdata.size());
		Sleep(TCP_SEND_INTERNAL);

		string strSend = "fdp:2|";
		for(int i=0;i<nCount; ++i)
		{
			char cSend[ONE_FILE_SIZE] = {0};
			memcpy(cSend,strSend.data(),6);
			unsigned int length = 0;
			if(i==nCount-1)
			{
				length = nLastLen+1;
			}
			else
			{
				length = FILE_UPDATE_LENGTH+1;
			}
			memcpy(cSend+11,&length,4);
			if(m_nDTUFilePackageNumber==59)			//因为长度位置会变成; 影响解析
				m_nDTUFilePackageNumber++;
			memcpy(cSend+6,&m_nDTUFilePackageNumber,4);
			sqlstream_index << m_nDTUFilePackageNumber << ",";
			m_nDTUFilePackageNumber++;
			if(m_nDTUFilePackageNumber>=1000)
			{
				m_nDTUFilePackageNumber = 0;
			}

			cSend[10] = '|';
			cSend[15] = '|';
			memcpy(cSend+16,cFile[i],length);
			bResult = SendDataByWrap(cSend, length+16);
			Sleep(TCP_SEND_INTERNAL);
		}

		//释放
		for(int i=0;i<FILE_SIZE;++i)  
		{  
			delete cFile[i];  
			cFile[i] = NULL;  
		}  
		delete [ONE_FILE_SIZE]cFile;  
		cFile = NULL;     

		//结束
		char cSend1[256] = {0};
		strSend = "fdp:1|";
		memcpy(cSend1,strSend.data(),6);
		memcpy(cSend1+11,&nLastLen,4);
		memcpy(cSend1+6,&nCount,4);

		string strFileIndex = sqlstream_index.str();
		strFileIndex.erase(--strFileIndex.end());
		strFileIndex += ";";
		memcpy(cSend1+16,strFileIndex.data(),strFileIndex.size());
		cSend1[10] = '|';
		cSend1[15] = ';';

		bResult = SendDataByWrap(cSend1,16+strFileIndex.size());
		Sleep(TCP_SEND_INTERNAL);
	}

	return bResult;
}

void CDTUSender::SetName( string strname )
{
	m_tcphandler->SetName(strname);
}

void CDTUSender::ResetTcpInfo( string strHost,u_short portnumer,string strTCPName )
{
	m_tcphandler->ReSet(strHost,portnumer,strTCPName);
}

void CDTUSender::AnalyzeWebUpdateFile( const unsigned char* buffer )
{
	try
	{
		if(buffer != NULL)
		{
			m_oleUpdateExeTime = COleDateTime::GetCurrentTime();

			if(buffer[0] == '0')		//开始
			{
				m_nCoreFileCount = 0;
				m_nLastUpdateExeSize = 0;
				m_mapFileUpdateState.clear();
				char cFileName[256] = {0};
				memcpy(cFileName,buffer+2,256);
				char* p = strtok(cFileName, "|");
				if(p)
				{
					CString str;
					str.Format(_T("DTU:Receive Core Update File Package:%s...\r\n"),Project::Tools::AnsiToWideChar(p).c_str());
					_tprintf(str);
					m_strCoreFileName = p;

					p = strtok(NULL, ";");
					if(p)
					{
						m_nCoreFileCount = ATOI(p);
						if(m_nCoreFileCount<0 || m_nCoreFileCount>MAX_UPDATE_SIZE)
							return;
						m_nUpdateFileSize = m_nCoreFileCount+10;
						for(int i=0; i<m_nCoreFileCount; ++i)
						{
							m_mapFileUpdateState[i] = 0;
						}
					}
				}
				m_nRecCoreFileCount = 0;
			}
			else if(buffer[0] == '1')
			{
				int nPos = 0;
				GetFilePackageInfo(buffer,m_nCoreFileCount,m_nLastUpdateExeSize,nPos);
				if(m_nCoreFileCount == m_nRecCoreFileCount)
				{
					GenerateUpdateFile();
				}
				else		//有丢包   启动线程去像服务器要丢失的包  直到接收时间超过1h，本次更新失败
				{
					//
					m_bHasLostFileFlag = true;
				}
			} 
			else if(buffer[0] == '2')
			{
				//int nPackageIndex = BufferToInt(buffer+2);
				//int nLength =   BufferToInt(buffer+7);
				int nPos = 0;
				int nPackageIndex = 0;
				int nLength =  0;
				GetFilePackageInfo(buffer,nPackageIndex,nLength,nPos);
				if(nPackageIndex>=0 && nPackageIndex<MAX_UPDATE_SIZE && nLength <= g_nPackageSize)
				{
					m_nUpdateFileSize = (nPackageIndex>=m_nUpdateFileSize)?nPackageIndex:m_nUpdateFileSize;
					CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize);

					memset(m_cFileUpdateBuffer[nPackageIndex],0,sizeof(m_cFileUpdateBuffer[nPackageIndex]));
					memcpy(m_cFileUpdateBuffer[nPackageIndex],buffer+nPos,nLength);
					m_mapFileUpdateState[nPackageIndex] = 1;
					m_nRecCoreFileCount++;
					CString str;
					str.Format(_T("DTU:Receive Core Update File Package:%d/%d.\r\n"),nPackageIndex,m_nCoreFileCount);
					_tprintf(str);
				}
				else
				{
					int a = 0;
				}
			}
			else if(buffer[0] == '4')		//补包后确认
			{
				if(ScanAndSendLostPackage())
				{
					m_bHasLostFileFlag = true;
				}
				else
				{
					GenerateUpdateFile();		
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
}

bool CDTUSender::GetFilePackageInfo( const unsigned char* pBuffer,int& nFirst, int& nSecond, int& nPos )
{
	char cBuffer[g_nPackageSize] = {0};
	memcpy(cBuffer,pBuffer,g_nPackageSize);
	return SplitString_(cBuffer,"|",nFirst,nSecond,nPos);
}

bool CDTUSender::SplitString_( const char* source, const char* sep,int& nFirst, int& nSecond, int& nPos )
{
	nFirst = 0;
	nSecond = 0;
	nPos = 0;
	int nCount = 0;
	int nFirstFind = 0;
	int nSecondFind = 0;
	int nThreeFind = 0;

	if(source != NULL)
	{
		for(int i=0; i<40; i++)
		{
			if(source[i] == '|' || (nCount>=1 &&source[i] == ';'))
			{
				if(nCount == 0)
				{
					nFirstFind = i;
				}
				else if(nCount == 1)
				{
					nSecondFind = i;
				}
				else if(nCount == 2)
				{
					nThreeFind = i;
				}
				nCount++;
			}
		}
	}
	if(nThreeFind > 0 )
	{
		char cBuffer[20] = {0};
		int nFirstLenth = nSecondFind-nFirstFind-1;
		nFirstLenth = nFirstLenth>20?20:nFirstLenth;
		memcpy(cBuffer,source+nFirstFind+1,nFirstLenth);
		nFirst = ATOI(cBuffer);

		nFirstLenth = nThreeFind-nSecondFind-1;
		nFirstLenth = nFirstLenth>20?20:nFirstLenth;
		memcpy(cBuffer,source+nSecondFind+1,nFirstLenth);
		nSecond = ATOI(cBuffer);
		nPos = nSecondFind+1;
		return true;
	}

	return false;
}

bool CDTUSender::DelteFoder(wstring strDir_)
{
	try
	{
		CString strDir = strDir_.c_str();
		strDir += '\0';
		SHFILEOPSTRUCT    shFileOp;  
		memset(&shFileOp,0,sizeof(shFileOp));  
		shFileOp.wFunc    = FO_DELETE;  
		shFileOp.pFrom    = strDir;  
		shFileOp.pTo    = NULL;  
		shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;  
		int nResult = ::SHFileOperation(&shFileOp);  
		return nResult == 0;
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

bool CDTUSender::CreateOrDeleteUpdateFileBuffer(int nSize /*= 1024*/,bool bDeelete /*= false*/)
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_recbufferlock);
	try
	{
		m_oleUpdateFileTime = COleDateTime::GetCurrentTime(); 
		if(bDeelete)
		{
			if(m_cFileUpdateBuffer)
			{
				for(int i=0; i<=nSize; ++i)
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
			if(m_cFileUpdateBuffer == NULL)
			{
				m_cFileUpdateBuffer = new char*[nSize];
				for(int i=0; i<=nSize; ++i)
				{
					m_cFileUpdateBuffer[i] = new char[g_nPackageSize];
				}
				return true;
			}
			else
			{
				if(nSize>m_nUpdateFileSize)
				{
					for(int i=0; i<m_nUpdateFileSize; ++i)
					{
						delete[] m_cFileUpdateBuffer[i];
						m_cFileUpdateBuffer[i] = NULL;
					}
					delete[] m_cFileUpdateBuffer;
					m_cFileUpdateBuffer = NULL;

					m_cFileUpdateBuffer = new char*[nSize];
					for(int i=0; i<=nSize; ++i)
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

void CDTUSender::ReleaseUpdateBufferSize(int nTimeOut /*= 300*/)
{
	try
	{
		if(m_cFileUpdateBuffer)
		{
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleUpdateFileTime;
			if(oleSpan.GetTotalSeconds() >= nTimeOut)
			{
				CreateOrDeleteUpdateFileBuffer(m_nUpdateFileSize,true);
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

UINT WINAPI CDTUSender::DTUThreadReleaseUpdateSize(LPVOID lparam)
{
	CDTUSender* pthis = (CDTUSender*)lparam;
	if (!pthis){
		return 0;
	}

	while(!pthis->GetReadThreadExit())
	{
		pthis->ReleaseUpdateBufferSize();
		Sleep(1000);
	}
	return 0;
}
