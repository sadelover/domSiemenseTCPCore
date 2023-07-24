#include "StdAfx.h"
#include "LogicParameterLink.h"


CLogicParameterLink::CLogicParameterLink(wstring strName, int nInOut,wstring strType,   CBEOPDataAccess *pDataAccess, CDLLObject *pObject, wstring strParaName)
	: CLogicParameter(strName,nInOut, strType, pDataAccess),
	m_pLinkedDllObject(pObject),
	m_strParameterName(strParaName)
{

}


CLogicParameterLink::~CLogicParameterLink(void)
{
}


bool CLogicParameterLink::UpdateValue(wstring &strValue)
{
	return true;
}


wstring CLogicParameterLink::GetOutputString()
{
	return L"";
}