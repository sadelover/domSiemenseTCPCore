#include "StdAfx.h"
#include "BEOPOPCLink.h"
#include "./OPCCtrl/BEOPOPCCtrl.h"
#include "Tools/EngineInfoDefine.h"

COPCEngine::COPCEngine(wstring strOPCServerIP,int nSleep,int nMutilCount,bool bUseThread,int nSleepFromDevice,int nPollSleep,int nPollSleepFromDevice,bool bDisableCheckQuality,int nPrecision,bool bAsync20Mode,int nRefresh20Interval)
	:m_opcsession(NULL)
	,m_strOPCServerIP(strOPCServerIP)
	,m_bMultiClientMode(false)
	,m_bConnectOK(false)
	,m_hDataReadThread(NULL)
	,m_hDataReadDeviceThread(NULL)
	,m_hOPCConnectThread(NULL)
	,m_bExitThread(false)
	,m_bUseThread(bUseThread)
	,m_nMutilCount(nMutilCount)
	,m_nSleep(nSleep)
	,m_nReSponse(0)
	,m_nReadCount(0)
	,m_nReadDeviceCount(0)
	,m_nReSponseDevice(0)
	,m_nOPCClientIndex(-1)
	,m_nOPCCmdSleepFromDevice(nSleepFromDevice)
	,m_nOPCPollSleep(nPollSleep)
	,m_nOPCPollSleepFromDevice(nPollSleepFromDevice)
	,m_nOPCReconnectInertval(30)
	,m_nOPCCheckReconnectInterval(5)
	,m_bReconnect(false)
	,m_nReconnectMinute(0),
	m_nOPCMainThreadOPCPollSleep(2),
	m_bEnableSecurity(false),
	m_nMainUpdateIndex(0),
	m_nSingleReadCount(0),
	m_bDisableCheckQuality(bDisableCheckQuality),
	m_nReloadErrItem(0),
	m_nDebugOPC(0),
	m_nPrecision(nPrecision),
	m_bAsync20Mode(bAsync20Mode),
	m_nRefresh20Interval(nRefresh20Interval)
{
	if(m_nOPCCmdSleepFromDevice <= 0)
		m_nOPCCmdSleepFromDevice = 50;

	if(m_nOPCPollSleepFromDevice <= 0)
		m_nOPCPollSleepFromDevice = 60;

	m_oleDeviceTime = COleDateTime::GetCurrentTime();
	m_oleUpdateTime = COleDateTime::GetCurrentTime(); 
	m_oleReConnectTime = COleDateTime::GetCurrentTime();
	m_oleRefresh20 = COleDateTime::GetCurrentTime();
	m_PositionValuemap.clear();
}


COPCEngine::~COPCEngine(void)
{
	ClearResource();
}

void COPCEngine::SetOpcPointList( const vector<OPCDataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

bool COPCEngine::Init(int nOPCClientIndex)
{
	//first , create opc connection.
	if (m_pointlist.empty())
	{
		OutErrLog(L"ERROR:Point List Is Empty!\n");
		return false;
	}
	m_PositionValuemap.clear();
	m_nOPCClientIndex = nOPCClientIndex;
	m_nSingleReadCount = 0;
	const OPCDataPointEntry& entry = m_pointlist[0];
	m_opcsession = CreateOPCObject(entry.GetServerType());
	ASSERT(m_opcsession);
	if (!m_opcsession)
	{
		return false;
	}

	wstring progname = entry.GetServerProgName();
	if (progname.empty())
	{
		OutErrLog(L"ERROR:OPC Name Is Empty!\n");
		return false;
	}

	m_opcsession->SetPointList(m_pointlist);
	m_opcsession->SetServerProgName(entry.GetServerProgName());

	m_bConnectOK = m_opcsession->Init(m_strOPCServerIP,m_bDisableCheckQuality,nOPCClientIndex);
	m_hOPCConnectThread = (HANDLE) _beginthreadex(NULL, 0, ThreadConnectOPCData, this, NORMAL_PRIORITY_CLASS, NULL);
	if(nOPCClientIndex>=0 && m_bUseThread )
	{
		m_hDataReadThread = (HANDLE) _beginthreadex(NULL, 0, ThreadUpdateOPCData, this, NORMAL_PRIORITY_CLASS, NULL);
	}
	CString strLog;
	strLog.Format(_T("Init OPC Engine (ip:%s, point:%d, thread:%d, cmd:%dms, poll:%ds).\n"),m_strOPCServerIP.c_str(),(int)m_pointlist.size(),(int)m_bUseThread,m_nSleep,m_nOPCPollSleep);
	OutErrLog(strLog.GetString(),true);
	return m_bConnectOK;
}
//
void COPCEngine::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	//分异步和同步读写
	if(m_bAsync20Mode)				//异步方式读取
	{
		//每隔几分钟 异步刷新一次
		UpdateOPCDataBuffer();

		COleDateTime oleCurrent = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleSpan = oleCurrent - m_oleRefresh20;
		if(oleSpan.GetTotalMinutes() >= m_nRefresh20Interval)
		{
			Refresh20();
			m_oleRefresh20 = oleCurrent;
		}

	}
	else
	{
		if(m_bUseThread && !m_bEnableSecurity)		//启用线程且未启用进程安全设置  可以启用线程批量读取
		{
			//在线程里执行 UpdateMutilOPCData
		}
		else if(m_bUseThread && m_bEnableSecurity)		//启用线程且启用进程安全设置  只能在主进程里批量读取
		{
			wstring wstrlog = L"****Check OPC Once: Using thread and enable security";
			_tprintf(wstrlog.c_str());
			int nUpdateInterval = (m_nOPCMainThreadOPCPollSleep/2>0)?m_nOPCMainThreadOPCPollSleep/2:1;
			if(m_nMainUpdateIndex%nUpdateInterval == 0)
			{
				wstring wstrlog = L"Start Read Update Mutil OPC Data(UpdateMutilOPCData) Once";
				_tprintf(wstrlog.c_str());
				UpdateMutilOPCData();
			}

			m_nMainUpdateIndex++;
			if(m_nMainUpdateIndex >= 100000)
				m_nMainUpdateIndex = 0;
		}
		else
		{
			int nUpdateInterval = (m_nOPCMainThreadOPCPollSleep/2>0)?m_nOPCMainThreadOPCPollSleep/2:1;
			if(m_nMainUpdateIndex%nUpdateInterval == 0)
				UpdateSingleOPCData();
			m_nMainUpdateIndex++;
			if(m_nMainUpdateIndex >= 100000)
				m_nMainUpdateIndex = 0;
		}
	}

	if(m_bConnectOK)				//连接成功时候才去拿值
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleReConnectTime;
		if(oleSpan.GetTotalMinutes() >= m_nReconnectMinute)
		{
			for (unsigned int i = 0; i < m_pointlist.size(); i++)
			{
				OPCDataPointEntry& entry = m_pointlist[i];
				if(entry.GetUpdateSignal()<10)
				{
					if(entry.GetOpcPointType() == VT_BSTR)
					{
						wstring strValueSet = entry.GetSValue();
						valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
					}
					else
					{
						int nPosition = _wtoi(entry.GetParam(7).c_str());
						if(nPosition>0)			//取位运算
						{
							wstring strValueSet = GetBitFromString(entry.GetSValue(),nPosition);
							valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
						}
						else		//可以计算倍率
						{
							wstring strValueSet = Project::Tools::RemoveDecimalW(entry.GetValue()/entry.GetMultiple(),m_nPrecision);
							valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
						}

					}
				}
			}
		}

		if(m_pointlist.size() >0 )
		{
			wstring strUpdateTime;
			Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

			CString strName;
			strName.Format(_T("%s%d"),TIME_UPDATE_OPC,m_nOPCClientIndex);
			valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
			//valuelist.push_back(make_pair(LOG_OPC, Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str())));
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//启用安全进程时候 所有的OPC操作都必须在主进程完成
	if(m_bEnableSecurity)
	{
		m_nReloadErrItem++;
		if(m_nReloadErrItem%30 == 0)
		{
			ReLoadItemHandle();
			if(m_nReloadErrItem%60 == 0)
				ReConnectOPC();
		}

		if(m_nReloadErrItem>=10000)
			m_nReloadErrItem = 0;
	}
}

void COPCEngine::ClearResource()
{
	m_bConnectOK = false;
	if (m_opcsession)
	{
		delete m_opcsession;
		m_opcsession = NULL;
	}
}

COPCConnection_AutoBrowse* COPCEngine::CreateOPCObject( OPCSERVER_TYPE_ type )
{
	if (TYPE_AUTOBROWSEITEM_ == type){
		COPCConnection_AutoBrowse* presult =  new COPCConnection_AutoBrowse();
	}
	else if (TYPE_MANUALBROWSEITEM_ == type){
		return new COPCConnection_ManualBrowse();
	}
	else{
		ASSERT(FALSE);
		return NULL;
	}

	return NULL;
}

bool COPCEngine::SetValue( const wstring& pointname, double value )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	bool	bResult = false;
	if (m_opcsession)
	{
		int nPosition = m_opcsession->GetPosition(pointname);
		if(nPosition>0)
		{
			double dwValue = 0;
			hash_map<wstring, double>::iterator iter = m_PositionValuemap.find(pointname);
			if(iter != m_PositionValuemap.end())
			{
				dwValue = (*iter).second;
			}
			value = SetBitToFloat(dwValue,nPosition,value);	
		}

		VARTYPE pointtype = m_opcsession->GetItemTypeByShortName(pointname);
		switch(pointtype)
		{
		case  VT_BOOL:
			{
				bool bValue = (fabs(value)>=1e-10);
				bResult = m_opcsession->SetValue(pointname, bValue);
			}
			break;
		case VT_I2:
		case VT_I4:
			{
				int nValue = (int) value;
				bResult = m_opcsession->SetValue(pointname, nValue);
			}
			break;
		case VT_R4:
			bResult = m_opcsession->SetValue(pointname, value);
			break;
		default:
			bResult = m_opcsession->SetValue(pointname, value);
			break;
		}
	}
	return bResult;
}

bool COPCEngine::SetValue( const wstring& pointname, wstring strvalue )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	bool	bResult = false;
	if (m_opcsession)
	{
		int nPosition = m_opcsession->GetPosition(pointname);
		if(nPosition>0)
		{
			double dwValue = 0;
			hash_map<wstring, double>::iterator iter = m_PositionValuemap.find(pointname);
			if(iter != m_PositionValuemap.end())
			{
				dwValue = (*iter).second;
			}
			strvalue = SetBitToString(dwValue,nPosition,strvalue);	
		}

		VARTYPE pointtype = m_opcsession->GetItemTypeByShortName(pointname);
		switch(pointtype)
		{
		case  VT_BOOL:
			{
				bResult = m_opcsession->SetValue(pointname, _wtoi(strvalue.c_str()));
			}
			break;
		case VT_I2:
		case VT_I4:
			{
				bResult = m_opcsession->SetValue(pointname, _wtoi(strvalue.c_str()));
			}
			break;
		case VT_R4:
			bResult = m_opcsession->SetValue(pointname, strvalue);
			break;
		default:
			bResult = m_opcsession->SetValue(pointname, strvalue);
			break;
		}
	}
	return bResult;
}

bool COPCEngine::GetValue( const wstring& pointname, wstring& strvalue )
{
	if (m_opcsession){
		int nPosition = m_opcsession->GetPosition(pointname);
		if(nPosition>0)
		{
			if(m_opcsession->GetValue(pointname, strvalue))
			{
				strvalue = GetBitFromString(strvalue,nPosition);
				return true;
			}
		}
		else
		{
			return m_opcsession->GetValue(pointname, strvalue);
		}
	}

	return false;
}

bool COPCEngine::GetValue( const wstring& pointname, bool& strvalue )
{
	if (m_opcsession){
		return m_opcsession->GetValue(pointname, strvalue);
	}
	return false;
}

bool COPCEngine::GetValue( const wstring& pointname, int& strvalue )
{
	if (m_opcsession){
		int nPosition = m_opcsession->GetPosition(pointname);
		if(nPosition>0)
		{
			if(m_opcsession->GetValue(pointname, strvalue))
			{
				strvalue = GetBitFromInt(strvalue,nPosition);
				return true;
			}
		}
		else
		{
			return m_opcsession->GetValue(pointname, strvalue);
		}
	}

	return false;
}

bool COPCEngine::GetValue( const wstring& pointname, double& strvalue )
{
	if (m_opcsession){
		int nPosition = m_opcsession->GetPosition(pointname);
		if(nPosition>0)
		{
			if(m_opcsession->GetValue(pointname, strvalue))
			{
				strvalue = GetBitFromFloat(strvalue,nPosition);
				return true;
			}
		}
		else
		{
			return m_opcsession->GetValue(pointname, strvalue);
		}
	}

	return false;
}

bool COPCEngine::Exit()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	m_bExitThread = true;
	if(m_hDataReadThread)
	{
		CloseHandle(m_hDataReadThread);
		m_hDataReadThread = NULL;
	}
	if(m_hDataReadDeviceThread)
	{
		CloseHandle(m_hDataReadDeviceThread);
		m_hDataReadDeviceThread = NULL;
	}
	if(m_hOPCConnectThread)
	{
		CloseHandle(m_hOPCConnectThread);
		m_hOPCConnectThread = NULL;
	}
	m_PositionValuemap.clear();
	m_opcsession->Exit();
	return true;
}

void COPCEngine::SetMultiMode( bool bMultiMode )
{
	m_bMultiClientMode = bMultiMode;
}

UINT WINAPI COPCEngine::ThreadUpdateOPCData( LPVOID lparam )
{
	COPCEngine* pthis = (COPCEngine*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread && !pthis->m_bEnableSecurity)
		{
			if(pthis->m_bAsync20Mode == false)
				pthis->UpdateMutilOPCData();	

			int nSleep = pthis->m_nOPCPollSleep;
			while(!pthis->m_bExitThread)
			{
				nSleep--;
				if(nSleep < 0)
					break;
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool COPCEngine::UpdateMutilOPCData()
{
	if(m_bConnectOK)
	{
		unsigned int nStart = 0;
		unsigned int nEnd = 0;
		m_nReSponse = 0;
		m_nReadCount = 0;
		for(nEnd; nEnd < m_pointlist.size(); ++nEnd)
		{		
			if(nEnd-nStart+1>=m_nMutilCount)	//查询
			{
				GetMutilOPCData(nStart,nEnd,m_pointlist);
				nStart = nEnd;
				Sleep(m_nSleep);
			}
		}

		if(nStart != nEnd)
		{
			GetMutilOPCData(nStart,m_pointlist.size(),m_pointlist);
			Sleep(m_nSleep);
		}
		CString strLog;
		strLog.Format(_T("OPC Engine(%d):Read OPCValue num:%d Response num:%d.\n"),m_nOPCClientIndex,m_nReadCount,m_nReSponse);
		OutErrLog(strLog.GetString(),true);
		//_tprintf(L"OPC Engine(%d):Read OPCValue num:%d Response num:%d).\r\n",m_nOPCClientIndex,m_nReadCount,m_nReSponse);
		return true;
	}
	else
	{
		
	}
	return false;
}

bool COPCEngine::GetMutilOPCData( int nStart, int nEnd, vector<OPCDataPointEntry>& pointlist )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(nStart>nEnd || nEnd>pointlist.size() || pointlist.size()<=0)
		return false;
	if(m_bExitThread || !m_bConnectOK)
		return false;
	map<wstring,OPC_VALUE_INFO> mapItem;
	for (unsigned int i = nStart; i < nEnd; i++)
	{
		OPCDataPointEntry& entry = pointlist[i];
		if(entry.GetOPCDataSourceType() == 3)
			continue;
		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();
		m_nReadCount++;
		wstring strValue = L"";

		OPC_VALUE_INFO info;
		info.strOPCName = pointname;
		info.strOPCValue = strValue;
		info.dMultiple = entry.GetMultiple();
		info.nIndex = i;
		info.nPosition = _wtoi(entry.GetParam(7).c_str());
		info.vType = entry.GetOpcPointType();
		mapItem[pointname] = info;
	}
	if(mapItem.size() <= 0)
		return false;

	//查询
	bool bresult = m_opcsession->GetMutilItemValue(mapItem);
	if(!bresult)
	{
		map<wstring,OPC_VALUE_INFO>::iterator iter = mapItem.begin();
		if(iter != mapItem.end())
		{
			CString strLog;
			strLog.Format(_T("OPC Engine(%d):Read MutilValue num %d(%s) Failed.\n"),m_nOPCClientIndex,mapItem.size(),iter->first.c_str());
			OutErrLog(strLog.GetString());
			//_tprintf(L"OPC Engine(%d):Read MutilValue num %d(%s) Failed.\r\n",m_nOPCClientIndex,mapItem.size(),iter->first.c_str());
		}
		return false;
	}
	else
	{
		m_oleUpdateTime = COleDateTime::GetCurrentTime(); 
		map<wstring,OPC_VALUE_INFO>::iterator iter = mapItem.begin();
		while(iter != mapItem.end())
		{
			wstring strOPCName = iter->first;
			int nIndex = iter->second.nIndex;
			if(nIndex<0 || nIndex>=pointlist.size())
			{
				++iter;
			}
			OPCDataPointEntry& entry = pointlist[nIndex];
			if(iter->second.bUpdate && (iter->second.vType == VT_BSTR || iter->second.strOPCValue.length()>0))
			{
				entry.SetUpdated();
				double fValueRead = 0.f;
				if(fabs(iter->second.dMultiple)<1e-10)//防止除零 
					fValueRead = _wtof(iter->second.strOPCValue.c_str());
				else
					fValueRead = _wtof(iter->second.strOPCValue.c_str())/iter->second.dMultiple;
				if(iter->second.nPosition>0)  //取位
				{
					m_PositionValuemap[entry.GetShortName()] = fValueRead;
					fValueRead = GetBitFromFloat(fValueRead,iter->second.nPosition);
				}
				entry.SetValue(fValueRead);
				if(entry.GetOpcPointType() == VT_BSTR)
				{
					entry.SetSValue(iter->second.strOPCValue);
				}
				m_nReSponse++;
			}
			else
			{
				//entry.SetValue(0.0);
				CString strLog;
				strLog.Format(_T("OPC Engine(%d):Read MutilValue(%s) Failed.\n"),m_nOPCClientIndex,entry.GetShortName().c_str());
				OutErrLog(strLog.GetString());
			}
			++iter;
		}

		CString strLog;
		strLog.Format(_T("OPC Engine(%d):Read MutilValue %s(%d-%d) Success.\n"),m_nOPCClientIndex,pointlist[nStart].GetShortName().c_str(),nStart,nEnd);
		OutErrLog(strLog.GetString(),true);
	}
	return true;
}

void COPCEngine::SetDebug( int nDebug ,int nOPCCheckReconnectInterval,int nOPCReconnectInertval,int nReconnectMinute,bool bEnableSecurity,int nOPCMainThreadOPCPollSleep)
{
	m_nOPCReconnectInertval = nOPCReconnectInertval;
	m_nOPCCheckReconnectInterval = nOPCCheckReconnectInterval;
	m_nReconnectMinute = nReconnectMinute;
	m_bEnableSecurity = bEnableSecurity;
	m_nOPCMainThreadOPCPollSleep = nOPCMainThreadOPCPollSleep;
	m_nDebugOPC = nDebug;
	if(m_opcsession)
		return m_opcsession->SetDebug(nDebug);
}

UINT WINAPI COPCEngine::ThreadConnectOPCData( LPVOID lparam )
{
	COPCEngine* pthis = (COPCEngine*) lparam;
	if (pthis != NULL)
	{
		while (!pthis->m_bExitThread)
		{
			if(!pthis->m_bEnableSecurity)
				pthis->ReConnectOPC();	
			int nSleep = pthis->m_nOPCCheckReconnectInterval*60;
			while(!pthis->m_bExitThread)
			{
				nSleep--;
				if(nSleep%60 == 0 && !pthis->m_bEnableSecurity)		//未启用安全策略
					pthis->ReLoadItemHandle();
				if(nSleep < 0)
					break;
				Sleep(1000);
			}
		}
	}

	return 0;
}

bool COPCEngine::ReConnectOPC()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleUpdateTime;
	if(oleSpan.GetTotalMinutes() >= m_nOPCReconnectInertval)
	{
		m_opcsession->SetPointList(m_pointlist);
		m_bConnectOK = m_opcsession->ReConnectOPC();
		CString strLog;
		if(m_bConnectOK)
		{
			m_nSingleReadCount = 0;
			m_oleReConnectTime = COleDateTime::GetCurrentTime();
			m_oleUpdateTime = m_oleReConnectTime;
			strLog = _T("ReConnectOPC Success.\n");
			OutErrLog(strLog.GetString(),true);
		}
		else
		{
			strLog = _T("ReConnectOPC Fail.\n");
			OutErrLog(strLog.GetString());
		}
	}
	return false;
}

bool COPCEngine::UpdateSingleOPCData()
{
	m_nSingleReadCount++;
	if(m_nSingleReadCount >= 10000)
		m_nSingleReadCount = 0;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
		OPCDataPointEntry& entry = m_pointlist[i];
		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();
		m_nReadCount++;

		if(m_bConnectOK)	//连接成功
		{
			if(m_opcsession)
			{
				CKItem* pItem = m_opcsession->FindItem(entry.GetPlcAddress());
				if(pItem)
				{
					if(pItem->IsValid())
					{
						wstring strValue = L"";
						bool bresult = false;
						VARTYPE type = m_opcsession->GetItemTypeByShortName(pointname);
						CString strType = _T("Undefine");
						bresult = m_opcsession->GetValue(pointname, strValue);
						StringFromVartype(type,strType);

						if (bresult && (type == VT_BSTR || strValue.length()>0))
						{
							entry.SetUpdated();
							m_nReSponse++;
							m_oleUpdateTime = COleDateTime::GetCurrentTime(); 
							double fValueRead = 0.f;
							if(fabs(entry.GetMultiple())<1e-10)//防止除零 
								fValueRead = _wtof(strValue.c_str());
							else
								fValueRead = _wtof(strValue.c_str())/entry.GetMultiple();
							int nPosition = _wtoi(entry.GetParam(7).c_str());
							if(nPosition>0)  //取位
							{
								m_PositionValuemap[pointname] = fValueRead;
								fValueRead = GetBitFromFloat(fValueRead,nPosition);
							}
							entry.SetValue(fValueRead);
							if(entry.GetOpcPointType() == VT_BSTR)
							{
								entry.SetSValue(strValue);
							}
							Sleep(m_nSleep);
						}
						else if(bresult && type != VT_BSTR  && strValue.length() == 0)
						{
							wstring strlog = _T("Error: Get Point ");
							strlog += pointname;
							strlog += _T("(");
							strlog += strType.GetString();
							strlog += _T(") Value Success But Value Is Null.Please Check!\r\n");
							_tprintf(strlog.c_str());
							//存下来
							OutErrLog(strlog);
						}
						else
						{		
							CString strLog_;
							strLog_.Format(_T("Error: Get Point %s(%s) Value Failed(%d).Please Check!\r\n"),pointname.c_str(),strType,m_nSingleReadCount);
							OutErrLog(strLog_.GetString());
							_tprintf(strLog_);
						}
					}
					else		//not valid
					{
						CString strLog_;
						strLog_.Format(_T("ERROR: Get Value %s Fail(Not Valid)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
						OutErrLog(strLog_.GetString());
						_tprintf(strLog_);
					}
				}
				else		//找不到 item
				{
					CString strLog_;
					strLog_.Format(_T("ERROR: Get Value %s Fail(Can't Find)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
					OutErrLog(strLog_.GetString());
					_tprintf(strLog_);
				}
			}
		}
		else				//连接失败
		{
			//CDebugLog::GetInstance()->SetErrCode(ERROR_CUSTOM_OPC_GET_VALUE_FAIL,true);		
		}
	}
	return true;
}

void COPCEngine::OutErrLog( wstring strLog_ ,bool bDaily)
{
	try
	{
		if(bDaily == true && m_nDebugOPC == 0)
			return;

		if(bDaily)
		{
			wstring strPath;
			GetSysPath(strPath);
			strPath += L"\\log";
			if(Project::Tools::FindOrCreateFolder(strPath))
			{
				SYSTEMTIME sNow;
				GetLocalTime(&sNow);
				CString strFilePath;
				strFilePath.Format(_T("%s\\opc_log_%d_%02d_%02d.log"),strPath.c_str(),sNow.wYear,sNow.wMonth,sNow.wDay);

				wstring strTime;
				Project::Tools::SysTime_To_String(sNow,strTime);
				CString strLog;
				strLog.Format(_T("%s::%s"),strTime.c_str(),strLog_.c_str());
				
				char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
				setlocale( LC_ALL, "chs" );
				//记录Log
				CStdioFile	ff;
				if(ff.Open(strFilePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
				{
					ff.Seek(0,CFile::end);
					ff.WriteString(strLog);
					ff.Close();
				}
				setlocale( LC_CTYPE, old_locale ); 
				free( old_locale ); 
			}
		}
		else
		{
			wstring strPath;
			GetSysPath(strPath);
			strPath += L"\\log";
			if(Project::Tools::FindOrCreateFolder(strPath))
			{
				SYSTEMTIME sNow;
				GetLocalTime(&sNow);
				CString strFilePath;
				strFilePath.Format(_T("%s\\opc_err_%d_%02d_%02d.log"),strPath.c_str(),sNow.wYear,sNow.wMonth,sNow.wDay);

				wstring strTime;
				Project::Tools::SysTime_To_String(sNow,strTime);
				CString strLog;
				strLog.Format(_T("%s::%s"),strTime.c_str(),strLog_.c_str());

				char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
				setlocale( LC_ALL, "chs" );
				//记录Log
				CStdioFile	ff;
				if(ff.Open(strFilePath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
				{
					ff.Seek(0,CFile::end);
					ff.WriteString(strLog);
					ff.Close();
				} 
				setlocale( LC_CTYPE, old_locale ); 
				free( old_locale ); 
			}
		}
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}

bool COPCEngine::ReLoadItemHandle()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(m_opcsession)
	{
		return m_opcsession->ReloadErrorItemsHandle();
	}
	return false;
}

double COPCEngine::GetBitFromFloat(float dValue, int nIndex)
{
	WORD dwValue = (WORD)dValue;
	WORD dwTemp = dwValue<<(16-nIndex);
	int nTemp = dwTemp>> 15;
	return (double)nTemp;
}

int COPCEngine::GetBitFromInt(int nValue, int nIndex)
{
	WORD dwValue = (WORD)nValue;
	WORD dwTemp = dwValue<<(16-nIndex);
	int nTemp = dwTemp>> 15;
	return nTemp;
}

wstring COPCEngine::GetBitFromString(wstring strValue, int nIndex)
{
	WORD dwValue = (WORD)_wtoi(strValue.c_str());
	WORD dwTemp = dwValue<<(16-nIndex);
	int nTemp = dwTemp>> 15;
	CString strValue_;
	strValue_.Format(_T("%d"),nTemp);
	return strValue_.GetString();
}

double COPCEngine::SetBitToFloat(double dwData, int nIndex,double dwValue)
{
	nIndex = nIndex-1;
	WORD nData = (WORD)dwData;
	int nValue = (int)dwValue;
	nData &= ~(1<<nIndex); //将*num的第pos位设置为0
	nData |= nValue<<nIndex;
	return (double)nData;
}

wstring COPCEngine::SetBitToString(double dwData, int nIndex,wstring strValue)
{
	nIndex = nIndex-1;
	WORD nData = (WORD)dwData;
	int nValue = _wtoi(strValue.c_str());
	nData &= ~(1<<nIndex); //将*num的第pos位设置为0
	nData |= nValue<<nIndex;
	CString strValue_;
	strValue_.Format(_T("%d"),nData);
	return strValue_.GetString();
}

bool COPCEngine::GetOPCConnectState()
{
	return m_bConnectOK;
}

bool COPCEngine::UpdateOPCDataBuffer()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	m_nSingleReadCount++;
	if(m_nSingleReadCount >= 10000)
		m_nSingleReadCount = 0;

	bool bUpdateSuccess = false;
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
		OPCDataPointEntry& entry = m_pointlist[i];
		wstring pointname = entry.GetShortName();
		entry.SetToUpdate();

		if(m_bConnectOK)
		{
			COleDateTime oleUpdate;
			bool bUpdateTimeAsync20 = false;
			if(m_opcsession->GetUpdateTime20(oleUpdate,bUpdateTimeAsync20))
			{
				m_oleUpdateTime = oleUpdate;
			}
			if(bUpdateTimeAsync20)
			{
				if(m_opcsession)
				{
					CKItem* pItem = m_opcsession->FindItem(entry.GetPlcAddress());
					if(pItem)
					{
						if(pItem->IsValid())			//有效点位才去获取值
						{
							entry.SetUpdated();

							VARTYPE type = pItem->GetDataType();
							CString strType = _T("Undefine");
							StringFromVartype(type,strType);
							CString strTemp = _T("");
							pItem->GetValue(strTemp);
							wstring strValue = strTemp.GetString();
							if (type == VT_BSTR || strValue.length()>0)
							{
								double fValueRead = 0.f;
								if(fabs(entry.GetMultiple())<1e-10)//防止除零 
									fValueRead = _wtof(strValue.c_str());
								else
									fValueRead = _wtof(strValue.c_str())/entry.GetMultiple();
								int nPosition = _wtoi(entry.GetParam(7).c_str());
								if(nPosition>0)  //取位
								{
									m_PositionValuemap[pointname] = fValueRead;
									fValueRead = GetBitFromFloat(fValueRead,nPosition);
								}
								entry.SetValue(fValueRead);
								if(entry.GetOpcPointType() == VT_BSTR)
								{
									entry.SetSValue(strValue);
								}
								bUpdateSuccess = true;
							}
							else if(type != VT_BSTR  && strValue.length() == 0)
							{
								CString strLog;
								strLog.Format(_T("ERROR:Get Point %s Value Success But Value Is Null.Please Check!\r\n"),pointname.c_str());

								_tprintf(strLog);
								//存下来
								OutErrLog(strLog.GetString());
							}
						}
						else
						{
							CString strLog_;
							strLog_.Format(_T("ERROR: Get Value %s Fail(Not Valid)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
							OutErrLog(strLog_.GetString());
							_tprintf(strLog_);
						}
					}
					else
					{
						CString strLog_;
						strLog_.Format(_T("ERROR: Get Value %s Fail(Can't Find)(%d).Please Check!\r\n"),pointname.c_str(),m_nSingleReadCount);
						OutErrLog(strLog_.GetString());
						_tprintf(strLog_);
					}
				}
			}
		}
		else
		{

		}
	}
	return bUpdateSuccess;
}

bool COPCEngine::Refresh20( bool bDeviceRead /*= false*/ )
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock);
	if(m_bConnectOK && m_opcsession)
	{
		return m_opcsession->RefreshAsync20(false);
	}
	return true;
}
