#pragma once

#include <string>

#include <WTypes.h>

namespace UtilString{
  std::wstring      ConvertMultiByteToWideChar(const std::string& strSrc);
  std::wstring      ConvertMultiByteToWideCharUTF8(const std::string& strSrc);

  std::string       ConvertWideCharToMultiByte(const std::wstring& wstrSrc);
  std::string       ConvertWideCharToMultiByteUTF8(const std::wstring& wstrSrc);

  void              WideToMultiByte(std::wstring &strSrc, char* strDes);

  bool              FindStringInString(const std::wstring& strSrc0, const std::wstring& strToFind0);
  bool              CheckStringIsNumber(const std::wstring& wstr, int& number);
  bool              CheckStringIsNumber(const std::wstring& wstr);
 
  std::string       StringMakeLower(const std::string& strSrc);
  std::string       StringMakeUpper(const std::string& strSrc);
  std::wstring      StringMakeUpper(const std::wstring& strSrc);
  std::wstring      StringMakeLower(const std::wstring& strSrc);

  std::wstring      NumberToWstring(int number);
  std::string       NumberToString(int number);

  UINT              ConvertStringToNumber(const std::wstring& wstr);
  UINT              ConvertHexStringToNumber(const std::wstring& wstrOld);

  std::string       ConvertVectorToString(const std::vector<int>& vecInt, const char* sep);
  void              SplitStringByChar(CString allStr, TCHAR x, std::vector<std::wstring> &strArray);
  void              SplitString(const std::string& source, const char* sep, std::vector<int>& resultlist);
  void              SplitString(const std::string& source, const char* sep, std::vector<double>& resultlist);

}
