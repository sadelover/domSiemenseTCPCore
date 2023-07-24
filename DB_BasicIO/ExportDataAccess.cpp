#include "StdAfx.h"
#include "ExportDataAccess.h"

#include <sstream>
#include <fstream>

#include "TableAnalyzer.h"
#include "../Tools/Scope_Ptr.h"
#include "MysqlDeleter.h"

namespace ExportDataAccess
{

	bool ExportDataAccess::ExportRecordCommon( const ExportParam& exportparam )
	{

		return false;
	}


	// export the dtu data to to specified path.
	// 问题在于跨表查询。
	// we need to calculate table name from the start time to end time.
	bool ExportDataAccess::ExportSensorDataDTU( const ExportParam& exportparam )
	{
		
		return true;
	}

	bool CDTUDatabasePointInfoAccess::ReadDatabaseList( vector<DTUDatabaseInfo>& dblist )
	{
		Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

		string sql = "select * from dtudatabaseinfo.databaseprojectinfo order by projectname";
		MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
		if (result == NULL)
		{
			return false;
		}
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


			MYSQL_ROW row = NULL;
			while (row =FetchRow(result))
			{
				DTUDatabaseInfo info;
				info.databasename = Project::Tools::UTF8ToWideChar(row[0]);
				info.projectname = Project::Tools::UTF8ToWideChar(row[1]);

				dblist.push_back(info);
			}

			return true;

	}


	bool CDTUDatabasePointInfoAccess::ReadPointList( const string& databasename,vector<DtuPointEntry>& resultlist )
	{

		Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
		string sql_readpointlist = "select * from ";
		sql_readpointlist += databasename;
		sql_readpointlist+=".pointlist order by name";
		MYSQL_RES* result = QueryAndReturnResult(sql_readpointlist.c_str());
		if (result == NULL)
		{
			return false;
		}
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

		long rownum = GetRowNum(result);
		if (rownum == 0){
			return true;
		}
		resultlist.reserve(rownum);
		MYSQL_ROW row = NULL;
		wstring pointname;
		wstring pointcomment;
		while(row = FetchRow(result))
		{
			DtuPointEntry entry;
			entry.index = ATOI(row[0]);
			Project::Tools::UTF8ToWideChar(row[1], pointname);
			wcscpy_s(entry.pointname,pointname.c_str());
			Project::Tools::UTF8ToWideChar(row[2], pointcomment);
			wcscpy_s(entry.comment,pointcomment.c_str());
			entry.type = ATOI(row[3]);
			entry.useindex = (ATOI(row[4]) == 0) ? false : true;
			entry.reserved1 = ATOI(row[5]);
			entry.reserved2 = ATOI(row[6]);

			resultlist.push_back(entry);
		}

		return true;
	}

	bool CDTUDatabasePointInfoAccess::ExportDTUDataToFile( const ExportParam& exportparam )
	{
		Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
		
		string str_start = Project::Tools::SystemTimeToString(exportparam.st_begin);
		string str_end = Project::Tools::SystemTimeToString(exportparam.st_end);
		std::ostringstream sqlstream;
		sqlstream << "select * from " 
			<< exportparam.dbname
			<< "."
			<< exportparam.tablename
			<< " where time >= '" << str_start << "' and time <= '" << str_end
			<< "' and pointname = '" << exportparam.pointname << "'";

		MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
		if (result == NULL)
		{
			return false;
		}
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

		
		std::ofstream filestream(exportparam.exportpath.c_str(), std::ios::out | std::ios::_Noreplace);
		if(!filestream.good()){
			return false;
		}
		filestream.seekp(0, std::ios::beg);;

		const string cstrLineEnd = "\n";
		const string cstrComma = ",";
		//title
		filestream << "time"  << cstrComma << exportparam.pointname << cstrLineEnd;
		MYSQL_ROW  row = NULL;
		while(row = FetchRow(result) )
		{   
			filestream << row[0] << cstrComma<< row[2] << cstrLineEnd;
		}
		filestream.close();
		return true;
	}


	/*
	 *分表查询。
	 *数据库的数据会按月分表。
	 *查询时，根据传进来的时间段，自动判断表名，并组装sql语句。
	 */

	bool CDTUDatabasePointInfoAccess::ExportDTUDataToMemory( const ExportParam& exportparam, vector<ExportValueEntry>& resultlist )
	{
		Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

		std::vector<CTableAnalyzer::queryentry> scantablelist;		//要扫描的表名
		CTableAnalyzer	cta(exportparam.tablename);
		cta.AnalyzeTableQuery(scantablelist, exportparam.st_begin, exportparam.st_end);
		if (scantablelist.empty()){
			return false;
		}

		std::ostringstream sqlstream;
		for (unsigned int i = 0; i < scantablelist.size(); i++)
		{
			const CTableAnalyzer::queryentry& entry = scantablelist[i];
			string str_start = Project::Tools::SystemTimeToString(exportparam.st_begin);
			string str_end = Project::Tools::SystemTimeToString(exportparam.st_end);
			
			if (i == 0)
			{
				sqlstream << "select * from " 
					<< exportparam.dbname
					<< "."
					<< entry.tablename
					<< " where time >= '" << str_start << "' and time <= '" << str_end
					<< "' and pointname = '" << exportparam.pointname << "'";
			}else
			{
				sqlstream << " union all select * from " 
					<< exportparam.dbname
					<< "."
					<< entry.tablename
					<< " where time >= '" << str_start << "' and time <= '" << str_end
					<< "' and pointname = '" << exportparam.pointname << "'";
			}
			
		}

		string sqlstatement = sqlstream.str();		
		MYSQL_RES* result = QueryAndReturnResult(sqlstatement.c_str());
		if (result == NULL)
		{
			return false;
		}
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

		
		my_ulonglong rownum = GetRowNum(result);
		if (rownum == 0){
			return true;
		}
		resultlist.reserve(rownum);

		MYSQL_ROW  row = NULL;
		while(row = FetchRow(result) )
		{   
			ExportValueEntry entry;
			Project::Tools::String_To_SysTime(row[0], entry.st_begin);
			entry.st_begin.wSecond = 0;
			entry.pointname = row[1];
			entry.value = ATOF(row[2]);

			resultlist.push_back(entry);
			
		}
		return true;
	}

	bool CDTUDatabasePointInfoAccess::ReadPointList_Local( const string& databasename, vector<DtuPointEntry>& resultlist )
	{
		Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

		CStringA tablename;
		SYSTEMTIME st;
		GetLocalTime(&st);
		tablename.Format("historydata_%d_%02d_%02d", st.wYear, st.wMonth, st.wDay);

		string sql_readpointlist = "select distinct pointname from ";
		sql_readpointlist += databasename;
		sql_readpointlist += ".";
		sql_readpointlist += tablename.GetString();
		sql_readpointlist+=" order by pointname";
		MYSQL_RES* result = QueryAndReturnResult(sql_readpointlist.c_str());
		if (result == NULL)
		{
			return false;
		}
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


		long rownum = GetRowNum(result);
		if (rownum == 0){
			return true;
		}
		resultlist.reserve(rownum);
		MYSQL_ROW row = NULL;
		wstring pointname;
		wstring pointcomment;
		while(row = FetchRow(result))
		{
			DtuPointEntry entry;
			Project::Tools::UTF8ToWideChar(row[0], pointname);
			wcscpy_s(entry.pointname,pointname.c_str());
			resultlist.push_back(entry);
		}

		return true;
	}

}

