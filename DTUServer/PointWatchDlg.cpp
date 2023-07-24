// PointWatchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DTUServer.h"
#include "PointWatchDlg.h"
#include "afxdialogex.h"
#include "DTUAddressMap.h"
#include "ValueChangeDlg.h"
//#include "../Tools/CustomTools/CustomTools.h"

// CPointWatchDlg dialog

IMPLEMENT_DYNAMIC(CPointWatchDlg, CDialog)

CPointWatchDlg::CPointWatchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPointWatchDlg::IDD, pParent)
	, m_strNameSearch(_T(""))
	, m_bExitThread(FALSE)
	, m_nCurSearchIndex(-1)
{
	m_strDTUName = "";
}

CPointWatchDlg::~CPointWatchDlg()
{
	m_bExitThread = TRUE;

	WaitForSingleObject(m_hThreadPoint,INFINITE);

	if(m_hThreadPoint)
	{
		CloseHandle(m_hThreadPoint);
		m_hThreadPoint = NULL;
	}
}

void CPointWatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ALL, m_ListAll);
	DDX_Text(pDX, IDC_EDIT_NAME_SEARCH, m_strNameSearch);
}

BEGIN_MESSAGE_MAP(CPointWatchDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_PREV, &CPointWatchDlg::OnBnClickedButtonSearchPrev)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CPointWatchDlg::OnBnClickedButtonSearch)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ALL, &CPointWatchDlg::OnLvnItemchangedListAll)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ALL, &CPointWatchDlg::OnNMDblclkListAll)
END_MESSAGE_MAP()

void CPointWatchDlg::SetPointInfoByDTUName( string strName)
{
	m_strDTUName = strName;
	InitPointByDTUName(m_strDTUName);
}

// CPointWatchDlg message handlers


void CPointWatchDlg::OnBnClickedButtonSearchPrev()
{
	// TODO: Add your control notification handler code here
	
	CString strCurSearchName = m_strNameSearch;

	UpdateData(TRUE);
	if(strCurSearchName!=m_strNameSearch)
		m_nCurSearchIndex = m_vecEntrylist.size()-1;


	if(m_strNameSearch.GetLength()==0)
		return;
	m_strNameSearch.MakeLower();


	for (int i=m_nCurSearchIndex;i>=0;i--)
	{
		const wstring strName =  Project::Tools::AnsiToWideChar(m_vecEntrylist[i].strpointname.c_str());
		CString strCName = strName.c_str();

		strCName.MakeLower();
		if(strCName.Find(m_strNameSearch)>=0)
		{
			m_nCurSearchIndex = i;
			break;
		}
	}

	m_ListAll.EnsureVisible(m_nCurSearchIndex+27, FALSE);
}

void CPointWatchDlg::OnBnClickedButtonSearch()
{
	// TODO: Add your control notification handler code here
	CString strCurSearchName = m_strNameSearch;

	UpdateData(TRUE);
	if(strCurSearchName!=m_strNameSearch)
		m_nCurSearchIndex = -1;

	if(m_strNameSearch.GetLength()==0)
		return;
	m_strNameSearch.MakeLower();

	for (size_t i=m_nCurSearchIndex+1;i<m_vecEntrylist.size();++i)
	{
		const wstring strName =  Project::Tools::AnsiToWideChar(m_vecEntrylist[i].strpointname.c_str());
		CString strCName = strName.c_str();

		strCName.MakeLower();
		if(strCName.Find(m_strNameSearch)>=0)
		{
			m_nCurSearchIndex = i;
			break;
		}
	}

	m_ListAll.EnsureVisible(m_nCurSearchIndex, FALSE);
}

void CPointWatchDlg::OnLvnItemchangedListAll(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CPointWatchDlg::OnNMDblclkListAll(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CString strName = m_ListAll.GetItemText(pNMItemActivate->iItem,1);
	CString strValue = m_ListAll.GetItemText(pNMItemActivate->iItem,2);
	CValueChangeDlg dlg;
	dlg.SetName(strName);
	dlg.SetValue(strValue);
	if(dlg.DoModal() == IDOK)
	{
		//更新output表
		SYSTEMTIME st;
		GetSystemTime(&st);
		string timestring = Project::Tools::SystemTimeToString(st);
		string name = Project::Tools::WideCharToAnsi(strName.GetString());
		string value = Project::Tools::WideCharToAnsi(strValue.GetString());

		CDTUServerHandler* phandle = m_handlelist.GetHandlerByDTUName(m_strDTUName);
		if(phandle != NULL)
		{
			const bool bRes1 = phandle->UpdateOutput(timestring,name,value);
			if (!bRes1)
			{
				AfxMessageBox(L"写数据库失败!");
			}
			else
			{
				m_ListAll.SetItemText(pNMItemActivate->iItem,3,dlg.m_strValue.GetString() );
			}
		}
	}
}

BOOL CPointWatchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ListAll.SetExtendedStyle(LVS_EX_GRIDLINES |LVS_EX_FULLROWSELECT|LVS_EDITLABELS); 
	m_ListAll.InsertColumn(0,L"时间",LVCFMT_LEFT,200,0);
	m_ListAll.InsertColumn(1,L"点名",LVCFMT_LEFT,200,0);
	m_ListAll.InsertColumn(2,L"值",LVCFMT_LEFT,100,0); 
	m_ListAll.ShowWindow(SW_SHOW);

	StartThreads();

	return TRUE;
}

void CPointWatchDlg::RefreshByDTUName( string strName )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	if(strName == "")
		return;

	CDTUServerHandler* phandle = m_handlelist.GetHandlerByDTUName(strName);
	if(phandle != NULL)
	{
		if(phandle->GetIfUpdateData())
		{
			m_vecEntrylist.clear();
			phandle->GetDataEntryList(m_vecEntrylist);

			int nNum = 0;
			CString strPointName, strValue, strTime;
			m_ListAll.DeleteAllItems();
			for (int i =0; i<m_vecEntrylist.size(); i++)
			{
				// Get equipment data point and format.
				spointwatch point = m_vecEntrylist[i];
				strPointName = point.strpointname.c_str();
				strValue = point.strvalue.c_str();
				strTime = point.strtime.c_str();
				m_ListAll.InsertItem(nNum, L"");
				m_ListAll.SetItemText(nNum,0, strTime);
				m_ListAll.SetItemText(nNum,1, strPointName);
				m_ListAll.SetItemText(nNum,2, strValue);
				nNum ++;
			}

			phandle->SetUpdateData(false);
		}
	}
	//UpdateData(FALSE);
}

void CPointWatchDlg::StartThreads()
{
	m_hThreadPoint = CreateThread(NULL, 0, ThreadFuncPoint, this, 0, NULL); 
}

DWORD WINAPI CPointWatchDlg::ThreadFuncPoint( LPVOID lpParameter )
{
	CPointWatchDlg* pDlg = (CPointWatchDlg*)lpParameter;
	while(TRUE)
	{
		Sleep(1000);

		if (pDlg->m_bExitThread)
		{
			return 0;
		}

		if(pDlg->IsWindowVisible()==false)
		{
			continue;
		}

		pDlg->RefreshByDTUName(pDlg->m_strDTUName);
	}
	return 0;
}

void CPointWatchDlg::ExitThreads()
{
	m_bExitThread = true;
}

void CPointWatchDlg::SetDTUServerHandlerPool( DTUServerHandlerPool handlelist )
{
	m_handlelist = handlelist;
}

void CPointWatchDlg::InitPointByDTUName( string strName )
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);

	m_ListAll.DeleteAllItems();

	if(strName == "")
		return;

	CDTUServerHandler* phandle = m_handlelist.GetHandlerByDTUName(strName);
	if(phandle != NULL)
	{		
		m_vecEntrylist.clear();
		phandle->GetDataEntryList(m_vecEntrylist);

		int nNum = 0;
		CString strPointName, strValue, strTime;

		for (int i =0; i<m_vecEntrylist.size(); i++)
		{
			// Get equipment data point and format.
			spointwatch point = m_vecEntrylist[i];
			strPointName = point.strpointname.c_str();
			strValue = point.strvalue.c_str();
			strTime = point.strtime.c_str();
			m_ListAll.InsertItem(nNum, L"");
			m_ListAll.SetItemText(nNum,0, strTime);
			m_ListAll.SetItemText(nNum,1, strPointName);
			m_ListAll.SetItemText(nNum,2, strValue);
			nNum ++;
		}	
	}
}
