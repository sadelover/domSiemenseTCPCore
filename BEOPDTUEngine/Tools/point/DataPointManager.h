
/*
 this class define the plc data point.
*/



#ifdef WIN32
	#pragma once
#endif

#ifndef CPLCDATAPOINT_H__
#define CPLCDATAPOINT_H__

#include "Tools/DllDefine.h"
#include "DataPointEntry.h"


#pragma warning(disable:4251)
#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::wstring;
using std::string;


//////////////////////////////////////////////////////////////////////////

class DLLEXPORT CDataPointManager
{
public:
	enum enumPointSavePeriod
	{
		PS_MINUTE,
		PS_HOUR,
		PS_DAY,
		PS_MONTH,
		PS_YEAR
	};

	// constructor
	CDataPointManager();

	//destructor
	~CDataPointManager();
    
	bool	Init();
public:
	
	void	Clear();
	void	AddEntry(const DataPointEntry& pentry);
	void	ParsePointlist();
	void    SetAllPointTypeAsVirtual();

	void	GetOpcPointList(vector<DataPointEntry>& opclist);
	void	GetModbusPointList(vector<DataPointEntry>& modbuspointlist);
	void	GetFCBusPointList(vector<DataPointEntry>& fcbuspointlist);
	void	GetBacnetPointList(vector<DataPointEntry>& bacnetpointlist);
	void	GetMemoryPointList(vector<DataPointEntry>& memorypointlist);
	void	GetProtocol104PointList(vector<DataPointEntry>& protocol104pointlist);
	void	GetMysqlPointList(vector<DataPointEntry>& mysqlpointlist);
	void    GetSiemensPointList(vector<DataPointEntry>& ptList);

	int    GetOPCPointCount();
	int    GetModbusPointCount();
	int    GetFCbusPointCount();
	int    GetBacnetPointCount();
	int    GetMemoryPointCount();
	int    GetProtocal104PointCount();
	int    GetMySQLPointCount();
	int		GetSiemensPointCount();


	const vector<DataPointEntry>& GetPointList() const;

	void	SetFilePath(const wstring& filepath);

	DataPointEntry	GetEntry(int index);
	DataPointEntry	GetEntry(const wstring& shortname);

	POINT_STORE_CYCLE GetPointSavePeriod(const string & pointname);

	map<int, DataPointEntry>& GetPointMap();

private:

	//===================map for pointindex and entry
	vector<DataPointEntry>  m_pointlist;
	
	//=================== map for long name and pointindex
	vector<DataPointEntry>	 m_opcpointlist;
	
	//===================map for short name and pointindex
	vector<DataPointEntry>	 m_modbuspointlist;

	//===================map for short name and pointindex
	vector<DataPointEntry>	 m_fcbuspointlist;
    
	//=================== map for short name and long name.
	vector<DataPointEntry>  m_bacnetpointlist;

	//=================== map for short name and long name.
	vector<DataPointEntry>  m_protocol104pointlist;

	//=================== map for short name and long name.
	vector<DataPointEntry>  m_mysqlpointlist;

	//=================== map for short name and long name.
	vector<DataPointEntry>  m_siemenspointlist;

	//===================
	vector<DataPointEntry>	m_memorypointlist;

	map<wstring, DataPointEntry>	m_namemap;
	map<int, DataPointEntry>		m_indexmap;

	bool	m_ifparsed;

	string		m_filepath;
};

#endif