
#include "StdAfx.h"
#include "SqliteAcess.h"
#include <cassert>

#pragma warning(disable:4996) 

#define  MAX_PATH  260

CSqliteAcess::CSqliteAcess(const std::string  lpszPath,const std::string  lpszPsw)
{

	m_db = NULL;
	m_stmt = NULL;
	m_bOpenState = false;
	m_lpszLocale = NULL;
	m_lpszLocale =_strdup( setlocale(LC_CTYPE,NULL) ); 
	setlocale( LC_CTYPE,"chs");
	int nIndex = lpszPath.find("4db");
	if(nIndex>=0)
		OpenDataBase(lpszPath.data(),"");
	else
	{
		OpenDataBase(lpszPath.data(),lpszPsw);
	}
	
}

CSqliteAcess::~CSqliteAcess(void)
{
	setlocale(LC_CTYPE, m_lpszLocale);
	free(m_lpszLocale);//golding2014-08-12
	CloseDataBase();
}

int  CSqliteAcess::CloseDataBase()
{
	if(m_db)
	{
		sqlite3_close(m_db); 
		m_db = NULL;
	}
	return 0;
}

bool CSqliteAcess::GetOpenState()
{
	return m_bOpenState;
}

int  CSqliteAcess::OpenDataBase(const char* pathName,const std::string lpszPsw)
{
	char * pErrMsg = 0;
	int ret = 0;
	if(m_db)
	{
		sqlite3_close(m_db); 
		m_db = NULL;
	}

	char  name[MAX_PATH] = "";
	strcpy(name, pathName);
	ret = sqlite3_open(name, &m_db);
	if(lpszPsw.length()>0)
		sqlite3_key(m_db,lpszPsw.data(),strlen(lpszPsw.data()));/*RNB.beop-2013*/
	if ( ret != SQLITE_OK )
	{
		fprintf(stderr, "Could not open database: %s", sqlite3_errmsg(m_db));
		exit(1);
	}
	m_bOpenState = true;
	return ret;
}


int CSqliteAcess::CreateTable(const char* sqlr)
{
	char* error = NULL;
	int  result;

	//创建工程表
	char project[512] = "Create TABLE MAIN.[project] ([ProjectID] INTEGER PRIMARY KEY autoincrement,"\
		"[Name] char(30,0) unique,"\
		"[Location] char(30,0),"\
		"[ProjectType] INTEGER,"\
		"[WeatherStationID] INTEGER,"\
		"[FloorArea] DOUBLE,"\
		"[Picture] BLOB,"\
		"[Memo] char(30,0),"\
		"[Creator] char(30,0))";
	result = sqlite3_exec(m_db, project, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	char chillerRoom[1536] = "Create TABLE MAIN.[chillerplantroom] ([CPR_ID] INTEGER PRIMARY KEY autoincrement,"\
		"[CPR_NAME] char(30,0),"\
		"[OF_PROJECT_ID] INTEGER,"\
		"[Setting_Max_LEWT] DOUBLE,"\
		"[Setting_Min_LEWT] DOUBLE,"\
		"[Setting_Max_ECWT] DOUBLE,"\
		"[Setting_Min_ECWT] DOUBLE,"\
		"[Setting_Max_Cooling_DT] DOUBLE,"\
		"[Setting_Max_Chilled_DT] DOUBLE,"\
		"[Setting_Min_ChillerLoadRatio] DOUBLE,"\
		"[Setting_Min_PumpFlowRatio] DOUBLE,"\
		"[Setting_Min_TowerLoadRatio] DOUBLE,"\
		"[dsetting_sys_min_chillflow] DOUBLE,"\
		"[dsetting_sys_min_coolflow] DOUBLE,"\
		"[HvacType] char(25,0),"\
		"[SChillOther] DOUBLE,"\
		"[SCoolOther] DOUBLE,"\
		"[RoomType] INTEGER,"\
		"[PumpGroup_Param] char(25,0),"\
		"[TowerChillerRatio] DOUBLE,"\
		"[PumpChillerRatio] DOUBLE,"\
		"[RatioUseable] INTEGER,"\
		"[setting_common_lewt] DOUBLE,"\
		"[vsd_coolpump] INTEGER,"\
		"[vsd_pri_chillpump] INTEGER,"\
		"[vsd_sec_chillpump] INTEGER,"\
		"[vsd_value] DOUBLE,"\
		"[setting_coolpump_minpowerratio] DOUBLE,"\
		"[setting_prichillpump_minpowerratio] DOUBLE,"\
		"[setting_secchillpump_minpowerratio] DOUBLE,"\
		"[setting_tower_powerindex] DOUBLE,"\
		"[setting_h0] DOUBLE,"\
		"[setting_chiller_k] DOUBLE,"\
		"[setting_ct_minhz] DOUBLE,"\
		"[setting_ct_eff_correction] DOUBLE,"\
		"[setting_electricpercent_max] DOUBLE,"\
		"[setting_approacht_add] DOUBLE,"\
		"[setting_approacht_subtract] DOUBLE,"\
		"[setting_diffflow] DOUBLE,"\
		"[vsd_value_pri] DOUBLE,"\
		"[vsd_value_sec] DOUBLE,"\
		"[vsd_value_ct] DOUBLE,"\
		"[vsd_coolingtower] INTEGER,"\
		"[ChillpumpChillerratio] DOUBLE,"\
		"[ct_outt_control] INTEGER,"\
		"[deltat_control] INTEGER,"\
		"[deltat_chill] DOUBLE,"\
		"[deltat_cool] DOUBLE)";

	result = sqlite3_exec(m_db, chillerRoom, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}


	char room_chiller[512] = "Create TABLE MAIN.[room_contain_chiller] ([Chiller_ID_In_Room] INTEGER PRIMARY KEY,"\
		"[Chiller_Name_In_Room] char(30,0),"\
		"[ChillerID] INTEGER,"\
		"[OF_CPR_ID] INTEGER,"\
		"[Enabled] INTEGER,"\
		"[X] INTEGER,"\
		"[Y] INTEGER,"\
		"[ChillerModelID] INTEGER,"\
		"[COMCtrl_ID] INTEGER,"\
		"[PLCCtrl_ID] INTEGER,"\
		"[RotateAngel] DOUBLE,"\
		"[Scale] DOUBLE,"\
		"[SChill] DOUBLE,"\
		"[SCool] DOUBLE,"\
		"[ChillerRate] DOUBLE,"\
		"[HeatPump] INTEGER)";
	result = sqlite3_exec(m_db, room_chiller, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 


	char room_coolingtower[512] = "Create TABLE MAIN.[room_contain_coolingtower] ([Tower_ID_In_Room] INTEGER PRIMARY KEY,"\
		"[Tower_Name_In_Room] char(30,0),"\
		"[TowerID] INTEGER,"\
		"[OF_CPR_ID] INTEGER,"\
		"[Enabled] INTEGER,"\
		"[Max_Capacity] DOUBLE,"\
		"[X] INTEGER,"\
		"[Y] INTEGER,"\
		"[CTModelID] INTEGER,"\
		"[PLCCtrl_ID] INTEGER,"\
		"[WithVSD] INTEGER,"\
		"[RotateAngel] DOUBLE,"\
		"[Scale] DOUBLE,"\
		"[SCool] DOUBLE)";
	result = sqlite3_exec(m_db, room_coolingtower, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 


	char room_pump[512] = "Create TABLE MAIN.[room_contain_pump] ([Pump_ID_In_Room] INTEGER PRIMARY KEY,"\
		"[Pump_Name_In_Room] char(30,0),"\
		"[PumpID] INTEGER,"\
		"[OF_CPR_ID] INTEGER,"\
		"[Pump_Location] INTEGER,"\
		"[Enabled] INTEGER,"\
		"[Standby] INTEGER,"\
		"[X] INTEGER,"\
		"[Y] INTEGER,"\
		"[PumpModelID] INTEGER,"\
		"[PLCCtrl_ID] INTEGER,"\
		"[WithVSD] INTEGER,"\
		"[RotateAngel] DOUBLE,"\
		"[Scale] DOUBLE)";
	result = sqlite3_exec(m_db, room_pump, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 


	char room_terminal[512] = "Create TABLE MAIN.[room_contain_terminal] ([Terminal_ID_InRoom] INTEGER PRIMARY KEY,"\
		"[Terminal_ID] INTEGER,"\
		"[X] INTEGER,"\
		"[Y] INTEGER,"\
		"[OF_CPR_ID] INTEGER,"\
		"[RotateAngel] DOUBLE,"\
		"[Scale] DOUBLE,"\
		"[SChill] DOUBLE)";
	result = sqlite3_exec(m_db, room_terminal, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 


	char room_corner[256] = "Create TABLE MAIN.[room_contain_corner] ([Corner_ID_InRoom] INTEGER PRIMARY KEY,"\
		"[Corner_ID] INTEGER,"\
		"[X] INTEGER,"\
		"[Y] INTEGER,"\
		"[OF_CPR_ID] INTEGER)";
	result = sqlite3_exec(m_db, room_corner, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 

	char room_pipe[512] = "Create TABLE MAIN.[room_contain_pipe] ([Pipe_ID_In_Room] INTEGER PRIMARY KEY,"\
		"[Pipe_Name_In_Room] char(30,0),"\
		"[PipeID] INTEGER,"\
		"[OF_CPR_ID] INTEGER,"\
		"[Enabled] INTEGER,"\
		"[From_Service_ID] INTEGER,"\
		"[From_Service_Type] INTEGER,"\
		"[From_Service_Port] INTEGER,"\
		"[To_Service_ID] INTEGER,"\
		"[To_Service_Type] INTEGER,"\
		"[To_Service_Port] INTEGER)";
	result = sqlite3_exec(m_db, room_pipe, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 


	char room_value[512] = "Create TABLE MAIN.[room_contain_valve] ([Valve_ID_In_Room] INTEGER PRIMARY KEY,"\
		"[Valve_Name_In_Room] char(30,0),"\
		"[ValveID] INTEGER,"\
		"[OF_CPR_ID] INTEGER,"\
		"[Enabled] INTEGER,"\
		"[X] INTEGER,"\
		"[Y] INTEGER,"\
		"[PLCCtrl_ID] INTEGER,"\
		"[RotateAngel] DOUBLE,"\
		"[Scale] DOUBLE)";
	result = sqlite3_exec(m_db, room_value, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 

	//model四个表
	char chiller_model[512] = "Create TABLE MAIN.[chiller_model] ([ChillerModelID] INTEGER PRIMARY KEY,"\
		"[ChillerModelName] char(50,0),"\
		"[ChillerID] INTEGER,"\
		"[Type] INTEGER,"\
		"[Model_Gen_Time] char(50,0),"\
		"[Model_Param] char(4096,0))";
	result = sqlite3_exec(m_db, chiller_model, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 

	char pump_model[512] = "Create TABLE MAIN.[pump_model] ([PumpModelID] INTEGER PRIMARY KEY,"\
		"[PumpModelName] char(100,0),"\
		"[PumpID] INTEGER,"\
		"[Type] INTEGER,"\
		"[Model_Gen_Time] char(100,0),"\
		"[Model_Param] char(4096,0))";
	result = sqlite3_exec(m_db, pump_model, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 

	char ct_model[512] = "Create TABLE MAIN.[coolingtower_model] ([CoolingTowerModelID] INTEGER PRIMARY KEY,"\
		"[CoolingTowerModelName] char(100,0),"\
		"[CoolingTowerID] INTEGER,"\
		"[Type] INTEGER,"\
		"[Model_Gen_Time] char(100,0),"\
		"[Model_Param] char(4096,0))";
	result = sqlite3_exec(m_db, ct_model, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 

	char water_model[512] = "Create TABLE MAIN.[waternet_model] ([model_id] INTEGER PRIMARY KEY,"\
		"[model_name] char(100,0),"\
		"[Type] INTEGER,"\
		"[Model_Gen_Time] char(100,0),"\
		"[Model_Param] char(4096,0))";
	result = sqlite3_exec(m_db, water_model, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 

	//ws_model
	char ws_model[512] = "Create TABLE MAIN.[watersoil_model] ([WaterSoilModelID] INTEGER PRIMARY KEY,"\
		"[WaterSoilModelName] char(50,0),"\
		"[WaterSoilID] INTEGER,"\
		"[Type] INTEGER,"\
		"[Model_Gen_Time] char(50,0),"\
		"[Model_Param] char(4096,0))";
	result = sqlite3_exec(m_db, ws_model, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	} 

    //lib 9个表
	//chillerlib
	char chillerlib[512] = "Create TABLE MAIN.[chillerlib] ([ChillerID] INTEGER PRIMARY KEY,"\
		"[ModelName] char(50,0),"\
		"[Brand] char(30,0),"\
		"[Type] INTEGER,"\
		"[Refrig] INTEGER,"\
		"[Motor_Start_Type] INTEGER,"\
		"[Rating_CoolingCap] DOUBLE,"\
		"[Rating_HeatCap] DOUBLE,"\
		"[Rating_Flow_Cond] DOUBLE,"\
		"[Rating_Flow_Evap] DOUBLE,"\
		"[Rating_COP] DOUBLE,"\
		"[Rating_LEWT] DOUBLE,"\
		"[Rating_ECWT] DOUBLE,"\
		"[Surge_Param1] DOUBLE,"\
		"[Surge_Param2] DOUBLE,"\
		"[Surge_Param3] DOUBLE,"\
		"[Surge_Param4] DOUBLE,"\
		"[Rating_DP_Evap] DOUBLE,"\
		"[Rating_DP_Cond] DOUBLE)";
	result = sqlite3_exec(m_db, chillerlib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//pumplib
	char pumplib[512] = "Create TABLE MAIN.[pumplib] ([PumpID] INTEGER PRIMARY KEY,"\
		"[ModelName] char(50,0),"\
		"[Brand] char(30,0),"\
		"[Type] INTEGER,"\
		"[Rating_RPM] INTEGER,"\
		"[Rating_Flow] DOUBLE,"\
		"[Rating_Head] DOUBLE,"\
		"[Rating_Power] DOUBLE)";
	result = sqlite3_exec(m_db, pumplib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//coolingTowerlib
	char coolingTowerlib[512] = "Create TABLE MAIN.[coolingtowerlib] ([CoolingTowerID] INTEGER PRIMARY KEY,"\
		"[CoolingTowerModelName] char(50,0),"\
		"[Brand] char(30,0),"\
		"[Type] INTEGER,"\
		"[Rating_Air_Flow] INTEGER,"\
		"[Rating_Water_Flow] DOUBLE,"\
		"[Rating_TWB_Air] DOUBLE,"\
		"[Rating_T_water_in] DOUBLE,"\
		"[Rating_T_water_out] DOUBLE,"\
		"[Power] DOUBLE,"\
		"[FansCount] DOUBLE)";
	result = sqlite3_exec(m_db, coolingTowerlib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//hxlib
	char hxlib[256] = "Create TABLE MAIN.[hxlib] ([HxID] INTEGER PRIMARY KEY,"\
		"[HxType] INTEGER,"\
		"[HxDP1] DOUBLE,"\
		"[HxDP2] DOUBLE,"\
		"[Tin1] DOUBLE,"\
		"[Tin2] DOUBLE,"\
		"[Tout1] DOUBLE,"\
		"[Tout2] DOUBLE,"\
		"[HxQ] DOUBLE)";
	result = sqlite3_exec(m_db, hxlib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//pipelib
	char pipelib[256] = "Create TABLE MAIN.[pipelib] ([PipeID] INTEGER PRIMARY KEY,"\
		"[Diameter] DOUBLE,"\
		"[Material] INTEGER)";
	result = sqlite3_exec(m_db, pipelib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//cornerlib
	char cornerlib[256] = "Create TABLE MAIN.[cornerlib] ([CornerID] INTEGER PRIMARY KEY,"\
		"[CornerType] INTEGER,"\
		"[CornerS] DOUBLE)";
	result = sqlite3_exec(m_db, cornerlib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}


	//Terminallib
	char terminallib[256] = "Create TABLE MAIN.[terminallib] ([Terminal_ID] INTEGER PRIMARY KEY,"\
		"[Terminal_Type] INTEGER)";
	result = sqlite3_exec(m_db, terminallib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//valuelib
	char valvelib[256] = "Create TABLE MAIN.[valvelib] ([ValveID] INTEGER PRIMARY KEY,"\
		"[ValveType] INTEGER,"\
		"[Valves] DOUBLE)";
	result = sqlite3_exec(m_db, valvelib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//tanklib
	char tanklib[256] = "Create TABLE MAIN.[tanklib] ([tankid] INTEGER PRIMARY KEY,"\
		"[type] INTEGER,"\
		"[brand] char(30,0),"\
		"[volumn] DOUBLE,"\
		"[capacity] DOUBLE,"\
		"[modelname] char(128,0))";
	result = sqlite3_exec(m_db, tanklib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//wslib
	char watersoillib[1024] = "Create TABLE MAIN.[watersoillib] ([WaterSoilID] INTEGER PRIMARY KEY,"\
		"[Name] char(50,0),"\
		"[Brand] char(30,0),"\
		"[Type] INTEGER,"\
		"[GroutDensity] DOUBLE,"\
		"[GroutSpecificHeat] DOUBLE,"\
		"[GroutThermalConductivity] DOUBLE,"\
		"[GroundThermalConductivity] DOUBLE,"\
		"[GroundDensity] DOUBLE,"\
		"[GroundSpecificHeat] DOUBLE,"\
		"[BoreholeType] INTEGER,"\
		"[FluidSpecificHeat] DOUBLE,"\
		"[BoreholeDepth] DOUBLE,"\
		"[BoreholeRadius] DOUBLE,"\
		"[RowCount] INTEGER,"\
		"[ColumnCount] INTEGER,"\
		"[RowSpace] DOUBLE,"\
		"[ColumnSpace] DOUBLE,"\
		"[UTubeShanks] DOUBLE,"\
		"[UPipeOuterRadius] DOUBLE,"\
		"[UPipeInterRadius] DOUBLE,"\
		"[ConvectiveHeatTransferCoefficient] DOUBLE,"\
		"[PipeThermalConductivity] DOUBLE,"\
		"[FlowRate] DOUBLE)";
	result = sqlite3_exec(m_db, watersoillib, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}

	//8736个东西
	char chiller_plant[512] = "Create TABLE MAIN.[chiller_plant_room_load_define] ([Time_] char(50, 0) PRIMARY KEY,"\
		"[CoolingLoad] DOUBLE,"\
		"[HeatingLoad] DOUBLE,"\
		"[Total_Flow] DOUBLE,"\
		"[Total_T_Supply] DOUBLE,"\
		"[Total_T_Return] DOUBLE,"\
		"[OF_CPR_ID] INTEGER,"\
		"[Tdry] DOUBLE,"\
		"[Twet] DOUBLE,"\
		"[electricprice] DOUBLE,"\
		"[defaut1] DOUBLE,"\
		"[coolingload1] DOUBLE,"\
		"[coolingload2] DOUBLE,"\
		"[coolingload3] DOUBLE,"\
		"[coolingload4] DOUBLE);";

	result = sqlite3_exec(m_db, chiller_plant, NULL, NULL, &error);
	if(result!=SQLITE_OK)
	{
		printf("Create table failed with error code:% d, wrong reasons:%s\n", result ,error);
	}
	return  result;
}


int CSqliteAcess::SqlExe(const char* sql)
{
	char* error;
	int  result;
	result = sqlite3_exec(m_db, sql, NULL, NULL, &error);

	if(result!=SQLITE_OK)
	{
		printf("Execute sql failed with error code:% d, wrong reasons%s\n", result ,error);
	}
	return  result;

}
int	CSqliteAcess::BeginTransaction()
{
	char* error;
	int result = sqlite3_exec(m_db, "BEGIN;", NULL, NULL, &error);

	return result;
}
int CSqliteAcess::CommitTransaction()
{
	char* error;
	int re = sqlite3_exec(m_db, "COMMIT;", NULL, NULL, &error);
	return  re;
}
int CSqliteAcess::SqlQuery(const char* sqlr)
{
	int re = sqlite3_prepare(m_db, sqlr, strlen(sqlr), &m_stmt, 0);

	return re;
}
int	CSqliteAcess::SqlFinalize()
{
	int rc = 0;
	rc = sqlite3_finalize(m_stmt);
	return rc;
}
int CSqliteAcess::SqlNext()
{
	int rc = 0;
	rc = sqlite3_step(m_stmt);
	return rc;
}
int CSqliteAcess::getColumn_Count()
{
	return sqlite3_column_count(m_stmt);
}
double CSqliteAcess::getColumn_Double(int i)
{
	return sqlite3_column_double(m_stmt, i);
}
int CSqliteAcess::getColumn_Int(int i)
{
	return sqlite3_column_int(m_stmt, i);
}
const char* CSqliteAcess::getColumn_Text(int i)
{
	return  reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, i));
}
const void*	CSqliteAcess::getColumn_Blob (const int i)
{
	return sqlite3_column_blob(m_stmt, i);
}
int	CSqliteAcess::getColumn_Bytes(const int i)
{
	return sqlite3_column_bytes(m_stmt, i);
}
int	CSqliteAcess::SqlStep()
{
	return sqlite3_step(m_stmt);
}
sqlite3* CSqliteAcess::GetDB() const
{
	if (m_db)
	{
		return m_db;
	}
	return NULL;
}

sqlite3_stmt* CSqliteAcess::GetDataSet() const
{
	if (m_stmt)
	{
		return m_stmt;
	}
	return NULL;
}

char* CSqliteAcess::GetLocale()
{
	if (m_lpszLocale)
	{
		return m_lpszLocale;
	}
	return NULL;
}

int CSqliteAcess::DeleteAllPageData()
{
	int rc = 0;
	 
	rc = sqlite3_exec(m_db, "delete from list_page", NULL, NULL, NULL);

	return rc;
}
int CSqliteAcess::SqlBind_Blob(char *pBlock,int iOrder ,int nSize)
{
	int rc = 0;
	rc = sqlite3_bind_blob(m_stmt,iOrder,pBlock,nSize,NULL);

	return rc;
}
int CSqliteAcess::InsertRecordToPage( const char* lpszType, const int nStartColor, const int nEndColor, const int nImageID, const int nID, const char* lpszName, const int nShowOrder, const int nBackGroundColor, const int nWidth, const int nHeight, const int nGroupID, const int nXPos, const int nYPos)
{
	assert(lpszType);
	assert(lpszName);

	char szSQL[1000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_page(id, bgcolor1, bgcolor2, bgimg, 'pagetype', 'name', showOrder, bgcolor1, width, hight, groupid, xposition, yposition)\
		VALUES(%d, %d, %d, %d, '%s', '%s', %d, %d, %d, %d, %d, %d, %d)", nID, nStartColor, nEndColor, nImageID, lpszType, lpszName, nShowOrder, nBackGroundColor, nWidth, nHeight, nGroupID, nXPos, nYPos);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::DeleteAllContainElements()
{
	int rc = 0;

	rc = sqlite3_exec(m_db, "delete from page_contain_elements", NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::DeleteProjectConfigData()
{
	int rc = 0;

	rc = sqlite3_exec(m_db, "delete from project_config", NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::InsertRecordToProjectConfig( const int nID, const char* lpszName, const int nWidth, const int nHeight, const int nVersion /*= 3*/, const char* lpszXupTime /*= ""*/ )
{
	char szSQL[1000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO project_config(ProjectID, 'ProjectName', ScreenWidth, ScreenHeight, Version, 'XupTime','HostIP','RealTimeDBName','UserName',port)\
									VALUES(%d, '%s', %d, %d, %d, '%s', '%s', '%s', '%s', %d)", nID, lpszName, nWidth, nHeight, nVersion, lpszXupTime, "localhost", "beopdata", "root", 3306);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::DeleteAllEventData()
{
	int rc = 0;

	rc = sqlite3_exec(m_db, "delete from event", NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::InsertRecordToContainElements( const int nID, const char* lpszName, const int nType, const int nXPos, const int nYPos, const int nWidth, const int nHeight, const char* lpszDataComID, const int nLayer, const int nPageID,const int nRotateAngle,const int nLinkPageID,const char* lpszShowString,const int nPicID,const int nSubType /*= 0*/ )
{
	char szSQL[1000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO page_contain_elements(elementid, 'elementName', elementtype, x, y, width, hight, DataComId, layer, pageid, rotateAngle,link,showString,elementSubType,pictureid)\
									VALUES(%d, '%s', %d, %d, %d, %d, %d, '%s', %d, %d, %d, %d, '%s', %d, %d)", nID, lpszName, nType, nXPos, nYPos, nWidth, nHeight, lpszDataComID, nLayer, nPageID, nRotateAngle, nLinkPageID, lpszShowString, nSubType,nPicID);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::InsertRecordToEvent( const int nPageID, const int nID, const char* lpszEventContent, const char* lpszBindContent )
{
	char szSQL[1000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO event(pageid, id, event_content,bind_content)\
									VALUES(%d, %d, '%s', '%s')", nPageID, nID, lpszEventContent,lpszBindContent);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::DeleteAllLineData()
{
	int rc = 0;
	rc = sqlite3_exec(m_db, "delete from list_line", NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::InsertRecordToLine( const int nPageID, const int nID, const int nInnerColor, const int nOutterColor, const char* lpszPointList, const int nWidth, const int nLayer )
{
	char szSQL[1000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_line(pageid, id, innerColor, outerColor, PointList, pipeWidth, layer)\
									VALUES(%d, %d, %d, %d, '%s', %d, %d)", nPageID, nID, nInnerColor, nOutterColor, lpszPointList, nWidth, nLayer);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::DeleteAllTextData()
{
	int rc = 0;
	rc = sqlite3_exec(m_db, "delete from list_text", NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::InsertRecordToText( const int nID, const int nPageID, const char* lpszInitialString, const int nXPos, const int nYPos, const int nWidth, const int nHeight, const int nFontSize, const int nFontColor, const char* lpszFontName, const char* lpszDataComID, const int nShowMode, const double dTransparence, const int nBorderType, const int nBorderColor, const int nLayer, const int nAlign )
{
	char szSQL[2000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	
	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_text(id, pageid, initialvalue, xpos, ypos, width, height, size, color, font, dataComId, showMode, transparence, borderType, border_color, layer, align)\
									VALUES(%d, %d, '%s', %d, %d, %d, %d, %d, %d, '%s', '%s', %d, %.2f, %d, %d, %d, %d)", 
									nID, nPageID, lpszInitialString, nXPos, nYPos, nWidth, nHeight, nFontSize, nFontColor, lpszFontName, lpszDataComID, nShowMode, dTransparence, nBorderType, nBorderColor, nLayer, nAlign);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	
	return rc;
}

int CSqliteAcess::DeleteAllGraphics()
{
	int rc = 0;

	rc = sqlite3_exec(m_db, "delete from list_graphic", NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::DeleteAllPoint()
{
	int rc = 0;

	rc = sqlite3_exec(m_db, "delete from list_point", NULL, NULL, NULL);

	return rc;
}

//int CSqliteAcess::InsertRecordToOPCPoint( const int nType, const char* lpszName, const char* lpszSource, const int nProperty, const char* lpszDescription, const char* lpszUnit, const double dHigh, const double dHighHigh, const double dLow, const double dLowLow, const char* lpszParam1,const char* lpszParam2, const char* lpszParam3,const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8, const char* lpszParam9,const char* lpszParam10 )
//{
//	char szSQL[3000] = {0};
//	memset(szSQL, 0, sizeof(szSQL));
//	int rc = 0;
//
//	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_point(type, name, SourceType, R_W, ch_description, unit, high, highhigh, low, lowlow, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10)\
//									VALUES(%d, '%s', '%s', %d, '%s', '%s', %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')", 
//									nType, lpszName, lpszSource, nProperty, lpszDescription, lpszUnit, (int)dHigh, (int)dHighHigh, (int)dLow, (int)dLowLow, lpszParam1, lpszParam2, lpszParam3,lpszParam4,lpszParam5,lpszParam6,lpszParam7,lpszParam8, lpszParam9,lpszParam10);
//	assert(strlen(szSQL)>0);
//	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
//
//	return rc;
//}


int CSqliteAcess::InsertRecordToOPCPoint(const int nIndex, const int nType, const char* lpszName, const char* lpszSource, const int nProperty, const char* lpszDescription, const char* lpszUnit, const double dHigh, const double dHighHigh, const double dLow, const double dLowLow, const char* lpszParam1,const char* lpszParam2, const char* lpszParam3,const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8, const char* lpszParam9,const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,const char* lpszParam15,const int nParam16,const int nParam17,const int nParam18)
{
	char szSQL[5000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	//int nMax = (int)atof(lpszParam12);
	//int nMin = (int)atof(lpszParam13);
	int nIsVisit = (int)atof(lpszParam14);
	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_point(id, type, name, SourceType, R_W, ch_description, unit, high, highhigh, low, lowlow, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param15, Param16, Param17, Param18)\
									VALUES(%d, %d, '%s', '%s', %d, '%s', '%s', %d, '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d)", nIndex,
									nType, lpszName, lpszSource, nProperty, lpszDescription, lpszUnit, (int)dHigh, lpszParam12, lpszParam13, nIsVisit, lpszParam1, lpszParam2, lpszParam3,lpszParam4,lpszParam5,lpszParam6,lpszParam7,lpszParam8, lpszParam9,lpszParam10,lpszParam11,lpszParam15,nParam16,nParam17,nParam18);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	assert(0 == rc);
	return rc;
}

int CSqliteAcess::InsertRecordToLogicPoint(const int nType, const char* lpszName, const char* lpszLogicType, const char* lpszParam1, const char* lpszParam2, const char* lpszParam3, const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8,const char* lpszParam9, const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,const char* lpszParam15, const char* lpszParam16,const char* lpszParam17,const char* lpszParam18,const char* lpszParam19,const char* lpszParam20,const char* lpszParam21, const char* lpszParam22,const char* lpszParam23,const char* lpszParam24,const char* lpszParam25,const char* lpszParam26,const char* lpszParam27, const char* lpszParam28,const char* lpszParam29,const char* lpszParam30,const char* lpszDescription )
{
	char szSQL[5000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_point(type, name, SubType, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10,\
									Param11,Param12,Param13,Param14,Param15,Param16,Param17,Param18,Param19,Param20,Param21,Param22,Param23,Param24,Param25,Param26,\
									Param27,Param28,Param29,Param30,ch_description)\
									VALUES(%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',\
									'%s','%s','%s','%s','%s','%s','%s','%s','%s')", 
									nType, lpszName, lpszLogicType, lpszParam1, lpszParam2, lpszParam3, lpszParam4,lpszParam5,lpszParam6,lpszParam7,lpszParam8,lpszParam9,
									lpszParam10,lpszParam11,lpszParam12,lpszParam13,lpszParam14,lpszParam15,lpszParam16,lpszParam17,lpszParam18,lpszParam19,lpszParam20,
									lpszParam21,lpszParam22,lpszParam23,lpszParam24,lpszParam25,lpszParam26,lpszParam27,lpszParam28,lpszParam29,lpszParam30,lpszDescription);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);

	return rc;
}

int CSqliteAcess::InsertRecordToAlarmPoint(const int nType, const char* lpszName, const char* lpszAlarmType, const char* lpszParam1, const char* lpszParam2, const char* lpszParam3, const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8,const char* lpszParam9, const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,const char* lpszParam15, const char* lpszParam16,const char* lpszParam17,const char* lpszParam18,const char* lpszParam19,const char* lpszParam20,const char* lpszDescription )
{
	char szSQL[5000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_point(type, name, SubType, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10,\
									Param11,Param12,Param13,Param14,Param15,Param16,Param17,Param18,Param19,Param20,ch_description)\
									VALUES(%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')", 
									nType, lpszName, lpszAlarmType, lpszParam1, lpszParam2, lpszParam3, lpszParam4,lpszParam5,lpszParam6,lpszParam7,lpszParam8,lpszParam9,
									lpszParam10,lpszParam11,lpszParam12,lpszParam13,lpszParam14,lpszParam15,lpszParam16,lpszParam17,lpszParam18,lpszParam19,lpszParam20,lpszDescription);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);

	return rc;
}

int	CSqliteAcess::InsertRecordToGraphics(const int nID, const int nPageID, const int nType, const int nLineColor, const int nLineStyle, const int nLineWidth, const int nFillColor, const char* lpszPointList,
									const int nFrontPageID, const double dTransparence, const int nIsBorder3D, const int nLayer)
{
	char szSQL[2000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;

	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_graphic(pageid,id,type,pointlist,fillcolor,linecolor,linewidth,linestyle,forwardpageid,transparence,textbordet,layer)\
									VALUES(%d,%d,%d,'%s',%d,%d,%d,%d,%d,%.2f,%d,%d)", 
									nPageID,nID,nType,lpszPointList,nFillColor,nLineColor,nLineWidth,nLineStyle,nFrontPageID,dTransparence,nIsBorder3D,nLayer);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::DeleteAllUsefulTables()
{
	int rc = 0;
	rc = DeleteAllGroupData();
	rc = DeleteAllPageData();
	rc = DeleteAllContainElements();
	rc = DeleteAllEventData();
	rc = DeleteAllLineData();
	rc = DeleteAllTextData();
	rc = DeleteAllGraphics();
	rc = DeleteAllPoint();
	rc = DeleteAllDashBoardData();
	return rc;
}

int CSqliteAcess::DeleteAllGroupData()
{
	int rc = 0;
	rc = sqlite3_exec(m_db, "delete from list_pagegroup", NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::InsertRecordToPageGroup( const int nGroupID, const char* lpszGroupName )
{
	char szSQL[200] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_pagegroup(id,name) VALUES(%d,'%s')", nGroupID, lpszGroupName);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::UpdateProjectConfig( const int nID, const char* lpszName, const int nWidth, const int nHeight, const int nVersion /*= 3*/, const char* lpszXupTime /*= ""*/ )
{
	char szSQL[1000] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "UPDATE project_config set ProjectID=%d, ProjectName='%s', ScreenWidth=%d, ScreenHeight=%d, Version=%d, XupTime='%s')", nID, lpszName, nWidth, nHeight, nVersion, lpszXupTime);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::InsertRecordIntoLibImage( const int nID, const char* lpszName, const char* lpszSpecy, void* pBlock, const char* lpszFileType, const char* lpszRemark, const int bIsSyatemImage, const int nSize)
{
	char szSQL[200] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "insert into lib_image(ID,name,specy,imagebinary,filetype,remark,issystemimage) values(%d,'%s','%s',?,'%s','%s',%d)", nID, lpszName, lpszSpecy, lpszFileType, lpszRemark, bIsSyatemImage);
	assert(strlen(szSQL)>0);
	if (SQLITE_OK == sqlite3_prepare( m_db, szSQL, -1, &m_stmt, 0 ))
	{
		if (m_stmt)
		{
			rc = sqlite3_bind_blob(m_stmt,1,pBlock,nSize,NULL);
			rc = sqlite3_step(m_stmt);
			rc = sqlite3_finalize(m_stmt);
		}
	}
	return rc;
}

int CSqliteAcess::InsertRecordToLibAnimation( const int nID, const char* lpszName, const char* lpszAnimationList )
{
	char szSQL[500] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO lib_animation(id,name,animationlist) VALUES(%d,'%s','%s')", nID, lpszName, lpszAnimationList);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::InsertRecordIntoLibImageAnimation( const int nID, const char* lpszName, const char* lpszSpecy, void* pBlock, const char* lpszFileType, const char* lpszRemark, const int bIsSyatemImage, const int nSize )
{
	char szSQL[200] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "insert into lib_image_animation(ID,name,specy,imagebinary,filetype,remark,issystemimage) values(%d,'%s','%s',?,'%s','%s',%d)", nID, lpszName, lpszSpecy, lpszFileType, lpszRemark, bIsSyatemImage);
	assert(strlen(szSQL)>0);
	if (SQLITE_OK == sqlite3_prepare( m_db, szSQL, -1, &m_stmt, 0 ))
	{
		if (m_stmt)
		{
			rc = sqlite3_bind_blob(m_stmt,1,pBlock,nSize,NULL);
			rc = sqlite3_step(m_stmt);
			rc = sqlite3_finalize(m_stmt);
		}
	}
	return rc;
}

int CSqliteAcess::InsertRecordToListDashBoard( const int nPageID, const int nID, const char* lpszBindPointName )
{
	char szSQL[500] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	sprintf_s(szSQL, sizeof(szSQL), "INSERT INTO list_dashboard(pageid,id,pointname) VALUES(%d,%d,'%s')", nPageID, nID, lpszBindPointName);
	assert(strlen(szSQL)>0);
	rc = sqlite3_exec(m_db, szSQL, NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::DeleteAllDashBoardData()
{
	int rc = 0;
	rc = sqlite3_exec(m_db, "delete from list_dashboard", NULL, NULL, NULL);
	return rc;
}

int CSqliteAcess::RollbackTransaction()
{
	char* error;
	int result = sqlite3_exec(m_db, "ROLLBACK;", NULL, NULL, &error);

	return result;
}
