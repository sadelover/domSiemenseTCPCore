#pragma once

#include "../Tools/CustomTools/CustomTools.h"
#include "DatabaseSession.h"
#include <map>
struct  unitStateInfo
{
	string unitproperty01;		
	string unitproperty02;
	string unitproperty03;
};

struct unitLogFileInfo
{
	wstring strStartTime;		//起始时间
	wstring strEndTime;			//结束时间
	wstring strFilter;			//筛选字段(已逗号分隔)
	wstring strFileName;			//压缩包名称
	int		nType;				//1 error；1:log 2：标识全部
};


class  CRealTimeDataEntry
{
public:
	SYSTEMTIME GetTimestamp() const;
	void	SetTimestamp(const SYSTEMTIME& stime);

	string	GetName() const;
	void	SetName(const string& name);

	wstring	GetValue() const;
	void	SetValue(const wstring& value);

	bool	IsEngineSumPoint();
private:
	SYSTEMTIME	m_timestamp;					// current timestamp
	string		m_pointname;					// the point name. shortname
	wstring		m_pointvalue;					// the real value.
};

class CDataHandler: public CDatabaseSeesion
{
public:
	CDataHandler(void);
	~CDataHandler(void);

	//connect to the database
	bool	Connect(const wstring& host, const wstring& username, 
		const wstring& password, const wstring& database,
		unsigned int port);

	bool	GetAllSchema(vector<CString>& vecSchema);
	bool	GetAllTableNamesBySchema(vector<CString>& vecTable,CString strSchema,CString strNotLike = _T(""));
	
	wstring ReadOrCreateCoreDebugItemValue(wstring strItemName,wstring strValue = L"0");
	bool    WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue);
	string	ReadErrCodeRecentMinute(string nMintue = "10");
	string	ReadMysqlVariable(string strVariableName);
	bool	ReadRealTimeData_Unit01(vector<unitStateInfo>& unitlist);
	bool	ReadLogFile_Unit06(vector<unitLogFileInfo>& unitlist);
	bool	UpdateLogFile_Unit06(unitLogFileInfo unitlist);
	bool	DeleteLogFile_Unit06();
	bool	UpdateLibInsertIntoFilestorage(const wstring strFilePathName, const wstring strFileName, const wstring strFileId);
	
	//////////////////////////////////////////////////////////////////////////
	bool	ReadTableNameByDate(vector<string>& vecTables,string strTablePrefix, int nDate, int nProtectDate = 30);			//获取N天表名
	bool	ReadTableNameByDeadDate(vector<string>& vecTables,string strTablePrefix, int nDeadDate);		//获取N天以前的表名
	bool	DropTables(vector<string> vecTables);							//删除表
	bool	OutPutLog(string strSql);										//输出删除表记录
	//////////////////////////////////////////////////////////////////////////
private:
	//CDatabaseSeesion m_dbsession;
	Project::Tools::Mutex	m_lock;
	Project::Tools::Mutex	m_tool_lock;
	bool					m_bConnectSuccess;
	string					m_strDataBaseName;
	bool					m_bIsBusy;
};

