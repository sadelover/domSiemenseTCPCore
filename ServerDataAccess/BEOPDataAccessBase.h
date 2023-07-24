#pragma once

#include "xstring"

using namespace std;

class CBEOPDataAccessBase
{
public:
	CBEOPDataAccessBase(void) {};
	virtual ~CBEOPDataAccessBase(void) {};

	virtual bool GetValue(const wstring& strName, bool &bParam) = 0;
	virtual bool GetValue(const wstring& strName, double &dbParam) =0;
	virtual bool GetValue(const wstring& strName, int &uiParam) = 0;
	virtual bool GetValue(const wstring& strName, wstring &bParam) = 0;
	virtual bool SetValue( const wstring& strName, bool bParam ) =0;
	virtual bool SetValue( const wstring& strName, int nParam ) = 0;
	virtual bool SetValue( const wstring& strName, double dParam ) =0;
	virtual bool SetValue( const wstring& strName, wstring nParam ) =0;
	virtual bool    GetPointExist(const wstring & strPointName) =0;
	virtual bool	AddWarning(int nWarningLevel , const wstring &strWarningContent, const wstring &strBindPoint, int nRuleID,
		const wstring &strOfPosition,const wstring &strOfSystem,const wstring &strOfDepartment,const wstring &strOfGroup,const wstring &strTag , const wstring &strInfoDetail, const wstring &strInfoRange, const wstring &strBindPointValue, const wstring &strUnitProperty03,const wstring &strUnitProperty04,const wstring &strUnitProperty05) =0;
	virtual bool    AddOperationRecord(const wstring &strUserName, const wstring & strOperationContent) =0;
	virtual bool GetHistoryValue(const wstring& strName,const SYSTEMTIME &st,const int &nTimeFormat,  wstring &strParam) = 0;
	virtual bool GetHistoryValue(const wstring& strName,const SYSTEMTIME &stStart,const SYSTEMTIME &stEnd,const int &nTimeFormat,  wstring &strParam) = 0;
	virtual bool	InsertLog(const wstring& loginfo) = 0;
	virtual bool	InsertLog(const vector<SYSTEMTIME> & sysTimeList, const vector<wstring> & loginfoList) =0;

};

