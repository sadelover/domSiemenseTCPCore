#ifndef TOOLS_FUNCTION_H
#define TOOLS_FUNCTION_H



#include <Iphlpapi.h>     // ethernet
#pragma comment(lib, "Iphlpapi.lib")
#include <Tlhelp32.h>     // process bar
#include <direct.h>       // _rmdir()
#include <string>
#include <vector>
using namespace std;

#ifdef _UNICODE
  typedef wstring _tstring;
#else
  typedef string _tstring;
#endif  // _UNICODE

#define MAX_PATH_SIZE         512

#define FILE_CONFIG_DATA      _T("config\\config.dat")  			// config information file
#define UPDATE_FILES_DIR      _T("UpdateFiles")    					// update files directory
#define UPDATE_FILES_PACKAGE  _T("UpdateFiles.rar")     			// update files compression package
#define REG_VALUE_PATH        "Software\\BEOPTechApp\\UpdateFiles"  // the path of register value


class CToolsFunction
{ 
public:
	CToolsFunction();                         // default constructor
	virtual ~CToolsFunction();                // default destructor

public:

	//--------------------------------------------------------
	// character operation 
	//--------------------------------------------------------

	void Ansi2Unicode(__in const char* szInput, __out wchar_t* &wszOutput);     	// ansi exchange to unicode
	void Unicode2Ansi(__in const wchar_t* wszInput, __out char* &szOutput);     	// unicode exchange to ansi
	void Utf82Unicode(__in const char* szInput, __out wchar_t* &wszOutput);     	// utf8 exchange to Unicode
	void Unicode2Utf8(__in const wchar_t* wszInput, __out char* &szOutput);     	// unicode exchange to utf8
	void Ansi2Utf8(__in const char* szInput, __out char* &szOutput);            	// ansi exchange to utf8
	void Utf82Ansi(__in const char* wszInput, __out char* &szOutput);           	// utf8 exchange to ansi

	string WChar2Ansi(__in LPCWSTR pwszSrc);                            // converting a WChar string to a Ansi string
	wstring Ansi2WChar(__in LPCSTR pszSrc, __in unsigned int nLen);     // converting a Ansi string to WChar string

	void Split(__out vector<string>& des, __in const string& src, __in string& delim);  // string split


	//--------------------------------------------------------
	// file operation 
	//--------------------------------------------------------

	void SetLogFilePath(__in LPCTSTR lpszLogFilePath);                  // set the log file path
	LPTSTR GetLogFilePath() const;                                      // get the log file path
	virtual void AppendLog2File(__in const char* str);                  // write log file
	void SetDatasFilePath(__in LPCTSTR lpszDatasFilePath);              // set the datas file path
	LPTSTR GetDatasFilePath() const;                                    // get the datas file path
	virtual void AppendDatas2File(__in const char* str);                // write datas to file

	LPTSTR GetProfileValue(__in LPCTSTR szSection, __in LPCTSTR szName, __in LPCTSTR szFileName, __in LPCTSTR szDefualt = TEXT(""));  // get the config information
	int GetProfileValueFromIniFile(__in LPCTSTR szSection, __in LPCTSTR szName, __in LPCTSTR szFileName, __out TCHAR** pBuffer, __in unsigned int nBufferLen, __in LPCTSTR szDefualt);  // get the config information from a ini file
	void SetProfileValue(LPCTSTR szSection, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szFileName);                 // set the config information
	BOOL SetProfileValueToIniFile(LPCTSTR szSection, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szFileName);        // set the config information to a ini file      
	LPTSTR GetCurretAppPath();                                       // get the current program path

	BOOL IsFileExist(__in LPCTSTR lpszFilePath);                     // is the assign file exist?
	void UnRarFiles();                                               // unrar the compression files
	void DeleteDirectory(__in LPCSTR lpszDeleteDirPath);             // delete the directory


	//--------------------------------------------------------
	// network operation 
	//--------------------------------------------------------

	IP_ADAPTER_INFO*& GetAllLocalMachineEthInfo();  // get local machine all ethernet information     
	bool MatchIpAddress(__in IP_ADAPTER_INFO* pIpAdapterInfo, __in const string& matchedIpAddr) const;  // match ip address


	//--------------------------------------------------------
	// program operation
	//--------------------------------------------------------

	bool CheckOnlyProcess(__in LPCTSTR OnlySignal);                  // the only process check
	bool FoundSpecialProcess(LPCTSTR lpszProcessName) const;         // find the special process in the system process bar


	//--------------------------------------------------------
	// Register operation
	//--------------------------------------------------------

	bool RegWriteValue(HKEY hKey, const string& SubKeyPath, const string& SubKey, const string& SubKeyValue = "", DWORD SubKeyType = REG_SZ);
	bool RegReadValue(HKEY hKey, const string& SubKeyPath, const string& SubKey, LPBYTE lpSubKeyValue, DWORD SubKeyType = REG_SZ);

private:
	TCHAR m_tszLogFilePath[MAX_PATH_SIZE];    // log file path string
	TCHAR m_tszDatasFilePath[MAX_PATH_SIZE];  // datas file path string

	PIP_ADAPTER_INFO m_pIpAdapterInfo;
};

#endif  // TOOLS_FUNCTION_H