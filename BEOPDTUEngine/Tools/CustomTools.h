#pragma once

#include"NormalDefines.h"

#include <vector>
#include <list>
#include <assert.h>
#include <string>
#include "comdef.h"
#include "StrSafe.h"
#include "ATLComTime.h"
#include <direct.h>
#include "StringDef.h"
#include "tchar.h"

#include "math.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <TlHelp32.h>

using std::vector;
using std::string;
using std::wstring;
using std::list;
using namespace std; 
using namespace Project::Tools;

namespace Project
{
	namespace Tools
	{

		
		//获取当前路径
		inline void GetAppPath(tstring &strPath)
		{
			TCHAR aszTmp[MAX_PATH] = {0};
			::GetCurrentDirectory(MAX_PATH, aszTmp);
			strPath = aszTmp;
            //strPath += _T("\\");
		}

		inline void GetSysPath(wstring &strSysPath)
		{
			wchar_t szExePath[MAX_PATH] = {0};
			::GetModuleFileName(NULL, szExePath, MAX_PATH);

			strSysPath = szExePath;
			int nFindIndex = strSysPath.find_last_of('\\');
			strSysPath = strSysPath.substr(0,nFindIndex);
		}

		/*
		Time tools
		*/
		//////////////////////////////////////////////////////////////////////////Basic translate
		//SysTime_To_String
		inline void SysTime_To_String( const SYSTEMTIME & systime, tstring & strTime )
		{
			TCHAR aszTmp[256]	= {0};
			StringCchPrintf(_tcschr( aszTmp, 0 ), 256, _T("%4d-%02d-%02d %02d:%02d:%02d")
				,systime.wYear, systime.wMonth, systime.wDay
				,systime.wHour, systime.wMinute,systime.wSecond);

			strTime = aszTmp;
		}
		//SysTime_To_OleTime
		inline void SysTime_To_OleTime( SYSTEMTIME systime, COleDateTime & OleTime )
		{
			OleTime.GetAsSystemTime( systime );
		}
		//OleTime_To_String
		inline void OleTime_To_String( const COleDateTime &	OleTime, tstring & strTime )
		{
			SYSTEMTIME		sysTime;
			OleTime.GetAsSystemTime( sysTime );
			SysTime_To_String( sysTime, strTime );
		}
		//String_To_SysTime
		inline void String_To_SysTime( const tstring & strTime, SYSTEMTIME & systime )
		{
			COleDateTime	OleTime;
			OleTime.ParseDateTime( strTime.c_str(), 0, LANG_USER_DEFAULT );
			OleTime.GetAsSystemTime( systime );
		}

		
		//String_To_OleTime
		inline void String_To_OleTime( const tstring & strTime, COleDateTime & OleTime )
		{
			OleTime.ParseDateTime( strTime.c_str(), 0, LANG_USER_DEFAULT );
		}
		//////////////////////////////////////////////////////////////////////////Basic translate end
		//获取当前时间
		inline void	GetDateTime( SYSTEMTIME & systime )
		{
			GetLocalTime( &systime );
		}
		//当前日期的字符串
		inline void	GetDate( tstring & strData )
		{
			SYSTEMTIME	systime;
			TCHAR		aszTmp[256]	= {0};

			//GetSystemTime( &systime );
			GetLocalTime( &systime );
			StringCchPrintf(_tcschr( aszTmp, 0 ), 256, _T("%4d-%02d-%02d"),
							systime.wYear, systime.wMonth, systime.wDay);

			strData = aszTmp;
		}
		//当前时间的字符串
		inline void GetDateTime( tstring & strTime )
		{
			SYSTEMTIME	systime;
			TCHAR		aszTmp[256]	= {0};

			GetLocalTime( &systime );
			StringCchPrintf(_tcschr( aszTmp, 0 ), 256, _T("%4d-%02d-%02d %02d:%02d:%02d"),
							systime.wYear, systime.wMonth, systime.wDay,
							systime.wHour, systime.wMinute,systime.wSecond);

			strTime = aszTmp;
		}
		//今天凌晨的时间SYSTEMTIME
		inline void GetTodayMidnight( SYSTEMTIME & systime )	//今天凌晨00：00：00
		{
 			GetLocalTime( &systime );
			systime.wHour			= 0;
			systime.wMinute			= 0;
			systime.wSecond			= 0;
			systime.wMilliseconds	= 0;
		}
		//今天凌晨的时间字符串
		inline void GetTodayMidnight( tstring & strTime )	//今天凌晨00：00：00
		{
			SYSTEMTIME	systime;
			GetTodayMidnight( systime );
			SysTime_To_String( systime, strTime );
		}
		//一小时的始末
		inline void Get_OneHour_Start_End( const COleDateTime & OleTime, COleDateTime & OleStart, COleDateTime & OleEnd )
		{
			OleStart.SetDateTime( OleTime.GetYear(), OleTime.GetMonth(), OleTime.GetDay(), OleTime.GetHour(), 0, 0 );
			//时间差
			COleDateTimeSpan	OleDateTimeSpan( 0, 1, 0, 0 );
			OleEnd = OleStart + OleDateTimeSpan;
		}
		inline void Get_OneHour_Start_End( const tstring & strTime, tstring & strStart, tstring & strEnd )
		{
			COleDateTime	OleTime;
			COleDateTime	OleStart;
			COleDateTime	OleEnd;
			String_To_OleTime( strTime, OleTime );
			Get_OneHour_Start_End( OleTime, OleStart, OleEnd );
			OleTime_To_String( OleStart, strStart );
			OleTime_To_String( OleEnd, strEnd );
		}
		//一天的始末
		inline void Get_OneDay_Start_End( const COleDateTime & OleTime, COleDateTime & OleStart, COleDateTime & OleEnd )
		{
			OleStart.SetDateTime( OleTime.GetYear(), OleTime.GetMonth(), OleTime.GetDay(), 0, 0, 0 );
			//时间差
			COleDateTimeSpan	OleDateTimeSpan( 1, 0, 0, 0 );
			OleEnd = OleStart + OleDateTimeSpan;
		}
		inline void Get_OneDay_Start_End( const tstring & strTime, tstring & strStart, tstring & strEnd )
		{
			COleDateTime	OleTime;
			COleDateTime	OleStart;
			COleDateTime	OleEnd;
			String_To_OleTime( strTime, OleTime );
			Get_OneDay_Start_End( OleTime, OleStart, OleEnd );
			OleTime_To_String( OleStart, strStart );
			OleTime_To_String( OleEnd, strEnd );
		}
		//一年的始末
		inline void Get_OneYear_Start_End( const COleDateTime & OleTime, COleDateTime & OleStart, COleDateTime & OleEnd )
		{
			OleStart.SetDateTime( OleTime.GetYear(), 1, 1, 0, 0, 0 );
			OleEnd.SetDateTime( OleTime.GetYear() + 1, 1, 1, 0, 0, 0 );
		}
		inline void Get_OneYear_Start_End( const tstring & strTime, tstring & strStart, tstring & strEnd )
		{
			COleDateTime	OleTime;
			COleDateTime	OleTime_Start;
			COleDateTime	OleTime_End;
			
			String_To_OleTime( strTime, OleTime );
			Get_OneYear_Start_End( OleTime, OleTime_Start, OleTime_End );
			OleTime_To_String( OleTime_Start, strStart );
			OleTime_To_String( OleTime_End, strEnd );
		}
		//一月的始末
		inline void Get_OneMonth_Start_End( const COleDateTime & OleTime, COleDateTime & OleStart, COleDateTime & OleEnd )
		{
			OleStart.SetDateTime( OleTime.GetYear(), OleTime.GetMonth(), 1, 0, 0, 0 );
			OleEnd.SetDateTime( OleTime.GetYear() + (unsigned int)( OleTime.GetMonth() / 12 ),
									( ( OleTime.GetMonth() + 1 + OleTime.GetMonth() / 13 ) ) % 13, 1, 0, 0, 0 );
		}
		inline void Get_OneMonth_Start_End( const tstring & strTime, tstring & strStart, tstring & strEnd )
		{
			COleDateTime	OleTime;
			COleDateTime	OleTime_Start;
			COleDateTime	OleTime_End;
			//strDay的时间转化为SYSTEMTIME
			String_To_OleTime( strTime, OleTime );
			Get_OneMonth_Start_End( OleTime, OleTime_Start, OleTime_End );
			OleTime_To_String( OleTime_Start, strStart );
			OleTime_To_String( OleTime_End, strEnd );
		}
		inline unsigned int Get_DaysNum_InMonth( const COleDateTime & OleTime )
		{
			COleDateTime	OleTime_Start;
			COleDateTime	OleTime_End;
			Get_OneMonth_Start_End( OleTime, OleTime_Start, OleTime_End );
			COleDateTimeSpan	OleDateTimeSpan( 1, 0, 0, 0 );
			OleTime_End = OleTime_End - OleDateTimeSpan;
			return OleTime_End.GetDay();
		}
		inline unsigned int Get_DaysNum_InMonth( const tstring & strTime )
		{
			COleDateTime	OleTime;
			String_To_OleTime( strTime, OleTime );
			return Get_DaysNum_InMonth( OleTime );
		}
		

		/*
		return:	< 0 ------> time1 < time2
				= 0 ------> time1 = time2
				< 0 ------> time1 > time2
		*/
		inline int	Compare_Date( const SYSTEMTIME & time1, const SYSTEMTIME & time2 )
		{
			int ushYearDispersion	= time1.wYear	- time2.wYear;
			if ( ushYearDispersion )											//year不同
			{
				return ushYearDispersion;
			}

			int ushMonthDispersion	= time1.wMonth	- time2.wMonth;
			if ( ushMonthDispersion )											//year相同但month不同
			{
				return ushMonthDispersion;
			}

			return ( time1.wDay	- time2.wDay );									//year、month相同判断day
		}
		
		//删除vector的指针元素
		template <typename T>
		void  ClearVector(vector<T*>& vec)
		{
			if (vec.size() == 0)
				return;

			for ( vector<T*>::iterator it = vec.begin(); it != vec.end(); it++)	
			{	
				if (*it)
					delete (*it);
			}	

			vec.clear();
		}
	
		template <typename T>
		void  ClearList(list<T*>& vec)
		{
			if (vec.size() == 0)
				return;

			for ( list<T*>::iterator it = vec.begin(); it != vec.end(); it++)	
			{	
				if (*it)
					delete (*it);
			}	

			vec.clear();
		}

		inline void
		String_To_Tstring( const char * pStr, tstring & str )
		{
			if(pStr==NULL)
				return;

			_bstr_t	bstr;
			bstr = pStr;
			str	 = bstr;
		}
		
		// Ansi To UTF16, please release the buffer returned by this function.
		inline wstring AnsiToWideChar(const char* source)
		{
			assert (source != NULL);
			if(source==NULL)
				return L"";
			int wlen = MultiByteToWideChar(CP_ACP, 0, source, (int)strlen(source), NULL, 0);
			wchar_t* buffer = new wchar_t[wlen + 1];
			int rflag = MultiByteToWideChar(CP_ACP, 0, source, (int)strlen(source), buffer, wlen);
			buffer[wlen] = L'\0';

            wstring result = buffer;
            delete[] buffer;

			return result;
		}

		//remove decimal zero
		inline string RemoveDecimalA(double value)
		{
			long tmp,k=1000000;
			std::ostringstream out;
			tmp = value * k;
			out << tmp / k;
			tmp %= k;

			if(tmp)
			{
				out << '.';
				while(tmp%10==0)
					tmp /= 10;
				out << tmp;
			}
			return out.str();
		}

		//remove decimal zero
		/*inline wstring RemoveDecimalW(double value)
		{
		long tmp,k=1000000;
		std::ostringstream out;
		tmp = value * k;
		out << tmp / k;
		tmp %= k;

		if(tmp)
		{
		out << '.';
		while(tmp%10==0)
		tmp /= 10;
		out << tmp;
		}
		return AnsiToWideChar(out.str().c_str());
		}*/

		inline wstring RemoveDecimalW(double value,int decprec=2)
		{
			//char buff[256] = {0};
			////sprintf(buff,"%f",value);
			//sprintf_s(buff,sizeof(buff),"%f",value);

			std::ostringstream out_;
			out_.precision(12);
			out_ << value;
			string strValue = out_.str();
			std::size_t sPos = strValue.find('.');
			int nPresion = 6;					//默认有效数据位
			if(sPos != std::string::npos)		//找到小数点
			{
				nPresion = sPos+decprec;
			}
			std::ostringstream out;
			out.precision(nPresion);
			out << value;
			return AnsiToWideChar(out.str().c_str());
		}

		// UTF16 To Ansi, please release the buffer returned by this function.
		inline string WideCharToAnsi(const wchar_t* source)
		{
			assert(source != NULL);
			if(source==NULL)
				return "";
			int anslen  = WideCharToMultiByte(CP_ACP, 0, source, (int)wcslen(source), NULL, 0, NULL, NULL);
			char* buffer = new char[anslen + 1];
			int retflags = WideCharToMultiByte(CP_ACP, 0, source, (int)wcslen(source), buffer, anslen, NULL, NULL);
			buffer[anslen] = '\0';

            string result = buffer;
            delete[] buffer;
			return result;
		}

		// UTF16 To UTF8, please release the buffer returned by this function.
		inline string WideCharToUtf8(const wchar_t* source)
		{
			assert (source != NULL);
			if(source==NULL)
				return "";
			//预转换，得到所需空间的大小，这次用的函数和上面名字相反 
			int u8Len = ::WideCharToMultiByte(CP_UTF8, NULL, source, (int)wcslen(source), NULL, 0, NULL, NULL); 
			//同上，分配空间要给'\0'留个空间 
			//UTF8虽然是Unicode的压缩形式，但也是多字节字符串，所以可以以char的形式保存 
			char* szU8 = new char[u8Len + 1]; 
			//转换 
			//unicode版对应的strlen是wcslen 
			::WideCharToMultiByte(CP_UTF8, NULL, source, (int)wcslen(source), szU8, u8Len, NULL, NULL); 
			//最后加上'\0' 
			szU8[u8Len] = '\0'; 
            
            string result = szU8;
            delete[] szU8;
			return result;
		}

		// UTF16 To UTF8, please release the buffer returned by this function.
		inline void WideCharToUtf8(const wstring& source, string& result)
		{
			if (0==source.size())
			{
				return;
			}
			//预转换，得到所需空间的大小，这次用的函数和上面名字相反 
			int u8Len = ::WideCharToMultiByte(CP_UTF8, NULL, source.c_str(), (int)source.length(), NULL, 0, NULL, NULL); 
			//同上，分配空间要给'\0'留个空间 
			//UTF8虽然是Unicode的压缩形式，但也是多字节字符串，所以可以以char的形式保存 
			char* szU8 = new char[u8Len + 1]; 
			//转换 
			//unicode版对应的strlen是wcslen 
			::WideCharToMultiByte(CP_UTF8, NULL, source.c_str(), (int)source.length(), szU8, u8Len, NULL, NULL); 
			//最后加上'\0' 
			szU8[u8Len] = '\0'; 

			result = szU8;
			delete[] szU8;			
		}
		
		// UTF8 To UTF16, please release the buffer returned by this function.
		inline wstring UTF8ToWideChar(const char* source)
		{
			
			if(source==NULL)
				return L"";
			//预转换，得到所需空间的大小 
			int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, source, (int)strlen(source), NULL, 0); 
			//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间 
			wchar_t* wszString = new wchar_t[wcsLen + 1]; 
			//转换 
			::MultiByteToWideChar(CP_UTF8, NULL, source, (int)strlen(source), wszString, wcsLen); 
			//最后加上'\0' 
			wszString[wcsLen] = L'\0'; 

            wstring result = wszString;
            delete[] wszString;

			return result;
		}

		// UTF8 To UTF16, please release the buffer returned by this function.
		inline void UTF8ToWideChar(const string& source, wstring& result)
		{
			if (0==source.size())
			{
				return;
			}
			//预转换，得到所需空间的大小 
			int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, source.c_str(), (int)source.length(), NULL, 0); 
			//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间 
			wchar_t* wszString = new wchar_t[wcsLen + 1]; 
			//转换 
			::MultiByteToWideChar(CP_UTF8, NULL, source.c_str(), (int)source.length(), wszString, wcsLen); 
			//最后加上'\0' 
			wszString[wcsLen] = L'\0';
			result = wszString;
			
			delete[] wszString;			
		}

		// ANSI TO UTF8, please release the buffer returned by this function.
		inline string MultiByteToUtf8(const char* source)
		{
			if(source==NULL)
				return "";
			wstring temp = AnsiToWideChar(source);
			string result = WideCharToUtf8(temp.c_str());
			return result;
		}

		// UTF8 To Ansi, please release the buffer returned by this function.
		inline string Utf8ToMultiByte(const char* source)
		{
			if(source==NULL)
				return "";
			wstring temp = UTF8ToWideChar(source);
			string result = WideCharToAnsi(temp.c_str());
			return result;
		}

		inline void UnicodeToGB2312(char* pOut,wchar_t uData)
		{
			WideCharToMultiByte(CP_ACP,NULL,&uData,1,pOut,sizeof(wchar_t),NULL,NULL);
			return;
		}    

		inline void Gb2312ToUnicode(wchar_t* pOut,char *gbBuffer)
		{
			::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,gbBuffer,2,pOut,1);
			return ;
		}

		//following add by golding
		inline void UTF_8ToUnicode(wchar_t* pOut,const char *pText)
		{
			char* uchar = (char *)pOut;

			uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
			uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);

			return;
		}

		inline void UnicodeToUTF_8(char* pOut,wchar_t* pText)
		{
			// 注意 WCHAR高低字的顺序,低字节在前，高字节在后
			char* pchar = (char *)pText;

			pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
			pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
			pOut[2] = (0x80 | (pchar[0] & 0x3F));

			return;
		}

		inline void GB2312ToUTF_8(string& pOut,char *pText, int pLen)
		{
			char buf[4];
			int nLength = pLen* 3;
			char* rst = new char[nLength];
   
			memset(buf,0,4);
			memset(rst,0,nLength);
   
			int i = 0;
			int j = 0;     
			while(i < pLen)
			{
					//如果是英文直接复制就能
					if( *(pText + i) >= 0)
					{
							rst[j++] = pText[i++];
					}
					else
					{
							wchar_t pbuffer;
							Gb2312ToUnicode(&pbuffer,pText+i);
                    
							UnicodeToUTF_8(buf,&pbuffer);
                    
							unsigned short int tmp = 0;
							tmp = rst[j] = buf[0];
							tmp = rst[j+1] = buf[1];
							tmp = rst[j+2] = buf[2];    
                    
							j += 3;
							i += 2;
					}
			}
			rst[j] = '\0';

			//返回结果
			pOut = rst;             
			delete []rst;  
   
			return;
		}

		inline  void UTF_8ToGB2312(string &pOut,const char *pText, int pLen)
		{
			char * newBuf = new char[pLen];
			char Ctemp[4];
			memset(Ctemp,0,4);

			int i =0;
			int j = 0;
   
			while(i < pLen)
			{
				if(pText > 0)
				{
						newBuf[j++] = pText[i++];                      
				}
				else                
				{
						WCHAR Wtemp;
						UTF_8ToUnicode(&Wtemp,pText + i);
        
						UnicodeToGB2312(Ctemp,Wtemp);
           
						newBuf[j] = Ctemp[0];
						newBuf[j + 1] = Ctemp[1];

						i += 3;    
						j += 2;  
				}
			}
			newBuf[j] = '\0';
   
			pOut = newBuf;
			delete []newBuf;
   
			return;
		}
		
		inline bool	FindOrCreateFolder( tstring strPath )
		{
			WIN32_FIND_DATA wfd;
			bool rValue = false;
			HANDLE hFind = FindFirstFile(strPath.c_str(), &wfd);
			if ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				rValue = true;  
			}
			else
			{
				string strPath_Ansi = WideCharToAnsi(strPath.c_str());
				int nResult = _mkdir(strPath_Ansi.c_str());
				if(nResult == 0)
					rValue = true;  
			}
			FindClose(hFind);
			return rValue;
		}

		inline bool	FindFileExist( tstring strPath )
		{
			fstream file;
			file.open(WideCharToAnsi(strPath.c_str()),ios::in);
			if(file)
				return true;
			return false;
		}

		inline void String_To_SysTime( const string & strTime, SYSTEMTIME & systime )
		{
			wstring str = UTF8ToWideChar(strTime.c_str());
			COleDateTime	OleTime;
			OleTime.ParseDateTime(str.c_str(), 0, LANG_USER_DEFAULT );
			OleTime.GetAsSystemTime( systime );
		}

		inline string OutTimeInfo(const string strInfo)
		{
			if(strInfo.length() > 225)
				return "";
			SYSTEMTIME st;
			GetLocalTime(&st);
			char buff[256];	
			sprintf_s(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d--%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,strInfo.c_str());
			string result = buff;
			return result;
		}

		class CCustomTools
		{
		public:
			CCustomTools(void);
		public:
			~CCustomTools(void);

		};

		// "yyyy-m-d hh:mm:ss";
		inline string SystemTimeToString(const SYSTEMTIME& st)
		{
			char buff[25];	
			sprintf_s(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			string result = buff;
			return result;
		}

		inline string tstringToString(const tstring & s)
		{
            if (s.empty())
            {
                return "";
            }
			_bstr_t temp = s.c_str();	
			string result((const char*)temp);
			return result;
		}

        //////////////////////////////////////////////////////////////////////////
        inline void SplitStringByChar(CString allStr, TCHAR x, std::vector<tstring> &strArray)
        {//split allStr to strArray
            // "3/4/5" "/" ->(3,4,5) array
            strArray.clear();
            if(allStr=="") return;
            int aa=allStr.Find(x);
            if(aa==-1) 
            {
                strArray.push_back((LPCTSTR)allStr);//add Element//////
                return;
            }
            strArray.push_back((LPCTSTR)allStr.Left(aa));//add Element//////
            int bb=allStr.Find(x, aa+1);
            if(bb==-1) 
            {
                strArray.push_back((LPCTSTR)allStr.Mid(aa+1));//add Element//////
                return;
            }
            while(bb!=-1)
            {
                strArray.push_back((LPCTSTR)allStr.Mid(aa+1,bb-aa-1));//add Element//////
                aa=bb;
                bb=allStr.Find(x,aa+1);//add Element//////
            }
            strArray.push_back((LPCTSTR)allStr.Mid(aa+1));//add Element//////	
        }

		inline void SplitString(const char* source, const char* sep, vector<int>& resultlist)
		{
			assert(source != NULL);
			char* strcopy = _strdup(source);
			assert(strcopy != NULL);

			char* token = NULL;
			char* nexttoken = NULL;
			token = strtok_s(strcopy, sep, &nexttoken);

			int temp = -1;
			while(token != NULL)
			{
				temp = atoi(token);
				resultlist.push_back(temp);
				token = strtok_s(NULL, sep, &nexttoken);
			}

			free(strcopy);
		}

		inline void SplitString(const char* source, const char* sep, vector<string>& resultlist)
		{
			assert(source != NULL);
			char* strcopy = _strdup(source);
			assert(strcopy != NULL);

			char* token = NULL;
			char* nexttoken = NULL;
			token = strtok_s(strcopy, sep, &nexttoken);

			while(token != NULL)
			{
				string temp = token;
				resultlist.push_back(temp);
				token = strtok_s(NULL, sep, &nexttoken);
			}
			free(strcopy);
		}

		inline void SplitString(const char* source, const char* sep, vector<double>& resultlist)
		{
			assert(source != NULL);
			char* strcopy = _strdup(source);
			assert(strcopy != NULL);

			char* token = NULL;
			char* nexttoken = NULL;
			token = strtok_s(strcopy, sep, &nexttoken);

			double temp = 0.0;
			while(token != NULL)
			{
				temp = atof(token);
				resultlist.push_back(temp);
				token = strtok_s(NULL, sep, &nexttoken);
			}

			free(strcopy);
		}

		inline bool DelteFile( CString strDir,CString strDeleteDir )
		{
			strDir += '\0';
			strDeleteDir += '\0';
			SHFILEOPSTRUCT    shFileOp;  
			memset(&shFileOp,0,sizeof(shFileOp));  
			shFileOp.wFunc    = FO_MOVE;  
			shFileOp.pFrom    = strDir;  
			shFileOp.pTo    = strDeleteDir;  
			shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION|FOF_ALLOWUNDO|FOF_NOCONFIRMMKDIR;  
			int nResult = ::SHFileOperation(&shFileOp);  
			return nResult == 0;  
		}

		inline bool DelteFile( CString strDir )
		{
			strDir += '\0';
			SHFILEOPSTRUCT    shFileOp;  
			memset(&shFileOp,0,sizeof(shFileOp));  
			shFileOp.wFunc    = FO_DELETE;  
			shFileOp.pFrom    = strDir;  
			shFileOp.pTo    = NULL;  
			shFileOp.fFlags    = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;  
			int nResult = ::SHFileOperation(&shFileOp);  
			return nResult == 0;
		}

		inline bool FindFile( CString strFolder,CString strName )
		{
			CFileFind tempFind;
			CString strFind;
			strFind.Format(_T("%s\\*.*"),strFolder);
			BOOL IsFinded = tempFind.FindFile(strFind);
			CString strBackupPath;
			while (IsFinded)
			{
				IsFinded = tempFind.FindNextFile();
				if (!tempFind.IsDots()) 
				{
					if (!tempFind.IsDirectory())
					{
						CString strFileName = tempFind.GetFileName();
						if(strFileName.Find(strName)>=0)
						{
							tempFind.Close();
							return true;
						}					
					}
				}
			}
			tempFind.Close();
			return false;
		}
        //////////////////////////////////////////////////////////////////////////
        
       inline  CString KGetComputerName()
        {
            TCHAR  name_s[MAX_PATH];
            memset(name_s, 0x00, MAX_PATH);
            DWORD  size = MAX_PATH;

            ::GetComputerName(name_s, &size);

            CString result(name_s);
            return  result;
        }

		class Mutex
		{
		public:
			Mutex() {  InitializeCriticalSection(&cs); }
			~Mutex() { DeleteCriticalSection(&cs); }

			void Lock() { EnterCriticalSection(&cs); }
			void UnLock() { LeaveCriticalSection(&cs);  }

		protected:
		private:
			CRITICAL_SECTION cs;
			// non copyable.
			Mutex(const Mutex&);
			Mutex& operator=(const Mutex&);
		};

		// this design will be much more elegant.
		template <typename T>
		class Scoped_Lock
		{
		public:
			Scoped_Lock(T& m):mutex(m)
			{
				mutex.Lock();
			}
			~Scoped_Lock()
			{
				mutex.UnLock();
			}
		private:
			T& mutex;

			// can not be copied
			Scoped_Lock(const Scoped_Lock&);
			Scoped_Lock& operator =(const Scoped_Lock&);
		};

		//删除指针
		template<typename T>
		void  DELETEPTR(T* pointer)
		{
			if (pointer != NULL)
			{
				delete pointer;
				pointer = NULL;
			}
		}
		
		//删除数组指针
		template<typename T>
		void  DELETEARRAYPTR(T* pointer)
		{
			if (pointer != NULL)
			{
				delete[] pointer;
				pointer = NULL;
			}
		}


		class TimeCounter
		{
		private:
			DWORD start;
		public:
			TimeCounter() {Start();}
			void Start(){start = GetTickCount(); }
			DWORD Now() 
			{ 
				DWORD end = GetTickCount(); 		
				return (end - start); // return miliseconds
			}	
		};


		class HighResolutionTimeCounter
		{
			LARGE_INTEGER start;
			LARGE_INTEGER freq;
		public:
			HighResolutionTimeCounter() { QueryPerformanceFrequency(&freq); Start();}
			void Start() { QueryPerformanceCounter(&start); }
			__int64 Now() //  return miliseconds
			{ 
				LARGE_INTEGER end = { 0 };
				QueryPerformanceCounter(&end);
				return (end.QuadPart - start.QuadPart)*1000 / freq.QuadPart;
			}
		};

        inline CString SafeFormat(const TCHAR* format, ...)
        {
            TCHAR buff[1024];
            va_list argList;
            va_start(argList, format);
            memset(buff, 0, 1024);
            StringCchVPrintf(buff, 1024, format, argList);//strsafe.h method
            va_end(argList);
              
            CString result(buff);
            return result;
        }

		inline bool  IsDoubleEqual(double d1, double d2)
		{
			return ::fabs(d1-d2) < 0.0001;
		}
	

		inline BOOL FindProcessByName(CString exefile)
		{
			int nExeNum = 0;
			CString strTemp = _T("");

			if (exefile.IsEmpty()) {
				return FALSE;
			}
			HANDLE handle =CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
			PROCESSENTRY32 pInfo;
			memset(&pInfo, 0x00, sizeof(PROCESSENTRY32));
			pInfo.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(handle, &pInfo))
			{
				while (Process32Next(handle, &pInfo) != FALSE)
				{
					strTemp = pInfo.szExeFile;
					if (0 == strTemp.CompareNoCase(exefile))
					{
						if (++nExeNum > 1)   // there are more than one instance running.
						{
							CloseHandle(handle);
							return TRUE;
						}					
					}
				}
			}

			//delete pInfo;
			CloseHandle(handle);
			return FALSE;
		}
	}
}