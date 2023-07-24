#pragma once

#include "Tools/point/DataPointEntry.h"
#include "Tools/point/DataPointManager.h"

typedef unsigned long           u32;
const int	  g_Point_Length = 25;
const int g_nLenPointAttrSystem	= 5;
const int g_nLenPointAttrDevice	= 7;
const int g_nLenPointAttrType	= 11;
const CString g_strPointAttrSystem[g_nLenPointAttrSystem] = {_T("无"), _T("暖通"), _T("动力"), _T("照明插座"), _T("精密空调")};
const CString g_strPointAttrDevice[g_nLenPointAttrDevice] = {_T("无"), _T("冷机"), _T("水泵"), _T("冷却塔"), _T("AHU"), _T("VAV"), _T("系统")};
const CString g_strPointAttrType[g_nLenPointAttrType] = {_T("无"), _T("电量表"), _T("冷量表"), _T("温度"), _T("流量"), _T("压力"), _T("电流"), _T("功率"), _T("频率"), _T("启停"), _T("报警")};

class CExcelOperator
{
public:
	CExcelOperator(void);
	~CExcelOperator(void);
	static CExcelOperator& Instance();
	static void DestoryInstance();
	void SaveToExcel(CString sExcelFile, CDataPointManager& pointmap);
	bool ReadFromExcel(CString sExcelFile, CDataPointManager& pointmap);
	bool ReadFromExcel(CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode);
	bool ReadFromCSV(CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode);
	bool ReadFromCSV_Comma(CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode);		//处理值里带逗号的CSV
	CString GetExcelDriver();
	bool	OutPutUpdateLog(CString strLog);		//记录更新Log
	void	RemoveSubStr(const string& r, string& str);
	int		GetColumnIndex(std::map<std::string, int> map, string strColumnName);
private:
	static CExcelOperator* m_instance;
};

