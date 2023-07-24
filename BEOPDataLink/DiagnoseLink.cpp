#include "StdAfx.h"
#include "DiagnoseLink.h"
#include "BEOPDataLinkManager.h"

#include <comdef.h>
#include <Wbemidl.h>
# pragma comment(lib, "wbemuuid.lib")

CDiagnoseLink::CDiagnoseLink( CBEOPDataLinkManager* datalinker,Beopdatalink::CCommonDBAccess*	pDBsession)
	: m_pDataLinker(datalinker)
	, m_pDBsession(pDBsession)
	, m_hDiagnoseThread(NULL)
	, m_bExitThread(false)
{
	m_mapDiagnoseValue.clear();
}

CDiagnoseLink::~CDiagnoseLink( void )
{

}

bool CDiagnoseLink::Init()
{
	if(m_hDiagnoseThread == NULL)
		m_hDiagnoseThread = (HANDLE) _beginthreadex(NULL, 0, ThreadDiagnose, this, NORMAL_PRIORITY_CLASS, NULL);
	return true;
}

bool CDiagnoseLink::Exit()
{
	m_bExitThread = true;
	if(m_hDiagnoseThread)
	{
		CloseHandle(m_hDiagnoseThread);
		m_hDiagnoseThread = NULL;
	}
	return true;
}

void CDiagnoseLink::GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist )
{

}

UINT WINAPI CDiagnoseLink::ThreadDiagnose( LPVOID lparam )
{
	CDiagnoseLink* pthis = (CDiagnoseLink*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			pthis->Diagnose();

			int nSleep = 60*5;
			while(!pthis->m_bExitThread)
			{
				nSleep--;
				if(nSleep <= 0)
					break;
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool CDiagnoseLink::Diagnose()
{
	//GetCPUTemp();		//»ñÈ¡ÎÂ¶È
	return true;
}

bool CDiagnoseLink::GetCPUTemp()
{
	try 
	{
		IWbemLocator *pLoc=NULL;
		if(SUCCEEDED(CoCreateInstance(CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator,(LPVOID *)&pLoc)))
		{
			IWbemServices *pSvc=NULL;
			if(SUCCEEDED(pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"),NULL,NULL,0,NULL,0,0,&pSvc)))
			{
				if(SUCCEEDED(CoSetProxyBlanket(pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE)))
				{
					IEnumWbemClassObject* pEnumerator=NULL;
					if(SUCCEEDED(pSvc->ExecQuery(bstr_t("WQL"),bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,NULL,&pEnumerator)))
					{
						IWbemClassObject *pclsObj;
						ULONG uReturn=0;
						while(pEnumerator)
						{
							pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn); 
							if(0==uReturn)
								break;
							VARIANT vtProp;
							VariantInit(&vtProp);
							pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);

							std::ostringstream sqlstream;
							sqlstream << (vtProp.intVal - 2732)/10.0;
							m_mapDiagnoseValue[DIAGNOSE_CPU_TEMP] = sqlstream.str().c_str();
							VariantClear(&vtProp);
							pclsObj->Release();
						}
					}				
				}
				pSvc->Release();
			}
			pLoc->Release();
		}
	}
	catch (_com_error err)
	{
	}
	return true; 
}
