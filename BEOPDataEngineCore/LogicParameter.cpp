#include "StdAfx.h"
#include "LogicParameter.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "Tools/Util/UtilString.h"


CLogicParameter::CLogicParameter(wstring strName, int nInOut,wstring strType,   CBEOPDataAccess *pDataAccess)
	:m_strName(strName),
	m_pDataAccess(pDataAccess),
	m_nInOut(nInOut),
	m_strType(strType)
{
}


CLogicParameter::~CLogicParameter(void)
{
}


wstring CLogicParameter::GetSettingValue()
{
	return m_strSettingValue;
}


bool CLogicParameter::SetSettingValue(wstring strValue)
{
	m_strSettingValue = strValue;
	return true;
}

wstring CLogicParameter::GetType()
{
	return m_strType;
}

wstring CLogicParameter::GetOutputString()
{
	return L"";
}

wstring CLogicParameter::GetName()
{
	return m_strName;
}

bool CLogicParameter::UpdateValue(wstring &strValueInput)
{
	return true;
}

bool CLogicParameter::IsInput()
{
	return m_nInOut==0;
}


bool CLogicParameter::IsOutput()
{
	return m_nInOut==1;
}

CBEOPDataAccess * CLogicParameter::GetDataAccess()
{
	return m_pDataAccess;
}

bool CLogicParameter::Enabled()
{
	return true;
	int nNumber = 0;
	if(UtilString::CheckStringIsNumber(m_strEnabledCondition))
	{
		if(nNumber==0)
			return false;
		else
			return true;
	}
	else
	{
		bool bEnabled = false;
		bool bReadSuccess = m_pDataAccess->GetValue(m_strEnabledCondition,bEnabled);
		
		if(bReadSuccess)
			return bEnabled;
		else
			return false;
	}
	return false;
}
