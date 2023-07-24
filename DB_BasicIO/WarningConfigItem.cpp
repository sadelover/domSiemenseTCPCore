#include "StdAfx.h"
#include "WarningConfigItem.h"


CWarningConfigItem::CWarningConfigItem(void)
{
	m_fLimitHH = 0.f;
	m_fLimitLL = 0.f;
	m_fLimitH = 0.f;
	m_fLimitL = 0.f;

	m_bEnableH = false;
	m_bEnableHH = false;
	m_bEnableLL = false;
	m_bEnableL = false;

	m_nWarningType = 0;
	m_strWarningContent = L"";
	m_nWarningLevel = 0;
}


CWarningConfigItem::~CWarningConfigItem(void)
{
}
