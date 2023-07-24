
#include "stdafx.h"

#include "UtilString.h"

#include <tchar.h>
#include <windows.h>

using namespace std;

//////////////////////////////////////////////////////////////////////////
namespace UtilString{

    //must free LPWSTR after call this func
    std::wstring ConvertMultiByteToWideChar(const std::string& strSrc)
    {
        const char* pStr = strSrc.c_str();
        //size_t nLenOld = strSrc.length();
        const int nLen = (int)strSrc.length();

        wstring retStr;
        TCHAR* pwstr = NULL;
        int len=0;

        len=MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pStr,nLen+1,pwstr,0);
        if(len==0)
            return retStr;

        pwstr=(LPWSTR)malloc(len*sizeof(WCHAR));
        len=MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,pStr,nLen+1,pwstr,len);
        if(len==0)
        {
            free(pwstr);
            return retStr;
        }
        retStr = pwstr;
        free(pwstr);

        return retStr;
    }

    std::string ConvertWideCharToMultiByte(const std::wstring& wstrSrc)                                       
    {
        const TCHAR* pWStr = wstrSrc.c_str();
        const int nLen = (int)wstrSrc.length();
        string ret;
        LPSTR pStr=0;
        int len=WideCharToMultiByte(CP_ACP,0,pWStr,nLen+1,0,0,0,0);
        if(!len)
            return ret;		

        pStr=(LPSTR)malloc(len);

        len=WideCharToMultiByte(CP_ACP,0,pWStr,nLen+1,pStr,len,0,0);
        if(!len)
        {
            free(pStr);
            return ret;		
        }
        ret = pStr;
        free(pStr);

        return ret;
    }

    wstring ConvertMultiByteToWideCharUTF8(const std::string& strSrc)
    {
        const char* pStr = strSrc.c_str();
        const int nLen = (int)strSrc.length();
        LPWSTR pwstr=NULL;
        wstring wstrRet;
        int len=0;

        len=MultiByteToWideChar(CP_UTF8,0,pStr,nLen+1,pwstr,0);
        if(len==0)
            return 0;

        pwstr=(LPWSTR)malloc(len*sizeof(WCHAR));
        len=MultiByteToWideChar(CP_UTF8,0,pStr,nLen+1,pwstr,len);
        if(len==0)
        {
            free(pwstr);
            return wstrRet;
        }
        pwstr[len-1] = 0;//Added by Kevin, 2008/08/19, in order to set pwstr to null terminated string
        wstrRet = pwstr;

        return wstrRet;
    }


    //bird 20080816
    //must free LPWSTR after call this func
    string ConvertWideCharToMultiByteUTF8(const std::wstring& wstrSrc)
    {
        const TCHAR* pWStr = wstrSrc.c_str();
        const int nLen = (int)wstrSrc.length();
        LPSTR pStr=0;
        string strRet;
        int len=0;

        //CP_ACP
        len=WideCharToMultiByte(CP_UTF8,0,pWStr,nLen+1,0,0,0,0);
        if(!len) //????
            return 0;		

        pStr=(LPSTR)malloc(len);

        //CP_ACP
        len=WideCharToMultiByte(CP_UTF8,0,pWStr,nLen+1,pStr,len,0,0);
        if(!len)
        {
            free(pStr);
            return strRet;		
        }
        strRet = pStr;
        return strRet;
    }

    //add huafengyou   the strDes must be the char array
	void WideToMultiByte(std::wstring &strSrc, char* strDes)
	{
		int i =  WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)strSrc.c_str(), -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)strSrc.c_str(), -1, strDes, i, NULL, NULL);
	}

    //bird add
    //make lower
    string  StringMakeLower(const string& strSrc)
    {        
        //_strlwr
        char* vecChar = new char[strSrc.length() +1];
        strcpy(vecChar, strSrc.c_str() ); //strSrc.length(), 
        string strRet = _strlwr( vecChar);
        SAFE_DELETE_ARRAY(vecChar);
        return strRet;
    }

    wstring  StringMakeLower(const wstring& strSrc)
    {        
        TCHAR* vecChar = new TCHAR[strSrc.length() +1];
        _tcscpy(vecChar, strSrc.c_str() ); //strSrc.length(), 
        wstring strRet = _wcslwr( vecChar);
        SAFE_DELETE_ARRAY(vecChar);
        return strRet;
    }

    //string StringMakeUpper(const string& strSrc)
    //{
    //    string strRet = strupr(strSrc.c_str() );
    //    return strRet;
    //}
    //
    //wstring StringMakeUpper(const wstring& strSrc)
    //{
    //    string strRet = strupr(strSrc.c_str() );
    //    return strRet;
    //}

    //convert to low string. then search str in str
    bool FindStringInString(const wstring& strSrc0, const wstring& strToFind0)
    {
        bool ifFind = false;
        //CString strSrc1 = strSrc0.c_str();
        //strSrc1.MakeLower();
        //CString strToFind1 = strToFind0.c_str();
        //strToFind1.MakeLower();

        wstring strSrc1 = StringMakeLower(strSrc0);
        wstring strToFind1 = StringMakeLower(strToFind0);

        int findPos = (int)strSrc1.find(strToFind1);
        if(findPos >=0 && findPos < (int)strSrc1.length() )
            ifFind = true;
        return ifFind;
    }

    //str to d
    UINT ConvertStringToNumber(const wstring& wstr)
    {
        if(_T(" ") == wstr){
            return 0;
        }
        const string str0= ConvertWideCharToMultiByte(wstr );  
        int num = atoi( str0.c_str() );
        return num;
    }

    //sscanf("a0", "%02X", &num1);
    //get number from string like "0x84acef"
    UINT ConvertHexStringToNumber(const std::wstring& wstrOld)
    {
        UINT num = 0;
        //vector<TCHAR> tcharArray;
        const wstring strHex = _T("0123456789abcdefxxxxx");

        const wstring wstrLow = StringMakeLower(wstrOld);

        const UINT strLength = (UINT) wstrLow.length();
        int pos = -1;
        TCHAR ch;
        for(UINT idx =0; idx < strLength; idx++){
            ch = wstrLow.at(idx);
            pos = (int)strHex.find(ch);
            if(pos< 0
                || pos >= (int)strHex.length() ){
                    //ASSERT(false);
                    continue;
            }
            else if(pos >15){ }
            else{
                num = num*16 + pos;
            }
        }
        return num;
    }

    std::string       NumberToString(int number)
    {
        char charvec[10];
        _itoa(number, charvec ,10);
        const string str = charvec;
        return str;
    }
    std::wstring        NumberToWstring(int number)
    {
        char charvec[10];
        _itoa(number, charvec ,10);
        const string str = charvec;
        const wstring wstr = ConvertMultiByteToWideChar( str);
        return wstr;
    }


	bool CheckStringIsNumber(const std::wstring& StrCheck)
	{
		CString Str = StrCheck.c_str();
		if (Str == L"")
		{
			return false;
		}
		if (Str[0] == '-')
		{
			Str = Str.Mid(1,Str.GetLength());
		}

		CString StrNum = L".0123456789";

		int iStrLen = Str.GetLength();

		int iPointCount = 0;
		for (int i=0;i<iStrLen;++i)
		{
			CString wch(Str[i]);
			if (StrNum.FindOneOf(wch)<0)
			{
				return false;
			}
			if (wch == L".")
			{
				iPointCount++;
			}
		}
		if (Str[0] == '.'||Str[Str.GetLength()-1] == '.')
		{
			return false;
		}
		if (iPointCount>1)
		{
			return false;
		}
		return true;
	}


    //判断是否一个string全部是数字, 如有, 转换
    bool    CheckStringIsNumber(const std::wstring& wstrOld, int& number)
    {
        
        number = 0;

        const wstring strNumber = L"0123456789-.";
        const wstring wstrLow = StringMakeLower(wstrOld);
        const UINT strLength = (UINT) wstrLow.length();

        int pos = -1;
        TCHAR ch;
        bool ifNumber =true;

        for(UINT idx =0; idx < strLength; idx++){
            ch = wstrLow.at(idx);
            pos = (int)strNumber.find(ch);
            if(pos< 0
                || pos >= (int)strNumber.length() ){
                    //ASSERT(false);
                    ifNumber =false;
                    break;
            }
        }
        if(ifNumber){
            number = ConvertStringToNumber(wstrOld);
        }

        return ifNumber;
    }
    //////////////////////////////////////////////////////////////////////////
    void SplitStringByChar(CString allStr, TCHAR x, std::vector<std::wstring> &strArray)
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

    void SplitString(const std::string& strsource, const char* sep, vector<int>& resultlist)
    {
        if( !sep)
            return;

        resultlist.clear();
        //assert(source != NULL);
        char* strcopy = _strdup(strsource.c_str() );
        //assert(strcopy != NULL);

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

    void SplitString(const std::string& source, const char* sep, vector<double>& resultlist)
    {
        if( !sep)
            return;

        resultlist.clear();

        //assert(source != NULL);
        char* strcopy = _strdup(source.c_str());
        //assert(strcopy != NULL);

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
    //////////////////////////////////////////////////////////////////////////

    std::string       ConvertVectorToString(const std::vector<int>& vecInt, const char* charSep)
    {
        string strRet;
        string strSep;
        string strTemp;
        if(charSep)
            strSep = charSep;
        for(UINT idx =0; idx< vecInt.size(); idx++){
            if(idx>0)
                strRet += strSep;
            strTemp = NumberToString(vecInt[idx]);
            strRet += strTemp;
        }

        return strRet;
    }
}; //namespace
