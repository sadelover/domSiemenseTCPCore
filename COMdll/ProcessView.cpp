#include "stdafx.h"
#include "ProcessView.h"
#include <shellapi.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

CProcessView::CProcessView(void)
{
	
}

CProcessView::~CProcessView(void)
{
}


BOOL CProcessView::FindProcessByName_Myexe(CString exefile)
{
	int nExeNum = 0;
	CString strTemp = _T("");

	if (exefile.IsEmpty())
	{
		return FALSE;
	}
	HANDLE handle =CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	PROCESSENTRY32 pInfo;
	memset(&pInfo, 0x00, sizeof(PROCESSENTRY32));
	pInfo.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(handle, &pInfo))
	{
		while (Process32Next(handle, &pInfo) != FALSE)
		{
			strTemp = pInfo.szExeFile;
			if (0 == strTemp.CompareNoCase(exefile))
			{
				if (++nExeNum > 1)   // there are more than one instance running.
				{
					CloseHandle(handle);
					return TRUE;
				}					
			}
		}
	}

	//delete pInfo;
	CloseHandle(handle);
	return FALSE;
}

BOOL CProcessView::FindProcessByName_other(CString exefile)
{
	int nExeNum = 0;
	CString strTemp = _T("");

	if (exefile.IsEmpty())
	{
		return FALSE;
	}
	HANDLE handle =CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	PROCESSENTRY32 pInfo;
	memset(&pInfo, 0x00, sizeof(PROCESSENTRY32));
	pInfo.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(handle, &pInfo))
	{
		while (Process32Next(handle, &pInfo) != FALSE)
		{
			strTemp = pInfo.szExeFile;
			if (0 == strTemp.CompareNoCase(exefile))
			{
				if (++nExeNum >= 1)   // there are more than one instance running.
				{
					CloseHandle(handle);
					return TRUE;
				}					
			}
		}
	}

	//delete pInfo;
	CloseHandle(handle);
	return FALSE;
}

BOOL CProcessView::OpenApp(CString exefile)
{
	STARTUPINFO						si		={sizeof(si)};
	PROCESS_INFORMATION		pi;
	//TCHAR		szCmd[]					= TEXT("D:\\Workspace\\MDO\\Code\\Debug\\MDODesigner.exe");
	LPWSTR test	= exefile.GetBuffer();
	BOOL ret = CreateProcess(NULL, test, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	exefile.ReleaseBuffer();

	//释放句柄，否则新进程将无法退出
	if(ret)
	{
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}
	return ret;
}

float CProcessView::GetProessCPU( CString exefile )
{
	if (FindProcessByName_other(exefile))
	{
			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (INVALID_HANDLE_VALUE == hSnapshot) {
				return 0;
			}
			PROCESSENTRY32 pe = { sizeof(pe) };
			BOOL fOk;
			for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe)) {
				if (!_tcscmp(pe.szExeFile, exefile)) {
					CloseHandle(hSnapshot);

			//进程句柄
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pe.th32ProcessID);
			if (NULL != hProcess)
			{
				//内存情况
				PROCESS_MEMORY_COUNTERS pmc;
				if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
				{
					float fCurUse = float(pmc.PagefileUsage / (1024.0*1024.0));
					float fMaxUse = float( pmc.PeakWorkingSetSize / (1024.0*1024.0));
					TRACE("[info] Main memory usage，Currently used memory：%.4fM，Max used：%.4fM", fCurUse, fMaxUse);
					CString str;
					str.Format(_T("%s Currently used memory：%.4fM，Max used：%.4fM"),exefile,fCurUse, fMaxUse);

					return fCurUse;
				}

				//关闭句柄
				CloseHandle(hProcess);
			
		}
				}
	}
	}
	return 0;
}

void CProcessView::LogData( const char* sql )
{
	
}

void CProcessView::LogData( const string& sql )
{
	LogData(sql.c_str());
}

BOOL CProcessView::CloseApp( CString exefile )
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		return FALSE;
	}
	PROCESSENTRY32 pe = { sizeof(pe) };
	BOOL fOk;
	for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe)) {
		if (!_tcscmp(pe.szExeFile, exefile)) {
			CloseHandle(hSnapshot);

			//进程句柄
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
			if (NULL != hProcess)
			{
				if(::TerminateProcess(hProcess,0))
				{
					CloseHandle(hProcess);
				}
			}
		}
	}
	return TRUE;
}