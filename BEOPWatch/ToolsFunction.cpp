// ToolsFunction.h implement

#include "stdafx.h"
#include <string>
#include <tchar.h>
#include "ToolsFunction.h"
using namespace std;

#pragma warning(disable:4267 4996 4554)


// Default constructor.
CToolsFunction::CToolsFunction()
: m_pIpAdapterInfo(NULL)
{
	memset(m_tszLogFilePath, 0, MAX_PATH_SIZE);
	memset(m_tszDatasFilePath, 0, MAX_PATH_SIZE);
}

// Default destructor.
CToolsFunction::~CToolsFunction()
{
	if (m_pIpAdapterInfo != NULL)
	{
		delete m_pIpAdapterInfo;
		m_pIpAdapterInfo = NULL;
	}
}

// Ansi exchange to unicode.
void CToolsFunction::Ansi2Unicode(const char* szInput, wchar_t* &wszOutput)
{
	// 预转换，得到所需空间的大小
	int wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, szInput, strlen(szInput), NULL, 0);

	// 分配空间要给'\0'留个空间，MultiByteToWideChar不会给' \0'空间
	wszOutput = new wchar_t[wcsLen + 1];

	// 转换
	::MultiByteToWideChar(CP_ACP, NULL, szInput, strlen(szInput), wszOutput, wcsLen);
	// 最后加上'\0'
	wszOutput[wcsLen] = '\0';
}

// Unicode exchange to ansi.
void CToolsFunction::Unicode2Ansi(const wchar_t* wszInput, char* &szOutput)
{
	/* unicode to ansi */

	// 预转换，得到所需空间的大小，这次用的函数和上面名字相反
	int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, wszInput, wcslen(wszInput), NULL, 0, NULL, NULL);

	// 分配空间要给'\0'留个空间
	szOutput = new char[ansiLen + 1];

	// 转换,unicode版对应的strlen是wcslen
	::WideCharToMultiByte(CP_ACP, NULL, wszInput, wcslen(wszInput), szOutput, ansiLen, NULL, NULL);
	// 最后加上'\0'
	szOutput[ansiLen] = '\0';
}

// Utf8 exchange to Unicode.
void CToolsFunction::Utf82Unicode(const char* szInput, wchar_t* &wszOutput)
{
	// 预转换，得到所需空间的大小
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szInput, strlen(szInput), NULL, 0);

	// 分配空间要给'\0'留个空间，MultiByteToWideChar不会给' \0'空间
	wszOutput = new wchar_t[wcsLen + 1];

	// 转换
	::MultiByteToWideChar(CP_UTF8, NULL, szInput, strlen(szInput), wszOutput, wcsLen);
	// 最后加上'\0'
	wszOutput[wcsLen] = '\0';
}

// Unicode exchange to utf8.
void CToolsFunction::Unicode2Utf8(const wchar_t* wszInput, char* &szOutput)
{
	// 预转换，得到所需空间的大小，这次用的函数和上面名字相反
	int u8Len = ::WideCharToMultiByte(CP_UTF8, NULL, wszInput, wcslen(wszInput), NULL, 0, NULL, NULL);

	// 分配空间要给'\0'留个空间
	// UTF8虽然是Unicode的压缩形式，但也是多字节字符串，所以可以以char的形式保存
	szOutput = new char[u8Len + 1];

	// 转换,unicode版对应的strlen是wcslen
	::WideCharToMultiByte(CP_UTF8, NULL, wszInput, wcslen(wszInput), szOutput, u8Len, NULL, NULL);
	// 最后加上'\0'
	szOutput[u8Len] = '\0';
}

// Ansi exchange to utf8.
void CToolsFunction::Ansi2Utf8(const char* szInput, char* &szOutput)
{
	// Ansi转换utf8就是上面2个的结合，把unicode作为中间量，进行2次转换即可
	wchar_t* wszTempOutput;

	Ansi2Unicode(szInput, wszTempOutput);
	Unicode2Utf8(wszTempOutput, szOutput);
}

// Utf8 exchange to ansi.
void CToolsFunction::Utf82Ansi(const char* szInput, char* &szOutput)
{
	// utf8转换Ansi就是上面2个的结合，把unicode作为中间量，进行2次转换即可
	wchar_t* wszTempOutput;

	Utf82Unicode(szInput, wszTempOutput);
	Unicode2Ansi(wszTempOutput, szOutput);
}

// Set the log file path.
void CToolsFunction::SetLogFilePath(LPCTSTR lpszLogFilePath)
{
	TCHAR tszCurAppPath[MAX_PATH_SIZE] = {0};
	LPTSTR lpszCurAppPath = TEXT("");

	lpszCurAppPath = GetCurretAppPath();
	_tcscpy_s(tszCurAppPath, MAX_PATH_SIZE, lpszCurAppPath);
	_tcscat_s(tszCurAppPath, MAX_PATH_SIZE, lpszLogFilePath);
	_tcscpy_s(m_tszLogFilePath, MAX_PATH_SIZE, tszCurAppPath);
}

// Get the log file path.
LPTSTR CToolsFunction::GetLogFilePath() const
{
	return const_cast<LPTSTR>(m_tszLogFilePath);
}

// Write log file.
void CToolsFunction::AppendLog2File(const char* str)
{
	LPTSTR lpszLogFilePath = GetLogFilePath();

	// Get the current system date.
	SYSTEMTIME systemtime;
	memset(&systemtime, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&systemtime);

	// Formating log information.
	string strLogDataFormat = "";
	char pchLogDataFormat[MAX_PATH] = {0};
	static unsigned int nLogCount = 0;
	sprintf_s(pchLogDataFormat, 
		MAX_PATH, 
		"%d.%d.%d %d:%d:%d (%d): ", 
		 systemtime.wYear, 
		 systemtime.wMonth, 
		 systemtime.wDay, 
		 systemtime.wHour, 
		 systemtime.wMinute, 
		 systemtime.wSecond,
		 ++nLogCount);
	strLogDataFormat = pchLogDataFormat;
	strLogDataFormat += str;

	if (_tcscmp(lpszLogFilePath, TEXT("")) != 0)
	{
		FILE* log;
		char szLogFilePath[MAX_PATH] = {0};
		wsprintfA(szLogFilePath, "%S", lpszLogFilePath);
		// Open file with append.
		fopen_s(&log, szLogFilePath, "a+");
		if (log == NULL)
		{
			OutputDebugStringA("Log file open failed.");
			return ;
		}

		// Write to file.
		fprintf(log, "%s", strLogDataFormat.c_str());
		fclose(log);
	}
} 

// Set the datas file path.
void CToolsFunction::SetDatasFilePath(LPCTSTR lpszDatasFilePath)
{
	_tcscpy(m_tszDatasFilePath, lpszDatasFilePath);
}

// Get the datas file path.
LPTSTR CToolsFunction::GetDatasFilePath() const
{
	return const_cast<LPTSTR>(m_tszDatasFilePath);
}

// Write datas to file.
void CToolsFunction::AppendDatas2File(const char* str)
{
	LPTSTR lpszDatasFilePath = GetDatasFilePath();

	if (_tcscmp(lpszDatasFilePath, TEXT("")) != 0)
	{
		FILE* log;
		char szDatasFilePath[MAX_PATH] = {0};
		wsprintfA(szDatasFilePath, "%S", lpszDatasFilePath);
		// Open file with append.
		fopen_s(&log, szDatasFilePath, "a+");
		if (log == NULL)
		{
			OutputDebugStringA("Log file open failed.");
			return ;
		}

		// Write to file.
		fprintf(log, "%s\r\n", str);
		fclose(log);
	}
}

// Converting a WChar string to a Ansi string.
string CToolsFunction::WChar2Ansi(LPCWSTR pwszSrc)
{
	// Get the wide char length.
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
	if (nLen <= 0)
	{
		return std::string("");
	}

	char* pszDst = new char[nLen];
	if (NULL == pszDst) 
	{
		return std::string("");
	}

	// Change the wide charmulti byte to multi byte.
	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;

	std::string strTemp(pszDst);

	delete [] pszDst;
	return strTemp;
}

// Converting a Ansi string to WChar string.
wstring CToolsFunction::Ansi2WChar(LPCSTR pszSrc, unsigned int nLen)
{
	// Get the multi byte length.
	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
	if (nSize <= 0) 
	{
		return std::wstring(L"");
	}

	WCHAR *pwszDst = new WCHAR[nSize + 1];
	if (NULL == pwszDst) 
	{
		return std::wstring(L"");
	}

	// Change the multi byte to wide char.
	MultiByteToWideChar(CP_ACP, 0,(LPCSTR)pszSrc, nLen, pwszDst, nSize);
	pwszDst[nSize] = 0;

	// Skip Oxfeff.
	if (pwszDst[0] == 0xFEFF)
	{
		for(int i = 0; i < nSize; i++)
		{
			pwszDst[i] = pwszDst[i + 1];
		}
	}

	wstring wcharString(pwszDst);

	delete pwszDst;
	return wcharString;
}

// Get the config information.
LPTSTR CToolsFunction::GetProfileValue(LPCTSTR szSection, LPCTSTR szName, LPCTSTR szFileName, LPCTSTR szDefualt /*= TEXT("")*/)
{
	const int len_buffer = 1024;
	TCHAR* buffer = TEXT("");

	LPTSTR lpszFileFullPath = GetCurretAppPath();
	_tcscat(lpszFileFullPath, szFileName);
	TCHAR tszFileFullPath[MAX_PATH] = {0}; 
	_tcscpy_s(tszFileFullPath, MAX_PATH, lpszFileFullPath);
	GetProfileValueFromIniFile(szSection, szName, tszFileFullPath, &buffer, len_buffer, szDefualt);

	return (LPTSTR)buffer;
}

// Get the config information from a ini file.
int CToolsFunction::GetProfileValueFromIniFile(LPCTSTR szSection, LPCTSTR szName, LPCTSTR szFileName, TCHAR** pBuffer, unsigned int nBufferLen, LPCTSTR szDefualt)
{
	if((NULL == pBuffer) || (0 == nBufferLen)) return -1;

	TCHAR buf[128 * 1024] = {0};

	GetPrivateProfileString(szSection, szName, szDefualt, buf, sizeof(buf), szFileName);
	if (nBufferLen < _tcslen(buf) + 1)
	{
		return 0;
	} 
	*pBuffer = buf;

	return _tcslen(*pBuffer);
}

// Set the config information.
void CToolsFunction::SetProfileValue(LPCTSTR szSection, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szFileName) 
{
	if (_tcscmp(szValue, TEXT("")) == 0)
	{
		return ;
	}

	LPTSTR lpszFileFullPath = GetCurretAppPath();
	_tcscat(lpszFileFullPath, szFileName);
	TCHAR tszFileFullPath[MAX_PATH] = {0}; 
	_tcscpy_s(tszFileFullPath, MAX_PATH, lpszFileFullPath);
	SetProfileValueToIniFile(szSection, szName, szValue, lpszFileFullPath);
}

// Set the config information to a ini file.
BOOL CToolsFunction::SetProfileValueToIniFile(LPCTSTR szSection, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szFileName)
{
	BOOL bRet = TRUE;
	bRet = WritePrivateProfileString(szSection, szName, szValue, szFileName);
	if (!bRet)
	{
		DWORD dwAtt = 0;
		// Get the file attributes.
		dwAtt = ::GetFileAttributes(szFileName);
		if (dwAtt & FILE_ATTRIBUTE_READONLY == FILE_ATTRIBUTE_READONLY)
		{
			dwAtt &= (~FILE_ATTRIBUTE_READONLY);
			// Set the file attributes.
			::SetFileAttributes(szFileName, dwAtt);
		}

		dwAtt = 0;
		// Get the file attributes.
		dwAtt = ::GetFileAttributes(szFileName);
		if (dwAtt & FILE_ATTRIBUTE_READONLY == FILE_ATTRIBUTE_READONLY)
		{
			dwAtt &= (~FILE_ATTRIBUTE_READONLY);
			// Set the file attributes.
			::SetFileAttributes(szFileName, dwAtt);
		}

		// Write the config information to the ini file.
		bRet = WritePrivateProfileString(szSection, szName, szValue, szFileName);
	}

	return bRet != 0;
}

// Get the current program path.
LPTSTR CToolsFunction::GetCurretAppPath()
{
	LPTSTR strAppPath = TEXT("");
	if (_tcslen(strAppPath) < 1)
	{
		TCHAR szAppBuffer[MAX_PATH] = {0};
		if (GetModuleFileName(NULL, szAppBuffer, sizeof(szAppBuffer)))
		{
			TCHAR* pLastQu = _tcsrchr(szAppBuffer, TEXT('\\'));
			if(pLastQu)
			{
				*(pLastQu + 1) = TEXT('\0');
				strAppPath = szAppBuffer;
			}
		}
	}

	return strAppPath;
}

// Is the assign file exist?
BOOL CToolsFunction::IsFileExist(LPCTSTR lpszFilePath)
{
	CFile afile;
	BOOL ret = afile.Open(lpszFilePath, CFile::modeRead | CFile::shareDenyNone);
	if (ret == TRUE)
	{
		afile.Close();
	}

	return ret;
}

// Unrar the compression files.
void CToolsFunction::UnRarFiles()
{
	// Get the current application path.
	TCHAR tszCurrentAppPath[MAX_PATH] = {0};
	_tcscpy_s(tszCurrentAppPath, MAX_PATH, GetCurretAppPath());
	if (_tcscmp(tszCurrentAppPath, _T("")) != 0)
	{
		// Get the update files directory.
		TCHAR tszUpdateFilesDir[MAX_PATH] = {0};
		_tcscpy_s(tszUpdateFilesDir, tszCurrentAppPath);
		_tcscat(tszUpdateFilesDir, UPDATE_FILES_DIR);

		// Get the Rar.exe path.
		TCHAR tszUpdateFilesRar[MAX_PATH] = {0};
		_tcscpy_s(tszUpdateFilesRar, tszCurrentAppPath);
		_tcscat(tszUpdateFilesRar, _T("Rar.exe"));

		// Get the update files package path.
		TCHAR tszUpdateFilesPackage[MAX_PATH] = {0};
		_tcscpy_s(tszUpdateFilesPackage, tszCurrentAppPath);
		_tcscat(tszUpdateFilesPackage, UPDATE_FILES_DIR);
		_tcscat(tszUpdateFilesPackage, _T("\\"));
		_tcscat(tszUpdateFilesPackage, UPDATE_FILES_PACKAGE);

		///////////////////////////////////////
		// The decompression command sush as:
		// x:   用绝对路径解压文件
		// -c-: 禁用注释显示
		// -o+: 设置覆盖模式
		// -r:  递归子目录
		// -y:  假设对全部询问都回答是
		//////////////////////////////////////
		TCHAR tszFormat[MAX_PATH] = {0};
		_stprintf(tszFormat, _T("x -c- -o+ -r -y %s"), tszUpdateFilesPackage);
		// Shell the Rar.exe program to execute the decompression.
		::ShellExecute(NULL, _T("open"), tszUpdateFilesRar, tszFormat, tszUpdateFilesDir, SW_HIDE);
	}
}

// Delete the directory.
void CToolsFunction::DeleteDirectory(LPCSTR lpszDeleteDirPath)
{
	if (strcmp(lpszDeleteDirPath, "") != 0)
	{
		_rmdir(lpszDeleteDirPath);
	}
}

// String split.
void CToolsFunction::Split(vector<string>& des, const string& src, string& delim)
{
	size_t nLast = 0;
	size_t nIndex = src.find_first_of(delim, nLast);     // get the first element

	while (nIndex != string::npos)
	{
		string subStr = src.substr(nLast, nIndex - nLast);
		des.push_back(subStr);
		nLast = nIndex + 1;
		nIndex = src.find_first_of(delim, nLast);
	}

	if (nIndex - nLast > 0)
	{
		des.push_back(src.substr(nLast, nIndex - nLast));
	}
}

// Get local machine all ethernet information.
IP_ADAPTER_INFO*& CToolsFunction::GetAllLocalMachineEthInfo()
{
	// Struct PIP_ADAPTER_INFO use for save localhost ethernet information.
//	PIP_ADAPTER_INFO pIpAdapterInfo = NULL;
	m_pIpAdapterInfo = new IP_ADAPTER_INFO();
	// Get the PIP_ADAPTER_INFO structure size for GetA	daptersInfo api parameter.
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);

	// Call GetAdaptersInfo(), fill with parameter pIpAdapterInfo;
	// parameter stSize is a input parameter, also a output parameter.
	int nRel = GetAdaptersInfo(m_pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		// If the function return value ERROR_BUFFER_OVERFLOW,
		// thus identify GetAdaptersInfo() allocater space no enough, meanwhile output the parameter stSize, to indicate needed space;
		// that is why parameter stSize is a input parameter, also a output parameter.

		// Free the memory space of last allocate
		delete m_pIpAdapterInfo;
		m_pIpAdapterInfo = NULL;

		// Re-apply memory space for save all ethernet information.
		m_pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		// Call GetAdaptersInfo() again, fill with parameter pIpAdapterInfo.
		nRel = GetAdaptersInfo(m_pIpAdapterInfo, &stSize);    
	}

	return m_pIpAdapterInfo;
}

// Match ip address.
bool CToolsFunction::MatchIpAddress(IP_ADAPTER_INFO* pIpAdapterInfo, const string& matchedIpAddr) const
{
	// Loop for multi-ethernet card.
	while (pIpAdapterInfo)
	{
		// Loop for multi-ip address in a ethernet card.
		IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
		do 
		{
			if (strcmp(pIpAddrString->IpAddress.String, matchedIpAddr.c_str()) != 0)
			{
				pIpAddrString = pIpAddrString->Next;
			}
			else
			{
// 				if (pIpAdapterInfo)
// 				{
// 					delete pIpAdapterInfo;
// 					pIpAdapterInfo = NULL;
// 				}

				return true;
			}

		} while (pIpAddrString);

		pIpAdapterInfo = pIpAdapterInfo->Next;
	}

// 	if (pIpAdapterInfo)
// 	{
// 		delete pIpAdapterInfo;
// 		pIpAdapterInfo = NULL;
// 	}

	return false;
}

// The only process check.
bool CToolsFunction::CheckOnlyProcess(LPCTSTR OnlySignal)
{
	HANDLE hSem = CreateSemaphore(NULL, 1, 1, OnlySignal);

	if (hSem)  // 信号对象创建成功
	{
		// 信号对象已经存在，则程序已有一个实例在运行。
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{                   
			// 关闭信号量句柄。
			CloseHandle(hSem);
			// 获取桌面窗口的一个子窗口。
			HWND hWndPrev = ::GetWindow(::GetDesktopWindow(), GW_CHILD);    

			while(::IsWindow(hWndPrev))
			{
				// 判断窗口是否有我们预先设置的标记，如有，则是我们寻找的窗口，并将它激活。
				if (::GetProp(hWndPrev, OnlySignal))    
				{
					// 如果主窗口已最小化，则恢复其大小。
					if (::IsIconic(hWndPrev))
					{
						::ShowWindow(hWndPrev, SW_RESTORE);
					}

					// 将应用程序的主窗口激活。
					::SetForegroundWindow(hWndPrev);

					// 退出实例。
					return false;
				}

				// 继续寻找下一个窗口。
				hWndPrev = ::GetWindow(hWndPrev, GW_HWNDNEXT); 
			}

			TRACE("已有一个实例在运行，但找不到它的主窗口！ \r\n");
			return  false;
		}
	}
	else
	{
		TRACE("创建信号对象失败，程序退出！ \r\n");
		return false;
	}

	return true;
}

// Find the special process in the system process bar.
bool CToolsFunction::FoundSpecialProcess(LPCTSTR lpszProcessName) const
{
	if (_tcscmp(lpszProcessName, TEXT("")) == 0)
	{
		return false;
	}

	HANDLE hProcess = NULL;
	PROCESSENTRY32* pProcessInfo = NULL;
	bool bResult = false;

	pProcessInfo = new PROCESSENTRY32();   
	pProcessInfo->dwSize = sizeof(PROCESSENTRY32); 

	// Create the snap shot from the process bar.
	hProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);     
	if (::Process32First(hProcess, pProcessInfo))   
	{   
		// Process within a loop.
		while(::Process32Next(hProcess, pProcessInfo) != FALSE)   
		{   
			TCHAR tszExeFile[MAX_PATH] = {0};    
			_tcscpy_s(tszExeFile, MAX_PATH, pProcessInfo->szExeFile); 
			if (_tcscmp(tszExeFile, TEXT("Rar.exe")) == 0)
			{  
				bResult = true; 
				break;
			}  
			else 
			{
				bResult = false;
			}
		}   
	}

	if (pProcessInfo)   
	{   
		delete pProcessInfo;
		pProcessInfo = NULL;
	} 
	CloseHandle(hProcess);
	hProcess = NULL;

	return bResult;
}

// Write value to register.
bool CToolsFunction::RegWriteValue(HKEY hKey, const string& SubKeyPath, const string& SubKey, const string& SubKeyValue /*= ""*/, DWORD SubKeyType /*= REG_SZ*/)
{
	HKEY hRegKey = NULL;
	DWORD dwDisposition;
	// Create the key or Open the key in register when the key is not exists.
	if (RegCreateKeyExA(hKey, SubKeyPath.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRegKey, &dwDisposition) == ERROR_SUCCESS)
	{
		// Set the key value of the subkey in register.
		if(RegSetValueExA(hRegKey, SubKey.c_str(), 0, SubKeyType, (BYTE*)(SubKeyValue.c_str()), SubKeyValue.length()) != ERROR_SUCCESS)
		{
			TRACE("RegSetValueEx return error: %d", GetLastError());

			// Close the register.
			RegCloseKey(hRegKey);
			hRegKey = NULL;
			return false;
		}

		// Close the register.
		RegCloseKey(hRegKey);
		hRegKey = NULL;
		return true;
	}

	TRACE("RegWriteValue return error: %d", GetLastError());

	// Close the register.
	RegCloseKey(hRegKey);
	hRegKey = NULL;
	return false;
}

// Read value from register.
bool CToolsFunction::RegReadValue(HKEY hKey, const string& SubKeyPath, const string& SubKey, LPBYTE lpSubKeyValue, DWORD SubKeyType /*= REG_SZ*/)
{
	HKEY hRegKey = NULL;
	BYTE subKeyValue[MAX_PATH] = {0};

	// Open the key in register.
	if (RegOpenKeyExA(hKey, SubKeyPath.c_str(), 0, KEY_READ, &hRegKey) == ERROR_SUCCESS)
	{
		// Get the key value of the subkey in register.
		DWORD dwType = REG_SZ;
		DWORD cbData = MAX_PATH;
		if(RegQueryValueExA(hRegKey, SubKey.c_str(), 0, &dwType, subKeyValue, &cbData) != ERROR_SUCCESS)
		{
			TRACE("RegGetValue return error: %d", GetLastError());

			// Close the register.
			RegCloseKey(hRegKey);
			hRegKey = NULL;
			return false;
		}

		memcpy(lpSubKeyValue, subKeyValue, cbData);

		// Close the register.
		RegCloseKey(hRegKey);
		hRegKey = NULL;
		return true;
	}

	TRACE("RegReadValue return error: %d", GetLastError());

	// Close the register.
	RegCloseKey(hRegKey);
	hRegKey = NULL;
	return false;
}
