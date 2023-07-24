#pragma once
#include <string>
#include <TlHelp32.h>
using namespace std;

class CProcessView
{
public:
	CProcessView(void);
	~CProcessView(void);

public:
	BOOL FindProcessByName_Myexe(CString exefile);		//检查进程是否在运行（本程序）
	BOOL FindProcessByName_other(CString exefile);			//检查进程是否在运行（不是本程序）
	BOOL OpenApp(CString exefile);
	BOOL CloseApp(CString exefile);
	float	 GetProessCPU(CString exefile);
	void LogData(const char* sql);
	void LogData(const string& sql);

};
