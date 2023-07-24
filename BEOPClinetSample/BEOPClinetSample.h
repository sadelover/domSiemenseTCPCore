
// BEOPClinetSample.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CBEOPClinetSampleApp:
// See BEOPClinetSample.cpp for the implementation of this class
//

class CBEOPClinetSampleApp : public CWinApp
{
public:
	CBEOPClinetSampleApp();

// Overrides
public:
	virtual BOOL InitInstance();
	BOOL FindProcessByName_Myexe(CString exefile);		//检查进程是否在运行（本程序）
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CBEOPClinetSampleApp theApp;