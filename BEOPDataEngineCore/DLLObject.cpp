#include "StdAfx.h"
#include "DLLObject.h"
#include "LogicParameter.h"
#include "LogicParameterConst.h"
#include "LogicParameterLink.h"
#include "LogicParameterPoint.h"


const int LogStoreSize = 500;
const wstring LogicSysVersion = L"V2.3";
CDLLObject::CDLLObject(CBEOPDataAccess *pDataAccess):m_pDataAccess(pDataAccess)
{

	m_pLB			= NULL;
	m_pHinstance	= NULL;
	m_sDllName		= L"";
	m_bDllRun       = true;
	m_pFun			= NULL;
	m_iTimeSpan     = 0;
	m_sImportTime	= L"";
	m_sDllAuthor	= L"";
	m_sDllPathFile  = L"";
	m_sDllVersion	= L"";
	m_sDllDescription	= L"";


	m_bAvailable    = true;
	m_bLoaded		= false;

	m_vecCurDllInputParameterList.clear();
	m_vecCurDllOutputParameterList.clear();

	m_sS3DBPath		= L"";

	m_strLogList.clear();

}
bool CDLLObject::ReSet()
{


	m_pLB			= NULL;
	m_pHinstance	= NULL;
	m_sDllName		= L"";
	m_bDllRun       = true;
	m_pFun			= NULL;
	m_iTimeSpan     = 0;
	m_bLogicActing      = false;
	m_sImportTime	= L"";
	m_sDllAuthor	= L"";
	m_sDllPathFile  = L"";
	m_sDllVersion	= L"";
	m_sDllDescription	= L"";
	//使m_hEventClose恢复为非激发状态

	m_bAvailable    = true;
	m_bLoaded		= false;

	unsigned int i = 0;
	for(i=0;i<m_vecCurDllInputParameterList.size();i++)
	{
		delete(m_vecCurDllInputParameterList[i]);
		m_vecCurDllInputParameterList[i] = NULL;
	}
	for(i=0;i<m_vecCurDllOutputParameterList.size();i++)
	{
		delete(m_vecCurDllOutputParameterList[i]);
		m_vecCurDllOutputParameterList[i] = NULL;
	}

	m_vecCurDllInputParameterList.clear();
	m_vecCurDllOutputParameterList.clear();

	m_strLogList.clear();

	return true;
}
CDLLObject::~CDLLObject(void)
{


}
void CDLLObject::SetMsgWnd(CWnd * MsgWnd)
{
	m_MsgWnd = MsgWnd;

	if(m_pLB)
		m_pLB->SetMsgWnd(MsgWnd);
}

void CDLLObject::SentMsgInfo(LPCTSTR loginfo)
{
	if (m_MsgWnd)
	{
	//	if(m_MsgWnd->GetSafeHwnd())
	//		::SendMessage(m_MsgWnd->GetSafeHwnd(), WM_DATAENGINELOG, (WPARAM)(L"Logic"), (LPARAM)loginfo);
	}
}

CLogicBase*	CDLLObject::GetLB()
{
	return m_pLB;
}
bool CDLLObject::SetLB(CLogicBase* pLB)
{
	m_pLB = pLB;
	return true;
}
HINSTANCE CDLLObject::GetHinstance()
{
	return m_pHinstance;
}
bool CDLLObject::SetHinstance(HINSTANCE pHinstance)
{
	m_pHinstance = pHinstance;
	return true;
}
wstring	CDLLObject::GetDllImportTime()
{
	return m_sImportTime;
}

wstring	CDLLObject::GetDllAuthor()
{
	return m_sDllAuthor;
}
bool CDLLObject::SetDllAuthor(wstring author)
{
	m_sDllAuthor = author;

	return true;
}
wstring	CDLLObject::GetDllPath()
{
	return m_sDllPathFile;
}
bool CDLLObject::SetDllPath(wstring path)
{
	m_sDllPathFile = path;

	return true;
}

bool CDLLObject::SetDllImportTime(wstring dllImporttime)
{
	m_sImportTime = dllImporttime;

	return true;
}

wstring CDLLObject::GetDllName()
{
	return m_sDllName;
}

bool CDLLObject::SetDllName(wstring dllname)
{
	m_sDllName = dllname;
	return true;
}

pfnLogicBase CDLLObject::GetFun()
{
	return m_pFun;
}

wstring		CDLLObject::GetOriginalDllName()
{
	return m_strOriginalName;
}

bool		CDLLObject::SetOriginalDllName(wstring dllname)
{
	m_strOriginalName = dllname;
	return true;
}

bool CDLLObject::SetFun(pfnLogicBase fun)
{
	m_pFun = fun;
	return true;
}
double	CDLLObject::GetTimeSpan()
{
	return m_iTimeSpan;
}
bool CDLLObject::SetTimeSpan(double Span)
{
	m_iTimeSpan = Span;
	return true;
}
bool CDLLObject::GetAvailable()
{
	return m_bAvailable;
}
bool CDLLObject::SetAvailable(bool use)
{
	m_bAvailable = use;

	return true;
}


bool CDLLObject::Exit()
{
	UnLoad();
	wstring str = L"========" + m_sDllName + L" Engine (Version: 1.0) is Unload...========";
	SentMsgInfo(str.c_str());

	wstring dll_name =m_sDllName;

	HWND hwnd = m_MsgWnd->GetSafeHwnd();

	ReSet();

	//::SendMessage(hwnd, WM_UPDATEDLLCOUNT, 0, (LPARAM)(LPCTSTR)dll_name.c_str());

	return true;
}

bool CDLLObject::GetLoaded()
{
	return m_bLoaded;
}
bool CDLLObject::SetLoaded( bool isload )
{
	m_bLoaded = isload;

	return true;
}

bool CDLLObject::GetDllInputParamterValue( CLogicBase* LB )
{
	if (LB == NULL)
	{
		return false;
	}

	for (int i=0;i<m_vecCurDllInputParameterList.size();++i)
	{
		wstring strCurValue;
		if(m_vecCurDllInputParameterList[i]->UpdateValue(strCurValue))
			m_pLB->SetInputValue(i, strCurValue.c_str());
	}

	return true;
}

bool CDLLObject::SetDllOutputParamterValue( CLogicBase* LB )
{
	if (LB == NULL)
	{
		return false;
	}

	for (int i=0;i<m_vecCurDllOutputParameterList.size();++i)
	{
		CLogicParameter *pPara = m_vecCurDllOutputParameterList[i];
		if (pPara->Enabled())
		{
			if (LB->GetOutputChangeFlag(i))//如果有值更新
			{
				CString strOutputValue;
				LB->GetOutputValue(i,strOutputValue);
				wstring wstr = strOutputValue.GetString();

				bool bSetSuccess = pPara->UpdateValue(wstr); 

				
				//变量设为已写
				LB->SetOutputChangeFlag(i,FALSE);

				if(bSetSuccess==false)
					return false;

			}
		}
	}

	return true;
}


bool  CDLLObject::SetDllInputParamter(wstring strVariableName, wstring strLinkValue, wstring strLinkType)
{ //可以改变类型
	int nPIndex = FindInputParameter(strVariableName);
	if(nPIndex<0)
		return false;


	CLogicParameter *pPara = m_vecCurDllInputParameterList[nPIndex];

	if(strLinkType== pPara->GetType())
		pPara->SetSettingValue(strLinkValue); 
	else
	{ // type 被修改
		if(strLinkType==L"const")
		{
			m_vecCurDllInputParameterList[nPIndex] = new CLogicParameterConst(pPara->GetName(),0,strLinkType, pPara->GetDataAccess(),strLinkValue);
		}
		else if(strLinkType==L"logiclink")
		{
			//不支持在线修改
		}
		else
		{
			m_vecCurDllInputParameterList[nPIndex] = new CLogicParameterPoint(pPara->GetName(),0,strLinkType, pPara->GetDataAccess(),strLinkValue);
		}

		if(pPara)
		{
			delete(pPara);
			pPara = NULL;
		}


	}

	return true;
}


bool  CDLLObject::SetDllOutputParamter(wstring strVariableName, wstring strLinkValue, wstring strLinkType)
{
	int nPIndex = FindOutputParameter(strVariableName);
	if(nPIndex<0)
		return false;

	CLogicParameter *pPara = m_vecCurDllOutputParameterList[nPIndex];
	
	if(strLinkType== pPara->GetType())
		pPara->SetSettingValue(strLinkValue); 
	else
	{ // type 被修改
		if(strLinkType==L"const" || strLinkType==L"strconst")
		{
			m_vecCurDllOutputParameterList[nPIndex] = new CLogicParameterConst(pPara->GetName(),1,strLinkType, pPara->GetDataAccess(),strLinkValue);
		}
		else if(strLinkType==L"logiclink")
		{
			//不支持在线修改
		}
		else
		{
			m_vecCurDllOutputParameterList[nPIndex] = new CLogicParameterPoint(pPara->GetName(),1,strLinkType, pPara->GetDataAccess(),strLinkValue);
		}

		if(pPara)
		{
			delete(pPara);
			pPara = NULL;
		}


	}
	
	return true;
}

int CDLLObject::FindInputParameter(wstring strVariableName)
{
	int i = 0;
	for(i=0;i<m_vecCurDllInputParameterList.size();i++)
	{
		if(m_vecCurDllInputParameterList[i]->GetName()== strVariableName)
			return i;
	}

	return -1;
}



int CDLLObject::FindOutputParameter(wstring strVariableName)
{
	for(int i=0;i<m_vecCurDllOutputParameterList.size();i++)
	{
		if(m_vecCurDllOutputParameterList[i]->GetName()== strVariableName)
			return i;
	}

	return -1;
}

bool CDLLObject::SetDllVersion( const wstring &ver )
{
	m_sDllVersion = ver;
	return true;
}

wstring CDLLObject::GetDllVersion()
{
	return m_sDllVersion;
}

bool CDLLObject::SetDllDescription( const wstring &des )
{
	m_sDllDescription = des;

	return true;
}

wstring CDLLObject::GetDllDescription()
{
	return m_sDllDescription;
}

wstring CDLLObject::GetS3DBPath()
{
	return m_sS3DBPath;
}

bool CDLLObject::SetS3DBPath(const wstring &s3dbpath)
{
	m_sS3DBPath = s3dbpath;

	return true;
}


void CDLLObject::AddLogInfo(const wstring &strLog)
{
	
		if (m_strLogList.size()<LogStoreSize)
		{
			m_strLogList.push_back(strLog);
		}
		else
		{
			// 超过一定条数量，将记录为一个文件，清空
			wstring strFilePath;
			Project::Tools::GetSysPath(strFilePath);

			COleDateTime tnow = COleDateTime::GetCurrentTime();
			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) ); 
			setlocale( LC_CTYPE, ("chs"));

			CString strLogPath;
			strLogPath.Format(L"%s\\log\\%s-%s.txt",strFilePath, GetDllName(), tnow.Format(L"%Y%m%d%H%M%S"));
			CStdioFile ff(strLogPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
			for(int iLine=0;iLine<m_strLogList.size();iLine++)
			{
				wstring strInfo = m_strLogList[iLine];
				strInfo+=L"\r\n";
				ff.WriteString(strInfo.c_str());
			}
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 


			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_strLog);
			m_strLogList.clear();
		}
	
}


bool CDLLObject::GetLogAll(wstring &strAllLog)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_strLog);
	for (int i =0;i<m_strLogList.size();++i)
	{
		strAllLog += m_strLogList[i];
		strAllLog += L"\r\n";
	}

	return true;
}



bool CDLLObject::SetDllThreadName(wstring strDllThreadName)
{
	m_sDllThreadName = strDllThreadName;

	return true;
}

wstring  CDLLObject::GetDllThreadName()
{
	return m_sDllThreadName;
}



void CDLLObject::ClearParameters()
{
	int i = 0;
	for(i=0;i<m_vecCurDllInputParameterList.size();i++)
	{
		SAFE_DELETE(m_vecCurDllInputParameterList[i]);
	}

	for(i=0;i<m_vecCurDllOutputParameterList.size();i++)
	{
		SAFE_DELETE(m_vecCurDllOutputParameterList[i]);
	}

	m_vecCurDllInputParameterList.clear();
	m_vecCurDllOutputParameterList.clear();
}

bool CDLLObject::UnLoad()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_LB);
	//告诉线程不在进行下一运行
	if (m_pLB != NULL)
	{
		m_pLB->Exit();
		delete m_pLB;
		m_pLB = NULL;
	}
	if (m_pHinstance != NULL)
	{
		FreeLibrary(m_pHinstance);
	}

	return true;
}

bool CDLLObject::Load(wstring strPath)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_LB);
	wstring strComp = GetDllName();

	if (GetLoaded())
	{
		return true;
	}

	HINSTANCE m_hDLL;
	m_hDLL = NULL;
	m_hDLL = LoadLibrary(strPath.c_str());//加载动态链接库
	int nErr = GetLastError();
	if(m_hDLL==NULL)
	{
		CString strLog;
		strLog.Format(_T("ERROR: Logic %s Loadlibrary return NULL!\r\n"),strComp.c_str());
		PrintLog(strLog.GetString());
		return false ;
	}

	pfnLogicBase m_pf_Dll = (pfnLogicBase)GetProcAddress(m_hDLL,"fnInitLogic");

	if(m_pf_Dll==NULL)
	{
		CString strLog;
		strLog.Format(_T("ERROR: Logic %s Cant find InitLogic entrance!\r\n"),strComp.c_str());
		PrintLog(strLog.GetString());

		FreeLibrary(m_hDLL);
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	//运行当前加载的dll函数
	CLogicBase *pLBInterface = m_pf_Dll();

	if(pLBInterface==NULL)
		return false;



	wstring dllver = pLBInterface->GetSystVersion().GetString();

	CString cstrdllver = dllver.c_str();
	CString cstrgatewayver = LogicSysVersion.c_str();

	double  ddllver = _ttof(cstrdllver.Mid(1,cstrdllver.GetLength()));
	double  dgatewayver = _ttof(cstrgatewayver.Mid(1,cstrgatewayver.GetLength()));

	if (ddllver < dgatewayver)
	{
		CString strLog;
		strLog.Format(_T("ERROR: Logic %s Version Overdue!\r\n"),strComp.c_str());
		PrintLog(strLog.GetString());
		FreeLibrary(m_hDLL);
		return false;

	}

	pLBInterface->SetDllName(GetDllName().c_str());
	pLBInterface->SetDataAccess(m_pDataAccess);

	//这个变量应该是用dll内部的
	SetDllVersion(pLBInterface->GetDllLogicVersion().GetString());
	SetDllAuthor(pLBInterface->GetDllAuthor().GetString());
	SetDllDescription(pLBInterface->GetDllDescription().GetString());


	//////////////////////////////////////////////////////////////////////////

	//存储dell相关信息
	//dll 句柄
	SetHinstance(m_hDLL);
	//dll 函数接口
	SetFun(m_pf_Dll);
	SetLB(pLBInterface);

	return true;
}


bool CDLLObject::RunOnce()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_LB);

	CLogicBase *pLB = GetLB();

	if ( pLB != NULL)
	{

		bool bInputPara = GetDllInputParamterValue(pLB); 
		if (bInputPara)
		{
			m_bLogicActing = true;
			pLB->ActLogic();
			m_bLogicActing = false;


			bool bSetSuccess = SetDllOutputParamterValue(pLB);
			if(!bSetSuccess) //写输出失败
			{
				CString strTemp;
				strTemp.Format(L"ERROR: Logic %s Set Output Parameters Value Failed.\r\n", pLB->GetDllName());
				PrintLog(strTemp.GetString());
			}
		}
		else //读取输入失败
		{
			CString strTemp;
			strTemp.Format(L"ERROR: Logic %s Get Input Parameters Value Failed.\r\n", pLB->GetDllName());
			PrintLog(strTemp.GetString());
		}
		//GetDllInputParamterValue(m_pLB);

	}

	return true;
}

void CDLLObject::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
