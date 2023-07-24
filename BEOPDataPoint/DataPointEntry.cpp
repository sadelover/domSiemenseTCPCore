#include "StdAfx.h"
#include "DataPointEntry.h"

#include "../Tools/CustomTools/CustomTools.h"

const wchar_t* PHOENIX_OPCSERVER_PROGNAME = L"PhoenixContact.AX-Server.21";		//菲尼克斯的PLC
const wchar_t* SIMENS_OPCSERVER_1200 = L"OPC.SimaticNet";						//西门子1200
const wchar_t* AB_OPCSERVER_RSLINK = L"rslinx opc server";						//AB RSlink server
const wchar_t* INSIGHT_OPCSERVER = L"Insight.OPCServerDA.1";				//InsightOPC
const wchar_t* WINCC_OPCSERVER = L"OPCServer.WinCC.1";						//WinCC
const wchar_t* KINGVIEW_OPCSERVER = L"KingView.View.1";						//组态王
const wchar_t* ArchestrA_3_OPCSERVER = L"ArchestrA.FSGateway.3";				//InTouch 3
const wchar_t* KEPWARE_4_OPCSERVER = L"KEPware.KEPServerEx.V4";				//InTouch v4
const wchar_t* ABB_OPCSERVER = L"ABB.MicroSCADA.OPC.Server.DA";				//ABB.MicroSCADA.OPC.Server.DA

//////////////////////////////////////////////////////////////////////////
map<VARENUM, string>  DataPointEntry::m_mapTypeString;
map<string, VARENUM> DataPointEntry::m_mapStringType;

DataPointEntry::DataPointEntry()
	:index(0)
	,rwproperty(PLC_READ)
	,m_type(VT_BSTR)
	,m_value(0)
	,bacnet_invoke_id(0)
	,opc_client_index(0)
	,bacnet_tag_type(0)
	,m_high_alarm(-9999)
	,m_highhigh_alarm(-9999)
	,m_low_alarm(-9999)
	,m_lowlow_alarm(-9999)
	,m_nUpdateSignal(99)
	,param9(L"0")
	,m_fvalue(0)
	,m_WValue(0)
	,m_nErrCore(0)
	,m_bModbusEnd(true)
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

void DataPointEntry::SetEquipmentName(const wstring& strName)
{
	m_wstrOfEquipment = strName;
}
void DataPointEntry::SetSystemName(const wstring& strName)
{
	m_wstrOfSystem = strName;
}
void DataPointEntry::SetGroupName(const wstring& strName)
{
	m_wstrOfGroup = strName;
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

wstring DataPointEntry::GetEquipmentName() const
{
	return m_wstrOfEquipment;
}

wstring DataPointEntry::GetSystemName() const
{
	return m_wstrOfSystem;
}

wstring DataPointEntry::GetGroupName() const
{
	return m_wstrOfGroup;
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
	m_fvalue =0;
	m_nErrCore = 0;
	rwproperty = PLC_READ;
	m_bModbusEnd = true;
	shortname = L"";
	description = L"";
	unit = L"";					//单位
	m_type = VT_BSTR;
	m_WValue = 0;

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

void DataPointEntry::Enable(bool bEnable)
{
	m_bEnabled = bEnable;
}

bool DataPointEntry::GetEnabled()
{
	return m_bEnabled;
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
	m_strvalue = Project::Tools::RemoveDecimalW(sval);
	m_value = sval;

	if(shortname==L"Plant01CWPVSDFreqSetting05" && m_value<=0.0)
	{
		int xx=0;
	}
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
	else if (sourcetype == WINCC1)
	{
		return WINCC_OPCSERVER;
	}
	else if (sourcetype == KINGVIEW1)
	{
		return KINGVIEW_OPCSERVER;
	}
	else if (sourcetype == ARCHESTRA3)
	{
		return ArchestrA_3_OPCSERVER;
	}
	else if (sourcetype == KEPWARE4)
	{
		return KEPWARE_4_OPCSERVER;
	}
	else if (sourcetype == ABBSCADA)
	{
		return ABB_OPCSERVER;
	}
	else if (sourcetype == OPC)
	{
		return param3;
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
	if (sourcetype == SIMENS300 || sourcetype == SIMENS1200 || sourcetype == PH370  || sourcetype == L"OPC" || sourcetype == L"opc" )
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
	}
	else if (sourcetype == L"vpoint" || sourcetype== L"dateTime" ||sourcetype==L"ModbusDirectReadInServer"   ||sourcetype==L"ModbusDirecctReadInServer")
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
	else if(sourcetype == L"DB-SQLSERVER")
	{
		return POINTTYPE_SQLSERVER;
	}
	else if(sourcetype == L"DB-SQLITE")
	{
		return POINTTYPE_SQLITE;
	}
	else if(sourcetype == L"DanfossFCProtocol")
	{
		return POINTTYPE_FCBUS;
	}
	else if(sourcetype == L"CO3P")
	{
		return POINTTYPE_CO3P;
	}
	else if(sourcetype == L"web" || sourcetype == L"WEB")
	{
		return POINTTYPE_WEB;
	}
	else if(sourcetype==L"wireless")
	{
		return POINTTYPE_WIRELESS;
	}
	else if(sourcetype==L"ModbusEquipment")
	{
		return POINTTYPE_MODBUS_EQUIPMENT;
	}
	else if(sourcetype==L"OPCUA")
	{
		return POINTTYPE_OPCUA;
	}
	else if(sourcetype==L"bacnet-py")
	{
		return POINTTYPE_BACNET_PY;
	}
	else if(sourcetype==L"PersagyController")
	{
		return POINTTYPE_PERSAGY_CONTROLLER;
	}
	else if(sourcetype==L"ThirdParty")
	{
		return POINTTYPE_THIRD_PARTY;
	}
	else if(sourcetype==L"")
	{
		return POINTTYPE_UNKNOWN;
	}
	return POINTTYPE_OPC; //如果没有的类型，都认为是vpoint
}


bool DataPointEntry::IsUnknownPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_UNKNOWN);
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

bool DataPointEntry::IsCO3PPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_CO3P);
}

bool DataPointEntry::IsSqlServerPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SQLSERVER);
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

DWORD DataPointEntry::GetServerID_Bacnet( int nMode /*= 0*/ ) const
{
	if(nMode == 0)
	{
		DWORD dwresult = 0;
		dwresult = _ttoi(param1.c_str());
		return dwresult;
	}
	else
	{
		return inet_addr(Project::Tools::WideCharToAnsi(param1.c_str()).c_str());
	}
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

	m_value = _wtof(sval.c_str());
	m_strvalue = sval;

	if(shortname==L"Plant01CWPVSDFreqSetting05" && m_value<=0.0)
	{
		int xx=0;
	}
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

float DataPointEntry::GetFValue() const
{
	return m_fvalue;
}

void DataPointEntry::SetFValue( float sval )
{
	m_fvalue = sval;
}

unsigned int DataPointEntry::GetOPClientIndex() const
{
	return opc_client_index;
}

void DataPointEntry::SetOPClientIndex( unsigned int index )
{
	opc_client_index = index;
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

DWORD DataPointEntry::GetOPCDataSourceType() const
{
	wstring strparam5 = GetParam(INDEX_SERVERIP);
	DWORD sourceType = _ttoi(strparam5.c_str());
	return sourceType;
}

WORD DataPointEntry::GetWORD() const
{
	return m_WValue;
}

void DataPointEntry::SetWORD( WORD wValue )
{
	m_WValue = wValue;
}

DWORD DataPointEntry::GetValueType() const
{
	wstring strparam1 = GetParam(INDEX_DATA_TYPE);
	DWORD type = _ttoi(strparam1.c_str());

	return type;
}

double DataPointEntry::GetValueRange() const
{
	wstring strparam4 = GetParam(INDEX_DATA_RANGED);
	if (strparam4.empty()){
		return 1;
	}
	else
		return _ttof(strparam4.c_str());
}

void DataPointEntry::SetModbusMutileEnd( bool bModbusEnd )
{
	m_bModbusEnd = bModbusEnd;
}

bool DataPointEntry::GetModbusMutileEnd()
{
	return m_bModbusEnd;
}

wstring DataPointEntry::GetOPCServerIP() const
{
	wstring strparam6 = GetParam(INDEX_DATA_TYPE);
	return strparam6;
}

bool DataPointEntry::IsSqlitePoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SQLITE);
}

VARENUM DataPointEntry::GetPlcValueType() const
{
	wstring typestring = GetParam(OPC_INDEX_POINTTYPE);
	string typestring_ansi = Project::Tools::WideCharToAnsi(typestring.c_str());
	VARENUM vval = VT_EMPTY;
	if (!typestring_ansi.empty())
	{
		vval =  m_mapStringType[typestring_ansi];
	}
	return vval;
}

bool DataPointEntry::IsWebPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_WEB);
}


bool DataPointEntry::IsWirelessPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_WIRELESS);
}

bool DataPointEntry::IsModbusEquipmentPoint() const
{

	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_MODBUS_EQUIPMENT);
}


bool DataPointEntry::IsBacnetPyPoint() const
{

	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_BACNET_PY);
}
bool DataPointEntry::IsOPCUAPoint() const
{

	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_OPCUA);
}

bool DataPointEntry::IsPersagyControllerPoint() const
{

	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_PERSAGY_CONTROLLER);
};

bool DataPointEntry::IsThirdPartyPoint() const
{

	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_THIRD_PARTY);
}

double DataPointEntry::GetMultipleReadValue( double dOldValue, bool bRead ) const
{
	wstring strparam4 = GetParam(INDEX_MULTIPLE);			//倍率 需要判断是否含有:
	if(strparam4.length() > 0)
	{
		int nFindPos = strparam4.find(':');
		if(nFindPos>0)		//需要枚举转换 
		{
			vector<wstring> vecChange;
			Project::Tools::SplitStringByChar(strparam4.c_str(),L';',vecChange);
			for(int i=0; i<vecChange.size(); ++i)
			{
				wstring wstrValue = vecChange[i];
				if(wstrValue.length() > 0)
				{
					vector<wstring> vecChangeValue;
					Project::Tools::SplitStringByChar(wstrValue.c_str(),L':',vecChangeValue);	//读取值:真实值

					if(bRead)			//读取时候  将读取值翻译成真实值
					{
						if(vecChangeValue.size() == 2)
						{
							if(_ttof(vecChangeValue[0].c_str()) == dOldValue)		//值匹配相等
							{
								return _ttof(vecChangeValue[1].c_str());			//真实值替换读写值
							}
						}
					}
					else
					{
						if(vecChangeValue.size() == 2)
						{
							if(_ttof(vecChangeValue[1].c_str()) == dOldValue)		//值匹配相等
							{
								return _ttof(vecChangeValue[0].c_str());			//读取值发送到现场
							}
						}
					}
				}
			}
		}			//倍率处理
		else		//判断是否为0
		{
			double dMulti = _ttof(strparam4.c_str());
			if(abs(dMulti) > 1e-10)		//倍率不等于0
			{
				if(bRead)			//读取值
				{
					return dOldValue/dMulti;
				}
				else				//写值
				{
					return dOldValue*dMulti;
				}
			}			
		}
	}
	return dOldValue;

}
