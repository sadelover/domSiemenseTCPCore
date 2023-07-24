
/*
 *这个类用来支持upppc data engine的软点功能
 *软点不从其他地方读取,只维护在内存里.
 *
 *
 */


#pragma once

#include "BEOPDataPoint/DataPointEntry.h"
#include <vector>
#include <utility>
#include <map>
#include "Tools/CustomTools/CustomTools.h"

class CBEOPDataLinkManager;
class CMemoryLink
{
public:
	CMemoryLink(CBEOPDataLinkManager* datalinker);
	~CMemoryLink(void);

	//set pointlist;
	void	SetPointList(const std::vector<DataPointEntry>& pointlist);
	int	GetPointType(const wstring& pointname);

	//init
	// set point must be called before init.
	bool	Init();
	bool    Exit();

	//return the value set.
	void	GetValueSet( std::vector< std::pair<wstring, wstring> >& valuelist );
	
	void	GetValue(const wstring& pointname, int& ival);
	void	GetValue(const wstring& pointname, double& dval);
	void	GetValue(const wstring& pointname, bool& bval);
	void	GetValue(const wstring& pointname, wstring& strvalue);
	

	bool	SetValue(const wstring& pointname, double dval);
	bool	SetValue(const wstring& pointname, int ival);
	bool	SetValue(const wstring& pointname, bool bval);

	bool	SetValue(const wstring& pointname, const wstring& value);


	void    AddVPoint(wstring wstrPointName);
	void    RemoveVPoint(wstring wstrPointName);


private:

	class MemoryPointEntry
	{
	public:
		MemoryPointEntry(const DataPointEntry& pointentry, CBEOPDataLinkManager* datalinker);
		virtual ~MemoryPointEntry();

		void SetPointname(const wstring& pointname);
		wstring GetPointname() const;

		virtual void SetDouble(double dval);
		virtual void SetInt(int intval);
		virtual void SetBool(bool bval);

		virtual void SetString(const wstring strunicode);

		virtual bool  GetDouble(double &dVal);
		virtual bool	GetInt(int &iVal);
		virtual bool	GetBool(bool &bVal);
		virtual wstring GetUnicodeString();

		const DataPointEntry& GetRefEntry();

		double GetPointValue(const wstring& pointname);

		void	InitValue();
	private:
		wstring m_pointname;
		Project::Tools::Mutex m_lock;
		wstring       m_strValue;
		DataPointEntry	m_pointentry;
	protected:
		CBEOPDataLinkManager*	m_datalinker;
	};

	class MemoryPointEntry_Relation : public MemoryPointEntry
	{
	public:
		enum RELATION_OPERATOR_TYPE{
			RO_NULL,
			RO_Equal,		// ==
			RO_UnEqual,		// != 
			RO_Bigger,		// >
			RO_Less,		// <
			RO_BiggerEqual,	// >= 
			RO_LessEqual,	// <=
		};
	public:
		MemoryPointEntry_Relation(const DataPointEntry& pointentry, CBEOPDataLinkManager* datalinker);
		virtual ~MemoryPointEntry_Relation();

		virtual bool	GetBool(bool &bVal);
						
		bool Evaluate(bool &bVal);

	private:
		wstring m_pointname_leftopertator;
		double  m_rightvalue;
				
		RELATION_OPERATOR_TYPE	m_evaluatetype;

		void Parse();

		bool	m_isvalid;
	};

	class MemoryPointEntry_Logic : public MemoryPointEntry
	{
	public:
		enum RELATION_OPERATOR_TYPE{
			RO_NULL,
			RO_And,
			RO_Or,
			RO_Xor
		};
	public:
		MemoryPointEntry_Logic(const DataPointEntry& pointentry, CBEOPDataLinkManager* datalinker);
		virtual ~MemoryPointEntry_Logic();

		virtual bool	GetBool(bool &bVal);
		
		bool Evaluate(bool &bVal);

	private:
		vector<wstring> m_evaluatelist;
		
		

		RELATION_OPERATOR_TYPE	m_evaluatetype;
		void Parse();
		
		bool	m_isvalid;
	};


private:
	std::vector<DataPointEntry>	m_pointlist;
	
	std::map<wstring, MemoryPointEntry*> m_valuemap;
	std::map<wstring, int>	m_typemap;

	CBEOPDataLinkManager*	m_datalinker;
};

