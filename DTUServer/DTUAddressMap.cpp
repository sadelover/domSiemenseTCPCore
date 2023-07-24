#include "StdAfx.h"

#include "DTUAddressMap.h"
#include "PackageTypeDefine.h"

DTUAddressMap* DTUAddressMap::m_pinstance = NULL;

DTUAddressMap::DTUAddressMap(void)
{
	
}

DTUAddressMap::~DTUAddressMap(void)
{
}

vector<mapentry> DTUAddressMap::GetEntryList()
{
	return m_entrylist;
}

string DTUAddressMap::GetDBByDTU( const string& dtuname )
{
	for (unsigned int i = 0; i < m_entrylist.size();i++)
	{
		mapentry& entry = m_entrylist[i];
		if (entry.dtuname == dtuname){
			return entry.databasename;
		}
	}

	return "";
}

string DTUAddressMap::GetRemarkByDTU( const string& dtuname )
{
	for (unsigned int i = 0; i < m_entrylist.size();i++)
	{
		mapentry& entry = m_entrylist[i];
		if (entry.dtuname == dtuname){
			return entry.remark;
		}
	}
	return "";
}

DTUAddressMap* DTUAddressMap::GetInstance()
{
	if (m_pinstance == NULL)
	{
		m_pinstance = new DTUAddressMap();
		atexit(DestroyInstance);
	}

	return m_pinstance;
}

void DTUAddressMap::DestroyInstance()
{
	if (m_pinstance){
		delete m_pinstance;
	}
}

bool DTUAddressMap::SetConfigList( vector<mapentry> entrylist )
{
	m_entrylist = entrylist;
	return true;
}

mapentry DTUAddressMap::GetDTUInfoByName( const string& dtuname )
{
	mapentry entry;
	for (unsigned int i = 0; i < m_entrylist.size();i++)
	{
		entry = m_entrylist[i];
		if (entry.dtuname == dtuname){
			return entry;
		}
	}
	return entry;
}

void DTUAddressMap::UpdateDTUEntryList( const char* dtuname, const char* buffer )
{
	mapentry entry = GetDTUInfoByName(dtuname);
	if(entry.databasename == "")
		return;

	char* newbuffer = _strdup(buffer);

	weldtech::CPackageType::RemovePrefix(newbuffer);

	string strtime;
	char* p = strtok(newbuffer, ";");
	if (p){
		strtime = p;
	}

	hash_map<string,spointwatch> m_mapPoint = entry.m_mapPoint;

	p = strtok(NULL, ";");
	while(p){
		char name[256], value[256];
		memset(name, 0, 256);
		memset(value, 0, 256);

		int i = 0;
		char* q = p;
		while(*q++){
			i++;
			if((*q) == ','){
				break;
			}
		}
		strcpy(value, q+1);
		*q = '\0';
		strcpy(name, p);

		SetPointValueByName(dtuname,name,strtime,value);
		p = strtok(NULL, ";");
	}
}

hash_map<string,spointwatch> DTUAddressMap::GetPointListByDTUName( const string& dtuname )
{
	mapentry entry;
	for (unsigned int i = 0; i < m_entrylist.size();i++)
	{
		entry = m_entrylist[i];
		if (entry.dtuname == dtuname){
			return entry.m_mapPoint;
		}
	}
	return entry.m_mapPoint;
}

void DTUAddressMap::UpdatePoint( const string& dtuname,const string& pointname,const string& pointvalue )
{
	for (unsigned int i = 0; i < m_entrylist.size();i++)
	{
		if (m_entrylist[i].dtuname == dtuname){
			m_entrylist[i].m_mapPoint[pointname].strvalue = pointvalue;
		}
	}
}

void DTUAddressMap::SetPointValueByName( const string dtuname, const string name, const string time, const string value )
{
	for (unsigned int i = 0; i < m_entrylist.size();i++)
	{
		if (m_entrylist[i].dtuname == dtuname){
			m_entrylist[i].m_mapPoint[name].strtime = time;
			m_entrylist[i].m_mapPoint[name].strvalue = value;
		}
	}
}
