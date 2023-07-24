#include "StdAfx.h"
#include "DataHandler.h"

#include <vector>
#include <utility>
#include <string>
#include <sstream>

#include "../Tools/CustomTools/CustomTools.h"
#include "../Log/Log/IConsoleLog.h"
#include <utility>

const char* POINTDATA_TABLENAME = "realtime_data";
const char* OPERATIONRECORD_TABLENAME = "operationrecord_data";
const char* CALCRESULT_TABLENAME = "calcresult_data";
const char* tablename_input = "realtimedata_input";
const char* tablename_historydata = "historydata";
const char* sql_createtable =  "(`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
							   `pointname` varchar(64) NOT NULL,\
							   `value` double(20,2) NOT NULL,\
							   PRIMARY KEY (`time`,`pointname`)) \
							   ENGINE=MyISAM DEFAULT CHARSET=utf8";

const char* tablename_warningoperation = "unit05";
const char* tablename_warningrecord = "warningrecord";

CDataHandler::CDataHandler(void)
	: m_bConnectSuccess(false)
	, m_strDataBaseName("")
{
}


CDataHandler::~CDataHandler(void)
{
}

bool CDataHandler::Connect(mapentry entry)
{
	m_bConnectSuccess = ConnectToDB(Project::Tools::AnsiToWideChar(entry.ip.c_str()), Project::Tools::AnsiToWideChar(entry.user.c_str()), Project::Tools::AnsiToWideChar(entry.psw.c_str()), Project::Tools::AnsiToWideChar(entry.databasename.c_str()), 3306);
	m_strDataBaseName = entry.databasename;
	return m_bConnectSuccess;
}

bool CDataHandler::InsertData(const char* databasename,  const char* buffer )
{
	ASSERT(databasename != NULL);
	ASSERT(buffer != NULL);
	if ((databasename == NULL) || (buffer == NULL)){
		return false;
	}

	int len = strlen(buffer);
	int len_dbname = strlen(databasename);
	if(len == 0){
		ADEBUG(_T("insert point data buffer empty"));
		return false;
	}

	if (len_dbname == 0){
		ADEBUG(_T("insert point data  dbname empty"));
		return false;
	}

	weldtech::CPackageType::DTU_DATATYPE type = weldtech::CPackageType::GetPackageType(buffer);

	char* buffer_copy = _strdup(buffer);
	weldtech::CPackageType::RemovePrefix(buffer_copy);

	bool bresult = false;
	switch(type)
	{
		case weldtech::CPackageType::Type_PointData:
			bresult =  InsertPointData(databasename, buffer_copy);
			break;
		case weldtech::CPackageType::Type_OperationRecord:
			bresult =  InsertOperationRecord(databasename, buffer_copy);
			break;
		case weldtech::CPackageType::Type_OptimizeResult:
			bresult = InsertCalcResultRecord(databasename, buffer_copy);
			break;
		case weldtech::CPackageType::Type_InValid:
			bresult =  InsertPointData(databasename, buffer_copy);
			break;
		case weldtech::CPackageType::Type_PointData_WithTime:
			bresult = UpdateRealDataInput(buffer_copy);
			break;
		default:
			ASSERT(false);
			break;
	}

	free(buffer_copy);

	return bresult;
}

bool CDataHandler::Init(mapentry entry)
{
	return Connect(entry);
}

bool CDataHandler::InsertPointData( const char* databasename, const char* buffer )
{
	string headersql;
	GeneratePointDataHeader(databasename, headersql);
	
	return GenerateSqlAndInsert(headersql, buffer);
}

bool CDataHandler::InsertOperationRecord( const char* databasename, const char* buffer )
{
	string headersql;
	GenerateOperationRecordDataHeader(databasename, headersql);
	SetConnectionCharacterSet(MYSQL_UTF8);

	return GenerateSqlAndInsert(headersql, buffer);
}

bool CDataHandler::InsertCalcResultRecord( const char* databasename, const char* buffer )
{
	string headersql;
	GenerateCalResultDataHeader(databasename, headersql);

	return GenerateSqlAndInsert(headersql, buffer);
}

bool CDataHandler::GenerateSqlAndInsert( const string& sqlheader, const char* buffer )
{
	if(!m_bConnectSuccess)
		return false;

	string strcopy = buffer;
	//replace
	int lastquote = 0;
	for (unsigned int i = 0; i < strcopy.length(); i++){
		if (strcopy[i] == ';'){
			strcopy[i] = ',';
			lastquote = i;
		}
	}
	strcopy[lastquote] = '\0';

	string sql = sqlheader;
	sql += strcopy;

	bool bResult = Execute(sql);
	if (!bResult){
		LogDBError(sql);
	}

	return bResult;
}

void CDataHandler::LogDBError(const string& sql)
{
	wstring sqlstatement = Project::Tools::AnsiToWideChar(sql.c_str());
	wstring mysqlerror = Project::Tools::AnsiToWideChar(GetMysqlError());
	mysqlerror += L" while ";
	mysqlerror += sqlstatement;
	ADEBUG(mysqlerror.c_str());
}

void CDataHandler::GenerateSQLHeader( weldtech::CPackageType::DTU_DATATYPE type,
									  const char* databasename,
									  string& headerresult )
{
	headerresult.clear();
	switch(type)
	{
	case weldtech::CPackageType::Type_PointData:
		GeneratePointDataHeader(databasename, headerresult);
		break;
	case weldtech::CPackageType::Type_OperationRecord:
		GenerateOperationRecordDataHeader(databasename, headerresult);
		break;
	case weldtech::CPackageType::Type_OptimizeResult:
		GenerateCalResultDataHeader(databasename, headerresult);
		break;
	case weldtech::CPackageType::Type_InValid:
		GeneratePointDataHeader(databasename, headerresult);
		break;
	}

}

void CDataHandler::GeneratePointDataHeader( const char* databasename, string& headerresult )
{
	headerresult += "insert into ";
	headerresult += databasename;
	headerresult += ".";
	headerresult += POINTDATA_TABLENAME;
	headerresult += "(pointname, value) values";
}

void CDataHandler::GenerateOperationRecordDataHeader( const char* databasename, string& headerresult )
{
	headerresult = "insert into ";
	headerresult += databasename;
	headerresult += ".";
	headerresult += OPERATIONRECORD_TABLENAME;
	headerresult += " values";
}

void CDataHandler::GenerateCalResultDataHeader( const char* databasename, string& headerresult )
{
	headerresult = "insert into ";
	headerresult += databasename;
	headerresult += ".";
	headerresult += CALCRESULT_TABLENAME;
	headerresult += " values";
}

bool CDataHandler::InsertPointDataWithTime( const char* databasename, const char* buffer )
{
	if(!m_bConnectSuccess)
		return false;

	string newtablename = GenerateTableName();
	string sql = "";
	sql += "insert into ";
	sql += databasename;
	sql += ".";
	sql += newtablename;
	sql += " values(time,pointname,value) ";

	//build the sql statement.
	// first , get the time string; data format is '2009-03-04 13-23-01';pointname,value;...
	//
	// split the string.
	// 
	char* newbuffer = _strdup(buffer);
	string strtime;
	char* p = strtok(newbuffer, ";");
	if (p){
		strtime = p;
	}

	vector< std::pair<string, string> > valuepointlist;
	p = strtok(NULL, ";");
	while(p){
		char name[256], value[256];
		memset(name, 0, 256);
		memset(value, 0, 256);
		
		int i = 0;
		char* q = p;
		while(*q++){
			i++;
			if((*q) == ','){
				break;
			}
		}
		strcpy(value, q);
		*q = '\0';
		strcpy(name, p);


		sql+="(";
		sql += strtime;
		sql += ",'";
		sql += name;
		sql += "'";
		sql += value;
		sql += "),";
		
		p = strtok(NULL, ";");
	}

	sql.erase(--sql.end());

	delete newbuffer;
	
	bool bresult = Execute(sql);
	if (!bresult)
	{
		// if the table does not exist.
		//create the table and insert again.
		if (CreateNewTable(databasename)){
			bresult =  Execute(sql);
		}
	}

	return bresult;
}

bool CDataHandler::CreateNewTable(const string& dbname)
{
	if(!m_bConnectSuccess)
		return false;

	string newtablename = GenerateTableName();
	std::ostringstream sqlstream;
	sqlstream << "create table if not exists "  << dbname << "." << newtablename <<  sql_createtable;

	bool bresult =  Execute(sqlstream.str());
	return bresult;
}

string CDataHandler::GenerateTableName()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	char buff[64];
	sprintf_s(buff, "%s_%d_%02d", POINTDATA_TABLENAME, st.wYear, st.wMonth);
	string newtablename = buff;

	return newtablename;
}

bool CDataHandler::UpdateRealDataInput( const char* buffer )
{
	if(!m_bConnectSuccess)
		return false;

	string sql;
	
	char* newbuffer = _strdup(buffer);
	string strtime;
	char* p = strtok(newbuffer, ";");
	if (p){
		strtime = p;
	}

	 StartTransaction();
	vector< std::pair<string, string> > valuepointlist;
	p = strtok(NULL, ";");
	while(p){
		char name[256], value[256];
		memset(name, 0, 256);
		memset(value, 0, 256);

		int i = 0;
		char* q = p;
		while(*q++){
			i++;
			if((*q) == ','){
				break;
			}
		}
		strcpy(value, q);
		*q = '\0';
		strcpy(name, p);

		sql = "";
		sql = "update realtimedata_input set time='";
		sql += strtime;
		sql += "',pointvalue='";
		sql += value;
		sql += "' where pointname = '";
		sql += name;
		sql += "';";

		 Execute(sql);
		
		p = strtok(NULL, ";");
	}
	delete newbuffer;
	 Commit();

	return true;
}

bool CDataHandler::UpdateInputTable(const string& dtuname)
{
	if(!m_bConnectSuccess)
		return false;

	hash_map<string,spointwatch> m_mapPoint = DTUAddressMap::GetInstance()->GetPointListByDTUName(dtuname);
	if (m_mapPoint.size()<=0){
		return false;
	}
	
	 StartTransaction();
	string sql = "delete from realtimedata_input";
	 Execute(sql);

	sql = "insert into realtimedata_input values ";
	for (hash_map<string,spointwatch>::const_iterator constIt = m_mapPoint.begin(); constIt != m_mapPoint.end(); constIt++)
	{
		spointwatch point = (*constIt).second;
		sql += "('";
		sql += point.strtime;
		sql += "','";
		sql += point.strpointname;
		sql += "','";
		sql += point.strvalue;
		sql += "'),";

	}
	sql.erase(--sql.end());
	bool result =  Execute(sql);
	 Commit();

	return result;
}

string CDataHandler::QueryOutputTable()
{
	if(!m_bConnectSuccess)
		return "";

	string m_sendbuffer;

	MYSQL_RES* result =  QueryAndReturnResult("SELECT * FROM realtimedata_output;");
	if (result == NULL){
		return m_sendbuffer;
	}
	
	MYSQL_ROW  row = NULL;
	while(row =  FetchRow(result) )
	{   
		m_sendbuffer += row[0];
		m_sendbuffer += ",";
		m_sendbuffer += row[1];
		m_sendbuffer += ";";
	}

	 FreeReturnResult(result);
	return m_sendbuffer;
}

bool CDataHandler::UpdateOutputTable( const string& strtime,const string& pointname,const string& pointvalue )
{
	if(!m_bConnectSuccess)
		return false;

	string sql = "insert into realtimedata_input values ('";
	sql += pointname;
	sql += "','";
	sql += pointvalue;
	sql += "');";
	bool result = true;
	if(! Execute(sql))
	{
		string sql = "update realtimedata_input set pointvalue ='";
		sql += pointvalue;
		sql += "' where pointname ='";
		sql += pointname;
		sql += "';";
		result =  Execute(sql);
	}
	return result;
}

bool CDataHandler::InsertHistoryData( const POINT_STORE_CYCLE &tCycle, const vector<spointwatch>& datalist )
{
	if(!m_bConnectSuccess)
		return false;

	if (datalist.empty() ){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);


	SYSTEMTIME st;
	String_To_SysTime(datalist[0].strtime, st);
	char buff[64];

	if(tCycle==E_STORE_FIVE_SECOND)
	{
		st.wSecond = 5*(st.wSecond/5);
		sprintf_s(buff, "%s_5second_%d_%02d_%02d", tablename_historydata, st.wYear, st.wMonth, st.wDay);
	}
	else if(tCycle==E_STORE_ONE_MINUTE)
	{
		st.wSecond = 0;
		sprintf_s(buff, "%s_minute_%d_%02d_%02d", tablename_historydata, st.wYear, st.wMonth, st.wDay);
	}
	else if(tCycle==E_STORE_FIVE_MINUTE)
	{	
		st.wMinute = 5*(st.wMinute/5);
		sprintf_s(buff, "%s_5minute_%d_%02d_%02d", tablename_historydata, st.wYear, st.wMonth, st.wDay);
	}
	else if(tCycle==E_STORE_ONE_HOUR)
	{	
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_hour_%d_%02d", tablename_historydata, st.wYear, st.wMonth);
	}
	else if(tCycle==E_STORE_ONE_DAY)
	{	
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_day_%d", tablename_historydata, st.wYear);
	}
	else if(tCycle==E_STORE_ONE_MONTH)
	{	
		st.wDay = 1;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_month_%02d", tablename_historydata, st.wYear);
	}
	else if(tCycle==E_STORE_ONE_YEAR)
	{	
		st.wMonth = 1;
		st.wDay = 1;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_year", tablename_historydata);
	}

	string newtablename = buff;

	std::ostringstream sqlstream;
	sqlstream << "create table if not exists " << newtablename <<  sql_createtable;

	bool bresult =  Execute(sqlstream.str());
	if (!bresult){
		return false;
	}
	// then , insert the values

	 StartTransaction();

	sqlstream.str("");
	sqlstream << "insert into "
		<< newtablename 
		<< " values";

	string str_temp = datalist[0].strtime;
	vector<spointwatch>::const_iterator it = datalist.begin();
	int nCount = 0;
	for (;it != datalist.end(); ++it)
	{
		sqlstream << "('" << str_temp << "','" 
			<< (*it).strpointname << "', '" 
			<< Project::Tools::MultiByteToUtf8((*it).strvalue.c_str()) << "'),";

		nCount++;
		if(nCount==10000)
		{
			string sql_temp = sqlstream.str();
			sql_temp.erase(--sql_temp.end());			
			bool result =   Execute(sql_temp);
			
			sqlstream.str("");
			sqlstream << "insert into "
				<< newtablename 
				<< " values";
			nCount = 0;
		}
	}

	string sql_temp = sqlstream.str();
	sql_temp.erase(--sql_temp.end());
	bool result =   Execute(sql_temp);
	 Commit();
	return result;
}

bool CDataHandler::InsertPointInput( const vector<spointwatch>& datalist )
{
	if(!m_bConnectSuccess)
		return false;

	if (datalist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();
	sqlstream.str("");
	sqlstream << "delete from " << tablename_input;
	 Execute(sqlstream.str());


	sqlstream.str("");
	sqlstream << "insert into "
		<< tablename_input 
		<< " values";

	//string str_temp = datalist[0].strtime;
	vector<spointwatch>::const_iterator it = datalist.begin();
	int nCount = 0;
	for (;it != datalist.end(); ++it)
	{
		sqlstream << "('" << (*it).strtime << "','" 
			<< (*it).strpointname << "', '" 
			<< Project::Tools::MultiByteToUtf8((*it).strvalue.c_str()) << "'),";

		nCount++;
		if(nCount==10000)
		{
			string sql_temp = sqlstream.str();
			sql_temp.erase(--sql_temp.end());
			bool result =   Execute(sql_temp);

			sqlstream.str("");
			sqlstream << "insert into "
				<< tablename_input 
				<< " values";
			nCount = 0;
		}
	}

	string sql_temp = sqlstream.str();
	sql_temp.erase(--sql_temp.end());
	bool result =   Execute(sql_temp);
	 Commit();
	return result;
}

bool CDataHandler::DeleteOutputTable()
{
	if(!m_bConnectSuccess)
		return false;
	string sql = "delete from realtimedata_output";
	return  Execute(sql);
}

bool CDataHandler::GetIsConnectSuccess()
{
	return m_bConnectSuccess;
}

bool CDataHandler::UpdateWarningRecord( const vector<warning>& datalist )
{
	for(int i=0; i<datalist.size(); ++i)
	{
		UpdateWarningTable(datalist[i]);
	}
	return true;
}

bool CDataHandler::UpdateWarningROperation( const vector<warningoperation>& datalist )
{
	StartTransaction();

	std::ostringstream sqlstream;
	string sql;
	sqlstream << "insert into " << tablename_warningoperation << " (unitproperty01, unitproperty02, unitproperty03, unitproperty04, unitproperty05) values ";
	for(int i=0; i<datalist.size(); ++i)
	{
		warningoperation op = datalist[i];
		sqlstream << "('" << op.strTime << "','" << Project::Tools::MultiByteToUtf8(op.strInfo.c_str()) << "','" << op.strPointName
			<< "','" << op.strUser << "','" <<  Project::Tools::MultiByteToUtf8(op.strOperation.c_str()) << "'),";
	}
	sql = sqlstream.str();
	sql.erase(--sql.end());
	bool result =  Execute(sql);
	Commit();
	return result;
}

bool CDataHandler::UpdateWarning( const warning war, bool bConfirmed /*= false*/ )
{
	std::ostringstream sqlstream;
	string sql;
	bool bResult = true;

	//先删除
	sqlstream << "delete from " << tablename_warningrecord << " where info='" << Project::Tools::MultiByteToUtf8(war.strInfo.c_str()) << "';";
	sql = sqlstream.str();
	bResult = Execute(sql);
	
	//插入
	sqlstream.str("");
	sqlstream << "insert into "		
		<< tablename_warningrecord 
		<< " value('" << war.strTime << "',0,'" <<  Project::Tools::MultiByteToUtf8(war.strInfo.c_str()) << "',"
		<< war.nLevel << ", '"  << war.strTime <<"', 0, '', '"<< war.strPointName<<"');"; 
	sql = sqlstream.str();
	bResult = Execute(sql);

	return bResult;
}

bool CDataHandler::CheckDBExist(mapentry entry)
{
	bool bresult = false;
	bresult =	ConnectToDB(Project::Tools::AnsiToWideChar(entry.ip.c_str()), Project::Tools::AnsiToWideChar(entry.user.c_str()), Project::Tools::AnsiToWideChar(entry.psw.c_str()), TEXT("information_schema"), 3306);	
	if(bresult)
	{
		std::ostringstream sqlstream;
		string sql;
		sqlstream << "SELECT * FROM information_schema.SCHEMATA where SCHEMA_NAME = '" << entry.databasename << "'";
		sql = sqlstream.str();
		MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
		if (result == NULL){
		bresult =  false;
		}
		else
		{
			int length = static_cast<int>(GetRowNum(result));
			if (0 == length){
			bresult =  false;
			}
			FreeReturnResult(result);
		}

		DisConnectFromDB();
	}
	return bresult;

}

bool CDataHandler::ReBuildDatabase(mapentry entry)
{
	bool bresult = false;
	bresult = ConnectToDB(Project::Tools::AnsiToWideChar(entry.ip.c_str()), Project::Tools::AnsiToWideChar(entry.user.c_str()), Project::Tools::AnsiToWideChar(entry.psw.c_str()), TEXT(""), 3306);	
	
	if(bresult)
	{
		bool bR = false;

		char* cFind = "beopdata";
		char cReplace[56];
		memset(cReplace,0,56);
		memcpy(cReplace,entry.databasename.data(),entry.databasename.length());
		
		//删除并创建数据库
		//m_dbSession.Execute(E_DROPDB_BEOP);	 
		bool bResult = Execute(RelaceStr(E_CREATEDB_BEOP,cFind,cReplace));
		int nCount = 0;

		if(!bResult)
		{
			nCount++;
		}

		//删除表
		 StartTransaction();
		bResult = Execute(RelaceStr(E_DROPTB_BEOPINFO,cFind,cReplace));
		if(!bResult)
		{
			nCount++;
		}
		Execute(RelaceStr(E_DROPTB_LOG,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_OPERATION_RECORD,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_POINT_VALUE_BUFFER,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_REALTIMEDATA_INPUT,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_REALTIMEDATA_OUTPUT,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT01,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT02,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT03,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT04,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT05,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT06,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT07,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT08,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT09,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_UNIT10,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_USERLIST_ONLINE,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_WARNING_CONFIG,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_WARNING_RECORD,cFind,cReplace));
		Execute(RelaceStr(E_DROPTB_FILESTORAGE,cFind,cReplace));
		 Commit();

		//创建表
		 StartTransaction();
		Execute(RelaceStr(E_CREATETB_BEOPINFO,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_LOG,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_OPERATION_RECORD,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_POINT_VALUE_BUFFER,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_REALTIMEDATA_INPUT,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_REALTIMEDATA_OUTPUT,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT01,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT02,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT04,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT05,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT06,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT07,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT08,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT09,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_UNIT10,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_USERLIST_ONLINE,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_WARNING_CONFIG,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_WARNING_RECORD,cFind,cReplace));
		Execute(RelaceStr(E_CREATETB_FILESTORAGE,cFind,cReplace));
		 Commit();

		//插入表
		 StartTransaction();
		Execute(RelaceStr(E_INSERTTB_BEOPINFO,cFind,cReplace));
		Execute(RelaceStr(E_INSERTTB_USERLIST_ONLINE,cFind,cReplace));
		Execute(RelaceStr(E_INSERTTB_WARNING_CONFIG,cFind,cReplace));
		Commit();
	 DisConnectFromDB();
	}

	return true;
}

string CDataHandler::RelaceStr( char* strSrc, char* strFind, char* strReplace)
{
	size_t pos = 0;
	wstring tempStr = AnsiToWideChar(strSrc);
	wstring oldStr = AnsiToWideChar(strFind);
	wstring newStr = AnsiToWideChar(strReplace);

	wstring::size_type newStrLen = newStr.length();
	wstring::size_type oldStrLen = oldStr.length();
	while(true)
	{
		pos = tempStr.find(oldStr, pos);
		if (pos == wstring::npos) break;

		tempStr.replace(pos, oldStrLen, newStr);        
		pos += newStrLen;
	}

	string strtempStr_Ansi;
	WideCharToUtf8(tempStr,strtempStr_Ansi);

	/*char strReturn[10000];
	memset(strReturn,0,10000);
	memcpy(strReturn,strtempStr_Ansi.data(),strtempStr_Ansi.length());*/
	return strtempStr_Ansi; 
}

bool CDataHandler::UpdateWarningTable( const warning war )
{
	std::ostringstream sqlstream;
	string sql;
	bool bResult = true;

	string strHappenTime_Utf8 = Project::Tools::MultiByteToUtf8(war.strHappenTime.c_str());
	string strEndTime_Utf8 = Project::Tools::MultiByteToUtf8(war.strTime.c_str());
	string strInfo_Utf8 = Project::Tools::MultiByteToUtf8(war.strInfo.c_str());

	//先删除
	sqlstream << "delete from " << tablename_warningrecord << " where info='" << strInfo_Utf8 << "' and time='"
		<< strHappenTime_Utf8 << "';";
	sql = sqlstream.str();
	bResult = Execute(sql);

	//插入
	sqlstream.str("");
	sqlstream << "insert into "		
		<< tablename_warningrecord 
		<< " value('" << strHappenTime_Utf8 << "',0,'" <<  strInfo_Utf8 << "',"
		<< war.nLevel << ", '"  << strEndTime_Utf8 <<"', " << war.nConfirmed << ", '', '"<< war.strPointName<<"');"; 
	sql = sqlstream.str();
	bResult = Execute(sql);

	return bResult;
}

bool CDataHandler::ReadRealPointFromInput( vector<spointwatch>& datalist )
{
	if(!m_bConnectSuccess)
		return false;

	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	datalist.clear();
	
	MYSQL_RES* result =  QueryAndReturnResult("SELECT * FROM realtimedata_input;");
	if (result == NULL){
		return false;
	}
	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	datalist.reserve(length);

	MYSQL_ROW  row = NULL;
	while(row =  FetchRow(result) )
	{   
		spointwatch point;
		point.nStore = E_STORE_NULL;
		point.strtime = row[0];
		point.strpointname = row[1];
		point.strvalue = row[2];
		datalist.push_back(point);
	}
	FreeReturnResult(result);
	return true;
}

