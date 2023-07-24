#pragma once


#include <string>
#include "TCHAR.h"
//#include "strsafe.h"

using std::wstring;
using std::string;
namespace Project
{
	namespace Tools
	{
		#define sstring	std::string
		#define tstring std::wstring

	}
}


//StringCchVPrintf	TCHAR[]
//_tcstod
//_tcstol
//.c_str()
//_bstr_t	comdef.h