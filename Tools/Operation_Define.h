#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////
//			以下数值皆是依赖于数据库CS.optgroup的设定，如果数据库的值改变，以下枚举也需要同步			//
//////////////////////////////////////////////////////////////////////////////////////////////////////
enum Operations
{
	Opt_StartSystem		= 1,	//自动开机
	Opt_StopSystem,				//自动关机
	Opt_System_Standby_Auto,	//自动待机
	Opt_System_Standby_manu,	//手动待机
	Opt_Optimize,				//优化计算
	Opt_Optimize_Cwp_Twb_Low,	//湿球温度低时的冷却水泵增减
	Opt_Switch_Auto,			//切换至自动
	Opt_Switch_Manu,			//切换至手动
	Opt_CoolDown,				//预冷
	Opt_AppStart,				//程序已经启动
};