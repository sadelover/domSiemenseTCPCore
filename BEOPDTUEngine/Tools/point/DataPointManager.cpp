
/*****************************************************************************
*CSystemDataManager.cpp - declare the class CSystemDataManager
*
*       Copyright (c) WeldTeh Corporation. All rights reserved.
*

*******************************************************************************/


#include "stdafx.h"
#include "datapointentry.h"
#include "DataPointManager.h"

#include "Tools/UtilString.h"
#include "sqlite/SqliteAcess.h"
#include "sstream"
#include "sqlite/ParseStringParam3.h"
#include "Tools/CustomTools.h"

using namespace std;


// clear the map entry
CDataPointManager::~CDataPointManager()
{	

}

CDataPointManager::CDataPointManager()
{
	m_ifparsed = false;
}

void CDataPointManager::AddEntry( const DataPointEntry& pentry )
{
	m_pointlist.push_back(pentry);
	m_namemap[pentry.GetShortName()] = pentry;
	m_indexmap[pentry.GetPointIndex()] = pentry;
}

void CDataPointManager::ParsePointlist()
{
	if (!m_ifparsed){
		// parse the points

		for (unsigned int i = 0; i < m_pointlist.size(); i++)
		{
			const DataPointEntry& entry = m_pointlist[i];
			
			if (entry.IsOpcPoint())
			{
				m_opcpointlist.push_back(entry);
			}
			else if (entry.IsModbusPoint())
			{
				m_modbuspointlist.push_back(entry);
			}
			else if (entry.IsFCbusPoint())
			{
				m_fcbuspointlist.push_back(entry);
			}
			else if (entry.IsBacnetPoint())
			{
				m_bacnetpointlist.push_back(entry);
			}
			else if (entry.IsVPoint())
			{
				m_memorypointlist.push_back(entry);
			}
			else if (entry.IsProtocol104Point())
			{
				m_protocol104pointlist.push_back(entry);
			}
			else if(entry.IsMysqlPoint())
			{
				m_mysqlpointlist.push_back(entry);
			}
			else if(entry.IsSiemens1200Point() || entry.IsSiemens300Point())
			{
				m_siemenspointlist.push_back(entry);
			}

			m_namemap[entry.GetShortName()] = entry;
			m_indexmap[entry.GetPointIndex()] = entry;
		}
	}

	m_ifparsed = true;
}

int    CDataPointManager::GetOPCPointCount()
{
	ParsePointlist();
	return m_opcpointlist.size();
}
int    CDataPointManager::GetModbusPointCount()
{
	ParsePointlist();
	return m_modbuspointlist.size();

}
int    CDataPointManager::GetBacnetPointCount()
{
	ParsePointlist();
	return m_bacnetpointlist.size();

}
int    CDataPointManager::GetMemoryPointCount()
{
	ParsePointlist();
	return m_memorypointlist.size();

}
int    CDataPointManager::GetProtocal104PointCount()
{
	ParsePointlist();
	return m_protocol104pointlist.size();

}
int   CDataPointManager::GetMySQLPointCount()
{
	ParsePointlist();
	return m_mysqlpointlist.size();

}

int CDataPointManager::GetSiemensPointCount()
{
	ParsePointlist();
	return m_siemenspointlist.size();
}

void CDataPointManager::GetOpcPointList( vector<DataPointEntry>& opclist )
{
	ParsePointlist();
	opclist = m_opcpointlist;
}

void CDataPointManager::GetModbusPointList( vector<DataPointEntry>& modbuspointlist )
{
	ParsePointlist();
	modbuspointlist = m_modbuspointlist;
}

void CDataPointManager::GetBacnetPointList( vector<DataPointEntry>& bacnetpointlist )
{
	ParsePointlist();
	bacnetpointlist = m_bacnetpointlist;
}

void CDataPointManager::GetMemoryPointList( vector<DataPointEntry>& memorypointlist )
{
	ParsePointlist();
	memorypointlist = m_memorypointlist;
}

void CDataPointManager::GetProtocol104PointList( vector<DataPointEntry>& protocol104pointlist )
{
	ParsePointlist();
	protocol104pointlist = m_protocol104pointlist;
}

void CDataPointManager::GetMysqlPointList( vector<DataPointEntry>& mysqlpointlist )
{
	ParsePointlist();
	mysqlpointlist = m_mysqlpointlist;
}

void CDataPointManager::GetSiemensPointList(vector<DataPointEntry>& ptList)
{
	ParsePointlist();
	ptList = m_siemenspointlist;
}

void CDataPointManager::Clear()
{
	m_pointlist.clear();
	m_opcpointlist.clear();
	m_modbuspointlist.clear();
	m_bacnetpointlist.clear();
	m_namemap.clear();
	m_indexmap.clear();
	m_fcbuspointlist.clear();
}

const vector<DataPointEntry>& CDataPointManager::GetPointList() const
{
	return m_pointlist;
}

void CDataPointManager::SetFilePath( const wstring& filepath )
{
	Project::Tools::WideCharToUtf8(filepath, m_filepath);
}

void CDataPointManager::SetAllPointTypeAsVirtual()
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		m_pointlist[i].SetSourceType(L"vpoint");
	}

	m_ifparsed = false;
	ParsePointlist();
}

bool CDataPointManager::Init()
{
	Clear();
	int tempInt = 0;

	DataPointEntry entry;
	CString cstr;
	const char* pchar = NULL;
	CString strPhysicalID;
	CString strParam3;
	CString strSourceType;
	int i = 1;
	CSqliteAcess access(m_filepath);
	ostringstream sqlstream;
	//sqlstream << "select * from list_point where list_point.type = '0';";
	sqlstream << "select * from list_point order by id ;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	string str;

	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		entry.Clear();  //reset entry

		//OPC id
		entry.SetPointIndex(i++);

		// get shortname
		//str =pElem->Attribute("physicalid");
		if (access.getColumn_Text(1) !=NULL)
		{
			string   Physicalid_temp(const_cast<char*>(access.getColumn_Text(1)));
			strPhysicalID = Physicalid_temp.c_str();
			wstring strBuf = Project::Tools::UTF8ToWideChar(access.getColumn_Text(1));
			entry.SetShortName(strBuf);
		}



		//get point source type
		if (access.getColumn_Text(24) !=NULL)
		{
			string   st_temp(const_cast<char*>(access.getColumn_Text(24)));
			str =st_temp;
			entry.SetSourceType(UtilString::ConvertMultiByteToWideChar(str));
			strSourceType = UtilString::ConvertMultiByteToWideChar(str).c_str();
		}


		//plcid name
		if (access.getColumn_Text(14) !=NULL)
		{
			string   st_temp1(const_cast<char*>(access.getColumn_Text(14)));
			str =st_temp1;
			entry.SetParam(1, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param2
		if (access.getColumn_Text(15) !=NULL)
		{
			string   st_temp2(const_cast<char*>(access.getColumn_Text(15)));
			str =st_temp2;
			entry.SetParam(2, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param3
		if (access.getColumn_Text(16) != NULL)
		{
			string   st_temp3(const_cast<char*>(access.getColumn_Text(16)));
			str =st_temp3;
			strParam3 = UtilString::ConvertMultiByteToWideChar(str).c_str();
			entry.SetParam(3, strParam3.GetString()) ;
		}

		//param4
		if (access.getColumn_Text(17) != NULL)
		{
			string   st_temp4(const_cast<char*>(access.getColumn_Text(17)));
			str =st_temp4;
			entry.SetParam(4, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param5
		if (access.getColumn_Text(18) !=NULL)
		{
			string   st_temp5(const_cast<char*>(access.getColumn_Text(18)));
			str =st_temp5;
			entry.SetParam(5, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param6
		if (access.getColumn_Text(19) != NULL)
		{
			string   st_temp6(const_cast<char*>(access.getColumn_Text(19)));
			str =st_temp6;
			entry.SetParam(6, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param7
		if (access.getColumn_Text(20) != NULL)
		{
			string   st_temp7(const_cast<char*>(access.getColumn_Text(20)));
			str =st_temp7;
			entry.SetParam(7, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param8
		if (access.getColumn_Text(21) != NULL)
		{
			string   st_temp8(const_cast<char*>(access.getColumn_Text(21)));
			str =st_temp8;
			entry.SetParam(8, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param9
		if (access.getColumn_Text(22) !=NULL)
		{
			string   st_temp9(const_cast<char*>(access.getColumn_Text(22)));
			str =st_temp9;
			entry.SetParam(9, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//param10
		if (access.getColumn_Text(23) != NULL)
		{
			string   st_temp10(const_cast<char*>(access.getColumn_Text(23)));
			str =st_temp10;
			entry.SetParam(10, UtilString::ConvertMultiByteToWideChar(str)) ;
		}


		//read write
		pchar = access.getColumn_Text(10);
		if(pchar != NULL){
			str = pchar;
			entry.SetProperty((str == ("0")) ? PLC_READ : PLC_WRITE);
		}
		else{
			entry.SetProperty(PLC_READ);
		}

		//remark
		if (access.getColumn_Text(8) != NULL)
		{
			string   st_remarkp(const_cast<char*>(access.getColumn_Text(8)));
			str = st_remarkp;
			entry.SetDescription(UtilString::ConvertMultiByteToWideChar(str));
		}

		//unit
		pchar = access.getColumn_Text(7);
		if(pchar != NULL)
		{
			str = pchar;  
		}
		else
		{
			str = "";            
		}
		entry.SetUnit(UtilString::ConvertMultiByteToWideChar(str));
		//high
		if (access.getColumn_Text(26) != NULL)
		{
			string   st_high(const_cast<char*>(access.getColumn_Text(26)));
			str = st_high;
			if (str.size()>0)
			{
				entry.SetHighAlarm(atof(str.c_str()));
			}
		}

		//low
		if (access.getColumn_Text(27) != NULL)
		{
			string   st_low(const_cast<char*>(access.getColumn_Text(27)));
			str = st_low;
			if (str.size()>0)
			{
				entry.SetLowAlarm(atof(str.c_str()));
			}
		}
		//open
		if (access.getColumn_Text(28) != NULL)
		{
			string   st_low(const_cast<char*>(access.getColumn_Text(28)));
			str = st_low;
			if (str.size()>0)
			{
				entry.SetOpenToThirdParty(atoi(str.c_str()));  
			}
		}
		wstring wstr = UtilString::ConvertMultiByteToWideChar(str);
		entry.SetUnit(wstr);

		//read cycle store period
		if (access.getColumn_Text(29))
		{
			POINT_STORE_CYCLE cycle = (POINT_STORE_CYCLE)atoi(access.getColumn_Text(29));
			entry.SetStoreCycle(cycle);
		}
		
		if (strPhysicalID.GetLength()>0 && strParam3.GetLength()>0 && strSourceType.CompareNoCase(L"simense1200") == 0)
		{
			CParseStringParam3::Instance().ParseParam3(strPhysicalID,strParam3);
		}

		AddEntry(entry);

		//pElem = pElem->NextSiblingElement(c_str_xml_data.c_str() );
	}

	access.SqlFinalize();
	access.CloseDataBase();
	return true;
}

DataPointEntry CDataPointManager::GetEntry( int index )
{
	DataPointEntry entry;
	map<int, DataPointEntry>::iterator it = m_indexmap.find(index);
	if (it != m_indexmap.end())
	{
		entry =  it->second;
	}

	return entry;
}

DataPointEntry CDataPointManager::GetEntry( const wstring& shortname )
{
	DataPointEntry entry;
	map<wstring, DataPointEntry>::iterator it = m_namemap.find(shortname);
	if (it != m_namemap.end()){
		entry =  it->second;
	}

	return entry;
}

POINT_STORE_CYCLE CDataPointManager::GetPointSavePeriod(const string & pointname)
{
	wstring strPTName = UtilString::ConvertMultiByteToWideChar(pointname);
	DataPointEntry entry;
	map<wstring, DataPointEntry>::iterator it = m_namemap.find(strPTName);
	if (it != m_namemap.end()){
		entry =  it->second;
		return entry.GetStoreCycle();
	}

	return E_STORE_NULL;

}


map<int, DataPointEntry>& CDataPointManager::GetPointMap()
{
	return m_indexmap;
}

void CDataPointManager::GetFCBusPointList( vector<DataPointEntry>& fcbuspointlist )
{
	ParsePointlist();
	fcbuspointlist = m_fcbuspointlist;
}

int CDataPointManager::GetFCbusPointCount()
{
	ParsePointlist();
	return m_fcbuspointlist.size();
}


