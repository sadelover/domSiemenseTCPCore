#include "StdAfx.h"
#include "DTUServerHandler.h"

#include "../Log/log/IConsoleLog.h"

#include "wcomm_dll.h"
#include "../Tools/zlib.h"

const int DTU_SEND_PACKAGE_INTERNAL = 1*1000;	//dtu连续发包的间隔时间
const int DTU_PACKAGE_SIZE = 2048;				//分包时最大包长。
const int MAX_POINT_LENGTH = 100;				//点名最大长度。

CDTUServerHandler::CDTUServerHandler()
{
	m_isbusy = false;
	memset(m_rawbuffer, 0x00, BUFFERSIZE);
	memset(m_id, 0x00, NAMESIZE);
	m_rawbuffersize = 0;
	m_bexitthread_history = false;
	m_tLastHistorySaved = COleDateTime::GetCurrentTime();
	m_tLastReceiveData = COleDateTime::GetCurrentTime();
	m_sendthread = NULL;
	m_hUpdateHistoryThread = NULL;
	m_nUpdateHistoryType = 0;
	m_bUpdateData = false;
	m_strLog = _T("");
	m_nLogCount = 0;
}

CDTUServerHandler::~CDTUServerHandler(void)
{
	m_bexitthread_history = true;
	WaitForSingleObject(m_cmdthread, INFINITE);
	WaitForSingleObject(m_sendthread, INFINITE);
	WaitForSingleObject(m_hUpdateHistoryThread, INFINITE);
	//WaitForSingleObject(m_datathread, INFINITE);

	if (m_hUpdateHistoryThread != NULL){
		::CloseHandle(m_hUpdateHistoryThread);
		m_hUpdateHistoryThread = NULL;
	}

	if (m_cmdthread != NULL){
		::CloseHandle(m_cmdthread);
		m_cmdthread = NULL;
	}

	if (m_sendthread != NULL){
		::CloseHandle(m_sendthread);
		m_sendthread = NULL;
	}

	if (m_datathread != NULL){
		::CloseHandle(m_datathread);
		m_datathread = NULL;
	}

	if (m_event != NULL){
		::CloseHandle(m_event);
		m_event = NULL;
	}

	if (m_eUpdateHistory != NULL){
		::CloseHandle(m_eUpdateHistory);
		m_eUpdateHistory = NULL;
	}
	//do_close_one_user((unsigned char *)m_id,NULL);
}

void CDTUServerHandler::SetDataEvent()
{
	SetEvent(m_event);
}

void CDTUServerHandler::ResetDataEvent()
{
	ResetEvent(m_event);
}

UINT WINAPI CDTUServerHandler::ThreadFunc( LPVOID lparam )
{
	CDTUServerHandler* pthis = (CDTUServerHandler*)lparam;
	if (pthis != NULL)
	{
		pthis->HandleData();
	}

	return 0;
}

void CDTUServerHandler::HandleData()
{
	while(!GetExitThread_History())
	{
		DWORD waitflag = WaitForSingleObject(m_event, INFINITE);
		if (waitflag == WAIT_OBJECT_0)
		{
			m_isbusy = true;
			Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
			UpdateDTUEntryList((const char*)m_rawbuffer);
			m_isbusy = false;
		}
	}

}

bool CDTUServerHandler::Init(mapentry entry)
{
	bool bresult = m_dl.Init(entry);
	if (!bresult){
		ADEBUG(_T("Connect to database server failed"));
		return false;
	}

	strcpy_s(m_id, entry.dtuname.c_str());
	m_mapPoint.clear();

	m_datathread = (HANDLE)_beginthreadex(NULL, 0 , ThreadFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	//m_cmdthread = (HANDLE)_beginthreadex(NULL, 0 , ThreadUpdateInput, this, NORMAL_PRIORITY_CLASS, NULL);
	m_hUpdateHistoryThread = (HANDLE)_beginthreadex(NULL, 0 , ThreadUpadateHistory, this, NORMAL_PRIORITY_CLASS, NULL);

	if(entry.bSendData)
		m_sendthread = (HANDLE)_beginthreadex(NULL, 0 , ThreadSendCmd, this, NORMAL_PRIORITY_CLASS, NULL);

	m_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_eUpdateHistory = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	InitRealDataFromInput();
	return true;
}

bool CDTUServerHandler::IsBusy() const
{
	return m_isbusy;
}

void CDTUServerHandler::SetData( const char* rawbuffer, const int rawsize, const char* id )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	m_isbusy = true;
	memcpy(m_rawbuffer, rawbuffer, rawsize);
	m_rawbuffersize = rawsize;
	strcpy_s(m_id, id);
	SetDataEvent();
}

string CDTUServerHandler::GetDTUHandleName()
{
	return m_id;
}

UINT WINAPI CDTUServerHandler::ThreadSendCmd( LPVOID lparam )
{
	CDTUServerHandler* pthis = (CDTUServerHandler*)lparam;
	if (pthis != NULL)
	{
		while(!pthis->GetExitThread_History())
		{
			pthis->SendData();
			Sleep(2000);
		}	
	}

	return 0;
}

void CDTUServerHandler::SendData()
{
	string m_sendbuffer = m_dl.QueryOutputTable();
	vector<string> bufferlist;
	SYSTEMTIME st;
	GetSystemTime(&st);
	BuildPackage(m_sendbuffer, bufferlist);
	if (bufferlist.empty()){
		return;
	}

	string pointdataprefix = "pdp:time:";
	string timestring = Project::Tools::SystemTimeToString(st);
	for(unsigned int i = 0; i < bufferlist.size(); i++)
	{
		string strdata = pointdataprefix;
		strdata += "'";
		strdata += timestring;
		strdata += "';";
		strdata += bufferlist[i];
		char* strbuf;
		int len = strdata.length();
		strbuf = (char *)malloc((len+1)*sizeof(char));
		strdata.copy(strbuf,len,0);
		int nError = do_send_user_data((unsigned char *)m_id, (unsigned char *)strbuf, len, NULL);
		if(nError !=0) //发送失败
		{
			return;
		}
		Sleep(DTU_SEND_PACKAGE_INTERNAL);
	}

	//char* strbuf = "a1234";
	////strcpy_s(strbuf, "a1234");
	//int nError = do_send_user_data((unsigned char *)m_id, (unsigned char *)strbuf, 6, NULL);
	//if(nError !=0 )
	//{
	//	return;
	//}
	m_dl.DeleteOutputTable();
}

UINT WINAPI CDTUServerHandler::ThreadUpdateInput( LPVOID lparam )
{
	CDTUServerHandler* pthis = (CDTUServerHandler*)lparam;
	if (pthis != NULL)
	{
		int nSecondCount = 0;
		while (!pthis->GetExitThread_History())
		{
			//30分钟没收到数据 暂停历史数据及实时表的更新
			COleDateTime tnow = COleDateTime::GetCurrentTime();
			COleDateTimeSpan tSpan = tnow - pthis->m_tLastReceiveData;
			if(tSpan.GetTotalMinutes() >= 30)
			{
				continue;
			}

			if(nSecondCount>=5)
			{
				pthis->UpdateInput();
				pthis->UpdateHistoryTable(E_STORE_FIVE_SECOND);
				nSecondCount = 0;
			}
			if (tnow.GetMinute()!= pthis->m_tLastHistorySaved.GetMinute())
			{
				pthis->UpdateHistoryTable(E_STORE_ONE_MINUTE);

				if (tnow.GetMinute()%5==0)
				{
					pthis->UpdateHistoryTable(E_STORE_FIVE_MINUTE);
				}

				if(tnow.GetHour()!=pthis->m_tLastHistorySaved.GetHour())
					pthis->UpdateHistoryTable(E_STORE_ONE_HOUR);

				if(tnow.GetDay()!=pthis->m_tLastHistorySaved.GetDay())
					pthis->UpdateHistoryTable(E_STORE_ONE_DAY);

				if(tnow.GetMonth()!=pthis->m_tLastHistorySaved.GetMonth())
					pthis->UpdateHistoryTable(E_STORE_ONE_MONTH);

				pthis->m_tLastHistorySaved = tnow;
			}
			Sleep(1*1000);
			nSecondCount++;
		}
	}

	return 0;
}

void CDTUServerHandler::UpdateInput()
{
	vector<spointwatch> entrylist;
	GetDataEntryList(entrylist);
	if (entrylist.empty()){
		return;
	}

	bool bresult = m_dl.InsertPointInput(entrylist);

}

void CDTUServerHandler::BuildPackage( const string& buffer, vector<string>& resultlist )
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

bool CDTUServerHandler::UpdateOutput( const string& strtime,const string& pointname,const string& pointvalue )
{
	m_mapPoint[pointname].strvalue = pointvalue;
	return m_dl.UpdateOutputTable(strtime,pointname,pointvalue);
}

bool CDTUServerHandler::GetExitThread_History() const
{
	return m_bexitthread_history;
}

void CDTUServerHandler::UpdateDTUEntryList( const char* buffer )
{
	weldtech::CPackageType::DTU_DATATYPE type = weldtech::CPackageType::GetPackageType(buffer);
	char* newbuffer = _strdup(buffer);
	weldtech::CPackageType::RemovePrefix(newbuffer);

	bool bresult = false;
	switch(type)
	{
	case weldtech::CPackageType::Type_PointData:
		{
			//AnzlyzeDTUData(newbuffer);
		}
		break;
	case weldtech::CPackageType::Type_OperationRecord:
		//bresult =  InsertOperationRecord(databasename, buffer_copy);
		break;
	case weldtech::CPackageType::Type_OptimizeResult:
		//bresult = InsertCalcResultRecord(databasename, buffer_copy);
		break;
	case weldtech::CPackageType::Type_InValid:
		//bresult =  InsertPointData(databasename, buffer_copy);
		break;
	case weldtech::CPackageType::Type_PointData_WithTime:
		AnalyzeRealData(newbuffer);
		break;
	case weldtech::CPackageType::Type_WarningData:						//报警数据
		AnalyzeWarningData(newbuffer);
		break;
	case weldtech::CPackageType::Type_WarningOperation:					//报警操作
		AnalyzeWarningOperation(newbuffer);
		break;
	default:
		ASSERT(false);
		break;
	}
	free(newbuffer);

	
}

void CDTUServerHandler::AnzlyzeDTUData( const char* newbuffer )
{
	
}

void CDTUServerHandler::GetDataEntryList( vector<spointwatch> &entrylist,const POINT_STORE_CYCLE &tCycle)
{
	entrylist.clear();
	for (hash_map<string,spointwatch>::const_iterator constIt = m_mapPoint.begin(); constIt != m_mapPoint.end(); constIt++)
	{
		if((*constIt).second.nStore == tCycle)
		{
			entrylist.push_back((*constIt).second);
		}
	}
}

void CDTUServerHandler::GetDataEntryList( vector<spointwatch> &entrylist )
{
	entrylist.clear();
	for (hash_map<string,spointwatch>::const_iterator constIt = m_mapPoint.begin(); constIt != m_mapPoint.end(); constIt++)
	{	
		entrylist.push_back((*constIt).second);
	}
}

void CDTUServerHandler::UpdateHistoryTable(const POINT_STORE_CYCLE &tCycle )
{
	vector<spointwatch> entrylist;

	GetDataEntryList(entrylist, tCycle);

	if (entrylist.empty()){
		return;
	}

	bool bresult = m_dl.InsertHistoryData(tCycle, entrylist);
}

void CDTUServerHandler::AnalyzeRealData( const char* buffer )
{
	string strtime;
	char* p = strtok((char*)buffer, ";");
	if (p){
		strtime = p;
	}

	p = strtok(NULL, ";");
	while(p){
		//逗号分隔
		vector<string> vec;
		SplitString(p,",",vec);
		if(vec.size() == 3)
		{
			spointwatch point;
			point.nStore = (POINT_STORE_CYCLE)(atoi(vec[0].c_str()));
			point.strtime = strtime;
			point.strpointname = vec[1];
			point.strvalue = vec[2];
			m_mapPoint[point.strpointname] = point;
		}

		p = strtok(NULL, ";");
	}
	m_tLastReceiveData = COleDateTime::GetCurrentTime();
	SetUpdateHistoryEventAndType(0);
	m_bUpdateData = true;
}

void CDTUServerHandler::AnalyzeWarningData( const char* buffer )
{
	string strtime;
	char* p = strtok((char*)buffer, ";");
	if (p){
		strtime = p;
	}

	p = strtok(NULL, ";");
	m_mapWarning.clear();
	// 
	while(p){
		//逗号分隔   happentime,level,info,point,confirmed
		vector<string> vec;
		SplitString(p,",",vec);
		if(vec.size() == 5)
		{
			warning war;
			war.strHappenTime = vec[0];
			war.nLevel = atoi(vec[1].c_str());
			war.strInfo = vec[2];
			war.strPointName = vec[3];
			war.nConfirmed = atoi(vec[4].c_str());
			war.strTime = strtime;
			m_mapWarning[war.strInfo] = war;
		}
		p = strtok(NULL, ";");
	}
	SetUpdateHistoryEventAndType(1);
}

void CDTUServerHandler::AnalyzeWarningOperation( const char* buffer )
{
	string strtime;
	char* p = strtok((char*)buffer, ";");
	if (p){
		strtime = p;
	}

	p = strtok(NULL, ";");
	m_mapWarningOpeartion.clear();
	while(p){
		//逗号分隔
		vector<string> vec;
		SplitString(p,",",vec);
		if(vec.size() == 4)
		{
			warningoperation oper;
			oper.strInfo = vec[0];
			oper.strPointName = vec[1];
			oper.strUser = vec[2];
			oper.strOperation = vec[3];
			oper.strTime = strtime;
			m_mapWarningOpeartion[oper.strInfo] = oper;
		}
		p = strtok(NULL, ";");
	}
	SetUpdateHistoryEventAndType(2);
}

void CDTUServerHandler::SplitString( const std::string& strsource, const char* sep, std::vector<string>& resultlist )
{
	if( !sep)
		return;

	resultlist.clear();
	char* strcopy = _strdup(strsource.c_str() );
	
	char* token = NULL;
	char* nexttoken = NULL;
	token = strtok_s(strcopy, sep, &nexttoken);

	while(token != NULL)
	{
		string str = token;
		resultlist.push_back(str);
		token = strtok_s(NULL, sep, &nexttoken);
	}
	free(strcopy);
}

UINT WINAPI CDTUServerHandler::ThreadUpadateHistory( LPVOID lparam )
{
	CDTUServerHandler* pthis = (CDTUServerHandler*)lparam;
	if (pthis != NULL)
	{
		while(!pthis->GetExitThread_History())
		{
			DWORD waitflag = WaitForSingleObject(pthis->GetUpdateHistoryEvent(), INFINITE);
			if (waitflag == WAIT_OBJECT_0)
			{
				pthis->UpdateHistoryData();
			}
		}
	}

	return 0;
}

HANDLE CDTUServerHandler::GetUpdateHistoryEvent()
{
	return m_eUpdateHistory;
}

bool CDTUServerHandler::UpdateHistoryData()
{
	if(m_nUpdateHistoryType == 0)
	{
		//更新实时表
		vector<spointwatch> entrylist;
		vector<spointwatch> entryHistorylist;
		for (hash_map<string,spointwatch>::iterator constIt = m_mapPoint.begin(); constIt != m_mapPoint.end(); constIt++)
		{	
			if(!(*constIt).second.bUpdate)
			{
				entryHistorylist.push_back((*constIt).second);	
				(*constIt).second.bUpdate = true;
			}
			entrylist.push_back((*constIt).second);			
		}

		if (!entrylist.empty()){
			//更新实时表
			m_dl.InsertPointInput(entrylist);
			CString str;
			str.Format(_T("更新实时数据%d个"),entrylist.size());
			AddLog(str);
		}
		
		if (!entryHistorylist.empty()){
			if(entryHistorylist[0].nStore > E_STORE_NULL)			//0不存储
			{
				//更新历史数据库
				m_dl.InsertHistoryData((POINT_STORE_CYCLE)entryHistorylist[0].nStore, entryHistorylist);
				CString str;
				str.Format(_T("更新历史数据%d个"),entryHistorylist.size());
				AddLog(str);
			}
		}

	}
	else if(m_nUpdateHistoryType == 1)
	{
		vector<warning> waringlist;
		for (hash_map<string,warning>::iterator constIt = m_mapWarning.begin(); constIt != m_mapWarning.end(); constIt++)
		{	
			if(!(*constIt).second.bUpdate)
			{
				waringlist.push_back((*constIt).second);	
				(*constIt).second.bUpdate = true;
			}		
		}

		if (!waringlist.empty()){
			//更新报警表
			m_dl.UpdateWarningRecord(waringlist);
			CString str;
			str.Format(_T("更新报警数据%d个"),waringlist.size());
			AddLog(str);
		}
	}
	else if(m_nUpdateHistoryType == 2)
	{
		vector<warningoperation> waringoplist;
		for (hash_map<string,warningoperation>::iterator constIt = m_mapWarningOpeartion.begin(); constIt != m_mapWarningOpeartion.end(); constIt++)
		{	
			if(!(*constIt).second.bUpdate)
			{
				waringoplist.push_back((*constIt).second);	
				(*constIt).second.bUpdate = true;
			}		
		}

		if (!waringoplist.empty()){
			//更新报警操作表
			m_dl.UpdateWarningROperation(waringoplist);
			CString str;
			str.Format(_T("更新报警操作数据%d个"),waringoplist.size());
			AddLog(str);
		}
	}
	return true;
}

bool CDTUServerHandler::SetUpdateHistoryEventAndType( int nUpdateHistoryType )
{
	m_nUpdateHistoryType = nUpdateHistoryType;
	SetEvent(m_eUpdateHistory);
	return true;
}

bool CDTUServerHandler::GetIfUpdateData()
{
	return m_bUpdateData;
}

bool CDTUServerHandler::SetUpdateData( bool bUpdateData )
{
	m_bUpdateData = bUpdateData;
	return true;
}

bool CDTUServerHandler::AddLog(CString strLog)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	string timestring = Project::Tools::SystemTimeToString(st);

	m_strLog += AnsiToWideChar(timestring.c_str()).c_str();
	m_strLog +=  _T("::");
	m_strLog += strLog;
	m_strLog += _T("\r\n");
	m_nLogCount++;

	if(m_nLogCount == 500)
	{
		m_strLog = _T("");
		m_nLogCount = 0;
	}
	return true;
}

CString CDTUServerHandler::GetLog()
{
	return m_strLog;
}

bool CDTUServerHandler::InitRealDataFromInput()
{
	vector<spointwatch> datalist;
	m_mapPoint.clear();
	if(m_dl.ReadRealPointFromInput(datalist))
	{
		for(int i=0; i<datalist.size(); ++i)
		{
			spointwatch point = datalist[i];
			m_mapPoint[point.strpointname] = point;
		}

		m_bUpdateData = true;
		return true;
	}
	return false;
}




