#include "StdAfx.h"
#include "ExcelOperator.h"
#include "afxdb.h"
#include <cassert>
#include "odbcinst.h"
#include "Tools/CustomTools/CustomTools.h"
#include "CSVParser.h"
using namespace CSVParser;

//严观微增加用于xls(97-2003)版本的excel读写类

CExcelOperator* CExcelOperator::m_instance = NULL;


CExcelOperator::CExcelOperator(void)
{ 

}


CExcelOperator::~CExcelOperator(void)
{
}

CExcelOperator& CExcelOperator::Instance()
{
	if (m_instance == NULL)
	{
		m_instance = new CExcelOperator();
		assert(m_instance);
	}
	return *m_instance;
}

void CExcelOperator::DestoryInstance()
{
	if (m_instance)
	{
		delete m_instance;
		m_instance = NULL;
	}
}

void   CExcelOperator::SaveToExcel(CString sExcelFile, CDataPointManager& pointmap)
  {   
	  if(GetExcelDriver().IsEmpty())
	  {
		  if(AfxMessageBox(L"Not to save the Excel file Engine, Please confirm installation?",MB_YESNO) == IDYES)
		  {
			  wstring strSys_;
			  GetSysPath(strSys_);
			  CString strSys = strSys_.c_str();
			  int nRet = (int)::ShellExecute(NULL,L"open",strSys+L"\\driver\\AccessDatabaseEngine.exe",NULL,NULL,SW_SHOWNORMAL);
			  if(nRet <= 32)
			  {
				  AfxMessageBox(L"Driver file does not exist, Please contact the relevant personnel!");
				  return;
			  }
		  }
	  }
      CDatabase database;   
      CString sDriver = L"Microsoft Excel Driver (*.xls)";
      CString   sSql;    
	  const CString	cFlag(_T("'"));
      try   
      {   
          sSql.Format(L"DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%s\";DBQ=%s",sDriver,sExcelFile,sExcelFile);   
          if(database.OpenEx(sSql,CDatabase::noOdbcDialog))   
          {   
			  if(_waccess(sExcelFile,0) == 0)
			  {
				  try
				  {
					  database.ExecuteSQL(L"DROP TABLE Sheet1");
				  }
				  catch(CDBException*  e)   
				  {   
					  wchar_t err[MAX_PATH] = {0};
					  e->GetErrorMessage(err,MAX_PATH);
					 // AfxMessageBox(err);
				  }   
			  }
              sSql = L"CREATE TABLE Sheet1 (pointindex TEXT,physicalid TEXT,source TEXT,remark TEXT,Unit TEXT,RWProperty TEXT,param1 TEXT,param2 TEXT,param3 TEXT,param4 TEXT,param5 TEXT,param6 TEXT,param7 TEXT,param8 TEXT,param9 TEXT,param10 TEXT,param11 TEXT,param12 TEXT,param13 TEXT,param14 NUMBER,storecycle TEXT,customName TEXT,system TEXT,device TEXT,type TEXT)";   
              database.ExecuteSQL(sSql);
			  map<int, DataPointEntry>& datalist = pointmap.GetPointMap();
			  map<int, DataPointEntry>::const_iterator it = datalist.begin();
			  for(;it != datalist.end(); it++)
			  {
				  const DataPointEntry& entry = it->second;
				  sSql = L"INSERT INTO Sheet1(pointindex,physicalid,source,remark,Unit,RWProperty,param1,param2,param3,param4,param5,param6,param7,param8,param9,param10,param11,param12,param13,param14,storecycle,customName,system,device,type) VALUES";
				  CString str;
				  CString strIndex;
				  strIndex.Format(L"%d",entry.GetPointIndex());
				  CString strPhysicalID = entry.GetShortName().c_str();
				  CString strCource = entry.GetSourceType().c_str();
				  CString strRemark = entry.GetDescription().c_str();
				  CString strUnit = entry.GetUnit().c_str();
				  CString strProperty;
				  CString strStoreCycle;
				  strStoreCycle.Format(L"%d",(int)entry.GetStoreCycle());
				  if(entry.GetProperty() == PLC_READ)
				  {
					  strProperty = L"R";
				  }
				  else if(entry.GetProperty() == PLC_WRITE)
				  {
					  strProperty = L"W";
				  }
				  else if(entry.GetProperty() == PLC_PROP_MAX)
				  {
					  strProperty = L"R/W";
				  }
				  CString strParam1 = entry.GetParam(1).c_str();
				  CString strParam2 = entry.GetParam(2).c_str();
				  CString strParam3 = entry.GetParam(3).c_str();
				  CString strParam4 = entry.GetParam(4).c_str();
				  CString strParam5 = entry.GetParam(5).c_str();
				  CString strParam6 = entry.GetParam(6).c_str();
				  CString strParam7 = entry.GetParam(7).c_str();
				  CString strParam8 = entry.GetParam(8).c_str();
				  CString strParam9 = entry.GetParam(9).c_str();
				  CString strParam10 = entry.GetParam(10).c_str();
				  CString strParam11 = entry.GetParam(11).c_str();
				  CString strParam12 = entry.GetParam(12).c_str();
				  CString strParam13 = entry.GetParam(13).c_str();
				  CString strParam14 = entry.GetParam(14).c_str();
				  CString strParam15 = entry.GetParam(15).c_str();
				  CString strParam16 = entry.GetParam(16).c_str();
				  CString strParam17 = entry.GetParam(17).c_str();
				  CString strParam18 = entry.GetParam(18).c_str();
				  if (cFlag == strIndex
					  || cFlag == strPhysicalID
					  || cFlag == strCource
					  || cFlag == strRemark
					  || cFlag == strUnit
					  || cFlag == strProperty
					  || cFlag == strParam1
					  || cFlag == strParam2
					  || cFlag == strParam3
					  || cFlag == strParam4
					  || cFlag == strParam5
					  || cFlag == strParam6
					  || cFlag == strParam7
					  || cFlag == strParam8
					  || cFlag == strParam9
					  || cFlag == strParam10
					  || cFlag == strParam11
					  || cFlag == strParam12
					  || cFlag == strParam13
					  || cFlag == strParam14
					  || cFlag == strStoreCycle
					  || cFlag == strParam15
					  || cFlag == strParam16
					  || cFlag == strParam17
					  || cFlag == strParam18)
				  {
					  database.Close();
					  CString strErr;
					  CString strNum;
					  int nPos = strIndex.Find(_T("."));
					  if (nPos != -1) {
						  strNum = strIndex.Left(nPos);
					  }
					  strErr.Format(_T("Index%s，\r\nPoint %s，\r\nParameter contains a single quotation mark,Export termination!"), strNum, strPhysicalID);
					  AfxMessageBox(strErr);
					  return;
				  }

				  str.Format(L"('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
					  strIndex,strPhysicalID,strCource,strRemark,strUnit,strProperty,strParam1,strParam2,strParam3,strParam4,strParam5,strParam6,strParam7,strParam8,strParam9,strParam10,strParam11,strParam12,strParam13,strParam14,strStoreCycle,strParam15,strParam16,strParam17,strParam18);
				  sSql += str;
				  database.ExecuteSQL(sSql);   
			  }
          } 
		  AfxMessageBox(L"Export table success!");
          database.Close();   
      }   
      catch(CDBException*  e)   
      {   
		  wchar_t err[MAX_PATH] = {0};
		  e->GetErrorMessage(err,MAX_PATH);
		  AfxMessageBox(err);
      }   
  }   

CString CExcelOperator::GetExcelDriver()
{
    wchar_t szBuf[2001];
    WORD cbBufMax = 2000;
    WORD cbBufOut;
    wchar_t *pszBuf = szBuf;
    CString sDriver;

    if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut))
	{
        return L"";
	}
    do
    {
        if (wcsstr(pszBuf, L"Excel") != 0)
        {
            sDriver = CString(pszBuf);
            break;
        }
        pszBuf = wcschr(pszBuf, L'\0') + 1;
    }
    while (pszBuf[1] != L'\0');
    return sDriver;
}

bool CExcelOperator::ReadFromExcel( CString sExcelFile, CDataPointManager& pointmap )
{
	if(GetExcelDriver().IsEmpty())
	{
		if(AfxMessageBox(L"Not to save the Excel file Engine, Please confirm installation?",MB_YESNO) == IDYES)
		{
			wstring strSys_;
			GetSysPath(strSys_);
			CString strSys = strSys_.c_str();
			int nRet = (int)::ShellExecute(NULL,L"open",strSys+L"\\driver\\AccessDatabaseEngine.exe",NULL,NULL,SW_SHOWNORMAL);
			if(nRet <= 32)
			{
				return false;
			}
		}
	}
	CDatabase database;   
	CString sDriver = L"Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)";
	CString   sSql;    
	const CString	cFlag(_T("'"));
	try   
	{   
		sSql.Format(L"DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=TRUE;CREATE_DB=\"%s\";DBQ=%s",sDriver,sExcelFile,sExcelFile);   
		if(database.OpenEx(sSql,CDatabase::noOdbcDialog))   
		{   
			CRecordset recset(&database);
			sSql = L"Select * from [Sheet1$]";   
			recset.Open(CRecordset::forwardOnly, sSql, CRecordset::readOnly);
			while(!recset.IsEOF())
			{
				DataPointEntry entry;
				CString strIndex;
				CString strPhysicalID;
				CString strSource;
				CString strRemark;
				CString strUnit;
				CString strRWProperty;
				CString strparam1;
				CString strparam2;
				CString strparam3;
				CString strparam4;
				CString strparam5;
				CString strparam6;
				CString strparam7;
				CString strparam8;
				CString strparam9;
				CString strparam10;
				CString strparam11;
				CString strparam12;
				CString strparam13;
				CString strparam14;
				CString strStoreCycle;
				CString strparam15;
				CString strparam16;
				CString strparam17;
				CString strparam18;
				recset.GetFieldValue(L"pointindex", strIndex);
				recset.GetFieldValue(L"physicalid", strPhysicalID);
				recset.GetFieldValue(L"source", strSource);
				recset.GetFieldValue(L"remark", strRemark);
				recset.GetFieldValue(L"Unit", strUnit);
				recset.GetFieldValue(L"RWProperty", strRWProperty);
				recset.GetFieldValue(L"param1", strparam1);
				recset.GetFieldValue(L"param2", strparam2);
				recset.GetFieldValue(L"param3", strparam3);
				recset.GetFieldValue(L"param4", strparam4);
				recset.GetFieldValue(L"param5", strparam5);
				recset.GetFieldValue(L"param6", strparam6);
				recset.GetFieldValue(L"param7", strparam7);
				recset.GetFieldValue(L"param8", strparam8);
				recset.GetFieldValue(L"param9", strparam9);
				recset.GetFieldValue(L"param10", strparam10);
				recset.GetFieldValue(L"param11", strparam11);
				recset.GetFieldValue(L"param12", strparam12);
				recset.GetFieldValue(L"param13", strparam13);
				recset.GetFieldValue(L"param14", strparam14);
				recset.GetFieldValue(L"storecycle", strStoreCycle);
				DWORD dwColNum = recset.GetODBCFieldCount();
				if (dwColNum > 21)
				{	// compatible old version
					recset.GetFieldValue(L"customName", strparam15);
					recset.GetFieldValue(L"system", strparam16);
					recset.GetFieldValue(L"device", strparam17);
					recset.GetFieldValue(L"type", strparam18);
				}

				if (cFlag == strIndex
					|| cFlag == strPhysicalID
					|| cFlag == strSource
					|| cFlag == strRemark
					|| cFlag == strUnit
					|| cFlag == strRWProperty
					|| cFlag == strparam1
					|| cFlag == strparam2
					|| cFlag == strparam3
					|| cFlag == strparam4
					|| cFlag == strparam5
					|| cFlag == strparam6
					|| cFlag == strparam7
					|| cFlag == strparam8
					|| cFlag == strparam9
					|| cFlag == strparam10
					|| cFlag == strparam11
					|| cFlag == strparam12
					|| cFlag == strparam13
					|| cFlag == strparam14
					|| cFlag == strStoreCycle
					|| cFlag == strparam15
					|| cFlag == strparam16
					|| cFlag == strparam17
					|| cFlag == strparam18)
				{
					database.Close();
					CString strErr;
					CString strNum;
					int nPos = strIndex.Find(_T("."));
					if (nPos != -1) {
						strNum = strIndex.Left(nPos);
					}
					strErr.Format(_T("Index%s，\r\nPoint %s，\r\nParameter contains a single quotation mark,Export termination!"), strNum, strPhysicalID);
					return false;
				}

				int nIndex = _wtoi(strIndex);
				entry.SetPointIndex(nIndex);
				entry.SetShortName(strPhysicalID.GetString());

				entry.SetSourceType(strSource.GetString());

				entry.SetDescription(strRemark.GetString());

				entry.SetUnit(strUnit.GetString());

				if(strRWProperty == L"R")
				{
					entry.SetProperty(PLC_READ);
				}
				else if(strRWProperty == L"W")
				{
					entry.SetProperty(PLC_WRITE);
				}
				else if(strRWProperty == L"R/W")
				{
					entry.SetProperty(PLC_PROP_MAX);
				}
				else
				{
					entry.SetProperty(PLC_READ);
				}
				entry.SetParam(1, strparam1.GetString());
				entry.SetParam(2, strparam2.GetString());
				entry.SetParam(3, strparam3.GetString());
				entry.SetParam(4, strparam4.GetString());
				entry.SetParam(5, strparam5.GetString());
				entry.SetParam(6, strparam6.GetString());
				entry.SetParam(7, strparam7.GetString());
				entry.SetParam(8, strparam8.GetString());
				entry.SetParam(9, strparam9.GetString());
				entry.SetParam(10, strparam10.GetString());
				entry.SetParam(11, strparam11.GetString());
				entry.SetParam(12, strparam12.GetString());
				entry.SetParam(13, strparam13.GetString());
				entry.SetParam(14, strparam14.GetString());
				entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
				entry.SetParam(15, strparam15.GetString());
				entry.SetParam(16, strparam16.GetString());
				entry.SetParam(17, strparam17.GetString());
				entry.SetParam(18, strparam18.GetString());
				if(entry.GetShortName().size() != 0)
				{
					pointmap.AddEntry(entry);
				}
				recset.MoveNext();
			} 
			database.Close();   
			return true;
		}   
	}
	catch(CDBException*  e)   
	{   
		wchar_t err[MAX_PATH] = {0};
		e->GetErrorMessage(err,MAX_PATH);
		AfxMessageBox(err);
	} 
}

bool CExcelOperator::ReadFromExcel( CString sExcelFile, vector<DataPointEntry>& vecPoint ,int& nErrCode)
{
	vecPoint.clear();
	if(GetExcelDriver().IsEmpty())
	{
		if(AfxMessageBox(L"Not to save the Excel file Engine, Please confirm installation?",MB_YESNO) == IDYES)
		{
			wstring strSys_;
			GetSysPath(strSys_);
			CString strSys = strSys_.c_str();
			int nRet = (int)::ShellExecute(NULL,L"open",strSys+L"\\driver\\AccessDatabaseEngine.exe",NULL,NULL,SW_SHOWNORMAL);
			if(nRet <= 32)
			{
				CString strLog;
				strLog.Format(_T("Engine failure install Excel files(%d)"),nRet);
				OutPutUpdateLog(strLog);
				nErrCode = nRet;
				return false;
			}
		}
	}

	//根据后缀名选择驱动  .xls默认.xls
	CString strSuffix = sExcelFile.Right(sExcelFile.GetLength() - sExcelFile.ReverseFind('.') - 1);
	if(strSuffix == _T("xls") || strSuffix == _T("XLS"))
	{
		CDatabase database;   
		CString sDriver = L"Microsoft Excel Driver (*.xls)";
		CString   sSql;    
		const CString	cFlag(_T("'"));
		try   
		{   
			sSql.Format(L"DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=TRUE;CREATE_DB=\"%s\";DBQ=%s",sDriver,sExcelFile,sExcelFile);   
			if(database.OpenEx(sSql,CDatabase::noOdbcDialog))   
			{   
				CRecordset recset(&database);
				sSql = L"Select * from [Sheet1$]";   
				recset.Open(CRecordset::forwardOnly, sSql, CRecordset::readOnly);
				while(!recset.IsEOF())
				{
					DataPointEntry entry;
					CString strIndex;
					CString strPhysicalID;
					CString strSource;
					CString strRemark;
					CString strUnit;
					CString strRWProperty;
					CString strparam1;
					CString strparam2;
					CString strparam3;
					CString strparam4;
					CString strparam5;
					CString strparam6;
					CString strparam7;
					CString strparam8;
					CString strparam9;
					CString strparam10;
					CString strparam11;
					CString strparam12;
					CString strparam13;
					CString strparam14;
					CString strStoreCycle;
					CString strparam15;
					CString strparam16;
					CString strparam17;
					CString strparam18;
					recset.GetFieldValue(L"pointindex", strIndex);
					recset.GetFieldValue(L"physicalid", strPhysicalID);
					recset.GetFieldValue(L"source", strSource);
					recset.GetFieldValue(L"remark", strRemark);
					recset.GetFieldValue(L"Unit", strUnit);
					recset.GetFieldValue(L"RWProperty", strRWProperty);
					recset.GetFieldValue(L"param1", strparam1);
					recset.GetFieldValue(L"param2", strparam2);
					recset.GetFieldValue(L"param3", strparam3);
					recset.GetFieldValue(L"param4", strparam4);
					recset.GetFieldValue(L"param5", strparam5);
					recset.GetFieldValue(L"param6", strparam6);
					recset.GetFieldValue(L"param7", strparam7);
					recset.GetFieldValue(L"param8", strparam8);
					recset.GetFieldValue(L"param9", strparam9);
					recset.GetFieldValue(L"param10", strparam10);
					recset.GetFieldValue(L"param11", strparam11);
					recset.GetFieldValue(L"param12", strparam12);
					recset.GetFieldValue(L"param13", strparam13);
					recset.GetFieldValue(L"param14", strparam14);
					recset.GetFieldValue(L"storecycle", strStoreCycle);
					DWORD dwColNum = recset.GetODBCFieldCount();
					if (dwColNum > 21)
					{	// compatible old version
						recset.GetFieldValue(L"customName", strparam15);
						recset.GetFieldValue(L"system", strparam16);
						recset.GetFieldValue(L"device", strparam17);
						recset.GetFieldValue(L"type", strparam18);
					}

					if (cFlag == strIndex
						|| cFlag == strPhysicalID
						|| cFlag == strSource
						|| cFlag == strRemark
						|| cFlag == strUnit
						|| cFlag == strRWProperty
						|| cFlag == strparam1
						|| cFlag == strparam2
						|| cFlag == strparam3
						|| cFlag == strparam4
						|| cFlag == strparam5
						|| cFlag == strparam6
						|| cFlag == strparam7
						|| cFlag == strparam8
						|| cFlag == strparam9
						|| cFlag == strparam10
						|| cFlag == strparam11
						|| cFlag == strparam12
						|| cFlag == strparam13
						|| cFlag == strparam14
						|| cFlag == strStoreCycle
						|| cFlag == strparam15
						|| cFlag == strparam16
						|| cFlag == strparam17
						|| cFlag == strparam18)
					{
						database.Close();
						CString strErr;
						CString strNum;
						int nPos = strIndex.Find(_T("."));
						if (nPos != -1) {
							strNum = strIndex.Left(nPos);
						}
						strErr.Format(_T("Index%s，\r\nPoint %s，\r\nParameter contains a single quotation mark,Export termination"), strNum, strPhysicalID);

						CString strLog;
						strLog.Format(_T("Read points table fails.(%s)"),strErr);
						OutPutUpdateLog(strLog);
						nErrCode = 31001;
						return false;
					}

					int nIndex = _wtoi(strIndex);
					entry.SetPointIndex(nIndex);
					entry.SetShortName(strPhysicalID.GetString());

					entry.SetSourceType(strSource.GetString());

					entry.SetDescription(strRemark.GetString());

					entry.SetUnit(strUnit.GetString());

					if(strRWProperty == L"R")
					{
						entry.SetProperty(PLC_READ);
					}
					else if(strRWProperty == L"W")
					{
						entry.SetProperty(PLC_WRITE);
					}
					else if(strRWProperty == L"R/W")
					{
						entry.SetProperty(PLC_PROP_MAX);
					}
					else
					{
						entry.SetProperty(PLC_READ);
					}
					entry.SetParam(1, strparam1.GetString());
					entry.SetParam(2, strparam2.GetString());
					entry.SetParam(3, strparam3.GetString());
					entry.SetParam(4, strparam4.GetString());
					entry.SetParam(5, strparam5.GetString());
					entry.SetParam(6, strparam6.GetString());
					entry.SetParam(7, strparam7.GetString());
					entry.SetParam(8, strparam8.GetString());
					entry.SetParam(9, strparam9.GetString());
					entry.SetParam(10, strparam10.GetString());
					entry.SetParam(11, strparam11.GetString());
					entry.SetParam(12, strparam12.GetString());
					entry.SetParam(13, strparam13.GetString());
					entry.SetParam(14, strparam14.GetString());
					entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
					entry.SetParam(15, strparam15.GetString());
					entry.SetParam(16, strparam16.GetString());
					entry.SetParam(17, strparam17.GetString());
					entry.SetParam(18, strparam18.GetString());
					if(entry.GetShortName().length() >0)
						vecPoint.push_back(entry);
					recset.MoveNext();
				} 
				database.Close();   
				return true;
			} 
			else
			{
				OutPutUpdateLog(_T("Open points table fails."));
				nErrCode = 31002;
			}
		}
		catch(CDBException*  e)   
		{   
			wchar_t err[MAX_PATH] = {0};
			e->GetErrorMessage(err,MAX_PATH);

			CString strLog;
			strLog.Format(_T("Read points table fails.(%s)"),err);
			OutPutUpdateLog(strLog);
			nErrCode = e->m_nRetCode;
			return false;
		}
	}
	else
	{
		CDatabase database;   
		CString sDriver = L"Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)";
		CString   sSql;    
		const CString	cFlag(_T("'"));
		try   
		{   
			sSql.Format(L"DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=TRUE;CREATE_DB=\"%s\";DBQ=%s",sDriver,sExcelFile,sExcelFile);   
			if(database.OpenEx(sSql,CDatabase::noOdbcDialog))   
			{   
				CRecordset recset(&database);
				sSql = L"Select * from [Sheet1$]";   
				recset.Open(CRecordset::forwardOnly, sSql, CRecordset::readOnly);
				while(!recset.IsEOF())
				{
					DataPointEntry entry;
					CString strIndex;
					CString strPhysicalID;
					CString strSource;
					CString strRemark;
					CString strUnit;
					CString strRWProperty;
					CString strparam1;
					CString strparam2;
					CString strparam3;
					CString strparam4;
					CString strparam5;
					CString strparam6;
					CString strparam7;
					CString strparam8;
					CString strparam9;
					CString strparam10;
					CString strparam11;
					CString strparam12;
					CString strparam13;
					CString strparam14;
					CString strStoreCycle;
					CString strparam15;
					CString strparam16;
					CString strparam17;
					CString strparam18;
					recset.GetFieldValue(L"pointindex", strIndex);
					recset.GetFieldValue(L"physicalid", strPhysicalID);
					recset.GetFieldValue(L"source", strSource);
					recset.GetFieldValue(L"remark", strRemark);
					recset.GetFieldValue(L"Unit", strUnit);
					recset.GetFieldValue(L"RWProperty", strRWProperty);
					recset.GetFieldValue(L"param1", strparam1);
					recset.GetFieldValue(L"param2", strparam2);
					recset.GetFieldValue(L"param3", strparam3);
					recset.GetFieldValue(L"param4", strparam4);
					recset.GetFieldValue(L"param5", strparam5);
					recset.GetFieldValue(L"param6", strparam6);
					recset.GetFieldValue(L"param7", strparam7);
					recset.GetFieldValue(L"param8", strparam8);
					recset.GetFieldValue(L"param9", strparam9);
					recset.GetFieldValue(L"param10", strparam10);
					recset.GetFieldValue(L"param11", strparam11);
					recset.GetFieldValue(L"param12", strparam12);
					recset.GetFieldValue(L"param13", strparam13);
					recset.GetFieldValue(L"param14", strparam14);
					recset.GetFieldValue(L"storecycle", strStoreCycle);
					DWORD dwColNum = recset.GetODBCFieldCount();
					if (dwColNum > 21)
					{	// compatible old version
						recset.GetFieldValue(L"customName", strparam15);
						recset.GetFieldValue(L"system", strparam16);
						recset.GetFieldValue(L"device", strparam17);
						recset.GetFieldValue(L"type", strparam18);
					}

					if (cFlag == strIndex
						|| cFlag == strPhysicalID
						|| cFlag == strSource
						|| cFlag == strRemark
						|| cFlag == strUnit
						|| cFlag == strRWProperty
						|| cFlag == strparam1
						|| cFlag == strparam2
						|| cFlag == strparam3
						|| cFlag == strparam4
						|| cFlag == strparam5
						|| cFlag == strparam6
						|| cFlag == strparam7
						|| cFlag == strparam8
						|| cFlag == strparam9
						|| cFlag == strparam10
						|| cFlag == strparam11
						|| cFlag == strparam12
						|| cFlag == strparam13
						|| cFlag == strparam14
						|| cFlag == strStoreCycle
						|| cFlag == strparam15
						|| cFlag == strparam16
						|| cFlag == strparam17
						|| cFlag == strparam18)
					{
						database.Close();
						CString strErr;
						CString strNum;
						int nPos = strIndex.Find(_T("."));
						if (nPos != -1) {
							strNum = strIndex.Left(nPos);
						}
						strErr.Format(_T("Index%s，\r\nPoint %s，\r\nParameter contains a single quotation mark,Export termination"), strNum, strPhysicalID);

						CString strLog;
						strLog.Format(_T("Read points table fails.(%s)"),strErr);
						OutPutUpdateLog(strLog);
						nErrCode = 31001;
						return false;
					}

					int nIndex = _wtoi(strIndex);
					entry.SetPointIndex(nIndex);
					entry.SetShortName(strPhysicalID.GetString());

					entry.SetSourceType(strSource.GetString());

					entry.SetDescription(strRemark.GetString());

					entry.SetUnit(strUnit.GetString());

					if(strRWProperty == L"R")
					{
						entry.SetProperty(PLC_READ);
					}
					else if(strRWProperty == L"W")
					{
						entry.SetProperty(PLC_WRITE);
					}
					else if(strRWProperty == L"R/W")
					{
						entry.SetProperty(PLC_PROP_MAX);
					}
					else
					{
						entry.SetProperty(PLC_READ);
					}
					entry.SetParam(1, strparam1.GetString());
					entry.SetParam(2, strparam2.GetString());
					entry.SetParam(3, strparam3.GetString());
					entry.SetParam(4, strparam4.GetString());
					entry.SetParam(5, strparam5.GetString());
					entry.SetParam(6, strparam6.GetString());
					entry.SetParam(7, strparam7.GetString());
					entry.SetParam(8, strparam8.GetString());
					entry.SetParam(9, strparam9.GetString());
					entry.SetParam(10, strparam10.GetString());
					entry.SetParam(11, strparam11.GetString());
					entry.SetParam(12, strparam12.GetString());
					entry.SetParam(13, strparam13.GetString());
					entry.SetParam(14, strparam14.GetString());
					entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
					entry.SetParam(15, strparam15.GetString());
					entry.SetParam(16, strparam16.GetString());
					entry.SetParam(17, strparam17.GetString());
					entry.SetParam(18, strparam18.GetString());
					if(entry.GetShortName().length() >0)
						vecPoint.push_back(entry);
					recset.MoveNext();
				} 
				database.Close();   
				return true;
			} 
			else
			{
				OutPutUpdateLog(_T("Open points table fails."));
				nErrCode = 31002;
			}
		}
		catch(CDBException*  e)   
		{   
			wchar_t err[MAX_PATH] = {0};
			e->GetErrorMessage(err,MAX_PATH);

			CString strLog;
			strLog.Format(_T("Read points table fails(%s)."),err);
			OutPutUpdateLog(strLog);
			nErrCode = e->m_nRetCode;
			return false;
		}
	} 
}

bool CExcelOperator::OutPutUpdateLog( CString strLog )
{
	wstring wstrFileFolder;
	Project::Tools::GetSysPath(wstrFileFolder);
	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wstrFileFolder += L"\\fileupdate";
	if(Project::Tools::FindOrCreateFolder(wstrFileFolder))
	{
		char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
		setlocale( LC_ALL, "chs" );

		CString strOut;
		wstring strTime;
		Project::Tools::SysTime_To_String(sNow,strTime);
		strOut.Format(_T("%s ::%s\n"),strTime.c_str(),strLog);

		CString strLogPath;
		strLogPath.Format(_T("%s\\fileupdate.log"),wstrFileFolder.c_str());

		//记录Log
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			ff.Seek(0,CFile::end);
			ff.WriteString(strOut);
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 
			return true;
		}
		setlocale( LC_CTYPE, old_locale ); 
		free( old_locale ); 
	}
	return false;
}

bool CExcelOperator::ReadFromCSV( CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode )
{
	FILE* pfile = fopen(Project::Tools::WideCharToAnsi(sExcelFile).c_str(), "r");
	if (pfile)
	{
		fseek(pfile,0,SEEK_END);
		u32 dwsize = ftell(pfile);
		rewind(pfile);
		char* filebuffer = new char[dwsize];
		fread(filebuffer, 1, dwsize, pfile);

		std::map<u32, std::string> StringMap;
		char* pBegin = filebuffer;
		char* pEnd = strchr(filebuffer, '\n');
		u32 uiIndex = 1;
		bool bFirst = true;
		while (pEnd != NULL)
		{
			std::string strbuff;
			strbuff.insert(0, pBegin, pEnd-pBegin);
			if (!strbuff.empty())
			{
				RemoveSubStr("\"",strbuff);
				StringMap[uiIndex] = strbuff;
			}
			pBegin = pEnd + 1;
			pEnd = strchr(pEnd + 1, '\n');
			++uiIndex;
		}
		delete[] filebuffer;
		fclose(pfile);

		//
		std::map<std::string, int> mapColumn;
		std::map<u32, std::string>::iterator iter = StringMap.begin();
		if(iter != StringMap.end())
		{
			string strData = iter->second;
			wstring strData_ = Project::Tools::AnsiToWideChar(strData.c_str()).c_str();
			vector<wstring> vecData;
			Project::Tools::SplitStringByChar(strData_.c_str(),L',',vecData);
			for(int i=0; i<vecData.size(); ++i)
			{
				string ColumnName = Project::Tools::WideCharToAnsi(vecData[i].c_str());
				mapColumn[ColumnName] = i;
			}
			++iter;
		}
		std::map<CString,int> mapPointName;
		for (; iter != StringMap.end(); ++iter)
		{	
			string strData = iter->second;
			wstring strData_ = Project::Tools::AnsiToWideChar(strData.c_str()).c_str();
			vector<wstring> vecPointInfo;
			Project::Tools::SplitStringByChar(strData_.c_str(),L',',vecPointInfo);
			if(vecPointInfo.size() < g_Point_Length)
				continue;

			//
			DataPointEntry entry;
			CString strIndex = vecPointInfo[GetColumnIndex(mapColumn,"pointindex")].c_str();
			CString strPhysicalID = vecPointInfo[GetColumnIndex(mapColumn,"physicalid")].c_str();
			std::map<CString,int>::iterator iter = mapPointName.find(strPhysicalID);
			if(iter != mapPointName.end())
				continue;
			mapPointName[strPhysicalID] = 1;
			CString strSource = vecPointInfo[GetColumnIndex(mapColumn,"source")].c_str();
			CString strRemark = vecPointInfo[GetColumnIndex(mapColumn,"remark")].c_str();
			CString strUnit = vecPointInfo[GetColumnIndex(mapColumn,"Unit")].c_str();
			CString strRWProperty = vecPointInfo[GetColumnIndex(mapColumn,"RWProperty")].c_str();
			CString strparam1 = vecPointInfo[GetColumnIndex(mapColumn,"param1")].c_str();
			CString strparam2 = vecPointInfo[GetColumnIndex(mapColumn,"param2")].c_str();
			CString strparam3 = vecPointInfo[GetColumnIndex(mapColumn,"param3")].c_str();
			CString strparam4 = vecPointInfo[GetColumnIndex(mapColumn,"param4")].c_str();
			CString strparam5 = vecPointInfo[GetColumnIndex(mapColumn,"param5")].c_str();
			CString strparam6 = vecPointInfo[GetColumnIndex(mapColumn,"param6")].c_str();
			CString strparam7 = vecPointInfo[GetColumnIndex(mapColumn,"param7")].c_str();
			CString strparam8 = vecPointInfo[GetColumnIndex(mapColumn,"param8")].c_str();
			CString strparam9 = vecPointInfo[GetColumnIndex(mapColumn,"param9")].c_str();
			CString strparam10 = vecPointInfo[GetColumnIndex(mapColumn,"param10")].c_str();
			
			CString strparam15 = vecPointInfo[GetColumnIndex(mapColumn,"customName")].c_str();
			CString strparam16 = vecPointInfo[GetColumnIndex(mapColumn,"system")].c_str();
			CString strparam17 = vecPointInfo[GetColumnIndex(mapColumn,"device")].c_str();
			CString strparam18 = vecPointInfo[GetColumnIndex(mapColumn,"type")].c_str();

			CString strparam11 = vecPointInfo[GetColumnIndex(mapColumn,"param11")].c_str();
			CString strparam12 = vecPointInfo[GetColumnIndex(mapColumn,"param12")].c_str();
			CString strparam13 = vecPointInfo[GetColumnIndex(mapColumn,"param13")].c_str();
			CString strparam14 = vecPointInfo[GetColumnIndex(mapColumn,"param14")].c_str();
			CString strStoreCycle = vecPointInfo[GetColumnIndex(mapColumn,"storecycle")].c_str();

			int nIndex = _wtoi(strIndex);
			entry.SetPointIndex(nIndex);
			entry.SetShortName(strPhysicalID.GetString());
			entry.SetSourceType(strSource.GetString());
			entry.SetDescription(strRemark.GetString());
			entry.SetUnit(strUnit.GetString());

			if(strRWProperty == L"R")
			{
				entry.SetProperty(PLC_READ);
			}
			else if(strRWProperty == L"W")
			{
				entry.SetProperty(PLC_WRITE);
			}
			else if(strRWProperty == L"R/W")
			{
				entry.SetProperty(PLC_PROP_MAX);
			}
			else
			{
				entry.SetProperty(PLC_READ);
			}
			entry.SetParam(1, strparam1.GetString());
			entry.SetParam(2, strparam2.GetString());
			entry.SetParam(3, strparam3.GetString());
			entry.SetParam(4, strparam4.GetString());
			entry.SetParam(5, strparam5.GetString());
			entry.SetParam(6, strparam6.GetString());
			entry.SetParam(7, strparam7.GetString());
			entry.SetParam(8, strparam8.GetString());
			entry.SetParam(9, strparam9.GetString());
			entry.SetParam(10, strparam10.GetString());
			entry.SetParam(11, strparam11.GetString());
			entry.SetParam(12, strparam12.GetString());
			entry.SetParam(13, strparam13.GetString());
			entry.SetParam(14, strparam14.GetString());
			entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
			entry.SetParam(15, strparam15.GetString());
			entry.SetParam(16, strparam16.GetString());
			entry.SetParam(17, strparam17.GetString());
			entry.SetParam(18, strparam18.GetString());
			if(entry.GetShortName().length() >0)
				vecPoint.push_back(entry);

		}
		return true;
	}

	return false;
}

bool CExcelOperator::ReadFromCSV_Comma( CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode )
{
	try
	{
		Csv csv = Csv(Project::Tools::WideCharToAnsi(sExcelFile).c_str());
		//
		if(csv.getRowCount() >0)
		{
			std::map<std::string, int> mapColumn;
			for(int i=0; i<csv.getColumnCount(); ++i)
			{
				string ColumnName = csv[0][i].c_str();
				mapColumn[ColumnName] = i;
			}

			if(csv.getColumnCount() != 25)
				return false;

			std::map<std::string, int>::iterator iterFind = mapColumn.find("pointindex");
			if(iterFind == mapColumn.end())
				return false;

			std::map<CString,int> mapPointName;
			for(int i=1; i<csv.getRowCount(); ++i)
			{
				//每一行
				vector<wstring> vecPointInfo;
				DataPointEntry entry;
				CString strIndex = Project::Tools::AnsiToWideChar(csv[i]["pointindex"].c_str()).c_str();
				CString strPhysicalID = Project::Tools::AnsiToWideChar(csv[i]["physicalid"].c_str()).c_str();
				std::map<CString,int>::iterator iter = mapPointName.find(strPhysicalID);
				if(iter != mapPointName.end())
					continue;
				mapPointName[strPhysicalID] = 1;
				CString strSource = Project::Tools::AnsiToWideChar(csv[i]["source"].c_str()).c_str();
				CString strRemark = Project::Tools::AnsiToWideChar(csv[i]["remark"].c_str()).c_str();
				CString strUnit = Project::Tools::AnsiToWideChar(csv[i]["Unit"].c_str()).c_str();
				CString strRWProperty = Project::Tools::AnsiToWideChar(csv[i]["RWProperty"].c_str()).c_str();
				CString strparam1 = Project::Tools::AnsiToWideChar(csv[i]["param1"].c_str()).c_str();
				CString strparam2 = Project::Tools::AnsiToWideChar(csv[i]["param2"].c_str()).c_str();
				CString strparam3 = Project::Tools::AnsiToWideChar(csv[i]["param3"].c_str()).c_str();
				CString strparam4 = Project::Tools::AnsiToWideChar(csv[i]["param4"].c_str()).c_str();
				CString strparam5 = Project::Tools::AnsiToWideChar(csv[i]["param5"].c_str()).c_str();
				CString strparam6 = Project::Tools::AnsiToWideChar(csv[i]["param6"].c_str()).c_str();
				CString strparam7 = Project::Tools::AnsiToWideChar(csv[i]["param7"].c_str()).c_str();
				CString strparam8 = Project::Tools::AnsiToWideChar(csv[i]["param8"].c_str()).c_str();
				CString strparam9 = Project::Tools::AnsiToWideChar(csv[i]["param9"].c_str()).c_str();
				CString strparam10 = Project::Tools::AnsiToWideChar(csv[i]["param10"].c_str()).c_str();

				CString strparam15 = Project::Tools::AnsiToWideChar(csv[i]["customName"].c_str()).c_str();
				CString strparam16 = Project::Tools::AnsiToWideChar(csv[i]["system"].c_str()).c_str();
				CString strparam17 = Project::Tools::AnsiToWideChar(csv[i]["device"].c_str()).c_str();
				CString strparam18 = Project::Tools::AnsiToWideChar(csv[i]["type"].c_str()).c_str();

				CString strparam11 = Project::Tools::AnsiToWideChar(csv[i]["param11"].c_str()).c_str();
				CString strparam12 = Project::Tools::AnsiToWideChar(csv[i]["param12"].c_str()).c_str();
				CString strparam13 = Project::Tools::AnsiToWideChar(csv[i]["param13"].c_str()).c_str();
				CString strparam14 = Project::Tools::AnsiToWideChar(csv[i]["param14"].c_str()).c_str();
				CString strStoreCycle = Project::Tools::AnsiToWideChar(csv[i]["storecycle"].c_str()).c_str();

				int nIndex = _wtoi(strIndex);
				entry.SetPointIndex(nIndex);
				entry.SetShortName(strPhysicalID.GetString());
				entry.SetSourceType(strSource.GetString());
				entry.SetDescription(strRemark.GetString());
				entry.SetUnit(strUnit.GetString());

				if(strRWProperty == L"R")
				{
					entry.SetProperty(PLC_READ);
				}
				else if(strRWProperty == L"W")
				{
					entry.SetProperty(PLC_WRITE);
				}
				else if(strRWProperty == L"R/W")
				{
					entry.SetProperty(PLC_PROP_MAX);
				}
				else
				{
					entry.SetProperty(PLC_READ);
				}
				entry.SetParam(1, strparam1.GetString());
				entry.SetParam(2, strparam2.GetString());
				entry.SetParam(3, strparam3.GetString());
				entry.SetParam(4, strparam4.GetString());
				entry.SetParam(5, strparam5.GetString());
				entry.SetParam(6, strparam6.GetString());
				entry.SetParam(7, strparam7.GetString());
				entry.SetParam(8, strparam8.GetString());
				entry.SetParam(9, strparam9.GetString());
				entry.SetParam(10, strparam10.GetString());
				entry.SetParam(11, strparam11.GetString());
				entry.SetParam(12, strparam12.GetString());
				entry.SetParam(13, strparam13.GetString());
				entry.SetParam(14, strparam14.GetString());
				entry.SetStoreCycle((POINT_STORE_CYCLE)_wtoi(strStoreCycle));
				entry.SetParam(15, strparam15.GetString());
				entry.SetParam(16, strparam16.GetString());
				entry.SetParam(17, strparam17.GetString());
				entry.SetParam(18, strparam18.GetString());
				if(entry.GetShortName().length() >0)
					vecPoint.push_back(entry);
			}
			return true;
		}
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
	return false;
}

void CExcelOperator::RemoveSubStr( const string& r, string& str )
{
	int rlen = r.length();
	int strlen = str.length();

	if (rlen <= 0 || strlen <=0)
		return;
	int i,j,pos;
	for (i = 0; i < strlen; ++i) {
		pos = i;
		for (j = 0; j < rlen; ++j) {
			if (str[i] != r[j]) {
				break;
			}
			else if ( j == (rlen-1)){
				str.erase(pos,rlen);
				strlen = str.length();// After trimming, the length of string has changed.
				i = -1;
			}
			else
				++i;
		}
	}
}

int CExcelOperator::GetColumnIndex( std::map<std::string, int> map, string strColumnName )
{
	int nIndex = 0;
	std::map<std::string, int>::iterator iter = map.find(strColumnName);
	if(iter != map.end())
	{
		return iter->second;
	}
	return nIndex;
}
