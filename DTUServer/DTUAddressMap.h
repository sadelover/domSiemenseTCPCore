#pragma once

#include <vector>
#include <string>
#include <hash_map>
#include "../Tools/CustomTools/CustomTools.h"

using std::string;
using std::vector;
using std::wstring;

using namespace std;

enum  POINT_STORE_CYCLE
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
};

struct spointwatch
{
	spointwatch()
	{
		bUpdate = false;
	}
	bool	bUpdate;		//标记已更新到数据库
	string strpointname;
	string strvalue;
	string strremark;
	string strtime;
	POINT_STORE_CYCLE	   nStore;	
};

struct warning
{
	warning()
	{
		bUpdate = false;
	}
	bool	bUpdate;		//标记已更新到数据库
	string strHappenTime;
	int	   nLevel;			//
	string strTime;
	string strInfo;
	string strPointName;
	int		nConfirmed;
};

struct warningoperation
{
	warningoperation()
	{
		bUpdate = false;
	}
	bool	bUpdate;		//标记已更新到数据库
	string strTime;
	string strInfo;
	string strPointName;
	string strUser;
	string strOperation;
};

struct mapentry
{
	int	   dtuid;
	string dtuname;
	string remark;
	string ip;
	string user;
	string psw;
	string databasename;
	string configpointtable;
	hash_map<string,spointwatch> m_mapPoint;
	bool	bSendData;

	void Reset() {dtuname.clear(); databasename.clear();remark.clear();m_mapPoint.clear();}
};

/*
 **
 **
 **this class will parse the dtu address map.
 **each dtu is identifiled by an id, and is binded to a database releated with the project.
 **
 **/
class DTUAddressMap
{
public:
	DTUAddressMap(void);
	~DTUAddressMap(void);

	static DTUAddressMap* GetInstance();
	static void DestroyInstance();

	bool SetConfigList(vector<mapentry> entrylist);

	vector<mapentry>  GetEntryList();

	string GetDBByDTU(const string& dtuname);

	mapentry GetDTUInfoByName(const string& dtuname);

	string GetRemarkByDTU(const string& dtuname);

	void	UpdateDTUEntryList(const char* dtuname, const char* buffer);

	hash_map<string,spointwatch> GetPointListByDTUName(const string& dtuname);

	void	UpdatePoint(const string& dtuname,const string& pointname,const string& pointvalue);

	void	SetPointValueByName(const string dtuname, const string name, const string time, const string value);
private:
	vector<mapentry>  m_entrylist;
	static DTUAddressMap* m_pinstance;
};

