// -*- c++ -*-
// ***********************************************************
//
// Copyright (C) 2013 R&B
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Module:   
// Author:   zhu huawei
// Date:     2010/7/13
// Revision: 1.0
//
// ***********************************************************

#ifndef DATABASESESSION_H
#define DATABASESESSION_H

#include <stdio.h>
#include <comutil.h>
#include <WinSock.h>
#include "./mySqlInclude/mysql.h"
#include "../Tools/CustomTools/CustomTools.h"
#include <string>
using std::string;
using std::wstring;

#define BUFFER_LEN 512

typedef struct tag_DataBaseParam
{// 数据库信息参数结构体
	tag_DataBaseParam()
		:nPort(0)
	{}
	string strDBIP;        //  host地址
	string strDBIP2;	   //  备用IP
	string strDBName;      //  数据库名字
	string strRealTimeDBName;      //  实时数据库名字
	string strUserName;    //  用户名
	int    nPort;          //  端口
	int    nAutoLogin;
	int    nDTUPortNumber;
	int    nDTUBaudrate;
}gDataBaseParam;

//数据库参数
typedef struct Database_Param
{
  const char*	host;			//主机名
  const char*	user;			//用户名
  const char*	password;		//密码
  const char*	db;				//数据库名
  unsigned int	port;			//端口，一般为0
  const char*	unix_socket;	//套接字，一般为NULL
  unsigned int	client_flag;	//一般为0
}DatabaseParam, *pDatabaseParam;

//数据库参数
typedef struct Database_Config
{
	string strDBIP;				//  host地址
	string strDBName;			//  数据库名字
	string strUserName;			//  用户名
	string strPsw;				//  密码
	int    nPort;				//  端口
};

//数据库操作参数
typedef struct Data_Param
{
  char*	db_name;			//数据库名
  char*	tab_name;			//表名
  char*	col_name;			//列名
  char*	select_exp;			//选择表达式
  char*	where_def;			//选择限定条件
  char*	insert_val;			//插入值
  char*	set_exp;			//更新数据库内容的表达式
}DataParam, *pDataParam;

/*
 This class will holds a connection to the database, and provide 
 a lot of database operations , such as query, insert and update. 
*/

enum MYSQL_CHARACTERSET
{
	MYSQL_UTF8,    // utf8编码
	MYSQL_LATIN1,  // 西欧拉丁字符集
	MYSQL_GBK,     // GBK字符集
	MYSQL_GB2312   // GB2312.
};

class CDatabaseSeesion
{
public:
  CDatabaseSeesion();
  ~CDatabaseSeesion();

public:
  
  /*
  取得mysql的错误信息
  */
  const char * GetMysqlError();					//输出错误信息

  /*
  取得客户机信息
  */  
  const char * GetClientInfo( void );			

  /*
  取得主机信息
  */    
  const char * GetHostInfo( void );
  
  /*
  取得服务器信息
  */    
  const char * GetServerInfo( void );

  /*
  取得服务器状态信息
  */    
  const char * GetServerState( void );

  /*
  创建数据库
  @param db, 数据库名
  @return    错误码
  */    
  int CreateDB( char * db );		

  /*
  删除数据库
  @param db, 数据库名
  @return    错误码
  */      
  bool DropDB(const char * db );					

  /*
  取得协议信息
  @param
  @return  错误码
  */        
  int GetProtocolInfo( void );				//协议信息

  /*
  取得Field的名字
  @param FieldNum, field的索引，从0开始
  @return  列名
  */        
  char *GetFieldName( int FieldNum );		

  /*
  取得结果集的列数,一般用在取得结果集后
  @param 
  @return  列数
  */          
  unsigned int GetFieldNum( void );		

  /*
  取得结果集的行数,一般用在取得结果集后
  @param 
  @return  结果集的的行数
  */            
  my_ulonglong GetRowNum( void );			

  /*
  迭代结果集的每一行,一般用在取得结果集后
  @param 
  @return  每一行的值，到行尾返回NULL
  */              
  MYSQL_ROW GetRow( void );			

  /*
  在查询结果集中寻找任意行。偏移值为行号，范围从0到mysql_num_rows(result)-1。
  该函数要求结果集结构包含查询的所有结果，因此，so mysql_data_seek()仅应与
  mysql_store_result()联合使用，而不是与mysql_use_result()。
  */
  void SeekData( int offset );	

  /*
  释放查询的结果集。查询并得到一个结果集后，必须释放，否则会有内存泄露
  @param 
  @return 
  */
  void FreeResultSet( void );					//释放结果集

  /*
  连接到指定数据库,该函数会自动关闭当前的连接
  @param 
  @return 
  */  
  bool Connect( Database_Param *pDbParam );	//连接数据库

  //连接到数据库
  bool ConnectToDB( const wstring& host, const wstring& username, 
					const wstring& password, const wstring& database,
					unsigned int port, int connectionflags = 0);

  /*
  断开到数据库的连接
  @param 
  @return 
  */    
  void DisConnectFromDB();

  /*
  使由db指定的数据库成为由mysql指定的连接上的默认数据库（当前数据库）。
  在后续查询中，该数据库将是未包含明确数据库区分符的表引用的默认数据库。
  除非已连接的用户具有使用数据库的权限，否则mysql_select_db()将失败。
  @param 
  @return 
  */
  bool SelectDB( const char* dbname );


  /*
  运行指定sql语句.
  @param command 需要运行的sql语句可以是一个select，insert，delete

  @return true表示成功，失败为false
  */  
  bool Execute(const char* command);

/*
  运行指定sql语句.
  @param command 需要运行的sql语句可以是一个select，insert，delete

  @return true表示成功，失败为false
  */  
  bool Execute(const string& command);

  
  /*
  查询并存储结果集到本地
  @param str 查询字符串
  @return true 表示成功，反之为false
  */
  bool QueryAndStoreResult(const char *str );	


  //以下5个函数已过期
  bool UpdateRecord( Data_Param *para );		//更新记录
  bool SelectRecord( Data_Param *para );		//选择记录
  bool InsertRecord( Data_Param *para );		//插入记录
  bool DeleteRecord( Data_Param *para );			//删除记录
  bool SelectAll( Data_Param *para );			//选择所有记录

  /* 返回上次UPDATE更改的行数，上次DELETE删除的行数，或上次INSERT语句插入的行数。
   对于UPDATE、DELETE或INSERT语句，可在mysql_query()后立刻调用。
   对于SELECT语句，mysql_affected_rows()的工作方式与mysql_num_rows()类似。
  */
  my_ulonglong GetAffectedRows(void);

  /*
  如果模式为“1”，启用autocommit模式；
  如果模式为“0”，禁止autocommit模式。
  成功返回true，失败返回false
  */
  bool SetAutoCommit(bool isautocommit);

  /*
  该函数提供了关于默认客户端字符集的信息。
  可以使用mysql_set_character_set()函数更改默认的字符集。
  该函数是在MySQL 5.0.10中增加的。
  */
  void GetCharacterSetInfo(MY_CHARSET_INFO* cs);

  /*
  提交当前事务
  return true表示成功，false表示失败
  */
  bool Commit(void);

  /*
  开始一个事务
  */
  bool StartTransaction();

  /*
  回滚当前事务
  */
  bool RollBack();

  /*
  设置当前连接字符集
  */
  bool SetConnectionCharacterSet(MYSQL_CHARACTERSET charset = MYSQL_UTF8);

  /*
  拷贝数据库,该函数会创建一个名为destdb的数据库，然后将sourcedb的所有表结构和
  数据拷贝到新数据库。
  */
  bool CopyDataBase(const char* destdb, const char* sourcedb);

  //只拷贝表结构，不拷贝数据
  bool CopyDatabaseWithNoData(const char* destdb, const char* sourcedb);

  //每个查询返回一个结果集
  MYSQL_RES* QueryAndReturnResult(const char* sql);
	
  MYSQL_ROW  FetchRow(MYSQL_RES* result);
  
  my_ulonglong GetRowNum(MYSQL_RES* result);

  void  FreeReturnResult(MYSQL_RES* result);

  bool TruncateTable(const char* tablename);

  MYSQL_FIELD* FetchField(MYSQL_RES* result);

  int	GetFieldNum(MYSQL_RES* result);

  unsigned long* GetFieldLengths(MYSQL_RES* result);

  string GetCreateTableSQL(string dbname, string tablename);

  //取得当前mysql的安装目录
  string	GetMysqlInstallPath();
	
  char * GetDBName();
	
  bool	 IsConnected();
  unsigned long RealEscapeString(char *pDest, const char *pSrc, unsigned long ulLen);
	bool Execute(const string& command, unsigned long ulLen);

	bool	AddMysqlErrLog(string strOperation, string strErrInfo);
	Database_Config GetDatabaseParam();
protected:
  MYSQL			m_mysql;	//数据库连接句柄
  MYSQL_ROW		m_row;		//记录集
  MYSQL_RES		*m_pQuery;	//结果集
  MYSQL_FIELD		*m_pField;	//字段信息（结构体）
  char m_szStrSQL[BUFFER_LEN];	
  bool m_isconnected;
  Database_Config	m_dbparam;		//数据库连接信息
  private:
	  Project::Tools::Mutex	m_lock;
};

#endif //_DATABS_H
