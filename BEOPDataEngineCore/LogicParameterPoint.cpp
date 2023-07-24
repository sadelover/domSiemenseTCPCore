#include "StdAfx.h"
#include "LogicParameterPoint.h"
#include "../ServerDataAccess/BEOPDataAccess.h"


CLogicParameterPoint::CLogicParameterPoint(wstring strName, int nInOut,wstring strType,   CBEOPDataAccess *pDataAccess, wstring strPointName , wstring strPermitPointName)
	: CLogicParameter(strName,  nInOut, strType,  pDataAccess),
	m_strPermitPointName(strPermitPointName)
{
	SetSettingValue(strPointName);
}


CLogicParameterPoint::~CLogicParameterPoint(void)
{
}


bool CLogicParameterPoint::UpdateValue(wstring &strValue)
{
	if(IsInput()) //is input port
	{
		if(m_strSettingValue.length()<=0)
			return true;

		wstring strValueRead;
		bool bReadSuccess = GetDataAccess()->GetValue(GetSettingValue(), strValueRead);
		if(bReadSuccess)
			strValue = strValueRead;

	}
	else //is output port
	{
		bool bReadSuccess = false;
		if(m_strSettingValue.length()<=0)
			return true;

		bool bPermit = true;
		if(m_strPermitPointName==L"1")
		{
			bPermit = true;
		}
		else if(m_strPermitPointName==L"0")
		{
			bPermit = false;
		}
		else if(m_strPermitPointName.length()>0)
		{
			bReadSuccess = GetDataAccess()->GetValue(m_strPermitPointName, bPermit);
			if(!bReadSuccess)
			{
				Project::Tools::PrintLog(L"ERROR: Logic Output Permit Point Not Exist.\r\n");
				return false;
			}
		}

		//不允许写则返回.
		if(!bPermit)
			return true;

		wstring strValueRead;
		bReadSuccess = GetDataAccess()->GetValue(GetSettingValue(), strValueRead);
		if(bReadSuccess && strValueRead!=strValue)
		{
			bool bWriteSuccess = GetDataAccess()->SetValue(GetSettingValue(), strValue);
			if(!bWriteSuccess)
				return false;
		}

		m_strValue =  strValue;
	}

	return true;
}

wstring CLogicParameterPoint::GetOutputString()
{
	return L"";
}