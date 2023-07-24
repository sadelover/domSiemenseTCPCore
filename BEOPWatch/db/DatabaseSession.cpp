
#include "stdafx.h"
#include "DBInterfaceExp.h"

#include "DatabaseSession.h"
#include <string>
using std::string;
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/Scope_Ptr.h"
#include "MysqlDeleter.h"

// Construction/Destruction
CDatabaseSeesion::CDatabaseSeesion()
{
  m_row = NULL;
  m_pField = NULL;
  m_pQuery = NULL;  
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));
  mysql_init(&m_mysql);

  //设置mysql自动重连。
  char value = 1;
  mysql_options(&m_mysql, MYSQL_OPT_RECONNECT, (char *)&value);
  unsigned int connectimeout = 5;
  unsigned int readwritetimeout = 20;
  mysql_options(&m_mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&connectimeout);
  mysql_options(&m_mysql, MYSQL_OPT_READ_TIMEOUT, (char*)&readwritetimeout);
  mysql_options(&m_mysql, MYSQL_OPT_WRITE_TIMEOUT, (char*)&readwritetimeout);
  mysql_options(&m_mysql, MYSQL_SET_CHARSET_NAME, "utf8");

  m_isconnected = false;

}

CDatabaseSeesion::~CDatabaseSeesion()
{
  DisConnectFromDB();

  if (m_pQuery != NULL)
  {
    mysql_free_result(m_pQuery);
  }  
}

//Member Function
/*
@brief:查找并保存结果集
@param: char *str 查询的sql语句
*/
bool CDatabaseSeesion::QueryAndStoreResult(const char *str )
{
  if ( mysql_query( &m_mysql, str ) )
  {
    return false;
  }
  m_pQuery = mysql_store_result(&m_mysql);

  return true;
}

/*
@brief:输出错误信息
*/
const char *CDatabaseSeesion::GetMysqlError( void )
{
  return ( mysql_error( &m_mysql ) );
}

/*
@brief:客户机信息
*/
const char * CDatabaseSeesion::GetClientInfo( void )
{
  return mysql_get_client_info();
}

/*
@brief:主机信息
*/
const char * CDatabaseSeesion::GetHostInfo( void )
{
  return mysql_get_host_info( &m_mysql );
}

/*
@brief:服务器信息
*/
const char * CDatabaseSeesion::GetServerInfo( void )
{
  return mysql_get_server_info( &m_mysql );
}

/*
@brief:服务器状态
*/
const char *CDatabaseSeesion::GetServerState()
{
  const char *pState = NULL;
  pState = mysql_stat(&m_mysql);
  if(!pState)
  {
    return GetMysqlError();
  }
  return pState;
}

/*
@brief:创建数据库，返回错误信息
@param:char *db 数据库名称等信息
*/
int CDatabaseSeesion::CreateDB(char * db)
{
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));

  sprintf_s(m_szStrSQL, sizeof(m_szStrSQL), "CREATE DATABASE %s", db);
  if (mysql_query(&m_mysql, m_szStrSQL))
  {    
    return 1;
  }
  //return (mysql_create_db(&m_mysql,db));
  return 0;
}

/*
@brief:删除数据库,返回错误信息
@param:char *db 数据库名称
*/
bool CDatabaseSeesion::DropDB(const char * db )
{
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));

  sprintf_s(m_szStrSQL, sizeof(m_szStrSQL), "DROP DATABASE %s", db);
  if ( mysql_query(&m_mysql, m_szStrSQL ))
  {    
    return 1;
  }
  //return (mysql_drop_db(&m_mysql,db));
  return 0;
}

/*
@brief:协议信息
*/
int CDatabaseSeesion::GetProtocolInfo( void )
{
  return mysql_get_proto_info( &m_mysql );
}

/*
@brief:字段名
@param: FieldNum字段号(即第几个字段)
*/
char *CDatabaseSeesion::GetFieldName( int FieldNum )
{
  m_pField = mysql_fetch_field_direct( m_pQuery, FieldNum );
  return m_pField->name;
}

/*
@brief:得到字段数
@param: void
*/
unsigned int CDatabaseSeesion::GetFieldNum(void)
{
  return ( mysql_num_fields( m_pQuery ) );
}

int CDatabaseSeesion::GetFieldNum( MYSQL_RES* result )
{
	return mysql_num_fields(result);
}

/*
@brief:得到记录数
@param: void
*/
my_ulonglong CDatabaseSeesion::GetRowNum(void)
{
  return ( mysql_num_rows( m_pQuery ) );
}

/*
@brief:得到结果（一个记录）
@param: void
*/
MYSQL_ROW CDatabaseSeesion::GetRow(void)
{
  return ( m_row = mysql_fetch_row( m_pQuery ) );
}

/*
@brief:查找指定数据
@param: offset 偏移号
*/
void CDatabaseSeesion::SeekData( int offset )
{
  mysql_data_seek( m_pQuery, offset );
}

/*
@brief:释放结果集
@param: void
*/
void CDatabaseSeesion::FreeResultSet( void )
{
  if (m_pQuery != NULL)
  {
    mysql_free_result(m_pQuery);
    m_pQuery = NULL;
  }
}

/*
@brief:连接数据库
@param: Database_Param *pDbParam 数据库参数
*/
bool CDatabaseSeesion::Connect(Database_Param *pDbParam)
{
	DisConnectFromDB();	

	int repeattime = 1;
	while(repeattime--)
	{
		if (!mysql_real_connect(&m_mysql
			, pDbParam->host
			, pDbParam->user
			, pDbParam->password
			, pDbParam->db
			, pDbParam->port
			, pDbParam->unix_socket
			, pDbParam->client_flag))
		{
			string strErr = GetMysqlError();
			// log the error.
			// or send the message to the log part.
			Sleep(3*1000);
		}
		else{
			m_isconnected = true;
			SetConnectionCharacterSet(MYSQL_UTF8);
			return true;
		}
	}

	return false;
}


bool CDatabaseSeesion::ConnectToDB( const wstring& host, const wstring& username, 
									const wstring& password, const wstring& database, 
									unsigned int port, int connectionflags )
{
	_bstr_t bstrhost = host.c_str();
	_bstr_t bstrusername = username.c_str();
	_bstr_t bstrpassword = password.c_str();
	_bstr_t bstrdatabase = database.c_str();

	DatabaseParam dbparam;
	memset(&dbparam, 0x00, sizeof(dbparam));
	dbparam.host = bstrhost;
	dbparam.user = bstrusername;
	dbparam.password = bstrpassword;

	if (database.empty())
	{
		dbparam.db = NULL;
	}
	else
	{
		dbparam.db =  bstrdatabase;	
	}

	dbparam.port = port;
	dbparam.client_flag = connectionflags;

	m_dbparam.strDBIP = bstrhost;
	m_dbparam.strDBName = bstrdatabase;
	m_dbparam.strUserName = bstrusername;
	m_dbparam.strPsw = bstrpassword;
	m_dbparam.nPort = port;
	return Connect(&dbparam);		
}


/*
@brief:选择数据库
@param: Data_Param *para 数据库操作参数
*/
bool CDatabaseSeesion::SelectDB(const char* dbname)
{
  if (mysql_select_db(&m_mysql, dbname))
    return false;	
  else
    return true;
}

/*
@brief:更新记录
@param: Data_Param *para 数据库操作参数
*/
bool CDatabaseSeesion::UpdateRecord(Data_Param *para)
{
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));

  sprintf_s(m_szStrSQL, sizeof(m_szStrSQL)
    , "update %s set %s where %s"
    , para->tab_name
    , para->set_exp
    , para->where_def);
  if (mysql_query(&m_mysql, m_szStrSQL))
  {
    return false;
  }
  return true;
}

/*
@brief:选择记录
@param: Data_Param *para 数据库操作参数
*/
bool CDatabaseSeesion::SelectRecord(Data_Param *para)
{
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));

  sprintf_s(m_szStrSQL, sizeof(m_szStrSQL)
    , "select %s from %s where %s"
    , para->select_exp
    , para->tab_name, para->where_def);
  if (!QueryAndStoreResult(m_szStrSQL))
  {
    return false;
  }

  return true;
}

/*
@brief:插入记录
@param: Data_Param *para 数据库操作参数
*/
bool CDatabaseSeesion::InsertRecord(Data_Param *para)
{
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));

  sprintf_s(m_szStrSQL, sizeof(m_szStrSQL)
    , "insert into %s values(%s)"
    , para->tab_name
    , para->insert_val);

  if (mysql_query(&m_mysql, m_szStrSQL))
  {
    return false;
  }

  return true;
}

/*
@brief:删除记录
@param: Data_Param *para 数据库操作参数
*/
bool CDatabaseSeesion::DeleteRecord(Data_Param *para)
{
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));

  sprintf_s(m_szStrSQL, sizeof(m_szStrSQL)
    , "delete from %s where %s"
    , para->tab_name
    , para->where_def);
  if (mysql_query(&m_mysql, m_szStrSQL))
  {
    return false;
  }

  return true;
}

/*
@brief:选择所有记录
@param: Data_Param *para 数据库操作参数
*/
bool CDatabaseSeesion::SelectAll(Data_Param *para)
{
  memset(m_szStrSQL, 0, sizeof(m_szStrSQL));

  sprintf_s(m_szStrSQL, sizeof(m_szStrSQL)
    , "select * from %s"
    , para->tab_name);
  if (!QueryAndStoreResult(m_szStrSQL))
  {
    return false;
  }
  return true;
}

void CDatabaseSeesion::DisConnectFromDB()
{
	if (m_isconnected)
	{
		return  mysql_close(&m_mysql);
	}  
}

my_ulonglong CDatabaseSeesion::GetAffectedRows( void )
{
  return mysql_affected_rows(&m_mysql);
}

bool CDatabaseSeesion::SetAutoCommit( bool isautocommit )
{
  return !mysql_autocommit(&m_mysql, isautocommit);
}

void CDatabaseSeesion::GetCharacterSetInfo( MY_CHARSET_INFO* cs )
{
  return mysql_get_character_set_info(&m_mysql, cs);
}

bool CDatabaseSeesion::Execute( const char* command )
{
  return !mysql_query(&m_mysql, command);
}


bool CDatabaseSeesion::Execute( const string& command )
{
	// this may faster.
	return !mysql_real_query(&m_mysql, command.data(), (unsigned long)command.length()); 
}

bool CDatabaseSeesion::Commit( void )
{
  return !mysql_commit(&m_mysql);
}

bool CDatabaseSeesion::StartTransaction()
{
  return !mysql_query(&m_mysql, "start transaction;");
}

bool CDatabaseSeesion::SetConnectionCharacterSet( MYSQL_CHARACTERSET charset /*= MYSQL_UTF8*/ )
{
	string s = "set names ";
	switch(charset)
	{
	case MYSQL_UTF8 :
		s += "utf8";
		break;
	case MYSQL_LATIN1:
		s += "latin1";
		break;
	case MYSQL_GBK:
		s += "gbk";
		break;
	case MYSQL_GB2312:
		s += "gb2312";
		break;
	}

	return !mysql_query(&m_mysql, s.c_str());
}

//所传入的字符串为ansi
bool CDatabaseSeesion::CopyDataBase( const char* destdb, const char* sourcedb )
{
	string str;
	SetConnectionCharacterSet();
	
	// use the source database.
	str = "use ";
	str += sourcedb;
	if (!Execute(str.c_str()))
	{		
	#ifdef _DEBUG
		string error = GetMysqlError();
	#endif
		return FALSE;
	}

	//select all the tables from the source database.
	vector<string>  table_names;
	if (QueryAndStoreResult("show tables"))
	{
		// if succeed		
		while (GetRow())
		{
			table_names.push_back(m_row[0]);
		}
		FreeResultSet();
	}
	else
	{	
	#ifdef _DEBUG
		string error = GetMysqlError();
	#endif
		return FALSE;
	}

	str.clear();
	str = "create database ";
	str.append(destdb);
	str += " character set utf8;";	
	_bstr_t bt = str.c_str();	
	// create dabase. 
	string  sql_createdb_utf8 = Project::Tools::WideCharToUtf8(bt);
	
	if (!Execute(sql_createdb_utf8))
	{
	#ifdef _DEBUG		
		string error = GetMysqlError();
	#endif		
		return FALSE;
	}

	// set the new dabase as the default database.
	str.clear();
	str = "use ";
	str += destdb;
	bt = str.c_str();
	string sql_usedb_utf8 = Project::Tools::WideCharToUtf8(bt);
	
	if (!Execute(sql_usedb_utf8))
	{
	#ifdef _DEBUG
		string error = GetMysqlError();
	#endif
		return FALSE;
	}

	for (vector<string>::iterator it = table_names.begin(); it != table_names.end(); it++)
	{
		// create table <tablename> select * from soucedb.<tablename>
		str.clear();
		str = "create table ";
		str += *it;
		str += " select * from ";
		str += sourcedb;
		str += '.';
		str += *it;

		if (!Execute(str.c_str()))
		{
		#ifdef _DEBUG
			string error =GetMysqlError();
		#endif
			return FALSE;
		}
	}

	return TRUE;
}

MYSQL_RES* CDatabaseSeesion::QueryAndReturnResult( const char* sql )
{
	if (mysql_query(&m_mysql, sql))
	{
		return NULL;
	}
	
	return mysql_store_result(&m_mysql);
}

void CDatabaseSeesion::FreeReturnResult( MYSQL_RES* result )
{
	if (result != NULL)
	{
		mysql_free_result(result);
	}
}

MYSQL_ROW CDatabaseSeesion::FetchRow( MYSQL_RES* result )
{
	assert(result != NULL);
	return mysql_fetch_row(result);
}

my_ulonglong CDatabaseSeesion::GetRowNum( MYSQL_RES* result )
{
	assert (result != NULL);
	return mysql_num_rows(result);
}

bool CDatabaseSeesion::TruncateTable( const char* tablename )
{
	string sql = "delete from ";
	sql += tablename;

	return !mysql_query(&m_mysql, sql.c_str());
}

MYSQL_FIELD* CDatabaseSeesion::FetchField( MYSQL_RES* result )
{
	return mysql_fetch_field(result);
}

unsigned long* CDatabaseSeesion::GetFieldLengths( MYSQL_RES* result )
{
	return mysql_fetch_lengths(result);
}

string CDatabaseSeesion::GetCreateTableSQL( string dbname, string tablename )
{
	string sql = "show create table ";
	sql += dbname;
	sql += ".";
	sql += tablename;

	string resultsql;
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL){
		return resultsql;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	MYSQL_ROW row = FetchRow(result);
	if (row != NULL)
		resultsql = row[1];

	return resultsql;
}

bool CDatabaseSeesion::CopyDatabaseWithNoData( const char* destdb, const char* sourcedb )
{
	string str;
	SetConnectionCharacterSet();

	// use the source database.
	str = "use ";
	str += sourcedb;
	if (!Execute(str.c_str()))
	{		
		string error = GetMysqlError();
		return FALSE;
	}

	//select all the tables from the source database.
	vector<string>  table_names;
	if (QueryAndStoreResult("show tables"))
	{
		// if succeed		
		while (GetRow())
			table_names.push_back(m_row[0]);
		FreeResultSet();
	}
	else
	{	
		string error = GetMysqlError();
		return FALSE;
	}

	// create the new database.
	str.clear();
	str = "create database ";
	str.append(destdb);
	str += " character set utf8;";	
	// create dabase. 
	if (!Execute(str))
	{
		string error = GetMysqlError();
		return FALSE;
	}

	// set the new dabase as the default database.
	str.clear();
	str = "use ";
	str += destdb;	

	if (!Execute(str))
	{
		string error = GetMysqlError();
		return FALSE;
	}

	for (vector<string>::iterator it = table_names.begin(); it != table_names.end(); it++)
	{
		// create table <tablename> select * from soucedb.<tablename>
		string sql_createtable = GetCreateTableSQL(sourcedb, *it);

		if (!Execute(sql_createtable))
		{
			string error =GetMysqlError();
			return FALSE;
		}
	}

	return TRUE;
}

bool CDatabaseSeesion::RollBack()
{
	return !mysql_rollback(&m_mysql);
}

string CDatabaseSeesion::GetMysqlInstallPath()
{
	string sql_getinstalldir = "show variables like \'basedir\'";
	
	MYSQL_RES* result = QueryAndReturnResult(sql_getinstalldir.c_str());
	if (result == NULL){
		return "";
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	
	MYSQL_ROW row = FetchRow(result);
	assert(row != NULL);
	
	return string(row[1]);
}


char * CDatabaseSeesion::GetDBName()
{
	return m_mysql.db;
}

bool CDatabaseSeesion::IsConnected()
{
	return m_isconnected;
}


bool CDatabaseSeesion::Execute(const string& command, unsigned long ulLen)
{
	return !mysql_real_query(&m_mysql, command.data(), ulLen); 
}


unsigned long CDatabaseSeesion::RealEscapeString(char *pDest, const char *pSrc, unsigned long ulLen)
{
	return mysql_real_escape_string(&m_mysql, pDest, pSrc, ulLen);
}

bool CDatabaseSeesion::AddMysqlErrLog( string strOperation, string strErrInfo )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	wstring strFilePath;
	Project::Tools::GetSysPath(strFilePath);
	strFilePath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strFilePath))
	{
		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		CString strLogPath;
		strLogPath.Format(_T("%s\\mysql_err_%d_%02d_%02d.log"),strFilePath.c_str(),stNow.wYear,stNow.wMonth,stNow.wDay);
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			wstring strTime,strOp,strEi;
			Project::Tools::SysTime_To_String(stNow,strTime);
			strOp = Project::Tools::AnsiToWideChar(strOperation.c_str());
			strEi = Project::Tools::AnsiToWideChar(strErrInfo.c_str());
			CString strLog;
			strLog.Format(_T("%s::%s(%s)...\r"),strTime.c_str(),strOp.c_str(),strEi.c_str());
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			return true;
		}
	}
	return false;
}

Database_Config CDatabaseSeesion::GetDatabaseParam()
{
	return m_dbparam;
}
