#pragma once

#include "../../Tools/DllDefine.h"
#include <map>
#include <vector>
using namespace std;

struct StructParam3
{
	CString strName;
	map<int,CString> mapping; 
};

class DLLEXPORT CParseStringParam3
{
public:
	CParseStringParam3(void);
	~CParseStringParam3(void);
	static CParseStringParam3& Instance();//获取该类的实例
	static void DestoryInstance();//销毁该类的实例
	void ParseParam3(const CString strName, const CString strParam3 );//输入参数为点的名称和param3字符串
	const CString GetText(const CString strName, const int nValue);//通过点的名称和param3中配置好的值，得到该值要显示的文本
private:
	static CParseStringParam3* m_instane;
	vector<StructParam3> m_vecStructParam3;
};

