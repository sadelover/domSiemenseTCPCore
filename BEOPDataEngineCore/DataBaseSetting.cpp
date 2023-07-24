#include "StdAfx.h"
#include "DataBaseSetting.h"


DataBaseSetting::DataBaseSetting(void)
{
	strDBPsw = "";
	nPort = 3306;
	strDBIP = "localhost";

	strRealTimeDBName = "beopdata";
	strUserName = "root";

}


DataBaseSetting::~DataBaseSetting(void)
{
}
