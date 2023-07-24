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
map<VARENUM, string>  ClientDataPointEntry::m_mapTypeString;
map<string, VARENUM> ClientDataPointEntry::m_mapStringType;

ClientDataPointEntry::ClientDataPointEntry()
	:index(0)
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

void ClientDataPointEntry::InitVecTypeString()
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

void ClientDataPointEntry::SetPointIndex(int nIndex)
{
	index = nIndex;
}


void ClientDataPointEntry::SetShortName(const wstring& strShortname)
{
	shortname = strShortname;
}

void ClientDataPointEntry::SetDescription(const wstring& strDiscription)
{
	description = strDiscription;
}

void ClientDataPointEntry::SetUnit(const wstring& strUnit)
{
	unit = strUnit;
}

int ClientDataPointEntry::GetPointIndex() const
{
	return index;
}


wstring ClientDataPointEntry::GetShortName() const
{
	return shortname;
}

wstring ClientDataPointEntry::GetDescription() const
{
	return description;
}

wstring ClientDataPointEntry::GetUnit() const
{
	return unit;
}

//////////////////////////////////////////////////////////////add for physical point define

void ClientDataPointEntry::SetSourceType(const wstring& strSourceType)
{
	source = strSourceType;
}

wstring ClientDataPointEntry::GetSourceType() const
{
	return source;
}


void ClientDataPointEntry::SetParam(unsigned int nParaid, const wstring& strParam)
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

wstring ClientDataPointEntry::GetParam(unsigned int nParaid) const
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
bool ClientDataPointEntry::CheckIfDuplicate(const ClientDataPointEntry& other) const
{
	const bool ifDup = (shortname == other.shortname);
	return ifDup;
}

//restore default value
void ClientDataPointEntry::Clear()
{
	index = 0;
	m_value = 0;
	m_fvalue =0;
	m_nErrCore = 0;
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
VARENUM  ClientDataPointEntry::GetType() const
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

string  ClientDataPointEntry::GetTypeString() const
{
	const string str = m_mapTypeString[m_type];
	return str;
}

void ClientDataPointEntry::SetType(VARENUM  type0)
{
	m_type = type0;
}


void  ClientDataPointEntry::SetType(const string& str)
{
	m_type = m_mapStringType[str];
	//return;
}

double ClientDataPointEntry::GetValue() const
{
	return m_value;
}

void ClientDataPointEntry::SetValue( double sval )
{
	m_strvalue = Project::Tools::RemoveDecimalW(sval);
	m_value = sval;
}


wstring ClientDataPointEntry::GetPlcAddress() const
{
	return GetParam(1);
}

wstring ClientDataPointEntry::GetServerProgName() const
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



DWORD ClientDataPointEntry::GetSlaveID() const
{
	wstring strparam1 = GetParam(INDEX_SLAVEID);
	DWORD slaveid = _ttoi(strparam1.c_str());

	return slaveid;
}

DWORD ClientDataPointEntry::GetHeadAddress() const
{
	wstring strparam2 = GetParam(INDEX_HEADADDRESS);
	DWORD headaddress = _ttoi(strparam2.c_str());

	return headaddress;
}

DWORD ClientDataPointEntry::GetFuncCode() const
{
	wstring strparam3 = GetParam(INDEX_FUNCODE);
	DWORD funccode= _ttoi(strparam3.c_str());

	return funccode;
}

//取得倍数,倍数固定,放在param4.
double ClientDataPointEntry::GetMultiple() const
{
	wstring strparam4 = GetParam(INDEX_MULTIPLE);
	if (strparam4.empty()){
		return 1;
	}
	else
		return _ttof(strparam4.c_str());
}

wstring ClientDataPointEntry::GetServerAddress() const
{
	wstring serveraddress = GetParam(INDEX_SERVERIP);
	return serveraddress;
}


VARTYPE ClientDataPointEntry::GetOpcPointType() const
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

ClientDataPointEntry::OPCPOINTTYPE ClientDataPointEntry::GetPointType() const
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
	return POINTTYPE_OPC;
}

bool ClientDataPointEntry::IsOpcPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_OPC);
}

bool ClientDataPointEntry::IsModbusPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_MODBUS);
}

bool ClientDataPointEntry::IsCO3PPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_CO3P);
}

bool ClientDataPointEntry::IsSqlServerPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SQLSERVER);
}

bool ClientDataPointEntry::IsBacnetPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_BACNET);
}

bool ClientDataPointEntry::IsProtocol104Point() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_PROTOCOL104);
}

bool ClientDataPointEntry::IsMysqlPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_MYSQL);
}

bool ClientDataPointEntry::IsSiemens1200Point() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SIEMENSE1200TCP);
}

bool ClientDataPointEntry::IsSiemens300Point() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SIEMENSE300TCP);
}

VARTYPE ClientDataPointEntry::GetMemoryPointType()const
{
	wstring typestring = GetParam(2);
	string typestring_ansi = Project::Tools::WideCharToAnsi(typestring.c_str());
	VARTYPE vval = VT_R4;
	if (!typestring_ansi.empty()){
		vval =  m_mapStringType[typestring_ansi];
	}

	return vval;
}

DWORD ClientDataPointEntry::GetServerID_Bacnet( int nMode /*= 0*/ ) const
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

DWORD ClientDataPointEntry::GetPointType_Bacnet() const
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

DWORD ClientDataPointEntry::GetPointAddress_Bacnet() const
{
	DWORD dwresult = 0;
	dwresult = _ttoi(param3.c_str());

	return dwresult;
}

unsigned int ClientDataPointEntry::GetBacnetInvokeID() const
{
	return bacnet_invoke_id;
}

void ClientDataPointEntry::SetBacnetInvokeID( unsigned int invokeid )
{
	bacnet_invoke_id = invokeid;
}

unsigned int ClientDataPointEntry::GetPointTag_Bacnet() const
{
	return bacnet_tag_type;
}

void ClientDataPointEntry::SetPointTag_Bacnet( unsigned int tagval )
{
	bacnet_tag_type = tagval;
}

bool ClientDataPointEntry::IsVPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_VPOINT);
}

void ClientDataPointEntry::SetHighAlarm( const double dHighAlarm )
{
	m_high_alarm = dHighAlarm;
}


void ClientDataPointEntry::SetLowAlarm( const double dLowAlarm )
{
	m_low_alarm = dLowAlarm;
}


double ClientDataPointEntry::GetHighAlarm() const
{
	return m_high_alarm;
}

double ClientDataPointEntry::GetLowAlarm() const
{
	return m_low_alarm;
}

wstring ClientDataPointEntry::GetSValue() const
{
	return m_strvalue;
}

void ClientDataPointEntry::SetSValue( wstring sval )
{
	m_value = _wtof(sval.c_str());
	m_strvalue = sval;
}


void    ClientDataPointEntry::SetToUpdate()
{

	if(m_nUpdateSignal>10000)
		return;

	m_nUpdateSignal++;
}
void    ClientDataPointEntry::SetUpdated()
{
	m_nUpdateSignal = 0;
}

int	ClientDataPointEntry::GetUpdateSignal()
{
	return m_nUpdateSignal;
}

int		ClientDataPointEntry::GetOpenToThirdParty() const
{
	return m_nOpenToThirdParty;
}
void	ClientDataPointEntry::SetOpenToThirdParty(int nOpen)
{
	m_nOpenToThirdParty = nOpen;
}

bool ClientDataPointEntry::IsFCbusPoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_FCBUS);
}

wstring ClientDataPointEntry::GetFCServerPort() const
{
	wstring serverport = GetParam(2);
	return serverport;
}

wstring ClientDataPointEntry::GetFCServerIP() const
{
	wstring serverip = GetParam(1);
	return serverip;
}

DWORD ClientDataPointEntry::GetFCPointAddress() const
{
	wstring pointaddress = GetParam(3);
	DWORD address = _ttoi(pointaddress.c_str());
	return address;
}

float ClientDataPointEntry::GetFValue() const
{
	return m_fvalue;
}

void ClientDataPointEntry::SetFValue( float sval )
{
	m_fvalue = sval;
}

unsigned int ClientDataPointEntry::GetOPClientIndex() const
{
	return opc_client_index;
}

void ClientDataPointEntry::SetOPClientIndex( unsigned int index )
{
	opc_client_index = index;
}

void ClientDataPointEntry::SetHighHighAlarm( const double dHighHighAlarm )
{
	m_highhigh_alarm = dHighHighAlarm;
}

void ClientDataPointEntry::SetLowLowAlarm( const double dLowLowAlarm )
{
	m_lowlow_alarm = dLowLowAlarm;
}

double ClientDataPointEntry::GetHighHighAlarm() const
{
	return m_highhigh_alarm;
}

double ClientDataPointEntry::GetLowLowAlarm() const
{
	return m_lowlow_alarm;
}

DWORD ClientDataPointEntry::GetOPCDataSourceType() const
{
	wstring strparam5 = GetParam(INDEX_SERVERIP);
	DWORD sourceType = _ttoi(strparam5.c_str());
	return sourceType;
}

WORD ClientDataPointEntry::GetWORD() const
{
	return m_WValue;
}

void ClientDataPointEntry::SetWORD( WORD wValue )
{
	m_WValue = wValue;
}

DWORD ClientDataPointEntry::GetValueType() const
{
	wstring strparam1 = GetParam(INDEX_DATA_TYPE);
	DWORD type = _ttoi(strparam1.c_str());

	return type;
}

double ClientDataPointEntry::GetValueRange() const
{
	wstring strparam4 = GetParam(INDEX_DATA_RANGED);
	if (strparam4.empty()){
		return 1;
	}
	else
		return _ttof(strparam4.c_str());
}

void ClientDataPointEntry::SetModbusMutileEnd( bool bModbusEnd )
{
	m_bModbusEnd = bModbusEnd;
}

bool ClientDataPointEntry::GetModbusMutileEnd()
{
	return m_bModbusEnd;
}

wstring ClientDataPointEntry::GetOPCServerIP() const
{
	wstring strparam6 = GetParam(INDEX_DATA_TYPE);
	return strparam6;
}

bool ClientDataPointEntry::IsSqlitePoint() const
{
	OPCPOINTTYPE pointype = GetPointType();
	return (pointype == POINTTYPE_SQLITE);
}
