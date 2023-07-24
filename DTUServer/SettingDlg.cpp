// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DTUServer.h"
#include "SettingDlg.h"
#include "afxdialogex.h"

#include "../Tools/CustomTools/CustomTools.h"
#include "../BEOPDataPoint/sqlite/SqliteAcess.h"
#include "DTUAddressMap.h"

// CSettingDlg dialog

#include <iostream>
#include <sstream>

using std::string;
using std::wstring;

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

const char* sql_createconfigtable =  "('id'  INTEGER,'name'  TEXT(256),'storetable'  TEXT(256),'storeid'  INTEGER,'updatetable'  TEXT(256),'updateid'  INTEGER,'type'  INTEGER,'unit'  TEXT(256),'ch_description'  TEXT(256),'en_description'  TEXT(256),'R_W'  INTEGER,\
	'group'  INTEGER,'calc_method'  TEXT(10000),'Source'  INTEGER DEFAULT '''12''','Param1'  TEXT(75) DEFAULT ''' ''','Param2'  TEXT(75) DEFAULT ''' ''','Param3'  TEXT(75) DEFAULT ''' ''','Param4'  TEXT(75) DEFAULT ''' ''','Param5'  TEXT(75) DEFAULT ''' ''',\
	'Param6'  TEXT(75) DEFAULT ''' ''','Param7'  TEXT(75) DEFAULT ''' ''','Param8'  TEXT(75) DEFAULT ''' ''','Param9'  TEXT(75) DEFAULT ''' ''','Param10'  TEXT(75) DEFAULT ''' ''','SourceType'  VARCHAR(20),'high'  FLOAT,'highhigh'  FLOAT,\
	'low'  FLOAT,'lowlow'  FLOAT,'Param11'  TEXT DEFAULT ''' ''','Param12'  TEXT DEFAULT ''' ''','Param13'  TEXT DEFAULT ''' ''','Param14'  TEXT DEFAULT ''' ''','Param15'  TEXT DEFAULT ''' ''',\
	'Param16'  TEXT DEFAULT ''' ''','Param17'  TEXT DEFAULT ''' ''','Param18'  TEXT DEFAULT ''' ''','Param19'  TEXT DEFAULT ''' ''','Param20'  TEXT DEFAULT ''' ''','Param21'  TEXT DEFAULT ''' ''','Param22'  TEXT DEFAULT ''' ''','Param23'  TEXT DEFAULT ''' ''',\
	'Param24'  TEXT DEFAULT ''' ''','Param25'  TEXT DEFAULT ''' ''','Param26'  TEXT DEFAULT ''' ''','Param27'  TEXT DEFAULT ''' ''','Param28'  TEXT DEFAULT ''' ''','Param29'  TEXT DEFAULT ''' ''','Param30'  TEXT DEFAULT ''' ''','SubType'  VARCHAR(50),PRIMARY KEY ('name' ASC))";

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDlg::IDD, pParent)
	, m_strDTUName(_T(""))
	, m_strDTURemark(_T(""))
	, m_strIP(_T("localhost"))
	, m_strPsw(_T("RNB.beop-2013"))
	, m_strUser(_T("root"))
	, m_strSchema(_T("beop"))
	, m_strDTUConfigTableName(_T(""))
	, m_bEdit(FALSE)
{

}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DTU_NAME, m_strDTUName);
	DDX_Text(pDX, IDC_EDIT_DTU_REMARK, m_strDTURemark);
	DDX_Text(pDX, IDC_EDIT_IP, m_strIP);
	DDX_Text(pDX, IDC_EDIT_PSW, m_strPsw);
	DDX_Text(pDX, IDC_EDIT_USER, m_strUser);
	DDX_Text(pDX, IDC_EDIT_SCHEMA, m_strSchema);
	DDX_Text(pDX, IDC_EDIT_CONFIG_TABLE, m_strDTUConfigTableName);
}

BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CSettingDlg::OnBnClickedButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, &CSettingDlg::OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_CANCLE, &CSettingDlg::OnBnClickedButtonCancle)
END_MESSAGE_MAP()


// CSettingDlg message handlers


void CSettingDlg::OnBnClickedButtonImport()
{
	// TODO: Add your control notification handler code here
	//判断内容是否为空
	UpdateData(TRUE);
	if(m_strDTUConfigTableName.GetLength()<=0)
	{
		MessageBox(_T("请先输入配置表名"));
		return;
	}

	CFileDialog dlgOpenFile(TRUE,NULL,L"*.s3db",OFN_HIDEREADONLY|OFN_HIDEREADONLY,L"*.s3db");
	if(dlgOpenFile.DoModal()==IDOK)
	{
		CString dbName = dlgOpenFile.GetPathName();
		LoadNewPointCongig(dbName);
	}

}

void CSettingDlg::LoadNewPointCongig( CString strNewDBFileName )
{
	//Read list_dllstore
	wstring strNewName = strNewDBFileName.GetString();

	string strUtf8;
	Project::Tools::WideCharToUtf8(strNewName, strUtf8);


	CSqliteAcess access(strUtf8);
	ostringstream sqlstream;
	string sql_;
	char *ex_sql;
	
	//Read list_point
	sqlstream.str("");
	sqlstream << "select * from list_point;";
	sql_ = sqlstream.str();
	ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	m_vecPoint.clear();
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		sPoint sPoint;
		sPoint.nID = access.getColumn_Int(0);
		sPoint.nStoreid = access.getColumn_Int(3);
		sPoint.nUpdateid = access.getColumn_Int(5);
		sPoint.nType = access.getColumn_Int(6);
		sPoint.nR_W = access.getColumn_Int(10);
		sPoint.nGroup = access.getColumn_Int(11);
		sPoint.nSource = access.getColumn_Int(13);
		sPoint.fHigh = access.getColumn_Int(25);
		sPoint.fHighhigh = access.getColumn_Int(26);
		sPoint.fLow = access.getColumn_Int(27);
		sPoint.fLowlow = access.getColumn_Int(28);

		if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
		{
			string   name_temp(const_cast<char*>(access.getColumn_Text(1)));
			sPoint.strName = name_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(2)) !=NULL)
		{
			string   t_temp(const_cast<char*>(access.getColumn_Text(2)));
			sPoint.strStoretable = t_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(4)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(4)));
			sPoint.strUpdatetable = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(7)) !=NULL)
		{
			string   r_temp(const_cast<char*>(access.getColumn_Text(7)));
			sPoint.strUnit = r_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(8)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(8)));
			sPoint.strCh_description = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(9)) !=NULL)
		{
			string   r_temp(const_cast<char*>(access.getColumn_Text(9)));
			sPoint.strEn_description = r_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(12)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(12)));
			sPoint.strCalc_method = d_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(14)) !=NULL)
		{
			string   name_temp(const_cast<char*>(access.getColumn_Text(14)));
			sPoint.strParam1 = name_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(15)) !=NULL)
		{
			string   t_temp(const_cast<char*>(access.getColumn_Text(15)));
			sPoint.strParam2 = t_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(16)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(16)));
			sPoint.strParam3 = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(17)) !=NULL)
		{
			string   r_temp(const_cast<char*>(access.getColumn_Text(17)));
			sPoint.strParam4 = r_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(18)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(18)));
			sPoint.strParam5 = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(19)) !=NULL)
		{
			string   r_temp(const_cast<char*>(access.getColumn_Text(19)));
			sPoint.strParam6 = r_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(20)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(20)));
			sPoint.strParam7 = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(21)) !=NULL)
		{
			string   name_temp(const_cast<char*>(access.getColumn_Text(21)));
			sPoint.strParam8 = name_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(22)) !=NULL)
		{
			string   t_temp(const_cast<char*>(access.getColumn_Text(22)));
			sPoint.strParam9 = t_temp;
		}

		if (const_cast<char*>(access.getColumn_Text(23)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(23)));
			sPoint.strParam10 = d_temp;
		}
		if (const_cast<char*>(access.getColumn_Text(24)) !=NULL)
		{
			string   r_temp(const_cast<char*>(access.getColumn_Text(24)));
			sPoint.strSourceType = r_temp;
		}

		m_vecPoint.push_back(sPoint);	
	}
	access.SqlFinalize();

	//CString strPath = _T("");
	//TCHAR szExePath[MAX_PATH] = {0};
	//::GetModuleFileName(NULL, szExePath,MAX_PATH);

	//strPath = szExePath;
	//strPath = strPath.Left(strPath.ReverseFind('\\'));
	//strPath = strPath + _T("\\beop.s3db");
	//
	//Project::Tools::WideCharToUtf8(strPath.GetString(), strUtf8);
	//CSqliteAcess accessOld(strUtf8);
	////createtable

	//string newtablename;
	//Project::Tools::WideCharToUtf8(m_strDTUConfigTableName.GetString(), newtablename);

	//sqlstream.str("");
	//sqlstream << "create table if not exists " << newtablename <<  sql_createconfigtable;
	//sql_ = sqlstream.str();
	//ex_sql = const_cast<char*>(sql_.c_str());
	//if (SQLITE_OK == accessOld.SqlExe(ex_sql))
	//{
	//	;
	//}

	////删除newtablename
	//sqlstream.str("");
	//sqlstream << "delete from " << newtablename; 
	//sql_ = sqlstream.str();
	//ex_sql = const_cast<char*>(sql_.c_str());
	//if (SQLITE_OK == accessOld.SqlExe(ex_sql))
	//{
	//	;
	//}

	////插入newtablename
	//accessOld.BeginTransaction();
	//for(int i=0; i<vecPoint.size(); i++)
	//{
	//	sqlstream.str("");
	//	sPoint sPoint = vecPoint[i];
	//	sqlstream << "INSERT INTO "<< newtablename << " (id, type, name, SourceType, R_W, ch_description, unit, high, highhigh, low, lowlow, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10) \
	//				values(" << sPoint.nID<< "," << sPoint.nType << ",'"<< sPoint.strName <<"','"<<sPoint.strSourceType <<"',"<<sPoint.nR_W<<",'"<<sPoint.strCh_description<<"','"<<sPoint.strUnit<<"',"<<sPoint.fHigh << "," <<sPoint.fHighhigh << ","<< sPoint.fLow <<","<<sPoint.fLowlow <<",'"<<sPoint.strParam1<<"','"<<sPoint.strParam2<<"','"<<sPoint.strParam3<<"','"<<sPoint.strParam4<<"','"<<sPoint.strParam5<<"','"<<sPoint.strParam6<<"','"<<sPoint.strParam7<<"','"<<sPoint.strParam8<<"','"<<sPoint.strParam9<<"','"<<sPoint.strParam10<<"');";
	//	string sql_in = sqlstream.str();
	//	accessOld.SqlExe(sql_in.c_str());
	//}
	//access.CommitTransaction();
}

void CSettingDlg::SetDTUInfoByDTUName( string strName )
{
	if(strName == "")
	{
		((CButton*)GetDlgItem(IDC_BUTTON_EDIT))->SetWindowText(_T("创建"));
		m_bEdit = FALSE;
	}
	else
	{
		mapentry entry = DTUAddressMap::GetInstance()->GetDTUInfoByName(strName);
		m_strDTUName = entry.dtuname.c_str();
		m_strDTURemark = entry.remark.c_str();
		m_strIP = entry.ip.c_str();
		m_strPsw = entry.psw.c_str();
		m_strUser = entry.user.c_str();
		m_strSchema = entry.databasename.c_str();
		m_strDTUConfigTableName = entry.configpointtable.c_str();
		m_bEdit = TRUE;
		UpdateData(FALSE);
	}	

	m_strDTU = strName;
}

void CSettingDlg::OnBnClickedButtonEdit()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_strDTUName.GetLength()<=0 || m_strDTURemark.GetLength()<=0 || m_strIP.GetLength()<=0 || m_strPsw.GetLength()<=0 || m_strUser.GetLength()<=0 || m_strSchema.GetLength()<=0 || m_strDTUConfigTableName.GetLength()<=0 )
	{
		MessageBox(_T("以上各项都不能为空"));
		return;
	}

	//写到数据库

	WriteS3db(m_strDTU);
}

void CSettingDlg::OnBnClickedButtonCancle()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CSettingDlg::WriteS3db(string strName)
{
	CString strPath = _T("");
	TCHAR szExePath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, szExePath,MAX_PATH);

	strPath = szExePath;
	strPath = strPath.Left(strPath.ReverseFind('\\'));
	strPath = strPath + _T("\\beop.s3db");

	string strUtf8;
	Project::Tools::WideCharToUtf8(strPath.GetString(), strUtf8);
	CSqliteAcess accessOld(strUtf8);

	ostringstream sqlstream;
	string sql_;
	char *ex_sql;

	//修改或添加dtu_config
	if(m_bEdit)		//修改
	{
		sqlstream.str("");
		sqlstream << "update  dtu_config set DTUName ='" << Project::Tools::WideCharToAnsi(m_strDTUName.GetString()) << "',DTURemark = '" <<  Project::Tools::WideCharToAnsi(m_strDTURemark.GetString()) <<"',IP ='" << Project::Tools::WideCharToAnsi(m_strIP.GetString()) <<"',User ='" << Project::Tools::WideCharToAnsi(m_strUser.GetString()) <<"',Psw='" << Project::Tools::WideCharToAnsi(m_strPsw.GetString()) <<"',Schema='" << Project::Tools::WideCharToAnsi(m_strSchema.GetString()) <<"',configpointtable='" << Project::Tools::WideCharToAnsi(m_strDTUConfigTableName.GetString()) <<"' where DTUName ='" << strName << "';";
		sql_ = sqlstream.str();
		ex_sql = const_cast<char*>(sql_.c_str());
		if (SQLITE_OK == accessOld.SqlExe(ex_sql))
		{
			;
		}
	}
	else
	{
		//删除dtu_config
		sqlstream.str("");
		sqlstream << "delete from dtu_config where DTUName ='" << Project::Tools::WideCharToAnsi(m_strDTUName.GetString()) << "';";
		sql_ = sqlstream.str();
		ex_sql = const_cast<char*>(sql_.c_str());
		if (SQLITE_OK == accessOld.SqlExe(ex_sql))
		{
			;
		}


		sqlstream.str("");
		sqlstream << "insert into dtu_config (DTUName,DTURemark,IP,User,Psw,Schema,configpointtable) values ('" << Project::Tools::WideCharToAnsi(m_strDTUName.GetString()) <<"','" <<  Project::Tools::WideCharToAnsi(m_strDTURemark.GetString()) <<"','"<<Project::Tools::WideCharToUtf8(m_strIP.GetString()) <<"','"<<Project::Tools::WideCharToAnsi(m_strUser.GetString()) <<"','"<<Project::Tools::WideCharToAnsi(m_strPsw.GetString()) <<"','"<<Project::Tools::WideCharToAnsi(m_strSchema.GetString()) <<"','"<<Project::Tools::WideCharToAnsi(m_strDTUConfigTableName.GetString()) << "');";
		sql_ = sqlstream.str();
		ex_sql = const_cast<char*>(sql_.c_str());
		if (SQLITE_OK == accessOld.SqlExe(ex_sql))
		{
			;
		}
	}

	//createtable

	string newtablename;
	Project::Tools::WideCharToUtf8(m_strDTUConfigTableName.GetString(), newtablename);

	sqlstream.str("");
	sqlstream << "create table if not exists " << newtablename <<  sql_createconfigtable;
	sql_ = sqlstream.str();
	ex_sql = const_cast<char*>(sql_.c_str());
	if (SQLITE_OK == accessOld.SqlExe(ex_sql))
	{
		;
	}

	//删除newtablename
	sqlstream.str("");
	sqlstream << "delete from " << newtablename; 
	sql_ = sqlstream.str();
	ex_sql = const_cast<char*>(sql_.c_str());
	if (SQLITE_OK == accessOld.SqlExe(ex_sql))
	{
		;
	}

	//插入newtablename
	accessOld.BeginTransaction();
	for(int i=0; i<m_vecPoint.size(); i++)
	{
		sqlstream.str("");
		sPoint sPoint = m_vecPoint[i];
		sqlstream << "INSERT INTO "<< newtablename << " (id, type, name, SourceType, R_W, ch_description, unit, high, highhigh, low, lowlow, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10) \
													  values(" << sPoint.nID<< "," << sPoint.nType << ",'"<< sPoint.strName <<"','"<<sPoint.strSourceType <<"',"<<sPoint.nR_W<<",'"<<sPoint.strCh_description<<"','"<<sPoint.strUnit<<"',"<<sPoint.fHigh << "," <<sPoint.fHighhigh << ","<< sPoint.fLow <<","<<sPoint.fLowlow <<",'"<<sPoint.strParam1<<"','"<<sPoint.strParam2<<"','"<<sPoint.strParam3<<"','"<<sPoint.strParam4<<"','"<<sPoint.strParam5<<"','"<<sPoint.strParam6<<"','"<<sPoint.strParam7<<"','"<<sPoint.strParam8<<"','"<<sPoint.strParam9<<"','"<<sPoint.strParam10<<"');";
		string sql_in = sqlstream.str();
		accessOld.SqlExe(sql_in.c_str());
	}
	accessOld.CommitTransaction();
}
