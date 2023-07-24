#include "StdAfx.h"
#include "MemoryLink.h"

#include "BEOPDataLinkManager.h"

#define GUARD_LOCK(lockmutex) Project::Tools::Scoped_Lock<Mutex> scope_lock(lockmutex)

CMemoryLink::CMemoryLink(CBEOPDataLinkManager* datalinker)
	:m_datalinker(datalinker)
{
}


CMemoryLink::~CMemoryLink(void)
{
	std::map<wstring, MemoryPointEntry*>::iterator it  = m_valuemap.begin();
	for(;it != m_valuemap.end();it++)
	{
		MemoryPointEntry* pentry = it->second;
		delete(pentry);
		it->second = NULL;
	}

	m_valuemap.clear();
}

void CMemoryLink::SetPointList( const vector<DataPointEntry>& pointlist )
{
	m_pointlist = pointlist;
}

bool CMemoryLink::Init()
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++){
		const DataPointEntry& entry = m_pointlist[i];
		const wstring& pointname = entry.GetShortName();
		
		wstring memorypointtype = entry.GetParam(1);
		MemoryPointEntry* mpentry = NULL;
		if (memorypointtype == L"RelationOP"){
			mpentry = new MemoryPointEntry_Relation(entry, m_datalinker);
			m_typemap[pointname] = VT_BOOL;
		}else if (memorypointtype == L"LogicAnd"){
			mpentry = new MemoryPointEntry_Logic(entry, m_datalinker);
			m_typemap[pointname] = VT_BOOL;
		}
		else if (memorypointtype == L"LogicOr")
		{
			mpentry = new MemoryPointEntry_Logic(entry, m_datalinker);
			m_typemap[pointname] = VT_BOOL;
		}
		else if (memorypointtype == L"LogicXor")
		{
			mpentry = new MemoryPointEntry_Logic(entry, m_datalinker);
			m_typemap[pointname] = VT_BOOL;
		}
		else
		{
			mpentry = new MemoryPointEntry(entry, m_datalinker);
			mpentry->InitValue();
			m_typemap[pointname] = entry.GetMemoryPointType();
		}
		
		m_valuemap[pointname] = mpentry;
		
	}

	return true;
} 


void CMemoryLink::GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist )
{
	for (unsigned int i = 0; i < m_pointlist.size(); i++)
	{
		const wstring& pointname = m_pointlist[i].GetShortName();
		wstring result = L"";
		std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
		if (it != m_valuemap.end())
		{
			MemoryPointEntry* pentry = it->second;

			result = pentry->GetUnicodeString();
			valuelist.push_back( std::make_pair(pointname, result));

		}
	}
}

void CMemoryLink::GetValue( const wstring& pointname, int& ival )
{
	if(pointname==L"ChOnOff07")
		int xx = 0;

	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end())
	{
		int iValueTemp = 0;
		double dValueTemp = 0.f;
		bool bValueTemp=  false;
		switch(GetPointType(pointname))
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
				if(it->second->GetInt(iValueTemp))
					ival = iValueTemp;
				break;
		case VT_R4:
				if(it->second->GetDouble(dValueTemp))
					ival = (int) dValueTemp;
				break;
		case VT_BOOL: 
				if(it->second->GetBool(bValueTemp))
					ival = (int) bValueTemp;
				break;
		case VT_BSTR:
				break;
		default:
				break;
		}
	}
}

void CMemoryLink::GetValue( const wstring& pointname, double& dval )
{

	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end())
	{
		int iValueTemp = 0;
		double dValueTemp = 0.f;
		bool bValueTemp=  false;

		switch(GetPointType(pointname))
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
			if(it->second->GetInt(iValueTemp))
				dval = (double) iValueTemp;
			break;
		case VT_R4:
			if(it->second->GetDouble(dValueTemp))
				dval = dValueTemp;
			break;
		case VT_BOOL:
			if(it->second->GetBool(bValueTemp))
				dval = (double) bValueTemp;
			break;
		case VT_BSTR:
			break;
		default:
			break;
		}
	}
}

void CMemoryLink::GetValue( const wstring& pointname, bool& bval )
{
	if(pointname==L"ChOnOff07")
		int xx = 0;


	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end())
	{
		int iValueTemp = 0;
		double dValueTemp = 0.f;
		bool bValueTemp=  false;

		switch(GetPointType(pointname))
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
			if(it->second->GetInt(iValueTemp))
				bval = (bool) iValueTemp;
			break;
		case VT_R4:
			if(it->second->GetDouble(dValueTemp))
				bval = dValueTemp;
			break;
		case VT_BOOL:
			if(it->second->GetBool(bValueTemp))
				bval = (bool) bValueTemp;
			break;
		case VT_BSTR:
			break;
		default:
			break;
		}
	}
}

void CMemoryLink::GetValue( const wstring& pointname, wstring& strvalue )
{
	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end()){

		switch(GetPointType(pointname))
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
		case VT_BOOL:
			break;
		case VT_BSTR:
			strvalue = it->second->GetUnicodeString();
			break;
		default:
			break;
		}
	}
}

bool CMemoryLink::SetValue( const wstring& pointname, double dval )
{
	if (!m_datalinker){
		return false;
	}
		
	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end()){
		if (m_datalinker->GetLogAccess()){
			CString loginfo;
			loginfo.Format(L"CMemoryLink::SetValue:: pointname:%s, pointvalue:%.1f", pointname.c_str(),dval);
			m_datalinker->GetLogAccess()->InsertLog(loginfo.GetString());
		}
		switch(GetPointType(pointname))
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
			it->second->SetInt((int)dval);
			break;
		case VT_R4:
			it->second->SetDouble(dval);
			break;
		case VT_BOOL:
			it->second->SetBool((bool)dval);
			break;
		default:
			break;
		}
	}

	return true;
}

bool CMemoryLink::SetValue( const wstring& pointname, int ival )
{
	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end()){

		switch(GetPointType(pointname))
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
			it->second->SetInt(ival);
			break;
		case VT_R4:
			it->second->SetDouble((double)ival);
			break;
		case VT_BOOL:
			it->second->SetBool((bool)ival);
			break;
		default:
			break;
		}
	}

	return true;
}

bool CMemoryLink::SetValue( const wstring& pointname, bool bval )
{
	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end()){

		switch(GetPointType(pointname))
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
			it->second->SetInt((int)bval);
			break;
		case VT_R4:
			it->second->SetDouble((double)bval);
			break;
		case VT_BOOL:
			it->second->SetBool(bval);
			break;
		default:
			break;
		}
	}

	return true;
}

//设置字符串值。
bool CMemoryLink::SetValue( const wstring& pointname, const wstring& strVal )
{
	std::map<wstring, MemoryPointEntry*>::iterator it = m_valuemap.find(pointname);
	if (it != m_valuemap.end())
	{
	//	if(GetPointType(pointname)==VT_BSTR)
	//	{
			//字符串相同时，返回false
			if(wcscmp(strVal.c_str(), it->second->GetUnicodeString().c_str())==0)
				return false;
			else
			{
				it->second->SetString(strVal.c_str());
				return true;
			}
	//	}
	}

	return false;
}

int CMemoryLink::GetPointType( const wstring& pointname )
{
	std::map<wstring, int>::iterator it = m_typemap.find(pointname);
	if (it != m_typemap.end()){
		return it->second;
	}

	return (int)VT_NULL;
}

bool CMemoryLink::Exit()
{
	std::map<wstring, MemoryPointEntry*>::iterator iterPoint = m_valuemap.begin();
	while(iterPoint != m_valuemap.end())
	{
		if((*iterPoint).second)
		{
			delete (*iterPoint).second;
			(*iterPoint).second = NULL;
		}
		++iterPoint;
	}
	return true;
}


CMemoryLink::MemoryPointEntry::MemoryPointEntry(const DataPointEntry& pointentry, CBEOPDataLinkManager* datalinker)
	:m_strValue(L""),
	 m_pointentry(pointentry),
	 m_pointname(pointentry.GetShortName()),
	 m_datalinker(datalinker)
{

}

CMemoryLink::MemoryPointEntry::~MemoryPointEntry()
{

}

void CMemoryLink::MemoryPointEntry::SetPointname( const wstring& pointname )
{
	m_pointname = pointname;
}

wstring CMemoryLink::MemoryPointEntry::GetPointname() const
{
	return m_pointname;
}

void CMemoryLink::MemoryPointEntry::SetDouble( double dval )
{
	GUARD_LOCK(m_lock);

	wchar_t wcChar[255];
	swprintf(wcChar, L"%lf", dval);
	wstring strValueSet = wcChar;
	m_strValue = strValueSet;
}

void CMemoryLink::MemoryPointEntry::SetInt( int intval )
{
	GUARD_LOCK(m_lock);
	m_strValue = to_wstring((long long) intval);

}

void CMemoryLink::MemoryPointEntry::SetBool( bool bval )
{
	GUARD_LOCK(m_lock);
	int intval = 0;
	if(bval)
		intval = 1;
	m_strValue = to_wstring((long long) intval);

}



void CMemoryLink::MemoryPointEntry::SetString( const wstring strunicode )
{
	GUARD_LOCK(m_lock);
	m_strValue = strunicode;
}

bool CMemoryLink::MemoryPointEntry::GetDouble(double &dVal)
{
	GUARD_LOCK(m_lock);

	dVal = _wtof(m_strValue.c_str());
	return true;
}

bool CMemoryLink::MemoryPointEntry::GetInt(int &iVal)
{
	GUARD_LOCK(m_lock);

	iVal = (int) _wtof(m_strValue.c_str());
	return true;
}

bool CMemoryLink::MemoryPointEntry::GetBool(bool &bVal)
{
	GUARD_LOCK(m_lock);
	int iVal = (int) _wtof(m_strValue.c_str());
	bVal =  (iVal==1);
	return true;
}

// need to be freeed.
wstring CMemoryLink::MemoryPointEntry::GetUnicodeString()
{
	GUARD_LOCK(m_lock);
	return m_strValue;
}

const DataPointEntry& CMemoryLink::MemoryPointEntry::GetRefEntry()
{
	return m_pointentry;
}

double CMemoryLink::MemoryPointEntry::GetPointValue( const wstring& pointname )
{
	if (pointname == m_pointname){
		return 0.f;
	}
	else{
		return m_datalinker->GetValue(pointname);
	}
}

void CMemoryLink::MemoryPointEntry::InitValue()
{
	wstring param1 = m_pointentry.GetParam(1);
	double defaultvalue = _wtof(param1.c_str());
	if (!param1.empty())
	{
		switch(m_pointentry.GetMemoryPointType())
		{
		case VT_INT:
		case VT_I2:
		case VT_I4:
			SetInt((int)defaultvalue);
			break;
		case VT_R4:
			SetDouble(defaultvalue);
			break;
		case VT_BOOL:
			SetBool((bool)defaultvalue);
			break;
		case VT_BSTR:
			SetString(param1.c_str());
			break;
		default:
			break;
		}
	}
}


bool CMemoryLink::MemoryPointEntry_Relation::Evaluate(bool &bVal)
{
	bool bSuccess = true;
	double leftvalue = GetPointValue(m_pointname_leftopertator);
	if(m_evaluatetype ==RO_Equal)
		bVal = (leftvalue == m_rightvalue);		// ==
	else if(m_evaluatetype == RO_UnEqual)
		bVal = (leftvalue != m_rightvalue);		// != 
	else if(m_evaluatetype == RO_Bigger)
		bVal = (leftvalue > m_rightvalue);		// >
	else if(m_evaluatetype == RO_Less)
		bVal = (leftvalue < m_rightvalue);		// <
	else if(m_evaluatetype == RO_BiggerEqual)
		bVal = (leftvalue >= m_rightvalue);		// >= 
	else if(m_evaluatetype == RO_LessEqual)									
		bVal = (leftvalue <= m_rightvalue);		// <=
	else
		bSuccess = false;

	return bSuccess;
}

void CMemoryLink::MemoryPointEntry_Relation::Parse()
{
	CString strexpression = GetRefEntry().GetParam(2).c_str();
	m_evaluatetype = RO_NULL;
	m_isvalid = false;
	
	int findresult = strexpression.Find(L"==");
	if (findresult > 0 && findresult < (strexpression.GetLength()-1)){
		m_evaluatetype = RO_Equal;
		CString leftpoint = strexpression.Left(findresult);
		leftpoint.Trim();
		CString rightpoint = strexpression.Right(strexpression.GetLength() - findresult - 2);
		rightpoint.Trim();
		if (leftpoint.IsEmpty() || rightpoint.IsEmpty()){
			m_isvalid = false;
			return;
		}
		m_pointname_leftopertator = leftpoint.GetString();
		m_rightvalue = _wtof(rightpoint.GetString());
		m_isvalid = true;
		return;
	}

	findresult = strexpression.Find(L">=");
	if (findresult > 0 && findresult < (strexpression.GetLength()-1))
	{
		m_evaluatetype = RO_BiggerEqual;
		CString leftpoint = strexpression.Left(findresult);
		leftpoint.Trim();
		CString rightpoint = strexpression.Right(strexpression.GetLength() - findresult - 2);
		rightpoint.Trim();
		if (leftpoint.IsEmpty() || rightpoint.IsEmpty()){
			m_isvalid = false;
			return;
		}
		m_pointname_leftopertator = leftpoint.GetString();
		m_rightvalue = _wtof(rightpoint.GetString());
		m_isvalid = true;
		return;
	}

	findresult = strexpression.Find(L"<=");
	if (findresult > 0 && findresult < (strexpression.GetLength()-1))
	{
		m_evaluatetype = RO_LessEqual;
		CString leftpoint = strexpression.Left(findresult);
		leftpoint.Trim();
		CString rightpoint = strexpression.Right(strexpression.GetLength() - findresult - 2);
		rightpoint.Trim();
		if (leftpoint.IsEmpty() || rightpoint.IsEmpty()){
			m_isvalid = false;
			return;
		}
		m_pointname_leftopertator = leftpoint.GetString();
		m_rightvalue = _wtof(rightpoint.GetString());
		m_isvalid = true;
		return;
	}

	findresult = strexpression.Find(L"!=");
	if (findresult > 0 && findresult < (strexpression.GetLength()-1))

	{
		m_evaluatetype = RO_UnEqual;
		CString leftpoint = strexpression.Left(findresult);
		leftpoint.Trim();
		CString rightpoint = strexpression.Right(strexpression.GetLength() - findresult - 2);
		rightpoint.Trim();
		if (leftpoint.IsEmpty() || rightpoint.IsEmpty()){
			m_isvalid = false;
			return;
		}
		m_pointname_leftopertator = leftpoint.GetString();
		m_rightvalue = _wtof(rightpoint.GetString());
		m_isvalid = true;
		return;
	}

	findresult = strexpression.Find(L'>');
	if (findresult > 0 && findresult < (strexpression.GetLength()-1))
	{
		m_evaluatetype = RO_Bigger;
		CString leftpoint = strexpression.Left(findresult);
		leftpoint.Trim();
		CString rightpoint = strexpression.Right(strexpression.GetLength() - findresult - 1);
		rightpoint.Trim();
		if (leftpoint.IsEmpty() || rightpoint.IsEmpty()){
			m_isvalid = false;
			return;
		}
		m_pointname_leftopertator = leftpoint.GetString();
		m_rightvalue = _wtof(rightpoint.GetString());
		m_isvalid = true;

		return;
	}

	findresult = strexpression.Find(L'<');
	if (findresult > 0 && findresult < (strexpression.GetLength()-1))
	{
		m_evaluatetype = RO_Less;
		CString leftpoint = strexpression.Left(findresult);
		leftpoint.Trim();
		CString rightpoint = strexpression.Right(strexpression.GetLength() - findresult - 1);
		rightpoint.Trim();
		if (leftpoint.IsEmpty() || rightpoint.IsEmpty()){
			m_isvalid = false;
			return;
		}
		m_pointname_leftopertator = leftpoint.GetString();
		m_rightvalue = _wtof(rightpoint.GetString());
		m_isvalid = true;

		return;
	}
}

CMemoryLink::MemoryPointEntry_Relation::MemoryPointEntry_Relation( const DataPointEntry& pointentry, CBEOPDataLinkManager* datalinker )
	:MemoryPointEntry(pointentry, datalinker)
{
	Parse();
}

bool CMemoryLink::MemoryPointEntry_Relation::GetBool(bool &bVal)
{
	return Evaluate(bVal);
}

CMemoryLink::MemoryPointEntry_Relation::~MemoryPointEntry_Relation()
{

}

CMemoryLink::MemoryPointEntry_Logic::MemoryPointEntry_Logic( const DataPointEntry& pointentry , CBEOPDataLinkManager* datalinker)
	:MemoryPointEntry(pointentry, datalinker)
{
	Parse();
}

void CMemoryLink::MemoryPointEntry_Logic::Parse()
{
	CString expression_type = GetRefEntry().GetParam(1).c_str();
	
	if (expression_type.CompareNoCase(L"LogicAnd") == 0){
		m_evaluatetype = RO_And;
	}
	else if (expression_type.CompareNoCase(L"LogicOr") == 0){
		m_evaluatetype = RO_Or;
	}
	else if (expression_type.CompareNoCase(L"LogicXor") == 0){
		m_evaluatetype = RO_Xor;
	}else
	{
		m_evaluatetype = RO_NULL;

		wstring loginfo = L"ERROR: ";
		loginfo += GetRefEntry().GetShortName();
		loginfo += L" expression not valid! can not parsed!";
		if (m_datalinker && m_datalinker->IsLogEnabled()){
				m_datalinker->AddLog(loginfo);
			m_datalinker->SendLogMsg(L"Memory", loginfo.c_str());
		}
	}

	for (unsigned int i = 2; i <= 10; i++)
	{
		wstring pointname = GetRefEntry().GetParam(i);
		if (!pointname.empty()){
			m_evaluatelist.push_back(pointname);
		}
	}

	if (m_evaluatetype == RO_NULL || m_evaluatelist.empty()){
		m_isvalid = false;
	}else
		m_isvalid = true;
}

bool CMemoryLink::MemoryPointEntry_Logic::Evaluate(bool &bVal)
{
	if (!m_isvalid)
	{
		return false;
	}

	if(m_evaluatetype == RO_And)
	{
		for (unsigned int i = 0; i < m_evaluatelist.size(); i++)
		{
			bool bval = (bool)GetPointValue(m_evaluatelist[i]);
			if (!bval)
			{
				bVal =  false;
				return true;
			}
		}
		bVal = true;
		return true;
	}	
	else if(m_evaluatetype == RO_Or)
	{
		for (unsigned int i = 0; i < m_evaluatelist.size(); i++)
		{
			bool bval = (bool)GetPointValue(m_evaluatelist[i]);
			if (bval)
			{
				bVal= true;
				return true;
			}
		}
		bVal =  false;
		return true;
	}
	else if(m_evaluatetype == RO_Xor)
	{
		if (m_evaluatelist.size() != 2)
		{
			return false;
		}

		bool bleft = (bool)GetPointValue(m_evaluatelist[0]);
		bool bright = (bool)GetPointValue(m_evaluatelist[1]);

		bVal = (bleft ^ bright);
		return true;
	}
	else
	{
		return false;
	}


	return true;
}

bool CMemoryLink::MemoryPointEntry_Logic::GetBool(bool &bVal)
{
	return Evaluate(bVal);
}

CMemoryLink::MemoryPointEntry_Logic::~MemoryPointEntry_Logic()
{

}



void    CMemoryLink::AddVPoint(wstring wstrPointName)
{
	DataPointEntry entry;
	entry.SetShortName(wstrPointName);
	entry.SetSourceType(_T("vpoint"));
	entry.SetStoreCycle(E_STORE_ONE_MINUTE);
	m_pointlist.push_back(entry);

	MemoryPointEntry * mpentry = new MemoryPointEntry(entry, m_datalinker);
	mpentry->InitValue();
	m_valuemap[wstrPointName] = mpentry;
}

void    CMemoryLink::RemoveVPoint(wstring wstrPointName)
{
	for(int i=0;i<m_pointlist.size();i++)
	{
		if(m_pointlist[i].GetShortName() == wstrPointName)
		{
			m_pointlist.erase(m_pointlist.begin()+i);
			i--;


			MemoryPointEntry * mpentry = m_valuemap[wstrPointName];
			if(mpentry)
			{
				
				delete(mpentry);
				mpentry = NULL;
			}
			m_valuemap.erase(wstrPointName);
		}
	}
}