#pragma once

#include <string>
#include <vector>
#include "windows.h"

#include "atlcomtime.h"

class CTableAnalyzer
{
public:
	CTableAnalyzer(const std::string& basetablename);
	~CTableAnalyzer();

public:
	struct queryentry
	{
		std::string tablename;
		SYSTEMTIME st_start;
		SYSTEMTIME st_end;
	};

public:

	 void AnalyzeTableQueryEqualYear(std::vector<queryentry>& resultlist, const SYSTEMTIME& start, const SYSTEMTIME& end);

	 void AnalyzeTableQuery(std::vector<queryentry>& resultlist, const SYSTEMTIME& start, const SYSTEMTIME& end);

	 int GetSpanTimeMonths(const COleDateTime& start, const COleDateTime& end );

	 void	AnalyzeTableQuery_v1(std::vector<queryentry>& resultlist,
									 const COleDateTime& start,
									 const COleDateTime& end);

		void GenerateEntry( const COleDateTime& start, const COleDateTime& end, queryentry& entry);

private:
	std::string		m_basetablename;

};

