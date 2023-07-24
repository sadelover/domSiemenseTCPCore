#include "StdAfx.h"
#include "ParseStringParam3.h"
#include "../Tools/Util/UtilString.h"
using namespace UtilString;
#include <cassert>

CParseStringParam3* CParseStringParam3::m_instane = NULL;

CParseStringParam3::CParseStringParam3(void)
{

}


CParseStringParam3::~CParseStringParam3(void)
{
}

CParseStringParam3& CParseStringParam3::Instance()
{
	if (m_instane == NULL)
	{
		m_instane = new CParseStringParam3();
		assert(m_instane);
	}
	return *m_instane;
}

void CParseStringParam3::DestoryInstance()
{
	if (m_instane)
	{
		delete m_instane;
		m_instane = NULL;
	}
}

void CParseStringParam3::ParseParam3(const CString strName, const CString strParam3 )
{
	assert(strName.GetLength()>0);
	assert(strParam3.GetLength()>0);

	vector<wstring> vecContent;
	SplitStringByChar(strParam3, L';', vecContent);
	StructParam3 param3;
	param3.strName = strName;
	for (size_t i=0;i<vecContent.size();++i)
	{
		vector<wstring> vecContentSub;
		SplitStringByChar(vecContent[i].c_str(), L':', vecContentSub);
		if(vecContentSub.size()<2)
			continue;
		const int nValue = _wtoi(vecContentSub[0].c_str());
		CString strText = vecContentSub[1].c_str();
		param3.mapping[nValue] = strText;
	}
	m_vecStructParam3.push_back(param3);
}

const CString CParseStringParam3::GetText( const CString strName, const int nValue )
{
	const int size = m_vecStructParam3.size();
	for (int i=0;i<size;++i)
	{
		if (strName == m_vecStructParam3[i].strName)
		{
			map<int,CString>::iterator iter = m_vecStructParam3[i].mapping.begin();
			map<int,CString>::iterator iter_end = m_vecStructParam3[i].mapping.end();
			for (;iter!=iter_end;++iter)
			{
				if (iter->first == nValue)
				{
					return iter->second;
				}
			}
		}
	}
	return L"";
}

