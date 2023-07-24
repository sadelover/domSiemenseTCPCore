
// OPC 变量读写的实现类
//
//

#include "StdAfx.h"

#include "OPCCtrl.h"
//#include "Tools/vld.h"
#include "../COMdll/DebugLog.h"

void COPCConnection_AutoBrowse::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}


vector<DataPointEntry> COPCConnection_AutoBrowse::GetPointList() const
{
	return m_pointlist;
}

bool COPCConnection_AutoBrowse::Init(wstring strOPCServerIP,bool bDisableCheckQuality,int nOPCClientIndex,int nUpdateRate, int nLanguageID, int nMultilAddItems, int nDebugOPC)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	m_bOPCConnect = false;
	m_bReloadSynErrorItem = false;
	m_nUpdateRate = nUpdateRate;
	m_bDisableCheckQuality = bDisableCheckQuality;
	m_nLanguageID = nLanguageID;
	m_nMultiAddItems = nMultilAddItems;
	m_nOutPut = nDebugOPC;
	if (m_pointlist.empty())
	{
		ASSERT(false);
		return false;
	}

	m_Positionmap.clear();
	//build search map;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry= m_pointlist[i];
		m_namemap[entry.GetShortName()] = entry.GetPlcAddress();
		int nPosition = _wtoi(entry.GetParam(7).c_str());
		if(nPosition>0)
			m_Positionmap[entry.GetShortName()] = nPosition;
	}

	//connect opc server 
	m_strOPCServerIP = strOPCServerIP;
	m_nOPCClientIndex = nOPCClientIndex;
	m_bOPCConnect = Connect(strOPCServerIP,nOPCClientIndex);
	if (!m_bOPCConnect)
	{
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_CONNECT_OPC);
		return m_bOPCConnect;
	}

	m_bAddGroupSuccess = AddGroup(m_nUpdateRate,m_nLanguageID);
	if(m_bAddGroupSuccess)
	{
		CString strLog;
		strLog.Format(_T("INFO : Add OPC Group Success(language:%d, rate:%dms, addrate:%d, index:%d).\r\n"),m_nLanguageID,m_nUpdateRate,m_nMultiAddItems,m_nOPCClientIndex);
		PrintLog(strLog.GetString(),false);
	}
	else
	{
		CString strLog;
		strLog.Format(_T("ERROR: Add OPC Group Fail(language:%d, rate:%dms, addrate:%d, index:%d).\r\n"),m_nLanguageID,m_nUpdateRate,m_nMultiAddItems,m_nOPCClientIndex);
		PrintLog(strLog.GetString(),false);
	}
	if(m_bAddGroupSuccess)
	{
		PrintLog(L"INFO : Add OPC Items...\r\n");
		m_bAddItemSuccess = AddItems(m_nMultiAddItems);
	}

	return m_bOPCConnect&&m_bAddGroupSuccess;
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
	m_bOPCConnect = ReConnect();
	if (!m_bOPCConnect)
	{
		PrintLog(L"ERROR: OPC Engine Connected Failed.\r\n",false);
		CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_CONNECT_OPC);
		return m_bOPCConnect;
	}
	else
	{
		PrintLog(L"INFO : OPC Engine Connected Success.\r\n",false);
		if(!m_bAddGroupSuccess)				//尚未添加组则需添加组
		{
			m_bAddGroupSuccess = AddGroup(m_nUpdateRate,m_nLanguageID);		
			if(m_bAddGroupSuccess)
			{
				CString strLog;
				strLog.Format(_T("INFO : Add OPC Group Success(language:%d, rate:%dms, index:%d).\r\n"),m_nLanguageID,m_nUpdateRate,m_nOPCClientIndex);
				PrintLog(strLog.GetString(),false);

			}
			else
			{
				CString strLog;
				strLog.Format(_T("ERROR: Add OPC Group Fail(language:%d, rate:%dms, index:%d).\r\n"),m_nLanguageID,m_nUpdateRate,m_nOPCClientIndex);
				PrintLog(strLog.GetString(),false);

			}
			if(m_bAddGroupSuccess)			//添加组后续重新添加点
			{
				m_bAddItemSuccess = AddItems(m_nMultiAddItems);
			}
		}
		else
		{
			if(!m_bAddItemSuccess)				//尚未添加点则需添加点
				m_bAddItemSuccess = AddItems(m_nMultiAddItems);
		}
		
		return m_bOPCConnect&&m_bAddItemSuccess;
	}
	return m_bOPCConnect&&m_bAddItemSuccess;
}

bool COPCConnection_AutoBrowse::ReLoadItemsHandle(vector<wstring> vecItemID,bool bAsync20)
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
				pItem->SetActive(TRUE);
				itemlist.Add(pItem);
			}
		}

		if(itemlist.GetCount() >0)
		{
			m_pGroup->RemoveItems(itemlist, (DWORD)itemlist.GetCount(),false);
			m_pGroup->AddItems(itemlist, (DWORD)itemlist.GetCount(),true);

			CString strOut;
			if(bAsync20)
			{
				strOut.Format(_T("INFO : OPC Engine(%d):ReloadItem(%d).\r\n"),m_nOPCClientIndex,(DWORD)itemlist.GetCount());
			}
			else
			{
				strOut.Format(_T("INFO : OPC Engine(%d):ReloadItem Async20(%d).\r\n"),m_nOPCClientIndex,(DWORD)itemlist.GetCount());
			}
			PrintLog(strOut.GetString(),false);
			return true;
		}
	}
	return false;
}

bool COPCConnection_AutoBrowse::ReloadErrorItemsHandle()
{
	if(m_pGroup)
	{
		//添加错误点
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
				ReLoadItemsHandle(vecItem);
				Sleep(1000);
			}
		}

		//添加错误无效点 异步模式点
		CObArray cItemAsync20List;
		if(m_pGroup->GetAsync20ErrorItem(cItemAsync20List))
		{
			if(cItemAsync20List.GetCount()>0)
			{
				vector<wstring> vecItem;
				for(int i=0;i<cItemAsync20List.GetCount(); ++i)
				{
					CKItem *pItem = (CKItem *) cItemAsync20List [i];
					if(pItem && !pItem->IsActive())
					{
						vecItem.push_back(pItem->GetItemID());
					}
				}
				return ReLoadItemsHandle(vecItem,true);
			}
		}

		//同步
		if(m_bReloadSynErrorItem)
		{
			CObArray cItemSync20List;
			if(m_pGroup->GetSync20ErrorItem(cItemSync20List))
			{
				if(cItemSync20List.GetCount()>0)
				{
					vector<wstring> vecItem;
					for(int i=0;i<cItemSync20List.GetCount(); ++i)
					{
						CKItem *pItem = (CKItem *) cItemSync20List [i];
						if(pItem && !pItem->IsActive())
						{
							vecItem.push_back(pItem->GetItemID());
						}
					}
					return ReLoadItemsHandle(vecItem);
				}
			}
		}
	}
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

bool COPCConnection_AutoBrowse::ActiveReloadSynErrorItems()
{
	m_bReloadSynErrorItem = true;
	return true;
}

void COPCConnection_AutoBrowse::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
	if(bSaveLog)
	{
		if(m_nOutPut == 1)
		{
			OutDebugInfo(strLog);
		}
	}
}

bool COPCConnection_AutoBrowse::ReReadAsync20Item(CObArray &cAsync20WriteItemList)
{
	bool bReadSuccess = false;
	//首先获取需要重新读取的点
	if(m_pGroup && m_bOPCConnect)
	{
		//添加异步写成功的点
		if(m_pGroup->GetAsync20WriteItem(cAsync20WriteItemList))
		{
			if(cAsync20WriteItemList.GetCount()>0)
			{
				//读写对应的点
				DWORD cdwItems = cAsync20WriteItemList.GetSize();
				bReadSuccess = m_pGroup->ReadSync(cAsync20WriteItemList, cdwItems, false);
			}
		}
	}
	return bReadSuccess;
}

bool COPCConnection_ManualBrowse::AddItems(int nMultiAdd)
{
	if (m_pointlist.empty())
	{
		return false;
	}

	if(m_nOutPut == 1)
	{
		CString strLog;
		strLog.Format(_T("INFO : Add OPC Items(%d).\r\n"),m_nMultiAddItems);
		PrintLog(strLog.GetString(),false);

	}

	m_itemlist.RemoveAll();
	CObArray	itemList;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const DataPointEntry& entry = m_pointlist[i];
		CKItem* pitem	= new CKItem(m_pGroup);
		pitem->SetAccessPath(_T(""));
		pitem->SetItemID(entry.GetPlcAddress().c_str());
		pitem->SetActive(TRUE);
		pitem->SetDataType(entry.GetOpcPointType() );
		m_itemlist.Add(pitem);
		m_searchmap.insert(std::make_pair(entry.GetPlcAddress().c_str(), pitem));

		if(m_nOutPut == 1)
		{
			CString strLog;
			strLog.Format(_T("INFO : Add OPC Items %s(%d/%d).\r\n"),entry.GetPlcAddress().c_str(),i,m_pointlist.size());
			PrintLog(strLog.GetString(),false);

		}
		//
		itemList.Add(pitem);
		DWORD dwSize = itemList.GetCount();
		if(dwSize >= nMultiAdd)
		{
			m_pGroup->AddItems(itemList, dwSize);
			itemList.RemoveAll();
			Sleep(10);
		}
	}

	DWORD dwSize = itemList.GetCount();
	if(dwSize > 0)
	{
		m_pGroup->AddItems(itemList, dwSize);
		itemList.RemoveAll();
		Sleep(10);
	}

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
		CString strLog;
		strLog.Format(_T("ERROR: OPCItem(%s) is Not Valid.\r\n"),pItem->GetItemID());
		PrintLog(strLog.GetString(),false);
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
		return false;
	}
	if(pItem->GetQualityCode() != OPC_QUALITY_GOOD_NON_SPECIFIC && !m_bDisableCheckQuality)
	{
		CString strOut;
		strOut.Format(_T("ERROR: OPC Engine(%d):GetItemValue(%s) Failed(%s).\r"),m_nOPCClientIndex,strItemID.c_str(),pItem->GetQuality());
		PrintLog(strOut.GetString());
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
		CString strOut;
		strOut.Format(_T("ERROR: OPCItem(%s) is Not Valid.\r\n"),pItem->GetItemID());
		PrintLog(strOut.GetString());

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
			CString strOut;
			strOut.Format(_T("ERROR: OPCItem(%s) is Not Valid.\r\n"),pItem->GetItemID());
			PrintLog(strOut.GetString());

			pItem->SetToUpdate(FALSE);
			mapValue[iter->first] = NULL;
			iter++;
		}
		else
		{
			pItem->SetToUpdate(FALSE);
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
					if(iter3->second->IsUpdated())			//值更新
					{
						CString strTmpVal = _T(""); 
						iter3->second->GetValue(strTmpVal);
						if(iter3->second->GetQualityCode() != OPC_QUALITY_GOOD_NON_SPECIFIC && !m_bDisableCheckQuality)
						{
							iter2->second.bUpdate = false;
							if(m_nOutPut == 1)
							{
								CString strOut;
								strOut.Format(_T("ERROR: COPCConnection_ManualBrowse(%d):Read MutilValue(%s) Failed(%s).\r\n"),m_nOPCClientIndex,strOPCName.c_str(),iter3->second->GetQuality());
								PrintLog(strOut.GetString());
							}
							CDebugLog::GetInstance()->SetErrPoint(strOPCName.c_str(),POINTTYPE_OPC_,ERROR_CUSTOM_OPC_VALUE_NOT_GOOD);
						}
						else
						{
							iter2->second.strOPCValue = (LPCTSTR)strTmpVal;
							iter2->second.bUpdate = true;
						}	
					}
					else
					{
						iter2->second.strOPCValue = L"";
						iter2->second.bUpdate = false;
					}	
				}
			}
			iter2++;
		}
	}
	catch (...)
	{
		
	}
	return true;
}

bool COPCConnection_ManualBrowse::RefreshAsync20(bool bDeviceRead)
{
	if(m_pGroup)
	{
		m_pGroup->RefreshAsync20(bDeviceRead);
		return true;
	}
	return false;
}
