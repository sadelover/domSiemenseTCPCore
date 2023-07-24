#include "StdAfx.h"
#include "BEOPLogicManager.h"
#include "sstream"
#include "VECTOR"
#include "LogicBase.h"
#include "../BEOPDataPoint/sqlite/SqliteAcess.h"
#include "../Tools/Util/UtilString.h"
#include "LogicDllThread.h"
#include "LogicParameterConst.h"
#include "LogicParameterLink.h"
#include "LogicParameterPoint.h"
#include "LogicPipeline.h"

CBEOPLogicManager::CBEOPLogicManager(wstring strDBFileName,CWnd *pWnd)
	:m_pWnd(pWnd)
	,m_strDBFileName(strDBFileName)
{
	m_calcinternal = 10;
	m_vecSelDllName.clear();
	vecImportDLLList.clear();
	m_bExitThread = false;
	m_bThreadExitFinished = false;
	m_pDataAccess = NULL;
	m_iDeleteItem   = -1;
	//添加版本号
	m_pSchedule = NULL;

}


CBEOPLogicManager::~CBEOPLogicManager(void)
{

}


bool CBEOPLogicManager::Exit()
{
	//save running state into ini
	bool bSuccess = true;
	unsigned int i = 0;
	for(i=0;i<vecImportDLLList.size();i++)
	{
		if(!vecImportDLLList[i]->Exit())
			bSuccess = false;
	}

	for(i=0;i<vecImportDLLList.size();i++)
	{
		SAFE_DELETE(vecImportDLLList[i]);
	}
	vecImportDLLList.clear();

	for(i=0;i<m_vecDllThreadList.size();i++)
	{
		SAFE_DELETE(m_vecDllThreadList[i]);
	}
	m_vecDllThreadList.clear();

	m_calcinternal = 10;
	m_vecSelDllName.clear();
	m_pDataAccess = NULL;

	if(m_pSchedule)
	{
		m_pSchedule->Exit();
		delete m_pSchedule;
		m_pSchedule = NULL;
	}
	return bSuccess;
}


bool CBEOPLogicManager::StoreDllToDB(CDLLObject* dllOb, wstring strDllPath)
{
	FILE *fp;
	long filesize = 0;
	char * ffile = NULL;

	_bstr_t tem_des = strDllPath.c_str();


	char* des = (char*)tem_des;
	fopen_s(&fp,des, "rb");

	if(fp != NULL)
	{
		//计算文件的大小
		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		//读取文件
		ffile = new char[filesize+1];
		size_t sz = fread(ffile, sizeof(char), filesize, fp);
		fclose(fp);
	}

	if (ffile != NULL)
	{
		InsertDLLtoDB(dllOb,ffile,filesize);
		delete [] ffile;
		return true;
	}

	return false;
}


bool CBEOPLogicManager::InsertDLLtoDB(CDLLObject* dllOb,char *pBlock,int nSize)
{

	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);

	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		PrintLog(L"ERROR: Open S3DB File Failed!\r\n");
	}
	char szSQL[1024] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	int nID = 1;
	wstring importtime = dllOb->GetDllImportTime();
	wstring author = dllOb->GetDllAuthor();
	wstring dllname = dllOb->GetDllName();

	wstring importime_ = importtime;
	_bstr_t tem_des = importime_.c_str();
	char* des = (char*)tem_des;
	std::string importtime__(des);

	wstring dllname_ = dllname;
	tem_des = dllname_.c_str();
	des = (char*)tem_des;
	std::string dllname__(des);

	wstring author_ = author;
	tem_des = author_.c_str();
	des = (char*)tem_des;
	std::string author__(des);

	//dll版本
	wstring version_ = dllOb->GetDllVersion();
	tem_des = version_.c_str();
	des = (char*)tem_des;
	std::string version__(des);
	//dll描述
	wstring description_ = dllOb->GetDllDescription();
	tem_des = description_.c_str();
	des = (char*)tem_des;
	std::string description__(des);
	//dll original name
	wstring strOriginalName = dllOb->GetOriginalDllName();
	int nDot = (strOriginalName.find(L".dll"));
	if(nDot>0)
		strOriginalName = strOriginalName.substr(0, nDot);


	tem_des = strOriginalName.c_str();
	des = (char*)tem_des;
	std::string strOriginalDllName__(des);

	wstring threadname_ = dllOb->GetDllThreadName();
	tem_des = threadname_.c_str();
	des = (char*)tem_des;
	std::string threadname__(des);
	//////////////////////////////////////////////////////////////////////////
	//删除已有的同名dll
	string sql_del;
	ostringstream sqlstream_del;
	sqlstream_del << "delete from list_dllstore where DllName = "<< "'" << dllname__ << "';"; 
	sql_del = sqlstream_del.str();
	if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
	{
		;
	}
	//////////////////////////////////////////////////////////////////////////
	int runstatus = 1;
	string sql_;
	ostringstream sqlstream;
	string em = "";
	sqlstream << "insert into list_dllstore(id,DllName,importtime,author,periodicity,dllcontent,runstatus,unitproperty01,unitproperty02,unitproperty03,unitproperty04,unitproperty05)  values('" << nID<< "','" << dllname__ << "','" \
		<< importtime__ << "','" << author__ << "','" << dllOb->GetTimeSpan() << "',?,'"<<runstatus<<"','"<<version__ <<"','"<<description__<<"','"<<strOriginalDllName__<<"','"<<threadname__<<"','"<<em<<"')";
	sql_ = sqlstream.str();

	if (SQLITE_OK == access.SqlQuery(sql_.c_str()))
	{
		if (access.m_stmt)
		{
			rc = access.SqlBind_Blob(pBlock,1,nSize);
			assert(0 == rc);
			rc = access.SqlNext();
			//assert(0 == rc);
			rc = access.SqlFinalize();
			assert(0 == rc);
		}
	}

	access.CloseDataBase();

	return true;
}

bool CBEOPLogicManager::ThreadExitFinish()
{
	return m_bThreadExitFinished;
}
bool CBEOPLogicManager::SaveSpanTimetoDB(wstring DllName,double SpanTime)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	string sql_del;
	ostringstream sqlstream_del;
	wstring dllname_ = DllName;
	_bstr_t tem_des = dllname_.c_str();
	char* des = (char*)tem_des;
	std::string sdllname(des);
	int runstatus = 0;
	sqlstream_del << "update list_dllstore set periodicity ="<< SpanTime <<",runstatus = "<< runstatus <<" where DllName = "<< "'" << sdllname << "';"; 
	sql_del = sqlstream_del.str();
	access.SqlExe(sql_del.c_str());

	access.CloseDataBase();
	return true;
}

bool   CBEOPLogicManager::SetTimeSpan(CDLLObject *pObject, double fSpanTime)
{
	bool bFindThread = false;
	unsigned int i=0;
	for(i=0;i<m_vecDllThreadList.size();i++)
	{
		if(m_vecDllThreadList[i]->GetName()==pObject->GetDllThreadName())
		{
			bFindThread = true;
			break;
		}

	}

	if(bFindThread)
	{
		CLogicDllThread *pDllThread = m_vecDllThreadList[i];
		for(int j=0;j<pDllThread->GetPipelineCount();j++)
		{
			CLogicPipeline *pPipeLine = pDllThread->GetPipeline(j);
			if(pPipeLine->FindDllObject(pObject))
			{
				pPipeLine->SetTimeSpanSeconds(fSpanTime);
				return true;
			}
		}
	}
	return false;
}

bool CBEOPLogicManager::UpdateDLLfromDB(const wstring &s3dbpath,  CDLLObject *pDllOb)
{
	if(pDllOb==NULL)
		return false;

	if(pDllOb->GetDllName().length()<=0)
		return false;

	string strUtf8;
	Project::Tools::WideCharToUtf8(s3dbpath, strUtf8);
	CSqliteAcess access(strUtf8);

	string strNameUtf8;
	Project::Tools::WideCharToUtf8(pDllOb->GetDllName(), strNameUtf8);
	ostringstream sqlstream;
	sqlstream << "select * from list_dllstore where DllName = '"<< strNameUtf8 <<"';";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	if(SQLITE_ROW != access.SqlNext())
	{
		return false;
	}

	int id = access.getColumn_Int(0);
	wstring dllname		= L"";
	wstring importtime  = L"";
	wstring author      = L"";
	wstring version		= L"";
	wstring description = L"";
	wstring dlloriginname = L"";
	wstring threadname = L"default";

	if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
	{
		string   name_temp(const_cast<char*>(access.getColumn_Text(1)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(name_temp);
		dllname = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
	{
		string   t_temp(const_cast<char*>(access.getColumn_Text(2)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(t_temp);
		importtime = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
	{
		string   a_temp(const_cast<char*>(access.getColumn_Text(3)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(a_temp);
		author = ss.c_str();
	}

	double timespan = access.getColumn_Double(4);

	const void* pf = access.getColumn_Blob(5);
	int nSize      = access.getColumn_Bytes(5);	
	int runstatus  = access.getColumn_Int(6);


	if (const_cast<char*>(access.getColumn_Text(7)) !=NULL)
	{
		string   v_temp(const_cast<char*>(access.getColumn_Text(7)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(v_temp);
		version = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(8)) !=NULL)
	{
		string   d_temp(const_cast<char*>(access.getColumn_Text(8)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
		description = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(9)) !=NULL)
	{
		string   d_temp(const_cast<char*>(access.getColumn_Text(9)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
		dlloriginname = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(10)) !=NULL)
	{
		string   d_temp(const_cast<char*>(access.getColumn_Text(10)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
		threadname = ss.c_str();
		if(threadname.length()<=0)
			threadname = L"default";
	}

	pDllOb->SetDllName(dllname.c_str());
	pDllOb->SetDllAuthor(author.c_str());
	pDllOb->SetDllImportTime(importtime.c_str());
	pDllOb->SetTimeSpan(timespan);
	pDllOb->SetAvailable(false);
	pDllOb->SetDllVersion(version.c_str());
	pDllOb->SetDllDescription(description.c_str());
	pDllOb->SetDllThreadName(threadname);
	pDllOb->SetOriginalDllName(dlloriginname+L".dll");

	//////////////////////////////////////////////////////////////////////////
	const char* pff = (char*)pf;
	//从数据库读取文件
	bool bCopyToDisk = false;
	if (pff != NULL && nSize >0)
	{
		bCopyToDisk = SaveMemToFile(pff,nSize,dllname,pDllOb);
	}
	
	access.SqlFinalize();
	access.CloseDataBase();
	return bCopyToDisk;
}

bool CBEOPLogicManager::ReadDLLfromDB(const wstring &s3dbpath,std::vector<CDLLObject*> &dllObList)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(s3dbpath, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	sqlstream << "select * from list_dllstore;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		int id = access.getColumn_Int(0);
		wstring dllname		= L"";
		wstring importtime  = L"";
		wstring author      = L"";
		wstring version		= L"";
		wstring description = L"";
		wstring dlloriginname = L"";
		wstring threadname = L"default";

		if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
		{
			string   name_temp(const_cast<char*>(access.getColumn_Text(1)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(name_temp);
			dllname = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(2)) !=NULL)
		{
			string   t_temp(const_cast<char*>(access.getColumn_Text(2)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(t_temp);
			importtime = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(3)) !=NULL)
		{
			string   a_temp(const_cast<char*>(access.getColumn_Text(3)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(a_temp);
			author = ss.c_str();
		}

		double timespan = access.getColumn_Double(4);

		const void* pf = access.getColumn_Blob(5);
		int nSize      = access.getColumn_Bytes(5);	
		int runstatus  = access.getColumn_Int(6);


		if (const_cast<char*>(access.getColumn_Text(7)) !=NULL)
		{
			string   v_temp(const_cast<char*>(access.getColumn_Text(7)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(v_temp);
			version = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(8)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(8)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
			description = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(9)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(9)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
			dlloriginname = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(10)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(10)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
			threadname = ss.c_str();
			if(threadname.length()<=0)
				threadname = L"default";
		}

		CDLLObject* pDllOb = new CDLLObject(m_pDataAccess);

		pDllOb->SetDllName(dllname.c_str());
		pDllOb->SetDllAuthor(author.c_str());
		pDllOb->SetDllImportTime(importtime.c_str());
		pDllOb->SetTimeSpan(timespan);
		pDllOb->SetAvailable(false);
		pDllOb->SetDllVersion(version.c_str());
		pDllOb->SetDllDescription(description.c_str());
		pDllOb->SetDllThreadName(threadname);
		pDllOb->SetOriginalDllName(dlloriginname+L".dll");

		dllObList.push_back(pDllOb);

		//////////////////////////////////////////////////////////////////////////
		const char* pff = (char*)pf;
		//从数据库读取文件
		if (pff != NULL && nSize >0)
		{
			SaveMemToFile(pff,nSize,dllname,pDllOb);
		}
	}
	access.SqlFinalize();
	access.CloseDataBase();

	return true;

}

bool CBEOPLogicManager::UpdateDllObjectByFile(wstring strDllOriginalName, wstring strFilePath)
{
	for(int i=0;i<vecImportDLLList.size();i++)
	{
		
		CDLLObject *pDllObject = vecImportDLLList[i];
		wstring strOriginname = pDllObject->GetOriginalDllName();


		if(pDllObject && strOriginname== strDllOriginalName)
		{
			//unload library
			pDllObject->UnLoad();
			

			if(!StoreDllToDB(pDllObject, strFilePath))
				return false;

			if(!UpdateDLLfromDB(m_strDBFileName,pDllObject))
				return false;

			//load new
			if(!pDllObject->Load(pDllObject->GetDllPath()))
				return false;
			
		}

		
	}
	return true;
}

BOOL CBEOPLogicManager::SaveMemToFile( const char* pBlock, const int nSize, wstring szFileName,CDLLObject* pDllOb)
{
	wstring  tempPath;
	Project::Tools::GetSysPath(tempPath);
	tempPath += L"\\importdll";

	const wchar_t* lpszFolderPath =  tempPath.c_str();
	if (!FolderExist(lpszFolderPath))
	{
		BOOL bCreateDir = CreateFolder(lpszFolderPath);
		if(!bCreateDir)
		{
			PrintLog(L"ERROR: SaveMemToFile CreateFolder failed.\r\n");
			return FALSE;
		}
	}
	wstring strFileName;
	wchar_t wcChar[1024];
	wsprintf(wcChar, L"%s\\%s", lpszFolderPath,szFileName.c_str());
	strFileName = wcChar;

	if (FileExist(strFileName))
	{
		BOOL bDelted = DeleteFile(strFileName.c_str());
		if(!bDelted)
		{
			PrintLog(L"ERROR: SaveMemToFile DeleteFile failed.\r\n");
			return FALSE;
		}
	}
	pDllOb->SetDllPath(strFileName);
	//写入文件
	FILE* pFile = NULL;
	if(_wfopen_s(&pFile, strFileName.c_str(), L"wb")==0)
	{
		fseek(pFile, 0, SEEK_SET);
		fwrite(pBlock, nSize, 1, pFile);
		fclose(pFile);
	}
	else
	{
		PrintLog(L"ERROR: SaveMemToFile _wfopen_s failed.\r\n");
		return FALSE;	}


	return TRUE;
}



bool CBEOPLogicManager::SaveParametertoDB(int nInOrOutput, wstring strDllName,  wstring strVariableName, wstring strLinkDefine, wstring strLinkType)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	if (!access.GetOpenState())
	{
		return false;
	}

	//dll name
	wstring dllname = strDllName;
	_bstr_t tem_des = dllname.c_str();
	char* des = (char*)tem_des;
	std::string dllname__(des);


	int nIN = 0;
	bool bAllSuccess = true;
	access.BeginTransaction();
	std::vector<wstring> onerecord;

	string sql_in;
	ostringstream sqlstream;


	//变量名
	wstring vname_ = strVariableName;
	tem_des = vname_.c_str();
	des = (char*)tem_des;
	std::string vname__(des);

	//点名
	wstring pointname_ = strLinkDefine;
	tem_des = pointname_.c_str();
	des = (char*)tem_des;
	std::string pointname__(des);

	//类型
	wstring ptype = strLinkType;
	tem_des = ptype.c_str();
	des = (char*)tem_des;
	std::string ptype__(des);

	std::string em("");

	sqlstream << "update list_paramterConfig set pname = '"<<pointname__<<"', ptype = '"<<ptype__<<"' where vname = '"<< vname__<<"' and DllName = '"<<dllname__ <<"' and INorOut = "<<nInOrOutput<<";";
	sql_in = sqlstream.str();

	int re = access.SqlExe(sql_in.c_str());

	if (re != SQLITE_OK)
	{
		bAllSuccess =false;
	}


	if (access.CommitTransaction() != SQLITE_OK || !bAllSuccess)
	{
		PrintLog(L"ERROR: Edit Dll Fails!\r\n");
	}

	access.CloseDataBase();
	return bAllSuccess;
}

bool CBEOPLogicManager::GetInputParameterfromDB(wstring strDllName,  std::vector<vec_wstr> &inputPara )
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);
	
	if (!access.GetOpenState())
	{
		TRACE(L"Warning:Open S3DB Fail!\r\n");
	}

	//dll name
	wstring dllname = strDllName;
	_bstr_t tem_des = dllname.c_str();
	char* des = (char*)tem_des;
	std::string dllname__(des);

	ostringstream sqlstream;
	sqlstream << "select * from list_paramterConfig where INorOut = 0 and DllName = '"<< dllname__ <<"';";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	if (re != SQLITE_OK)
	{
		TRACE(L"Warning:Read S3db Paramter Config Fail!\r\n");
	}
	inputPara.clear();
	std::vector<wstring> onerecord;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		onerecord.clear();
		if(access.getColumn_Text(0) != NULL)
		{
			string  vname(const_cast<char*>(access.getColumn_Text(0)));
			wstring vname_ = UtilString::ConvertMultiByteToWideChar(vname);
			onerecord.push_back(vname_.c_str());
		}
		if(access.getColumn_Text(1) != NULL)
		{
			string  pname(const_cast<char*>(access.getColumn_Text(1)));
			wstring pname_ = UtilString::ConvertMultiByteToWideChar(pname);
			onerecord.push_back(pname_.c_str());
		}

		if(access.getColumn_Text(2) != NULL)
		{
			string  ptype(const_cast<char*>(access.getColumn_Text(2)));
			wstring ptype_ = UtilString::ConvertMultiByteToWideChar(ptype);
			onerecord.push_back(ptype_.c_str());
		}

		if(access.getColumn_Text(3) != NULL)
		{
			string  pexplain(const_cast<char*>(access.getColumn_Text(3)));
			wstring pexplain_ = UtilString::ConvertMultiByteToWideChar(pexplain);
			onerecord.push_back(pexplain_.c_str());
		}

		inputPara.push_back(onerecord);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	return true;
}

bool CBEOPLogicManager::GetOutputParameterfromDB(wstring strDllName,  std::vector<vec_wstr> &outputPara )
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		TRACE(L"Warning:Open S3DB Fail!\r\n");
	}
	//dll name
	wstring dllname = strDllName;
	_bstr_t tem_des = dllname.c_str();
	char* des = (char*)tem_des;
	std::string dllname__(des);

	ostringstream sqlstream;
	sqlstream << "select * from list_paramterConfig where INorOut = 1 and DllName = '"<< dllname__<<"';";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	if (re != SQLITE_OK)
	{
		TRACE(L"Warning:Read S3db Paramter Config Fail!\r\n");
	}
	outputPara.clear();
	std::vector<wstring> onerecord;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		onerecord.clear();
		if(access.getColumn_Text(0) != NULL)
		{
			string  vname(const_cast<char*>(access.getColumn_Text(0)));
			wstring vname_ = UtilString::ConvertMultiByteToWideChar(vname);
			onerecord.push_back(vname_.c_str());
		}
		if(access.getColumn_Text(1) != NULL)
		{
			string  pname(const_cast<char*>(access.getColumn_Text(1)));
			wstring pname_ = UtilString::ConvertMultiByteToWideChar(pname);
			onerecord.push_back(pname_.c_str());
		}

		if(access.getColumn_Text(2) != NULL)
		{
			string  ptype(const_cast<char*>(access.getColumn_Text(2)));
			wstring ptype_ = UtilString::ConvertMultiByteToWideChar(ptype);
			onerecord.push_back(ptype_.c_str());
		}

		if(access.getColumn_Text(3) != NULL)
		{
			string  pexplain(const_cast<char*>(access.getColumn_Text(3)));
			wstring pexplain_ = UtilString::ConvertMultiByteToWideChar(pexplain);
			onerecord.push_back(pexplain_.c_str());
		}

		if(access.getColumn_Text(6) != NULL)
		{
			string  pexplain(const_cast<char*>(access.getColumn_Text(6)));
			wstring pexplain_ = UtilString::ConvertMultiByteToWideChar(pexplain);
			onerecord.push_back(pexplain_.c_str());
		}

		outputPara.push_back(onerecord);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	return true;
}

bool CBEOPLogicManager::CompareInputParameter( std::vector<vec_wstr> &inputPara ,std::vector<vec_wstr> &Ini_inputPara)
{

	for (int i=0;i<Ini_inputPara.size();++i)
	{
		for (int j=0;j<inputPara.size();++j)
		{
			if (Ini_inputPara[i][0] == inputPara[j][0])
			{
				Ini_inputPara[i] = inputPara[j];
				break;
			}
		}
	}

	return true;
}

bool CBEOPLogicManager::CompareOutputParameter( std::vector<vec_wstr> &outputPara ,std::vector<vec_wstr> &Ini_outputPara)
{

	for (int i=0;i<Ini_outputPara.size();++i)
	{
		for (int j=0;j<outputPara.size();++j)
		{
			if (Ini_outputPara[i][0] == outputPara[j][0])
			{
				Ini_outputPara[i] = outputPara[j];
				break;
			}
		}
	}

	return true;
}
std::vector<vec_wstr> CBEOPLogicManager::SeparateParameter( wstring strLongPatameter )
{
	std::vector<wstring> OneGroup;
	std::vector<vec_wstr> MatrixPara;
	TCHAR Sep = '/';
	wstring P = L"";
	CString cstrLongPatameter = strLongPatameter.c_str();
	for (int i=0;i<cstrLongPatameter.GetLength();++i)
	{
		if (cstrLongPatameter.GetAt(i) == Sep)
		{
			OneGroup.push_back(P);
			P = L"";
			if (OneGroup.size() == 4)
			{
				MatrixPara.push_back(OneGroup);
				OneGroup.clear();
			}
		}else
		{
			CString ii(cstrLongPatameter.GetAt(i));
			P += ii.GetString();
		}
	}

	return MatrixPara;
}

wstring CBEOPLogicManager::LinkParameter( std::vector<vec_wstr> &matrixpara )
{
	wstring strLongPara = L"";
	if (matrixpara.empty())
	{
		return strLongPara;
	}else
	{
		if (matrixpara[0].size() == 4)
		{
			for (int i=0;i<matrixpara.size();++i)
			{
				strLongPara += matrixpara[i][0]+_T("/")+matrixpara[i][1]+_T("/")+matrixpara[i][2]+_T("/")+matrixpara[i][3]+_T("/");
			}
			return strLongPara;

		}else if (matrixpara[0].size() == 5)
		{
			for (int i=0;i<matrixpara.size();++i)
			{
				strLongPara += matrixpara[i][0]+_T("/")+matrixpara[i][1]+_T("/")+matrixpara[i][2]+_T("/")+matrixpara[i][3]+_T("/")+ matrixpara[i][4]+_T("/");
			}
			return strLongPara;
		}
	}

	return strLongPara;
}


CDLLObject * CBEOPLogicManager::GetDLLObjectByName(wstring dllname)
{

	for(int i =0;i<vecImportDLLList.size();i++)
	{
		wstring strComp = vecImportDLLList[i]->GetDllName();
		if(strComp==dllname)
		{

			return vecImportDLLList[i];
		}
	}

	return NULL;
}

bool CBEOPLogicManager::LoadDllDetails()
{
	if (vecImportDLLList.empty())
	{
		return false;
	}
	CDLLObject *pDllOb = NULL;
	for (int k = 0;k < vecImportDLLList.size();++k)
	{
		if(!vecImportDLLList[k]->Load(vecImportDLLList[k]->GetDllPath()))
		{
			return false;
		}

		vecImportDLLList[k]->SetMsgWnd(m_pWnd);
		vecImportDLLList[k]->SetS3DBPath(m_strDBFileName.c_str());

	}
	return true;
}

void CBEOPLogicManager::SortDllName( std::vector<wstring> &iniDllList,std::vector<vec_wstr> &DllList )
{
	if (iniDllList.empty() || DllList.empty())
	{
		return;
	}
	std::vector<vec_wstr> temList(DllList);
	std::vector<int>  label(DllList.size(),1);

	DllList.clear();
	for (int i= 0;i<iniDllList.size();++i)
	{
		for (int j = 0;j<temList.size();++j)
		{
			if (iniDllList[i] == temList[j][0])
			{
				DllList.push_back(temList[j]);
				label[j] = 0;
				break;
			}
		}
	}

	for (int i = 0;i<temList.size();++i)
	{
		if (label[i] == 1)
		{
			DllList.push_back(temList[i]);
		}
	}

	return;
}


BOOL CBEOPLogicManager::CopyDllFile(wstring selFilePath,wstring DllName)
{
	wstring m_wstrFilePath;
	Project::Tools::GetSysPath(m_wstrFilePath);
	m_wstrFilePath += L'\\';
	m_wstrFilePath += L"importdll";
	wstring dec(m_wstrFilePath.c_str());
	if (!FolderExist(dec.c_str()))
	{
		CreateFolder(dec.c_str());
	}
	dec += L"\\";
	dec += DllName;
	CopyFile(selFilePath.c_str(),dec.c_str(),false);

	return TRUE;
}



BOOL CBEOPLogicManager::FolderExist(wstring strPath)
{
	WIN32_FIND_DATA wfd;
	BOOL rValue = FALSE;
	HANDLE hFind = FindFirstFile(strPath.c_str(), &wfd);
	if ((hFind!=INVALID_HANDLE_VALUE) &&
		(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
	{
		rValue = TRUE;
	}
	FindClose(hFind);
	return rValue;
}


BOOL CBEOPLogicManager::CreateFolder(wstring strPath)
{
	SECURITY_ATTRIBUTES attrib;
	attrib.bInheritHandle = FALSE;
	attrib.lpSecurityDescriptor = NULL;
	attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
	return ::CreateDirectory(strPath.c_str(), &attrib);
}

BOOL CBEOPLogicManager::FileExist(wstring strFileName)
{
	CFileFind fFind;
	return fFind.FindFile(strFileName.c_str());
}

int CBEOPLogicManager::GetAllImportDll()
{
	return vecImportDLLList.size();
}


CDLLObject * CBEOPLogicManager::FindDLLObject(wstring strDllName)
{
	for(int i=0;i<vecImportDLLList.size();i++)
	{
		if(vecImportDLLList[i]->GetDllName() == strDllName)
			return vecImportDLLList[i];
	}

	return NULL;
}

bool CBEOPLogicManager::InitRelationships()
{
	//generate all threads
	for (int i =0 ;i<vecImportDLLList.size();++i)
	{
		CDLLObject *curImportDLL = vecImportDLLList[i];
		CString strDllThreadName = curImportDLL->GetDllThreadName().c_str();
		CLogicDllThread *pDllThread = FindDLLThreadByName(strDllThreadName.GetString());
		if(pDllThread==NULL)
		{
			pDllThread = new CLogicDllThread(strDllThreadName.GetString());
			m_vecDllThreadList.push_back(pDllThread);
			pDllThread->AddDll(curImportDLL);

		}
		else
			pDllThread->AddDll(curImportDLL);
	}

	//
	for(int i=0;i<m_vecDllThreadList.size();i++)
	{
		LoadDllThreadRelationship(m_vecDllThreadList[i]);
		m_vecDllThreadList[i]->GeneratePipelines();
	}

	return true;
}



CLogicDllThread * CBEOPLogicManager::FindDLLThreadByName(wstring strDllThreadName)
{
	for(int i=0;i<m_vecDllThreadList.size();i++)
	{
		if(m_vecDllThreadList[i]->GetName()==strDllThreadName)
			return m_vecDllThreadList[i];
	}

	return NULL;
}

bool CBEOPLogicManager::LoadDllThreadRelationship(CLogicDllThread *pDllThread)
{
	int i=0, j=0;
	wstring strDllVersionList = L"";
	for(i=0;i<pDllThread->GetDllCount();i++)
	{
		CDLLObject *pObject=  pDllThread->GetDllObject(i);

		strDllVersionList += pObject->GetDllVersion();
		strDllVersionList += L"|||";

		std::vector<vec_wstr> strParaList;
		pObject->ClearParameters();

		GetInputParameterfromDB(pObject->GetDllName(),strParaList);
		for(j=0;j< strParaList.size();j++)
		{
			CLogicParameter *pp = NULL;
			vec_wstr strPara = strParaList[j];
			if(strPara[2]==L"const" || strPara[2]==L"strconst")
			{
				pp = new CLogicParameterConst(strPara[0], 0,strPara[2], m_pDataAccess, strPara[1]);
			}
			else if(strPara[2]==L"logiclink")
			{
				CString strTT = strPara[1].c_str();
				int nMao = strTT.Find(':');
				if(nMao>=0)
				{
					CString strLinkDllName = strTT.Left(nMao);
					CString strLinkDllPortName = strTT.Mid(nMao+1);
					CDLLObject *pLinkObject = FindDLLObject(strLinkDllName.GetString());
					pp = new CLogicParameterLink(strPara[0],0,strPara[2], m_pDataAccess, pLinkObject, strLinkDllPortName.GetString());
				}

			}
			else
			{
				pp = new CLogicParameterPoint(strPara[0],0,strPara[2], m_pDataAccess, strPara[1]);
			}
			pObject->m_vecCurDllInputParameterList.push_back(pp);
		}

		//read output
		std::vector<vec_wstr> strOutputParaList;
		GetOutputParameterfromDB(pObject->GetDllName(),strOutputParaList);
		for(j=0;j< strOutputParaList.size();j++)
		{
			CLogicParameter *pp = NULL;
			vec_wstr strPara = strOutputParaList[j];
			if(strPara[2]==L"const" || strPara[2]==L"strconst")
			{
				pp = new CLogicParameterConst(strPara[0],1,strPara[2], m_pDataAccess, strPara[1]);
			}
			else if(strPara[2]== L"logiclink")
			{
				CString strPP = strPara[1].c_str();
				int nMao = strPP.Find(':');
				if(nMao>=0)
				{
					CString strLinkDllName = strPP.Left(nMao);
					CString strLinkDllPortName = strPP.Mid(nMao+1);
					CDLLObject *pLinkObject = FindDLLObject(strLinkDllName.GetString());
					pp = new CLogicParameterLink(strPara[0],1,strPara[2], m_pDataAccess, pLinkObject, strLinkDllPortName.GetString());
				}

			}
			else
			{
				pp = new CLogicParameterPoint(strPara[0],1,strPara[2], m_pDataAccess, strPara[1], strPara[4]);
			}
			pObject->m_vecCurDllOutputParameterList.push_back(pp);
		}

	}

	m_pDataAccess->WriteCoreDebugItemValue2(pDllThread->GetName(), strDllVersionList);

	return true;
}

bool CBEOPLogicManager::IfExistDll(const wstring &s3dbpath)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(s3dbpath, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	sqlstream << "  select count(*) from list_dllstore;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	int nCount = 0;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		nCount = access.getColumn_Int(0);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	if(nCount > 0)
		return true;
	return false;
}

bool CBEOPLogicManager::InitSchedule()
{
	if(m_pSchedule == NULL)
	{
		m_pSchedule = new CScheduleThread(m_pDataAccess);
		m_pSchedule->Init();
	}
	return true;
}

void CBEOPLogicManager::PrintLog( const wstring &strLog,bool bSaveLog /*= true*/ )
{
	_tprintf(strLog.c_str());
}
