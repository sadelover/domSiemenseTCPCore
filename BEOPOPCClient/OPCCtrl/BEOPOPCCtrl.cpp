
// OPC 变量读写的实现类
//
//

#include "StdAfx.h"

#include "BEOPOPCCtrl.h"


void COPCConnection_AutoBrowse::SetPointList( const vector<OPCDataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}


vector<OPCDataPointEntry> COPCConnection_AutoBrowse::GetPointList() const
{
	return m_pointlist;
}

bool COPCConnection_AutoBrowse::Init(wstring strOPCServerIP,bool bDisableCheckQuality,int nOPCClientIndex)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	m_bOPCConnect = false;
	m_bDisableCheckQuality = bDisableCheckQuality;
	if (m_pointlist.empty())
	{
		ASSERT(false);
		return false;
	}

	m_Positionmap.clear();
	//build search map;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const OPCDataPointEntry& entry= m_pointlist[i];
		m_namemap[entry.GetShortName()] = entry.GetPlcAddress();
		int nPosition = _wtoi(entry.GetParam(7).c_str());
		if(nPosition>0)
			m_Positionmap[entry.GetShortName()] = nPosition;
	}

	//connect opc server 
	m_strOPCServerIP = strOPCServerIP;
	m_nOPCClientIndex = nOPCClientIndex;
	m_nOutPut = 0;
	m_bOPCConnect = Connect(strOPCServerIP,nOPCClientIndex);
	if (!m_bOPCConnect)
	{
		return m_bOPCConnect;
	}

	AddGroup();
	AddItems();

	return true;
}

wstring COPCConnection_AutoBrowse::GetPlcNameByShortName( const wstring& shortname )
{
	hash_map<wstring, wstring>::const_iterator it = m_namemap.find(shortname);
	if (it != m_namemap.end()){
		return it->second;
	}

	return _T("");
}

void COPCConnection_AutoBrowse::AddErrorLog(const wstring& shortname)
{

}

VARTYPE COPCConnection_AutoBrowse::GetItemTypeByShortName( const wstring& strShortName )
{
	wstring plcaddress = GetPlcNameByShortName(strShortName);
	VARTYPE vartype = GetVarType(plcaddress);
	return vartype;
}


LPCTSTR COPCConnection_AutoBrowse::GetItemQualityByShortName( const wstring& strShortName )
{
	wstring plcaddress = GetPlcNameByShortName(strShortName);
	LPCTSTR itemquality = GetVarQuality(plcaddress);
	return itemquality;
}

void COPCConnection_AutoBrowse::ClearPointList()
{
	m_pointlist.clear();
}

bool COPCConnection_AutoBrowse::Exit()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	m_bOPCConnect = false;
	m_Positionmap.clear();
	if(m_pServer && m_pServer->IsConnected())
	{
		if(m_pGroup)
			m_pGroup->RemoveAllItems(true);
		m_pServer->RemoveAllGroups(true);
		DisConnect();
		//m_pGroup = NULL;
		if(m_pGroup != NULL)
		{
			m_pGroup = NULL;
		}
		if (NULL != m_pServer)
		{
			delete m_pServer;
			m_pServer = NULL;
		}
		//m_pServer = NULL;
	}
	return true;
}

void COPCConnection_AutoBrowse::SetDebug( int nDebug )
{
	m_nOutPut = nDebug;
}

void COPCConnection_AutoBrowse::OutDebugInfo( const wstring strOut )
{
	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";	 
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		SYSTEMTIME sNow;
		GetLocalTime(&sNow);
		CString strFilePath;
		strFilePath.Format(_T("%s\\opc_debug_%d_%02d_%02d.log"),strPath.c_str(),sNow.wYear,sNow.wMonth,sNow.wDay);

		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		CString strLog;
		strLog.Format(_T("%s::%s"),strTime.c_str(),strOut.c_str());

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strFilePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
		}
	}
}

bool COPCConnection_AutoBrowse::ReConnectOPC()
{
	Exit();
	//connect opc server 
	m_bOPCConnect = Connect(m_strOPCServerIP,m_nOPCClientIndex);
	if (!m_bOPCConnect)
	{
		_tprintf(L"OPC Engine Connected Failed.\r\n");
		return m_bOPCConnect;
	}
	_tprintf(L"OPC Engine Connected Success.\r\n");
	AddGroup();
	AddItems();
	return true;
}

bool COPCConnection_AutoBrowse::ReLoadItemsHandle(vector<wstring> vecItemID)
{
	if(vecItemID.size() <= 0)
		return false;

	if(vecItemID.size() > 0)
	{
		CObArray	itemlist;	//保存所有的Item
		for(int i=0; i<vecItemID.size(); ++i)
		{
			wstring strItemID = vecItemID[i];
			CKItem *pItem = FindItem(strItemID);
			if (NULL != pItem )
			{
				itemlist.Add(pItem);
			}
		}

		if(itemlist.GetCount() >0)
		{
			m_pGroup->RemoveItems(itemlist, (DWORD)itemlist.GetCount(),false);
			m_pGroup->AddItems(itemlist, (DWORD)itemlist.GetCount());

			CString strOut;
			strOut.Format(_T("OPC Engine(%d):ReloadItem(%d).\r\n"),m_nOPCClientIndex,(DWORD)itemlist.GetCount());
			_tprintf(strOut);
			return true;
		}
	}
	return false;
}

bool COPCConnection_AutoBrowse::ReloadErrorItemsHandle()
{
	if(m_pGroup)
	{
		CObArray cItemList;
		if(m_pGroup->GetErrorItem(cItemList))
		{
			if(cItemList.GetCount()>0)
			{
				vector<wstring> vecItem;
				for(int i=0;i<cItemList.GetCount(); ++i)
				{
					CKItem *pItem = (CKItem *) cItemList [i];
					if(pItem)
					{
						vecItem.push_back(pItem->GetItemID());
					}
				}
				return ReLoadItemsHandle(vecItem);
			}
		}
	}

	/*if(m_pGroup)
	{
		wstring str = L"$秒.Value";
		vector<wstring> vecItem;
		vecItem.push_back(str);
		return ReLoadItemsHandle(vecItem);
	}*/

	return false;
}

int COPCConnection_AutoBrowse::GetPosition(const wstring shortname)
{
	hash_map<wstring, int>::const_iterator it = m_Positionmap.find(shortname);
	if (it != m_Positionmap.end()){
		return it->second;
	}
	return 0;
}

bool COPCConnection_ManualBrowse::AddItems()
{
	if (m_pointlist.empty()){
		return false;
	}
	
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const OPCDataPointEntry& entry = m_pointlist[i];
		CKItem* pitem	= new CKItem(m_pGroup);
		pitem->SetAccessPath(_T(""));
		pitem->SetItemID(entry.GetPlcAddress().c_str());
		pitem->SetActive(TRUE);
		pitem->SetDataType(entry.GetOpcPointType() );
		m_itemlist.Add(pitem);

		// 
		m_searchmap.insert(std::make_pair(entry.GetPlcAddress().c_str(), pitem));

		//add by golding
		OPCDataPointEntry entryTemp = entry;
		m_searchEntrymap.insert(std::make_pair(entry.GetPlcAddress().c_str(), entryTemp));
	}
	
	m_pGroup->AddItems(m_itemlist, (DWORD)m_itemlist.GetCount());

	//clear point add by robert 20150828
	ClearPointList();
	return true;
}

bool COPCConnection_ManualBrowse::GetItemValue( const wstring& strItemID, wstring &strValue , bool bFromDevice)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	CKItem *pItem = FindItem(strItemID);
	//	assert(pItem);
	if (NULL == pItem )
	{
		return false;
	}

	if(!pItem->IsValid())
	{
		_tprintf(L"ERROR:OPCItem(%s) is Not Valid.\r\n",pItem->GetItemID());
		return false;
	}

	/*CString cstrFile = _T("E:/opcdebug.ini");
	wchar_t charDBName[1024];
	GetPrivateProfileString(L"opc", L"defaultpoint", L"", charDBName, 1024, cstrFile);
	CString strDefaultDB = charDBName;
	if(pItem->GetItemID() == strDefaultDB)
	{
		int a = 0;
	}*/

	CString strTmpVal = _T(""); 
	try
	{
		CObArray		cItems;
		CStringArray	cValues;
		DWORD			cdwItems	= 0;

		// Construct an item and value list for this write operation:
		cItems.SetSize(1);
		cValues.SetSize(1);
		cItems[cdwItems] = pItem;

		// Add corresponding write value to value array:
		cValues[cdwItems] = strValue.c_str();
		++cdwItems;
		//m_pGroup->RefreshAsync20(true);
		bool bReadSuccess = m_pGroup->ReadSync(cItems, cdwItems, bFromDevice);
		if(!bReadSuccess)
			return false;
	}
	catch (...)
	{
		_tprintf(L"catch exception");
		return false;
	}
	if(pItem->GetQualityCode() != OPC_QUALITY_GOOD_NON_SPECIFIC && !m_bDisableCheckQuality)
	{
		CString strOut;
		strOut.Format(_T("OPC Engine(%d):GetItemValue(%s) Failed(%s).\r"),m_nOPCClientIndex,strItemID.c_str(),pItem->GetQuality());
		_tprintf(L"%s\n",strOut);

		if(m_nOutPut == 1)
		{
			OutDebugInfo(strOut.GetString());
		}
		return false;
	}
	pItem->GetValue(strTmpVal);
	strValue = (LPCTSTR)strTmpVal;
	return true;
}

bool COPCConnection_ManualBrowse::SetItemValue( const wstring& strItemID, wstring &strValue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	CKItem *pItem = FindItem(strItemID);
	assert(pItem);
	if (NULL == pItem){
		return false;
	}

	if(pItem->GetDataType()!=pItem->GetDataType2())
	{
		int xx=0;
	}
	if(!pItem->IsValid())
	{
		_tprintf(L"ERROR:OPCItem(%s) is Not Valid.\r\n",pItem->GetItemID());
		return false;
	}

	try
	{
		CObArray	  cItems;
		CStringArray  cValues;
		DWORD		  cdwItems = 0;

		// Construct an item and value list for this write operation:
		cItems.SetSize (1);
		cValues.SetSize (1);
		cItems[cdwItems] = pItem;

		// Add corresponding write value to value array:
		cValues[cdwItems] = strValue.c_str();
		++cdwItems;

		m_pGroup->WriteSync(cItems, cValues, cdwItems);
	}
	catch (...)
	{
	}


	if(pItem->GetDataType()!=pItem->GetDataType2())
	{
		int xx=0;
	}

	return true;
}

bool COPCConnection_ManualBrowse::GetMutilItemValue( map<wstring,OPC_VALUE_INFO>& mapItem )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	DWORD nSize = mapItem.size();
	if(nSize <= 0)
		return false;
	CObArray		cItems;
	DWORD			cdwItems	= 0;
	CStringArray	cValues;
	map<wstring,OPC_VALUE_INFO>::iterator iter = mapItem.begin();
	map<wstring,CKItem*> mapValue;
	cItems.SetSize(nSize);
	cValues.SetSize(nSize);
	while(iter != mapItem.end())
	{
		wstring plcaddress = GetPlcNameByShortName(iter->first);
		CKItem *pItem = FindItem(plcaddress);
		//	assert(pItem);
		if (NULL == pItem)
		{
			mapValue[iter->first] = NULL;
			iter++;
		}
		else if(pItem != NULL && !pItem->IsValid())
		{
			_tprintf(L"ERROR:OPCItem(%s) is Not Valid.\r\n",pItem->GetItemID());
			mapValue[iter->first] = NULL;
			iter++;
		}
		else
		{
			mapValue[iter->first] = pItem;
			cItems[cdwItems] = pItem;
			cValues[cdwItems] = iter->second.strOPCValue.c_str();
			cdwItems++;
			iter++;
		}
	}

	try
	{
		bool bReadSuccess = m_pGroup->ReadSync(cItems, cdwItems, false);
		if(!bReadSuccess)
			return false;

		map<wstring,OPC_VALUE_INFO>::iterator iter2 = mapItem.begin();
		while(iter2 != mapItem.end())
		{
			wstring strOPCName = iter2->first;
			map<wstring,CKItem*>::iterator iter3 = mapValue.find(strOPCName);
			if(iter3 != mapValue.end())
			{
				if(iter3->second == NULL)
				{
					iter2->second.strOPCValue = L"";
					iter2->second.bUpdate = false;
				}
				else
				{
					CString strTmpVal = _T(""); 
					iter3->second->GetValue(strTmpVal);
					if(iter3->second->GetQualityCode() != OPC_QUALITY_GOOD_NON_SPECIFIC && !m_bDisableCheckQuality)
					{
						iter2->second.bUpdate = false;
						if(m_nOutPut == 1)
						{
							CString strOut;
							strOut.Format(_T("OPC Engine(%d):Read MutilValue(%s) Failed(%s).\r"),m_nOPCClientIndex,strOPCName.c_str(),iter3->second->GetQuality());
							_tprintf(L"%s\n",strOut);
							OutDebugInfo(strOut.GetString());
						}
					}
					else
					{
						iter2->second.strOPCValue = (LPCTSTR)strTmpVal;
						iter2->second.bUpdate = true;
					}		
				}
			}
			iter2++;
		}
	}
	catch (...)
	{
		_tprintf(L"catch exception");
	}
	return true;
}

bool COPCConnection_ManualBrowse::RefreshAsync20(bool bDeviceRead)
{
	m_pGroup->RefreshAsync20(bDeviceRead);
	return true;
}
