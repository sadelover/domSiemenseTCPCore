#include "StdAfx.h"
#include "DataHandler.h"
#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <utility>
#include "MysqlDeleter.h"
#include "../Scope_Ptr.h"
#include "../CustomTools.h"

const char* tablename_input = "realtimedata_input";
const char* tablename_dtu_receive = "dtu_receive_buffer";
const char* tablename_dtu_send = "dtu_send_buffer";
const char* tablename_dtu_update = "dtu_update_buffer";
const char* tablename_unit01 = "unit01";
const char* tablename_output = "realtimedata_output";
const char* tablename_errcode = "unit08";

CDataHandler::CDataHandler(void)
{
}

CDataHandler::~CDataHandler(void)
{
}

wstring CDataHandler::ReadOrCreateCoreDebugItemValue_Defalut(wstring strItemName, wstring strItemValue)
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
			<< " value " << "('" << strItemName_utf8 << "', '" << strItemValue_utf8 << "','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult = Execute(sqlstatement);
		if (!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err", strMysqlErr);
		}
		return strItemValue;
	}

	MYSQL_ROW  row = NULL;
	if (row = FetchRow(result))
	{
		return Project::Tools::UTF8ToWideChar(row[1]);
	}
	return strItemValue;
}

bool CDataHandler::WriteCoreDebugItemValue(const wstring & strItemName, const wstring & strItemValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	
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
			<< " value " << "('" << strItemName_utf8 << "', '" << strItemValue_utf8 << "','','','','','','','','','','','','','')";

		string sqlstatement = sqlstream.str();
		bool bresult = Execute(sqlstatement);
		if (!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("insert unit01 err", strMysqlErr);
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
		bool bresult = Execute(sqlstatement);
		if (!bresult)
		{
			string strMysqlErr = GetMysqlError();
			AddMysqlErrLog("update unit01 err", strMysqlErr);
		}
		return bresult;
	}

	return true;
}

bool CDataHandler::GetOneLatestNormalFile(DTU_DATA_INFO & data, int nTryLimit)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " << tablename_dtu_send << " where filename !='' and subtype>0  and trycount<" << nTryLimit << " order by time desc limit 0,1;";
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
	if (row = FetchRow(result))
	{
		data.strID = Project::Tools::Utf8ToMultiByte(row[0]);
		data.strTime = Project::Tools::Utf8ToMultiByte(row[1]);
		data.strFileName = Project::Tools::Utf8ToMultiByte(row[2]);
		data.nSubType = ATOI(row[3]);
		data.nTryCount = ATOI(row[4]);
		data.strContent = Project::Tools::Utf8ToMultiByte(row[5]);
	}
	return true;
}

bool CDataHandler::GetOneLatestDataFile(DTU_DATA_INFO & data)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " << tablename_dtu_send << " where filename !='' and subtype=0  order by filename desc limit 0,1;";
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
	if (row = FetchRow(result))
	{
		data.strID = Project::Tools::Utf8ToMultiByte(row[0]);
		data.strTime = Project::Tools::Utf8ToMultiByte(row[1]);
		data.strFileName = Project::Tools::Utf8ToMultiByte(row[2]);
		data.nSubType = ATOI(row[3]);
		data.nTryCount = ATOI(row[4]);
		data.strContent = Project::Tools::Utf8ToMultiByte(row[5]);
	}
	return true;
}

bool CDataHandler::GetLatestString(DTU_DATA_INFO & data, int nTryLimit)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " << tablename_dtu_send << " where filename ='' and trycount<" << nTryLimit << " order by time desc limit 0,1;";
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
	if (row = FetchRow(result))
	{
		data.strID = Project::Tools::Utf8ToMultiByte(row[0]);
		data.strTime = Project::Tools::Utf8ToMultiByte(row[1]);
		data.strFileName = Project::Tools::Utf8ToMultiByte(row[2]);
		data.nSubType = ATOI(row[3]);
		data.nTryCount = ATOI(row[4]);
		data.strContent = Project::Tools::Utf8ToMultiByte(row[5]);
	}
	return true;
}

bool CDataHandler::GetLatestReceiveData(DTU_RECEIVE_INFO& data, int nMinute/*=60*/)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	//SetConnectionCharacterSet(MYSQL_UTF8);

	COleDateTime oleDeadTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0, 0, nMinute, 0);
	wstring wstrDeadTime;
	Project::Tools::OleTime_To_String(oleDeadTime, wstrDeadTime);
	string strDeadTime_Ansi = Project::Tools::WideCharToAnsi(wstrDeadTime.c_str());
	
	std::ostringstream sqlstream;
	sqlstream << "delete from " << tablename_dtu_receive << " where time <='" << strDeadTime_Ansi << "';";
	Execute(sqlstream.str());

	sqlstream.str("");
	sqlstream << "select * from " << tablename_dtu_receive << " order by time desc limit 0,1;";
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
	if (row = FetchRow(result))
	{
		data.nID = ATOI(row[0]);
		data.strTime = Project::Tools::Utf8ToMultiByte(row[1]);
		data.nType = ATOI(row[2]);
		data.nCmdType = ATOI(row[3]);
		data.strCmdID = Project::Tools::Utf8ToMultiByte(row[4]);
		data.strContent = Project::Tools::Utf8ToMultiByte(row[5]);
	}
	return true;
}

bool CDataHandler::HandlerOneData(DTU_DATA_INFO & data, bool bResult)
{
	if (bResult)
	{
		return DeleteOneData(data);
	}
	else
	{
		return SetDataFailCount(data);
	}
	return false;
}

bool CDataHandler::SaveOneSendData(DTU_DATA_INFO & data)
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

bool CDataHandler::SaveMulSendData(vector<DTU_DATA_INFO>& vecData)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream_delete, sqlstream_insert;
	sqlstream_delete << "delete from " << tablename_dtu_send << " where filename in(";
	sqlstream_insert << "insert into " << tablename_dtu_send << " (time, filename, subtype, trycount, content) Values";
	for (int i = 0; i < vecData.size(); ++i)
	{
		sqlstream_delete << "'" << vecData[i].strFileName << "',";
		sqlstream_insert << "('" << vecData[i].strTime << "','" << vecData[i].strFileName << "'," << vecData[i].nSubType << "," << vecData[i].nTryCount << ",'" << Project::Tools::MultiByteToUtf8(vecData[i].strContent.c_str()) << "'),";
	}

	string strDelete = sqlstream_delete.str();
	strDelete.erase(--strDelete.end());
	strDelete += ");";

	if (Execute(strDelete))
	{
		string strInsert = sqlstream_insert.str();
		strInsert.erase(--strInsert.end());
		return Execute(strInsert);
	}
	return false;
}

bool CDataHandler::SaveOneRecData(DTU_RECEIVE_INFO& data)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "insert into " << tablename_dtu_receive << " (time, type, cmdtype,cmdid,content) Values('"
		<< data.strTime << "'," << data.nType << "," << data.nCmdType << ",'" 
		<< Project::Tools::MultiByteToUtf8(data.strCmdID.c_str()) << "','"
		<< Project::Tools::MultiByteToUtf8(data.strContent.c_str()) << "');";
	return Execute(sqlstream.str());
}

bool CDataHandler::ClearExpireData(string strExpireTime)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "update " << tablename_dtu_send << " set trycount= 99  where (filename !='' and subtype>0) or filename='' and time <='" << strExpireTime << "';";
	return Execute(sqlstream.str());
}

bool CDataHandler::DeleteOneRecData(int nID)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "delete from " << tablename_dtu_receive << " where id=" << nID << ";";
	return Execute(sqlstream.str());
}

bool CDataHandler::ReadUnit01(vector<UNIT01_INFO>& unitList)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " << tablename_unit01;

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
	unitList.reserve(length);
	MYSQL_ROW  row = NULL;
	UNIT01_INFO unit;
	while (row = FetchRow(result))
	{
		unit.strProterty01 = Project::Tools::Utf8ToMultiByte(row[0]);
		unit.strProterty02 = Project::Tools::Utf8ToMultiByte(row[1]);
		unit.strProterty03 = Project::Tools::Utf8ToMultiByte(row[2]);
		unitList.push_back(unit);
	}
	return true;
}

bool CDataHandler::ChangeValues(vector<REAL_DATA_INFO>& vecValue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "insert into " << tablename_output << " (pointname, pointvalue) values ";
	for (int i = 0; i < vecValue.size(); ++i)
	{
		sqlstream << "('" << vecValue[i].strPointName << "','" << Project::Tools::MultiByteToUtf8(vecValue[i].strPointValue.c_str()) << "'),";
	}
	string sql_temp = sqlstream.str();
	sql_temp.erase(--sql_temp.end());
	return Execute(sql_temp);
}

string CDataHandler::ReadErrCodeRecentMinute(string strMintue)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	int nMintue = atoi(strMintue.c_str());
	COleDateTime oleTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0, 0, nMintue, 0);
	wstring strTime;
	Project::Tools::OleTime_To_String(oleTime, strTime);
	string strTime_Ansi = Project::Tools::WideCharToAnsi(strTime.c_str());
	std::ostringstream sqlstream;
	sqlstream << "select unitproperty01 from " << tablename_errcode << " where unitproperty02 >='" << strTime_Ansi << "' order by unitproperty02";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL) {
		return "";
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
	int length = static_cast<int>(GetRowNum(result));
	if (0 == length) {
		return "";
	}
	sqlstream.str("");
	MYSQL_ROW  row = NULL;
	while (row = FetchRow(result))
	{
		sqlstream << Project::Tools::Utf8ToMultiByte(row[0]) << "|";
	}
	return sqlstream.str();
}

string CDataHandler::ReadMysqlVariable(string strVariableName)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);
	std::ostringstream sqlstream;
	sqlstream << "SHOW VARIABLES like '" << strVariableName << "';";

	MYSQL_RES* result = QueryAndReturnResult(sqlstream.str().c_str());
	if (result == NULL) {
		return "";
	}
	Project::Tools::scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter>  resultholder(result);
	int length = static_cast<int>(GetRowNum(result));
	if (0 == length) {
		return "";
	}
	sqlstream.str("");
	MYSQL_ROW  row = NULL;
	if (row = FetchRow(result))
	{
		sqlstream << Project::Tools::Utf8ToMultiByte(row[1]);
	}
	return sqlstream.str();
}

bool CDataHandler::DeleteOneData(DTU_DATA_INFO & data)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "delete from " << tablename_dtu_send << " where id=" << data.strID << ";";
	return Execute(sqlstream.str());
}

bool CDataHandler::SetDataFailCount(DTU_DATA_INFO & data)
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "update " << tablename_dtu_send << " set trycount=" << (data.nTryCount+1) << " where id=" << data.strID << ";";
	return Execute(sqlstream.str());
}

bool CDataHandler::SaveOneUpdateData( DTU_UPDATE_INFO& data )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "delete from " << tablename_dtu_update << " where filename='" << data.strFileName << "';";
	Execute(sqlstream.str());

	sqlstream.str("");
	sqlstream << "insert into " << tablename_dtu_update << " (time, filename, cmdid) Values('"
		<< data.strTime << "','" << data.strFileName << "','" << data.strCmdID << "');";
	return Execute(sqlstream.str());
}

bool CDataHandler::GetUpdateData( DTU_UPDATE_INFO& data )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "select * from " << tablename_dtu_update << " order by time limit 0,1;";
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
	if (row = FetchRow(result))
	{
		data.strTime = Project::Tools::Utf8ToMultiByte(row[0]);
		data.strFileName = Project::Tools::Utf8ToMultiByte(row[1]);
		data.strCmdID = Project::Tools::Utf8ToMultiByte(row[2]);
	}
	return true;
}

bool CDataHandler::DeleteOneUpdateData( DTU_UPDATE_INFO& data )
{
	Project::Tools::Scoped_Lock<Mutex>  scopelock(m_lock);
	SetConnectionCharacterSet(MYSQL_UTF8);

	std::ostringstream sqlstream;
	sqlstream << "delete from " << tablename_dtu_update << " where time='" << data.strTime << "' and filename='" << data.strFileName << "';";
	return Execute(sqlstream.str());
}
