#include "StdAfx.h"
#include "BEOPDataLinkManager_Redundancy.h"
#include "OPCLink.h"
#include "ModbusLink.h"
#include "MemoryLink.h"
#include "BacnetLink.h"
#include "BEOPDataPoint/DataPointManager.h"
#include "DB_BasicIO/RealTimeDataAccess.h"
#include "../LAN_WANComm/NetworkComm.h"



 CBEOPDataLinkManager_Redundacy::CBEOPDataLinkManager_Redundacy( CDataPointManager* pointmanager, 
													Beopdatalink::CRealTimeDataAccess* pdbcon, bool bRealSite)
													:CBEOPDataLinkManager(pointmanager, pdbcon, bRealSite, false,false,false),
													m_opcengine_redundancy(NULL)
 {
	
 }

 bool CBEOPDataLinkManager_Redundacy::Init()
 {
	 /*InitPointManagerRedundancy();

	 vector<DataPointEntry> opcpointlist;
	 m_pointmanager_redundancy->GetOpcPointList(opcpointlist);

	 if (!opcpointlist.empty()){
		 m_opcengine_redundancy = new COPCEngine(this);
		 m_opcengine_redundancy->SetOpcPointList(opcpointlist);
		 m_opcengine_redundancy->SetLogSession(GetLogAccess());
		 if (!InitOPC(m_opcengine_redundancy))
		 {
			 return false;
		 }
	 }*/

	//InitMasterSlave();
	
	return CBEOPDataLinkManager::Init();
 }

 void CBEOPDataLinkManager_Redundacy::GetOPCValueSets( vector<pair<wstring, wstring> >& opcvaluesets )
 {
	 COPCEngine* pmainengine = GetMainOPCSession();

	 if (pmainengine)
	 {
		 return pmainengine->GetValueSet(opcvaluesets);
	 }
	 else
	 {

	 }
 }

 bool CBEOPDataLinkManager_Redundacy::WriteOPCValue( const Beopdatalink::CRealTimeDataEntry& entry )
 {
	 COPCEngine* pmainengine = GetMainOPCSession();
	 if (pmainengine){
		 wstring pointname = Project::Tools::AnsiToWideChar(entry.GetName().c_str());
		 return pmainengine->SetValue(pointname, _wtof(entry.GetValue().c_str()));
	 }

	 return false;
 }

 COPCEngine* CBEOPDataLinkManager_Redundacy::GetMainOPCSession()
 {
	 bool ismain_1 = false;
	 bool bok_1 = false;
	 COPCEngine* pmainengine = GetOPCEngine();
	 if (pmainengine){
		 bok_1 = pmainengine->GetValueFromDevice(_T("Is_Main_PLC1"), ismain_1);
	 }
	  
	 bool ismain_2 = false;
	 bool bok_2 = false;
	 if (m_opcengine_redundancy){
		 bok_2 = m_opcengine_redundancy->GetValueFromDevice(_T("Is_Main_PLC2"), ismain_2);

	 }

	 if (ismain_1&&bok_1)
	 {
		 return pmainengine;
	 }else if (ismain_2&&bok_2)
	 {
		 return m_opcengine_redundancy;
	 }

	 return NULL;
 }

 void CBEOPDataLinkManager_Redundacy::UpdateMainServerMode()
 {
	 bool ismainserver = CNetworkComm::GetInstance()->IsMainServerRunningMode();
	 if (ismainserver){
		 SetEngineMode(BeopDataEngineMode::Mode_Master);
	 }else
		 SetEngineMode(BeopDataEngineMode::Mode_Slave);
 }

 void CBEOPDataLinkManager_Redundacy::InitMasterSlave()
 {
	 CNetworkComm::GetInstance()->SetUpperMachineMode(DOUBLE_MODE);
	 CNetworkComm::GetInstance()->StartUdpComm();
	 UpdateMainServerMode();
 }

 CBEOPDataLinkManager_Redundacy::~CBEOPDataLinkManager_Redundacy()
 {
	 if (m_opcengine_redundancy){
		 delete m_opcengine_redundancy;
		 m_opcengine_redundancy = NULL;
	 }

	 if (m_pointmanager_redundancy){
		 delete m_pointmanager_redundancy;
		 m_pointmanager_redundancy = NULL;
	 }
 }

 bool CBEOPDataLinkManager_Redundacy::InitPointManagerRedundancy()
 {
	 return true;
/*	 m_pointmanager_redundancy = new CDataPointManager();

	 gUIParam uiParam;
	 CBeopServerXml::GetInstance()->GetUIParam(uiParam); 
	 const wstring wstrFileName = Project::Tools::AnsiToWideChar(uiParam.strFilePath_Redundancy.c_str());

	 wstring wstrFilePath;
	 Project::Tools::GetSysPath(wstrFilePath);
	 m_pointmanager_redundancy->SetFilePath(wstrFilePath + wstrFileName);

	 return m_pointmanager_redundancy->Init();*/
 }


