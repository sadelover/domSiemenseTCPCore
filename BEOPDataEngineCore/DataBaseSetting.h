#pragma once


#include "XSTRING"

using namespace std;

class  DataBaseSetting
{
public:
	DataBaseSetting(void);
	~DataBaseSetting(void);



	// 数据库信息参数结构体
		string strDBIP;					//  host地址
		string strDBIP2;				//  备用IP
		string strDBName;				//  数据库名字
		string strRealTimeDBName;       //  实时数据库名字
		string strUserName;				//  用户名
		string strDBPsw;				//  数据库密码
		int    nAutoLogin;				//	是否自动登录
		int    DTUPortNumber;			//	DTU端口号
		int    DTUBaudrate;				//	DTU波特率
		int    nPort;					//  端口


};

