#pragma once
#include "logicparameter.h"
class CLogicParameterConst :
	public CLogicParameter
{
public:
	CLogicParameterConst(wstring strName, int nInOut, wstring strType,  CBEOPDataAccess *pDataAccess, wstring strValue);
	virtual ~CLogicParameterConst(void);
	

	virtual bool UpdateValue(wstring &strValue);
	virtual wstring GetOutputString();

	wstring m_strValue;
};

