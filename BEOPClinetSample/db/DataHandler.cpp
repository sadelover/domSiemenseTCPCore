#include "StdAfx.h"
#include "DataHandler.h"
#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <utility>

const char* POINTDATA_TABLENAME = "realtime_data";
const char* OPERATIONRECORD_TABLENAME = "operationrecord_data";
const char* CALCRESULT_TABLENAME = "calcresult_data";
const char* tablename_input = "realtimedata_input";
const char* tablename_buffer = "point_value_buffer";
const char* tablename_historydata = "historydata";
const char* sql_createtable =  "(`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
							   `pointname` varchar(64) NOT NULL,\
							   `value` varchar(256) NOT NULL,\
							   PRIMARY KEY (`time`,`pointname`)) \
							   ENGINE=InnoDB DEFAULT CHARSET=utf8";

const char* sql_createsyntable =  "(`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
								`pointname` varchar(64) NOT NULL DEFAULT '',\
								`pointvalue` varchar(256) NOT NULL DEFAULT '',\
								PRIMARY KEY (`pointname`)\
								) ENGINE=InnoDB DEFAULT CHARSET=utf8";

const char* sql_createsynuseroperationtable =  "(`RecordTime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
								`user` varchar(128) NOT NULL DEFAULT '',\
								`OptRemark` varchar(256) NOT NULL DEFAULT ''\
								) ENGINE=InnoDB DEFAULT CHARSET=utf8";

const char* sql_createsynwarningtable =  "(`time` timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
								`code` int(10) unsigned NOT NULL DEFAULT '0',\
								`info` varchar(1024) NOT NULL DEFAULT '',\
								`level` int(10) unsigned NOT NULL DEFAULT '0',\
								`endtime` timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
								`confirmed` int(10) unsigned NOT NULL DEFAULT '0',\
								`confirmeduser` varchar(2000) NOT NULL DEFAULT '',\
								`bindpointname` varchar(255) DEFAULT NULL\
								) ENGINE=InnoDB DEFAULT CHARSET=utf8";

const char* tablename_warningoperation = "unit05";
const char* tablename_warningrecord = "warningrecord";
const char* tablename_unit01 = "unit01";
const char* tablename_operationrecord = "operation_record";

SYSTEMTIME CRealTimeDataEntry::GetTimestamp() const
{
	return m_timestamp;
}

void CRealTimeDataEntry::SetTimestamp( const SYSTEMTIME& stime )
{
	m_timestamp = stime;
}

string CRealTimeDataEntry::GetName() const
{
	return m_pointname;
}

void CRealTimeDataEntry::SetName( const string& name )
{
	m_pointname = name;
}

wstring CRealTimeDataEntry::GetValue() const
{
	return m_pointvalue;
}

void CRealTimeDataEntry::SetValue(const wstring& value)
{
	m_pointvalue = value;
}

bool CRealTimeDataEntry::IsEngineSumPoint()
{
	if(m_pointname.length() <= 0)
		return false;

	unsigned int pos = m_pointname.find("time_update_");
	if(pos != m_pointname.npos)
		return true;

	unsigned int pos1 = m_pointname.find("err_engine_");
	if(pos1 != m_pointname.npos)
		return true;

	return false;
}

CDataHandler::CDataHandler(void)
	: m_bConnectSuccess(false)
	, m_strDataBaseName("")
{
}


CDataHandler::~CDataHandler(void)
{
}

bool CDataHandler::Connect(const wstring& host, const wstring& username, 
	const wstring& password, const wstring& database,
	unsigned int port)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	m_bConnectSuccess = false;
	m_bConnectSuccess = ConnectToDB(host,username,password,database,port);
	return m_bConnectSuccess;
}

bool CDataHandler::GetAllSchema( vector<CString>& vecSchema )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	vecSchema.clear();
	MYSQL_RES* result = QueryAndReturnResult("SELECT SCHEMA_NAME FROM information_schema.SCHEMATA order by SCHEMA_NAME");
	if (result == NULL)
	{
		return false;
	}

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}

	MYSQL_ROW row = NULL;
	while(row = FetchRow(result))
	{
		vecSchema.push_back(Project::Tools::AnsiToWideChar(row[0]).c_str());
	}
	FreeReturnResult(result);
	return true;
}

bool CDataHandler::GetAllTableNamesBySchema( vector<CString>& vecTable,CString strSchema,CString strNotLike /*= _T("")*/ )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	vecTable.clear();

	ostringstream sqlstream;
	if(strNotLike.GetLength() <= 0)
	{
		sqlstream << "SELECT TABLE_NAME FROM information_schema.`TABLES` where TABLE_SCHEMA ='" << Project::Tools::WideCharToAnsi(strSchema)
			<< "' order by TABLE_NAME";
	}
	else
	{
		sqlstream << "SELECT TABLE_NAME FROM information_schema.`TABLES` where TABLE_SCHEMA ='" << Project::Tools::WideCharToAnsi(strSchema)
			<< "' and TABLE_NAME not like '%" << Project::Tools::WideCharToAnsi(strNotLike) << "%' order by TABLE_NAME";
	}

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}

	MYSQL_ROW row = NULL;
	while(row = FetchRow(result))
	{
		vecTable.push_back(Project::Tools::AnsiToWideChar(row[0]).c_str());
	}
	FreeReturnResult(result);
	return true;
}

wstring CDataHandler::ReadOrCreateCoreDebugItemValue( wstring strItemName,wstring strItemValue /*= L""*/ )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return strItemValue;
	}
	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");

		string strItemValue_utf8;
		Project::Tools::WideCharToUtf8(strItemValue, strItemValue_utf8);
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '" << strItemValue_utf8 <<"','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);		
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
		}
		return strItemValue;
	}

	MYSQL_ROW  row = NULL;
	if(row = FetchRow(result) )
	{
		strItemValue =  Project::Tools::UTF8ToWideChar(row[1]);
	}
	FreeReturnResult(result);
	return strItemValue;
}

bool CDataHandler::WriteCoreDebugItemValue( const wstring &strItemName, const wstring &strItemValue )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;
	// try to insert the entry.

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);
	string strItemValue_utf8;
	Project::Tools::WideCharToUtf8(strItemValue, strItemValue_utf8);

	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '" << strItemValue_utf8 << "','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);	
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
		}
	}
	else
	{
		FreeReturnResult(result);

		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".unit01"
			<< " set unitproperty02 = '" << strItemValue_utf8
			<< "' where unitproperty01 ='" << strItemName_utf8 << "'";
		string sqlstatement = sqlstream.str();
		bool bresult =   Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
		}
		return bresult;
	}

	return true;
}

bool CDataHandler::ReadRealTimeData_Unit01( vector<unitStateInfo>& unitlist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	
	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	unitlist.reserve(length);
	MYSQL_ROW  row = NULL;
	unitStateInfo unit;
	while(row = FetchRow(result) )
	{
		unit.unitproperty01 = Project::Tools::Utf8ToMultiByte(row[0]);
		unit.unitproperty02 = Project::Tools::Utf8ToMultiByte(row[1]);
		unit.unitproperty03 = Project::Tools::Utf8ToMultiByte(row[2]);
		unitlist.push_back(unit);
	}
	FreeReturnResult(result);
	return true;
}

bool CDataHandler::ReadLogFile_Unit06( vector<unitLogFileInfo>& unitlist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit06 where unitproperty06='0'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	unitlist.reserve(length);
	MYSQL_ROW  row = NULL;
	unitLogFileInfo unit;
	while(row = FetchRow(result) )
	{
		unit.strStartTime = Project::Tools::UTF8ToWideChar(row[0]);
		unit.strEndTime = Project::Tools::UTF8ToWideChar(row[1]);
		unit.strFilter = Project::Tools::UTF8ToWideChar(row[2]);
		unit.strFileName = Project::Tools::UTF8ToWideChar(row[3]);
		unit.nType = ATOI(row[4]);
		unitlist.push_back(unit);
	}
	FreeReturnResult(result);
	return true;
}

bool CDataHandler::UpdateLogFile_Unit06( unitLogFileInfo unitlist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);
	std::ostringstream sqlstream;
	sqlstream << "update " << GetDBName() 
		<< ".unit06 set unitproperty06='1' where unitproperty01='" << Project::Tools::WideCharToAnsi(unitlist.strStartTime.c_str())
		<< "' and  unitproperty02='" << Project::Tools::WideCharToAnsi(unitlist.strEndTime.c_str())
		<< "' and  unitproperty03='" << Project::Tools::WideCharToAnsi(unitlist.strFilter.c_str())
		<< "' and  unitproperty04='" << Project::Tools::WideCharToAnsi(unitlist.strFileName.c_str())
		<< "';" ;
	string sqlstatement = sqlstream.str();
	return Execute(sqlstatement);
}

bool CDataHandler::DeleteLogFile_Unit06()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);
	std::ostringstream sqlstream;
	sqlstream << "delete from " 
		<< GetDBName() 
		<< ".unit06";
	string sqlstatement = sqlstream.str();
	return Execute(sqlstatement);
}

bool CDataHandler::UpdateLibInsertIntoFilestorage( const wstring strFilePathName, const wstring strFileName, const wstring strFileId )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	off_t			iSize;
	char            *pSql; 
	FILE            *fFp; 
	char            *cBuf; 
	size_t          n = 0; 
	char            *cEnd; 
	size_t			lSize = 0;
	string strUtf8PathName;
	string strUtf8FileName;
	string strUtf8FileId;
	USES_CONVERSION;
	strUtf8PathName = T2A(strFilePathName.c_str());
	strUtf8FileName = T2A(strFileName.c_str());
	strUtf8FileId = T2A(strFileId.c_str());
	if ((fFp = fopen(strUtf8PathName.c_str(),"rb")) == NULL) 
	{ 
		perror("fopen file"); 
		return false; 
	}
	long cur_pos;
	long len;
	cur_pos = ftell( fFp );
	fseek(fFp, 0, SEEK_END);
	len = ftell( fFp );
	//将文件流的读取位置还原为原先的值
	fseek(fFp, cur_pos, SEEK_SET);
	if ((cBuf = new char[sizeof(char)*(len+1)]) == NULL) 
	{ 
		return false; 
	}
	iSize = len;
	if ((n = fread(cBuf, 1, iSize, fFp)) < 0) 
	{ 
		return false; 
	}
	lSize = sizeof(char) * n * 2 + 256;
	pSql = new char[lSize];
	if (pSql == NULL) 
	{ 
		return false; 
	}

	//删除选文件
	std::ostringstream sqlstream;
	string sqlstatement;
	sqlstream << "delete from filestorage where fileid = '" << strUtf8FileId << "';";
	sqlstatement = sqlstream.str();
	Execute(sqlstatement);


	sqlstream.str("");
	sqlstream << "insert into filestorage(fileid, filename, filedescription, reserve01, reserve02, reserve03, reserve04, reserve05, fileblob) values('"
		<< strUtf8FileId << "','" << strUtf8FileName << "', '', '', '', '', '', '', ";
	sqlstatement = sqlstream.str();
	strcpy_s(pSql, lSize, sqlstatement.c_str());

	cEnd = pSql; 
	cEnd += strlen(pSql); //point sql tail 
	*cEnd++ = '\''; 
	cEnd += RealEscapeString(cEnd, cBuf, n); 
	*cEnd++ = '\''; 
	*cEnd++ = ')'; 

	sqlstatement = pSql;
	if (!Execute(sqlstatement, (unsigned int)(cEnd-pSql)))
	{
		fclose(fFp); 
		fFp = NULL;
		if (pSql != NULL)
		{
			delete[] pSql;
			pSql = NULL;
		}
		return false;
	}

	fclose(fFp); 
	fFp = NULL;
	if (pSql != NULL)
	{
		delete[] pSql;
		pSql = NULL;
	}

	if(cBuf!=NULL)
	{
		delete[] cBuf;
		cBuf = NULL;
	}

	return true;
}

string CDataHandler::ReadErrCodeRecentMinute( string strMintue )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);
	int nMintue = atoi(strMintue.c_str());
	COleDateTime oleTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0,0,nMintue,0);
	wstring strTime;
	Project::Tools::OleTime_To_String(oleTime,strTime);
	string strTime_Ansi = Project::Tools::WideCharToAnsi(strTime.c_str());
	std::ostringstream sqlstream;
	sqlstream << "select unitproperty01 from " 
		<< GetDBName() 
		<< ".unit08 where unitproperty02 >='" << strTime_Ansi << "' order by unitproperty02";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return "";
	}

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return "";
	}
	sqlstream.str("");
	MYSQL_ROW  row = NULL;
	while(row = FetchRow(result) )
	{
		sqlstream << Project::Tools::Utf8ToMultiByte(row[0]) << "|";
	}
	FreeReturnResult(result);
	return sqlstream.str();
}

bool CDataHandler::UpdateOutPutValue( vector<pair<wstring, wstring> >& opcvaluesets )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	if(opcvaluesets.size()<=0)
		return false;

	std::ostringstream sqlstream;
	string value_ansi;

	StartTransaction();

	for(int i=0; i<opcvaluesets.size(); i++)
	{
		sqlstream.str("");
		string value_ansi,name_ansi;
		Project::Tools::WideCharToUtf8(opcvaluesets[i].second, value_ansi);
		Project::Tools::WideCharToUtf8(opcvaluesets[i].first, name_ansi);
		sqlstream << "REPLACE INTO `realtimedata_output`(pointname, pointvalue) VALUES ('" 
			<< name_ansi << "', '" << value_ansi << "')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);
	}
	Commit();
	return true;
}
