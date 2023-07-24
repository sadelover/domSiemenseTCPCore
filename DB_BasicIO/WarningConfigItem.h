#pragma once

#include <string>

#include "DBInterfaceImp.h"

class   CWarningConfigItem
{
public:
	CWarningConfigItem(void);
	~CWarningConfigItem(void);

	double m_fLimitHH;
	double m_fLimitLL;
	double m_fLimitL;
	double m_fLimitH;

	bool m_bEnableHH;
	bool m_bEnableLL;
	bool m_bEnableH;
	bool m_bEnableL;

	std::wstring m_strPointName;
	std::wstring m_strWarningContent_HH;
	std::wstring m_strWarningContent_H;
	std::wstring m_strWarningContent_L;
	std::wstring m_strWarningContent_LL;

	//以下是bool点报警配置信息
	int m_nWarningType; //是否是bool警报点
	std::wstring m_strWarningContent;
	int m_nWarningLevel;

	std::wstring m_strOfPosition;
	std::wstring m_strOfSystem;
	std::wstring m_strOfDepartment;
	std::wstring m_strOfGroup;
	std::wstring m_strTag;


	std::wstring m_strUnitProperty01;
	std::wstring m_strUnitProperty02;
	std::wstring m_strUnitProperty03;
	std::wstring m_strUnitProperty04;
	std::wstring m_strUnitProperty05;


	int m_nRuleID;
};

