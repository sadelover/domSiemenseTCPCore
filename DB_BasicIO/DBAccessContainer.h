
// -*- c++ -*-
// ***********************************************************
// Copyright (C) 2013 R&B
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Module:   
// Author:   david
// Date:     2010/7/13
// Revision: 1.0
//
// ***********************************************************
#ifndef DBACCESSCONTAINER_H
#define DBACCESSCONTAINER_H

#pragma once
#include <map>
#include <vector>
#include <utility>
#include <string>

#include "DBInterfaceImp.h"
#include "DatabaseSession.h"

#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/NormalDefines.h"

#include "RunningRecordOperation.h" //bird add

using std::string;
using std::wstring;
using std::map;
using std::multimap;
using std::vector;
using std::pair;
using Project::Tools::Mutex;
using Project::Tools::Scoped_Lock;
;
#define TEMPLATEDATABASE_CREATE "Beop_template";

typedef struct tag_Project_DB
{
	wstring		strprojectname;		//	项目名字,需要在界面上显示
	string		strdatabasename;	//	数据库名字
}Project_DB;

//工程相关数据的信息
typedef struct tag_StartUpInfo
{
	string		bpicture;			//	项目图片
	Project_DB  projectdb;		//	数据库名和项目名
}StartUpInfo;

typedef struct tag_BeopUserInfo
{
	wstring		username;
	int			readright;
	int			writeright;
	int			modelright;
	int			simulateright;
	wstring		lastlogintime;
}BeopUserInfo;

class DBDLL LoadRecord
{
public:
	SYSTEMTIME recordtime;
	double	   loadvalue;
};


//数据库访问类
class DBDLL DBAccessContainer : public CDatabaseSeesion
{
private:
	DBAccessContainer();
public:
	~DBAccessContainer();
	static DBAccessContainer *GetInstance();
	static void DestroyInstance();

public:


	// 报警记录
	bool		InsertWarningRecord(int warningindex);	// 插入警报记录
	bool		InsertWarningRecord(SYSTEMTIME st, const wstring& warninginfo);	
	// 用户操作记录
	bool		InsertOperationRecord(const SYSTEMTIME& st, const wstring& remark, const wstring& wstrUser);
	
	//Beop_USER.users表读写接口
	bool	DeleteUser(wstring username);			//删除一个user
	bool	ReadAllUsers(vector<wstring>& userlist);	//读所有的user列表
	bool	ReadUserInfo(wstring username, BeopUserInfo* puserinfo);	// 读user的信息
	bool	ReadALLUserInfo(vector<BeopUserInfo*>& userlist);
	bool	ValidateUser(wstring username, wstring password); // 判断用户是否存在
	
    bool    ReadAllOperationRecord(VEC_OP_RECORD&  vecVecOperation, const COleDateTime& timeStart, const COleDateTime& timeStop);
	
	bool    ReadWarningRecord(WARNING_LIST& warninglist, const COleDateTime& timebegin, const COleDateTime& timeend);
    //bird add, read power consumption total

	vector<wstring> ReadWarningConfigList();
	
	void	LogDBError(const char* sql);
	void	LogDBError(const string& sql);
	
private:

	static Mutex               m_mutex;

    Mutex       m_recordmutex;
	static DBAccessContainer * g_DBAccessContainer;
};
#endif