#pragma once


#include "DatabaseSession.h"
#include "../CustomTools.h"


namespace ExportDataAccess
{

	struct	 ExportParam{
		string	tablename;
		string  dbname;
		SYSTEMTIME	st_begin;
		SYSTEMTIME	st_end;
		string	exportpath;
		string pointname;
	};

	struct  ExportValueEntry
	{
		SYSTEMTIME	st_begin;
		string		pointname;
		double		value;
		ExportValueEntry(){
			value = 0.0;
			GetLocalTime(&st_begin);
		}
	};

	class ExportDataAccess: public CDatabaseSeesion
	{
	public:
		 bool	ExportSensorDataDTU(const ExportParam& exportparam);
		 bool	ExportRecordCommon(const ExportParam& exportparam);

		 //将传感器数据导出到内存里。
		 bool	ExportSensorDataDTU(const ExportParam& exportparam, vector<ExportValueEntry>& resultlist);
	private:
		Project::Tools::Mutex	m_lock;
	};
	

	struct  DTUDatabaseInfo{
		std::wstring databasename;
		std::wstring projectname;
	};
	
	struct  DtuPointEntry
	{
		enum {
			POINTNAME_SIZE = 128,
			POINTCOMMENT_SIZE = 256
		};
		int index;				//编号
		WCHAR pointname[POINTNAME_SIZE];	//点名
		WCHAR comment[POINTCOMMENT_SIZE];		//注释
		int type;				//类型
		bool useindex;			//描述dtu历史数据使用编号还是点名来标识点位，true使用编号，false使用点名
		int	 reserved1;			//保留
		int  reserved2;			//保留

		DtuPointEntry(){
			index = 0;
			memset(pointname, 0, POINTNAME_SIZE);
			memset(comment, 0, POINTCOMMENT_SIZE);
			type = 0;
			useindex = false;
			reserved1 = reserved2 = 0;
		}
	};

	class  CDTUDatabasePointInfoAccess : public CDatabaseSeesion
	{
	public:
		bool	ReadDatabaseList(vector<DTUDatabaseInfo>& dblist);

		//读数据中心的点。
		bool	ReadPointList(const string& databasename, vector<DtuPointEntry>& resultlist);

		//读本地数据库的点
		bool	ReadPointList_Local(const string& databasename, vector<DtuPointEntry>& resultlist);

		//导出DTU数据到文件
		bool	ExportDTUDataToFile(const ExportParam& exportparam);

		bool	ExportDTUDataToMemory(const ExportParam& exportparam, vector<ExportValueEntry>& resultlist);

	private:
		Project::Tools::Mutex m_lock;
	};
}

