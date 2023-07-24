

/*
 *
 *
 *这个类存储最近接受的一些数据。用于调试。
 *
 *
 */
#pragma once


#include <map>
#include <string>

using std::map;
using std::wstring;


class CContentCache
{
public:
	CContentCache(void);
	~CContentCache(void);

	//add one entry
	void	AddString(int keyval, const wstring& buffer);

	//seach string by 
	wstring	FindString(int keyval);

	//clear all
	void	Clear();

private:
	
	map<int, wstring>  m_buffercache;
};

