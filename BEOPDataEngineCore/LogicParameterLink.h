#pragma once
#include "logicparameter.h"

class CDLLObject;

class CLogicParameterLink :
	public CLogicParameter
{
public:
	CLogicParameterLink(wstring strName, int nInOut, wstring strType,  CBEOPDataAccess *pDataAccess, CDLLObject *pObject, wstring strParaName);
	virtual ~CLogicParameterLink(void);


	virtual bool UpdateValue(wstring &strValue);
	virtual wstring GetOutputString();

	CDLLObject *m_pLinkedDllObject;
	wstring m_strParameterName;
};

