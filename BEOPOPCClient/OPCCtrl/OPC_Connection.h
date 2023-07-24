#pragma once

#include "OPCdefine.h"

#include <hash_map>
using namespace stdext;

#define WRITE_SYNC		0
#define WRITE_ASYNC10	1
#define WRITE_ASYNC20	2

#include "Tools/CustomTools/CustomTools.h"
#include "../DataPointEntry.h"

struct OPC_VALUE_INFO
{
	wstring strOPCName;
	wstring strOPCValue;
	VARTYPE vType;
	double	dMultiple;
	int		nPosition;
	DWORD	nIndex;
	bool	bUpdate;
};

// using namespace Project::Logs;
class COPCConnection
{
public:
	COPCConnection();
	~COPCConnection(void);

private:
	void GetValue(VARIANT &vtVal		// [in]
		        , TCHAR *pBuffer				// [out]
		        , int nBufLen);					// [in]
        
	void AddLeaves(LPENUMSTRING pIEnumString);

public:
    //设置连接参数
	void	SetServerProgName(const wstring& strProgID);
	wstring	GetServerProgName() const;

	//-----------------------------------------------------------------
    //连接到OPC服务器
	bool Connect(wstring strOPCServerIP,int nOPCClientIndex = -1);
    
    //从opc服务器断开
	void DisConnect(void);

	//从远程计算机通过服务名称获取CLSID
	bool	GetCLSIDByProgID(wstring strOPCServerIP,wstring strProgID,CLSID& clsid);

    //添加OPC group
	virtual void AddGroup(void);

    //添加所有opc变量
	virtual bool AddItems(void);
	
	//------------------------------------------------------------------------------
    virtual bool GetItemValue(const wstring& strItemID, wstring &strValue, bool bFromDevice = false);

	virtual bool GetMutilItemValue( map<wstring,OPC_VALUE_INFO>& mapItem );
    
    bool	UpdateItemValue(const wstring& strItemID, wstring &strValue);
		
	virtual bool SetItemValue(const wstring& strItemID, const wstring& strValue);
	

	virtual bool	RefreshAsync20(bool bDeviceRead);

	VARTYPE GetVarType(const wstring& strItemName);
	LPCTSTR	GetVarQuality(const wstring& strItemName);

	CKItem* FindItem(const wstring& strItemName);
    
public:
	
	OPCSERVER_TYPE_	GetServerType()const;
	bool		IsConnect()const ;
	
protected:
	wstring		m_strserverprogid;
	wstring		m_strserverhost;

	wstring		m_strFilterLeaf;
	BOOL		m_bBrowseFlat;
	
	CObArray	m_itemlist;	//保存所有的Item
    
    hash_map<wstring, CKItem*> m_searchmap;    //保存所有item的检索表
	hash_map<wstring, OPCDataPointEntry> m_searchEntrymap;    //保存所有item entity的检索表
    
	CKServer	*m_pServer;     // server对象
	CKGroup		*m_pGroup;      //group对象
	bool			m_isconnected;
	
    Project::Tools::Mutex m_lock;
	
};