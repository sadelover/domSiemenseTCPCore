#include "stdafx.h"
#include "WebHttpCtrl.h"
#include <iostream>
#include "Tools/json/json.h"
#include "Tools/json/reader.h"
#include "Tools/json/value.h"
#include "Tools/EngineInfoDefine.h"
using namespace Json;

#ifdef _DEBUG
#pragma comment(lib,"json_vc71_libmtd.lib")
#else
#pragma comment(lib,"json_vc71_libmt.lib")
#endif

CWebHttpCtrl::CWebHttpCtrl( Beopdatalink::CCommonDBAccess*	dbsession_history,int nDurationInMinutes,int nUpdateInterval,int nActiveMinutes,string strSubscriptionType,bool bChangesOnly,bool bDeleteSubcription,wstring strSubcriptions )
	: m_bExitThread(false)
	, m_nUpdateInterval(nUpdateInterval)
	, m_strUser(_T(""))
	, m_strHost(_T(""))
	, m_strPsw(_T(""))
	, m_strToken("")
	, m_nDurationInMinutes(nDurationInMinutes)
	, m_strSubscriptionType(strSubscriptionType)
	, m_nActiveMinutes(nActiveMinutes)
	, m_bChangesOnly(bChangesOnly)
	, m_bDeleteSubcription(bDeleteSubcription)
	, m_strSubcriptions(strSubcriptions)
	, m_nEngineIndex(0)
	, m_pConnection(NULL)
	, m_hSendCmd(NULL)
	, m_nInvalidResponseCount(3)
	, m_nTimeOutCount(0)
	, m_bCreateSubcription(false)
	, m_dbsession_history(dbsession_history)
	, m_nCmdCount(0)
	, m_nResponseCount(0)
	, m_nUpdatePointCount(0)
	, m_strUpdateLog("")
{
	m_oleUpdateTime = COleDateTime::GetCurrentTime();
	m_mapSubcription.clear();
}

CWebHttpCtrl::~CWebHttpCtrl( void )
{

}

bool CWebHttpCtrl::Init()
{
	if (m_pointlist.empty())
	{
		return false;
	}
	wstring strServerhost = m_pointlist[0].GetParam(3);
	vector<wstring> vecParam;
	Project::Tools::SplitStringByChar(strServerhost.c_str(),L':',vecParam);
	if(vecParam.size() == 2)			//IP/端口
	{
		m_strHost = vecParam[0].c_str();
		m_nPort = _wtoi(vecParam[1].c_str());
	}
	else if(vecParam.size() == 1)		//IP:80
	{
		m_strHost = vecParam[0].c_str();
		m_nPort = 80;
	}

	m_strUser = m_pointlist[0].GetParam(5).c_str();
	m_strPsw  = m_pointlist[0].GetParam(6).c_str();

	if(m_strHost.GetLength()<= 0 || m_strUser.GetLength()<= 0 || m_strPsw.GetLength()<= 0)
	{
		PrintLog(L"ERROR: WEB Param Valid!");
		return false;
	}

	bool bConnectOk = false;
	bConnectOk = CreateConnection(m_strHost,m_nPort);
	m_hSendCmd = (HANDLE) _beginthreadex(NULL, 0, ThreadSendCommandsFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	return bConnectOk;
}

bool CWebHttpCtrl::Exit()
{
	m_bExitThread = true;
	if(m_hSendCmd)
		WaitForSingleObject(m_hSendCmd,2000);
	if(m_hSendCmd)
	{
		CloseHandle(m_hSendCmd);
		m_hSendCmd = NULL;
	}
	return true;
}

void CWebHttpCtrl::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

UINT WINAPI CWebHttpCtrl::ThreadSendCommandsFunc( LPVOID lparam )
{
	CWebHttpCtrl* pthis = (CWebHttpCtrl*)lparam;
	if (pthis != NULL)
	{
		if(!pthis->m_bExitThread)
			pthis->GenerateSubcription();
		if(!pthis->m_bExitThread)
			pthis->DeletAllSuccription();			//删除所有以前订阅	
		while (!pthis->m_bExitThread)
		{
			pthis->SendSuccriptions();	
			pthis->SaveSuccriptions();
			int nPollSleep = pthis->m_nUpdateInterval;
			while (!pthis->m_bExitThread)
			{
				if(nPollSleep <= 0)
				{
					break;
				}
				nPollSleep--;
				Sleep(1000);
			}
		}
	}
	return 0;
}

bool CWebHttpCtrl::GenerateSubcription()
{
	m_mapSubcription.clear();
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		string strSubcriptionID = Project::Tools::WideCharToUtf8(m_pointlist[i].GetParam(2).c_str());
		string strPointAddress = Project::Tools::WideCharToUtf8(m_pointlist[i].GetParam(1).c_str());
		wstring wstrPointName = m_pointlist[i].GetShortName();
		hash_map<string,SubscriptionInfo>::iterator iterSubcription = m_mapSubcription.find(strSubcriptionID);
		if(iterSubcription != m_mapSubcription.end())			//找到订阅号
		{
			(*iterSubcription).second.mapSubPointAddress.insert(make_pair(strPointAddress,0));
			(*iterSubcription).second.mapSubPoint.insert(make_pair(wstrPointName,&m_pointlist[i]));
		}
		else				//没找到添加
		{
			SubscriptionInfo sub;
			sub.strSubscriptionID = strSubcriptionID;
			sub.mapValue.clear();
			sub.mapSubPointAddress.insert(make_pair(strPointAddress,0));
			sub.mapSubPoint.insert(make_pair(wstrPointName,&m_pointlist[i]));
			m_mapSubcription[strSubcriptionID] = sub;
		}
	}

	hash_map<string,SubscriptionInfo>::iterator  iterSubcription = m_mapSubcription.begin();
	while(iterSubcription != m_mapSubcription.end())
	{
		if((*iterSubcription).second.mapSubPointAddress.size() >2000)
		{
			CString strLog;
			strLog.Format(_T("WARNING: WEB Subcription(%s) Too Big!\r\n"),Project::Tools::AnsiToWideChar((*iterSubcription).second.strSubscriptionID.c_str()).c_str());
			PrintLog(strLog.GetString(),true);
		}
		++iterSubcription;
	}

	return true;
}

bool CWebHttpCtrl::SendSuccriptions()
{
	SumReadAndResponse();
	for(int i=0; i<m_pointlist.size(); ++i)
	{
		m_nCmdCount++;
	}

	hash_map<string,SubscriptionInfo>::iterator  iterSubcription = m_mapSubcription.begin();
	while(iterSubcription != m_mapSubcription.end())
	{
		SendOneSuccription((*iterSubcription).second);
		++iterSubcription;
	}

	return true;
}

bool CWebHttpCtrl::SendOneSuccription( SubscriptionInfo& subcription)
{
	if(GetToken())
	{
		CString strLog;
		if(subcription.strSubscriptionID_Active.length() <= 0)			//订阅ID为空  需要先添加订阅
		{
			if(CreateOneSuccription(subcription) == false)				//创建订阅
			{
				strLog.Format(_T("ERROR: Create Succription(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
				PrintLog(strLog.GetString(),true);
				return false;
			}
			else
			{
				m_bCreateSubcription = true;
			}
		}

		//创建消息时候 根据是否值变化更新 来创建消息
		if(subcription.bUpdateAll == false)			//建立返回全部ID
		{
			if(subcription.strSubscriptionID_Active.length() > 0)			//订阅ID不为空
			{
				if(subcription.strNotificationIDAll_Active.length() <= 0)		//消息ID为空  需要先添加消息
				{
					if(CreateOneNotificationAll(subcription) == false)			//创建消息
					{
						strLog.Format(_T("ERROR: Create NotificationAll(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
						PrintLog(strLog.GetString(),true);
						return false;
					}
					else
					{
						m_bCreateSubcription = true;
					}
				}
			}
		}
		else
		{
			if(subcription.strSubscriptionID_Active.length() > 0)			//订阅ID不为空
			{
				if(subcription.strNotificationIDAll_Active.length() > 0)		//删除订阅的全部消息
				{
					if(DeleteOneNotificationAll(subcription) == false)			//删除订阅的全部消息
					{
						strLog.Format(_T("ERROR: Delete NotificationAll(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
						PrintLog(strLog.GetString(),true);
						return false;
					}
				}

				if(subcription.strNotificationID_Active.length() <= 0)		//消息ID为空  需要先添加消息
				{
					if(CreateOneNotification(subcription) == false)			//创建消息
					{
						strLog.Format(_T("ERROR: Create Notification(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
						PrintLog(strLog.GetString(),true);
						return false;
					}
					else
					{
						m_bCreateSubcription = true;
					}
				}
			}
		}

		if(subcription.bActive == false)								//订阅失效 需删除该订阅 同时创建新订阅
		{	
			DeleteOneSuccription(subcription);

			if(CreateOneSuccription(subcription) == false)	
			{
				strLog.Format(_T("ERROR: Create Succription(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
				PrintLog(strLog.GetString(),true);
				return false;
			}
			else
			{
				m_bCreateSubcription = true;
			}

			if(subcription.strSubscriptionID_Active.length() > 0)			//订阅ID不为空
			{
				if(subcription.strNotificationIDAll_Active.length() <= 0)		//消息ID为空  需要先添加消息
				{
					if(CreateOneNotificationAll(subcription) == false)			//创建消息
					{
						strLog.Format(_T("ERROR: Create NotificationAll(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
						PrintLog(strLog.GetString(),true);
						return false;
					}
					else
					{
						m_bCreateSubcription = true;
					}
				}
			}

			//创建消息时候 根据是否值变化更新 来创建消息
			if(subcription.bUpdateAll == false)			//建立返回全部ID
			{
				if(subcription.strSubscriptionID_Active.length() > 0)			//订阅ID不为空
				{
					if(subcription.strNotificationIDAll_Active.length() <= 0)		//消息ID为空  需要先添加消息
					{
						if(CreateOneNotificationAll(subcription) == false)			//创建消息
						{
							strLog.Format(_T("ERROR: Create NotificationAll(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
							PrintLog(strLog.GetString(),true);
							return false;
						}
						else
						{
							m_bCreateSubcription = true;
						}
					}
				}
			}
			else
			{
				if(subcription.strSubscriptionID_Active.length() > 0)			//订阅ID不为空
				{				
					if(subcription.strNotificationID_Active.length() <= 0)		//消息ID为空  需要先添加消息
					{
						if(CreateOneNotification(subcription) == false)			//创建消息
						{
							strLog.Format(_T("ERROR: Create Notification(%d) Fail!"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
							PrintLog(strLog.GetString(),true);
							return false;
						}
						else
						{
							m_bCreateSubcription = true;
						}
					}
				}
			}
		}
		else															//订阅有效
		{
			if(ActiveOneSuccription(subcription) == false)				//激活续订订阅
			{
				strLog.Format(_T("ERROR: Active Succription(%d) Fail!\r\n"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
				PrintLog(strLog.GetString(),true);
				return false;
			}
		}

		if(PollOneSuccription(subcription) == false)								//轮询订阅值
		{
			strLog.Format(_T("ERROR: Poll Succriptio(%d) Fail!\r\n"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
			PrintLog(strLog.GetString(),true);
			return false;
		}

		//删除订阅点
		if(subcription.strNotificationID_Active.length() > 0)		//删除订阅的全部消息
		{
			if(DeleteOneNotification(subcription) == false)			//删除订阅的全部消息
			{
				strLog.Format(_T("ERROR: Delete Notification(%d) Fail!\r\n"),Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
				PrintLog(strLog.GetString(),true);
				return false;
			}
		}
	}
	else
	{
		PrintLog(L"ERROR:WebHttp GetToken Fail!",true);
	}
}

bool CWebHttpCtrl::CreateOneSuccription( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			Json::Value root;
			root["DurationInMinutes"] = m_nDurationInMinutes;
			root["SubscriptionType"] = m_strSubscriptionType;
			hash_map<string,int>::iterator iterPoint = subcription.mapSubPointAddress.begin();
			while(iterPoint != subcription.mapSubPointAddress.end())
			{
				root["Ids"].append((*iterPoint).first);
				iterPoint++;
			}
			string strData_Utf8 = root.toStyledString();

			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,TEXT("/Subscriptions/Create"));
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			std::ostringstream sqlstream;
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,(LPVOID *)strData_Utf8.c_str(), strData_Utf8.length()))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return JsonParseCreateSuccription(strHtml,subcription);
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::CreateOneNotification( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			Json::Value root;
			root["SubscriptionId"] = subcription.strSubscriptionID_Active;
			root["ChangesOnly"] = m_bChangesOnly;//m_bChangesOnly;
			string strData_Utf8 = root.toStyledString();

			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,TEXT("/Notifications/Create"));
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			std::ostringstream sqlstream;
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,(LPVOID *)strData_Utf8.c_str(), strData_Utf8.length()))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return JsonParseCreateNotification(strHtml,subcription);
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::DeleteOneSuccription( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			sqlstream << "/Subscriptions/" << subcription.strSubscriptionID_Active << "/Delete";
			string strData_Utf8 = sqlstream.str();
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_DELETE,Project::Tools::UTF8ToWideChar(strData_Utf8.c_str()).c_str());
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			sqlstream.str("");
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,NULL,0))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				subcription.strSubscriptionID_Active = "";
				subcription.strNotificationID_Active = "";
				subcription.strNotificationIDAll_Active = "";
				subcription.bUpdateAll = false;

				m_nInvalidResponseCount = 0;
				return true;
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::DeleteOneSuccription( string strSubcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			sqlstream << "/Subscriptions/" << strSubcription << "/Delete";
			string strData_Utf8 = sqlstream.str();
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_DELETE,Project::Tools::UTF8ToWideChar(strData_Utf8.c_str()).c_str());
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			sqlstream.str("");
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,NULL,0))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return true;
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::DeletAllSuccription( )
{
	if(GetToken())
	{
		vector<wstring> vecSuccription;
		if(m_strSubcriptions.length() > 0)
		{
			Project::Tools::SplitStringByChar(m_strSubcriptions.c_str(),L',',vecSuccription);
			for(int i=0; i<vecSuccription.size(); ++i)
			{
				if(vecSuccription[i].length()>0)
					DeleteOneSuccription(Project::Tools::WideCharToUtf8(vecSuccription[i].c_str()));
			}
		}
		return true;
	}
	return false;
}

bool CWebHttpCtrl::GetAllSuccription(vector<string>& vecSuccription)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,TEXT("/Subscriptions"));
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			sqlstream.str("");
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,NULL,0))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return JsonParseAllSuccription(strHtml,vecSuccription);
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::ActiveOneSuccription( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			sqlstream << "/Subscriptions/" << subcription.strSubscriptionID_Active << "/Renew";
			string strPath_Utf8 = sqlstream.str();
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_PUT,Project::Tools::UTF8ToWideChar(strPath_Utf8.c_str()).c_str());
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			sqlstream.str("");
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());

			sqlstream.str("");
			sqlstream << m_nActiveMinutes;
			string strData_Utf8 = sqlstream.str();
			if(pFile != NULL && pFile->SendRequest(NULL,0,(LPVOID *)strData_Utf8.c_str(), strData_Utf8.length()))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return JsonParseActiveSuccription(strHtml,subcription);
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::PollOneSuccription( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			bool bNotificationAll = false;
			if(subcription.strNotificationIDAll_Active.length() > 0)
			{
				bNotificationAll = true;
				sqlstream << "/Notifications/" << subcription.strNotificationIDAll_Active << "/Items?take=" << subcription.mapSubPointAddress.size()+1;
			}
			else
			{
				sqlstream << "/Notifications/" << subcription.strNotificationID_Active << "/Items?take=" << subcription.mapSubPointAddress.size()+1;
			}
			string strUrl_Utf8 = sqlstream.str();
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,Project::Tools::UTF8ToWideChar(strUrl_Utf8.c_str()).c_str());
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			sqlstream.str("");
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,NULL,0))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return JsonParsePollSuccription(strHtml,subcription,bNotificationAll);
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::GetToken()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			sqlstream << "grant_type=password&username=" << Project::Tools::WideCharToAnsi(m_strUser) << "&password=" << Project::Tools::WideCharToAnsi(m_strPsw);
			string strData_Utf8 = sqlstream.str();
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,TEXT("/GetToken"));
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			if(pFile != NULL && pFile->SendRequest(NULL,0,(LPVOID *)strData_Utf8.c_str(), strData_Utf8.length()))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return JsonParseToken(strHtml);
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::CreateDefaultConnection()
{
	if(m_strHost.GetLength() >0 && m_nPort>0)
	{
		if(m_pConnection == NULL)
		{
			try
			{				
				m_pConnection = m_Isession.GetHttpConnection((LPCTSTR)m_strHost,m_nPort);
			}
			catch (CInternetException * e)
			{
				if(e->m_dwError == 12029 || e->m_dwError == 6)
				{
					CloseConnection();
				}
				PrintLog(L"ERROR: ReConnect WEB Failed",true);
				return false;
			}
		}
	}
	return false;
}

bool CWebHttpCtrl::CloseConnection()
{
	if(m_pConnection != NULL)
	{
		m_pConnection->Close();
		delete m_pConnection;
		m_pConnection = NULL;
	}
	return true;
}

bool CWebHttpCtrl::CreateConnection( CString strHost, INTERNET_PORT nPort )
{
	if(m_pConnection == NULL)
	{
		try
		{
			m_Isession.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000 * 5);
			m_Isession.SetOption(INTERNET_OPTION_SEND_TIMEOUT,3000);//发送请求的超时时间
			m_Isession.SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 1000);
			m_Isession.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);

			m_pConnection = m_Isession.GetHttpConnection((LPCTSTR)strHost,nPort);
			m_strHost = strHost;
			m_nPort = nPort;
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			PrintLog(L"ERROR: Connect WEB Failed.",true);
			return false;
		}
	}
	return true;
}

bool CWebHttpCtrl::JsonParseToken( string strJson )
{
	string strJson_Utf8 = Project::Tools::MultiByteToUtf8(strJson.c_str());
	Reader reader;
	Value value;
	if (reader.parse(strJson, value))
	{
		Value arrayObj = value;
		if(!arrayObj["access_token"].isNull() && arrayObj["access_token"].isString())
		{
			m_strToken = arrayObj["access_token"].asString();
			return true;
		}
	}
	return false;
}

bool CWebHttpCtrl::JsonParseCreateSuccription( string strJson ,SubscriptionInfo& subcription)
{
	string strJson_Utf8 = Project::Tools::MultiByteToUtf8(strJson.c_str());
	Reader reader;
	Value value;
	if (reader.parse(strJson, value))
	{
		Value arrayObj = value;
		if(!arrayObj["IsActive"].isNull() && arrayObj["IsActive"].isBool())
		{
			subcription.bActive = arrayObj["IsActive"].asBool();
		}

		if(!arrayObj["Id"].isNull() && arrayObj["Id"].isString())
		{
			subcription.strSubscriptionID_Active = arrayObj["Id"].asString();
		}

		if(subcription.strSubscriptionID_Active.length() >0 && subcription.bActive)
		{
			//创建成功将属性置位
			subcription.strNotificationID_Active = "";
			subcription.strNotificationIDAll_Active = "";
			subcription.bUpdateAll = m_bChangesOnly;			//true表示更新全部
			return true;
		}
	}
	return false;
}

bool CWebHttpCtrl::JsonParseActiveSuccription( string strJson,SubscriptionInfo& subcription )
{
	string strJson_Utf8 = Project::Tools::MultiByteToUtf8(strJson.c_str());
	Reader reader;
	Value value;
	if (reader.parse(strJson, value))
	{
		Value arrayObj = value;
		if(!arrayObj["IsActive"].isNull() && arrayObj["IsActive"].isBool())
		{
			subcription.bActive = arrayObj["IsActive"].asBool();
		}

		if(!arrayObj["Id"].isNull() && arrayObj["Id"].isString())
		{
			subcription.strSubscriptionID_Active = arrayObj["Id"].asString();
		}

		if(subcription.strSubscriptionID_Active.length() >0 && subcription.bActive)
			return true;
	}
	return false;
}

bool CWebHttpCtrl::JsonParseCreateNotification( string strJson,SubscriptionInfo& subcription )
{
	string strJson_Utf8 = Project::Tools::MultiByteToUtf8(strJson.c_str());
	Reader reader;
	Value value;
	if (reader.parse(strJson, value))
	{
		Value arrayObj = value;
		if(!arrayObj["SubscriptionId"].isNull() && arrayObj["SubscriptionId"].isString())
		{
			if(subcription.strSubscriptionID_Active == arrayObj["SubscriptionId"].asString())
			{
				if(!arrayObj["Id"].isNull() && arrayObj["Id"].isString())
				{
					subcription.strNotificationID_Active = arrayObj["Id"].asString();
					return true;
				}
			}
		}
	}
	return false;
}

bool CWebHttpCtrl::JsonParsePollSuccription( string strJson,SubscriptionInfo& subcription,bool bNotificationAll )
{
	subcription.mapValue.clear();
	string strJson_Utf8 = Project::Tools::MultiByteToUtf8(strJson.c_str());
	Reader reader;
	Value value;
	if (reader.parse(strJson, value))
	{
		Value arrayObj = value;
		bool bResult = false;
		if(arrayObj.isArray())
		{
			bResult = true;
			for(int i=0; i<arrayObj.size(); i++)
			{
				string strName_Utf8,strValue_Utf8;
				if(!arrayObj[i]["ChangedItemId"].isNull() && arrayObj[i]["ChangedItemId"].isString())
				{
					strName_Utf8 = arrayObj[i]["ChangedItemId"].asString();
				}

				if(!arrayObj[i]["Value"].isNull() && arrayObj[i]["Value"].isString())
				{
					strValue_Utf8 = arrayObj[i]["Value"].asString();
				}

				if(strName_Utf8.length()>0)
				{
					m_nResponseCount++;
					subcription.mapValue[strName_Utf8] = strValue_Utf8;
				}
			}
		}		

		//赋值
		int nUpdate = 0;
		if(subcription.mapValue.size() > 0)
		{
			hash_map<wstring,DataPointEntry*>::iterator itertPoint;
			for(itertPoint = subcription.mapSubPoint.begin();itertPoint != subcription.mapSubPoint.end(); ++itertPoint)
			{
				DataPointEntry* saveData = (*itertPoint).second;
				if(saveData)
				{
					string strPointAddress = Project::Tools::WideCharToUtf8(saveData->GetParam(1).c_str());
					hash_map<string,string>::iterator iterValue = subcription.mapValue.find(strPointAddress);
					if(iterValue != subcription.mapValue.end())
					{
						wstring strValue;
						Project::Tools::UTF8ToWideChar((*iterValue).second,strValue);
						if(strValue != saveData->GetSValue())
							nUpdate++;
						saveData->SetSValue(strValue);
						saveData->SetUpdated();
						m_nUpdatePointCount++;
					}
				}
			}
		}

		if(bNotificationAll)
		{
			if(!bResult)
			{
				CString strLog;
				strLog.Format(_T("ERROR: WEB Engine(%d):PollSuccriptionAll(%s) Fail,Please Check!\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
				PrintLog(strLog.GetString(),true);
			}
			else
			{
				CString strLog;
				strLog.Format(_T("INFO : WEB Engine(%d):SuccriptionAll(%s) Return(%d) Change(%d) Total(%d)\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str(),subcription.mapValue.size(),nUpdate,subcription.mapSubPoint.size());
				PrintLog(strLog.GetString(),false);
			}
		}
		else
		{
			if(!bResult)
			{
				CString strLog;
				strLog.Format(_T("ERROR: WEB Engine(%d):PollSuccription(%s) Fail,Please Check!\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str());
				PrintLog(strLog.GetString(),true);
			}
			else
			{
				CString strLog;
				strLog.Format(_T("INFO : WEB Engine(%d):Succription(%s) Return(%d) Change(%d) Total(%d)\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str(),subcription.mapValue.size(),nUpdate,subcription.mapSubPoint.size());
				PrintLog(strLog.GetString(),false);
			}
		}

		if(bResult)
		{
			if(bNotificationAll)
			{
				subcription.bUpdateAll = true;
			}
		}
		return bResult;
	}
	return false;
}

bool CWebHttpCtrl::JsonParseAllSuccription( string strJson ,vector<string>& vecSuccription)
{
	string strJson_Utf8 = Project::Tools::MultiByteToUtf8(strJson.c_str());
	Reader reader;
	Value value;
	if (reader.parse(strJson, value))
	{
		Value arrayObj = value;
		if(arrayObj.isArray())
		{
			for(int i=0; i<arrayObj.size(); i++)
			{
				if(!arrayObj[i]["Id"].isNull() && arrayObj[i]["Id"].isString())
				{
					vecSuccription.push_back(arrayObj[i]["Id"].asString());
				}
			}
			return true;
		}
	}
	return false;
}

void CWebHttpCtrl::GetValueSet( vector< pair<wstring, wstring> >& valuelist )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		DataPointEntry  entry = m_pointlist[i];
		if(entry.GetUpdateSignal()<10)				//连续5次没有读到数据将被认为丢失，不再计入实时表。
		{
			if(entry.GetParam(4).length() > 0)		//有倍率问题
			{
				wstring strValueSet = Project::Tools::RemoveDecimalW(entry.GetMultipleReadValue(entry.GetValue()));
				valuelist.push_back(make_pair(entry.GetShortName(),strValueSet));
			}
			else
			{
				wstring strValueSet = entry.GetSValue();
				valuelist.push_back(make_pair(entry.GetShortName(), strValueSet));
			}
		}
	}

	if(m_pointlist.size() >0 )
	{
		wstring strUpdateTime;
		Project::Tools::OleTime_To_String(m_oleUpdateTime,strUpdateTime);

		CString strName;
		strName.Format(_T("%s%d"),TIME_UPDATE_WEBHTTP,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), strUpdateTime));
		strName.Format(_T("%s%d"),LOG_WEBHTTP,m_nEngineIndex);
		valuelist.push_back(make_pair(strName.GetString(), Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str())));
	}
}

void CWebHttpCtrl::SetIndex( int nIndex )
{
	m_nEngineIndex = nIndex;
}

bool CWebHttpCtrl::SaveSuccriptions()
{
	if(m_bCreateSubcription)
	{
		std::ostringstream sqlstream;
		hash_map<string,SubscriptionInfo>::iterator  iterSubcription = m_mapSubcription.begin();
		while(iterSubcription != m_mapSubcription.end())
		{
			OutPutWebServerInfo((*iterSubcription).second);
			sqlstream << (*iterSubcription).second.strSubscriptionID_Active << ",";
			++iterSubcription;
		}
		string strSubcriptions = sqlstream.str();
		if(strSubcriptions.length() > 0)
		{
			strSubcriptions.erase(--strSubcriptions.end());
		}
		m_strSubcriptions = Project::Tools::AnsiToWideChar(strSubcriptions.c_str());
		if(m_dbsession_history)
		{
			m_dbsession_history->WriteCoreDebugItemValue(L"webSubcriptions",m_strSubcriptions);
			//存进数据库里
			m_bCreateSubcription = false;
			return true;
		}
	}
	return false;
}

void CWebHttpCtrl::SumReadAndResponse()
{
	if(m_nCmdCount > 0)			//统计信息
	{
		std::ostringstream sqlstream;
		m_nResponseCount = (m_nResponseCount>m_nCmdCount)?m_nUpdatePointCount:m_nResponseCount;
		sqlstream << "Read(" << m_nCmdCount << ") Response(" << m_nResponseCount << ") UpdatePoint(" << m_nUpdatePointCount << ")";
		m_strUpdateLog = sqlstream.str();

		CString strOut;
		strOut.Format(_T("INFO : WEB Engine(%d):%s.\r\n"),m_nEngineIndex,Project::Tools::AnsiToWideChar(m_strUpdateLog.c_str()).c_str());
		PrintLog(strOut.GetString(),false);
	}
	m_nCmdCount = 0;
	m_nResponseCount = 0;
	m_nUpdatePointCount = 0;
}

bool CWebHttpCtrl::OutPutWebServerInfo( SubscriptionInfo& subcription )
{
	wstring strFolderPath;
	GetSysPath(strFolderPath);
	strFolderPath += L"\\log";	 
	if(Project::Tools::FindOrCreateFolder(strFolderPath))
	{
		std::ostringstream sqlstream;
		sqlstream << subcription.strSubscriptionID_Active << "," << subcription.strNotificationID_Active << "," << subcription.strNotificationIDAll_Active << "," << subcription.mapSubPointAddress.size() << "/" << subcription.mapSubPoint.size();

		//写配置文件
		wstring exepath;
		exepath = strFolderPath + L"\\webserver.cnf";
		WritePrivateProfileString(L"webserver", Project::Tools::AnsiToWideChar(subcription.strSubscriptionID.c_str()).c_str(), Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str(), exepath.c_str());
		return true;
	}
	return false;
}

bool CWebHttpCtrl::CreateOneNotificationAll( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			Json::Value root;
			root["SubscriptionId"] = subcription.strSubscriptionID_Active;
			root["ChangesOnly"] = true;		//	boolean If true, all items will be returned. If false, only changes 
			string strData_Utf8 = root.toStyledString();

			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,TEXT("/Notifications/Create"));
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			std::ostringstream sqlstream;
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,(LPVOID *)strData_Utf8.c_str(), strData_Utf8.length()))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				m_nInvalidResponseCount = 0;
				return JsonParseCreateNotificationAll(strHtml,subcription);
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::DeleteOneNotificationAll( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			sqlstream << "/Notifications/" << subcription.strNotificationIDAll_Active << "/Delete";
			string strData_Utf8 = sqlstream.str();
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_DELETE,Project::Tools::UTF8ToWideChar(strData_Utf8.c_str()).c_str());
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			sqlstream.str("");
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,NULL,0))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;
				
				subcription.strNotificationIDAll_Active = "";

				m_nInvalidResponseCount = 0;
				return true;
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}

bool CWebHttpCtrl::JsonParseCreateNotificationAll( string strJson,SubscriptionInfo& subcription )
{
	string strJson_Utf8 = Project::Tools::MultiByteToUtf8(strJson.c_str());
	Reader reader;
	Value value;
	if (reader.parse(strJson, value))
	{
		Value arrayObj = value;
		if(!arrayObj["SubscriptionId"].isNull() && arrayObj["SubscriptionId"].isString())
		{
			if(subcription.strSubscriptionID_Active == arrayObj["SubscriptionId"].asString())
			{
				if(!arrayObj["Id"].isNull() && arrayObj["Id"].isString())
				{
					subcription.strNotificationIDAll_Active = arrayObj["Id"].asString();
					return true;
				}
			}
		}
	}
	return false;
}

void CWebHttpCtrl::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
	if(bSaveLog)
	{
		OutPutLogString(strLog);
	}
}

bool CWebHttpCtrl::OutPutLogString( wstring strOut )
{
	char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
	setlocale( LC_ALL, "chs" );

	CString strLog;
	SYSTEMTIME st;
	GetLocalTime(&st);
	wstring strTime;
	Project::Tools::SysTime_To_String(st,strTime);
	strLog += strTime.c_str();
	strLog += _T(" - ");

	strLog += strOut.c_str();
	strLog += _T("\n");

	wstring strPath;
	GetSysPath(strPath);
	strPath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strPath))
	{
		CString strLogPath;
		strLogPath.Format(_T("%s\\webserver_%d_%02d_%02d.log"),strPath.c_str(),st.wYear,st.wMonth,st.wDay);

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
	}
	setlocale( LC_CTYPE, old_locale ); 
	free( old_locale ); 
	return false;
}

bool CWebHttpCtrl::DeleteOneNotification( SubscriptionInfo& subcription )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_mtuex_http);
	if(m_pConnection == NULL)
	{
		CreateDefaultConnection();
	}

	if(m_pConnection != NULL)
	{		
		try
		{
			//准备参数
			std::ostringstream sqlstream;
			sqlstream << "/Notifications/" << subcription.strNotificationID_Active << "/Delete";
			string strData_Utf8 = sqlstream.str();
			CHttpFile* pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_DELETE,Project::Tools::UTF8ToWideChar(strData_Utf8.c_str()).c_str());
			pFile->AddRequestHeaders(_T("Content-Type: application/json"));
			sqlstream.str("");
			sqlstream << "Authorization: Bearer " << m_strToken;
			pFile->AddRequestHeaders(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()).c_str());
			if(pFile != NULL && pFile->SendRequest(NULL,0,NULL,0))
			{
				string strHtml = "";
				char tChars[1025] = {0};
				while ((pFile->Read((void*)tChars, 1024)) > 0)  
				{  
					strHtml += tChars;  
				}  
				pFile->Close();
				delete pFile;

				subcription.strNotificationID_Active = "";

				m_nInvalidResponseCount = 0;
				return true;
			}
		}
		catch (CInternetException * e)
		{
			if(e->m_dwError == 12029)
			{
				CloseConnection();
			}
			else if(e->m_dwError == 12002)	//超时
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			else if(e->m_dwError == 12152)	//服务器返回无效或无法识别的响应
			{
				m_nInvalidResponseCount++;
				if(m_nInvalidResponseCount >= m_nTimeOutCount)
				{
					m_nInvalidResponseCount = 0;
					CloseConnection();
				}
			}
			return false;
		}		
	}
	return false;
}
