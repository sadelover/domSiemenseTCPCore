#include "StdAfx.h"

#include "RealTimeDataAccess.h"
#include <iostream>
#include <sstream>

#include "MysqlDeleter.h"
#include "../Tools/Scope_Ptr.h"

#include "../Tools/Util/UtilString.h"


namespace Beopdatalink
{
	//  this three will hold a session
	const char* tablename_input = "realtimedata_input_siemense_tcp";
	const char* tablename_output = "realtimedata_output_siemense_tcp";
	
	//log function will hold a session
	const char* tablename_log = "log";

	//action info will hold a session
	const char* tablename_actioninfo = "actioninfo";
	
	// action will hold a session
	const char* tablename_action = "action";

	// history data will hold a session
	const char* tablename_historydata = "historydata";
	const char* sql_createtable =  "(`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
								   `pointname` varchar(64) NOT NULL,\
								   `value` varchar(20000) NOT NULL,\
								   INDEX `Index_t_n` (`time`,`pointname`)) \
								   ENGINE=InnoDB DEFAULT CHARSET=utf8";

	const char* sql_createlog = "(`time` timestamp NOT NULL DEFAULT '2000-01-01 00:00:00',\
									`loginfo` varchar(20000), \
									`source` varchar(255) NOT NULL DEFAULT '',\
									`level` int(10)  NOT NULL DEFAULT 0,\
									INDEX `Index_t_s` (`time`,`source`)) \
									 ENGINE=InnoDB DEFAULT CHARSET=utf8";
	
	//warning record will hold a session
	const char* tablename_warningrecord_prefiex = "warningrecord";

	// this 2 will hold a session.
	const char* tablename_datacom_status = "datacom_status";
	const char* tablename_userlistonline = "userlist_online";

	
	//Operation record will hold a session
	const char* tablename_operationrecord = "operation_record";

	//controlparam record will hold a session
	const char* tablename_controlparamrecord = "controlparam_record";

	//user online record will hold a session
	const char* tablename_userrecord = "userlist_online";

	const char* tablename_warningconfig = "warning_config";

	const char* tablename_beopinfo	= "beopinfo";

	const char* tablename_warningoperation = "unit05";

	//schedule_list
	const char* tablename_schedule_list = "schedule_list";

	//schedule_info_weeky
	const char* tablename_schedule_info_weeky = "schedule_info_weeky";

	const char* tablename_dtu_send = "dtu_send_buffer";
}


SYSTEMTIME Beopdatalink::CRealTimeDataEntry::GetTimestamp() const
{
	return m_timestamp;
}

void Beopdatalink::CRealTimeDataEntry::SetTimestamp( const SYSTEMTIME& stime )
{
	m_timestamp = stime;
}

string Beopdatalink::CRealTimeDataEntry::GetName() const
{
	return m_pointname;
}

void Beopdatalink::CRealTimeDataEntry::SetName( const string& name )
{
	m_pointname = name;
}

wstring Beopdatalink::CRealTimeDataEntry::GetValue() const
{
	return m_pointvalue;
}

void Beopdatalink::CRealTimeDataEntry::SetValue(const wstring& value)
{
	m_pointvalue = value;
}

bool Beopdatalink::CRealTimeDataEntry::IsEngineSumPoint()
{
	if(m_pointname.length() <= 0)
		return false;

	unsigned int pos = m_pointname.find("time_update_");
	if(pos != m_pointname.npos)
		return true;

	unsigned int pos1 = m_pointname.find("err_engine_");
	if(pos1 != m_pointname.npos)
		return true;

	unsigned int pos2 = m_pointname.find("log_engine_");
	if(pos2 != m_pointname.npos)
		return true;

	return false;
}

wstring Beopdatalink::CRealTimeDataStringEntry::GetValue() const
{
	return m_strvalue;
}

void Beopdatalink::CRealTimeDataStringEntry::SetValue( const wstring& value )
{
	m_strvalue = value;
}

void Beopdatalink::CRealTimeDataEntry_Output::SetInit( bool binit )
{
	m_binit = binit;
}


bool Beopdatalink::CRealTimeDataEntry_Output::IsValueInit()
{
	return m_binit;
}

Beopdatalink::CRealTimeDataEntry_Output::CRealTimeDataEntry_Output()
	:CRealTimeDataEntry()
{
	m_binit = false;
}


bool	Beopdatalink::CRealTimeDataAccess::ReadRealtimeData_Input_ModbusEquipment(vector<pair<wstring, wstring> >& modbusvaluesets)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".realtimedata_input_modbus_equipment";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	
	MYSQL_ROW  row = NULL;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		wstring wstrPointName;
		Project::Tools::UTF8ToWideChar(row[1], wstrPointName);
		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		modbusvaluesets.push_back(make_pair(wstrPointName,wstrValue));
	}


	return true;
}
bool	Beopdatalink::CRealTimeDataAccess::ReadRealtimeData_Input_BacnetPy(vector<pair<wstring, wstring> >& modbusvaluesets)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".realtimedata_input_bacnetpy";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}

	MYSQL_ROW  row = NULL;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		wstring wstrPointName;
		Project::Tools::UTF8ToWideChar(row[1], wstrPointName);
		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		modbusvaluesets.push_back(make_pair(wstrPointName,wstrValue));
	}


	return true;
}

bool	Beopdatalink::CRealTimeDataAccess::ReadRealtimeData_Input_OPCUA(vector<pair<wstring, wstring> >& modbusvaluesets)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".realtimedata_input_opcua";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}

	MYSQL_ROW  row = NULL;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		wstring wstrPointName;
		Project::Tools::UTF8ToWideChar(row[1], wstrPointName);
		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		modbusvaluesets.push_back(make_pair(wstrPointName,wstrValue));
	}


	return true;
}



bool	Beopdatalink::CRealTimeDataAccess::ReadRealtimeData_Input_PersagyController(vector<pair<wstring, wstring> >& valuesets)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".realtimedata_input_persagy_controller";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}

	MYSQL_ROW  row = NULL;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		wstring wstrPointName;
		Project::Tools::UTF8ToWideChar(row[1], wstrPointName);
		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		valuesets.push_back(make_pair(wstrPointName,wstrValue));
	}


	return true;
}

bool	Beopdatalink::CRealTimeDataAccess::ReadRealtimeData_Input_ThirdParty(vector<pair<wstring, wstring> >& valuesets)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".realtimedata_input_thirdparty";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}

	MYSQL_ROW  row = NULL;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		wstring wstrPointName;
		Project::Tools::UTF8ToWideChar(row[1], wstrPointName);
		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		valuesets.push_back(make_pair(wstrPointName,wstrValue));
	}


	return true;
}

bool Beopdatalink::CRealTimeDataAccess::ReadRealtimeData_Input( vector<CRealTimeDataEntry>& entrylist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_input;

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	entrylist.reserve(length);
	MYSQL_ROW  row = NULL;
	CRealTimeDataEntry entry;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		entry.SetTimestamp(sttime);
		entry.SetName(row[1]);
		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		entry.SetValue(wstrValue);

		entrylist.push_back(entry);
	}


	return true;
}


bool Beopdatalink::CRealTimeDataAccess::UpdatePointData( const CRealTimeDataEntry& entry )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);
	
	sqlstream << "insert into  "
		<< GetDBName() 
		<< "."
		<< tablename_output 
		<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< "."
			<< tablename_output
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		sqlstatement = sqlstream.str();
		bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_output err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool  Beopdatalink::CRealTimeDataAccess::UpdatePointData_PersagyController(const CRealTimeDataEntry& entry)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "insert into  "
		<< GetDBName() 
		<< ".realtimedata_output_persagy_controller"
		<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".realtimedata_output_persagy_controller"
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		sqlstatement = sqlstream.str();
		bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_output_persagy_controller err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}


bool  Beopdatalink::CRealTimeDataAccess::UpdatePointData_ModbusEquipment(const CRealTimeDataEntry& entry)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "insert into  "
		<< GetDBName() 
		<< ".realtimedata_output_modbus_equipment"
		<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".realtimedata_output_modbus_equipment"
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		sqlstatement = sqlstream.str();
		bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_output_modbus_equipment err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool   Beopdatalink::CRealTimeDataAccess::UpdatePointData_BacnetPy(const CRealTimeDataEntry& entry)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "insert into  "
		<< GetDBName() 
		<< ".realtimedata_output_bacnetpy"
		<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".realtimedata_output_bacnetpy"
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		sqlstatement = sqlstream.str();
		bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_output_bacnet_py err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool  Beopdatalink::CRealTimeDataAccess::UpdatePointData_OPCUA(const CRealTimeDataEntry& entry)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "insert into  "
		<< GetDBName() 
		<< ".realtimedata_output_opcua"
		<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".realtimedata_output_opcua"
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		sqlstatement = sqlstream.str();
		bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_output_opcua err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}


bool  Beopdatalink::CRealTimeDataAccess::UpdatePointData_ThirdParty(const CRealTimeDataEntry& entry)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "insert into  "
		<< GetDBName() 
		<< ".realtimedata_output_thirdparty"
		<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".realtimedata_output_thirdparty"
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		sqlstatement = sqlstream.str();
		bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_output_thirdparty err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool Beopdatalink::CRealTimeDataAccess::UpdatePointData_Wireless( const CRealTimeDataEntry& entry )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "insert into  "
		<< GetDBName() 
		<< ".realtimedata_output_wireless"
		<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".realtimedata_output_wireless"
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		sqlstatement = sqlstream.str();
		bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_output err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool Beopdatalink::CRealTimeDataAccess::UpdatePointData( vector<CRealTimeDataEntry>& entrylist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	if(entrylist.size()<=0)
		return false;

	std::ostringstream sqlstream;
	string value_ansi;

	StartTransaction();

	for(int i=0; i<entrylist.size(); i++)
	{
		CRealTimeDataEntry entry = entrylist[i];
		Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< "."
			<< tablename_output 
			<< " value " << "('" << entry.GetName() << "', '" << value_ansi << "')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);
		if (!bresult)
		{
			sqlstream.str("");
			sqlstream << "update  "
				<< GetDBName()
				<< "."
				<< tablename_output
				<< " set pointvalue= '" << value_ansi
				<< "' where pointname='" << entry.GetName() << "'";
			sqlstatement = sqlstream.str();
			bresult = Execute(sqlstatement);
			if(!bresult)
			{
				string strMysqlErr = GetMysqlError();
				AddMysqlErrLog("update realtimedata_output err",strMysqlErr);
			}
		}
	}

	Commit();
	return true;
}

bool Beopdatalink::CRealTimeDataAccess::ReadRealTimeData_Output( vector<CRealTimeDataEntry>& entrylist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_output;

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return true;
	}
	entrylist.reserve(length);
	MYSQL_ROW  row = NULL;
	CRealTimeDataEntry entry;

	SYSTEMTIME  st;
	GetLocalTime(&st);

	while(row = FetchRow(result) )
	{
		entry.SetTimestamp(st);
		entry.SetName(row[0]);
		wstring wstrValue;
		wstring wstr_value_ansi;
		string str_value = row[1];
		//wstr_value_ansi = Project::Tools::AnsiToWideChar(str_name.data());
		Project::Tools::UTF8ToWideChar(str_value, wstr_value_ansi);
		bool bReplaced;
		wstring wstr_value_del_yinhao = Project::Tools::ReplaceInString(wstr_value_ansi, L"\'", L"\"", bReplaced);
		entry.SetValue(wstr_value_del_yinhao);
		entrylist.push_back(entry);
	}
	//FreeReturnResult(result);

	return true;
}

bool  Beopdatalink::CCommonDBAccess::WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemNameSub,const wstring &strItemValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);
	string strItemSub_utf8;
	Project::Tools::WideCharToUtf8(strItemNameSub, strItemSub_utf8);

	string strItemValue_utf8;
	Project::Tools::WideCharToUtf8(strItemValue, strItemValue_utf8);

	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "' and unitproperty02 = '"
		<< strItemSub_utf8  << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '" << strItemSub_utf8  << "', '" << strItemValue_utf8 << "','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);	
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
	}
	else
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".unit01"
			<< " set unitproperty03 = '" << strItemValue_utf8
			<< "' where unitproperty01 ='" << strItemName_utf8
			<< "' and unitproperty02 = '"
			<< strItemSub_utf8  << "'";

		string sqlstatement = sqlstream.str();
		bool bresult =   Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update unit01 err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool     Beopdatalink::CCommonDBAccess::WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);
	string strItemValue_utf8;
	Project::Tools::WideCharToUtf8(strItemValue, strItemValue_utf8);

	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '" << strItemValue_utf8 << "','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);	
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
	}
	else
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".unit01"
			<< " set unitproperty02 = '" << strItemValue_utf8
			<< "' where unitproperty01 ='" << strItemName_utf8 << "'";
		string sqlstatement = sqlstream.str();
		bool bresult =   Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update unit01 err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}


bool     Beopdatalink::CCommonDBAccess::WriteCoreDebugItemValue2(const wstring &strItemName, const wstring &strItemValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);
	string strItemValue_utf8;
	Project::Tools::WideCharToUtf8(strItemValue, strItemValue_utf8);

	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '', '" << strItemValue_utf8 << "','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
	}
	else
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".unit01"
			<< " set unitproperty03 = '" << strItemValue_utf8
			<< "' where unitproperty01 ='" << strItemName_utf8 << "'";
		string sqlstatement = sqlstream.str();
		bool bresult =   Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update unit01 err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool  Beopdatalink::CCommonDBAccess::ReadCoreDebugItemValue(wstring strItemName, wstring &strItemValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		return false;
	}

	MYSQL_ROW  row = NULL;
	if(row = FetchRow(result) )
	{
		strItemValue = Project::Tools::UTF8ToWideChar(row[1]);

		return true;
	}
	
	return false;

}


bool  Beopdatalink::CCommonDBAccess::ReadCoreDebugItemValue(wstring strItemName, wstring strItemSub, wstring &strItemValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);
	string strItemSub_utf8;
	Project::Tools::WideCharToUtf8(strItemSub, strItemSub_utf8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "' and unitproperty02 = '"
		<< strItemSub_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		return false;
	}

	MYSQL_ROW  row = NULL;
	if(row = FetchRow(result) )
	{
		strItemValue = Project::Tools::UTF8ToWideChar(row[2]);

		return true;
	}

	return false;

}

bool   Beopdatalink::CCommonDBAccess::ClearLogicOutputPointRecordBeforeTime(wstring wstrTime)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);


	string wstrTime_utf8;
	Project::Tools::WideCharToUtf8(wstrTime, wstrTime_utf8);


	std::ostringstream sqlstream;
	sqlstream << "delete from "
		<< GetDBName() << ".logic_output_point_record where pointtime <= '" << wstrTime_utf8<< "'";

	string sqlstatement = sqlstream.str();

	return Execute(sqlstatement);
}

wstring     Beopdatalink::CCommonDBAccess::ReadOrCreateCoreDebugItemValue(wstring strItemName)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return L"";
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '0','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);		
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
		return L"0";
	}
	
	MYSQL_ROW  row = NULL;
	if(row = FetchRow(result) )
	{
		return Project::Tools::UTF8ToWideChar(row[1]);
	}

}

bool     Beopdatalink::CCommonDBAccess::DeleteLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	

	string strThreadName_utf8;
	Project::Tools::WideCharToUtf8(strThreadName, strThreadName_utf8);
	string strLogicName_utf8;
	Project::Tools::WideCharToUtf8(strLogicName, strLogicName_utf8);
	string strSetType_utf8;
	Project::Tools::WideCharToUtf8(strSetType, strSetType_utf8);
	string strVariableName_utf8;
	Project::Tools::WideCharToUtf8(strVariableName, strVariableName_utf8);
	string strLinkName_utf8;
	Project::Tools::WideCharToUtf8(strLinkName, strLinkName_utf8);
	string strLinkType_utf8;
	Project::Tools::WideCharToUtf8(strLinkType, strLinkType_utf8);


	std::ostringstream sqlstream;
	sqlstream << "delete from "
		<< GetDBName() << ".unit02 where unitproperty01 = '" << strThreadName_utf8 
		<<"' and unitproperty02 = '" << strLogicName_utf8
		<<"' and unitproperty03 = '" << strSetType_utf8 
		<<"' and unitproperty04 = '" << strVariableName_utf8
		<<"' and unitproperty05 = '" << strLinkName_utf8
		<<"' and unitproperty06 = '" << strLinkType_utf8 <<"'";

	string sqlstatement = sqlstream.str();

	return Execute(sqlstatement);
}

bool Beopdatalink::CCommonDBAccess::ClearLogicParameters()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	char* strdbname= GetDBName();
	if (!strdbname)
		return false;

	std::ostringstream sqlstream;
	sqlstream << "delete from "
		<< GetDBName() << ".unit02";

	string sqlstatement = sqlstream.str();

	bool bExecute = Execute(sqlstatement);

	sqlstream.str("");
	sqlstream << "delete from "
		<< GetDBName() << ".filestorage where filename = '';";

	sqlstatement = sqlstream.str();

	bExecute =  Execute(sqlstatement) && bExecute;

	return bExecute;

}


bool     Beopdatalink::CCommonDBAccess::InsertLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType, wstring strCondition)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string strThreadName_utf8;
	Project::Tools::WideCharToUtf8(strThreadName, strThreadName_utf8);
	string strLogicName_utf8;
	Project::Tools::WideCharToUtf8(strLogicName, strLogicName_utf8);
	string strSetType_utf8;
	Project::Tools::WideCharToUtf8(strSetType, strSetType_utf8);
	string strVariableName_utf8;
	Project::Tools::WideCharToUtf8(strVariableName, strVariableName_utf8);
	string strLinkName_utf8;
	Project::Tools::WideCharToUtf8(strLinkName, strLinkName_utf8);
	string strLinkType_utf8;
	Project::Tools::WideCharToUtf8(strLinkType, strLinkType_utf8);
	string strCondition_utf8;
	Project::Tools::WideCharToUtf8(strCondition, strCondition_utf8);

	sqlstream.str("");
	sqlstream << "insert into  "
		<< GetDBName() 
		<< ".unit02"
		<< " value " << "('" <<strThreadName_utf8 << "', '" << strLogicName_utf8 << "', '" << strSetType_utf8<< "', '" << strVariableName_utf8
		<< "', '" << strLinkName_utf8<< "', '" << strLinkType_utf8<< "', '" << strCondition_utf8<< "','','','','','','','','')";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);		
	if(!bresult)
	{
		string strMysqlErr = GetMysqlError();
		AddMysqlErrLog("insert unit02 err",strMysqlErr);
	}

	return bresult;
}


bool     Beopdatalink::CCommonDBAccess::ReadLogicParameters(wstring &strThreadName, wstring &strLogicName, wstring &strSetType, wstring &strVariableName, wstring &strLinkName, wstring &strLinkType)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit02";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		return false;
	}

	MYSQL_ROW  row = NULL;

	if(row = FetchRow(result) )
	{
		strThreadName = Project::Tools::UTF8ToWideChar(row[0]);
		strLogicName = Project::Tools::UTF8ToWideChar(row[1]);
		strSetType = Project::Tools::UTF8ToWideChar(row[2]);
		strVariableName = Project::Tools::UTF8ToWideChar(row[3]);
		strLinkName = Project::Tools::UTF8ToWideChar(row[4]);
		strLinkType = Project::Tools::UTF8ToWideChar(row[5]);
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::ReadWarningConfig(vector<CWarningConfigItem>& configList)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select HHEnable, HEnable, LEnable, LLEnable, HHLimit, HLimit, LLimit, LLLimit, pointname, HHwarninginfo, \
				 Hwarninginfo, Lwarninginfo, LLwarninginfo, boolwarning, boolwarninginfo,boolwarninglevel, unitproperty01, unitproperty02,unitproperty03,unitproperty04,unitproperty05,\
				 script,ofPosition,ofSystem,ofDepartment,ofGroup,tag,id from " 
		<< GetDBName() 
		<< "."
		<< tablename_warningconfig
		<< "  order by pointname";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		return false;
	}
	configList.reserve(length);
	MYSQL_ROW  row = NULL;

	CWarningConfigItem wcItem;

	while(row = FetchRow(result) )
	{
		wcItem.m_bEnableHH = ATOI(row[0])==1;
		wcItem.m_bEnableH = ATOI(row[1])==1;
		wcItem.m_bEnableL = ATOI(row[2])==1;
		wcItem.m_bEnableLL = ATOI(row[3])==1;
		wcItem.m_fLimitHH = ATOF(row[4]);
		wcItem.m_fLimitH = ATOF(row[5]);
		wcItem.m_fLimitL = ATOF(row[6]);
		wcItem.m_fLimitLL = ATOF(row[7]);
		wcItem.m_strPointName = Project::Tools::UTF8ToWideChar(row[8]);
		wcItem.m_strWarningContent_HH = Project::Tools::UTF8ToWideChar(row[9]);
		wcItem.m_strWarningContent_H = Project::Tools::UTF8ToWideChar(row[10]);
		wcItem.m_strWarningContent_L = Project::Tools::UTF8ToWideChar(row[11]);
		wcItem.m_strWarningContent_LL = Project::Tools::UTF8ToWideChar(row[12]);
		wcItem.m_nWarningType = ATOI(row[13]);
		wcItem.m_strWarningContent = Project::Tools::UTF8ToWideChar(row[14]);
		wcItem.m_nWarningLevel = ATOI(row[15]);
		wcItem.m_strOfPosition =  Project::Tools::UTF8ToWideChar(row[22]);
		wcItem.m_strOfSystem =  Project::Tools::UTF8ToWideChar(row[23]);
		wcItem.m_strOfDepartment =  Project::Tools::UTF8ToWideChar(row[24]);
		wcItem.m_strOfGroup =  Project::Tools::UTF8ToWideChar(row[25]);
		wcItem.m_strTag =  Project::Tools::UTF8ToWideChar(row[26]);

		wcItem.m_strUnitProperty01 =  Project::Tools::UTF8ToWideChar(row[16]);
		wcItem.m_strUnitProperty02 =  Project::Tools::UTF8ToWideChar(row[17]);
		wcItem.m_strUnitProperty03 =  Project::Tools::UTF8ToWideChar(row[18]);
		wcItem.m_strUnitProperty04 =  Project::Tools::UTF8ToWideChar(row[19]);
		wcItem.m_strUnitProperty05 =  Project::Tools::UTF8ToWideChar(row[20]);

		wcItem.m_nRuleID = ATOI(row[27]);

		configList.push_back(wcItem);
	}

	return true;
}

bool Beopdatalink::CRealTimeDataAccess::DeleteRealTimeData_Output(vector<CRealTimeDataEntry>& entrylist)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	char* strdbname= GetDBName();
	if (!strdbname)
		return false;
	

	StartTransaction();

	for(int i=0;i<entrylist.size();i++)
	{
		std::ostringstream sqlstream;
		sqlstream << "delete from "
			<< GetDBName() << "." << tablename_output << " where pointname = \"" << entrylist[i].GetName() << "\"";

		string sqlstatement = sqlstream.str();

		bool bresult=  Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_input err",strMysqlErr);
		}
	}

	return Commit();
}


bool Beopdatalink::CRealTimeDataAccess::UpdatePointValue_InputBuffer( const CRealTimeDataEntry& entry )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.
	SYSTEMTIME  st;
	GetLocalTime(&st);
	string str_time = Project::Tools::SystemTimeToString(st);

	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".point_value_buffer "
		<< "where pointname = '" << entry.GetName() <<"'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".point_value_buffer"
			<< " value " << "('" <<str_time << "'," << entry.GetName() << "', '" << value_ansi << "')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert point_value_buffer err",strMysqlErr);
		}
		return bresult;
	}
	else
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".point_value_buffer"
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entry.GetName() << "'";
		string sqlstatement = sqlstream.str();
		bool bresult=  Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update point_value_buffer err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool	Beopdatalink::CRealTimeDataAccess::UpdatePointValue_InputArray(const vector<CRealTimeDataEntry> & entryList)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.
	SYSTEMTIME  st;
	GetLocalTime(&st);
	string str_time = Project::Tools::SystemTimeToString(st);
	string value_ansi;

	StartTransaction();

	for(int i=0;i<entryList.size();i++)
	{
		Project::Tools::WideCharToUtf8(entryList[i].GetValue(), value_ansi);
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< "."
			<< tablename_input
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << entryList[i].GetName() << "'";
		string sqlstatement = sqlstream.str();
		bool bresult=  Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_input err",strMysqlErr);
		}
	}

	Commit();

	return true;
}

bool Beopdatalink::CRealTimeDataAccess::UpdatePointValue_Input( const CRealTimeDataEntry& entry )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.
	SYSTEMTIME  st;
	GetLocalTime(&st);
	string str_time = Project::Tools::SystemTimeToString(st);
	string value_ansi;
	Project::Tools::WideCharToUtf8(entry.GetValue(), value_ansi);

	sqlstream << "select * from "
		<< GetDBName() 
		<< "."
		<< tablename_input 
		<< " where pointname =  '" <<entry.GetName() << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
	
	sqlstream.str("");
	sqlstream << "update  "
		<< GetDBName()
		<< "."
		<< tablename_input
		<< " set pointvalue= '" << value_ansi
		<< "' where pointname='" << entry.GetName() << "'";
	string sqlstatement = sqlstream.str();
	bool bresult=  Execute(sqlstatement);
	if(!bresult)
	{
		string strMysqlErr = GetMysqlError();
		AddMysqlErrLog("update realtimedata_input err",strMysqlErr);
	}

	return bresult;
}

bool Beopdatalink::CRealTimeDataAccess::DeleteEntryInOutput( const string& pointname )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	char* strdbname= GetDBName();
	if (!strdbname)
		return false;

	std::ostringstream sqlstream;
	sqlstream << "delete from "
		<< GetDBName() << "." << tablename_output 
		<< " where pointname = '" << pointname << "'";

	string sqlstatement = sqlstream.str();

	return Execute(sqlstatement);
}

bool	Beopdatalink::CLogDBAccess::InsertLog(const vector<SYSTEMTIME> & sysTimeList, const vector<wstring> & loginfoList)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	SYSTEMTIME st;
	GetLocalTime(&st);
	char buff[64];

	sprintf_s(buff, "log_%d_%02d_%02d", st.wYear, st.wMonth, st.wDay);
	string newtablename = buff;
	std::ostringstream sqlstream;
	sqlstream << "create table if not exists " << newtablename <<  sql_createlog;

	bool bresult = Execute(sqlstream.str());
	if(!bresult)
	{
		string strMysqlErr = GetMysqlError();
		AddMysqlErrLog("create table err",strMysqlErr);
		return false;
	}
	
	SetConnectionCharacterSet(MYSQL_UTF8);

	StartTransaction();

	//generate the insert string
	for (int i=0;i<loginfoList.size(); i++)
	{
		sqlstream.str("");

		string log_utf8, time_utf8;
		Project::Tools::WideCharToUtf8(loginfoList[i], log_utf8);
		time_utf8 = Project::Tools::SystemTimeToString(sysTimeList[i]);

		sqlstream << "insert into "
			<< "domlog"
			<<"."
			<< newtablename 
			<< " value"
			<< "('" << time_utf8 << "','" << log_utf8 << "')"; 
		string strsql = sqlstream.str();
		if(strsql.length() > 1000)
			strsql = strsql.substr(0,1000);
		bool bresult=  Execute(strsql);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert log err",strMysqlErr);
		}
	}
	Commit();

	return true;
}

bool Beopdatalink::CLogDBAccess::InsertLog( const wstring& loginfo )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	SYSTEMTIME st;
	GetLocalTime(&st);
	char buff[64];

	sprintf_s(buff, "log_%d_%02d_%02d", st.wYear, st.wMonth, st.wDay);
	string newtablename = buff;
	std::ostringstream sqlstream;
	sqlstream << "create table if not exists " << newtablename <<  sql_createlog;

	bool bresult = Execute(sqlstream.str());
	if(!bresult)
	{
		string strMysqlErr = GetMysqlError();
		AddMysqlErrLog("create table err",strMysqlErr);
		return false;
	}
	
	SetConnectionCharacterSet(MYSQL_UTF8);
	//generate the insert string
	string loginfo_utf8, strtime;
	Project::Tools::WideCharToUtf8(loginfo, loginfo_utf8);
	strtime = Project::Tools::SystemTimeToString(st);

	sqlstream.str("");
	sqlstream << "insert into "
		<< "domlog"
		<<"."
		<< newtablename 
		<< " value"
		<< "('" << strtime << "','" << loginfo_utf8 << "')"; 

	string sqlstatement = sqlstream.str();

	/*string sqlstatement = "insert into";
	sqlstatement += GetDBName();
	sqlstatement += ".";
	sqlstatement += tablename_log;
	sqlstatement += " value ('";
	sqlstatement += strtime;
	sqlstatement += "','";
	sqlstatement += loginfo_utf8;
	sqlstatement += "');";*/
	return Execute(sqlstatement);
}

bool Beopdatalink::CLogDBAccess::ReadLog( vector< pair<wstring,wstring> >& loglist, const SYSTEMTIME& start, const SYSTEMTIME& end )
{//不能跨日期
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	string str_start = Project::Tools::SystemTimeToString(start);
	string str_end = Project::Tools::SystemTimeToString(end);

	char buff[64];
	sprintf_s(buff, "log_%d_%02d_%02d", end.wYear, end.wMonth, end.wDay);
	string newtablename = buff;


	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< "domlog"
		<< "."
		<< newtablename
		<< " where time >= '" << str_start << "' and time <= '" << str_end << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	loglist.reserve(length);
	MYSQL_ROW  row = NULL;
	
	while(row = FetchRow(result) ){
		wstring logtime = Project::Tools::UTF8ToWideChar(row[0]);
		wstring loginfo = Project::Tools::UTF8ToWideChar(row[1]);
		loglist.push_back(make_pair(logtime,loginfo));
	}
	
	return true;
}

double Beopdatalink::CCommonDBAccess::GetMySQLDBVersion()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	//string str_start = Project::Tools::SystemTimeToString(start);
	//string str_end = Project::Tools::SystemTimeToString(end);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< "beopinfo where infoname = 'version'";
	//<< " where time >= '" << str_start << "' and time <= '" << str_end << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return 0.f;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return 0.f;
	}

	MYSQL_ROW  row = NULL;

	CUserOnlineEntry entry;
	SYSTEMTIME st;
	if(row = FetchRow(result) )
	{
		wstring strVersion = Project::Tools::UTF8ToWideChar(row[1]);
		return _wtof(strVersion.c_str());
	}

	return 0.f;
}

bool Beopdatalink::CRealTimeDataAccess::UpdateRealTimeDatas_Input( const vector<CRealTimeDataEntry>& entrylist )
{
	if (entrylist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	StartTransaction();

	//generate the insert string
	std::ostringstream sqlstream;

	vector<CRealTimeDataEntry>::const_iterator it = entrylist.begin();
	for (;it != entrylist.end(); ++it)
	{
		sqlstream.str("");

		string value_ansi;
		Project::Tools::WideCharToUtf8((*it).GetValue(), value_ansi);

		sqlstream << "update  "
			<< GetDBName()
			<< "."
			<< tablename_input
			<< " set pointvalue= '" << value_ansi
			<< "' where pointname='" << (*it).GetName() << "'";
		string strsql = sqlstream.str();
		bool bresult = Execute(strsql);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update realtimedata_input err",strMysqlErr);
		}
	}
	Commit();

	return true;
}

bool Beopdatalink::CRealTimeDataAccess::DeleteRealTimeData_PointBuffer()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	char* strdbname= GetDBName();
	if (!strdbname)
		return false;

	std::ostringstream sqlstream;
	sqlstream << "delete from "
		<< GetDBName() << ".point_value_buffer";

	string sqlstatement = sqlstream.str();

	return Execute(sqlstatement);
}

bool Beopdatalink::CRealTimeDataAccess::InsertPointBuffer( const vector<CRealTimeDataEntry>& datalist )
{
	if (datalist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".point_value_buffer";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<CRealTimeDataEntry>::const_iterator it = datalist.begin();
	bool bSuccess = true;
	for (;it != datalist.end(); ++it)
	{
		string value_ansi;
		Project::Tools::WideCharToUtf8((*it).GetValue(), value_ansi);
		string time_ansi = Project::Tools::SystemTimeToString((*it).GetTimestamp());;

		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".point_value_buffer "
			<< " value"
			<< "('" << time_ansi << "', '"  << (*it).GetName() << "' , '"<< value_ansi<<"')"; 

		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert point_value_buffer err",strMysqlErr);
		}
	}

	return Commit();
}

bool Beopdatalink::CRealTimeDataAccess::InsertUnit01( const vector<unitStateInfo>& unitlist ,const string reip)
{
	if (unitlist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit01";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<unitStateInfo>::const_iterator it = unitlist.begin();
	bool bSuccess = true;
	for (;it != unitlist.end(); ++it)
	{
		string strUnitP01 = (*it).unitproperty01;
		string strUnitP02 = (*it).unitproperty02;
		if(strUnitP01 == "redundencyip")
		{
			//strUnitP02 为本机IP
			strUnitP02 = reip;
		}
		string strUnitP03 = (*it).unitproperty03;

		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".unit01 "
			<< " value"
			<< "('" << strUnitP01 << "', '"  << strUnitP02 << "' , '"<< strUnitP03<<"','','','','','','','','','','','','');"; 

		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
	}

	return Commit();
}

bool     Beopdatalink::CRealTimeDataAccess::WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	// first, check whether

	std::ostringstream sqlstream;
	// try to insert the entry.

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);
	string strItemValue_utf8;
	Project::Tools::WideCharToUtf8(strItemValue, strItemValue_utf8);

	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '" << strItemValue_utf8 << "','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);	
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
	}
	else
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".unit01"
			<< " set unitproperty02 = '" << strItemValue_utf8
			<< "' where unitproperty01 ='" << strItemName_utf8 << "'";
		string sqlstatement = sqlstream.str();
		bool bresult =   Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update unit01 err",strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool Beopdatalink::CRealTimeDataAccess::InsertErrInfo( const vector<ErrInfo> errlist )
{
	if (errlist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit07";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<ErrInfo>::const_iterator it = errlist.begin();
	bool bSuccess = true;
	for (;it != errlist.end(); ++it)
	{
		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".unit07 "
			<< " value"
			<< "('" << (*it).nErrCode << "', '"  << (*it).strErrInfo << "' , '','','','','','','','','','','','','');"; 

		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit07 err",strMysqlErr);
		}
	}

	return Commit();
}

bool Beopdatalink::CRealTimeDataAccess::InsertErrCodeInfo( const vector<ErrCodeInfo> errlist )
{
	if (errlist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit08";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<ErrCodeInfo>::const_iterator it = errlist.begin();
	bool bSuccess = true;
	for (;it != errlist.end(); ++it)
	{
		wstring strTime;
		Project::Tools::OleTime_To_String((*it).oleErrTime,strTime);

		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".unit08 "
			<< " value"
			<< "('" << (*it).nErrCode << "', '"  << Project::Tools::WideCharToAnsi(strTime.c_str()) << "' , '','','','','','','','','','','','','');"; 

		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit08 err",strMysqlErr);
		}
	}

	return Commit();
}

bool Beopdatalink::CRealTimeDataAccess::InsertErrPoints( const vector<ErrPointInfo> errlist )
{
	if (errlist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit09";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<ErrPointInfo>::const_iterator it = errlist.begin();
	bool bSuccess = true;
	for (;it != errlist.end(); ++it)
	{
		wstring strTime;
		Project::Tools::OleTime_To_String((*it).oleErrTime,strTime);
		//点名 时间 错误代码 类型
		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".unit09 "
			<< " value"
			<< "('" << (*it).strPointName << "', '"  << Project::Tools::WideCharToAnsi(strTime.c_str()) 
			<< "','" << (*it).nErrCode << "','" << (*it).nPointType << "','','','','','','','','','','','');";
			
		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit09 err",strMysqlErr);
		}
	}

	return Commit();
}

string  Beopdatalink::CCommonDBAccess::GetHistoryTableNameByStoreCycle(const SYSTEMTIME &stNow, const POINT_STORE_CYCLE &tCycle)
{
	SYSTEMTIME st = stNow;
	char buff[64];

	if(tCycle==E_STORE_FIVE_SECOND)
	{
		st.wSecond = 5*(st.wSecond/5);
		sprintf_s(buff, "%s_5second_%d_%02d_%02d", tablename_historydata, st.wYear, st.wMonth, st.wDay);
	}
	else if(tCycle==E_STORE_ONE_MINUTE)
	{
		st.wSecond = 0;
		sprintf_s(buff, "%s_minute_%d_%02d_%02d", tablename_historydata, st.wYear, st.wMonth, st.wDay);
	}
	else if(tCycle==E_STORE_FIVE_MINUTE)
	{	
		st.wMinute = 5*(st.wMinute/5);
		sprintf_s(buff, "%s_5minute_%d_%02d_%02d", tablename_historydata, st.wYear, st.wMonth, st.wDay);
	}
	else if(tCycle==E_STORE_ONE_HOUR)
	{	
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_hour_%d_%02d", tablename_historydata, st.wYear, st.wMonth);
	}
	else if(tCycle==E_STORE_ONE_DAY)
	{	
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_day_%d", tablename_historydata, st.wYear);
	}
	else if(tCycle==E_STORE_ONE_MONTH)
	{	
		st.wDay = 1;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_month_%02d", tablename_historydata, st.wYear);
	}
	else if(tCycle==E_STORE_ONE_YEAR)
	{	
		st.wMonth = 1;
		st.wDay = 1;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		sprintf_s(buff, "%s_year", tablename_historydata);
	}

	string newtablename = buff;

	return newtablename;
}

bool Beopdatalink::CCommonDBAccess::CheckCreateHistoryTable(const SYSTEMTIME &stNow, const POINT_STORE_CYCLE &tCycle)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);


	string newtablename = GetHistoryTableNameByStoreCycle(stNow, tCycle);

	std::ostringstream sqlstream;
	sqlstream << "SELECT * FROM information_schema.tables WHERE table_schema = '" << GetDBName() 
		<< "' AND table_name = '" << newtablename << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false; //没有创建
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));

	if (length > 0)
		return false;
	else if(length ==0)
	{ //没有查找到存储表，将创建
		std::ostringstream sqlstreamCreate;
		sqlstreamCreate << "create table if not exists " << newtablename <<  sql_createtable;

		string sqlstatement = sqlstreamCreate.str();
		bool bresult = Execute(sqlstatement);
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("create table err",strMysqlErr);
			return false;
		}
		return true; //创建成功
	}

	return false;// 默认返回没有创建
}

bool Beopdatalink::CCommonDBAccess::InsertHistoryData(const SYSTEMTIME &stNow, const POINT_STORE_CYCLE &tCycle, const vector<CRealTimeDataEntry>& datalist,int nMaxInsert )
{
	// first, generate table name
	if (datalist.empty() ){
		return true;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	string newtablename = GetHistoryTableNameByStoreCycle(stNow, tCycle);

	StartTransaction();

	std::ostringstream sqlstream;
	sqlstream.str("");
	sqlstream << "insert into "
		<< GetDBName()
		<<"."
		<< newtablename 
		<< " values";

	//const SYSTEMTIME& sttime = datalist[0].GetTimestamp();
	string str_temp = Project::Tools::SystemTimeToString(stNow);
	vector<CRealTimeDataEntry>::const_iterator it = datalist.begin();
	int nCount = 0;
	bool bInsertResult = true;
	for (;it != datalist.end(); ++it)
	{
		string value_ansi;
		Project::Tools::WideCharToUtf8((*it).GetValue(), value_ansi);

		sqlstream << "('" << str_temp << "','" 
			<< (*it).GetName() << "', '" 
			<< value_ansi << "'),";

		nCount++;
		if(nCount == nMaxInsert)
		{
			string sql_temp = sqlstream.str();
			sql_temp.erase(--sql_temp.end());
			bool bresult = Execute(sql_temp);
			if(!bresult)
			{
				string strMysqlErr = GetMysqlError();
				AddMysqlErrLog("insert history err",strMysqlErr);
				bInsertResult = false;
			}

			sqlstream.str("");
			sqlstream << "insert into "
				<< GetDBName()
				<<"."
				<< newtablename 
				<< " values";
			nCount = 0;
		}
	}
	string sql_temp = sqlstream.str();
	sql_temp.erase(--sql_temp.end());
	bool bresult = Execute(sql_temp);
	if(!bresult)
	{
		string strMysqlErr = GetMysqlError();
		AddMysqlErrLog("insert history err",strMysqlErr);
		AddMysqlErrLog("insert history err data",sql_temp);
		bInsertResult = false;
	}
	Commit();
	return bInsertResult;
}

bool   Beopdatalink::CCommonDBAccess::InsertPointBuffer(const vector<CRealTimeDataEntry>& datalist)
{
	if (datalist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	vector<CRealTimeDataEntry>::const_iterator it = datalist.begin();
	bool bSuccess = true;
	for (;it != datalist.end(); ++it)
	{
		string value_ansi;
		Project::Tools::WideCharToUtf8((*it).GetValue(), value_ansi);
		string time_ansi = Project::Tools::SystemTimeToString((*it).GetTimestamp());;

		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".point_value_buffer "
			<< " value"
			<< "('" << time_ansi << "', '"  << (*it).GetName() << "' , '"<< value_ansi<<"')"; 

		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert point_value_buffer err",strMysqlErr);
		}
	}

	return Commit();
}


bool   Beopdatalink::CCommonDBAccess::UpdatePointBuffer(const vector<CRealTimeDataEntry>& datalist)
{
	if (datalist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);
	std::ostringstream sqlstream;

	StartTransaction();

	vector<CRealTimeDataEntry>::const_iterator it = datalist.begin();
	for (;it != datalist.end(); ++it)
	{
		string value_utf8;
		Project::Tools::WideCharToUtf8((*it).GetValue(), value_utf8);
		string time_ansi = Project::Tools::SystemTimeToString((*it).GetTimestamp());
		string name_ansi = (*it).GetName();
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< ".point_value_buffer"
			<< " set pointvalue= '" << value_utf8
			<< "', time = '"<<time_ansi<<"'  where pointname='" << name_ansi << "'";
		string sqlstatement = sqlstream.str();
		bool bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update point_value_buffer err",strMysqlErr);
			break;
		}
	}



	bool bResult2 = Commit();

	//WriteCoreDebugItemValue(L"reset", L"心海");
	return bResult2;
}

//edit by luo 20140613
//根据info读最后1条报警，若confiremed=2，则新增1条报警，若为0，则更新结束时间endtime
//bool Beopdatalink::CCommonDBAccess::AddWarning( const CWarningEntry& entry )
//{
//	bool bWarningExist = false;
//	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
//	SetConnectionCharacterSet(MYSQL_UTF8);
//
//	string warninginfo_utf8;
//	Project::Tools::WideCharToUtf8(entry.GetWarningInfo(), warninginfo_utf8);
//
//	string strNewEndTime_ansi = Project::Tools::SystemTimeToString(entry.GetEndTimestamp());
//
//	std::ostringstream sqlstream;
//	sqlstream << "select * from " 
//		<< GetDBName() 
//		<< "."
//		<< tablename_warningrecord
//		<< " where info = '" << warninginfo_utf8 << "'  order by time desc limit 0,1;";
//
//	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
//	if (result == NULL){
//		return false;
//	}
//	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
//
//	MYSQL_ROW  row = NULL;
//	int nConfirmed = 2;
//	string strHappenTime;
//	while(row = FetchRow(result) ){
//		nConfirmed = ATOI(row[5]);
//		strHappenTime = row[0];
//	}
//
//	if(nConfirmed != 0)			//不为0就新增
//	{
//		string strtime = Project::Tools::SystemTimeToString(entry.GetTimestamp());
//		string strPointName;
//		Project::Tools::WideCharToUtf8(entry.GetWarningPointName(),strPointName);
//
//		//generate the insert string
//		std::ostringstream sqlstreamInsert;
//		sqlstreamInsert << "insert into "
//			<< GetDBName()
//			<<"."
//			<< tablename_warningrecord 
//			<< " value"
//			<< "('" << strtime << "',"  << entry.GetWarningCode() << ",'" <<  warninginfo_utf8 << "',"
//			<< entry.GetWarningLevel() << ", '"  << strNewEndTime_ansi <<"', 0, '', '"<< strPointName<<"')"; 
//
//		string sqlstatement = sqlstreamInsert.str();
//		bool bSuccess = Execute(sqlstatement);
//		if(!bSuccess)
//		{
//			string strMysqlErr = GetMysqlError();
//			AddMysqlErrLog("insert warningrecord err",strMysqlErr);
//		}
//		return bSuccess;
//	}
//	else
//	{
//		//generate the insert string
//		std::ostringstream sqlstreamUpdate;
//		sqlstreamUpdate << "update "
//			<< GetDBName()
//			<<"."
//			<< tablename_warningrecord 
//			<< " set endtime = '"<< strNewEndTime_ansi << "' "
//			<< " where info = '" << warninginfo_utf8 << "'  and time='" << strHappenTime << "';";
//
//		string sqlstatement = sqlstreamUpdate.str();
//		bool bSuccess = Execute(sqlstatement);
//		if(!bSuccess)
//		{
//			string strMysqlErr = GetMysqlError();
//			AddMysqlErrLog("update warningrecord err",strMysqlErr);
//		}
//		return bSuccess;
//	}
//}


//edit by luo 20141022  去除报警处理逻辑
//先判断数据库有没有该条报警记录，若无则新增一条
//根据info读最后1条报警，若confiremed=2，则新增1条报警
//若confiremed=0，且结束时间与当前差在10分钟以内则更新结束时间endtime
//若confiremed=0，且结束时间与当前差超过10分钟以内则新插入一条报警

int Beopdatalink::CCommonDBAccess::GetMaxIDInTable(string strTableName)
{
	std::ostringstream sqlstream;
	sqlstream.str("");
	sqlstream << "SELECT max(id) FROM "<< strTableName;

	int nMaxID = 1;
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{

	}
	else
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


		if (result){	
			MYSQL_ROW row = NULL;
			if (row = mysql_fetch_row(result)){
				nMaxID = ATOI(row[0]);
			}
		}
	}


	return nMaxID;
}


bool Beopdatalink::CCommonDBAccess::AddWarning( const CWarningEntry& entry )
{
	bool bWarningExist = false;
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string warninginfo_utf8;
	Project::Tools::WideCharToUtf8(entry.GetWarningInfo(), warninginfo_utf8);


	string warninginfo_detail_utf8;
	Project::Tools::WideCharToUtf8(entry.GetWarningInfoDetail(), warninginfo_detail_utf8);

	string strNewEndTime_ansi = Project::Tools::SystemTimeToString(entry.GetEndTimestamp());
	string strPointName;
	Project::Tools::WideCharToUtf8(entry.GetWarningPointName(),strPointName);


	string strWarningRecordTableName = GetWarningRecordTableName(COleDateTime::GetCurrentTime());

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< strWarningRecordTableName
		<< " where ruleId = '" << entry.GetRuleID() << "' and info = '"<< warninginfo_utf8 <<"' order by time desc limit 0,1;";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	MYSQL_ROW  row = NULL;
	int nConfirmed = 2;
	int nCurWaringID = -1;
	string strHappenTime,strEndTime;
	while(row = FetchRow(result) ){
		nConfirmed = ATOI(row[5]);
		strHappenTime = row[0];
		strEndTime = row[4];
		nCurWaringID = ATOI(row[8]);
	}


	if(nConfirmed != 0)			//不为0（nConfirmed为2,或不存在）新增
	{
		//generate the insert string

		int nNewId = GetMaxIDInTable(strWarningRecordTableName)+1;

		std::ostringstream sqlstreamInsert;
		sqlstreamInsert << "insert into "
			<< GetDBName()
			<<"."
			<< strWarningRecordTableName 
			<< "(time,code,info,level,endtime,confirmed,confirmeduser,bindpointname,id, ruleId, ofPosition, ofSystem, ofDepartment, ofGroup,tag, infodetail, unitproperty01, unitproperty02,unitproperty03, unitproperty04, unitproperty05) value"
			<< "('" << strNewEndTime_ansi << "',"  << entry.GetWarningCode() << ",'" <<  warninginfo_utf8 << "',"
			<< entry.GetWarningLevel() << ", '"  << strNewEndTime_ansi <<"', 0, '', '"<< strPointName<<"','"<<nNewId<<"','"<<entry.GetRuleID()
			<<"','" << entry.m_strOfPosition <<"','" << entry.m_strOfSystem <<"','"<< entry.m_strOfDepartment <<"','"<< entry.m_strOfGroup <<"','"
			<< entry.m_strTag<<"','"<< warninginfo_detail_utf8 <<"','"<< entry.m_strGoodRange <<"','"<< entry.m_strBindPointValue <<"','"
			<< entry.m_strUnitProperty03 <<"','"<< entry.m_strUnitProperty04  <<"','"<< entry.m_strUnitProperty05 <<"')"; 

		string sqlstatement = sqlstreamInsert.str();
		bool bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert warningrecord err",strMysqlErr);
		}
		return bSuccess;
	}
	else
	{
		//generate the insert string
		SYSTEMTIME sNewEndTime,sEndTime;
		Project::Tools::String_To_SysTime(strNewEndTime_ansi, sNewEndTime);
		Project::Tools::String_To_SysTime(strEndTime, sEndTime);
		COleDateTime oleNewEnd(sNewEndTime);
		COleDateTime oleEnd(sEndTime);
		COleDateTimeSpan oleTimeSpan = oleNewEnd -oleEnd;
		int nMinuteSpan = oleTimeSpan.GetTotalMinutes();
		if(nMinuteSpan >= 15)		//新插入
		{
			int nNewId = GetMaxIDInTable(strWarningRecordTableName)+1;
			std::ostringstream sqlstreamInsert;
			sqlstreamInsert << "insert into "
				<< GetDBName()
				<<"."
				<< strWarningRecordTableName 
				<< "(time,code,info,level,endtime,confirmed,confirmeduser,bindpointname,id, ruleId, ofPosition, ofSystem, ofDepartment, ofGroup,tag,  infodetail, unitproperty01, unitproperty02,unitproperty03, unitproperty04, unitproperty05) value"
				<< "('" << strNewEndTime_ansi << "',"  << entry.GetWarningCode() << ",'" <<  warninginfo_utf8 << "',"
				<< entry.GetWarningLevel() << ", '"  << strNewEndTime_ansi <<"', 0, '', '"<< strPointName<<"','"<<nNewId<<"','"<<entry.GetRuleID()
				<<"','" << entry.m_strOfPosition <<"','" << entry.m_strOfSystem <<"','"<< entry.m_strOfDepartment <<"','"<< entry.m_strOfGroup <<"','"<< entry.m_strTag
				<<"','"<< warninginfo_detail_utf8 <<"','"<< entry.m_strGoodRange <<"','"<< entry.m_strBindPointValue <<"','"
				<< entry.m_strUnitProperty03 <<"','"<< entry.m_strUnitProperty04  <<"','"<< entry.m_strUnitProperty05 <<"')"; 

			string sqlstatement = sqlstreamInsert.str();
			bool bSuccess = Execute(sqlstatement);
			if(!bSuccess)
			{
				string strMysqlErr = GetMysqlError();
				AddMysqlErrLog("insert warningrecord err",strMysqlErr);
			}
			return bSuccess;
		}
		else
		{
			std::ostringstream sqlstreamUpdate;
			sqlstreamUpdate << "update "
				<< GetDBName()
				<<"."
				<< strWarningRecordTableName 
				<< " set endtime = '"<< strNewEndTime_ansi << "' "
				<< " where id = '" << nCurWaringID << "';";

			string sqlstatement = sqlstreamUpdate.str();
			bool bSuccess = Execute(sqlstatement);
			if(!bSuccess)
			{
				string strMysqlErr = GetMysqlError();
				AddMysqlErrLog("update warningrecord err",strMysqlErr);
			}
			return bSuccess;
		}
	}
}

///*报警逻辑说明：luo
//	1:先判断数据库有没有该条报警记录，若无则新增一条
//	2：存在报警，若发生时间和结束时间在5分钟内，则只修改结束时间
//	3：存在报警，若发生时间和结束时间超过5分钟，则重置确认字段，并将发生时间和结束时间置为当前时间
//*/
//bool Beopdatalink::CCommonDBAccess::AddWarning( const CWarningEntry& entry )
//{
//	bool bWarningExist = false;
//	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
//	SetConnectionCharacterSet(MYSQL_UTF8);
//
//	string warninginfo_utf8;
//	Project::Tools::WideCharToUtf8(entry.GetWarningInfo(), warninginfo_utf8);
//
//	std::ostringstream sqlstream;
//	sqlstream << "select * from " 
//		<< GetDBName() 
//		<< "."
//		<< tablename_warningrecord
//		<< " where info = '" << warninginfo_utf8 <<"' order by time desc limit 0,1;";	//确保只有一条报警被选中
//
//	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
//	if (result == NULL){
//		bWarningExist = false;
//		return false;
//	}
//	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
//
//	int length = static_cast<int>(GetRowNum(result));
//	if (0 == length){
//		bWarningExist = false;
//	}
//	else
//		bWarningExist = true;
//	
//	if(bWarningExist) //
//	{
//		SYSTEMTIME tNow;
//		GetLocalTime(&tNow);
//		string strCurrentTime_ansi = Project::Tools::SystemTimeToString(tNow);
//
//		MYSQL_ROW  row = FetchRow(result);
//		SYSTEMTIME sHappenTime,sEndTime;
//		if (row != NULL){
//			Project::Tools::String_To_SysTime(row[0], sHappenTime);
//			Project::Tools::String_To_SysTime(row[4], sEndTime);
//		}
//
//		string strEndTime_ansi = Project::Tools::SystemTimeToString(sEndTime);
//
//		????? 莫名其妙的逻辑， by golding
//		/*COleDateTime oleStart(sHappenTime);
//		COleDateTime oleEnd(sEndTime);
//		COleDateTimeSpan oleTimeSpan = oleEnd -oleStart;
//		int nMinuteSpan = oleTimeSpan.GetTotalMinutes();
//		if(nMinuteSpan >= 5)
//		{
//			generate the insert string
//			std::ostringstream sqlstreamUpdate;
//			sqlstreamUpdate << "update "
//				<< GetDBName()
//				<<"."
//				<< tablename_warningrecord 
//				<< " set endtime = '"<< strCurrentTime_ansi << "',time='" << strCurrentTime_ansi << "',confirmed=0 "
//				<< " where info = '" << warninginfo_utf8 << "'  and endtime = '" << strEndTime_ansi <<"';";
//
//			string sqlstatement = sqlstreamUpdate.str();
//
//			return Execute(sqlstatement);
//		}
//		else
//		{*/
//			generate the insert string
//			std::ostringstream sqlstreamUpdate;
//			sqlstreamUpdate << "update "
//				<< GetDBName()
//				<<"."
//				<< tablename_warningrecord 
//				<< " set endtime = '"<< strCurrentTime_ansi << "' "
//				<< " where info = '" << warninginfo_utf8 << "'  and endtime = '" << strEndTime_ansi <<"';";
//
//			string sqlstatement = sqlstreamUpdate.str();
//			return Execute(sqlstatement);
//
//		}
//	}
//	else
//	{
//		string strHappenTime_ansi = Project::Tools::SystemTimeToString(entry.GetTimestamp());
//		string strEndTime_ansi = Project::Tools::SystemTimeToString(entry.GetEndTimestamp());
//		string strPointName;
//		Project::Tools::WideCharToUtf8(entry.GetWarningPointName(),strPointName);
//
//		generate the insert string
//		std::ostringstream sqlstreamInsert;
//		sqlstreamInsert << "insert into "
//			<< GetDBName()
//			<<"."
//			<< tablename_warningrecord 
//			<< " value"
//			<< "('" << strHappenTime_ansi << "',"  << entry.GetWarningCode() << ",'" <<  warninginfo_utf8 << "',"
//			<< entry.GetWarningLevel() << ", '"  << strEndTime_ansi <<"', 0, '', '"<< strPointName<<"');"; 
//
//		string sqlstatement = sqlstreamInsert.str();
//
//		return Execute(sqlstatement);
//	}
//
//}

bool Beopdatalink::CCommonDBAccess::ReadWarning( vector<CWarningEntry>& resultlist, const SYSTEMTIME& start, const SYSTEMTIME& end )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string str_start = Project::Tools::SystemTimeToString(start);
	string str_end = Project::Tools::SystemTimeToString(end);

	COleDateTime olet;
	Project::Tools::SysTime_To_OleTime(start, olet);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< GetWarningRecordTableName(olet)
		<< " where time >= '" << str_start << "' and time <= '" << str_end << "'  order by time DESC";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	resultlist.reserve(length);
	MYSQL_ROW  row = NULL;

	CWarningEntry entry;
	SYSTEMTIME st;
	while(row = FetchRow(result) ){
		Project::Tools::String_To_SysTime(row[0], st);
		entry.SetTimestamp(st);
		entry.SetWarningCode(ATOI(row[1]));
		wstring warninginfo = Project::Tools::UTF8ToWideChar(row[2]);
		entry.SetWarningInfo(warninginfo);
		entry.SetWarningLevel(ATOI(row[3]));

		Project::Tools::String_To_SysTime(row[4], st);
		entry.SetEndTimestamp(st);

		resultlist.push_back(entry);
	}

	return true;
}


bool Beopdatalink::CCommonDBAccess::DeleteWarning()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;
	sqlstream << "delete from " 
			  << GetDBName() 
			  << "."
			  << GetWarningRecordTableName(COleDateTime::GetCurrentTime());

	return Execute(sqlstream.str());
}

bool Beopdatalink::CCommonDBAccess::SetStatus( const wstring& statustype, int value )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	// first, check whether
	string name = Project::Tools::WideCharToAnsi(statustype.c_str());
	std::ostringstream sqlstream;
	// try to insert the entry.

	sqlstream << "insert into  "
		<< GetDBName() 
		<< "."
		<< tablename_datacom_status 
		<< " value " << "('" << name << "'," << value << ")";

	string sqlstatement = sqlstream.str();
	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< "."
			<< tablename_datacom_status
			<< " set status=" << value
			<< " where typename='" << name << "'";
		sqlstatement = sqlstream.str();
		return Execute(sqlstatement);
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::InsertErrInfo( const vector<Beopdatalink::ErrInfo> errlist )
{
	if (errlist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit07";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<ErrInfo>::const_iterator it = errlist.begin();
	bool bSuccess = true;
	for (;it != errlist.end(); ++it)
	{
		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".unit07 "
			<< " value"
			<< "('" << (*it).nErrCode << "', '"  << (*it).strErrInfo << "' , '','','','','','','','','','','','','');"; 

		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit07 err",strMysqlErr);
		}
	}

	return Commit();
}

bool Beopdatalink::CCommonDBAccess::InsertErrCodeInfo( const vector<Beopdatalink::ErrCodeInfo> errlist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit08";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	if(errlist.size() >0 )
	{
		vector<ErrCodeInfo>::const_iterator it = errlist.begin();
		bool bSuccess = true;
		for (;it != errlist.end(); ++it)
		{
			wstring strTime;
			Project::Tools::OleTime_To_String((*it).oleErrTime,strTime);

			sqlstream.str("");
			sqlstream << "insert into "
				<< GetDBName()
				<<".unit08 "
				<< " value"
				<< "('" << (*it).nErrCode << "', '"  << Project::Tools::WideCharToAnsi(strTime.c_str()) << "' , '','','','','','','','','','','','','');"; 

			string sqlstatement = sqlstream.str();
			bSuccess = Execute(sqlstatement);
			if(!bSuccess)
			{
				string strMysqlErr = GetMysqlError();
				AddMysqlErrLog("insert unit08 err",strMysqlErr);
			}
		}
	}
	return Commit();
}

bool Beopdatalink::CCommonDBAccess::InsertErrPoints( const vector<Beopdatalink::ErrPointInfo> errlist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit09";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	if(errlist.size() > 0)
	{
		vector<ErrPointInfo>::const_iterator it = errlist.begin();
		bool bSuccess = true;
		for (;it != errlist.end(); ++it)
		{
			wstring strTime;
			Project::Tools::OleTime_To_String((*it).oleErrTime,strTime);
			//点名 时间 错误代码 类型
			sqlstream.str("");
			sqlstream << "insert into "
				<< GetDBName()
				<<".unit09 "
				<< " value"
				<< "('" << (*it).strPointName << "', '"  << Project::Tools::WideCharToAnsi(strTime.c_str()) 
				<< "','" << (*it).nErrCode << "','" << (*it).nPointType << "','','','','','','','','','','','');";

			string sqlstatement = sqlstream.str();
			bSuccess = Execute(sqlstatement);
			if(!bSuccess)
			{
				string strMysqlErr = GetMysqlError();
				AddMysqlErrLog("insert unit09 err",strMysqlErr);
			}
		}
	}
	return Commit();
}

bool Beopdatalink::CCommonDBAccess::GetStatus( vector< pair<wstring, int> >& statuslist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_datacom_status;

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	statuslist.reserve(length);
	MYSQL_ROW  row = NULL;

	while(row = FetchRow(result) )
	{
		wstring statusname = Project::Tools::AnsiToWideChar(row[0]);
		int value = ATOI(row[1]);

		statuslist.push_back(make_pair(statusname, value));
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::SetPLCConnectionStatus( bool bconnected )
{
	return SetStatus(_T("PLC Connection"), bconnected ? 1 : 0);
}

bool Beopdatalink::CCommonDBAccess::GetPLCConnectionStatus()
{
	vector< pair<wstring, int> > statuslist;
	GetStatus(statuslist);
	for (unsigned int i = 0; i < statuslist.size(); i++)
	{
		const pair<wstring, int>& entry = statuslist[i];
		if (entry.first == _T("PLC Connection")){
			return entry.second != 0;
		}
	}

	return false;
}

bool Beopdatalink::CCommonDBAccess::GetModbusConnectionStatus()
{
	return true;
}

bool Beopdatalink::CCommonDBAccess::InsertOperationRecord(const SYSTEMTIME& st, const wstring& remark, const wstring& wstrUser)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	//string actioninfo_utf8;
	//Project::Tools::WideCharToUtf8(actioninfo, actioninfo_utf8);
	//string actionname_utf8;
	//Project::Tools::WideCharToUtf8(actionname, actionname_utf8);

	//SetConnectionCharacterSet(MYSQL_UTF8);
	////generate the insert string
	//std::ostringstream sqlstream;
	//sqlstream << "insert into "
	//	<< GetDBName()
	//	<<"."
	//	<< tablename_actioninfo 
	//	<< " value"
	//	<< "('" << actioninfo_utf8 << "','" << actionname_utf8 << "')"; 

	//string sqlstatement = sqlstream.str();

	//return Execute(sqlstatement);


	SetConnectionCharacterSet();

	const string strRemark = Project::Tools::WideCharToUtf8(remark.c_str());
	const string strUser = Project::Tools::WideCharToUtf8(wstrUser.c_str());

	std::ostringstream sqlstream;
	sqlstream	<< "insert into "
		<< GetDBName() << "."<<tablename_operationrecord <<" values('"				
		<< Project::Tools::SystemTimeToString(st) << "'"
		<< "," <<"'"<< strUser << "'"
		<< "," <<"'"<< strRemark << "'"
		<< ")";

	string sql = sqlstream.str();
	bool bResult = Execute(sql);
	if(!bResult)
	{
		string strMysqlErr = GetMysqlError();
		AddMysqlErrLog("insert operationrecord err",strMysqlErr);
	}
	//if (!bResult)
	//{
	//	LogDBError(sql);
	//}

	return bResult;
}

bool Beopdatalink::CCommonDBAccess::ReadAllOperationRecord(VEC_OPT_RECORD&  vecOperation, const COleDateTime& timeStart, const COleDateTime& timeStop)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	SetConnectionCharacterSet();
	bool ifok = false;

	//const int hourMax = 24;
	wstring wstrStart, wstrStop;
	string strStart, strStop;
	Project::Tools::OleTime_To_String(timeStart, wstrStart);
	Project::Tools::OleTime_To_String(timeStop, wstrStop);
	strStart = UtilString::ConvertWideCharToMultiByte(wstrStart);
	strStop = UtilString::ConvertWideCharToMultiByte(wstrStop);

	std::ostringstream sqlstream;
	//SELECT * FROM operation_record where recordtime >= '2011-05-30 00:00:00' and recordtime <= '2011-05-30 14:00:00';
	sqlstream << "select * from " << tablename_operationrecord <<" where recordtime >= '"<< strStart.c_str() <<"' and recordtime <= '"<< strStop.c_str() << "'";

	string sql = sqlstream.str();
	COperationRecords  tempOpera;
	vecOperation.clear();

	COleDateTime old;


	//MYSQL_RES* result = mysql_use_result(&m_mysql);
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


	MYSQL_ROW row = NULL;
	while (row = mysql_fetch_row(result))
	{
		if (row[0] && row[1] && row[2]){
			//daterange.push_back(row[0]);
			tempOpera.m_strTime = UtilString::ConvertMultiByteToWideChar(row[0] );
			Project::Tools::UTF8ToWideChar(row[1],tempOpera.m_strUser); //bird add, 0630
			Project::Tools::UTF8ToWideChar(row[2],tempOpera.m_strOperation);

			vecOperation.push_back(tempOpera);
		}
	}

	return true;

}

bool Beopdatalink::CCommonDBAccess::DeleteOperationRecord()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;
	sqlstream << "delete from " 
		<< GetDBName() 
		<< "."
		<< tablename_operationrecord;

	return Execute(sqlstream.str());
}

bool Beopdatalink::CLogDBAccess::ReadLog( vector< pair<wstring,wstring> >& loglist, const int nCount)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;
	
	char szCount[MAX_PATH] = {0};
	itoa(nCount, szCount, 10);
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_log<<" order by time desc limit "<<szCount;
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());

	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	loglist.reserve(length);
	MYSQL_ROW  row = NULL;
	
	while(row = FetchRow(result) ){
		wstring logtime = Project::Tools::UTF8ToWideChar(row[0]);
		wstring loginfo = Project::Tools::UTF8ToWideChar(row[1]);
		loglist.push_back(make_pair(logtime,loginfo));
	}
	
	return true;
}

bool    Beopdatalink::CLogDBAccess::DeleteLog(const SYSTEMTIME tEnd)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;
	string str_end = Project::Tools::SystemTimeToString(tEnd);
	sqlstream << "delete from "
		<< GetDBName() 
		<<"."
		<<tablename_log
		<< " where time <='"
		<< str_end
		<<"'";

	return Execute(sqlstream.str());
}

bool Beopdatalink::CLogDBAccess::ExportLog( const SYSTEMTIME& start, const SYSTEMTIME& end, const wstring& filepath )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	string filepath_ansi = Project::Tools::WideCharToAnsi(filepath.c_str());
	string str_start = Project::Tools::SystemTimeToString(start);
	string str_end = Project::Tools::SystemTimeToString(end);

	std::ostringstream sqlstream;
	sqlstream << "select * from "
			  << GetDBName() 
			  <<"."
			  <<tablename_log
			  << " where time >='"
			  << str_start
			  << "' and time <='"
			  << str_end
			  <<"'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());

	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}

	MYSQL_ROW  row = NULL;

	vector<wstring> vcline;
	while(row = FetchRow(result) ){
		wstring logtime = Project::Tools::UTF8ToWideChar(row[0]);
		wstring loginfo = Project::Tools::UTF8ToWideChar(row[1]);

		wstring strtemp = logtime + _T("    ")+ loginfo;
		vcline.push_back(strtemp);
	}
	//写文件
	{
		assert(filepath.size()>0);
		
		FILE* fd = NULL;
		const errno_t rs = _wfopen_s(&fd, filepath.c_str(), L"wt,ccs=UNICODE");
		assert(0 == rs);
		assert(fd);
		for (UINT i=0;i<vcline.size();++i)
		{
			fwprintf_s(fd, L"%s\n",vcline[i].c_str());
		}
		fclose(fd);
	}

	return true;
}


Beopdatalink::CUserOnlineEntry::CUserOnlineEntry()
{
	GetLocalTime(&m_time);
	m_priority = 0;
}

wstring Beopdatalink::CUserOnlineEntry::GetUserName() const
{
	return m_username;
}

void Beopdatalink::CUserOnlineEntry::SetUserName( const wstring& username )
{
	m_username = username;
}


wstring Beopdatalink::CUserOnlineEntry::GetUserHost() const
{
	return m_userhost;
}

void Beopdatalink::CUserOnlineEntry::SetUserHost( const wstring& userhost )
{
	m_userhost = userhost;
}


wstring Beopdatalink::CUserOnlineEntry::GetUserType() const
{
	return m_usertype;
}

void Beopdatalink::CUserOnlineEntry::SetUserType( const wstring& usertype )
{
	m_usertype = usertype;
}


int Beopdatalink::CUserOnlineEntry::GetUserPriority() const
{
	return m_priority;
}

void Beopdatalink::CUserOnlineEntry::SetUserPriority( int userpriority )
{
	m_priority = userpriority;
}

SYSTEMTIME Beopdatalink::CUserOnlineEntry::GetTimestamp() const
{
	return m_time;
}

void Beopdatalink::CUserOnlineEntry::SetTimestamp( const SYSTEMTIME& stime )
{
	m_time = stime;
}


bool Beopdatalink::CCommonDBAccess::RegisterUserRecord( const CUserOnlineEntry& entry )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	string strtime = Project::Tools::SystemTimeToString(entry.GetTimestamp());

	string username_utf8;
	Project::Tools::WideCharToUtf8(entry.GetUserName(), username_utf8);

	string userhost_utf8;
	Project::Tools::WideCharToUtf8(entry.GetUserHost(), userhost_utf8);

	string usertype_utf8;
	Project::Tools::WideCharToUtf8(entry.GetUserType(), usertype_utf8);

	SetConnectionCharacterSet(MYSQL_UTF8);

	//generate the insert string
	std::ostringstream sqlstream;
	sqlstream << "insert into "
		<< GetDBName()
		<<"."
		<< tablename_userrecord 
		<< " value"
		<< "('" << username_utf8 << "'" << ",'" <<  userhost_utf8 << "',"
		<< entry.GetUserPriority() << ",'" <<  usertype_utf8 << "'" << ",'" << strtime << "','','','','','')"; 

	string sqlstatement = sqlstream.str();

	bool bresult =  Execute(sqlstatement);
	if (!bresult)
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName()
			<< "."
			<< tablename_userrecord
			<< " set time='" << strtime << "', priority=" << entry.GetUserPriority() << " "
			<< " where username='" << username_utf8 << "'"
			<< " and usertype='" << usertype_utf8 << "'";

		sqlstatement = sqlstream.str();
		return Execute(sqlstatement);
	}

	return true;

}



bool Beopdatalink::CCommonDBAccess::UpdateUserOnlineTime(wstring username, wstring usertype, SYSTEMTIME time)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	string strtime = Project::Tools::SystemTimeToString(time);

	string username_utf8;
	Project::Tools::WideCharToUtf8(username, username_utf8);

	string usertype_utf8;
	Project::Tools::WideCharToUtf8(usertype, usertype_utf8);

	SetConnectionCharacterSet(MYSQL_UTF8);

	//generate the insert string
	std::ostringstream sqlstream;
	sqlstream << "update "
		<< GetDBName()
		<<"."
		<< tablename_userrecord 
		<< " set time='" << strtime << "'"
		<< " where username='" << username_utf8 << "'"
		<< " and usertype='" << usertype_utf8 << "'";

	string sqlstatement = sqlstream.str();

	return Execute(sqlstatement);
}




bool Beopdatalink::CCommonDBAccess::ReadUserRecord( vector<CUserOnlineEntry>& resultlist)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	//string str_start = Project::Tools::SystemTimeToString(start);
	//string str_end = Project::Tools::SystemTimeToString(end);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_userrecord;
		//<< " where time >= '" << str_start << "' and time <= '" << str_end << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	resultlist.reserve(length);
	MYSQL_ROW  row = NULL;

	CUserOnlineEntry entry;
	SYSTEMTIME st;
	while(row = FetchRow(result) )
	{
		wstring username = Project::Tools::UTF8ToWideChar(row[0]);
		entry.SetUserName(username);

		wstring userhost = Project::Tools::UTF8ToWideChar(row[1]);
		entry.SetUserHost(userhost);

		entry.SetUserPriority(ATOI(row[2]));

		wstring usertype = Project::Tools::UTF8ToWideChar(row[3]);
		entry.SetUserType(usertype);

		Project::Tools::String_To_SysTime(row[4], st);
		entry.SetTimestamp(st);

		resultlist.push_back(entry);
	}

	return true;
}


bool Beopdatalink::CCommonDBAccess::DeleteUserRecord(wstring username, wstring usertype)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	string username_utf8;
	Project::Tools::WideCharToUtf8(username, username_utf8);

	string usertype_utf8;
	Project::Tools::WideCharToUtf8(usertype, usertype_utf8);

	std::ostringstream sqlstream;
	sqlstream << "delete from " 
		<< GetDBName() 
		<< "."
		<< tablename_userrecord
		<< " where username = '" << username_utf8 << "' and usertype = '" << usertype_utf8 << "'";

	return Execute(sqlstream.str());
}


bool Beopdatalink::CCommonDBAccess::GetServerTime(SYSTEMTIME &tServerTime)
{
	bool bReadSuccess = false;
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;
	sqlstream << "select now()"; 

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	MYSQL_ROW  row = NULL;

	SYSTEMTIME st;
	if(row = FetchRow(result) )
	{
		Project::Tools::String_To_SysTime(row[0], st);
		tServerTime = st;
		bReadSuccess  = true;
	}

	return bReadSuccess;
}


bool Beopdatalink::CCommonDBAccess::IsUserOnline(wstring username, wstring usertype)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	vector<CUserOnlineEntry> userOnlinelist;

	if(ReadUserRecord(userOnlinelist))
	{
		for(int i=0;i<userOnlinelist.size();i++)
		{
			if((username == userOnlinelist[i].GetUserName()) &&
				(usertype == userOnlinelist[i].GetUserType()) )

			{
				COleDateTime   usetime(userOnlinelist[i].GetTimestamp());

				if((usertype == _T("dataengine")) || (usertype == _T("serverengine")))
				{
					std::ostringstream sqlstream;
					sqlstream << "select now()"; 

					MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
					if (result == NULL){
						return false;
					}
					Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

					int length = static_cast<int>(GetRowNum(result));
					if (0 == length){
						return false;
					}
					MYSQL_ROW  row = NULL;

					SYSTEMTIME st;
					if(row = FetchRow(result) )
					{
						Project::Tools::String_To_SysTime(row[0], st);
						COleDateTime   engineCurTime(st);

						COleDateTimeSpan enginespan = engineCurTime - usetime;	

						double tempsec = enginespan.GetTotalSeconds();
						if (tempsec < 1*60)	
						{	
							return true;
						}
					}
				}
				else
				{
					COleDateTimeSpan standbyspan = COleDateTime::GetCurrentTime() - usetime;	

					double temptotalsec = standbyspan.GetTotalSeconds();
					if (temptotalsec < 1*60)	
					{	
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool Beopdatalink::CCommonDBAccess::ReadPointBufferData( vector<CRealTimeDataEntry>& resultlist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	SYSTEMTIME st_time;
	GetLocalTime(&st_time);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName()
		<< ".point_value_buffer";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return true;
	}
	//resultlist.reserve(length);
	MYSQL_ROW  row = NULL;
	CRealTimeDataEntry entry;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		entry.SetTimestamp(sttime);
		entry.SetName(row[1]);


		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		entry.SetValue(wstrValue);

		resultlist.push_back(entry);
	}

	return true;
	
}

bool Beopdatalink::CCommonDBAccess::ReadPointBufferData( hash_map<string,CRealTimeDataEntry>& resultmap )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	SYSTEMTIME st_time;
	GetLocalTime(&st_time);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName()
		<< ".point_value_buffer";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return true;
	}
	//resultlist.reserve(length);
	MYSQL_ROW  row = NULL;
	CRealTimeDataEntry entry;

	while(row = FetchRow(result) )
	{   
		SYSTEMTIME sttime;
		Project::Tools::String_To_SysTime(row[0], sttime);
		entry.SetTimestamp(sttime);
		entry.SetName(row[1]);
		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		entry.SetValue(wstrValue);
		resultmap[row[1]] = entry;
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::InsertS3DBpath(const wstring& s3dbpathname,const wstring& s3dbpathvalue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	string s3dbpathname_utf8;
	Project::Tools::WideCharToUtf8(s3dbpathname, s3dbpathname_utf8);
	string s3dbpathvalue_utf8;
	Project::Tools::WideCharToUtf8(s3dbpathvalue, s3dbpathvalue_utf8);
	std::ostringstream sqlstream;
	//先删除
	sqlstream << "delete from " 
		<< GetDBName() 
		<< "."
		<< tablename_beopinfo
		<< " where  infoname = '" << s3dbpathname_utf8 << "'";
	Execute(sqlstream.str());
	//后插入
	SetConnectionCharacterSet(MYSQL_UTF8);
	std::ostringstream sqlstream_;
	sqlstream_ << "insert into "
		<< GetDBName()
		<<"."
		<< tablename_beopinfo
		<< " value"
		<< "('" << s3dbpathname_utf8 << "','"<< s3dbpathvalue_utf8 << "')";

	string sqlstatement = sqlstream_.str();
	return Execute(sqlstatement);
}

bool Beopdatalink::CCommonDBAccess::GetS3DBPath( const wstring& s3dbpathname,wstring& s3dbpathvalue )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);
	string s3dbpathname_utf8;
	Project::Tools::WideCharToUtf8(s3dbpathname, s3dbpathname_utf8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_beopinfo
		<< " where  infoname = '" << s3dbpathname_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	if (result == NULL)
	{
		return _T("");
	}

	MYSQL_ROW  row = NULL;
	string path;
	while(row = FetchRow(result) )
	{

		s3dbpathvalue = Project::Tools::UTF8ToWideChar(row[1]);
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::GetHistoryValue(const wstring& strName,const SYSTEMTIME &st,const int &nTimeFormat,  wstring &strParam)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	string histablename = GetHistoryTableNameByStoreCycle(st, (POINT_STORE_CYCLE)nTimeFormat);

	COleDateTime oleTimeStart(st);

	wstring wstrStart, wstrStop;
	string strStart, strStop;
	Project::Tools::OleTime_To_String(oleTimeStart, wstrStart);
	Project::Tools::WideCharToUtf8(wstrStart, strStart);

	string strPointName_ansi;
	Project::Tools::WideCharToUtf8(strName, strPointName_ansi);

	std::ostringstream sqlstream;
	sqlstream << "select * from " << histablename 
		<< " where time <= '" << strStart <<"' and pointname = '"
		<< strPointName_ansi
		<< "' order by time desc";


	string sql = sqlstream.str();
	// execute the query
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


	long rownum = GetRowNum(result);
	if (rownum == 0){
		return false;
	}

	MYSQL_ROW row = NULL;
	if(row = FetchRow(result))
	{
		strParam = Project::Tools::UTF8ToWideChar(row[2]);
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::GetHistoryValue(const wstring& strName,const SYSTEMTIME &stStart,const SYSTEMTIME &stEnd,const int &nTimeFormat, wstring &strParam)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	char buff[64];

	COleDateTimeSpan tspan;
	if(nTimeFormat==E_STORE_FIVE_SECOND)
	{
		tspan.SetDateTimeSpan(0,0,0,5);
	}
	else if(nTimeFormat==E_STORE_ONE_MINUTE)
	{
		tspan.SetDateTimeSpan(0,0,1,0);
	}
	else if(nTimeFormat==E_STORE_FIVE_MINUTE)
	{
		tspan.SetDateTimeSpan(0,0,5,0);
	}
	else if(nTimeFormat==E_STORE_ONE_HOUR)
	{
		tspan.SetDateTimeSpan(0,1,0,0);
	}
	else if(nTimeFormat==E_STORE_ONE_DAY)
	{
		tspan.SetDateTimeSpan(1,0,0,0);
	}
	else if(nTimeFormat==E_STORE_ONE_MONTH)
	{
		tspan.SetDateTimeSpan(30,0,0,0);
	}
	else if(nTimeFormat==E_STORE_ONE_YEAR)
	{
		tspan.SetDateTimeSpan(365,0,0,0);
	}

	string histablename = GetHistoryTableNameByStoreCycle(stStart, (POINT_STORE_CYCLE) nTimeFormat);

	COleDateTime oleTimeStart(stStart),oleTimeEnd(stEnd);

	wstring wstrStart, wstrStop;
	string strStart, strStop;

	Project::Tools::OleTime_To_String(oleTimeStart, wstrStart);
	Project::Tools::OleTime_To_String(oleTimeEnd, wstrStop);
	Project::Tools::WideCharToUtf8(wstrStart, strStart);
	Project::Tools::WideCharToUtf8(wstrStop, strStop);

	string strPointName_ansi;
	Project::Tools::WideCharToUtf8(strName, strPointName_ansi);

	std::ostringstream sqlstream;
	sqlstream << "select * from " << histablename 
		<< " where pointname = '"
		<< strPointName_ansi
		<< "' order by time";


	string sql = sqlstream.str();

	// execute the query
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


	long rownum = GetRowNum(result);
	if (rownum == 0){
		return false;
	}

	MYSQL_ROW row = NULL;
	vector<COleDateTime> tQueryList;
	vector<wstring>  strQueryList;

	while(row = FetchRow(result))
	{
		SYSTEMTIME st;
		Project::Tools::String_To_SysTime(row[0], st);
		COleDateTime   oleQt(st);
		tQueryList.push_back(oleQt);

		wstring wstrValue;
		Project::Tools::UTF8ToWideChar(row[2], wstrValue);
		strQueryList.push_back(wstrValue);
	}

	//将查到的数据补齐
	unsigned int i = 0;
	for(i=0;(i+1)<tQueryList.size();i++)
	{
		COleDateTime sti = tQueryList[i];
		COleDateTime stj = tQueryList[i+1];

		COleDateTime tNextRight = sti + tspan;
		if(tNextRight < stj)
		{
			tQueryList.insert(tQueryList.begin()+ i+1, tNextRight);
			wstring wstrTemp = strQueryList[i];
			strQueryList.insert(strQueryList.begin()+ i+1, wstrTemp);
		}

	}


	//补齐后面的
	if(tQueryList.size()>0)
	{
		COleDateTime tCur = tQueryList.back();
		wstring strValueLack = strQueryList.back(); // _T("err");
		while(tCur<oleTimeEnd)
		{
			tCur += tspan;
			tQueryList.push_back(tCur);
			strQueryList.push_back(strValueLack);
		}
	}


	//从补齐的数据中挑选
	for(i=0;i<tQueryList.size();i++)
	{
		if(tQueryList[i]>=oleTimeStart && tQueryList[i]<= oleTimeEnd)
		{
			wstring strValueItem = strQueryList[i];
			strParam  = strParam + strValueItem;
			strParam = strParam + L",";
		}
	}
	return true;
}

bool Beopdatalink::CCommonDBAccess::GetAllHistoryValueLastInOneDay(const COleDateTime tt, vector<Beopdatalink::CRealTimeDataEntry> &entrylist)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	map<string,wstring> mapHistoryValue;
	std::ostringstream sqlstream;

	SYSTEMTIME stStart;
	tt.GetAsSystemTime(stStart);

	string histablename = GetHistoryTableNameByStoreCycle(stStart,E_STORE_ONE_MINUTE);
	sqlstream.str("");
	sqlstream << "select time, pointname, value from " << histablename  <<" order by time";

	string sql = sqlstream.str();
	bool bResult = true;
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
	if (result == NULL)
	{
		bResult = false;
	}
	if(bResult)
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
		long rownum = GetRowNum(result);
		if (rownum == 0){
			bResult = false;
		}
		if(bResult)
		{
			MYSQL_ROW row = NULL;
			while(row = FetchRow(result))
			{
				mapHistoryValue[row[1]] = Project::Tools::UTF8ToWideChar(row[2]);
			}
		}
	}

	entrylist.clear();
	map<string,wstring>::iterator iter =  mapHistoryValue.begin();
	while(iter != mapHistoryValue.end())
	{
		Beopdatalink::CRealTimeDataEntry entry;
		entry.SetName(iter->first);
		entry.SetValue(iter->second);
		entrylist.push_back(entry);
		iter++;
	}

	if(entrylist.size() > 0)
		return true;
	return false;
}

bool Beopdatalink::CCommonDBAccess::GetAllHistoryValueByTime( const SYSTEMTIME stStart,const SYSTEMTIME stEnd,vector<Beopdatalink::CRealTimeDataEntry> &entrylist,bool bFileExist )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	map<string,wstring> mapHistoryValue;
	std::ostringstream sqlstream;
	COleDateTime oleTimeEnd(stEnd),oleTimeStart(stStart);
	COleDateTimeSpan oleSpan = oleTimeEnd - oleTimeStart;
	if(bFileExist)			//无需查零点的值
	{
		//从小时表拿小时变化的数据
		if(oleSpan.GetTotalHours() >= 1)
		{
			string histablename = GetHistoryTableNameByStoreCycle(stStart,E_STORE_ONE_HOUR);
			sqlstream.str("");
			sqlstream << "select * from " << histablename 
				<< " where time <= '" << Project::Tools::SystemTimeToString(stEnd) << "' and time >= '"
				<< Project::Tools::SystemTimeToString(stStart) <<"' order by time";

			string sql = sqlstream.str();
			bool bResult = true;
			MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
			if (result == NULL)
			{
				bResult = false;
			}
			if(bResult)
			{
				Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
				long rownum = GetRowNum(result);
				if (rownum == 0){
					bResult = false;
				}
				if(bResult)
				{
					MYSQL_ROW row = NULL;
					while(row = FetchRow(result))
					{
						mapHistoryValue[row[1]] = Project::Tools::UTF8ToWideChar(row[2]);
					}
				}
			}
		}

		//再从5分钟表拿分钟变化的数据
		if(oleSpan.GetTotalMinutes() >= 1)
		{
			string histablename = GetHistoryTableNameByStoreCycle(stStart,E_STORE_FIVE_MINUTE);
			sqlstream.str("");
			sqlstream << "select * from " << histablename 
				<< " where time <= '" << Project::Tools::SystemTimeToString(stEnd) << "' and time >= '"
				<< stEnd.wYear << "-" << stStart.wMonth << "-" << stStart.wDay << " " << stStart.wHour <<":00:00' order by time";

			string sql = sqlstream.str();
			bool bResult = true;
			MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
			if (result == NULL)
			{
				bResult = false;
			}
			if(bResult)
			{
				Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
				long rownum = GetRowNum(result);
				if (rownum == 0){
					bResult = false;
				}
				if(bResult)
				{
					MYSQL_ROW row = NULL;
					while(row = FetchRow(result))
					{
						mapHistoryValue[row[1]] = Project::Tools::UTF8ToWideChar(row[2]);
					}
				}
			}
		}
	}
	else
	{
		//拿零点的数据
		{
			string histablename = GetHistoryTableNameByStoreCycle(stStart,E_STORE_FIVE_MINUTE);
			sqlstream.str("");
			sqlstream << "select * from " << histablename 
				<< " where time <= '" << stStart.wYear << "-" << stStart.wMonth << "-" << stStart.wDay << " 01:00:00' order by time";

			string sql = sqlstream.str();
			bool bResult = true;
			MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
			if (result == NULL)
			{
				bResult = false;
			}
			if(bResult)
			{
				Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
				long rownum = GetRowNum(result);
				if (rownum == 0){
					bResult = false;
				}
				if(bResult)
				{
					MYSQL_ROW row = NULL;
					while(row = FetchRow(result))
					{
						mapHistoryValue[row[1]] = Project::Tools::UTF8ToWideChar(row[2]);
					}
				}
			}
		}

		//从小时表拿小时变化的数据
		if(oleSpan.GetTotalHours() >= 1)
		{
			string histablename = GetHistoryTableNameByStoreCycle(stStart,E_STORE_ONE_HOUR);
			sqlstream.str("");
			sqlstream << "select * from " << histablename 
				<< " where time <= '" << Project::Tools::SystemTimeToString(stEnd) << "' and time >= '"
				<< Project::Tools::SystemTimeToString(stStart) <<"' order by time";

			string sql = sqlstream.str();
			bool bResult = true;
			MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
			if (result == NULL)
			{
				bResult = false;
			}
			if(bResult)
			{
				Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
				long rownum = GetRowNum(result);
				if (rownum == 0){
					bResult = false;
				}
				if(bResult)
				{
					MYSQL_ROW row = NULL;
					while(row = FetchRow(result))
					{
						mapHistoryValue[row[1]] = Project::Tools::UTF8ToWideChar(row[2]);
					}
				}
			}
		}

		//再从5分钟表拿分钟变化的数据
		if(oleSpan.GetTotalMinutes() >= 1)
		{
			string histablename = GetHistoryTableNameByStoreCycle(stStart,E_STORE_FIVE_MINUTE);
			sqlstream.str("");
			sqlstream << "select * from " << histablename 
				<< " where time <= '" << Project::Tools::SystemTimeToString(stEnd) << "' and time >= '"
				<< stEnd.wYear << "-" << stStart.wMonth << "-" << stStart.wDay << " " << stStart.wHour <<":00:00' order by time";

			string sql = sqlstream.str();
			bool bResult = true;
			MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );
			if (result == NULL)
			{
				bResult = false;
			}
			if(bResult)
			{
				Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
				long rownum = GetRowNum(result);
				if (rownum == 0){
					bResult = false;
				}
				if(bResult)
				{
					MYSQL_ROW row = NULL;
					while(row = FetchRow(result))
					{
						mapHistoryValue[row[1]] = Project::Tools::UTF8ToWideChar(row[2]);
					}
				}
			}
		}
	}

	entrylist.clear();
	map<string,wstring>::iterator iter =  mapHistoryValue.begin();
	while(iter != mapHistoryValue.end())
	{
		Beopdatalink::CRealTimeDataEntry entry;
		entry.SetName(iter->first);
		entry.SetValue(iter->second);
		entrylist.push_back(entry);
		iter++;
	}

	if(entrylist.size() > 0)
		return true;
	return false;
}

bool Beopdatalink::CRealTimeDataAccess::InsertRealTimeDatas_Input_NoClear( const vector<CRealTimeDataEntry>& entrylist)
{
	if (entrylist.empty()){
		return false;
	}

	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	StartTransaction();

	std::ostringstream sqlstream;



	//generate the insert string
	sqlstream.str("");
	sqlstream << "replace into "
		<< GetDBName()
		<<"."
		<< tablename_input 
		<< " value";

	const SYSTEMTIME& sttime = entrylist[0].GetTimestamp();
	string str_temp = Project::Tools::SystemTimeToString(sttime);
	vector<CRealTimeDataEntry>::const_iterator it = entrylist.begin();
	int nCount = 0;
	for (;it != entrylist.end(); ++it)
	{
		string value_ansi;
		Project::Tools::WideCharToUtf8((*it).GetValue(), value_ansi);

		sqlstream << "('" << str_temp << "','" 
			<< (*it).GetName() << "', '" 
			<< value_ansi << "'),";
		nCount++;
	}

	string sql_temp = sqlstream.str();
	sql_temp.erase(--sql_temp.end());
	//string strsql = Project::Tools::MultiByteToUtf8(sql_temp.c_str());
	bool result =  Execute(sql_temp);
	if(!result)
	{
		string strMysqlErr = GetMysqlError();
		//AddMysqlErrLog("insert realtimedata_input err",strMysqlErr);
		AddMysqlErrSqlLog("insert realtimedata_input err",strMysqlErr,sql_temp);
	}
	Commit();

	return result;
}


bool    Beopdatalink::CCommonDBAccess::UpdateRealtimeData(const wstring strPointName, const wstring strPointValue)
{
	if (strPointName.size()==0){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	std::ostringstream sqlstream;
	//generate the insert string

	string strPointName_Utf8;
	string strPointValue_Utf8;

	Project::Tools::WideCharToUtf8(strPointName, strPointName_Utf8);
	Project::Tools::WideCharToUtf8(strPointValue, strPointValue_Utf8);
	sqlstream.str("");
	sqlstream << "update "
		<< GetDBName()
		<<"."
		<< tablename_input 
		<< " set pointvalue = '" << strPointValue_Utf8 << "' where pointname = '" << strPointName_Utf8 <<"'";

	string sqlstatement = sqlstream.str();

	return Execute(sqlstatement);

}

bool Beopdatalink::CCommonDBAccess::ReadRealTimeData_Unit01( vector<unitStateInfo>& unitlist )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01";
	
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	unitlist.reserve(length);
	MYSQL_ROW  row = NULL;

	unitStateInfo unit;

	while(row = FetchRow(result) )
	{
		unit.unitproperty01 = Project::Tools::Utf8ToMultiByte(row[0]);
		unit.unitproperty02 = Project::Tools::Utf8ToMultiByte(row[1]);
		unit.unitproperty03 = Project::Tools::Utf8ToMultiByte(row[2]);

		unitlist.push_back(unit);
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::InsertUnit01( const vector<unitStateInfo>& unitlist,const string reip )
{
	if (unitlist.empty()){
		return false;
	}
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit01";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<unitStateInfo>::const_iterator it = unitlist.begin();
	bool bSuccess = true;
	for (;it != unitlist.end(); ++it)
	{
		string strUnitP01 = (*it).unitproperty01;
		string strUnitP02 = (*it).unitproperty02;
		if(strUnitP01 == "redundencyip")
		{
			//strUnitP02 为本机IP
			strUnitP02 = reip;
		}
		string strUnitP03 = (*it).unitproperty03;

		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".unit01 "
			<< " value"
			<< "('" << strUnitP01 << "', '"  << strUnitP02 << "' , '"<< strUnitP03<<"','','','','','','','','','','','','');"; 

		string sqlstatement = sqlstream.str();
		bSuccess = Execute(sqlstatement);
		if(!bSuccess)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
	}

	return Commit();
}

bool Beopdatalink::CCommonDBAccess::UpdateSavePoints(const vector<DataPointEntry>& ptList)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	std::ostringstream sqlstream;

	StartTransaction();

	sqlstream << "delete from "
		<< GetDBName() << ".unit04";

	string sqldelete = sqlstream.str();
	Execute(sqldelete);

	vector<DataPointEntry>::const_iterator it = ptList.begin();
	bool bSuccess = true;
	for (;it != ptList.end(); ++it)
	{
		int nIndex = (*it).GetPointIndex();
		wstring wstrPointName = (*it).GetShortName();
		string strType = (*it).GetTypeString();

		wstring wstrSourceType = (*it).GetSourceType();
		string strSourceType;
		Project::Tools::WideCharToUtf8(wstrSourceType, strSourceType);
		
		string strPointName;
		Project::Tools::WideCharToUtf8(wstrPointName, strPointName);

		wstring wstrDescription = (*it).GetDescription();
		string strDescription;
		Project::Tools::WideCharToUtf8(wstrDescription, strDescription);

		int nOpenFlag = (*it).GetOpenToThirdParty();

		string strRW = "R";
		if((*it).GetProperty()==PLC_WRITE)
			strRW = "W";

		string strSavePeriod = "null";
		POINT_STORE_CYCLE ptSaveCycle = (*it).GetStoreCycle();
		if(ptSaveCycle==E_STORE_FIVE_SECOND)
			strSavePeriod = "s5";
		else if(ptSaveCycle==E_STORE_ONE_MINUTE)
			strSavePeriod = "m1";
		else if(ptSaveCycle==E_STORE_FIVE_MINUTE)
			strSavePeriod = "m5";
		else if(ptSaveCycle==E_STORE_HALF_HOUR)
			strSavePeriod = "m30";
		else if(ptSaveCycle==E_STORE_ONE_HOUR)
			strSavePeriod = "h1";
		else if(ptSaveCycle==E_STORE_ONE_DAY)
			strSavePeriod = "d1";
		else if(ptSaveCycle==E_STORE_ONE_WEEK)
			strSavePeriod = "w1";
		else if(ptSaveCycle==E_STORE_ONE_MONTH)
			strSavePeriod = "Month1";
		else if(ptSaveCycle==E_STORE_ONE_YEAR)
			strSavePeriod = "Year1";

		sqlstream.str("");
		sqlstream << "insert into "
			<< GetDBName()
			<<".unit04 "
			<< " value"
			<< "('" << nIndex << "', '"  << strPointName << "' , '"<< strSourceType << "' , '"<< strType << "' , '"<< strDescription <<"','"<< strRW <<"','','','','','','','','"<< nOpenFlag <<"','"<<strSavePeriod <<"');"; 

		string sqlstatement = sqlstream.str();
		bool bResult = Execute(sqlstatement);
		if(!bResult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit04 err",strMysqlErr);
			bSuccess = false;
		}
	}

	bSuccess = Commit()&& bSuccess;
	return bSuccess;
}

bool Beopdatalink::CCommonDBAccess::ReadRecentWarning( vector<CWarningEntry>& resultlist, const int nSecondBefore )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	COleDateTime oleCurrent = COleDateTime::GetCurrentTime();
	COleDateTime oleBefore = oleCurrent - COleDateTimeSpan(0,0,0,nSecondBefore);
	SYSTEMTIME sysTime;
	oleBefore.GetAsSystemTime(sysTime);
	string str_end = Project::Tools::SystemTimeToString(sysTime);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< GetWarningRecordTableName(oleCurrent)
		<< " where endtime >= '" << str_end << "';";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	resultlist.reserve(length);
	MYSQL_ROW  row = NULL;

	CWarningEntry entry;
	SYSTEMTIME st;
	while(row = FetchRow(result) ){
		Project::Tools::String_To_SysTime(row[0], st);
		entry.SetTimestamp(st);
		entry.SetWarningCode(ATOI(row[1]));
		wstring warninginfo = Project::Tools::UTF8ToWideChar(row[2]);
		entry.SetWarningInfo(warninginfo);
		entry.SetWarningLevel(ATOI(row[3]));
		wstring pointname = Project::Tools::UTF8ToWideChar(row[7]);
		entry.SetWarningPointName(pointname);
		entry.SetWarningConfirmedType(ATOI(row[5]));
		Project::Tools::String_To_SysTime(row[4], st);
		entry.SetEndTimestamp(st);

		resultlist.push_back(entry);
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::ReadRecentOperation( vector<WarningOperation>& resultlist, const int nSecondBefore )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	COleDateTime oleCurrent = COleDateTime::GetCurrentTime();
	COleDateTime oleBefore = oleCurrent - COleDateTimeSpan(0,0,0,nSecondBefore);
	SYSTEMTIME sysTime;
	oleBefore.GetAsSystemTime(sysTime);
	string str_end = Project::Tools::SystemTimeToString(sysTime);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_warningoperation
		<< " where unitproperty01 >= '" << str_end << "' order by unitproperty01 desc;";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	resultlist.reserve(length);
	MYSQL_ROW  row = NULL;

	WarningOperation entry;
	SYSTEMTIME st;
	while(row = FetchRow(result) ){
		entry.strTime = Project::Tools::UTF8ToWideChar(row[0]);
		entry.strWarningInfo = Project::Tools::UTF8ToWideChar(row[1]);
		entry.strBindPointName = Project::Tools::UTF8ToWideChar(row[2]);
		entry.strOpeartion = Project::Tools::UTF8ToWideChar(row[4]);
		entry.strUser = Project::Tools::UTF8ToWideChar(row[3]);		
		resultlist.push_back(entry);
	}

	return true;
}

bool Beopdatalink::CCommonDBAccess::DownloadFile(wstring strFileID, wstring strFileSavePath)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string value_ansi;
	Project::Tools::WideCharToUtf8(strFileID, value_ansi);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".filestorage"
		<< " where fileid = '" << value_ansi << "';";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}


	wstring strFileName;
	wstring strFileDescription;
	SYSTEMTIME updateTime;
	MYSQL_ROW  row = NULL;
	FILE *fp = NULL; 

	if(row = FetchRow(result) )
	{
		strFileName = Project::Tools::UTF8ToWideChar(row[1]);
		wstring strTime = Project::Tools::UTF8ToWideChar(row[3]);
		Project::Tools::String_To_SysTime(strTime, updateTime);

		unsigned long* pSize = GetFieldLengths(result);
		long nFileSize = pSize[9];

		if(_waccess_s(strFileSavePath.c_str(),0) == 0)
		{
			if (nFileSize > 0)
			{
				_stat32 fs;
				_wstat32(strFileSavePath.c_str(), &fs);

				if(_wfopen_s(&fp,strFileSavePath.c_str(),L"wb,ccs=UNICODE") == 0)
				{
					fwrite(row[9],nFileSize,1,fp);
					fclose(fp);
					
				}
				
			}
		}
		else
		{
			if(_wfopen_s(&fp,strFileSavePath.c_str(),L"wb,ccs=UNICODE") == 0)
			{
				fwrite(row[9],nFileSize,1,fp);
				fclose(fp);
			}
		}
	}

	return true;

}


bool Beopdatalink::CCommonDBAccess::UpdateLibInsertIntoFilestorage(const wstring strFilePathName, const wstring strFileName, const wstring strFileId)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	off_t			iSize;
	char            *pSql; 
	FILE            *fFp; 
	char            *cBuf; 
	size_t          n = 0; 
	char            *cEnd; 
	size_t			lSize = 0;
	string strUtf8PathName;
	string strUtf8FileName;
	string strUtf8FileId;
	USES_CONVERSION;
	strUtf8PathName = T2A(strFilePathName.c_str());
	strUtf8FileName = T2A(strFileName.c_str());
	strUtf8FileId = T2A(strFileId.c_str());
	if ((fFp = fopen(strUtf8PathName.c_str(),"rb")) == NULL) 
	{ 
		perror("fopen file"); 
		return false; 
	}
	long cur_pos;
	long len;
	cur_pos = ftell( fFp );
	fseek(fFp, 0, SEEK_END);
	len = ftell( fFp );
	//将文件流的读取位置还原为原先的值
	fseek(fFp, cur_pos, SEEK_SET);
	if ((cBuf = new char[sizeof(char)*(len+1)]) == NULL) 
	{ 
		return false; 
	}
	iSize = len;
	if ((n = fread(cBuf, 1, iSize, fFp)) < 0) 
	{ 
		return false; 
	}
	lSize = sizeof(char) * n * 2 + 256;
	pSql = new char[lSize];
	if (pSql == NULL) 
	{ 
		return false; 
	}

	//删除选文件
	std::ostringstream sqlstream;
	string sqlstatement;
	sqlstream << "delete from filestorage where fileid = '" << strUtf8FileId << "';";
	sqlstatement = sqlstream.str();
	Execute(sqlstatement);


	sqlstream.str("");
	sqlstream << "insert into filestorage(fileid, filename, filedescription, reserve01, reserve02, reserve03, reserve04, reserve05, fileblob) values('"
		<< strUtf8FileId << "','" << strUtf8FileName << "', '', '', '', '', '', '', ";
	sqlstatement = sqlstream.str();
	strcpy_s(pSql, lSize, sqlstatement.c_str());

	cEnd = pSql; 
	cEnd += strlen(pSql); //point sql tail 
	*cEnd++ = '\''; 
	cEnd += RealEscapeString(cEnd, cBuf, n); 
	*cEnd++ = '\''; 
	*cEnd++ = ')'; 

	sqlstatement = pSql;
	if (!Execute(sqlstatement, (unsigned int)(cEnd-pSql)))
	{
		fclose(fFp); 
		fFp = NULL;
		if (pSql != NULL)
		{
			delete[] pSql;
			pSql = NULL;
		}
		return false;
	}

	fclose(fFp); 
	fFp = NULL;
	if (pSql != NULL)
	{
		delete[] pSql;
		pSql = NULL;
	}

	if(cBuf!=NULL)
	{
		delete[] cBuf;
		cBuf = NULL;
	}

	return true;
}



bool Beopdatalink::CCommonDBAccess::DeleteFile(wstring strFileID)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string value_ansi;
	Project::Tools::WideCharToUtf8(strFileID, value_ansi);

	std::ostringstream sqlstream;
	sqlstream << "delete from " 
		<< GetDBName() 
		<< ".filestorage"
		<< " where fileid = '" << value_ansi << "';";


	string sqlstatement = sqlstream.str();

	return Execute(sqlstatement);

	return true;

}

bool Beopdatalink::CCommonDBAccess::ReadRecentUserOperation( vector<UserOperation>& resultlist, const int nSecondBefore )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	COleDateTime oleCurrent = COleDateTime::GetCurrentTime();
	COleDateTime oleBefore = oleCurrent - COleDateTimeSpan(0,0,0,nSecondBefore);
	SYSTEMTIME sysTime;
	oleBefore.GetAsSystemTime(sysTime);
	string str_end = Project::Tools::SystemTimeToString(sysTime);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_operationrecord
		<< " where RecordTime >= '" << str_end << "' order by RecordTime desc;";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	resultlist.reserve(length);
	MYSQL_ROW  row = NULL;

	UserOperation entry;
	SYSTEMTIME st;
	while(row = FetchRow(result) ){
		entry.strTime = Project::Tools::UTF8ToWideChar(row[0]);
		entry.strOpeartion = Project::Tools::UTF8ToWideChar(row[2]);
		entry.strUser = Project::Tools::UTF8ToWideChar(row[1]);		
		resultlist.push_back(entry);
	}

	return true;
}


string  Beopdatalink::CCommonDBAccess::GetWarningRecordTableName(COleDateTime &tt)
{
	return tablename_warningrecord_prefiex;
	
	
	//string strPrefix = tablename_warningrecord_prefiex;
	//std::ostringstream ss;
	//ss<< strPrefix<< tt.GetYear();
	//return ss.str().c_str();
}

bool Beopdatalink::CCommonDBAccess::ReadRecentUserOperation( vector<UserOperation>& resultlist, wstring strTime )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string str_end = Project::Tools::WideCharToAnsi(strTime.c_str());
	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< "."
		<< tablename_operationrecord
		<< " where RecordTime > '" << str_end << "' order by RecordTime desc;";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	resultlist.reserve(length);
	MYSQL_ROW  row = NULL;

	UserOperation entry;
	SYSTEMTIME st;
	while(row = FetchRow(result) ){
		entry.strTime = Project::Tools::UTF8ToWideChar(row[0]);
		entry.strOpeartion = Project::Tools::UTF8ToWideChar(row[2]);
		entry.strUser = Project::Tools::UTF8ToWideChar(row[1]);		
		resultlist.push_back(entry);
	}

	return true;
}

wstring Beopdatalink::CCommonDBAccess::ReadOrCreateCoreDebugItemValue_Defalut( wstring strItemName,wstring strItemValue /*= L"0"*/ )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string strItemName_utf8;
	Project::Tools::WideCharToUtf8(strItemName, strItemName_utf8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " 
		<< GetDBName() 
		<< ".unit01 where unitproperty01 = '"
		<< strItemName_utf8 << "'";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return strItemValue;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		sqlstream.str("");
		string strItemValue_utf8;
		Project::Tools::WideCharToUtf8(strItemValue, strItemValue_utf8);
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".unit01"
			<< " value " << "('" <<strItemName_utf8 << "', '" << strItemValue_utf8 <<"','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);		
		if(!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err",strMysqlErr);
		}
		return strItemValue;
	}

	MYSQL_ROW  row = NULL;
	if(row = FetchRow(result) )
	{
		return Project::Tools::UTF8ToWideChar(row[1]);
	}
	return strItemValue;
}

bool Beopdatalink::CCommonDBAccess::CreateHistoryDataStatusTableIfNotExist()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	bool bCreateList = true;
	//检查schedule_list
	std::ostringstream sqlstream;
	sqlstream << "SELECT * FROM information_schema.tables WHERE table_schema = '" << GetDBName() 
		<< "' AND table_name = 'core_status'";
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result != NULL)
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
		int length = static_cast<int>(GetRowNum(result));
		if (length == 0)
		{
			//创建表
			std::ostringstream sqlstreamCreate;
			sqlstreamCreate << "CREATE TABLE if not exists  `"<<GetDBName() << "`.`core_status` (\
							   `id` int(10) NOT NULL,\
							   `timeFrom` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\
							   `timeTo` timestamp NOT NULL,\
							   `process` varchar(255) NOT NULL DEFAULT '',\
							   `status` int(10) NOT NULL DEFAULT 1,\
							   `status01` int(10) NOT NULL DEFAULT 1,\
							   `status02` int(10) NOT NULL DEFAULT 1,\
							   `status03` int(10) NOT NULL DEFAULT 1,\
							   `status04` int(10) NOT NULL DEFAULT 1,\
							   `status05` int(10) NOT NULL DEFAULT 1,\
							   `status06` int(10) NOT NULL DEFAULT 1,\
							   `status07` int(10) NOT NULL DEFAULT 1,\
							   `status08` int(10) NOT NULL DEFAULT 1,\
							   `status09` int(10) NOT NULL DEFAULT 1,\
							   `status10` int(10) NOT NULL DEFAULT 1,\
							   `status11` int(10) NOT NULL DEFAULT 1,\
							   `status12` int(10) NOT NULL DEFAULT 1,\
							   `status13` int(10) NOT NULL DEFAULT 1,\
							   `status14` int(10) NOT NULL DEFAULT 1,\
							   `status15` int(10) NOT NULL DEFAULT 1\
							   ) ENGINE=InnoDB DEFAULT CHARSET=utf8;";

			string sqlstatement = sqlstreamCreate.str();
			bCreateList = Execute(sqlstatement);

			if(!bCreateList)
			{
				string strMysqlErr = GetMysqlError();
				int xx=0;
			}
		}
	}

	return bCreateList;
}
bool Beopdatalink::CCommonDBAccess::CreateScheduleTableIfNotExist()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	bool bCreateList = true;
	bool bCreateInfo = true;
	//检查schedule_list
	std::ostringstream sqlstream;
	sqlstream << "SELECT * FROM information_schema.tables WHERE table_schema = '" << GetDBName() 
		<< "' AND table_name = 'schedule_list'";
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result != NULL)
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
		int length = static_cast<int>(GetRowNum(result));
		if (length == 0)
		{
			//创建表
			std::ostringstream sqlstreamCreate;
			sqlstreamCreate << "CREATE TABLE  `beopdata`.`schedule_list` (\
							   `id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
							   `type` int(10) unsigned NOT NULL DEFAULT '0',\
							   `name` varchar(255) NOT NULL,\
							   `point` varchar(20000) NOT NULL,\
							   `loop` int(10) unsigned NOT NULL DEFAULT '0',\
							   `enable` int(10) unsigned NOT NULL DEFAULT '0',\
							   `author` varchar(255) NOT NULL,\
							   PRIMARY KEY (`id`)\
							   ) ENGINE=InnoDB DEFAULT CHARSET=utf8;";

			string sqlstatement = sqlstreamCreate.str();
			bCreateList = Execute(sqlstatement);
		}
	}

	//检查schedule_info_weeky
	sqlstream.str("");
	sqlstream << "SELECT * FROM information_schema.tables WHERE table_schema = '" << GetDBName() 
		<< "' AND table_name = 'schedule_info_weeky'";
	MYSQL_RES* result1 = QueryAndReturnResult(sqlstream.str().c_str());
	if (result1 != NULL)
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result1);
		int length = static_cast<int>(GetRowNum(result1));
		if (length == 0)
		{
			//创建表
			std::ostringstream sqlstreamCreate;
			sqlstreamCreate << "CREATE TABLE  `beopdata`.`schedule_info_weeky` (\
							   `id` int(10) unsigned NOT NULL AUTO_INCREMENT,\
							   `weekday` int(10) unsigned NOT NULL,\
							   `timeFrom` varchar(255) NOT NULL,\
							   `timeTo` varchar(255) NOT NULL,\
							   `value` varchar(255) NOT NULL,\
							   `groupid` int(10) unsigned NOT NULL,\
							   `timeExecute` varchar(255) NOT NULL,\
							   PRIMARY KEY (`id`)\
							   ) ENGINE=InnoDB DEFAULT CHARSET=utf8;";

			string sqlstatement = sqlstreamCreate.str();
			bCreateInfo = Execute(sqlstatement);
		}
	}
	return bCreateList&&bCreateInfo;// 默认返回没有创建
}

bool Beopdatalink::CCommonDBAccess::GetScheduleInfo( const string strDate,const string strTimeFrom,const string strTimeTo, const int nDayOfWeek,vector<ScheduleInfo>& vecSchedule )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	//SELECT s.*,l.point,l.loop FROM beopdata.schedule_info_weeky s,beopdata.schedule_list l where s.groupid = l.id and l.enable=1 and s.weekday=0 and s.timeFrom>='' and s.timeTo <'' order by s.id;
	std::ostringstream sqlstream;
	sqlstream << "SELECT s.*,l.point,l.loop,l.name FROM beopdata.schedule_info_weeky s,beopdata.schedule_list l where s.groupid = l.id and l.enable=1 and s.weekday=" 
		<< nDayOfWeek << " and s.timeFrom <= '" << strTimeFrom << "' and s.timeTo >'" << strTimeTo  << "' and timeExecute !='" << strDate  << "' order by s.id;";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length){
		return false;
	}
	vecSchedule.clear();
	MYSQL_ROW  row = NULL;

	ScheduleInfo schedule;
	SYSTEMTIME st;
	while(row = FetchRow(result) )
	{
		schedule.nID = ATOI(row[0]);
		schedule.nWeedday = ATOI(row[1]);
		schedule.strTimeFrom = row[2];
		schedule.strTimeTo = row[3];
		schedule.strValue = row[4];
		schedule.nGroupID = ATOI(row[5]);
		schedule.strPoint = row[7];
		schedule.nLoop = ATOI(row[8]);
		schedule.strName = row[9];
		vecSchedule.push_back(schedule);
	}
	return true;
}

bool Beopdatalink::CCommonDBAccess::DeleteScheduleInfo( const int nID )
{
	std::ostringstream sqlstream;
	sqlstream << "delete from "
		<< GetDBName() << "." << tablename_schedule_info_weeky << " where id=" << nID;
	string sqlstatement = sqlstream.str();
	return Execute(sqlstatement);
}

bool Beopdatalink::CCommonDBAccess::GetScheduleExist()
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * FROM beopdata.schedule_info_weeky order by id limit 0,1;" ;

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL){
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		return false;
	}
	return true;
}

bool Beopdatalink::CCommonDBAccess::UpdateScheduleInfoExecuteDate( const int nID,const string strDate )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);

	SetConnectionCharacterSet(MYSQL_UTF8);

	//generate the insert string
	std::ostringstream sqlstream;
	sqlstream << "update "
		<< GetDBName()
		<<"."
		<< tablename_schedule_info_weeky 
		<< " set timeExecute='" << strDate << "'"
		<< " where id=" << nID << ";";

	string sqlstatement = sqlstream.str();
	return Execute(sqlstatement);
}

bool Beopdatalink::CCommonDBAccess::ReadMysqlVariable( wstring wstrVariableName, wstring& wstrVariableValue )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	wstrVariableValue = L"";
	string strVariableName_utf8;
	Project::Tools::WideCharToUtf8(wstrVariableName, strVariableName_utf8);

	std::ostringstream sqlstream;
	sqlstream << "SHOW VARIABLES like '" << strVariableName_utf8 << "';";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		return false;
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	int length = static_cast<int>(GetRowNum(result));
	if (0 == length)
	{
		return false;
	}

	MYSQL_ROW  row = NULL;
	if(row = FetchRow(result) )
	{
		wstrVariableValue =  Project::Tools::UTF8ToWideChar(row[1]);
	}
	return true;
}

bool Beopdatalink::CCommonDBAccess::SaveOneSendData( DTU_DATA_INFO & data )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "delete from " << tablename_dtu_send << " where filename='" << data.strFileName << "' and subtype = 0;";
	Execute(sqlstream.str());

	sqlstream.str("");
	sqlstream << "insert into " << tablename_dtu_send << " (time, filename, subtype, trycount, content) Values('"
		<< data.strTime << "','" << data.strFileName << "'," << data.nSubType << "," << data.nTryCount << ",'" << Project::Tools::MultiByteToUtf8(data.strContent.c_str()) << "');";
	return Execute(sqlstream.str());
}


bool Beopdatalink::CCommonDBAccess::CheckHistoryDataTableHealthy_loss_start_m1(SYSTEMTIME tCheckDay)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	string strTableName = GetHistoryTableNameByStoreCycle(tCheckDay, E_STORE_ONE_MINUTE);

	SYSTEMTIME stTemp = tCheckDay;

	std::ostringstream sqlstream;
	sqlstream << "SELECT count(*) FROM "<< strTableName <<" where time <= '" <<
		stTemp.wYear << "-" << stTemp.wMonth << "-" << stTemp.wDay << " 00:00:00' order by time";

	int nRecordCount = -1;
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		//repair
		return false;
	}
	else
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

		
		if (result){	
			MYSQL_ROW row = NULL;
			if (row = mysql_fetch_row(result)){
				nRecordCount = ATOI(row[0]);
			}
		}
	}

	if(nRecordCount==0)
	{
		//need repair
		return false;
	}

	return true;

}

bool Beopdatalink::CCommonDBAccess::CheckHistoryDataTableHealthy_loss_start_m5(SYSTEMTIME tCheckDay)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	SetConnectionCharacterSet();

	string strTableName = GetHistoryTableNameByStoreCycle(tCheckDay, E_STORE_FIVE_MINUTE);

	SYSTEMTIME stTemp = tCheckDay;

	std::ostringstream sqlstream;
	sqlstream << "SELECT count(*) FROM "<< strTableName <<" where time <= '" <<
		stTemp.wYear << "-" << stTemp.wMonth << "-" << stTemp.wDay << " 00:00:00' order by time";

	int nRecordCount = -1;
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		//repair
		return false;
	}
	else
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


		if (result){	
			MYSQL_ROW row = NULL;
			if (row = mysql_fetch_row(result)){
				nRecordCount = ATOI(row[0]);
			}
		}
	}

	if(nRecordCount==0)
	{
		//need repair
		return false;
	}

	return true;

}

bool Beopdatalink::CCommonDBAccess::CheckHistoryDataTableHealthy_loss_oneday_d1(SYSTEMTIME tCheckDay)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	SetConnectionCharacterSet();

	string strTableName = GetHistoryTableNameByStoreCycle(tCheckDay, E_STORE_ONE_DAY);

	SYSTEMTIME stTemp = tCheckDay;

	std::ostringstream sqlstream;
	sqlstream << "SELECT count(*) FROM "<< strTableName <<" where time = '" <<
		stTemp.wYear << "-" << stTemp.wMonth << "-"<< stTemp.wDay<<" 00:00:00'";

	int nRecordCount = -1;
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		//repair
		return false;
	}
	else
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


		if (result){	
			MYSQL_ROW row = NULL;
			if (row = mysql_fetch_row(result)){
				nRecordCount = ATOI(row[0]);
			}
		}
	}

	if(nRecordCount==0)
	{
		//need repair
		return false;
	}

	return true;

}

bool Beopdatalink::CCommonDBAccess::CheckHistoryDataTableHealthy_loss_start_h1(SYSTEMTIME tCheckDay)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	SetConnectionCharacterSet();

	string strTableName = GetHistoryTableNameByStoreCycle(tCheckDay, E_STORE_ONE_HOUR);

	SYSTEMTIME stTemp = tCheckDay;

	std::ostringstream sqlstream;
	sqlstream << "SELECT count(*) FROM "<< strTableName <<" where time <= '" <<
		stTemp.wYear << "-" << stTemp.wMonth << "-01 00:00:00' order by time";

	int nRecordCount = -1;
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		//repair
		return false;
	}
	else
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


		if (result){	
			MYSQL_ROW row = NULL;
			if (row = mysql_fetch_row(result)){
				nRecordCount = ATOI(row[0]);
			}
		}
	}

	if(nRecordCount==0)
	{
		//need repair
		return false;
	}

	return true;

}

bool Beopdatalink::CCommonDBAccess::UpdateHistoryStatusTable(CString strProcessName)
{
	
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	SetConnectionCharacterSet();

	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	string strTableName = "core_status";

	SYSTEMTIME stTemp = stNow;
	COleDateTime tNow;
	Project::Tools::SysTime_To_OleTime(stTemp, tNow);

	string strProcessName_utf8;
	Project::Tools::WideCharToUtf8(strProcessName.GetString(), strProcessName_utf8);

	std::ostringstream sqlstream;
	sqlstream << "SELECT id,timeFrom, timeTo FROM "<< strTableName <<" where process = '"<< strProcessName_utf8<<"' order by id DESC";

	bool bNeedInsertNew = true;
	int nCurId = 10;
	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL)
	{
		bNeedInsertNew = true;
	}
	else
	{
		Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


		if (result){	
			MYSQL_ROW row = NULL;
			if (row = mysql_fetch_row(result)){
				nCurId = ATOI(row[0]);
				SYSTEMTIME sttime;
				Project::Tools::String_To_SysTime(row[2], sttime);
				COleDateTime tTo;
				Project::Tools::SysTime_To_OleTime(sttime, tTo);

				bNeedInsertNew = false;
			}
			else
			{
				bNeedInsertNew = true;
			}
		}

		
	}

	string str_temp = Project::Tools::SystemTimeToString(stNow);

	if(bNeedInsertNew)
	{
		sqlstream.str("");
		sqlstream << "SELECT max(id) FROM "<< strTableName;

		int nMaxID = 10;
		MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
		if (result == NULL)
		{
			
		}
		else
		{
			Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);


			if (result){	
				MYSQL_ROW row = NULL;
				if (row = mysql_fetch_row(result)){
					nMaxID = ATOI(row[0]);
				}
			}
		}

		int nNewId = nMaxID+1;

		sqlstream.str("");
		sqlstream << "insert into  "
			<< GetDBName() 
			<< ".core_status(id, timeFrom, timeTo, process, status)"
			<< " values("<<nNewId << ", '" << str_temp << "', '" <<str_temp << "','"<<strProcessName_utf8<<"',1)";

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);

	}
	else
	{
		sqlstream.str("");
		sqlstream << "update  "
			<< GetDBName() 
			<< ".core_status set timeTo = '"<< str_temp <<"' where id = " <<nCurId << " and process = '"<<strProcessName_utf8<<"'" ;

		string sqlstatement = sqlstream.str();
		bool bresult =  Execute(sqlstatement);
	}

	return true;
}

bool Beopdatalink::CDBAccessToDTU::GetOperationRecordAsString( vector<optrecordfordtu>& resultlist,
									const COleDateTime& timeStart, const COleDateTime& timeStop )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	SetConnectionCharacterSet();
	bool ifok = false;

	//const int hourMax = 24;
	wstring wstrStart, wstrStop;
	string strStart, strStop;
	Project::Tools::OleTime_To_String(timeStart, wstrStart);
	Project::Tools::OleTime_To_String(timeStop, wstrStop);
	strStart = UtilString::ConvertWideCharToMultiByte(wstrStart);
	strStop = UtilString::ConvertWideCharToMultiByte(wstrStop);
	
	std::ostringstream sqlstream;
	//SELECT * FROM operation_record where recordtime >= '2011-05-30 00:00:00' and recordtime <= '2011-05-30 14:00:00';
	sqlstream << "select * from " << tablename_operationrecord <<" where recordtime >= '"<< strStart.c_str() <<"' and recordtime <= '"<< strStop.c_str() << "'";

	string sql = sqlstream.str();
	optrecordfordtu  tempOpera;
	COleDateTime old;
	// execute the query
	
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str() );

	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);

	if (result){	
		MYSQL_ROW row = NULL;
		while (row = mysql_fetch_row(result)){
			if (row[0] && row[1] && row[2]){
				tempOpera.time = row[0];
				tempOpera.username = row[1];
				tempOpera.info = row[2];
				resultlist.push_back(tempOpera);
			}
		}

	}
	
	return true;
}

Beopdatalink::CDBAccessToDTU::CDBAccessToDTU():CDatabaseSeesion()
{

}


Beopdatalink::DTUDBAccessSingleton* Beopdatalink::DTUDBAccessSingleton::GetInstance()
{
	if (_pinstance == NULL)
	{

		Project::Tools::Scoped_Lock<Project::Tools::Mutex> lock(m_lock);
		if (_pinstance == NULL)
		{
			_pinstance = new DTUDBAccessSingleton();
			atexit(DestroyInstance);
		}
	}

	return _pinstance;
}

void Beopdatalink::DTUDBAccessSingleton::DestroyInstance()
{
	if (_pinstance){
		delete _pinstance;
	}
}

Beopdatalink::DTUDBAccessSingleton::DTUDBAccessSingleton()
	:CDBAccessToDTU()
{

}


Project::Tools::Mutex Beopdatalink::DTUDBAccessSingleton::m_lock;
Beopdatalink::DTUDBAccessSingleton* Beopdatalink::DTUDBAccessSingleton::_pinstance = NULL;