
// BEOPWatch.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#define WATCH_VERSION _T("V2.1.6 2017.08.14")

// CBEOPWatchApp:
// See BEOPWatch.cpp for the implementation of this class
//

class CBEOPWatchApp : public CWinApp
{
public:
	CBEOPWatchApp();

// Overrides
public:
	virtual BOOL InitInstance();
	BOOL FindProcessByName_Myexe(CString exefile);		//检查进程是否在运行（本程序）
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CBEOPWatchApp theApp;