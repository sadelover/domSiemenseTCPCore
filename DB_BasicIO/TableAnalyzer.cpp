#include "stdafx.h"
#include "TableAnalyzer.h"



void CTableAnalyzer::AnalyzeTableQueryEqualYear( std::vector<queryentry>& resultlist,
												 const SYSTEMTIME& start,
												 const SYSTEMTIME& end )
{
	

	int startyear = start.wYear;
	int startmonth = start.wMonth;

	int endyear = end.wYear;
	int endmonth = end.wMonth;


	if (startyear == endyear)
	{
		//如果在同一年
		if (startmonth == endmonth){
			//如果在同一月不需要跨表
			queryentry entry;
			char buf[32] = {0};
			sprintf_s(buf, "%s_%d_%02d", m_basetablename.c_str(), startyear, startmonth);
			entry.tablename = buf;
			entry.st_start = start;
			entry.st_end = end;

			resultlist.push_back(entry);
			return;
		}
		else if(endmonth > startmonth)
		{
			//在同一年里跨月
			int monthdiff = endmonth - startmonth;
			if (monthdiff >= 1 )
			{
				queryentry entry;
				char buf[32] = {0};
				sprintf_s(buf, "%s_%d_%02d", m_basetablename.c_str(), startyear, startmonth);
				entry.tablename = buf;
				entry.st_start = start;
				SYSTEMTIME newend = start;
				newend.wMonth++;
				newend.wDay = 1;
				newend.wHour = 0;
				newend.wMinute = 0;
				newend.wSecond = 0;
				newend.wMilliseconds = 0;
				entry.st_end = newend;

				resultlist.push_back(entry);

				if (monthdiff >= 2)
				{
					for (int i = 1; i < monthdiff; i++)
					{
						queryentry entry;
						char buf[32] = {0};
						sprintf_s(buf, "%s_%d_%02d", m_basetablename.c_str(), startyear, startmonth+i);
						entry.tablename = buf;
						
						SYSTEMTIME newstart = start;
						newstart.wMonth = startmonth+i;
						newstart.wDay = 1;
						newstart.wHour = 0;
						newstart.wMinute = 0;
						newstart.wSecond = 0;
						newstart.wMilliseconds = 0;
						entry.st_start = newstart;
						
						SYSTEMTIME newend = newstart;
						newend.wMonth++;
						newend.wDay = 1;
						newend.wHour = 0;
						newend.wMinute = 0;
						newend.wSecond = 0;
						newend.wMilliseconds = 0;
						entry.st_end = newend;

						resultlist.push_back(entry);
					}
				}


				//handle the end part
				if (end.wDay == 1 &&end.wHour == 0 && end.wMinute == 0){
					return;
				}
				
				sprintf_s(buf, "%s_%d_%02d", m_basetablename.c_str(), startyear, endmonth);
				entry.tablename = buf;
				
				SYSTEMTIME newstart = end;
				newstart.wDay = 1;
				newstart.wHour = 0;
				newstart.wMinute = 0;
				newstart.wSecond = 0;
				newstart.wMilliseconds = 0;
				entry.st_start = newstart;
				entry.st_end = end;
				resultlist.push_back(entry);
			}

		}
	}else
	{
		//不在同一年
		return ;
	}

}

void CTableAnalyzer::AnalyzeTableQuery( std::vector<queryentry>& resultlist,
									   const SYSTEMTIME& start,
									   const SYSTEMTIME& end)
{
	if (m_basetablename.empty()){
		return;
	}

	int startyear = start.wYear;
	int endyear = end.wYear;

	if (startyear == endyear){
		return AnalyzeTableQueryEqualYear(resultlist, start, end);
	}else
	{
		int yeardiff = endyear - startyear;
		if (yeardiff >= 1){
			SYSTEMTIME st_newend = start;
			st_newend.wMonth = 12;
			st_newend.wDay = 30;
			st_newend.wHour = 23;
			st_newend.wMinute = 59;
			st_newend.wSecond = 59;
			st_newend.wMilliseconds  = 0;
			AnalyzeTableQueryEqualYear(resultlist, start, st_newend);

			if (yeardiff >= 2){
				for (int i = 1; i < yeardiff; i++){
					SYSTEMTIME st1 = start;
					st1.wYear = start.wYear + i;
					st1.wMonth = 1;
					st1.wDay = 1;
					st1.wHour = 0;
					st1.wMinute = 0;
					st1.wSecond = 0;
					st1.wMilliseconds = 0;

					SYSTEMTIME st2 = st1;
					st2.wMonth = 12;
					st2.wDay = 30;
					st2.wHour = 23;
					st2.wMinute = 59;
					st2.wSecond = 59;
					st2.wMilliseconds = 0;

					AnalyzeTableQueryEqualYear(resultlist, st1, st2);
				}
			}

			if (end.wMonth == 1 && end.wDay == 1 && end.wHour == 0 && end.wMinute == 0){
				return;
			}

			SYSTEMTIME st3;
			st3.wYear = endyear;
			st3.wMonth =1;
			st3.wDay = 1;
			st3.wHour = 0;
			st3.wMinute = 0;
			st3.wSecond = 0;
			st3.wMilliseconds = 0;
			AnalyzeTableQueryEqualYear(resultlist, st3, end);
		}
	}
}

void CTableAnalyzer::AnalyzeTableQuery_v1( std::vector<queryentry>& resultlist,
										  const COleDateTime& start,
										  const COleDateTime& end )
{
	if (m_basetablename.empty()){
		return;
	}

	int nMonths = GetSpanTimeMonths(start, end);
	COleDateTime mDate;
	COleDateTime mStart = start;
	COleDateTime mEnd = end;
	
	if(mStart.GetMonth()==12){
		mDate.SetDate(mStart.GetYear()+1,1,1);	
	}
	else{
		mDate.SetDate(mStart.GetYear(),mStart.GetMonth()+1,1);	
	}
	
	resultlist.clear();
	if(nMonths==0)
	{
		queryentry entry;
		GenerateEntry(mStart, mEnd, entry);
		resultlist.push_back(entry);
	}
	else if(nMonths > 0)
	{
		for(int i = 0; i < nMonths; i++){
			queryentry entry;
			GenerateEntry(mStart, mEnd, entry);
			resultlist.push_back(entry);
			
			mStart = mDate;
			int nYear = mDate.GetYear();
			int nMonth = mDate.GetMonth()+1;
			if(nMonth > 12){
				nYear = nYear+1;
				nMonth = nMonth -12;
			}

			mDate.SetDate(nYear,nMonth,1);
		}

		queryentry entry;
		GenerateEntry(mStart, mEnd, entry);
		resultlist.push_back(entry);
	}

	return;
}

int CTableAnalyzer::GetSpanTimeMonths( const COleDateTime& start, const COleDateTime& end )
{
	return (end.GetYear()-start.GetYear())*12 + (end.GetMonth()-start.GetMonth());
}

void CTableAnalyzer::GenerateEntry( const COleDateTime& start, const COleDateTime& end, queryentry& entry)
{
	
	char buf[20] = {0};
	sprintf_s(buf, "%s_%d_%02d", m_basetablename.c_str(), start.GetYear(), start.GetMonth());
	entry.tablename = buf;
	start.GetAsSystemTime(entry.st_start);
	end.GetAsSystemTime(entry.st_end);
}

CTableAnalyzer::CTableAnalyzer( const std::string& basetablename )
	:m_basetablename(basetablename)
{

}

CTableAnalyzer::~CTableAnalyzer()
{

}


