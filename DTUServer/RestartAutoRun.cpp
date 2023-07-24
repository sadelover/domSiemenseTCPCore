#include "StdAfx.h"
#include "RestartAutoRun.h"

const WCHAR* RESTARTAUTORUN_KEYPATH = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

bool CRestartAutoRun::SetAutoRun(const CString& exepath)
{
	HKEY   hRegKey = NULL;
	bool   bResult = false;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, RESTARTAUTORUN_KEYPATH, 0, KEY_ALL_ACCESS , &hRegKey) != ERROR_SUCCESS)
		bResult=false;
	else
	{
		CString keyname;
		_wsplitpath(exepath.GetString(), NULL, NULL, keyname.GetBuffer(MAX_PATH+1), NULL);
		keyname.ReleaseBuffer();
		if(::RegSetValueEx(hRegKey,
			keyname.GetString(),
			0,
			REG_SZ,
			(const BYTE *)exepath.GetString(),
			exepath.GetLength()*sizeof(TCHAR)) != ERROR_SUCCESS)
			bResult = false;
		else
			bResult = true;
		RegCloseKey(hRegKey);
	}

	return   bResult;
}

bool CRestartAutoRun::RemoveAutoRun(const CString& exename)
{
	HKEY key_temp = NULL;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, RESTARTAUTORUN_KEYPATH, &key_temp) != ERROR_SUCCESS){
		return false;
	}else{
		 bool bresult = (RegDeleteValue(key_temp, exename.GetString()) != ERROR_SUCCESS);
		 RegCloseKey(key_temp);
		 return bresult;
	}
}
