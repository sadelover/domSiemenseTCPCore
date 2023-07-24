#include "stdafx.h"
#include "SqliteRW.h"
//#include "../Equipments/Models/ChillerModel.h"

//extern FileInterface *g_pDatabase;

CSqliteRW::CSqliteRW(void)
{
}
CSqliteRW::~CSqliteRW(void)
{
}
//
//bool CSqliteRW::GetEquipmentSql(DEVICETYPE type,void* record,string &sql_)
//{
//	assert(record != NULL);
//	char sql[512];
//	memset(sql, 0x00, sizeof(sql));
//	switch(type)
//	{
//	case Type_CHILLER:
//		{
//			gRoom_Contain_Chiller* data = (gRoom_Contain_Chiller*)record;
//			_bstr_t t = data->strCHillerName_InRoom.c_str();  
//			char* pchar = (char*)t;  
//			string name = pchar;
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_chiller values(%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d, %f, %f, %f, %f, %f,%d); ", \
//				data->iChillerID_In_Room, name.c_str(), data->iChillerID, data->iOF_CPR_ID, \
//				data->bEnabled, data->iX, data->iY, data->iChillerModelID, data->iChillerComID, data->iPlcCtrl_id, data->fRotateAngel, data->fScale, data->fSChill, data->fSCool,data->fChillerRate,data->iHeatPump);
//			break;
//		}
//	case Type_PUMP:
//		{	
//			gRoom_Contain_Pump* data = (gRoom_Contain_Pump*)record;
//			_bstr_t t = data->strPumpName_InRoom.c_str();  
//			char* pchar = (char*)t;  
//			string name = pchar;
//
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_pump values(%d, '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %f, %f);" ,\
//				data->iPumpID_In_Room, name.c_str(), data->iPumpID, data->iOF_CPR_ID, \
//				data->iPump_Location, data->bEnabled, data->bStandby, data->iX, data->iY, data->iPumpModelID, data->iPlcCtrl_id, data->bWithVSD, data->fRotateAngel, data->fScale);
//			break;
//		}
//	case Type_COOLINGTOWER:
//		{	
//			gRoom_Contain_CoolingTower* data = (gRoom_Contain_CoolingTower*)record;			
//			_bstr_t t = data->strTowerName_InRoom.c_str();  
//			char* pchar = (char*)t;  
//			string name = pchar;
//
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_coolingtower values (%d, '%s', %d, %d, %d, %f, %d, %d,%d, %d, %d, %f, %f, %f);", \
//				data->iTowerID_In_Room, name.c_str(), data->iTowerID, data->iOF_CPR_ID, \
//				data->bEnabled, data->dMax_Capacity, data->iX, data->iY, data->iCTModelID, data->iPlcCtrl_id,data->bWithVSD, data->fRotateAngel, data->fScale, data->fSCool);
//			break;
//		}
//	case Type_HX:
//		{
//			gRoom_Contain_Hx* data = (gRoom_Contain_Hx*)record;
//			_bstr_t t = data->strHxName_InRoom.c_str();  
//			char* pchar = (char*)t;  
//			string name = pchar;
//
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_hx VALUES (%d,'%s',%d,%d,%d,%f,%d,%d, %d, %f, %f);", \
//				data->iHxID_In_Room, name.c_str(), data->iHxID, data->iOF_CPR_ID, \
//				data->bEnabled, data->dMax_Capacity, data->iX, data->iY, data->iPlcCtrl_id, data->fRotateAngel, data->fScale);
//			break;
//		}
//	case Type_PIPE:
//		{
//			gRoom_Contain_Pipe* data = (gRoom_Contain_Pipe*)record;
//			_bstr_t t = data->strPipeName_InRoom.c_str();  
//			char* pchar = (char*)t;  
//			string name = pchar;
//
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_pipe values (%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d);" , \
//				data->iPipeID_In_Room, name.c_str(), data->iPipeID, data->iOF_CPR_ID, \
//				data->bEnabled, data->from_service_id, data->from_service_type, data->from_service_port,\
//				data->to_service_id, data->to_service_type, data->to_service_port);
//			break;
//		}		
//	case Type_CORNER:
//		{
//			gRoom_Contain_Corner* data = (gRoom_Contain_Corner*) record;
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_corner values (%d,%d,%d,%d,%d);", \
//				data->iCorner_ID_InRoom, data->iCorner_ID, data->iX, data->iY, data->iOF_CPR_ID);
//			break;
//		}
//	case Type_TERMINAL:
//		{
//			gRoom_Contain_Terminal* data = (gRoom_Contain_Terminal*)record;
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_terminal values (%d,%d,%d,%d,%d, %f, %f, %f);", \
//				data->iTerminal_ID_InRoom, data->iTerminal_ID, data->iX, data->iY, data->iOF_CPR_ID, data->fRotateAngel, data->fScale, data->fSChill);
//			break;
//		}
//	case Type_VALVE:
//		{
//			gRoom_Contain_Valve* data = (gRoom_Contain_Valve*)record;
//			_bstr_t t = data->strValveName_InRoom.c_str();  
//			char* pchar = (char*)t;  
//			string name = pchar;
//
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_valve values(%d,'%s',%d,%d,%d,%d,%d,%d, %f, %f);", \
//				data->iValveID_In_Room, name.c_str(), data->iValveID, data->iOF_CPR_ID,\
//				data->bEnabled, data->iX, data->iY, data->iPlcCtrl_id, data->fRotateAngel, data->fScale);
//			break;
//		}		
//	case Type_WATERNET:
//		{
//			gRoomContainWaternet* data = (gRoomContainWaternet*)record;
//			string pumpgroup = Project::Tools::tstringToString(data->strPumpGroup);
//			string towergroup = Project::Tools::tstringToString(data->strTowerGroup);
//			string chillergroup  = Project::Tools::tstringToString(data->strChillerGroup);
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_waternet values(%d,'%s','%s','%s',%d,%d,%d)", data->iWaternetId, \
//				pumpgroup.c_str(), towergroup.c_str(), chillergroup.c_str(), data->iOF_CPR_ID, data->iWaternetModelID, \
//				data->iChillOrCool);
//			break;
//		}
//	case Type_Tank:
//		{
//			Tank_Entry_InRoom* data = (Tank_Entry_InRoom*)record;
//			_bstr_t t = data->strName_In_Room.c_str();  
//			char* pchar = (char*)t;  
//			string tankname = pchar;
//
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_tank values(%d, '%s', %d, %d, %d, %d)", \
//				data->iId_In_Room, tankname.c_str(), data->iOf_CPR_ID, data->iX, data->iY,/*data->dVolumn,data->dCapacity,*/data->iTankID_In_Lib);
//			break;
//		}
//	case Type_WATERSOIL:
//		{
//			WaterSoil_Entry_InRoom* data = (WaterSoil_Entry_InRoom*)record;
//			sprintf_s(sql, sizeof(sql), "insert into room_contain_watersoil values(%d, %d, %f, %f, %f, %d, %d, %d, %d, %d )",\
//				data->iId, data->iType, data->dTemp_Winter, data->dTemp_Summer, data->dTGrade, data->iOf_CPR_ID,\
//				data->iX, data->iY,data->iWaterSoilID,data->iWSModelID);
//			break;
//		}
//	}
//
//	sql_ = sql;
//
//	return true;
//}
//bool CSqliteRW::SaveProject(string &sql_)
//{
//	sql_ = "";
//	gProject *gp =  g_pDatabase->GetProject()->GetProperty();
//	
//	_bstr_t t = gp->strName.c_str();  
//	char* pchar = (char*)t;  
//	string name = pchar;
//
//	string location		= Project::Tools::WideCharToUtf8(gp->strLocation.c_str());
//	string memo			= Project::Tools::WideCharToUtf8(gp->strMemo.c_str());
//	string creator		= Project::Tools::tstringToString(gp->strCreator);
//	unsigned long len	= (unsigned long)gp->strPicture.length();
//	//char* buffer		= new char[2 * len + 1];
//	//memset(buffer, 0x00, 2 * len + 1);
//	{	
//		//long newlen = mysql_real_escape_string(&m_mysql, buffer, gp->strPicture.data(), len);		
//		ostringstream sqlstream;
//		sqlstream << "insert into project values(" << gp->iProjectID<< ",'" << name << "','" \
//			<< location << "'," << gp->iProjectType << "," << gp->iWeatherStation \
//			<< "," << gp->dFloorArea << ",'" << "" << "','" << memo  << "','" << creator << "')";
//		//delete[] buffer;
//		sql_ = sqlstream.str();
//	}
//	return true;
//}
//bool CSqliteRW::ReadProject(CSqliteAcess &sqliteAcess,gProject &gp,string &sql_)
//{
//	
//	ostringstream sqlstream;
//	sqlstream << "select * from project;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//
//		gp.iProjectID = sqliteAcess.getColumn_Int(0);
//
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		gp.strName    = name;
//
//		CString   location_temp(const_cast<char*>(sqliteAcess.getColumn_Text(2)));
//		wstring   location = location_temp.GetString();
//		gp.strLocation    = location;
//
//		gp.iProjectType			= sqliteAcess.getColumn_Int(3);
//		gp.iWeatherStation		= sqliteAcess.getColumn_Int(4);
//		gp.dFloorArea			= sqliteAcess.getColumn_Double(5);
//
//		CString   Memo_temp(const_cast<char*>(sqliteAcess.getColumn_Text(7)));
//		wstring   Memo = Memo_temp.GetString();
//		gp.strMemo    = Memo;
//
//		CString   Creator_temp(const_cast<char*>(sqliteAcess.getColumn_Text(8)));
//		wstring   Creator = Creator_temp.GetString();
//		gp.strCreator    = Creator;
//
//	}
//
//
//
//	return true;
//}
//bool CSqliteRW::SaveChillerPlantRoom(string &sql_)
//{
//	sql_ = "";
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	gCHPlantRoom room;
//	room.iCPR_ID                             	= pRoom->m_iRoomID;
//	room.iOF_PROJECT_ID                      	= pRoom->m_iProjectID;
//	room.strCPR_NAME                         	= pRoom->m_strName;
//	room.dSetting_Max_LEWT 				  		= pRoom->dSetting_Max_LEWT;
//	room.dSetting_Min_LEWT 				  		= pRoom->dSetting_Min_LEWT;
//	room.dSetting_Max_ECWT 				  		= pRoom->dSetting_Max_ECWT;
//	room.dSetting_Min_ECWT 				  		= pRoom->dSetting_Min_ECWT;
//	room.dSetting_Max_Chilled_DT             	= pRoom->dSetting_Max_Chill_DT;
//	room.dSetting_Max_Cooling_DT             	= pRoom->dSetting_Max_Cool_DT;
//	room.dSetting_Min_Chiller_LoadRatio      	= pRoom->dSetting_Min_Chiller_LoadRatio;
//	room.dSetting_Min_Pump_FlowRatio         	= pRoom->dSetting_Min_Pump_FlowRatio;
//	room.dSetting_Min_Tower_LoadRatio        	= pRoom->dSetting_Min_Tower_LoadRatio;
//	room.dSetting_Sys_Min_ChillFlow          	= pRoom->dSetting_Sys_Min_ChillFlow;
//	room.dSetting_Sys_Min_CoolFlow           	= pRoom->dSetting_Sys_Min_CoolFlow;
//	room.strHVACType                         	= pRoom->m_strHVACSystemType;
//	room.dSChillOther                        	= pRoom->m_fS_Chill;
//	room.dSCoolOther                         	= pRoom->m_fS_Cool;
//	room.iRoomType                           	= pRoom->m_RoomType;
//	room.dTowerChillerCountRatio  				= pRoom->m_fTowerChiller_CountRatio;
//	room.dPumpChillerCountRatio   				= pRoom->m_fPumpChiller_CountRatio;
//	room.bRatioUseable            				= pRoom->m_bRatioUseable;
//
//	room.dSetting_Common_LEWT     				= pRoom->m_dSetting_Common_LEWT;
//	room.bVSD_CoolPump            				= pRoom->m_bVSD_CoolPump?1:0;
//	room.bVSD_Pri_ChillPump       				= pRoom->m_bVSD_Pri_ChillPump?1:0;
//	room.bVSD_Sec_ChillPump       				= pRoom->m_bVSD_Sec_ChillPump?1:0;
//	room.dVSD_Value               				= pRoom->m_dVSD_Value;
//
//	room.dSetting_CoolPump_MinPowerRatio		= pRoom->dSetting_CoolPump_MinPowerRatio;
//	room.dSetting_PriChillPump_MinPowerRatio	= pRoom->dSetting_PriChillPump_MinPowerRatio;
//	room.dSetting_SecChillPump_MinPowerRatio	= pRoom->dSetting_SecChillPump_MinPowerRatio;
//	room.dSetting_Tower_PowerIndex				= pRoom->dSetting_Tower_PowerIndex;
//
//	room.dSetting_dH0							= pRoom->m_dH0;						        
//	room.dSetting_dChiller_K					= pRoom->m_dChiller_K;				
//	room.dSetting_dCT_MinHz						= pRoom->m_dCT_MinHz;					
//	room.dSetting_dCT_eff_Correction			= pRoom->m_dCT_eff_Correction;
//	//////////////////////////////////////////////////////////////////////////////////////////////
//	//现场加减冷机判断需要的几个参数
//	room.dSetting_dElectricPercent				= pRoom->m_dElectricPercent;   
//	room.dSetting_dApproachT_Add				= pRoom->m_dApproachT_Add;     
//	room.dSetting_dApproachT_Subtract			= pRoom->m_dApproachT_Subtract;
//	room.dSetting_dDiffFlow						= pRoom->m_dDiffFlow;
//
//	room.dSetting_dVSD_Value_PRI				= pRoom->m_dVSD_Value_PRI;              
//	room.dSetting_dVSD_Value_SEC             	= pRoom->m_dVSD_Value_SEC;              
//	room.dSetting_dVSD_Value_CT              	= pRoom->m_dVSD_Value_CT;               
//	room.dSetting_bVSD_CoolingTower          	= pRoom->m_bVSD_CoolingTower;           
//
//	room.dSetting_fChillPumpChiller_CountRatio	= pRoom->m_fChillPumpChiller_CountRatio;
//	room.dSetting_bCT_OutT_Control           	= pRoom->m_bCT_OutT_Control;            
//	room.dSetting_bDeltaT_Control            	= pRoom->m_bDeltaT_Control;             
//	room.dSetting_dDeltaT_Chill              	= pRoom->m_dDeltaT_Chill;               
//	room.dSetting_dDeltaT_Cool               	= pRoom->m_dDeltaT_Cool;                
//
//	//string name					= Project::Tools::tstringToString(room.strCPR_NAME);
//	_bstr_t t = room.strCPR_NAME.c_str();  
//	char* pchar = (char*)t;  
//	string name = pchar;
//
//	string controlparam			= Project::Tools::tstringToString(room.strPumpGroup_Param);
//	string roomtype				= Project::Tools::tstringToString(room.strHVACType);
//	int    rario_useable		= room.bRatioUseable?1:0;
//	int    vsd_coolpump			= room.bVSD_CoolPump?1:0;
//	int    vsd_prichillpump		= room.bVSD_Pri_ChillPump?1:0;
//	int    vsd_secchillpump		= room.bVSD_Sec_ChillPump?1:0;
//	int    vsd_coolingtower		= room.dSetting_bVSD_CoolingTower?1:0;
//	int    ct_outT_control      = room.dSetting_bCT_OutT_Control?1:0;
//	int    deltaT_control       = room.dSetting_bDeltaT_Control?1:0;
//
//	ostringstream sqlstream;
//	sqlstream << "insert into chillerplantroom values(" << room.iCPR_ID <<",'" << name << "'," << room.iOF_PROJECT_ID \
//		<< "," << room.dSetting_Max_LEWT << ","<<room.dSetting_Min_LEWT<<","<<room.dSetting_Max_ECWT<<"," << room.dSetting_Min_ECWT \
//		<< "," << room.dSetting_Max_Cooling_DT << "," << room.dSetting_Max_Chilled_DT \
//		<<","<< room.dSetting_Min_Chiller_LoadRatio<<"," << room.dSetting_Min_Pump_FlowRatio <<"," << room.dSetting_Min_Tower_LoadRatio\
//		<<","<< room.dSetting_Sys_Min_ChillFlow<<"," << room.dSetting_Sys_Min_CoolFlow \
//
//		<< ",'"<<roomtype<<"',"<< room.dSChillOther<<","<<room.dSCoolOther<<","<<room.iRoomType<< ",'" << controlparam << "',"<< room.dTowerChillerCountRatio\
//		<< ","<<room.dPumpChillerCountRatio<<","<<rario_useable<<","<<room.dSetting_Common_LEWT<<","<<vsd_coolpump<<","<<vsd_prichillpump\
//		<< ","<<vsd_secchillpump<<","<<room.dVSD_Value<<","<<room.dSetting_CoolPump_MinPowerRatio<<","<<room.dSetting_PriChillPump_MinPowerRatio\
//		<< ","<<room.dSetting_SecChillPump_MinPowerRatio<<","<<room.dSetting_Tower_PowerIndex\
//
//		<< ","<<room.dSetting_dH0<<","<<room.dSetting_dChiller_K<<","<<room.dSetting_dCT_MinHz<<","<<room.dSetting_dCT_eff_Correction\
//		<< ","<<room.dSetting_dElectricPercent<<","<<room.dSetting_dApproachT_Add<<","<<room.dSetting_dApproachT_Subtract<<","<<room.dSetting_dDiffFlow\
//
//		<< ","<<room.dSetting_dVSD_Value_PRI<<","<<room.dSetting_dVSD_Value_SEC<<","<<room.dSetting_dVSD_Value_CT<<","<<vsd_coolingtower\
//		<< ","<<room.dSetting_fChillPumpChiller_CountRatio<<","<<ct_outT_control<<","<<deltaT_control<<","<<room.dSetting_dDeltaT_Chill<<","<<room.dSetting_dDeltaT_Cool\
//		<< ")"; 
//
//	sql_ = sqlstream.str();
//
//	return true;
//}
//bool CSqliteRW::ReadChillerPlantRoom(CSqliteAcess &sqliteAcess,gCHPlantRoom &room,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from chillerplantroom;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//
//		room.iCPR_ID                             	= sqliteAcess.getColumn_Int(0);
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		room.strCPR_NAME                         	= name;
//		room.iOF_PROJECT_ID                      	= sqliteAcess.getColumn_Int(2);
//		room.dSetting_Max_LEWT 				  		= sqliteAcess.getColumn_Double(3);
//		room.dSetting_Min_LEWT 				  		= sqliteAcess.getColumn_Double(4);
//		room.dSetting_Max_ECWT 				  		= sqliteAcess.getColumn_Double(5);
//		room.dSetting_Min_ECWT 				  		= sqliteAcess.getColumn_Double(6);
//		room.dSetting_Max_Chilled_DT             	= sqliteAcess.getColumn_Double(7);
//		room.dSetting_Max_Cooling_DT             	= sqliteAcess.getColumn_Double(8);
//		room.dSetting_Min_Chiller_LoadRatio      	= sqliteAcess.getColumn_Double(9);
//		room.dSetting_Min_Pump_FlowRatio         	= sqliteAcess.getColumn_Double(10);
//		room.dSetting_Min_Tower_LoadRatio        	= sqliteAcess.getColumn_Double(11);
//		room.dSetting_Sys_Min_ChillFlow          	= sqliteAcess.getColumn_Double(12);
//		room.dSetting_Sys_Min_CoolFlow           	= sqliteAcess.getColumn_Double(13);
//		CString   type_temp(const_cast<char*>(sqliteAcess.getColumn_Text(14)));
//		wstring   type = type_temp.GetString();
//		room.strHVACType                         	= type;
//		room.dSChillOther                        	= sqliteAcess.getColumn_Double(15);
//		room.dSCoolOther                         	= sqliteAcess.getColumn_Double(16);
//		room.iRoomType                           	= sqliteAcess.getColumn_Int(17);
//		CString   PGP_temp(const_cast<char*>(sqliteAcess.getColumn_Text(18)));
//		wstring   PGP = PGP_temp.GetString();
//		room.strPumpGroup_Param                     = PGP;
//		room.dTowerChillerCountRatio  				= sqliteAcess.getColumn_Double(19);
//		room.dPumpChillerCountRatio   				= sqliteAcess.getColumn_Double(20);
//		room.bRatioUseable            				= sqliteAcess.getColumn_Int(21);
//		room.dSetting_Common_LEWT     				= sqliteAcess.getColumn_Double(22);
//		room.bVSD_CoolPump            				= sqliteAcess.getColumn_Int(23);
//		room.bVSD_Pri_ChillPump       				= sqliteAcess.getColumn_Int(24);
//		room.bVSD_Sec_ChillPump       				= sqliteAcess.getColumn_Int(25);
//		room.dVSD_Value               				= sqliteAcess.getColumn_Double(26);
//		room.dSetting_CoolPump_MinPowerRatio		= sqliteAcess.getColumn_Double(27);
//		room.dSetting_PriChillPump_MinPowerRatio	= sqliteAcess.getColumn_Double(28);
//		room.dSetting_SecChillPump_MinPowerRatio	= sqliteAcess.getColumn_Double(29);
//		room.dSetting_Tower_PowerIndex				= sqliteAcess.getColumn_Double(30);
//		room.dSetting_dH0							= sqliteAcess.getColumn_Double(31);
//		room.dSetting_dChiller_K					= sqliteAcess.getColumn_Double(32);
//		room.dSetting_dCT_MinHz						= sqliteAcess.getColumn_Double(33);
//		room.dSetting_dCT_eff_Correction			= sqliteAcess.getColumn_Double(34);
//		room.dSetting_dElectricPercent				= sqliteAcess.getColumn_Double(35);
//		room.dSetting_dApproachT_Add				= sqliteAcess.getColumn_Double(36);
//		room.dSetting_dApproachT_Subtract			= sqliteAcess.getColumn_Double(37);
//		room.dSetting_dDiffFlow						= sqliteAcess.getColumn_Double(38);
//		room.dSetting_dVSD_Value_PRI				= sqliteAcess.getColumn_Double(39);
//		room.dSetting_dVSD_Value_SEC             	= sqliteAcess.getColumn_Double(40);
//		room.dSetting_dVSD_Value_CT              	= sqliteAcess.getColumn_Double(41);
//		room.dSetting_bVSD_CoolingTower          	= sqliteAcess.getColumn_Int(42);
//		room.dSetting_fChillPumpChiller_CountRatio	= sqliteAcess.getColumn_Double(43);
//		room.dSetting_bCT_OutT_Control           	= sqliteAcess.getColumn_Int(44);
//		room.dSetting_bDeltaT_Control            	= sqliteAcess.getColumn_Int(45);
//		room.dSetting_dDeltaT_Chill              	= sqliteAcess.getColumn_Double(46);
//		room.dSetting_dDeltaT_Cool               	= sqliteAcess.getColumn_Double(47);
//	}
//
//
//	return true;
//}
//bool CSqliteRW::SaveChiller(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//	gRoom_Contain_Chiller *pChillerData = new gRoom_Contain_Chiller();
//
//	sqliteAcess.BeginTransaction();
//	for (int nIndex = 0; nIndex<pRoom->m_pChillerList.size(); nIndex++)
//	{
//		pChillerData->iChillerID			= pRoom->m_pChillerList[nIndex]->GetProperty()->iChillerID;
//		pChillerData->iChillerID_In_Room	= pRoom->m_pChillerList[nIndex]->GetIDInRoom();
//		pRoom->m_pChillerList[nIndex]->GetEquipmentName(pChillerData->strCHillerName_InRoom); 
//		pChillerData->iOF_CPR_ID			= pRoom->m_pChillerList[nIndex]->GetRoomID();
//		pChillerData->bEnabled				= pRoom->m_pChillerList[nIndex]->GetEnabled();
//		pChillerData->iX					= pRoom->m_pChillerList[nIndex]->GetEqmtPos()->nPosX;
//		pChillerData->iY					= pRoom->m_pChillerList[nIndex]->GetEqmtPos()->nPosY;
//		pChillerData->iChillerModelID		= pRoom->m_pChillerList[nIndex]->GetChillerModel()->GetModelID();
//		pChillerData->iPlcCtrl_id			= pRoom->m_pChillerList[nIndex]->GetPlcControlID();
//		pChillerData->iChillerComID			= pRoom->m_pChillerList[nIndex]->GetCommSlaveID();
//		pChillerData->fRotateAngel			= pRoom->m_pChillerList[nIndex]->m_fRotateAngel;
//		pChillerData->fScale				= pRoom->m_pChillerList[nIndex]->m_fScale;
//		pChillerData->fSChill				= pRoom->m_pChillerList[nIndex]->GetS_Chill();
//		pChillerData->fSCool				= pRoom->m_pChillerList[nIndex]->GetS_Cool();
//		pChillerData->fChillerRate			= pRoom->m_pChillerList[nIndex]->Get_ChillerRate();
//		pChillerData->iHeatPump				= pRoom->m_pChillerList[nIndex]->Get_IsHeatPump();
//		
//		GetEquipmentSql(Type_CHILLER,pChillerData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//	delete pChillerData;
//	pChillerData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadChiller(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Chiller> &chllerlist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_chiller;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoom_Contain_Chiller MyChiller;
//
//		MyChiller.iChillerID_In_Room = sqliteAcess.getColumn_Int(0);
//
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyChiller.strCHillerName_InRoom    = name;
//
//		MyChiller.iChillerID  = sqliteAcess.getColumn_Int(2);
//		MyChiller.iOF_CPR_ID  = sqliteAcess.getColumn_Int(3);
//		MyChiller.bEnabled    = sqliteAcess.getColumn_Int(4);
//		MyChiller.iX		  = sqliteAcess.getColumn_Int(5);
//		MyChiller.iY		  = sqliteAcess.getColumn_Int(6);
//		MyChiller.iChillerModelID		= sqliteAcess.getColumn_Int(7);
//		MyChiller.iChillerComID			= sqliteAcess.getColumn_Int(8);
//		MyChiller.iPlcCtrl_id			= sqliteAcess.getColumn_Int(9);
//		MyChiller.fRotateAngel			= sqliteAcess.getColumn_Double(10);
//		MyChiller.fScale				= sqliteAcess.getColumn_Double(11);
//		MyChiller.fSChill				= sqliteAcess.getColumn_Double(12);
//		MyChiller.fSCool				= sqliteAcess.getColumn_Double(13);
//		MyChiller.fChillerRate			= sqliteAcess.getColumn_Double(14);
//		MyChiller.iHeatPump				= sqliteAcess.getColumn_Int(15);
//
//		chllerlist.push_back(MyChiller);
//	}
//
//	return true;
//}
//bool CSqliteRW::SavePump(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//
//	unsigned int nIndex = 0;
//	gRoom_Contain_Pump *pPumpData = new gRoom_Contain_Pump();
//
//	sqliteAcess.BeginTransaction();
//
//	for (nIndex = 0; nIndex<pRoom->m_pPumpList.size(); nIndex++)
//	{
//		pPumpData->iPumpID           = pRoom->m_pPumpList[nIndex]->GetProperty()->iPumpID;
//		pPumpData->iPumpID_In_Room   = pRoom->m_pPumpList[nIndex]->GetIDInRoom();
//		pRoom->m_pPumpList[nIndex]->GetEquipmentName(pPumpData->strPumpName_InRoom);
//		pPumpData->iOF_CPR_ID        = pRoom->m_pPumpList[nIndex]->GetRoomID();
//		pPumpData->bEnabled          = pRoom->m_pPumpList[nIndex]->GetEnabled();
//		pPumpData->bStandby          = pRoom->m_pPumpList[nIndex]->GetStandby();
//		pPumpData->iPump_Location    = pRoom->m_pPumpList[nIndex]->GetLocation();
//		pPumpData->iX                = pRoom->m_pPumpList[nIndex]->GetEqmtPos()->nPosX;
//		pPumpData->iY                = pRoom->m_pPumpList[nIndex]->GetEqmtPos()->nPosY;
//		pPumpData->iPumpModelID      = pRoom->m_pPumpList[nIndex]->GetPumpModel()->GetModelID();
//		pPumpData->iPlcCtrl_id		 = pRoom->m_pPumpList[nIndex]->GetPlcControlID();
//		pPumpData->bWithVSD			 = pRoom->m_pPumpList[nIndex]->GetVSD();
//		pPumpData->fRotateAngel		 = pRoom->m_pPumpList[nIndex]->m_fRotateAngel;
//		pPumpData->fScale			 = pRoom->m_pPumpList[nIndex]->m_fScale;
//
//		GetEquipmentSql(Type_PUMP,pPumpData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//
//	delete pPumpData;
//	pPumpData = NULL;
//
//	return true;
//}
//
//bool CSqliteRW::ReadPump(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Pump> &pumplist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_pump;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoom_Contain_Pump MyPump;
//
//		MyPump.iPumpID_In_Room = sqliteAcess.getColumn_Int(0);
//
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyPump.strPumpName_InRoom    = name;
//
//		MyPump.iPumpID		= sqliteAcess.getColumn_Int(2);
//		MyPump.iOF_CPR_ID	= sqliteAcess.getColumn_Int(3);
//		MyPump.iPump_Location		= sqliteAcess.getColumn_Int(4);
//		MyPump.bEnabled				= sqliteAcess.getColumn_Int(5);
//		MyPump.bStandby				= sqliteAcess.getColumn_Int(6);
//		MyPump.iX					= sqliteAcess.getColumn_Int(7);
//		MyPump.iY					= sqliteAcess.getColumn_Int(8);
//		MyPump.iPumpModelID			= sqliteAcess.getColumn_Int(9);
//		MyPump.iPlcCtrl_id			= sqliteAcess.getColumn_Int(10);
//		MyPump.bWithVSD				= sqliteAcess.getColumn_Int(11);
//		MyPump.fRotateAngel			= sqliteAcess.getColumn_Double(12);
//		MyPump.fScale				= sqliteAcess.getColumn_Double(13);
//
//		pumplist.push_back(MyPump);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveTower(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//	unsigned int nIndex = 0;
//	gRoom_Contain_CoolingTower *pCTData = new gRoom_Contain_CoolingTower();
//
//	sqliteAcess.BeginTransaction();
//	for (nIndex = 0; nIndex<pRoom->m_pCoolingTowerList.size(); nIndex++)
//	{
//		pCTData->iTowerID         = pRoom->m_pCoolingTowerList[nIndex]->GetProperty()->iCTID;
//		pCTData->iTowerID_In_Room = pRoom->m_pCoolingTowerList[nIndex]->GetIDInRoom();
//		pRoom->m_pCoolingTowerList[nIndex]->GetEquipmentName( pCTData->strTowerName_InRoom );
//		pCTData->iOF_CPR_ID       = pRoom->m_pCoolingTowerList[nIndex]->GetRoomID();
//		pCTData->bEnabled         = pRoom->m_pCoolingTowerList[nIndex]->GetEnabled();
//		pCTData->dMax_Capacity    = pRoom->m_pCoolingTowerList[nIndex]->GetMaxCapacity();
//		pCTData->iX               = pRoom->m_pCoolingTowerList[nIndex]->GetEqmtPos()->nPosX;
//		pCTData->iY               = pRoom->m_pCoolingTowerList[nIndex]->GetEqmtPos()->nPosY;
//		pCTData->iCTModelID		  = pRoom->m_pCoolingTowerList[nIndex]->m_pModel->GetModelID();
//		pCTData->iPlcCtrl_id	  = pRoom->m_pCoolingTowerList[nIndex]->GetPlcControlID();
//		pCTData->bWithVSD		  = pRoom->m_pCoolingTowerList[nIndex]->m_bVSD;
//		pCTData->fRotateAngel     = pRoom->m_pCoolingTowerList[nIndex]->m_fRotateAngel;
//		pCTData->fScale           = pRoom->m_pCoolingTowerList[nIndex]->m_fScale;
//		pCTData->fSCool           = pRoom->m_pCoolingTowerList[nIndex]->GetS();
//
//		GetEquipmentSql(Type_COOLINGTOWER,pCTData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//	delete pCTData;
//	pCTData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadTower(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_CoolingTower> &towerlist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_coolingtower;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoom_Contain_CoolingTower MyTower;
//
//		MyTower.iTowerID_In_Room = sqliteAcess.getColumn_Int(0);
//
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyTower.strTowerName_InRoom    = name;
//
//		MyTower.iTowerID			= sqliteAcess.getColumn_Int(2);
//		MyTower.iOF_CPR_ID			= sqliteAcess.getColumn_Int(3);
//		MyTower.bEnabled			= sqliteAcess.getColumn_Int(4);
//		MyTower.dMax_Capacity		= sqliteAcess.getColumn_Double(5);
//		MyTower.iX					= sqliteAcess.getColumn_Int(6);
//		MyTower.iY					= sqliteAcess.getColumn_Int(7);
//		MyTower.iCTModelID			= sqliteAcess.getColumn_Int(8);
//		MyTower.iPlcCtrl_id			= sqliteAcess.getColumn_Int(9);
//		MyTower.bWithVSD			= sqliteAcess.getColumn_Int(10);
//		MyTower.fRotateAngel		= sqliteAcess.getColumn_Int(11);
//		MyTower.fScale				= sqliteAcess.getColumn_Double(12);
//		MyTower.fSCool				= sqliteAcess.getColumn_Double(13);
//
//		towerlist.push_back(MyTower);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveCorner(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//	unsigned int nIndex = 0;
//	gRoom_Contain_Corner *pCornerData = new gRoom_Contain_Corner();
//
//	sqliteAcess.BeginTransaction();
//
//	for (nIndex = 0; nIndex<pRoom->m_pCornerList.size(); nIndex++)
//	{
//		pCornerData->iCorner_ID          = pRoom->m_pCornerList[nIndex]->GetProperty()->iCornerID;
//		pCornerData->iCorner_ID_InRoom   = pRoom->m_pCornerList[nIndex]->GetIDInRoom();
//		pCornerData->iOF_CPR_ID          = pRoom->m_pCornerList[nIndex]->GetRoomID();
//		pCornerData->iX                  = pRoom->m_pCornerList[nIndex]->GetEqmtPos()->nPosX;
//		pCornerData->iY                  = pRoom->m_pCornerList[nIndex]->GetEqmtPos()->nPosY;
//		
//		GetEquipmentSql(Type_CORNER,pCornerData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//	delete pCornerData;
//	pCornerData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadCorner(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Corner> &cornerlist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_corner;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoom_Contain_Corner MyCorner;
//		MyCorner.iCorner_ID_InRoom = sqliteAcess.getColumn_Int(0);
//		MyCorner.iCorner_ID			= sqliteAcess.getColumn_Int(1);
//		MyCorner.iX					= sqliteAcess.getColumn_Int(2);
//		MyCorner.iY					= sqliteAcess.getColumn_Int(3);
//		MyCorner.iOF_CPR_ID			= sqliteAcess.getColumn_Int(4);
//
//		cornerlist.push_back(MyCorner);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveHx(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//	unsigned int nIndex = 0;
//	gRoom_Contain_Hx *pHxData = new gRoom_Contain_Hx();
//	for (nIndex = 0; nIndex<pRoom->m_pHeatExchangerList.size(); nIndex++)
//	{
//		pHxData->iHxID          = pRoom->m_pHeatExchangerList[nIndex]->GetProperty()->iHxID;
//		pHxData->iHxID_In_Room  = pRoom->m_pHeatExchangerList[nIndex]->GetIDInRoom();
//		pRoom->m_pHeatExchangerList[nIndex]->GetEquipmentName(pHxData->strHxName_InRoom);
//		pHxData->iOF_CPR_ID     = pRoom->m_pHeatExchangerList[nIndex]->GetRoomID();
//		pHxData->bEnabled       = pRoom->m_pHeatExchangerList[nIndex]->GetEnabled();
//		pHxData->dMax_Capacity  = pRoom->m_pHeatExchangerList[nIndex]->GetMax_Capacity();
//		pHxData->iX             = pRoom->m_pHeatExchangerList[nIndex]->GetEqmtPos()->nPosX;
//		pHxData->iY             = pRoom->m_pHeatExchangerList[nIndex]->GetEqmtPos()->nPosY;
//		pHxData->iPlcCtrl_id    = pRoom->m_pHeatExchangerList[nIndex]->GetPlcControlID();
//		pHxData->fRotateAngel   = pRoom->m_pHeatExchangerList[nIndex]->m_fRotateAngel;
//		pHxData->fScale         = pRoom->m_pHeatExchangerList[nIndex]->m_fScale;
//
//		GetEquipmentSql(Type_HX,pHxData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//
//	}
//	delete pHxData;
//	pHxData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadHx(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Hx> &hxlist,string &sql_)
//{
//	//ostringstream sqlstream;
//	//sqlstream << "select * from room_contain_corner;";
//	//sql_ = sqlstream.str();
//	//char *ex_sql = const_cast<char*>(sql_.c_str());
//	//int re = sqliteAcess.SqlQuery(ex_sql);
//
//	//while(true)
//	//{
//	//	if(SQLITE_ROW != sqliteAcess.SqlNext())
//	//	{
//	//		break;
//	//	}
//	//	gRoom_Contain_Corner MyCorner;
//	//	MyCorner.iCorner_ID_InRoom = sqliteAcess.getColumn_Int(0);
//	//	MyCorner.iCorner_ID			= sqliteAcess.getColumn_Int(1);
//	//	MyCorner.iX					= sqliteAcess.getColumn_Int(2);
//	//	MyCorner.iY					= sqliteAcess.getColumn_Int(3);
//	//	MyCorner.iOF_CPR_ID			= sqliteAcess.getColumn_Int(4);
//
//	//	cornerlist.push_back(MyCorner);
//	//}
//
//	return true;
//}
//bool CSqliteRW::SaveTerminal(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//
//	unsigned int nIndex = 0;
//	gRoom_Contain_Terminal *pTerminalData = new gRoom_Contain_Terminal();
//
//
//	for (nIndex = 0; nIndex<pRoom->m_pTerminalList.size(); nIndex++)
//	{
//		pTerminalData->iTerminal_ID        = pRoom->m_pTerminalList[nIndex]->GetProperty()->iTerminal_id;
//		pTerminalData->iTerminal_ID_InRoom = pRoom->m_pTerminalList[nIndex]->GetIDInRoom();
//		pTerminalData->iOF_CPR_ID          = pRoom->m_pTerminalList[nIndex]->GetRoomID();
//		pTerminalData->iX                  = pRoom->m_pTerminalList[nIndex]->GetEqmtPos()->nPosX;
//		pTerminalData->iY                  = pRoom->m_pTerminalList[nIndex]->GetEqmtPos()->nPosY;
//		pTerminalData->fRotateAngel        = pRoom->m_pTerminalList[nIndex]->m_fRotateAngel;
//		pTerminalData->fScale              = pRoom->m_pTerminalList[nIndex]->m_fScale;
//		pTerminalData->fSChill             = pRoom->m_pTerminalList[nIndex]->GetS();
//
//		GetEquipmentSql(Type_TERMINAL,pTerminalData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	delete pTerminalData;
//	pTerminalData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadTerminal(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Terminal> &terminallist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_terminal;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoom_Contain_Terminal MyTerminal;
//		MyTerminal.iTerminal_ID_InRoom	= sqliteAcess.getColumn_Int(0);
//		MyTerminal.iTerminal_ID			= sqliteAcess.getColumn_Int(1);
//		MyTerminal.iX					= sqliteAcess.getColumn_Int(2);
//		MyTerminal.iY					= sqliteAcess.getColumn_Int(3);
//		MyTerminal.iOF_CPR_ID			= sqliteAcess.getColumn_Int(4);
//		MyTerminal.fRotateAngel			= sqliteAcess.getColumn_Double(5);
//		MyTerminal.fScale				= sqliteAcess.getColumn_Double(6);
//		MyTerminal.fSChill				= sqliteAcess.getColumn_Double(7);
//		terminallist.push_back(MyTerminal);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveWaterSoil(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//
//	unsigned int nIndex = 0;
//	WaterSoil_Entry_InRoom *pWaterSoilData = new WaterSoil_Entry_InRoom();
//
//	sqliteAcess.BeginTransaction();
//	for (nIndex = 0; nIndex<pRoom->m_pWaterSoilList.size(); nIndex++)
//	{
//		pWaterSoilData->iId					= pRoom->m_pWaterSoilList[nIndex]->GetIDInRoom();
//		pWaterSoilData->iType				= EQMType_WATERSOIL;
//		pWaterSoilData->dTemp_Winter		= pRoom->m_pWaterSoilList[nIndex]->GetT_Winter();
//		pWaterSoilData->dTemp_Summer		= pRoom->m_pWaterSoilList[nIndex]->GetT_Summer();
//		pWaterSoilData->dTGrade				= pRoom->m_pWaterSoilList[nIndex]->GetT_Grade();
//		pWaterSoilData->iOf_CPR_ID          = pRoom->m_pWaterSoilList[nIndex]->GetRoomID();
//
//		pWaterSoilData->iX                  = pRoom->m_pWaterSoilList[nIndex]->m_Pt.nPosX;
//		pWaterSoilData->iY                  = pRoom->m_pWaterSoilList[nIndex]->m_Pt.nPosY;
//		pWaterSoilData->iWaterSoilID        = pRoom->m_pWaterSoilList[nIndex]->GetProperty()->iWSID;
//		pWaterSoilData->iWSModelID          = pRoom->m_pWaterSoilList[nIndex]->m_pModel->GetModelID();
//
//		GetEquipmentSql(Type_WATERSOIL,pWaterSoilData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//	delete pWaterSoilData;
//	pWaterSoilData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadWaterSoil(CSqliteAcess &sqliteAcess,std::vector<WaterSoil_Entry_InRoom> &wslist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_watersoil;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		WaterSoil_Entry_InRoom MyWS;
//		MyWS.iId						= sqliteAcess.getColumn_Int(0);
//		MyWS.iType						= sqliteAcess.getColumn_Int(1);
//		MyWS.dTemp_Winter				= sqliteAcess.getColumn_Double(2);
//		MyWS.dTemp_Summer				= sqliteAcess.getColumn_Double(3);
//		MyWS.dTGrade					= sqliteAcess.getColumn_Double(4);
//		MyWS.iOf_CPR_ID					= sqliteAcess.getColumn_Int(5);
//		MyWS.iX							= sqliteAcess.getColumn_Int(6);
//		MyWS.iY							= sqliteAcess.getColumn_Int(7);
//		MyWS.iWaterSoilID				= sqliteAcess.getColumn_Int(8);
//		MyWS.iWSModelID					= sqliteAcess.getColumn_Int(9);
//
//		wslist.push_back(MyWS);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveValve(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//
//	unsigned int nIndex = 0;
//	gRoom_Contain_Valve *pValveData = new gRoom_Contain_Valve();
//
//	sqliteAcess.BeginTransaction();
//
//	for (nIndex = 0; nIndex<pRoom->m_pValveList.size(); nIndex++)
//	{
//		pValveData->iValveID            = pRoom->m_pValveList[nIndex]->GetProperty()->iValveID;
//		pValveData->iValveID_In_Room    = pRoom->m_pValveList[nIndex]->GetIDInRoom();
//		pRoom->m_pValveList[nIndex]->GetEquipmentName(pValveData->strValveName_InRoom);
//		pValveData->iOF_CPR_ID          = pRoom->m_pValveList[nIndex]->GetRoomID();
//		pValveData->bEnabled            = pRoom->m_pValveList[nIndex]->GetEnabled();
//		pValveData->iX                  = pRoom->m_pValveList[nIndex]->GetEqmtPos()->nPosX;
//		pValveData->iY                  = pRoom->m_pValveList[nIndex]->GetEqmtPos()->nPosY;
//		pValveData->iPlcCtrl_id			= pRoom->m_pValveList[nIndex]->GetPlcControlID();
//		pValveData->fRotateAngel        = pRoom->m_pValveList[nIndex]->m_fRotateAngel;
//		pValveData->fScale              = pRoom->m_pValveList[nIndex]->m_fScale;
//
//		GetEquipmentSql(Type_VALVE,pValveData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//	delete pValveData;
//	pValveData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadValve(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Valve> &valvelist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_valve;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoom_Contain_Valve MyValve;
//
//		MyValve.iValveID_In_Room = sqliteAcess.getColumn_Int(0);
//
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyValve.strValveName_InRoom    = name;
//
//		MyValve.iValveID			= sqliteAcess.getColumn_Int(2);
//		MyValve.iOF_CPR_ID			= sqliteAcess.getColumn_Int(3);
//		MyValve.bEnabled			= sqliteAcess.getColumn_Int(4);
//		MyValve.iX					= sqliteAcess.getColumn_Double(5);
//		MyValve.iY					= sqliteAcess.getColumn_Int(6);
//		MyValve.iPlcCtrl_id			= sqliteAcess.getColumn_Int(7);
//		MyValve.fRotateAngel		= sqliteAcess.getColumn_Int(8);
//		MyValve.fScale				= sqliteAcess.getColumn_Double(9);
//
//		valvelist.push_back(MyValve);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveTank(CSqliteAcess &sqliteAcess,string &sql_)
//{
//
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//
//	unsigned int nIndex = 0;
//	Tank_Entry_InRoom *pTankData = new Tank_Entry_InRoom();
//
//	sqliteAcess.BeginTransaction();
//
//	for (nIndex = 0; nIndex<pRoom->m_pTankList.size(); nIndex++)
//	{
//		pTankData->iId_In_Room			= pRoom->m_pTankList[nIndex]->GetIDInRoom();
//		pTankData->strName_In_Room		=_T("");
//		pTankData->iOf_CPR_ID			= pRoom->m_pTankList[nIndex]->GetRoomID();
//		pTankData->iX					= pRoom->m_pTankList[nIndex]->m_Pt.nPosX;
//		pTankData->iY					= pRoom->m_pTankList[nIndex]->m_Pt.nPosY;
//		pTankData->iTankID_In_Lib		= pRoom->m_pTankList[nIndex]->GetProperty()->iTankID;
//
//		GetEquipmentSql(Type_Tank,pTankData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//	delete pTankData;
//	pTankData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadTank(CSqliteAcess &sqliteAcess,std::vector<Tank_Entry_InRoom> &tanklist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_tank;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		Tank_Entry_InRoom MyTank;
//
//		MyTank.iId_In_Room = sqliteAcess.getColumn_Int(0);
//
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyTank.strName_In_Room    = name;
//
//		MyTank.iOf_CPR_ID			= sqliteAcess.getColumn_Int(2);
//		MyTank.iX					= sqliteAcess.getColumn_Double(3);
//		MyTank.iY					= sqliteAcess.getColumn_Int(4);
//		MyTank.iTankID_In_Lib		= sqliteAcess.getColumn_Double(5);
//
//		tanklist.push_back(MyTank);
//	}
//
//	return true;
//}
//bool CSqliteRW::SavePipe(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//
//	unsigned int nIndex = 0;
//	gRoom_Contain_Pipe *pPipeData = new gRoom_Contain_Pipe();
//
//	sqliteAcess.BeginTransaction();
//	for (nIndex = 0; nIndex<pRoom->m_pPipeList.size(); nIndex++)
//	{
//		pPipeData->iPipeID            = pRoom->m_pPipeList[nIndex]->GetProperty()->iPipeID;
//		pPipeData->iPipeID_In_Room    = pRoom->m_pPipeList[nIndex]->GetIDInRoom();
//		pRoom->m_pPipeList[nIndex]->GetEquipmentName( pPipeData->strPipeName_InRoom );
//		pPipeData->iOF_CPR_ID         = pRoom->m_pPipeList[nIndex]->GetRoomID();
//		pPipeData->bEnabled           = pRoom->m_pPipeList[nIndex]->GetEnabled();
//		pPipeData->from_service_id    = pRoom->m_pPipeList[nIndex]->GetConnectionParam().From_Device_ID;
//		pPipeData->from_service_port  = pRoom->m_pPipeList[nIndex]->GetConnectionParam().From_Device_Port;
//		pPipeData->from_service_type  = pRoom->m_pPipeList[nIndex]->GetConnectionParam().From_Device_Type;     
//		pPipeData->to_service_id      = pRoom->m_pPipeList[nIndex]->GetConnectionParam().To_Device_ID;
//		pPipeData->to_service_port    = pRoom->m_pPipeList[nIndex]->GetConnectionParam().To_Device_Port;
//		pPipeData->to_service_type    = pRoom->m_pPipeList[nIndex]->GetConnectionParam().To_Device_Type;
//		
//		
//		GetEquipmentSql(Type_PIPE,pPipeData,sql_);
//		char *ex_sql = const_cast<char*>(sql_.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//
//	sqliteAcess.CommitTransaction();
//
//	delete pPipeData;
//	pPipeData = NULL;
//
//	return true;
//}
//bool CSqliteRW::ReadPipe(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Pipe> &pipelist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from room_contain_pipe;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoom_Contain_Pipe MyPipe;
//
//		MyPipe.iPipeID_In_Room = sqliteAcess.getColumn_Int(0);
//
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyPipe.strPipeName_InRoom    = name;
//
//		MyPipe.iPipeID				= sqliteAcess.getColumn_Int(2);
//		MyPipe.iOF_CPR_ID			= sqliteAcess.getColumn_Int(3);
//		MyPipe.bEnabled				= sqliteAcess.getColumn_Double(4);
//		MyPipe.from_service_id		= sqliteAcess.getColumn_Int(5);
//		MyPipe.from_service_type	= sqliteAcess.getColumn_Int(6);
//		MyPipe.from_service_port	= sqliteAcess.getColumn_Int(7);
//
//		MyPipe.to_service_id		= sqliteAcess.getColumn_Int(8);
//		MyPipe.to_service_type		= sqliteAcess.getColumn_Int(9);
//		MyPipe.to_service_port		= sqliteAcess.getColumn_Int(10);
//
//		pipelist.push_back(MyPipe);
//	}
//
//	return true;
//}
//
//bool CSqliteRW::SaveLoad(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//	if (pRoom == NULL)
//	{
//		return false;
//	}
//	std::vector<gRoomLoad*> record = pRoom->m_RoomLoad.m_VecLoad;
//	gRoomLoad* data;
//	string str;
//	string dt;
//	sqliteAcess.BeginTransaction();
//	for (vector<gRoomLoad*>::const_iterator it = record.begin(); it != record.end(); it++)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into chiller_plant_room_load_define values";
//
//		data = *it;
//		dt = SystemTimeToString(data->tmTime);
//		sqlstream << "('" << dt << "'," << data->dCoolingLoad << "," << data->dHeatingLoad << "," \
//			<< data->dTotal_Flow << "," << data->dTotal_T_Supply << "," << data->dTotal_T_Return << "," << data->iRoomID << "," << data->dTdry << "," \
//			<< data->dTwet <<","<<data->fPowerBill <<","<<data->fPowerBill <<","<<data->dCoolingLoad1<<","<<data->dCoolingLoad2<<","<<data->dCoolingLoad3<<","<<data->dCoolingLoad4<< ")";
//		if (it != record.end() - 1)			
//			sqlstream << ";" ;	
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::ReadLoad(CSqliteAcess &sqliteAcess,std::vector<gRoomLoad> &loadlist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from chiller_plant_room_load_define;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gRoomLoad MyLoad;
//
//		SYSTEMTIME st;
//		tstring strTemp = Project::Tools::UTF8ToWideChar(sqliteAcess.getColumn_Text(0));
//		Project::Tools::String_To_SysTime(strTemp , st );
//		MyLoad.tmTime			= st;
//
//		MyLoad.dCoolingLoad		= sqliteAcess.getColumn_Double(1);
//
//		MyLoad.dHeatingLoad		= sqliteAcess.getColumn_Double(2);
//		MyLoad.dTotal_Flow		= sqliteAcess.getColumn_Double(3);
//		MyLoad.dTotal_T_Supply	= sqliteAcess.getColumn_Double(4);
//		MyLoad.dTotal_T_Return	= sqliteAcess.getColumn_Double(5);
//		MyLoad.iRoomID			= sqliteAcess.getColumn_Double(6);
//		MyLoad.dTdry			= sqliteAcess.getColumn_Double(7);
//
//		MyLoad.dTwet			= sqliteAcess.getColumn_Double(8);
//		MyLoad.fPowerBill		= sqliteAcess.getColumn_Double(9);
//
//		MyLoad.dCoolingLoad1	= sqliteAcess.getColumn_Double(11);
//		MyLoad.dCoolingLoad2	= sqliteAcess.getColumn_Double(12);
//		MyLoad.dCoolingLoad3	= sqliteAcess.getColumn_Double(13);
//		MyLoad.dCoolingLoad4	= sqliteAcess.getColumn_Double(14);
//
//		loadlist.push_back(MyLoad);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveChillerLibAndModel(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	//查询相关设备的属性及模型，并存储
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//
//	std::vector<gCHLib>		chLibList,chLibList_temp; 
//	std::vector<gCH_Model>	chModelList,chModelList_temp;
//
//	for (int i=0;i<pRoom->m_pChillerList.size();++i)
//	{
//		chLibList.push_back(*pRoom->m_pChillerList[i]->GetProperty());
//		chModelList.push_back(*pRoom->m_pChillerList[i]->GetChillerModel()->Get_ModelInfo());
//	}
//	//排序
//	int n = chLibList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gCHLib temp=chLibList[i];
//		while (j>=0 && chLibList[j].iChillerID >temp.iChillerID)
//		{
//			chLibList[j+1]=chLibList[j];
//			j--;
//		}
//		chLibList[j+1]=temp;
//	}
//
//	n = chModelList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gCH_Model temp=chModelList[i];
//		while (j>=0 && chModelList[j].iChillerModelID >temp.iChillerModelID)
//		{
//			chModelList[j+1]=chModelList[j];
//			j--;
//		}
//		chModelList[j+1]=temp;
//	}
//	//去掉重复的
//	if (chLibList.empty())
//	{
//		return false;
//	}
//	if (chModelList.empty())
//	{
//		return false;
//	}
//	chLibList_temp.push_back(chLibList[0]);
//	for (int i=1;i<chLibList.size();++i)
//	{
//		if (chLibList[i-1].iChillerID != chLibList[i].iChillerID)
//		{
//			chLibList_temp.push_back(chLibList[i]);
//		}
//	}
//
//	chModelList_temp.push_back(chModelList[0]);
//	for (int i=1;i<chModelList.size();++i)
//	{
//		if (chModelList[i-1].iChillerModelID != chModelList[i].iChillerModelID)
//		{
//			chModelList_temp.push_back(chModelList[i]);
//		}
//	}
//	//存储到sqlite数据库中
//	CSqliteRW::savechillerlib(sqliteAcess,sql_,chLibList_temp);
//	CSqliteRW::savechillermodel(sqliteAcess,sql_,chModelList_temp);
//
//	return true;
//}
//
//bool CSqliteRW::savechillerlib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCHLib> &chLibList)
//{
//	string str;
//	gCHLib data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<chLibList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into chillerlib values";
//
//		data = chLibList[i];
//
//		_bstr_t t = data.strModelName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t b = data.strBrand.c_str(); 
//		char* pcharb = (char*)b;  
//		string br = pcharb;
//
//		sqlstream << "(" << data.iChillerID << ",'" << name << "','" <<br<< "'," \
//			<< data.iType << "," << data.iRefrig << "," << data.iMotor_Start_Type << "," << data.dRating_CoolingCap<< "," << data.dRating_HeatCap << "," \
//			<< data.dRating_Flow_Evap <<","<<data.dRating_Flow_Cond<<","<<data.dRating_COP<<","<<data.dRating_LEWT<<","<<data.dRating_ECWT<<","\
//			<< data.dSurge_Param1<<","<<data.dSurge_Param2<<","<<data.dSurge_Param3<<","<<data.dSurge_Param4<<","<<data.dRating_DP_Evap<<","<<data.dRating_DP_Cond<< ");";
//		
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::savechillermodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCH_Model> &chModelList)
//{
//	string str;
//	string dt;
//	gCH_Model data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<chModelList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into chiller_model values";
//
//		data = chModelList[i];
//
//		_bstr_t t = data.strCHillerModelName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t tt = data.strModel_Param.c_str(); 
//		char* ppchar = (char*)tt;  
//		string p = ppchar;
//
//		dt = SystemTimeToString(data.tmModel_Gen_Time);
//
//		sqlstream << "(" << data.iChillerModelID << ",'" << name << "'," <<data.iChillerID<< "," \
//			<< data.iType << ",'" <<dt<< "','" <<p<<  "');";
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//
//bool CSqliteRW::SavePumpLibAndModel(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	//查询相关设备的属性及模型，并存储
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//
//	std::vector<gPumpLib>		pumpLibList,pumpLibList_temp; 
//	std::vector<gPump_Model>	pumpModelList,pumpModelList_temp;
//
//	for (int i=0;i<pRoom->m_pPumpList.size();++i)
//	{
//		pumpLibList.push_back(*pRoom->m_pPumpList[i]->GetProperty());
//		pumpModelList.push_back(*pRoom->m_pPumpList[i]->GetPumpModel()->Get_ModelInfo());
//	}
//	//排序
//	int n = pumpLibList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gPumpLib temp=pumpLibList[i];
//		while (j>=0 && pumpLibList[j].iPumpID >temp.iPumpID)
//		{
//			pumpLibList[j+1]=pumpLibList[j];
//			j--;
//		}
//		pumpLibList[j+1]=temp;
//	}
//
//	n = pumpModelList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gPump_Model temp=pumpModelList[i];
//		while (j>=0 && pumpModelList[j].iPumpModelID >temp.iPumpModelID)
//		{
//			pumpModelList[j+1]=pumpModelList[j];
//			j--;
//		}
//		pumpModelList[j+1]=temp;
//	}
//	//去掉重复的
//	if (pumpLibList.empty())
//	{
//		return false;
//	}
//	if (pumpModelList.empty())
//	{
//		return false;
//	}
//	pumpLibList_temp.push_back(pumpLibList[0]);
//	for (int i=1;i<pumpLibList.size();++i)
//	{
//		if (pumpLibList[i-1].iPumpID != pumpLibList[i].iPumpID)
//		{
//			pumpLibList_temp.push_back(pumpLibList[i]);
//		}
//	}
//
//	pumpModelList_temp.push_back(pumpModelList[0]);
//	for (int i=1;i<pumpModelList.size();++i)
//	{
//		if (pumpModelList[i-1].iPumpModelID != pumpModelList[i].iPumpModelID)
//		{
//			pumpModelList_temp.push_back(pumpModelList[i]);
//		}
//	}
//	//存储到sqlite数据库中
//	CSqliteRW::savepumplib(sqliteAcess,sql_,pumpLibList_temp);
//	CSqliteRW::savepumpmodel(sqliteAcess,sql_,pumpModelList_temp);
//
//	return true;
//}
//bool CSqliteRW::savepumplib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gPumpLib> &pumpLibList)
//{
//	string str;
//	gPumpLib data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<pumpLibList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into pumplib values";
//
//		data = pumpLibList[i];
//
//		_bstr_t t = data.strModelName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t b = data.strBrand.c_str(); 
//		char* pcharb = (char*)b;  
//		string br = pcharb;
//
//		sqlstream << "(" << data.iPumpID << ",'" << name << "','" <<br<< "'," \
//			<< data.iType << "," << data.iRating_RPM << "," << data.dRating_Flow << "," << data.dRating_Head<< "," << data.dRating_Power << ");";
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::savepumpmodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gPump_Model> &pumpModelList)
//{
//	string str;
//	string dt;
//	gPump_Model data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<pumpModelList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into pump_model values";
//
//		data = pumpModelList[i];
//
//		_bstr_t t = data.strPumpModelName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t tt = data.strModel_Param.c_str(); 
//		char* ppchar = (char*)tt;  
//		string p = ppchar;
//
//		dt = SystemTimeToString(data.tmModel_Gen_Time);
//
//		sqlstream << "(" << data.iPumpModelID << ",'" << name << "'," <<data.iPumpID<< "," \
//			<< data.iType << ",'" <<dt<< "','" <<p<<  "');";
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::SaveTowerLibAndModel(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	//查询相关设备的属性及模型，并存储
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//
//	std::vector<gCTLib>		ctLibList,ctLibList_temp; 
//	std::vector<gCT_Model>	ctModelList,ctModelList_temp;
//
//	for (int i=0;i<pRoom->m_pCoolingTowerList.size();++i)
//	{
//		ctLibList.push_back(*pRoom->m_pCoolingTowerList[i]->GetProperty());
//		ctModelList.push_back(*pRoom->m_pCoolingTowerList[i]->GetCTModel()->Get_ModelInfo());
//	}
//	//排序
//	int n = ctLibList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gCTLib temp=ctLibList[i];
//		while (j>=0 && ctLibList[j].iCTID >temp.iCTID)
//		{
//			ctLibList[j+1]=ctLibList[j];
//			j--;
//		}
//		ctLibList[j+1]=temp;
//	}
//
//	n = ctModelList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gCT_Model temp=ctModelList[i];
//		while (j>=0 && ctModelList[j].iModelID >temp.iModelID)
//		{
//			ctModelList[j+1]=ctModelList[j];
//			j--;
//		}
//		ctModelList[j+1]=temp;
//	}
//	//去掉重复的
//	if (ctLibList.empty())
//	{
//		return false;
//	}
//	if (ctModelList.empty())
//	{
//		return false;
//	}
//	ctLibList_temp.push_back(ctLibList[0]);
//	for (int i=1;i<ctLibList.size();++i)
//	{
//		if (ctLibList[i-1].iCTID != ctLibList[i].iCTID)
//		{
//			ctLibList_temp.push_back(ctLibList[i]);
//		}
//	}
//
//	ctModelList_temp.push_back(ctModelList[0]);
//	for (int i=1;i<ctModelList.size();++i)
//	{
//		if (ctModelList[i-1].iModelID != ctModelList[i].iModelID)
//		{
//			ctModelList_temp.push_back(ctModelList[i]);
//		}
//	}
//	//存储到sqlite数据库中
//	CSqliteRW::savetowerlib(sqliteAcess,sql_,ctLibList_temp);
//	CSqliteRW::savetowermodel(sqliteAcess,sql_,ctModelList_temp);
//
//	return true;
//}
//bool CSqliteRW::savetowerlib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCTLib> &ctLibList)
//{
//	string str;
//	gCTLib data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<ctLibList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into coolingtowerlib values";
//
//		data = ctLibList[i];
//
//		_bstr_t t = data.strModelName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t b = data.strBrand.c_str(); 
//		char* pcharb = (char*)b;  
//		string br = pcharb;
//
//		sqlstream << "(" << data.iCTID << ",'" << name << "','" <<br<< "'," \
//			<< data.iType <<","<<data.dRating_Air_Flow<<","<<data.dRating_Water_Flow<<","<<data.dRating_TWB_Air<<","<<data.dRating_T_Water_In<<","\
//			<< data.dRating_T_Water_Out << "," << data.dPower << ","<< data.iFansCount << ");";
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::savetowermodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCT_Model> &ctModelList)
//{
//	string str;
//	string dt;
//	gCT_Model data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<ctModelList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into coolingtower_model values";
//
//		data = ctModelList[i];
//
//		_bstr_t t = data.strModelName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t tt = data.strModel_Param.c_str(); 
//		char* ppchar = (char*)tt;  
//		string p = ppchar;
//
//		dt = SystemTimeToString(data.tmModel_Gen_Time);
//
//		sqlstream << "(" << data.iModelID << ",'" << name << "'," <<data.iCoolingTowerID<< "," \
//			<< data.iType << ",'" <<dt<< "','" <<p<<  "');";
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::SaveWSLibAndModel(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	//查询相关设备的属性及模型，并存储
//	CChillerplantroom *pRoom = g_pDatabase->GetProject()->GetCurrentBuilding();
//
//	std::vector<gWSLib>				wsLibList,wsLibList_temp; 
//	std::vector<gWaterSoil_Model>	wsModelList,wsModelList_temp;
//
//	for (int i=0;i<pRoom->m_pWaterSoilList.size();++i)
//	{
//		wsLibList.push_back(*pRoom->m_pWaterSoilList[i]->GetProperty());
//		wsModelList.push_back(*pRoom->m_pWaterSoilList[i]->GetWaterSoilModel()->Get_ModelInfo());
//	}
//	//排序
//	int n = wsLibList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gWSLib temp	= wsLibList[i];
//		while (j>=0 && wsLibList[j].iWSID >temp.iWSID)
//		{
//			wsLibList[j+1]=wsLibList[j];
//			j--;
//		}
//		wsLibList[j+1]=temp;
//	}
//
//	n = wsModelList.size();
//	for (int i=1;i<n;i++)
//	{
//		int j=i-1;
//		gWaterSoil_Model temp=wsModelList[i];
//		while (j>=0 && wsModelList[j].iWaterSoilModelID >temp.iWaterSoilModelID)
//		{
//			wsModelList[j+1]=wsModelList[j];
//			j--;
//		}
//		wsModelList[j+1]=temp;
//	}
//	//去掉重复的
//	if (wsLibList.empty())
//	{
//		return false;
//	}
//	if (wsModelList.empty())
//	{
//		return false;
//	}
//	wsLibList_temp.push_back(wsLibList[0]);
//	for (int i=1;i<wsLibList.size();++i)
//	{
//		if (wsLibList[i-1].iWSID != wsLibList[i].iWSID)
//		{
//			wsLibList_temp.push_back(wsLibList[i]);
//		}
//	}
//
//	wsModelList_temp.push_back(wsModelList[0]);
//	for (int i=1;i<wsModelList.size();++i)
//	{
//		if (wsModelList[i-1].iWaterSoilModelID != wsModelList[i].iWaterSoilModelID)
//		{
//			wsModelList_temp.push_back(wsModelList[i]);
//		}
//	}
//	//存储到sqlite数据库中
//	CSqliteRW::savewslib(sqliteAcess,sql_,wsLibList_temp);
//	CSqliteRW::savewsmodel(sqliteAcess,sql_,wsModelList_temp);
//
//	return true;
//}
//
//bool CSqliteRW::savewslib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gWSLib> &wsLibList)
//{
//	string str;
//	gWSLib data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<wsLibList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into watersoillib values";
//
//		data = wsLibList[i];
//
//		_bstr_t t = data.strName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t b = data.strBrand.c_str(); 
//		char* pcharb = (char*)b;  
//		string br = pcharb;
//
//		sqlstream << "(" << data.iWSID << ",'" << name << "','" <<br<< "'," \
//			<< data.iType << "," << data.GroutDensity << "," << data.GroutSpecificHeat << "," << data.GroutThermalConductivity<< "," << data.GroundThermalConductivity << "," \
//			<< data.GroundDensity <<","<<data.GroundSpecificHeat<<","<<data.BoreholeType<<","<<data.FluidSpecificHeat<<","<<data.BoreholeDepth<<","\
//			<< data.BoreholeRadius<<","<<data.RowCount<<","<<data.ColumnCount<<","<<data.RowSpace<<","<<data.ColumnSpace<<","<<data.UTubeShanks<< ","\
//			<< data.UPipeOuterRadius<<","<<data.UPipeInterRadius<<","<<data.ConvectiveHeatTransferCoefficient<<","<<data.PipeThermalConductivity<<","<<data.FlowRate<< ");";
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::savewsmodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gWaterSoil_Model> &wsModelList)
//{
//	string str;
//	string dt;
//	gWaterSoil_Model data;
//	sqliteAcess.BeginTransaction();
//	for (int i=0;i<wsModelList.size(); ++i)
//	{
//		ostringstream sqlstream;
//		sqlstream << "insert into watersoil_model values";
//
//		data = wsModelList[i];
//
//		_bstr_t t = data.strWaterSoilModelName.c_str(); 
//		char* pchar = (char*)t;  
//		string name = pchar;
//
//		_bstr_t tt = data.strModel_Param.c_str(); 
//		char* ppchar = (char*)tt;  
//		string p = ppchar;
//
//		dt = SystemTimeToString(data.tmModel_Gen_Time);
//
//		sqlstream << "(" << data.iWaterSoilModelID << ",'" << name << "'," <<data.iWaterSoilID<< "," \
//			<< data.iType << ",'" <<dt<< "','" <<p<<  "');";
//
//		str = sqlstream.str();
//
//		char *ex_sql = const_cast<char*>(str.c_str());
//		sqliteAcess.SqlExe(ex_sql);
//	}
//	sqliteAcess.CommitTransaction();
//
//	return true;
//}
//bool CSqliteRW::SaveOtherLib(CSqliteAcess &sqliteAcess,string &sql_)
//{
//	//存储其他没有模型的设备
//	sqliteAcess.BeginTransaction();
//	//corner lib
//	string str;
//	ostringstream sqlstream;
//	sqlstream << "insert into cornerlib values";
//	int		CornerID	= 0;
//	int		CornerType	= 2;
//	double  CornerS		= 0.015;
//	sqlstream << "(" <<CornerID << "," << CornerType << "," << CornerS << ");";
//	str = sqlstream.str();
//	char *ex_sql = const_cast<char*>(str.c_str());
//	sqliteAcess.SqlExe(ex_sql);
//
//	//hxlib
//	sqlstream.clear();
//	sqlstream.str("");
//	sqlstream << "insert into hxlib values";
//
//	sqlstream << "(" <<0 << "," << 1 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << ");";
//	str = sqlstream.str();
//	ex_sql = const_cast<char*>(str.c_str());
//	sqliteAcess.SqlExe(ex_sql);
//
//	//pipelib
//	sqlstream.clear();
//	sqlstream.str("");
//	sqlstream << "insert into pipelib values";
//	sqlstream << "(" <<0 << "," << 20.0 << "," << 1 << ");";
//	str = sqlstream.str();
//	ex_sql = const_cast<char*>(str.c_str());
//	sqliteAcess.SqlExe(ex_sql);
//
//	//tanklib
//	sqlstream.clear();
//	sqlstream.str("");
//	sqlstream << "insert into tanklib values";
//	//_bstr_t t = data.strName.c_str(); 
//	//char* pchar = (char*)t;  
//	string name = "YYY";
//	//_bstr_t b = data.strBrand.c_str(); 
//	//char* pcharb = (char*)b;  
//	string br = "xxx";
//	sqlstream << "(" << 0 << "," << 0 <<",'"<< br <<"',"<< 100.0 <<","<< 500.0 <<",'"<<name << "');";
//	str = sqlstream.str();
//	ex_sql = const_cast<char*>(str.c_str());
//	sqliteAcess.SqlExe(ex_sql);
//
//
//	//terminallib
//	sqlstream.clear();
//	sqlstream.str("");
//	sqlstream << "insert into terminallib values";
//
//	sqlstream << "(" <<0 << "," << 0 << ");";
//	str = sqlstream.str();
//	ex_sql = const_cast<char*>(str.c_str());
//	sqliteAcess.SqlExe(ex_sql);
//
//	//vavelib
//	sqlstream.clear();
//	sqlstream.str("");
//	sqlstream << "insert into valvelib values";
//	sqlstream << "(" <<0 << ","<<1 << "," << 1 << ");";
//	str = sqlstream.str();
//	ex_sql = const_cast<char*>(str.c_str());
//
//	sqliteAcess.SqlExe(ex_sql);
//
//	sqliteAcess.CommitTransaction();
//
//	return true;
//
//}
//bool CSqliteRW::ReadChillerModel(CSqliteAcess &sqliteAcess,std::vector<gCH_Model> &chllermodellist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from chiller_model;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gCH_Model MyChillerModel;
//
//		MyChillerModel.iChillerModelID			= sqliteAcess.getColumn_Int(0);
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyChillerModel.strCHillerModelName		= name;
//
//		MyChillerModel.iChillerID				= sqliteAcess.getColumn_Int(2);
//		MyChillerModel.iType					= sqliteAcess.getColumn_Int(3);
//
//		CString   t_temp(const_cast<char*>(sqliteAcess.getColumn_Text(4)));
//		wstring   t = t_temp.GetString();
//		SYSTEMTIME  systime;
//		String_To_SysTime( t,systime );
//		MyChillerModel.tmModel_Gen_Time			= systime;
//
//		CString   p_temp(const_cast<char*>(sqliteAcess.getColumn_Text(5)));
//		wstring   p = p_temp.GetString();
//
//		MyChillerModel.strModel_Param			= p;
//
//		chllermodellist.push_back(MyChillerModel);
//	}
//
//	return true;
//}
//bool CSqliteRW::ReadPumpModel(CSqliteAcess &sqliteAcess,std::vector<gPump_Model> &pumpmodellist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from pump_model;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gPump_Model MyPumpModel;
//
//		MyPumpModel.iPumpModelID			= sqliteAcess.getColumn_Int(0);
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyPumpModel.strPumpModelName		= name;
//
//		MyPumpModel.iPumpID					= sqliteAcess.getColumn_Int(2);
//		MyPumpModel.iType					= sqliteAcess.getColumn_Int(3);
//
//		CString   t_temp(const_cast<char*>(sqliteAcess.getColumn_Text(4)));
//		wstring   t = t_temp.GetString();
//		SYSTEMTIME  systime;
//		String_To_SysTime( t,systime );
//		MyPumpModel.tmModel_Gen_Time		= systime;
//
//		CString   p_temp(const_cast<char*>(sqliteAcess.getColumn_Text(5)));
//		wstring   p = p_temp.GetString();
//
//		MyPumpModel.strModel_Param			= p;
//
//		pumpmodellist.push_back(MyPumpModel);
//	}
//
//	return true;
//}
//bool CSqliteRW::ReadTowerModel(CSqliteAcess &sqliteAcess,std::vector<gCT_Model> &towermodellist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from coolingtower_model;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gCT_Model MyCTModel;
//
//		MyCTModel.iModelID			= sqliteAcess.getColumn_Int(0);
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyCTModel.strModelName		= name;
//
//		MyCTModel.iCoolingTowerID	= sqliteAcess.getColumn_Int(2);
//		MyCTModel.iType				= sqliteAcess.getColumn_Int(3);
//
//		CString   t_temp(const_cast<char*>(sqliteAcess.getColumn_Text(4)));
//		wstring   t = t_temp.GetString();
//		SYSTEMTIME  systime;
//		String_To_SysTime( t,systime );
//		MyCTModel.tmModel_Gen_Time	= systime;
//
//		CString   p_temp(const_cast<char*>(sqliteAcess.getColumn_Text(5)));
//		wstring   p = p_temp.GetString();
//
//		MyCTModel.strModel_Param	= p;
//
//		towermodellist.push_back(MyCTModel);
//	}
//
//	return true;
//}
//bool CSqliteRW::ReadWaterSoilModel(CSqliteAcess &sqliteAcess,std::vector<gWaterSoil_Model> &wsmodellist,string &sql_)
//{
//	ostringstream sqlstream;
//	sqlstream << "select * from watersoil_model;";
//	sql_ = sqlstream.str();
//	char *ex_sql = const_cast<char*>(sql_.c_str());
//	int re = sqliteAcess.SqlQuery(ex_sql);
//
//	while(true)
//	{
//		if(SQLITE_ROW != sqliteAcess.SqlNext())
//		{
//			break;
//		}
//		gWaterSoil_Model MyWSModel;
//
//		MyWSModel.iWaterSoilModelID			= sqliteAcess.getColumn_Int(0);
//		CString   name_temp(const_cast<char*>(sqliteAcess.getColumn_Text(1)));
//		wstring   name = name_temp.GetString();
//		MyWSModel.strWaterSoilModelName		= name;
//
//		MyWSModel.iWaterSoilID				= sqliteAcess.getColumn_Int(2);
//		MyWSModel.iType						= sqliteAcess.getColumn_Int(3);
//
//		CString   t_temp(const_cast<char*>(sqliteAcess.getColumn_Text(4)));
//		wstring   t = t_temp.GetString();
//		SYSTEMTIME  systime;
//		String_To_SysTime( t,systime );
//		MyWSModel.tmModel_Gen_Time			= systime;
//
//		CString   p_temp(const_cast<char*>(sqliteAcess.getColumn_Text(5)));
//		wstring   p = p_temp.GetString();
//
//		MyWSModel.strModel_Param			= p;
//
//		wsmodellist.push_back(MyWSModel);
//	}
//
//	return true;
//}
//bool CSqliteRW::SaveEquipmentModel(std::vector<gCH_Model> &chllermodellist,
//	std::vector<gPump_Model> &pumpmodellist,
//	std::vector<gCT_Model> &towermodellist,
//	std::vector<gWaterSoil_Model> &wsmodellist)
//{
//	for (int i=0;i<chllermodellist.size();++i)
//	{
//		g_pDatabase->m_pDBAccess->InsertIntoModelTable(Type_CHILLER,&chllermodellist[i]);
//	}
//	for (int i=0;i<pumpmodellist.size();++i)
//	{
//		g_pDatabase->m_pDBAccess->InsertIntoModelTable(Type_PUMP,&pumpmodellist[i]);
//	}
//
//	for (int i=0;i<towermodellist.size();++i)
//	{
//		g_pDatabase->m_pDBAccess->InsertIntoModelTable(Type_COOLINGTOWER,&towermodellist[i]);
//	}
//
//	for (int i=0;i<wsmodellist.size();++i)
//	{
//		g_pDatabase->m_pDBAccess->InsertIntoModelTable(Type_WATERSOIL,&wsmodellist[i]);
//	}
//
//	return true;
//}