#pragma once

#include "DataBaseSetting.h"
#include "LogicBase.h"
#include "../ServerDataAccess/BEOPDataAccess.h"

class CBEOPDataAccess;
class CLogicBase;
class CLogicParameter;


typedef CLogicBase* (*pfnLogicBase)();

typedef std::vector<std::wstring> vec_wstr;

class  CDLLObject
{
public:
	CDLLObject(CBEOPDataAccess *pDataAccess);
	virtual ~CDLLObject(void);
public:

	bool Exit();
	bool ReSet();

	CLogicBase*	GetLB();
	bool		SetLB(CLogicBase* pLB);
	HINSTANCE	GetHinstance();
	bool		SetHinstance(HINSTANCE pHinstance);

	wstring		GetDllName();
	bool		SetDllName(wstring dllname);
	wstring		GetDllAuthor();
	bool		SetDllAuthor(wstring author);

	wstring		GetOriginalDllName();
	bool		SetOriginalDllName(wstring dllname);

	wstring		GetDllPath();
	bool		SetDllPath(wstring path);

	wstring		GetDllImportTime();
	bool		SetDllImportTime(wstring dllImporttime);

	pfnLogicBase	GetFun();
	bool		SetFun(pfnLogicBase fun);

	double		GetTimeSpan();
	bool		SetTimeSpan(double Span);

	bool		GetAvailable();
	bool		SetAvailable(bool use);

	void		SetMsgWnd(CWnd * MsgWnd);
	void		SentMsgInfo(LPCTSTR loginfo);

	bool		GetLoaded();
	bool		SetLoaded(bool isload);

	bool        GetDllInputParamterValue(CLogicBase* LB);
	bool        SetDllOutputParamterValue(CLogicBase* LB);
	
	bool        SetDllInputParamter(wstring strVariableName, wstring strLinkValue, wstring strLinkType);
	bool        SetDllOutputParamter(wstring strVariableName, wstring strLinkValue, wstring strLinkType);

	int			FindInputParameter(wstring strVariableName);
	int			FindOutputParameter(wstring strVariableName);

	bool		SetDllVersion(const wstring &ver);
	wstring		GetDllVersion();
	bool		SetDllDescription(const wstring &des);
	wstring		GetDllDescription();
	wstring     GetS3DBPath();
	bool		SetS3DBPath(const wstring &s3dbpath);
	void		AddLogInfo(const wstring &strLog);
	bool        GetLogAll(wstring &strAllLog);

	bool        SetDllThreadName(wstring strDllThreadName);
	wstring     GetDllThreadName();
	void		ClearParameters();

	bool		Load(wstring strPath);
	bool		UnLoad();
	bool		RunOnce();
	void 		PrintLog(const wstring &strLog,bool bSaveLog = true);
protected:
	CLogicBase*					m_pLB;
	HINSTANCE					m_pHinstance;
	wstring						m_sDllName;
	wstring						m_sDllVersion;
	wstring						m_sDllDescription;
	bool						m_bDllRun;
	wstring                     m_sImportTime;
	wstring						m_sDllAuthor;
	wstring						m_sDllPathFile;
	wstring                     m_sDllThreadName;
	wstring						m_strOriginalName;
	pfnLogicBase	m_pFun;
	double			m_iTimeSpan;
	bool			m_bAvailable;

	bool			m_bLoaded;
public:
	CBEOPDataAccess *m_pDataAccess;
	bool			m_bLogicActing;
	std::vector<CLogicParameter *> m_vecCurDllInputParameterList;
	std::vector<CLogicParameter *> m_vecCurDllOutputParameterList;

	Project::Tools::Mutex	m_lock_LB;
	Project::Tools::Mutex	m_lock_strLog;
	std::vector<wstring> m_strLogList; 
private:
	CWnd*			m_MsgWnd;
	wstring			m_sS3DBPath;
};

