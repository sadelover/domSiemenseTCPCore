// ValueChangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DTUServer.h"
#include "ValueChangeDlg.h"
#include "afxdialogex.h"


// CValueChangeDlg dialog

IMPLEMENT_DYNAMIC(CValueChangeDlg, CDialog)

CValueChangeDlg::CValueChangeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CValueChangeDlg::IDD, pParent)
	, m_strValue(_T(""))
	, m_strName(_T(""))
{

}

CValueChangeDlg::~CValueChangeDlg()
{
}

void CValueChangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VALUE, m_strValue);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
}


BEGIN_MESSAGE_MAP(CValueChangeDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CValueChangeDlg::OnBnClickedOk)
END_MESSAGE_MAP()

void CValueChangeDlg::SetName( CString strName )
{
	m_strName = strName;
}

void CValueChangeDlg::SetValue( CString strValue )
{
	m_strValue = strValue;
}


// CValueChangeDlg message handlers


void CValueChangeDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
