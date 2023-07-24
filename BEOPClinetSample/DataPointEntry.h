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

#ifndef WINCC1
#define WINCC1 _T("WinCC1")
#endif

#ifndef KINGVIEW1
#define KINGVIEW1 _T("KingView1")
#endif

#ifndef ARCHESTRA3
#define ARCHESTRA3 _T("ArchestrA3")
#endif

#ifndef KEPWARE4
#define KEPWARE4 _T("KEPware4")
#endif

#ifndef ABBSCADA
#define ABBSCADA _T("ABBScada")
#endif

#ifndef OPC
#define OPC _T("OPC")
#endif

class  ClientDataPointEntry
{
public:

	ClientDataPointEntry();

	// 设置点位
	void SetPointIndex(int nIndex); 

	// 设置plc变量短名
	void SetShortName(const wstring& strShortname);

	// 设置描述
	void SetDescription(const wstring& strDiscription);

	// 设置单位
	void SetUnit(const wstring& strUnit);
	
	// 获取点位
	int GetPointIndex() const; 

	// 获取plc变量短名
	wstring GetShortName() const;

	// 获取描述
	wstring GetDescription() const;

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

	bool    CheckIfDuplicate(const ClientDataPointEntry& entry1) const;

	void    Clear();

	void    InitVecTypeString();

	double	GetValue() const;
	void	SetValue(double sval);

	float	GetFValue() const;
	void	SetFValue(float sval);

	wstring	GetSValue() const;
	void	SetSValue(wstring sval);

	WORD	GetWORD() const;
	void	SetWORD(WORD wValue);

	void    SetToUpdate();
	void    SetUpdated();
	int		GetUpdateSignal();
	int		GetOpenToThirdParty() const;
	void	SetOpenToThirdParty(int nOpen);

	void	SetModbusMutileEnd(bool bModbusEnd);
	bool	GetModbusMutileEnd();

private:
	int index;						//点位
	wstring shortname;				//plc变量短名
	wstring description;			//描述	
	wstring	unit;					//单位
	int m_nUpdateSignal;			//读取更新用的标记
	int	m_nErrCore;					//错误代码

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

	wstring  m_strvalue;

	static  map<VARENUM, string>      m_mapTypeString;
	static  map<string, VARENUM>      m_mapStringType;

	double m_value;
	float  m_fvalue;
	double m_high_alarm;
	double m_highhigh_alarm;
	double m_low_alarm;
	double m_lowlow_alarm;
	WORD	m_WValue;
	bool	m_bModbusEnd;			//用于标志Modbus多寄存器的最后一个地址
	int m_nOpenToThirdParty;

public:

	wstring			GetPlcAddress()const;
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
	POINTTYPE_SQLITE,
	POINTTYPE_CO3P,
	POINTTYPE_SQLSERVER,
};

OPCPOINTTYPE GetPointType() const;
bool	IsOpcPoint() const;
bool	IsModbusPoint() const;
bool	IsCO3PPoint() const;
bool	IsSqlServerPoint() const;
bool	IsFCbusPoint() const;
bool	IsBacnetPoint() const;
bool	IsProtocol104Point() const;
bool	IsMysqlPoint() const;
bool	IsSqlitePoint() const;
bool    IsVPoint() const;
bool	IsSiemens1200Point() const;
bool	IsSiemens300Point() const;

public:
	DWORD	GetSlaveID() const;
	DWORD	GetHeadAddress() const;
	DWORD	GetFuncCode() const;
	double	GetMultiple() const;
	DWORD	GetValueType() const;
	double	GetValueRange() const;
	DWORD	GetOPCDataSourceType() const;		//0或不存在时候 读缓存  1：启动时候读一次缓存  2:每次读缓存
	wstring	GetOPCServerIP()	const;				//获取OPCServer IP 

	wstring	GetServerAddress() const;
	wstring	GetFCServerPort() const;
	wstring	GetFCServerIP() const;
	DWORD	GetFCPointAddress() const;
	
	enum {
		INDEX_SLAVEID = 1,
		INDEX_HEADADDRESS,
		INDEX_FUNCODE,
		INDEX_MULTIPLE,
		INDEX_SERVERIP,
		INDEX_DATA_TYPE,
		INDEX_DATA_RANGED,
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

	DWORD	GetServerID_Bacnet(int nMode = 0) const;
	DWORD	GetPointType_Bacnet() const;
	DWORD	GetPointAddress_Bacnet() const;

	unsigned int GetBacnetInvokeID() const;
	void	SetBacnetInvokeID(unsigned int invokeid);

	unsigned int GetPointTag_Bacnet() const;
	void SetPointTag_Bacnet(unsigned int tagval);
	unsigned int bacnet_invoke_id;
	unsigned int bacnet_tag_type;

	unsigned int GetOPClientIndex() const;
	void	SetOPClientIndex(unsigned int index);
	unsigned int opc_client_index;
};

