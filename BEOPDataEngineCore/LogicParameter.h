#pragma once

#include "STRING"

using namespace std;

class CBEOPDataAccess;



class CLogicParameter
{
public:
	CLogicParameter(wstring strName, int nInOut, wstring strType,  CBEOPDataAccess *pDataAccess);
	virtual ~CLogicParameter(void);

	wstring GetName();
	wstring GetSettingValue();
	bool SetSettingValue(wstring strValue);
	virtual wstring GetOutputString();
	wstring GetType();

	bool Enabled();
	bool IsInput();
	bool IsOutput();

	virtual bool UpdateValue(wstring &strValueInput);
	CBEOPDataAccess * GetDataAccess();

protected:
	wstring m_strName;
	wstring m_strSettingValue;

	wstring m_strValue;

	CBEOPDataAccess *m_pDataAccess;
	wstring m_strEnabledCondition;

	wstring m_strType;
	int m_nInOut;
};

