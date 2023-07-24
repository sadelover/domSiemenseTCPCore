// BEOPDTUEngine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BEOPDTUEngine.h"
#include "BEOPDTUEngineWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

CBEOPDTUEngineWrapper* m_pBEOPDTUEngineWrapper = NULL;

bool ctrlhandler(DWORD fdwctrltype)
{
	switch (fdwctrltype)
	{
		// handle the ctrl-c signal.
	case CTRL_C_EVENT:
		printf("ctrl-c event\n");
		break;
		// ctrl-close: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		printf("ctrl-close event\n");
		return m_pBEOPDTUEngineWrapper->Exit();
	case CTRL_BREAK_EVENT:
		printf("ctrl-break event\n");
		break;
	case CTRL_LOGOFF_EVENT:
		printf("ctrl-logoff event\n");
		break;
	case CTRL_SHUTDOWN_EVENT:
		printf("ctrl-shutdown event\n");
		break;
	default:
		break;
	}
	return false;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("ERROR: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlhandler, true))
			{
				m_pBEOPDTUEngineWrapper = new CBEOPDTUEngineWrapper();
				m_pBEOPDTUEngineWrapper->Run();
			}
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("ERROR: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
