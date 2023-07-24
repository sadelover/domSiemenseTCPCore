

#pragma once
#include <vector>
#include <string>
#include "xstring"

class CBEOPDataAccessBase;

typedef std::vector<CString> vec_str;

class CLogicBase 
{
public: 
	CLogicBase(void){};
	virtual ~CLogicBase(void){};
	virtual bool	Init()		= 0;
	virtual bool	ActLogic()	= 0;
	virtual bool	Exit()		= 0;
	virtual void    SetMsgWnd(CWnd* MsgWnd)		 = 0;
	virtual CWnd*   GetMsgWnd()					 = 0;
	virtual void    SentMsgInfo(LPCTSTR loginfo) = 0;
	virtual void    SetDataAccess(CBEOPDataAccessBase *pDataAccess) = 0;
	virtual CString	GetDllAuthor() = 0;
	virtual void	SetDllAuthor(CString dllauthor) = 0;
	virtual CString GetDllName() = 0;
	virtual void    SetDllName(CString dllname) = 0;
	virtual int     GetInputCount() = 0;
	virtual int     GetOutputCount() = 0;
	virtual CString GetInputName(int nIndex) = 0;
	virtual CString GetOutputName(int nIndex) = 0;
	virtual CString GetAllInputParameter() = 0;
	virtual CString GetAllOutputParameter()= 0;
public:
	virtual bool    SetInputValue( int nIndex, const CString &strValue ) = 0;
	virtual bool    GetOutputValue( int nIndex,CString &strValue ) = 0;

	virtual CString	GetSystVersion() = 0;
	virtual CString GetDllLogicVersion() = 0;
	virtual bool	SetDllLogicVersion(const CString &strVer) = 0;
	virtual CString GetDllDescription() = 0;
	virtual bool	SetDllDescription(const CString &strDes) = 0;

	virtual BOOL    GetOutputChangeFlag(int nIndex) = 0;
	virtual void    SetOutputChangeFlag(int nIndex, BOOL bSet) = 0;
};

extern "C"  CLogicBase *fnInitLogic();
