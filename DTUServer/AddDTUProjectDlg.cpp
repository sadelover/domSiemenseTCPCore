// AddDTUProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DTUServer.h"
#include "AddDTUProjectDlg.h"
#include "afxdialogex.h"


// CAddDTUProjectDlg dialog

IMPLEMENT_DYNAMIC(CAddDTUProjectDlg, CDialog)

CAddDTUProjectDlg::CAddDTUProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddDTUProjectDlg::IDD, pParent)
	, m_strDTUName(_T(""))
	, m_strDTURemark(_T(""))
	, m_strHost(_T(""))
	, m_strUser(_T(""))
	, m_strPsw(_T(""))
	, m_strDataBase(_T(""))
	, m_bSendCmd(FALSE)
{

}

CAddDTUProjectDlg::~CAddDTUProjectDlg()
{
}

void CAddDTUProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DTU_NAME, m_strDTUName);
	DDX_Text(pDX, IDC_EDIT_DTU_REMARK, m_strDTURemark);
	DDX_Text(pDX, IDC_EDIT_IP, m_strHost);
	DDX_Text(pDX, IDC_EDIT_USER, m_strUser);
	DDX_Text(pDX, IDC_EDIT_PSW, m_strPsw);
	DDX_Text(pDX, IDC_EDIT_SCHEMA, m_strDataBase);
	DDX_Check(pDX, IDC_CHECK_SEND, m_bSendCmd);
}


BEGIN_MESSAGE_MAP(CAddDTUProjectDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAddDTUProjectDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddDTUProjectDlg message handlers


void CAddDTUProjectDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_strDTUName.GetLength() <= 0 && m_strHost.GetLength() <= 0 && m_strDataBase.GetLength() <= 0)
	{
		MessageBox(_T("DTU名字，数据库地址或数据库名不能为空"));
		return;
	}


	CDialog::OnOK();
}
