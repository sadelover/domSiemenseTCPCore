#include "StdAfx.h"
#include "CsvManager.h"
#include "DataPointEntry.h"


CCsvManager::CCsvManager(const CString strPathName)
{
	m_strPathName = strPathName;
}


CCsvManager::~CCsvManager(void)
{
}

BOOL CCsvManager::GetFileInfo(vector<OPCDataPointEntry>& vecData)
{
	setlocale(LC_CTYPE, "chs");
	TCHAR* pFileName = m_strPathName.GetBuffer();
	CStdioFile file;
	if (!file.Open(pFileName, CFile::modeRead | CFile::typeText))
	{
		TRACE(_T("Unable to open file\n"));
		return FALSE;
	}

	vecData.clear();
	const CString strFlag(_T(","));
	CString strBuff;

	try
	{
		int nRow = 1;
		while (file.ReadString(strBuff))
		{
			strBuff.TrimRight(strFlag);
			strBuff += strFlag;
			OPCDataPointEntry dataPt;
			int nCnt = 0;
			dataPt.SetPointIndex(nRow);
			while (TRUE)
			{
				int nIndex = strBuff.Find(strFlag);
				if (-1 == nIndex)
				{
					break;
				}
				else
				{
					CString strTemp(strBuff.Left(nIndex));
					strBuff = strBuff.Mid(nIndex + 1);
					dataPt.SetSourceType(L"OPC");
					switch (nCnt)
					{
					case 0:
						dataPt.SetShortName(strTemp.GetString());
						break;
					case 1:
						dataPt.SetParam(1, strTemp.GetString());
						break;
					case 2:
						dataPt.SetParam(2, strTemp.GetString());
						break;
					case 3:
						dataPt.SetParam(3, strTemp.GetString());
						break;
					case 4:
						dataPt.SetParam(4, strTemp.GetString());
						break;
					default:
						break;
					}
					++nCnt;
				}
			}
			vecData.push_back(dataPt);
			++nRow;
		}
	}
	catch (...)
	{
		return FALSE;
	}

	if (vecData.size() <= 0)
	{
		return FALSE;
	}
	return TRUE;
}
