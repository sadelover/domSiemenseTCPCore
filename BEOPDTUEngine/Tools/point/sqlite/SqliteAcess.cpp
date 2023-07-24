
#include "StdAfx.h"
#include "SqliteAcess.h"
#include <cassert>
#include "Tools/CustomTools.h"

#pragma warning(disable:4996) 

typedef int(*sqliteClose)(sqlite3 *);
typedef int(*sqliteOpen)(const char*,sqlite3** );
typedef int(*sqliteKey)(sqlite3 *,const void *,int);
typedef int(*sqliteExec)(sqlite3*,const char *,int (*callback)(void*,int,char**,char**),void *,char **);
typedef int(*sqlitePrepare)(sqlite3 *,const char *,int ,sqlite3_stmt **,const char **);
typedef int(*sqliteFinalize)(sqlite3_stmt *pStmt);
typedef int(*sqliteStep)(sqlite3_stmt*);
typedef int(*sqliteColumnCount)(sqlite3_stmt *);
typedef int(*sqliteColumnInt)(sqlite3_stmt*, int iCol);
typedef int(*sqliteColumnText)(sqlite3_stmt*, int iCol);
typedef int(*sqlBindBlob)(sqlite3_stmt*, int, const void*, int n, void(*)(void*));

#define  MAX_PATH  260
CSqliteAcess::CSqliteAcess(const std::string  lpszPath)
{
	m_hDLL = NULL;
	m_db = NULL;
	m_stmt = NULL;
	m_bOpenState = false;
	m_lpszLocale = NULL;
	m_lpszLocale =_strdup( setlocale(LC_CTYPE,NULL) ); 
	setlocale( LC_CTYPE,"chs");

	wstring strSysPath;
	::GetSysPath(strSysPath);
	CString strDllPath,strDllPath_Old;
	strDllPath.Format(_T("%s\\sqlite3watch.dll"),strSysPath.c_str());
	strDllPath_Old.Format(_T("%s\\sqlite3.dll"),strSysPath.c_str());
	bool bDllExist = true;
	if(!Project::Tools::FindFileExist(strDllPath.GetString()))
	{
		bDllExist = false;
		if(CopyFile(strDllPath_Old,strDllPath,FALSE))
		{
			bDllExist = true;
		}
	}

	if(m_hDLL == NULL && bDllExist)
	{
		m_hDLL = LoadLibrary(strDllPath);
		if(m_hDLL == NULL)
		{
			FreeLibrary(m_hDLL);
		}
	}
	OpenDataBase(lpszPath.data());
}

CSqliteAcess::~CSqliteAcess(void)
{
	setlocale(LC_CTYPE, m_lpszLocale);
	free(m_lpszLocale);//golding2014-08-12
	CloseDataBase();
}

int  CSqliteAcess::CloseDataBase()
{
	if(m_db)
	{
		if(m_hDLL)
		{
			sqliteClose close = (sqliteClose)GetProcAddress(m_hDLL,"sqlite3_close");
			if(close)
				close(m_db); 
			m_db = NULL;
		}
	}
	return 0;
}

bool CSqliteAcess::GetOpenState()
{
	return m_bOpenState;
}

int  CSqliteAcess::OpenDataBase(const char* pathName)
{
	char * pErrMsg = 0;
	int ret = 0;
	if(m_hDLL)
	{
		if(m_db)
		{
			sqliteClose close = (sqliteClose)GetProcAddress(m_hDLL,"sqlite3_close");
			if(close)
				close(m_db); 
			m_db = NULL;
		}

		char  name[MAX_PATH] = "";
		strcpy(name, pathName);

		sqliteOpen open = (sqliteOpen)GetProcAddress(m_hDLL,"sqlite3_open");
		if(open)
			ret = open(name, &m_db);

		sqliteKey key = (sqliteKey)GetProcAddress(m_hDLL,"sqlite3_key");
		if(key)
			key(m_db,"RNB.beop-2013",strlen("RNB.beop-2013"));/*RNB.beop-2013*/
		if ( ret != 0 )
		{
			exit(1);
		}
		m_bOpenState = true;
	}
	return ret;
}

int CSqliteAcess::SqlExe(const char* sql)
{
	char* error;
	int  result = 0;
	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			result = exec(m_db, sql, NULL, NULL, &error);
	}
	return  result;

}

int	CSqliteAcess::BeginTransaction()
{
	char* error;
	int result = 0;
	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			result = exec(m_db, "BEGIN;", NULL, NULL, &error);
	}
	return result;
}

int CSqliteAcess::CommitTransaction()
{
	char* error;
	int result = 0;
	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			result = exec(m_db, "COMMIT;", NULL, NULL, &error);
	}
	return  result;
}

int CSqliteAcess::SqlQuery(const char* sqlr)
{
	int result = 0;
	if(m_hDLL)
	{
		sqlitePrepare exec = (sqlitePrepare)GetProcAddress(m_hDLL,"sqlite3_prepare");
		if(exec)
			result = exec(m_db, sqlr, strlen(sqlr), &m_stmt, 0);
	}
	return result;
}

int	CSqliteAcess::SqlFinalize()
{
	int rc = 0;
	if(m_hDLL)
	{
		sqliteFinalize exec = (sqliteFinalize)GetProcAddress(m_hDLL,"sqlite3_finalize");
		if(exec)
			rc = exec(m_stmt);
	}
	return rc;
}

int CSqliteAcess::SqlNext()
{
	int rc = 0;
	if(m_hDLL)
	{
		sqliteStep exec = (sqliteStep)GetProcAddress(m_hDLL,"sqlite3_step");
		if(exec)
			rc = exec(m_stmt);
	}
	return rc;
}

int CSqliteAcess::getColumn_Count()
{
	int rc = 0;
	if(m_hDLL)
	{
		sqliteColumnCount exec = (sqliteColumnCount)GetProcAddress(m_hDLL,"sqlite3_column_count");
		if(exec)
			rc = exec(m_stmt);
	}
	return rc;
}

int CSqliteAcess::getColumn_Int(int i)
{
	int rc = 0;
	if(m_hDLL)
	{
		sqliteColumnInt exec = (sqliteColumnInt)GetProcAddress(m_hDLL,"sqlite3_column_int");
		if(exec)
			rc = exec(m_stmt, i);
	}
	return rc;
}

const char* CSqliteAcess::getColumn_Text(int i)
{
	char* rc = "";
	if(m_hDLL)
	{
		sqliteColumnText exec = (sqliteColumnText)GetProcAddress(m_hDLL,"sqlite3_column_text");
		if(exec)
			return reinterpret_cast<const char*>(exec(m_stmt, i));
	}
	return rc;
}

int	CSqliteAcess::SqlStep()
{
	int rc = 0;
	if(m_hDLL)
	{
		sqliteStep exec = (sqliteStep)GetProcAddress(m_hDLL,"sqlite3_step");
		if(exec)
			rc = exec(m_stmt);
	}
	return rc;
}

sqlite3* CSqliteAcess::GetDB() const
{
	if (m_db)
	{
		return m_db;
	}
	return NULL;
}

sqlite3_stmt* CSqliteAcess::GetDataSet() const
{
	if (m_stmt)
	{
		return m_stmt;
	}
	return NULL;
}

char* CSqliteAcess::GetLocale()
{
	if (m_lpszLocale)
	{
		return m_lpszLocale;
	}
	return NULL;
}

int CSqliteAcess::DeleteAllPoint()
{
	int  result = 0;
	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			result = exec(m_db, "delete from list_point", NULL, NULL, NULL);
	}
	return  result;
}

int CSqliteAcess::InsertRecordToOPCPoint(const int nIndex, const int nType, const char* lpszName, const char* lpszSource, const int nProperty, const char* lpszDescription, const char* lpszUnit, const double dHigh, const double dHighHigh, const double dLow, const double dLowLow, const char* lpszParam1,const char* lpszParam2, const char* lpszParam3,const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8, const char* lpszParam9,const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,const char* lpszParam15,const int nParam16,const int nParam17,const int nParam18)
{
	char szSQL[5000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	//int nMax = (int)atof(lpszParam12);
	//int nMin = (int)atof(lpszParam13);
	int nIsVisit = (int)atof(lpszParam14);
	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_point(id, type, name, SourceType, R_W, ch_description, unit, high, highhigh, low, lowlow, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param15, Param16, Param17, Param18)\
									VALUES(%d, %d, '%s', '%s', %d, '%s', '%s', %d, '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d)", nIndex,
									nType, lpszName, lpszSource, nProperty, lpszDescription, lpszUnit, (int)dHigh, lpszParam12, lpszParam13, nIsVisit, lpszParam1, lpszParam2, lpszParam3,lpszParam4,lpszParam5,lpszParam6,lpszParam7,lpszParam8, lpszParam9,lpszParam10,lpszParam11,lpszParam15,nParam16,nParam17,nParam18);
	assert(strlen(szSQL)>0);

	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			rc = exec(m_db, szSQL, NULL, NULL, NULL);
	}
	assert(0 == rc);
	return rc;
}

int CSqliteAcess::RollbackTransaction()
{
	int  result = 0;
	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			result = exec(m_db, "ROLLBACK;", NULL, NULL, NULL);
	}
	return  result;
}

int CSqliteAcess::DeletePointByID(const int nID)
{
	char szSQL[500] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "delete from list_point where id =%d", nID);
	assert(strlen(szSQL)>0);

	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			rc = exec(m_db, szSQL, NULL, NULL, NULL);
	}
	return rc;
}

int CSqliteAcess::DeletePointByName( const char* lpszName )
{
	char szSQL[500] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "delete from list_point where name ='%s'", lpszName);
	assert(strlen(szSQL)>0);

	if(m_hDLL)
	{
		sqliteExec exec = (sqliteExec)GetProcAddress(m_hDLL,"sqlite3_exec");
		if(exec)
			rc = exec(m_db, szSQL, NULL, NULL, NULL);
	}
	return rc;
}

int CSqliteAcess::SqlBindBlob(char *pBlock,int iOrder ,int nSize)
{
	int  result = 0;
	if(m_hDLL)
	{
		sqlBindBlob exec = (sqlBindBlob)GetProcAddress(m_hDLL,"sqlite3_bind_blob");
		if(exec)
			result = exec(m_stmt,iOrder,pBlock,nSize,NULL);
	}
	return  result;
}