#pragma once


#include <string>
#include <map>


using std::wstring;
using std::string;
using std::map;

#ifndef PH370
#define PH370 _T("phoenix370")
#endif

#ifndef SIMENS1200
#define SIMENS1200 _T("simense1200")
#endif

#ifndef SIMENS300
#define SIMENS300 _T("simense300")
#endif

#ifndef SIMENS1200TCP
#define SIMENS1200TCP _T("simense1200TCP")
#endif

#ifndef SIMENS300TCP
#define SIMENS300TCP _T("simense300TCP")
#endif

#ifndef AB500
#define AB500 _T("ab500")
#endif

#ifndef INSIGHT
#define INSIGHT _T("Insight")
#endif

enum PLCVALUEPROPERTY
{
	PLC_READ=0,		// read
	PLC_WRITE,		// write

	PLC_PROP_MAX,
};



enum POINT_STORE_CYCLE
{
	E_STORE_NULL = 0,
	E_STORE_FIVE_SECOND,
	E_STORE_ONE_MINUTE,
	E_STORE_FIVE_MINUTE,
	E_STORE_HALF_HOUR,
	E_STORE_ONE_HOUR,
	E_STORE_ONE_DAY,
	E_STORE_ONE_WEEK,
	E_STORE_ONE_MONTH,
	E_STORE_ONE_YEAR,
	E_STORE_NO_SAVE,
};


enum OPCSERVER_TYPE
{
	TYPE_AUTOBROWSEITEM,		//支持自动浏览点位
	TYPE_MANUALBROWSEITEM,		//不支持自动浏览点位,只能手动增加点位
};

class  DataPointEntry
{
public:

	DataPointEntry();

	// 设置点位
	void SetPointIndex(int nIndex); 

	// 设置plc变量短名
	void SetShortName(const wstring& strShortname);

	// 设置描述
	void SetDescription(const wstring& strDiscription);

	// 设置读写标识
	void SetProperty(PLCVALUEPROPERTY InProperty);

	// 设置单位
	void SetUnit(const wstring& strUnit);
	
	// 获取点位
	int GetPointIndex() const; 

	// 获取plc变量短名
	wstring GetShortName() const;

	// 获取描述
	wstring GetDescription() const;

	// 获取读写标识
	PLCVALUEPROPERTY GetProperty() const;

	//读标识时为true，否则为false
	bool IsReadProperty();

	// 获得单位
	wstring GetUnit() const;

	// 设置SourceType
	void SetSourceType(const wstring& strSourceType);
	// 获取SourceType
	wstring GetSourceType() const;

	// 设置第n个Param
	void SetParam(unsigned int nParaid, const wstring& strParam);

	// 获取第n个Param
	wstring GetParam(unsigned int nParaid) const;
	//设置高位报警
	void SetHighAlarm(const double dHighAlarm);

	//设置低位报警
	void SetLowAlarm(const double dLowAlarm);

	//获取高位报警
	double GetHighAlarm() const;

	//获取低位报警
	double GetLowAlarm() const;

	//设置高高位报警
	void SetHighHighAlarm(const double dHighHighAlarm);

	//设置低低位报警
	void SetLowLowAlarm(const double dLowLowAlarm);

	//获取高高位报警
	double GetHighHighAlarm() const;

	//获取低低位报警
	double GetLowLowAlarm() const;

	VARENUM             GetType() const;
	string              GetTypeString() const;
	void                SetType(VARENUM  type0);
	void                SetType(const string& str);

	bool    CheckIfDuplicate(const DataPointEntry& entry1) const;

	void    Clear();

	void    InitVecTypeString();

	double	GetValue() const;
	void	SetValue(double sval);

	wstring	GetSValue() const;
	void	SetSValue(wstring sval);

	void	SetStoreCycle(const POINT_STORE_CYCLE cycle);
	const POINT_STORE_CYCLE GetStoreCycle() const;

	void    SetToUpdate();
	void    SetUpdated();
	int		GetUpdateSignal();
	int		GetOpenToThirdParty() const;
	void	SetOpenToThirdParty(int nOpen);

private:
	int index;						//点位
	wstring shortname;				//plc变量短名
	wstring description;			//描述	
	wstring	unit;					//单位
	int m_nUpdateSignal;			//读取更新用的标记

	PLCVALUEPROPERTY    rwproperty;
	VARENUM             m_type;
	
	wstring source;		//点位来源类型：phynix, simens, modbus, bacnet
	wstring param1;		//		
	wstring param2;		//		
	wstring	param3;		//

	wstring  param4;	//
	wstring  param5;	//

	wstring  param6;	//
	wstring  param7;	//
	wstring  param8;	//
	wstring  param9;	//
	wstring  param10;	//
	wstring param12;
	wstring param13;
	wstring param14;
	wstring	param15;
	wstring	param16;
	wstring	param17;
	wstring	param18;
	wstring  m_strvalue;

	static  map<VARENUM, string>      m_mapTypeString;
	static  map<string, VARENUM>      m_mapStringType;

	double m_value;

	double m_high_alarm;
	double m_highhigh_alarm;
	double m_low_alarm;
	double m_lowlow_alarm;


	int m_nOpenToThirdParty;

	POINT_STORE_CYCLE m_store_cycle;
public:

	wstring			GetPlcAddress()const;
	OPCSERVER_TYPE	GetServerType() const;
	wstring			GetServerProgName() const;
	VARTYPE			GetOpcPointType() const;
enum
{
	OPC_INDEX_LONGNAME = 1,
	OPC_INDEX_POINTTYPE
};

enum OPCPOINTTYPE
{
	POINTTYPE_OPC,
	POINTTYPE_MODBUS,
	POINTTYPE_BACNET,
	POINTTYPE_VPOINT,
	POINTTYPE_PROTOCOL104,
	POINTTYPE_MYSQL,
	POINTTYPE_SIEMENSE1200TCP,
	POINTTYPE_SIEMENSE300TCP,
	POINTTYPE_FCBUS,
	POINTTYPE_AB500,
	POINTTYPE_INSIGHT,
};

OPCPOINTTYPE GetPointType() const;
bool	IsOpcPoint() const;
bool	IsModbusPoint() const;
bool	IsFCbusPoint() const;
bool	IsBacnetPoint() const;
bool	IsProtocol104Point() const;
bool	IsMysqlPoint() const;
bool    IsVPoint() const;
bool	IsSiemens1200Point() const;
bool	IsSiemens300Point() const;

public:
	DWORD	GetSlaveID() const;
	DWORD	GetHeadAddress() const;
	DWORD	GetFuncCode() const;
	double	GetMultiple() const;
	wstring	GetServerAddress() const;
	wstring	GetFCServerPort() const;
	wstring	GetFCServerIP() const;
	DWORD	GetFCPointAddress() const;
	DWORD	GetPortNum() const;

	enum {
		INDEX_SLAVEID = 1,
		INDEX_HEADADDRESS,
		INDEX_FUNCODE,
		INDEX_MULTIPLE,
		INDEX_SERVERIP,
		INDEX_PORTNUMBER
	};

public:
	VARTYPE	GetMemoryPointType()const;

	// bacnet ctrl points
public:
	enum eBacnetDeviceType{
		OBJECT_ANALOG_INPUT = 0,
		OBJECT_ANALOG_OUTPUT = 1,
		OBJECT_ANALOG_VALUE = 2,
		OBJECT_BINARY_INPUT = 3,
		OBJECT_BINARY_OUTPUT = 4,
		OBJECT_BINARY_VALUE = 5,
		OBJECT_MULTI_STATE_INPUT = 13,
		OBJECT_MULTI_STATE_OUTPUT = 14,
		OBJECT_MULTI_STATE_VALUE = 19,
		OBJECT_MAX, 
	};

	DWORD	GetServerID_Bacnet() const;
	DWORD	GetPointType_Bacnet() const;
	DWORD	GetPointAddress_Bacnet() const;

	unsigned int GetBacnetInvokeID() const;
	void	SetBacnetInvokeID(unsigned int invokeid);

	unsigned int GetPointTag_Bacnet() const;
	void SetPointTag_Bacnet(unsigned int tagval);
	unsigned int bacnet_invoke_id;
	unsigned int bacnet_tag_type;
};

