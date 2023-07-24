#include "StdAfx.h"
#include "LogicParameterConst.h"


CLogicParameterConst::CLogicParameterConst(wstring strName, int nInOut,wstring strType,   CBEOPDataAccess *pDataAccess, wstring strValue)
	: CLogicParameter(strName, nInOut,strType, pDataAccess)
{
	SetSettingValue(strValue);
}


CLogicParameterConst::~CLogicParameterConst(void)
{
}

bool CLogicParameterConst::UpdateValue(wstring &strValue)
{
	if(m_nInOut==0) //input port
	{
		strValue = m_strSettingValue; //const
	}
	else //output port
	{
		m_strValue = strValue;
	}
	return true;
}

wstring CLogicParameterConst::GetOutputString()
{
	return L"const:" + m_strValue;
}