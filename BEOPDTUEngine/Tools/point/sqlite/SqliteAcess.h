
#ifndef _SQLITE_ACESS_H_
#define _SQLITE_ACESS_H_
extern "C"
{
#include "sqlite3.h"
};

#include "Tools/DllDefine.h"
#include "vector"
#include "string"
using namespace std;
/*******************************************************
sqlite文件标题的操作，写入时要GBToUTF8，读取时UTF8ToGB
不过写入表中的汉字不需要转
********************************************************/

class DLLEXPORT CSqliteAcess
{
public:
	CSqliteAcess(const std::string lpszPath);
	~CSqliteAcess(void);

public:
	sqlite3*						GetDB() const;
	sqlite3_stmt*                   GetDataSet() const;
	int								SqlExe(const char* sqlr);
	bool							GetOpenState();
	int								OpenDataBase(const char* pathName);
	int								CloseDataBase();

	int								BeginTransaction();	
	int								CommitTransaction();
	int								RollbackTransaction();
	int								SqlQuery(const char* sqlr);
	int								SqlNext();
	int								SqlFinalize();
	int								getColumn_Count();
	int								getColumn_Int(const int i);
	const char*						getColumn_Text(const int i);
	char*							GetLocale();

	int								SqlStep();

	//清空list_point
	int								DeleteAllPoint();
	int								DeletePointByID(const int nID);
	int								DeletePointByName( const char* lpszName);
	
	//插入opc数据到list_point
	int								InsertRecordToOPCPoint(const int nIndex, const int nType, const char* lpszName, const char* lpszSource, const int nProperty, const char* lpszDescription, const char* lpszUnit,
		const double dHigh, const double dHighHigh, const double dLow, const double dLowLow, const char* lpszParam1,const char* lpszParam2,
		const char* lpszParam3,const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8,
		const char* lpszParam9,const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,
		const char* lpszParam15,const int nParam16,const int nParam17,const int nParam18);

	int								SqlBindBlob(char *pBlock,int iOrder ,int nSize);

public:
	sqlite3*						m_db;
	sqlite3_stmt*					m_stmt;
	bool							m_bOpenState;
	char*							m_lpszLocale;
	char							m_lpszDBPath[MAX_PATH];
	HINSTANCE						m_hDLL;
};
#endif