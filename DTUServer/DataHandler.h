#pragma once


#include "../DB_BasicIO/DatabaseSession.h"
#include "PackageTypeDefine.h"
#include "DTUAddressMap.h"
#include "DTUDataBaseInfo.h"

class CDataHandler: public CDatabaseSeesion
{
public:
	CDataHandler(void);
	~CDataHandler(void);

	//connect to the database
	bool	Connect(mapentry entry);

	//init
	bool Init(mapentry entry);

	bool UpdateRealDataInput(const char* buffer);

	//insert data 
	bool InsertData(const char* databasename, const char* buffer);

	//insert pointdata
	bool InsertPointData(const char* databasename, const char* buffer);
	bool InsertPointDataWithTime(const char* databasename, const char* buffer);

	//insert operation record
	bool InsertOperationRecord(const char* databasename, const char* buffer);

	//insert calc result
	bool InsertCalcResultRecord(const char* databasename, const char* buffer);
	
	void LogDBError(const string& sql);

	void	GenerateSQLHeader(weldtech::CPackageType::DTU_DATATYPE type, 
							  const char* databasename,
							  string& headerresult);

	void	GeneratePointDataHeader(const char* databasename, string& headerresult);
	void	GenerateOperationRecordDataHeader(const char* databasename, string& headerresult);
	void	GenerateCalResultDataHeader(const char* databasename, string& headerresult);

	bool	GenerateSqlAndInsert(const string& sqlheader, const char* buffer);

	bool	CreateNewTable(const string& dbname);
	string  GenerateTableName();

	bool	UpdateInputTable(const string& dtuname);
	string	QueryOutputTable();
	bool	DeleteOutputTable();

	bool	UpdateOutputTable(const string& strtime,const string& pointname,const string& pointvalue );

	bool	InsertHistoryData(const POINT_STORE_CYCLE &tCycle, const vector<spointwatch>& datalist);
	bool    InsertPointInput(const vector<spointwatch>& datalist);
	bool	ReadRealPointFromInput(vector<spointwatch>& datalist);

	bool	GetIsConnectSuccess();

	bool	UpdateWarningRecord(const vector<warning>& datalist);		//更新报警记录表
	bool	UpdateWarning(const warning war, bool bConfirmed = false);			//
	bool	UpdateWarningTable(const warning war);

	bool	UpdateWarningROperation(const vector<warningoperation>& datalist);		//更新报警操作表

	bool	CheckDBExist(mapentry entry);					//检查数据库是否存在
	bool	ReBuildDatabase(mapentry entry);					//创建数据库

	string	RelaceStr(char* strSrc, char* strFind, char* strReplace);

private:
	//CDatabaseSeesion m_dbsession;
	Project::Tools::Mutex	m_lock;
	bool					m_bConnectSuccess;
	string					m_strDataBaseName;
};

