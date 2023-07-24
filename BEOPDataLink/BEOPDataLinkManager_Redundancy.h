#pragma once
#include "BEOPDataLinkManager.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "BEOPDataPoint/DataPointEntry.h"

class COPCEngine;
class CModbusEngine;
class CMemoryLink;
class CBacnetEngine;
class CDataPointManager;


#include <map>
using std::map;

#include "DataEngineMode.h"
/*
*西门子冗余的engine解决方案
*
 */
class CBEOPDataLinkManager_Redundacy: public CBEOPDataLinkManager
{
public:
	CBEOPDataLinkManager_Redundacy(CDataPointManager* pointmanager,
							 Beopdatalink::CRealTimeDataAccess* pdbcon,
							 bool bRealSite);

	virtual ~CBEOPDataLinkManager_Redundacy();

	virtual bool	Init();
	bool	InitPointManagerRedundancy();

	virtual void	GetOPCValueSets(vector<pair<wstring, wstring> >& opcvaluesets);
	virtual	bool	WriteOPCValue(const Beopdatalink::CRealTimeDataEntry& entry);
	
	COPCEngine*		GetMainOPCSession();		//当为主时,从主PLC更新数据.


	/*
	 *从服务器会一直ping主服务器,如果收不到回应,则会自动成为主服务器
	 */
	virtual void			UpdateMainServerMode();		//更新主从.

	void	InitMasterSlave();
private:
	CDataPointManager*	m_pointmanager_redundancy;
	COPCEngine*			m_opcengine_redundancy;
};