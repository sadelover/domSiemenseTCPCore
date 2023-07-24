// DTUProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DTUServer.h"
#include "DTUProjectDlg.h"
#include "afxdialogex.h"
#include "TinyXml/tinyxml.h"
#include "TinyXml/tinystr.h"
#include "AddDTUProjectDlg.h"
#include "DataHandler.h"

// CDTUProjectDlg dialog

IMPLEMENT_DYNAMIC(CDTUProjectDlg, CDialog)

CDTUProjectDlg::CDTUProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDTUProjectDlg::IDD, pParent)
	, m_strFilePath(_T(""))
{
	m_strDTUServerIP = "";
}

CDTUProjectDlg::~CDTUProjectDlg()
{
}

void CDTUProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROJECT, m_listProject);
}

BEGIN_MESSAGE_MAP(CDTUProjectDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_NEW, &CDTUProjectDlg::OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, &CDTUProjectDlg::OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CDTUProjectDlg::OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CDTUProjectDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_DB, &CDTUProjectDlg::OnBnClickedButtonCreateDb)
END_MESSAGE_MAP()


// CDTUProjectDlg message handlers


void CDTUProjectDlg::OnBnClickedButtonNew()
{
	// TODO: Add your control notification handler code here
	CAddDTUProjectDlg dlg;
	if(dlg.DoModal() == IDOK)
	{
		mapentry entry;
		entry.bSendData = dlg.m_bSendCmd;
		entry.databasename = Project::Tools::WideCharToAnsi(dlg.m_strDataBase.GetString());
		entry.dtuname = Project::Tools::WideCharToAnsi(dlg.m_strDTUName.GetString());
		entry.ip = Project::Tools::WideCharToAnsi(dlg.m_strHost.GetString());
		entry.psw = Project::Tools::WideCharToAnsi(dlg.m_strPsw.GetString());
		entry.user = Project::Tools::WideCharToAnsi(dlg.m_strUser.GetString());
		entry.remark = Project::Tools::WideCharToAnsi(dlg.m_strDTURemark.GetString());
		m_vecProject.push_back(entry);

		RefreshList();
	}
}

void CDTUProjectDlg::OnBnClickedButtonEdit()
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_listProject.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(_T("No items were selected!\n"));
		return;
	}

	if(pos)
	{
		int nItem = m_listProject.GetNextSelectedItem(pos);
		CAddDTUProjectDlg dlg;
		CString strDTUName = m_listProject.GetItemText(nItem,0);
		dlg.m_strDTUName  = strDTUName;
		dlg.m_strDTURemark  = m_listProject.GetItemText(nItem,1);
		dlg.m_strHost  = m_listProject.GetItemText(nItem,2);
		dlg.m_strUser  = m_listProject.GetItemText(nItem,3);
		dlg.m_strPsw  = m_listProject.GetItemText(nItem,4);
		dlg.m_strDataBase  = m_listProject.GetItemText(nItem,5);
		dlg.m_bSendCmd = FALSE;
		if(m_listProject.GetItemText(nItem,6) == _T("是"))
		{
			dlg.m_bSendCmd = TRUE;
		}
		if(dlg.DoModal() == IDOK)
		{
			string strDTUName_Ansi = Project::Tools::WideCharToAnsi(strDTUName.GetString());
			int nNum = -1;
			for(int i=0; i<m_vecProject.size(); ++i)
			{
				if(m_vecProject[i].dtuname == strDTUName_Ansi)
				{
					nNum = i;
					break;
				}
			}

			if(nNum >=0)
			{
				m_vecProject[nNum].bSendData = dlg.m_bSendCmd;
				m_vecProject[nNum].databasename = Project::Tools::WideCharToAnsi(dlg.m_strDataBase.GetString());
				m_vecProject[nNum].dtuname = Project::Tools::WideCharToAnsi(dlg.m_strDTUName.GetString());
				m_vecProject[nNum].ip = Project::Tools::WideCharToAnsi(dlg.m_strHost.GetString());
				m_vecProject[nNum].psw = Project::Tools::WideCharToAnsi(dlg.m_strPsw.GetString());
				m_vecProject[nNum].user = Project::Tools::WideCharToAnsi(dlg.m_strUser.GetString());
				m_vecProject[nNum].remark = Project::Tools::WideCharToAnsi(dlg.m_strDTURemark.GetString());
			}

			RefreshList();
		}		
	}
}


void CDTUProjectDlg::OnBnClickedButtonDelete()
{
	// TODO: Add your control notification handler code here
	vector<int> vec;
	POSITION pos = m_listProject.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(_T("No items were selected!\n"));
		return;
	}
	while(pos)
	{
		int nItem = m_listProject.GetNextSelectedItem(pos);
		CString strDTUName = m_listProject.GetItemText(nItem,0);
		string strDTUName_Ansi = Project::Tools::WideCharToAnsi(strDTUName.GetString());

		for(vector<mapentry>::iterator iter=m_vecProject.begin(); iter!=m_vecProject.end(); )
		{
			if( iter->dtuname == strDTUName_Ansi)
			{   
				m_vecProject.erase(iter);
				break;
			}
			iter++;
		}
	}
	RefreshList();
}

void CDTUProjectDlg::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	Save(m_strFilePath.GetString());
}

bool CDTUProjectDlg::Read( const std::wstring& filename )
{
	m_vecProject.clear();
	TiXmlDocument xmlDoc;
	const string strFileName = Project::Tools::WideCharToAnsi(filename.c_str());;
	const bool ifload = xmlDoc.LoadFile(strFileName.c_str() );
	if(!ifload)
		return false;

	TiXmlElement* pElemRoot = xmlDoc.RootElement();  //project
	ASSERT(pElemRoot);
	if(!pElemRoot)
		return false;
	m_strDTUServerIP = 	pElemRoot->Attribute("dscip");

	TiXmlElement* pElemProject = pElemRoot->FirstChildElement("project");
	while(pElemProject)
	{
		string strDTUName,strDTUReamark,strPsw,strUser,strDBName,strHost;
		int nPort = 3306;
		int nSendData = 0;
		mapentry project;
		project.dtuname = pElemProject->Attribute("dtuname");
		project.ip = pElemProject->Attribute("dbip");
		project.user = pElemProject->Attribute("dbuser");
		project.databasename = pElemProject->Attribute("dbname");
		project.psw = pElemProject->Attribute("dbpsw");
		project.remark = pElemProject->Attribute("dtuRemark");
		pElemProject->Attribute("port",&nPort);
		pElemProject->Attribute("bSendData",&nSendData);
		project.bSendData = nSendData;
		m_vecProject.push_back(project);
		pElemProject = pElemProject->NextSiblingElement();
	}

	return true;
}

bool CDTUProjectDlg::Save( const std::wstring& filename )
{
	// first , new doc object
	// new root node;
	TiXmlDocument* pdocument = new TiXmlDocument;
	TiXmlDeclaration tides("1.0", "GB2312", "yes");
	pdocument->InsertEndChild(tides);

	TiXmlElement xmlroot("DTUServerprojects");
	xmlroot.SetAttribute("dscip", m_strDTUServerIP.c_str());
	for(int i=0; i<m_vecProject.size(); ++i)
	{
		mapentry entry = m_vecProject[i];
		TiXmlElement xmlproject("project");
		xmlproject.SetAttribute("dtuname", entry.dtuname.c_str());
		xmlproject.SetAttribute("dbip", entry.ip.c_str());
		xmlproject.SetAttribute("dbuser", entry.user.c_str());
		xmlproject.SetAttribute("dbname", entry.databasename.c_str());
		xmlproject.SetAttribute("dbpsw", entry.psw.c_str());
		xmlproject.SetAttribute("dtuRemark", entry.remark.c_str());
		xmlproject.SetAttribute("port", "3306");
		CString str;
		str.Format(_T("%d"),entry.bSendData);
		xmlproject.SetAttribute("bSendData", Project::Tools::WideCharToAnsi(str).c_str());

		xmlroot.InsertEndChild(xmlproject);
	}
	pdocument->InsertEndChild(xmlroot);

	string filename_ansi = Project::Tools::WideCharToAnsi(filename.c_str());
	bool bret = pdocument->SaveFile(filename_ansi.c_str());

	return bret;
}

BOOL CDTUProjectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitList();
	Project::Tools::GetSysPath(m_strFilePath);
	m_strFilePath = m_strFilePath + L"\\DTUServerConfig.dsc";
	Read(m_strFilePath.GetString());
	RefreshList();
	return TRUE;
}

bool CDTUProjectDlg::InitList()
{
	m_listProject.SetExtendedStyle(LVS_EX_GRIDLINES |LVS_EX_FULLROWSELECT|LVS_EDITLABELS); 
	m_listProject.InsertColumn(0,L"DTU名称",LVCFMT_LEFT,100,0);
	m_listProject.InsertColumn(1,L"DTU注释",LVCFMT_LEFT,100,0);
	m_listProject.InsertColumn(2,L"DBHost",LVCFMT_LEFT,100,0); 
	m_listProject.InsertColumn(3,L"DBUser",LVCFMT_LEFT,50,0); 
	m_listProject.InsertColumn(4,L"DBPsw",LVCFMT_LEFT,100,0); 
	m_listProject.InsertColumn(5,L"DBName",LVCFMT_LEFT,100,0); 
	m_listProject.InsertColumn(6,L"SendCmd",LVCFMT_LEFT,50,0); 
	m_listProject.ShowWindow(SW_SHOW);

	return TRUE;
}

bool CDTUProjectDlg::RefreshList()
{
	m_listProject.DeleteAllItems();
	for(int i=0; i<m_vecProject.size(); ++i)
	{
		mapentry entry = m_vecProject[i];
		m_listProject.InsertItem(i, L"");
		m_listProject.SetItemText(i,0, Project::Tools::AnsiToWideChar(entry.dtuname.c_str()).c_str());
		m_listProject.SetItemText(i,1, Project::Tools::AnsiToWideChar(entry.remark.c_str()).c_str());
		m_listProject.SetItemText(i,2, Project::Tools::AnsiToWideChar(entry.ip.c_str()).c_str());
		m_listProject.SetItemText(i,3, Project::Tools::AnsiToWideChar(entry.user.c_str()).c_str());
		m_listProject.SetItemText(i,4, Project::Tools::AnsiToWideChar(entry.psw.c_str()).c_str());
		m_listProject.SetItemText(i,5, Project::Tools::AnsiToWideChar(entry.databasename.c_str()).c_str());

		CString strSend = _T("否");
		if(entry.bSendData)
			strSend = _T("是");
		m_listProject.SetItemText(i,6, strSend);
	}

	return true;
}

void CDTUProjectDlg::OnBnClickedButtonCreateDb()
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_listProject.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		AfxMessageBox(_T("No items were selected!\n"));
		return;
	}

	if(pos)
	{
		int nItem = m_listProject.GetNextSelectedItem(pos);
		CString strHost = m_listProject.GetItemText(nItem,2);
		CString strUser  = m_listProject.GetItemText(nItem,3);
		CString strPsw  = m_listProject.GetItemText(nItem,4);
		CString strDBName   = m_listProject.GetItemText(nItem,5);

		mapentry entry;
		entry.databasename = Project::Tools::WideCharToAnsi(strDBName.GetString());
		entry.ip = Project::Tools::WideCharToAnsi(strHost.GetString());
		entry.psw = Project::Tools::WideCharToAnsi(strPsw.GetString());
		entry.user = Project::Tools::WideCharToAnsi(strUser.GetString());

		CDataHandler* pDataHandle = new CDataHandler();
		if(pDataHandle != NULL)
		{
			bool bExist = false;
			bool bCreate = true;
			if(pDataHandle->CheckDBExist(entry))
			{
				bExist = true;
			}

			if(bExist)
			{
				CString strInfo;
				strInfo.Format(_T("数据库%s已存在是否重建"),strDBName);
				if(MessageBox(strInfo,_T("警告"),MB_ICONEXCLAMATION|MB_OKCANCEL)==IDCANCEL)
				{
					bCreate = false;
				}
			}

			if(bCreate)
			{
				pDataHandle->ReBuildDatabase(entry);

				CString strInfo;
				strInfo.Format(_T("数据库%s重建成功"),strDBName);
				MessageBox(strInfo);
			}

			delete pDataHandle;
			pDataHandle = NULL;
		}
	}
}
