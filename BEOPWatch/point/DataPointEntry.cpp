#include "StdAfx.h"
#include "DataPointEntry.h"

#include "../Tools/CustomTools/CustomTools.h"

const wchar_t* PHOENIX_OPCSERVER_PROGNAME = L"PhoenixContact.AX-Server.21";		//菲尼克斯的PLC
const wchar_t* SIMENS_OPCSERVER_1200 = L"OPC.SimaticNet";						//西门子1200
const wchar_t* AB_OPCSERVER_RSLINK = L"rslinx opc server";						//AB RSlink server
const wchar_t* INSIGHT_OPCSERVER = L"Insight.OPCServerDA.1";				//InsightOPC

//////////////////////////////////////////////////////////////////////////
map<VARENUM, string>  DataPointEntry::m_mapTypeString;
map<string, VARENUM> DataPointEntry::m_mapStringType;

DataPointEntry::DataPointEntry()
	:index(0)
	,rwproperty(PLC_READ)
	,m_type(VT_BSTR)
	,m_value(0)
	,bacnet_invoke_id(0)
	,bacnet_tag_type(0)
	,m_high_alarm(-9999)
	,m_highhigh_alarm(-9999)
	,m_low_alarm(-9999)
	,m_lowlow_alarm(-9999)
	,m_nUpdateSignal(99)
	,param9(L"0")
{
	InitVecTypeString();
}

void DataPointEntry::InitVecTypeString()
{
	//const int size0 = m_mapTypeString.size();
	if(m_mapTypeString.size() > 0
		&& m_mapStringType.size() > 0)
		return;

	m_mapTypeString[VT_BOOL] = "VT_BOOL" ;
	m_mapTypeString[VT_I1] = "VT_I1" ;
	m_mapTypeString[VT_I2] = "VT_I2" ;
	m_mapTypeString[VT_I4] = "VT_I4" ;
	m_mapTypeString[VT_INT] = "VT_INT" ;
	m_mapTypeString[VT_UI1] = "VT_UI1" ;
	m_mapTypeString[VT_UI2] = "VT_UI2" ;
	m_mapTypeString[VT_UI4] = "VT_UI4" ;
	m_mapTypeString[VT_UINT] = "VT_UINT" ;
	m_mapTypeString[VT_R4] = "VT_R4" ;
	m_mapTypeString[VT_R8] = "VT_R8" ;
	m_mapTypeString[VT_BSTR] = "VT_BSTR" ;
	m_mapTypeString[VT_CY] = "VT_CY" ;
	m_mapTypeString[VT_DATE] = "VT_DATE" ;
	m_mapTypeString[VT_DECIMAL] = "VT_DECIMAL" ;
	m_mapTypeString[VT_DISPATCH] = "VT_DISPATCH" ;
	m_mapTypeString[VT_EMPTY] = "VT_EMPTY" ;
	m_mapTypeString[VT_ERROR] = "VT_ERROR" ;
	m_mapTypeString[VT_NULL] = "VT_NULL" ;
	m_mapTypeString[VT_UNKNOWN] = "VT_UNKNOWN" ;

	m_mapStringType["VT_BOOL"] = VT_BOOL;
	m_mapStringType["VT_I1"] = VT_I1;
	m_mapStringType["VT_I2"] = VT_I2;
	m_mapStringType["VT_I4"] = VT_I4;
	m_mapStringType["VT_INT"] = VT_INT;
	m_mapStringType["VT_UI1"] = VT_UI1;
	m_mapStringType["VT_UI2"] = VT_UI2;
	m_mapStringType["VT_UI4"] = VT_UI4;
	m_mapStringType["VT_UINT"] = VT_UINT;
	m_mapStringType["VT_R4"] = VT_R4;
	m_mapStringType["VT_R8"] = VT_R8;
	m_mapStringType["VT_BSTR"] = VT_BSTR;
	m_mapStringType["VT_CY"] = VT_CY;
	m_mapStringType["VT_DATE"] = VT_DATE;
	m_mapStringType["VT_DECIMAL"] = VT_DECIMAL;
	m_mapStringType["VT_DISPATCH"] = VT_DISPATCH;
	m_mapStringType["VT_EMPTY"] = VT_EMPTY;
	m_mapStringType["VT_ERROR"] = VT_ERROR;
	m_mapStringType["VT_NULL"] = VT_NULL;
	m_mapStringType["VT_UNKNOWN"] = VT_UNKNOWN;

}

void DataPointEntry::SetPointIndex(int nIndex)
{
	index = nIndex;
}


void DataPointEntry::SetShortName(const wstring& strShortname)
{
	shortname = strShortname;
}

void DataPointEntry::SetDescription(const wstring& strDiscription)
{
	description = strDiscription;
}

void DataPointEntry::SetProperty(PLCVALUEPROPERTY InProperty)
{
	rwproperty = InProperty;
}

void DataPointEntry::SetUnit(const wstring& strUnit)
{
	unit = strUnit;
}

int DataPointEntry::GetPointIndex() const
{
	return index;
}


wstring DataPointEntry::GetShortName() const
{
	return shortname;
}

wstring DataPointEntry::GetDescription() const
{
	return description;
}

PLCVALUEPROPERTY DataPointEntry::GetProperty() const
{
	return rwproperty;
}

bool DataPointEntry::IsReadProperty()
{
	bool bReadFlg = false;
	if (PLC_READ == rwproperty)
	{
		bReadFlg = true;
	}
	return bReadFlg;
}

wstring DataPointEntry::GetUnit() const
{
	return unit;
}

//////////////////////////////////////////////////////////////add for physical point define

void DataPointEntry::SetSourceType(const wstring& strSourceType)
{
	source = strSourceType;
}

wstring DataPointEntry::GetSourceType() const
{
	return source;
}


void DataPointEntry::SetParam(unsigned int nParaid, const wstring& strParam)
{
	switch(nParaid)
	{
	case 1:
		param1 = strParam;
		break;
	case 2:
		param2 = strParam;
		break;
	case 3:
		param3 = strParam;
		break;
	case 4:
		param4 = strParam;
		break;
	case 5:
		param5 = strParam;
		break;
	case 6:
		param6 = strParam;
		break;
	case 7:
		param7 = strParam;
		break;
	case 8:
		param8 = strParam;
		break;
	case 9:
		param9 = strParam;
		break;
	case 10:
		param10 = strParam;
		break;
	case 12:
		param12 = strParam;
		break;
	case 13:
		param13 = strParam;
		break;
	case 14:
		param14 = strParam;
		break;
	case 15:
		param15 = strParam;
		break;
	case 16:
		param16 = strParam;
		break;
	case 17:
		param17 = strParam;
		break;
	case 18:
		param18 = strParam;
		break;
	default:
		break;
	}
}

wstring DataPointEntry::GetParam(unsigned int nParaid) const
{
	wstring strParam = L"";
	switch(nParaid)
	{
	case 1:
		strParam = param1;
		break;
	case 2:
		strParam = param2;
		break;
	case 3:
		strParam = param3;
		break;
	case 4:
		strParam = param4;
		break;
	case 5:
		strParam = param5;
		break;
	case 6:
		strParam = param6;
		break;
	case 7:
		strParam = param7;
		break;
	case 8:
		strParam = param8;
		break;
	case 9:
		strParam = param9;
		break;
	case 10:
		strParam = param10;
		break;
	case 12:
		strParam = param12;
		break;
	case 13:
		strParam = param13;
		break;
	case 14:
		strParam = param14;
		break;
	case 15:
		strParam = param15;
		break;
	case 16:
		strParam = param16;
		break;
	case 17:
		strParam = param17;
		break;
	case 18:
		strParam = param18;
		break;
	default:
		break;
	}

	return strParam;
}



//bird add 
//check if same with other entry
bool DataPointEntry::CheckIfDuplicate(const DataPointEntry& other) const
{
	const bool ifDup = (shortname == other.shortname);
	return ifDup;
}

//restore default value
void DataPointEntry::Clear()
{
	index = 0;
	m_value = 0;
	rwproperty = PLC_READ;

	shortname = L"";
	description = L"";
	unit = L"";					//单位
	m_type = VT_BSTR;


	source = L"";			//点位来源类型：phynix, simens, modbus, bacnet
	param1 = L"";			//长名	
	param2 = L"";				
	param3 = L"";					//

	param4 = L"";			//
	param5 = L"";		    //

	param6 = L"";	//
	param7 = L"";	//
	param8 = L"";	//
	param9 = L"0";	//
	param10 = L"";
	param12 = _T("");
	param13 = _T("");
	param14 = _T("");
	m_high_alarm = -9999;
	m_highhigh_alarm = -9999;
	m_low_alarm = -9999;
	m_lowlow_alarm = -9999;
}

//////////////////////////////////////////////////////////////////////////
VARENUM  DataPointEntry::GetType() const
{
	if(L"simense1200" == source || L"phynix" == source || L"vpoint" == source )
	{
		return (VARENUM) GetOpcPointType();
	}
	else if((L"modbus" == source) || (L"bacnet" == source))
	{
		return VT_R4;
	}
	return m_type;
}

string  DataPointEntry::GetTypeString() const
{
	const string str = m_mapTypeString[m_type];
	return str;
}

void DataPointEntry::SetType(VARENUM  type0)
{
	m_type = type0;
}


void  DataPointEntry::SetType(const string& str)
{
	m_type = m_mapStringType[str];
	//return;
}

double DataPointEntry::GetValue() const
{
	return m_value;
}

void DataPointEntry::SetValue( double sval )
{
	m_value = sval;
}


wstring DataPointEntry::GetPlcAddress() const
{
	return GetParam(1);
}

OPCSERVER_TYPE DataPointEntry::GetServerType() const
{
	const wstring& sourcetype = GetSourceType();
	if (sourcetype == L"phoenix370"){
		return TYPE_AUTOBROWSEITEM;
	}else
		return TYPE_MANUALBROWSEITEM;
}

wstring DataPointEntry::GetServerProgName() const
{
	const wstring& sourcetype = GetSourceType();
	if (sourcetype == PH370){
		return PHOENIX_OPCSERVER_PROGNAME;
	}
	else if (sourcetype == SIMENS1200)
	{
		return	SIMENS_OPCSERVER_1200;
	}
	else if (sourcetype == SIMENS300)
	{
		return SIMENS_OPCSERVER_1200;
	}
	else if (sourcetype == AB500)
	{
		return AB_OPCSERVER_RSLINK;
	}
	else if (sourcetype == INSIGHT)
	{
		return INSIGHT_OPCSERVER;
	}
	else
	{
		return L"";
	}
}



DWORD DataPointEntry::GetSlaveID() const
{
	wstring strparam1 = GetParam(INDEX_SLAVEID);
	DWORD slaveid = _ttoi(strparam1.c_str());

	return slaveid;
}

DWORD DataPointEntry::GetHeadAddress() const
{
	wstring strparam2 = GetParam(INDEX_HEADADDRESS);
	DWORD headaddress = _ttoi(strparam2.c_str());

	return headaddress;
}

DWORD DataPointEntry::GetFuncCode() const
{
	wstring strparam3 = GetParam(INDEX_FUNCODE);
	DWORD funccode= _ttoi(strparam3.c_str());

	return funccode;
}

//取得倍数,倍数固定,放在param4.
double DataPointEntry::GetMultiple() const
{
	wstring strparam4 = GetParam(INDEX_MULTIPLE);
	if (strparam4.empty()){
		return 1;
	}
	else
		return _ttof(strparam4.c_str());
}

wstring DataPointEntry::GetServerAddress() const
{
	wstring serveraddress = GetParam(INDEX_SERVERIP);
	return serveraddress;
}

DWORD DataPointEntry::GetPortNum() const
{
	wstring strportnum = GetParam(INDEX_PORTNUMBER);
	DWORD portnum = _ttoi(strportnum.c_str());

	return portnum;
}

VARTYPE DataPointEntry::GetOpcPointType() const
{
	wstring typestring = GetParam(OPC_INDEX_POINTTYPE);
	
	string typestring_ansi = Project::Tools::WideCharToAnsi(typestring.c_str());

	VARTYPE vval = VT_BOOL;
	if (!typestring_ansi.empty())
	{
		vval =  m_mapStringType[typestring_ansi];
	}

	return vval;

}

DataPointEntry::OPCPOINTTYPE DataPointEntry::GetPointType() const
{
	wstring sourcetype = GetSourceType();
	if (sourcetype == SIMENS300 || sourcetype == SIMENS1200 || sourcetype == PH370)
	{
		return POINTTYPE_OPC;
	}
	else if (sourcetype == L"simense1200tcp" || sourcetype == L"simense1200TCP")
	{
		return POINTTYPE_SIEMENSE1200TCP;
	}
	else if (sourcetype == L"simense300tcp" || sourcetype == L"simense300TCP")
	{
		return POINTTYPE_SIEMENSE300TCP;
	}
	else if (sourcetype == L"modbus")
	{
		return POINTTYPE_MODBUS;
	}
	else if (sourcetype == L"bacnet")
	{
		return POINTTYPE_BACNET;
	}else if (sourcetype == L"ModbusEquipment")
	{
		return POINTTYPE_MODBUS_EQUIPMENT;
	}else if (sourcetype == L"ThirdParty")
	{
		return POINTTYPE_THIRD_PARTY;
	}
	else if (sourcetype == L"vpoint" || sourcetype== L"dateTime")
	{
		return POINTTYPE_VPOINT;
	}
	else if(sourcetype == L"protocol104")
	{
		return POINTTYPE_PROTOCOL104;
	}
	else if(sourcetype == L"DB-MySQL")
	{
		return POINTTYPE_MYSQL;
	}
	else if(sourcetype == L"DanfossFCProtocol")
	{
		return POINTTYPE_FCBUS;
	}
	return POINTTYPE_OPC;
}

bool DataPointEntry::IsOpcPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_OPC);
}

bool DataPointEntry::IsModbusPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_MODBUS);
}

bool DataPointEntry::IsBacnetPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_BACNET);
}

bool DataPointEntry::IsProtocol104Point() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_PROTOCOL104);
}

bool DataPointEntry::IsMysqlPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_MYSQL);
}

bool DataPointEntry::IsSiemens1200Point() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SIEMENSE1200TCP);
}

bool DataPointEntry::IsSiemens300Point() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SIEMENSE300TCP);
}

VARTYPE DataPointEntry::GetMemoryPointType()const
{
	wstring typestring = GetParam(2);
	string typestring_ansi = Project::Tools::WideCharToAnsi(typestring.c_str());
	VARTYPE vval = VT_R4;
	if (!typestring_ansi.empty()){
		vval =  m_mapStringType[typestring_ansi];
	}

	return vval;
}

DWORD DataPointEntry::GetServerID_Bacnet() const
{
	DWORD dwresult = 0;
	dwresult = _ttoi(param1.c_str());

	return dwresult;
}

DWORD DataPointEntry::GetPointType_Bacnet() const
{
	if (param2 == L"AI"){
		return OBJECT_ANALOG_INPUT;
	}
	else if (param2 == L"AO"){
		return OBJECT_ANALOG_OUTPUT;
	}
	else if(param2 == L"AV"){
		return OBJECT_ANALOG_VALUE;
	}
	else if(param2 == L"BI"){
		return OBJECT_BINARY_INPUT;
	}
	else if (param2 == L"BO"){
		return OBJECT_BINARY_OUTPUT;
	} 
	else if(param2 == L"BV"){
		return OBJECT_BINARY_VALUE;
	}
	else if (param2 == L"MI"){
		return OBJECT_MULTI_STATE_INPUT;
	} 
	else if(param2 == L"MO"){
		return OBJECT_MULTI_STATE_OUTPUT;
	}
	else if(param2 == L"MV"){
		return OBJECT_MULTI_STATE_VALUE;
	}
	return OBJECT_ANALOG_INPUT;
}

DWORD DataPointEntry::GetPointAddress_Bacnet() const
{
	DWORD dwresult = 0;
	dwresult = _ttoi(param3.c_str());

	return dwresult;
}

unsigned int DataPointEntry::GetBacnetInvokeID() const
{
	return bacnet_invoke_id;
}

void DataPointEntry::SetBacnetInvokeID( unsigned int invokeid )
{
	bacnet_invoke_id = invokeid;
}

unsigned int DataPointEntry::GetPointTag_Bacnet() const
{
	return bacnet_tag_type;
}

void DataPointEntry::SetPointTag_Bacnet( unsigned int tagval )
{
	bacnet_tag_type = tagval;
}

bool DataPointEntry::IsVPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_VPOINT);
}

void DataPointEntry::SetHighAlarm( const double dHighAlarm )
{
	m_high_alarm = dHighAlarm;
}


void DataPointEntry::SetLowAlarm( const double dLowAlarm )
{
	m_low_alarm = dLowAlarm;
}


double DataPointEntry::GetHighAlarm() const
{
	return m_high_alarm;
}

double DataPointEntry::GetLowAlarm() const
{
	return m_low_alarm;
}


void DataPointEntry::SetStoreCycle( const POINT_STORE_CYCLE cycle )
{
	m_store_cycle = cycle;
}

const POINT_STORE_CYCLE DataPointEntry::GetStoreCycle() const
{
	return m_store_cycle;
}

wstring DataPointEntry::GetSValue() const
{
	return m_strvalue;
}

void DataPointEntry::SetSValue( wstring sval )
{
	m_strvalue = sval;
}


void    DataPointEntry::SetToUpdate()
{

	if(m_nUpdateSignal>10000)
		return;

	m_nUpdateSignal++;
}
void    DataPointEntry::SetUpdated()
{
	m_nUpdateSignal = 0;
}

int	DataPointEntry::GetUpdateSignal()
{
	return m_nUpdateSignal;
}

int		DataPointEntry::GetOpenToThirdParty() const
{
	return m_nOpenToThirdParty;
}
void	DataPointEntry::SetOpenToThirdParty(int nOpen)
{
	m_nOpenToThirdParty = nOpen;
}

bool DataPointEntry::IsFCbusPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_FCBUS);
}

wstring DataPointEntry::GetFCServerPort() const
{
	wstring serverport = GetParam(2);
	return serverport;
}

wstring DataPointEntry::GetFCServerIP() const
{
	wstring serverip = GetParam(1);
	return serverip;
}

DWORD DataPointEntry::GetFCPointAddress() const
{
	wstring pointaddress = GetParam(3);
	DWORD address = _ttoi(pointaddress.c_str());
	return address;
}

void DataPointEntry::SetHighHighAlarm( const double dHighHighAlarm )
{
	m_highhigh_alarm = dHighHighAlarm;
}

void DataPointEntry::SetLowLowAlarm( const double dLowLowAlarm )
{
	m_lowlow_alarm = dLowLowAlarm;
}

double DataPointEntry::GetHighHighAlarm() const
{
	return m_highhigh_alarm;
}

double DataPointEntry::GetLowLowAlarm() const
{
	return m_lowlow_alarm;
}
