
// BEOPClinetSample.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "BEOPClinetSample.h"
#include "BEOPClinetSampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBEOPClinetSampleApp

BEGIN_MESSAGE_MAP(CBEOPClinetSampleApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CBEOPClinetSampleApp construction

CBEOPClinetSampleApp::CBEOPClinetSampleApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CBEOPClinetSampleApp object

CBEOPClinetSampleApp theApp;


// CBEOPClinetSampleApp initialization

BOOL CBEOPClinetSampleApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	if(FindProcessByName_Myexe(_T("BEOPClinetSample.exe")))
	{
		//AfxMessageBox(_T("BEOPDog.exe已经运行，请关掉当前程序再试！"));
		return FALSE;
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CBEOPClinetSampleDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CBEOPClinetSampleApp::FindProcessByName_Myexe( CString exefile )
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

