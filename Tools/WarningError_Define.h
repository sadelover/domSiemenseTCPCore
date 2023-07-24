#pragma once

#define WARNING_ID_START	10000
#define WARNING_ID_END		20000

#define CANCELWARNING_MULTIPLE	4

//#define STRING_CONNECT_PLC_FAIL _T("与PLC连接断开，请检查网络连接")

//////////////////////////////////////////////////////////////////////////////////////////////////////
//			以下数值皆是依赖于数据库CS.warniggroup的设定，如果数据库的值改变，以下枚举也需要同步			//
//////////////////////////////////////////////////////////////////////////////////////////////////////
enum Error_System			//系统错误
{
	Error_Interbus		= 10001,
	Error_PLCMode,
	Error_Connect_PLC_Faied,
	Error_DB_opt_failed,
	Error_DB_InstSensor,			//数据库插入传感器错误
	Error_DB_InstCalc,				//数据库插入优化计算错误
	Error_DB_InstOpt,				//数据库插入用户操作错误
	Error_Connect_CH_Failed,
	Error_PLC_Value_Read_Failed,
	Error_PLC_Value_Write_Failed,
};

enum Error_Function			//功能操作错误
{
	Optimize_failed		= 10101,
	Start_System_Failed,
	Stop_System_Failed,
	Standby_Failed,
	Open_CH_Failed,
	Open_CHValve_Failed,
	Open_Tower_Failed,
	Open_CWP_Failed,
	Open_PriChwp_Failed,
	Open_SecChwp_Failed,
	Open_ThrChwp_Failed,
	Shut_CH_Failed,
	Shut_CHValve_Failed,
	Shut_Tower_Failed,
	Shut_CWP_Failed,
	Shut_PriChwp_Failed,
	Shut_SecChwp_Failed,
	Shut_ThrChwp_Failed,
	CoolDown_Failed,
};

enum Error_CH				//冷机报警
{
	Warning_CH_Start		= 10200,
	Warning_CH_1			= 10201,
};

enum Error_CHValve			//冷机阀门故障
{
	Error_CH_Valve_Start	= 10300,
	Error_CH_Valve_1		= 10301,
};

enum Error_Tower			//冷却塔故障
{
	Error_CT_Start			= 10400,
	Error_CT_1 				= 10401,
};

enum Error_TowerFan			//冷却塔风机过流
{
	CT_Fan_Overflow_Start	= 10500,
	CT_1_Fan_Overflow		= 10501,
};

enum Error_Tower_VSD		//冷却塔变频器故障
{
	Error_Tower_VSD_Start	= 20500,
	Error_Tower_1_VSD		= 20501,
};


enum Error_CWP				//冷却水泵故障
{
	Error_CWP_Start			= 10600,
	Error_CWP_1				= 10601,
};

enum Error_CWP_VSD			//冷却水泵变频器故障
{
	Error_CWP_Transducer_Start	= 10700,
	Error_CWP_1_Transducer		= 10701,
};


enum Error_PriChwp			//一次冷冻水泵故障
{
	Error_PriChwp_Start		= 10800,
	Error_PriChwp_1			= 10801,
};

enum Error_PriChwp_VSD		//一次冷冻水泵变频器故障
{
	Error_PriChwp_VSD_Start	= 10900,
	Error_PriChwp_1_VSD		= 10901,
};

enum Error_SecChwp			//二次冷冻水泵故障
{
	Error_SecChwp_Start		= 11000,
	Error_SecChwp_1			= 11001,
};

enum Error_SecChwp_VSD		//二次冷冻水泵变频器故障
{
	Error_SecChwp_VSD_Start	= 11100,
	Error_SecChwp_1_VSD		= 11101,
};

enum Error_ThrChwp			//三次冷冻水泵故障
{
	Error_ThrChwp_Start		= 11200,
	Error_ThrChwp_1			= 11201,
};

enum Error_ThrChwp_VSD		//三次冷冻水泵变频器故障
{
	Error_ThrChwp_VSD_Start	= 11300,
	Error_ThrChwp_1_VSD		= 11301,
};

enum Warning_CH_Surge		//冷冻机喘振
{
	Warning_Surge_CH_Start	= 11400,
	Warning_Surge_CH_1		= 11401,
};

enum AutoErr_CHValve		//冷机阀门模式故障
{
	Error_CHValve_notAuto_Start	= 11500,
	Error_CHValve_1_notAuto		= 11501,
};

enum AutoErr_CT				//冷却塔模式故障
{
	Error_CT_notAuto_Start	= 11600,
	Error_CT_1_notAuto		= 11601,
};

enum AutoErr_CWP			//冷却水泵模式故障
{
	Error_CWP_notAuto_Start	= 11700,
	Error_CWP_1_notAuto		= 11701,
};

enum AutoErr_PriCHWP		//一次泵模式故障
{
	Error_PriCHWP_notAuto_Start	= 11800,
	Error_PriCHWP_1_notAuto		= 11801,
};

enum AutoErr_SecCHWP		//二次泵模式故障
{
	Error_SecCHWP_notAuto_Start	= 11900,
	Error_SecCHWP_1_notAuto		= 11901,
};

enum AutoErr_ThrCHWP		//三次泵模式故障
{
	Error_ThrCHWP_notAuto_Start	= 12000,
	Error_ThrCHWP_1_notAuto		= 12001,
};

enum CT_Valve_Error
{
	Error_CTValve_Error_Start = 12100,
	Error_CTValve_1_Error = 12101,
};

enum AutoErr_CTValve		//冷机阀门模式故障
{
	Error_CTValve_notAuto_Start	= 12200,
	Error_CTValve_1_notAuto		= 12201,
};


enum NetWorkConnectionError
{
	Error_NetWorkError_Start = 13000,
	Error_NetWorkError_Disconnected,
};

enum Pump_Pressure_Error
{
	Error_PumpPressure_Start	= 14000,
	Error_PumpPressure_1_InValid	= 14001,
};

#define  NETWORKERRORID  (WARNING_ID_START + 10)
