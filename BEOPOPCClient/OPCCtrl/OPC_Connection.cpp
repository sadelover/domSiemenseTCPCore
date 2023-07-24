#pragma once

#include "StdAfx.h"
#include "strsafe.h"
#include "OPC_Connection.h"
#include "Tools/CustomTools/CustomTools.h"

const GUID CLSID_OPCServerList={0x13486D51, 0x4821, 0x11D2, { 0xA4, 0x94, 0x3C, 0xB3, 0x06, 0xC1, 0x00, 0x00 } };

COPCConnection::COPCConnection()
: m_strFilterLeaf( L"*" )
, m_bBrowseFlat( TRUE )
, m_isconnected(false)
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
	m_strserverprogid		= L"";

	m_strserverhost = L"localhost";

	m_pServer		= NULL;
	m_pGroup		= NULL;
}

COPCConnection::~COPCConnection(void)
{
    CoUninitialize();
}

void COPCConnection::GetValue(VARIANT &vtVal	// [in]
							   , TCHAR *pBuffer	// [out]
							   , int nBufLen)	// [in]
{
	ASSERT (pBuffer != NULL);
	ASSERT (nBufLen > 0);

	// Declare a CString to help construct result string:
	CString strVal = _T("");

	// Format string according to data type:
	switch (vtVal.vt)
	{
	case VT_BOOL:
		strVal.Format (_T("%d"), vtVal.boolVal);
		break;

	case VT_UI1:
		strVal.Format (_T("%u"), vtVal.bVal);
		break;

	case VT_I1:
		strVal.Format (_T("%d"), vtVal.cVal);
		break;

	case VT_UI2:
		strVal.Format (_T("%u"), vtVal.uiVal);
		break;

	case VT_I2:
		strVal.Format (_T("%d"), vtVal.iVal);
		break;

	case VT_UI4:
		strVal.Format (_T("%u"), vtVal.ulVal);
		break;

	case VT_I4:
		strVal.Format (_T("%d"), vtVal.lVal);
		break;

	case VT_R4:
		strVal.Format (_T("%G"), vtVal.fltVal);
		break;

	case VT_R8:
		strVal.Format (_T("%G"), vtVal.dblVal);
		break;

	case VT_BSTR:
		strVal = vtVal.bstrVal;
		break;

	case VT_DATE:
		{
			bool bSuccess = false;

			// Cariant time to system time (UTC):
			SYSTEMTIME systime;
			if ( VariantTimeToSystemTime ( vtVal.dblVal, &systime ) )
			{
				// Get time zone information:
				TIME_ZONE_INFORMATION tTZI;
				if ( GetTimeZoneInformation (&tTZI) != TIME_ZONE_ID_INVALID )
				{
					// Localize system time:
					SYSTEMTIME systimelocal;
					if ( SystemTimeToTzSpecificLocalTime( &tTZI, &systime, &systimelocal ) )
					{
						strVal.Format (_T("%02d:%02d:%02d"), 
							systimelocal.wHour, systimelocal.wMinute, systimelocal.wSecond);			

						bSuccess = true;
					}
				}
			}
			if (!bSuccess)
				strVal = _T("???");
		}
		break;

	case VT_UI1	| VT_ARRAY:
	case VT_I1	| VT_ARRAY:
	case VT_UI2	| VT_ARRAY:
	case VT_I2	| VT_ARRAY:
	case VT_UI4	| VT_ARRAY:
	case VT_I4	| VT_ARRAY:
	case VT_R4	| VT_ARRAY:
	case VT_R8	| VT_ARRAY:
		{
			CSafeArray *pSafeArr = (CSafeArray *) vtVal.parray;
			DWORD dwCols = pSafeArr->GetNumCols ();
			DWORD dwSize = pSafeArr->GetByteLength ();
			ULONG cbElements = pSafeArr->cbElements;
			LPBYTE lpByte = (LPBYTE)pSafeArr->pvData;
			DWORD dwCol = 0;

			// Start row delimiter:
			strVal = L"[ ";

			// Cycle through the elements:
			for (DWORD i=0; i<dwSize; i+=cbElements, lpByte+=cbElements)
			{
				TCHAR szNum[32] = {0};

				// Format element according to data size:
				if (cbElements == 1)
				{
					if ( vtVal.vt == (VT_UI1 | VT_ARRAY) )
						StringCchPrintf( szNum, 32, _T("%u"), *lpByte);
					else
						StringCchPrintf( szNum, 32, _T("%d"), *(char *)lpByte);
				}
				else if (cbElements == 2)
				{
					if ( vtVal.vt ==	(VT_UI2 | VT_ARRAY) )
						StringCchPrintf ( szNum, 32, _T("%u"), *(WORD *)lpByte);
					else 
						StringCchPrintf (szNum, 32, _T("%d"), *(short *)lpByte);
				}
				else if (cbElements == 4)
				{
					if (vtVal.vt ==	(VT_R4	| VT_ARRAY))
						StringCchPrintf (szNum, 32, _T("%G"), *(float *)lpByte);
					else if (vtVal.vt ==	(VT_UI4	| VT_ARRAY))
						StringCchPrintf (szNum, 32, _T("%u"), *(DWORD *)lpByte);
					else if (vtVal.vt ==	(VT_I4	| VT_ARRAY))
						StringCchPrintf (szNum, 32, _T("%d"), *(DWORD *)lpByte);
				}

				else if (cbElements == 8)
					StringCchPrintf (szNum, 32, _T("%G"), *(double *)lpByte);
				else
				{
					ASSERT (FALSE);
				}

				// Delimit each element within the row with a comma:
				if (dwCol != 0)
					strVal += _T(", ");

				// Append the formatted element data:
				strVal += szNum;

				// Terminate each row (except the last):
				if (++dwCol == dwCols)
				{
					if (i < dwSize - cbElements)
						strVal += _T(" ] [ ");

					dwCol = 0;
				}
			}
			// End delimiter:
			strVal += _T(" ]");
		}
		break;
	default:
		// Unsupported datatype:
		strVal = _T ("<Bad VARTYPE>");
		break;
	}

	// Copy value to output buffer:
	lstrcpyn (pBuffer, strVal, nBufLen);
}

void COPCConnection::AddLeaves (LPENUMSTRING pIEnumString)
{
	ULONG celt			= 1;
	LPOLESTR rgelt		= NULL;
	ULONG celtFetched	= 0;
	int nIndex			= 0;

#ifndef _UNICODE
	TCHAR szBuffer[DEFBUFFSIZE];
#endif

	// Delete any leaves that are presently being displayed:
	//	m_pLeafList->DeleteAllItems ();

	// Start at the beginning of the list:
	pIEnumString->Reset();
	pIEnumString->Next(celt, &rgelt, &celtFetched);

	// Add each leaf to the leaf control:
	while (celtFetched > 0) 
	{	
		wstring itemname = rgelt;
		CKItem* pItem = new CKItem(m_pGroup);

		pItem->SetAccessPath(_T(""));
		pItem->SetItemID(itemname.c_str());
		pItem->SetActive(TRUE);
		m_itemlist.Add(pItem);
        
        // 
        m_searchmap.insert(make_pair(itemname, pItem));

		// Free the branch name:
		CoTaskMemFree(rgelt);	

		// Re-initialize and get the next item:
		celt = 1;
		celtFetched = 0;
		pIEnumString->Next(celt, &rgelt, &celtFetched);
	}

	m_pGroup->AddItems(m_itemlist, (DWORD)m_itemlist.GetCount());
}

void COPCConnection::SetServerProgName(const wstring& strProgID)
{
	m_strserverprogid = strProgID;
}

bool COPCConnection::Connect(wstring strOPCServerIP,int nOPCClientIndex)
{
	if(strOPCServerIP.length()>0)
		m_strserverhost = strOPCServerIP;

	CString progid = m_strserverprogid.c_str();
	CString serverhost = m_strserverhost.c_str();
	if (NULL == m_pServer)
	{		
		if(nOPCClientIndex < 0)
		{
			_tprintf(L"Connect to OPC Server :");
			_tprintf(serverhost.GetString());
			_tprintf(L"\r\n");
		}
		else
		{	
			CString strOut;
			strOut.Format(_T("Connect to OPC Server :%s(index:%d)\r\n"),serverhost,nOPCClientIndex);
			_tprintf(strOut.GetString());
		}

		m_pServer = new CKServer(progid, serverhost);
		if (NULL == m_pServer){
			return false;
		}
	}

	if(m_strserverhost == L"localhost" || m_strserverhost == L"127.0.0.1")
	{
		m_isconnected = m_pServer->Connect();
	}
	else
	{
		CLSID clsid;
		if(GetCLSIDByProgID(strOPCServerIP,m_strserverprogid,clsid))
		{
			m_isconnected = m_pServer->ConnectByCLSID(clsid);
		}
		else
		{
			_tprintf(_T("ERROR:Can not find OPC Service...\r\n"));
		}
	}
	return m_isconnected;
}

void COPCConnection::DisConnect(void)
{
	m_isconnected = false;
	if (NULL != m_pServer){
		m_pServer->Disconnect();
	}

	/*if (NULL != m_pServer){
	delete m_pServer;
	m_pServer = NULL;
	}*/
	
	m_itemlist.RemoveAll();

	//clear item;
	hash_map<wstring, CKItem*>::iterator it = m_searchmap.begin();
	for(;it!=m_searchmap.end();it++)
	{
		CKItem *pitem = (*it).second;
		if(pitem)
		{
			delete(pitem);
			pitem = NULL;
		}
	}

	m_searchmap.clear();
	m_searchEntrymap.clear();
}

void COPCConnection::AddGroup(void)
{
	if (NULL == m_pGroup){
		m_pGroup = new CKGroup(m_pServer);
	}

	m_pGroup->SetName(_T("beopopc"));
	m_pGroup->SetActive(TRUE);
	m_pGroup->SetLanguageID(1033);
	m_pGroup->SetDeadband(0);
	m_pGroup->SetBias(0);
	m_pGroup->SetUpdateRate(100);
	m_pGroup->SetUpdateMethod(2);

	m_pServer->AddGroup(m_pGroup);
}

bool COPCConnection::AddItems()
{
	int			 nPos	= 0;
	HRESULT		 hr		= 0;
	LPENUMSTRING pIEnumString = NULL;
	CStringArray strBranches;

#ifndef _UNICODE
	WCHAR szFilter [DEFBUFFSIZE];
#endif
	hr = S_OK;

	// > 0 we do not want to include the "Root" item since the client
	// only uses this branch:
	IOPCBrowseServerAddressSpace* pIBrowser = m_pServer->GetIBrowse();
	do
	{
		hr = pIBrowser->ChangeBrowsePosition (OPC_BROWSE_UP, L"\0");
	} while (SUCCEEDED (hr));

	// Now browse down to the new position:
	strBranches.SetSize ( nPos + 1 );
	// 	hParentItem = hItem;

	for (int i = 0; i<=nPos; i++){
		strBranches.Add(m_strserverprogid.c_str());
	}

	while (SUCCEEDED(hr) && nPos-->0)
	{
#ifdef _UNICODE
		hr = pIBrowser->ChangeBrowsePosition(OPC_BROWSE_DOWN, strBranches[nPos]);
#else
		WCHAR szBranch [DEFBUFFSIZE];

		_mbstowcsz (szBranch, strBranches [nPos], sizeof (szBranch) / sizeof (WCHAR));
		hr = pIBrowser->ChangeBrowsePosition (OPC_BROWSE_DOWN, szBranch);
#endif
	}

	// Browse for root level:
#ifdef _UNICODE
	hr = pIBrowser->BrowseOPCItemIDs(
									m_bBrowseFlat ? OPC_FLAT : OPC_LEAF,	// provide items with children
									m_strFilterLeaf.c_str(),						// item id filtering
									VT_EMPTY,							// datatype filter
									0,					// access rights filtering
									&pIEnumString);							// store the interface pointer here
#else
	_mbstowcsz (szFilter, m_strFilterLeaf, sizeof (szFilter) / sizeof (WCHAR));

	hr = pIBrowser->BrowseOPCItemIDs (
		m_bBrowseFlat ? OPC_FLAT : OPC_LEAF,	// provide items with children
		szFilter,								// item id filtering
		VT_EMPTY,							// datatype filter
		0,					// access rights filtering
		&pIEnumString);							// store the interface pointer here
#endif

	// On success add the branches to the root:
	if (SUCCEEDED(hr) && pIEnumString){
		AddLeaves(pIEnumString);
		pIEnumString->Release();

		return true;
	}
	else
		return false;
}

bool COPCConnection::GetItemValue(const wstring& strItemID, wstring &strValue, bool bFromDevice)
{
	if (strItemID.empty()){
		return false;
	}
	CKItem *pItem = FindItem(strItemID);
	if (NULL == pItem){
		//没有找到此节点
		return false;
	}

	wstring str_accesspath = pItem->GetAccessPath();
	int vtdatatype	= pItem->GetDataType();
	wstring str_ItemID		= pItem->GetItemID();
	IOPCItemProperties* pItempPro	= m_pServer->GetIItemProps();

	if(pItempPro==NULL)
	{
		//_tprintf(L"IOPCItemProperties OPC return NULL.\r\n");
		return false;
	}

	// Fill available 2.0 item properties (We won't be able to do this
	// unless we have a pointer to the item properties interface.)
	if (pItempPro != NULL)
	{
		TCHAR szBuffer[DEFBUFFSIZE] = {0};
		DWORD	dwCount			= 0;
		DWORD	dwIndex			= 0;
		DWORD	*pdwIDs			= NULL;
		LPWSTR	*pszDescriptions= NULL;
		LPWSTR	*pszLookupIDs	= NULL;
		VARTYPE *pvtDataTypes	= NULL;
		VARIANT *pvtValues		= NULL;
		HRESULT *pValErrs		= NULL;
		HRESULT *pLookupErrs	= NULL;
		HRESULT	hr				= E_FAIL;
		CString strTemp			= _T("");
		wstring	strQualifiedID  = _T("");

		// Obtain the qualified ID:
		ASSERT (!str_ItemID.empty());

		if (!str_accesspath.empty() && m_pServer->IsKepServerEx()){
			strQualifiedID = str_accesspath;
			strQualifiedID += _T(".");
			strQualifiedID += str_ItemID;
		}
		else{
			strQualifiedID = str_ItemID;
		}

		if (SUCCEEDED(hr))
		{
			for (dwIndex = 0; dwIndex<dwCount; dwIndex++)
			{
				// ID:
				_itot_s(pdwIDs[dwIndex], szBuffer, 10);
				// {roperty value:
				if (pValErrs && SUCCEEDED (pValErrs[dwIndex]))
				{
					GetValue(pvtValues[dwIndex], szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
				}
				else
				{
					lstrcpyn(szBuffer, _T("???"), sizeof(szBuffer)/sizeof(TCHAR));
				}

				// Lookup item ID (may have to convert from UNICODE):
				if (pLookupErrs && SUCCEEDED(pLookupErrs[dwIndex]))
				{
#ifdef _UNICODE
					lstrcpyn(szBuffer, pszLookupIDs [dwIndex], sizeof(szBuffer)/sizeof(TCHAR));
#else
					_wcstombsz(szBuffer, pszLookupIDs [dwIndex], sizeof(szBuffer)/sizeof(TCHAR));
#endif
				}
				else
				{
					// No lookup item ID, so place "N/A" in list control:
					lstrcpyn(szBuffer, _T("N/A"), sizeof(szBuffer)/sizeof (TCHAR));
					pszLookupIDs[dwIndex] = NULL;
				}
			}
		}
		pItem->GetValue(strTemp);
		strValue = strTemp;

		// COM requires us to free memory for data passed back to us:
		for (dwIndex = 0; dwIndex<dwCount; dwIndex++)
		{
			if (pszDescriptions && pszDescriptions[dwIndex]) 
				CoTaskMemFree(pszDescriptions[dwIndex]);

			if (pszLookupIDs && pszLookupIDs[dwIndex])
				CoTaskMemFree(pszLookupIDs[dwIndex]);

			// Clear variants:
			if (pvtValues)
				VariantClear(&pvtValues[dwIndex]);
		}

		if (pdwIDs)
			CoTaskMemFree(pdwIDs);

		if (pszDescriptions)
			CoTaskMemFree(pszDescriptions);

		if (pszLookupIDs)
			CoTaskMemFree(pszLookupIDs);

		if (pvtDataTypes)
			CoTaskMemFree(pvtDataTypes);

		if (pvtValues)
			CoTaskMemFree(pvtValues);

		if (pValErrs)
			CoTaskMemFree(pValErrs);

		if (pLookupErrs)
			CoTaskMemFree(pLookupErrs);

		/*if (pszItemID)
			CoTaskMemFree(pszItemID);*/
	}

	return true;
}

bool COPCConnection::SetItemValue(const wstring& strItemID, const wstring& strValue)
{
	if (strItemID.empty()){
		return false;
	}

	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	CKItem *pItem = FindItem(strItemID);

	//没有找到此节点
	if (NULL == pItem){

		return false;
	}

	try
	{
		CObArray		cItems;
		CStringArray	cValues;
		DWORD			cdwItems	= 0;

		// Construct an item and value list for this write operation:
		cItems.SetSize (1);
		cValues.SetSize (1);
		ASSERT(pItem != NULL);
		cItems[cdwItems] = pItem;
		cValues[cdwItems] = strValue.c_str();
		++cdwItems;
		m_pGroup->WriteAsync20(cItems, cValues, cdwItems);
	}
	catch (...)
	{
	}
	
	return true;
}

bool COPCConnection::UpdateItemValue(const wstring& strItemID, wstring &strValue)
{
    //CWaitCursor wc;
    CKItem *pItem = FindItem(strItemID);
    //没有找到此节点
    if (NULL == pItem)
        return false;
    try
    {
        CObArray		cItems;
        CStringArray	cValues;
        DWORD			cdwItems	= 0;

        // Construct an item and value list for this write operation:
        cItems.SetSize (1);
        cValues.SetSize (1);

        ASSERT(pItem != NULL);
        cItems[cdwItems] = pItem;

        // Add corresponding write value to value array:
        cValues[cdwItems] = strValue.c_str();
        ++cdwItems;
        m_pGroup->ReadAsync20(cItems, cdwItems);
    }
    catch (...)
    {
    }

    CString value; 
    pItem->GetValue(value);
    strValue = (LPCTSTR)value;
	return true;
}

VARTYPE COPCConnection::GetVarType( const wstring& strItemName )
{
	VARTYPE result = VT_EMPTY;
	CKItem *pItem = FindItem(strItemName);
	
	if (pItem){
		return pItem->GetDataType();
	}
	return result;
}

CKItem* COPCConnection::FindItem( const wstring& strItemName )
{
	if (strItemName.empty())
		return NULL;

	hash_map<wstring, CKItem*>::iterator it = m_searchmap.find(strItemName);
	hash_map<wstring, OPCDataPointEntry>::iterator it_entry = m_searchEntrymap.find(strItemName);

	if (it != m_searchmap.end())
	{
		CKItem *pitem =  (*it).second;
		OPCDataPointEntry entry = (*it_entry).second;
		if(pitem->GetDataType()!= pitem->GetDataType2())
		{
			return pitem;
		}
		else
			return pitem;
	}

	return NULL;
}

wstring COPCConnection::GetServerProgName() const
{
	return m_strserverprogid;
}

LPCTSTR COPCConnection::GetVarQuality( const wstring& strItemName )
{
	CKItem *pItem = FindItem(strItemName);
	if (pItem){
		return pItem->GetQuality();
	}

	return _T("");
}

bool COPCConnection::GetCLSIDByProgID( wstring strOPCServerIP,wstring strProgID,CLSID& clsid1 )
{
	bool bResult = false;
	MULTI_QI mqi;
	COSERVERINFO sin,*sinptr;
	DWORD clsctx;
	unsigned long c;
	CLSID clsid;
	IEnumGUID *pEnumGUID;
	CLSID catid = CATID_OPCDAServer20;
	IOPCServerList *gpOPC = NULL;
	clsid = CLSID_OPCServerList;
	sinptr = &sin;
	sin.dwReserved1 = sin.dwReserved2 = 0;
	sin.pwszName =  (LPWSTR)(LPCWSTR)strOPCServerIP.c_str();
	sin.pAuthInfo = 0;
	clsctx = CLSCTX_REMOTE_SERVER;
	mqi.pIID = &IID_IOPCServerList;
	mqi.hr = 0;
	mqi.pItf = 0;
	HRESULT hr = CoCreateInstanceEx(clsid,NULL,clsctx,sinptr,1,&mqi);
	gpOPC = (IOPCServerList *)mqi.pItf;
	if(gpOPC)
	{
		hr = gpOPC->EnumClassesOfCategories(1,&catid,1,&catid,&pEnumGUID);
		if(pEnumGUID)
		{
			while(S_OK == pEnumGUID->Next(1,&clsid,&c))
			{
				LPOLESTR pszProgID;
				LPOLESTR pszUserType;
				hr = gpOPC->GetClassDetails(clsid,&pszProgID,&pszUserType);
				if(hr == S_OK)
				{
					CString str;
					str.Format(_T("%s"),pszProgID);
					if(str.Find(strProgID.c_str())>=0)
					{
						clsid1 = clsid;
						bResult = true;
					}
				}
				CoTaskMemFree(pszProgID);
				CoTaskMemFree(pszUserType);
			}
		}
		if(gpOPC)
			gpOPC->Release();
	}
	return bResult;
}

bool COPCConnection::GetMutilItemValue( map<wstring,OPC_VALUE_INFO>& mapItem )
{
	return true;
}

bool COPCConnection::RefreshAsync20( bool bDeviceRead )
{
	return true;
}
	
