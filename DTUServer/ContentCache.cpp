#include "StdAfx.h"
#include "ContentCache.h"


CContentCache::CContentCache(void)
{
}


CContentCache::~CContentCache(void)
{
}

void CContentCache::AddString( int keyval, const wstring& buffer )
{
	m_buffercache[keyval] = buffer;
}

wstring CContentCache::FindString( int keyval )
{
	wstring result;
	map<int, wstring>::const_iterator it = m_buffercache.find(keyval);
	if (it != m_buffercache.end()){
		result = it->second;
	}

	return result;
}

void CContentCache::Clear()
{
	m_buffercache.clear();
}
