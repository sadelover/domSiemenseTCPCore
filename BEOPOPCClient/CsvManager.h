#pragma once

#include <vector>
using namespace std;
class OPCDataPointEntry;

class CCsvManager
{
public:
	CCsvManager(const CString strPathName);
	~CCsvManager(void);

	BOOL GetFileInfo(vector<OPCDataPointEntry>& vecData);

private:
	CString	m_strPathName;
};

