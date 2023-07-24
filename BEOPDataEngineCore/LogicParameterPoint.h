#pragma once
#include "logicparameter.h"
class CLogicParameterPoint :
	public CLogicParameter
{
public:
	CLogicParameterPoint(wstring strName, int nInOut, wstring strType,  CBEOPDataAccess *pDataAccess, wstring strPointName, wstring strPermitPointName = L"");
	virtual ~CLogicParameterPoint(void);

	virtual bool UpdateValue(wstring &strValue);
	virtual wstring GetOutputString();

	wstring m_strPermitPointName;

};

