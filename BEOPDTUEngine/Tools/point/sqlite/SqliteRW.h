#pragma once

//#include "FileInterface.h"

#include "SqliteAcess.h"
#include "sqlite3.h"
#include <iostream>
#include <sstream>

class CSqliteRW
{
public:
	CSqliteRW(void);
	~CSqliteRW(void);
public:	
//	//存储设备信息
//	bool static GetEquipmentSql(DEVICETYPE type,void* record,string &sql_);
//	bool static SaveProject(string &sql_);
//	bool static SaveChillerPlantRoom(string &sql_);
//	bool static SaveChiller(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SavePump(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveTower(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveCorner(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveHx(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveTerminal(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveWaterSoil(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveValve(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveTank(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SavePipe(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveLoad(CSqliteAcess &sqliteAcess,string &sql_);
//public:
//	//读取项目信息
//	bool static ReadProject(CSqliteAcess &sqliteAcess,gProject &gp,string &sql_);
//	bool static ReadChillerPlantRoom(CSqliteAcess &sqliteAcess,gCHPlantRoom &room,string &sql_);
//	bool static ReadChiller(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Chiller> &chllerlist,string &sql_);
//	bool static ReadPump(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Pump> &pumplist,string &sql_);
//	bool static ReadTower(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_CoolingTower> &towerlist,string &sql_);
//	bool static ReadCorner(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Corner> &cornerlist,string &sql_);
//	bool static ReadHx(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Hx> &hxlist,string &sql_);
//	bool static ReadTerminal(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Terminal> &terminallist,string &sql_);
//	bool static ReadWaterSoil(CSqliteAcess &sqliteAcess,std::vector<WaterSoil_Entry_InRoom> &wslist,string &sql_);
//	bool static ReadValve(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Valve> &valvelist,string &sql_);
//	bool static ReadTank(CSqliteAcess &sqliteAcess,std::vector<Tank_Entry_InRoom> &tanklist,string &sql_);
//	bool static ReadPipe(CSqliteAcess &sqliteAcess,std::vector<gRoom_Contain_Pipe> &pipelist,string &sql_);
//	bool static ReadLoad(CSqliteAcess &sqliteAcess,std::vector<gRoomLoad> &loadlist,string &sql_);
//public:
//	//读取设备模型
//	bool static ReadChillerModel(CSqliteAcess &sqliteAcess,std::vector<gCH_Model> &chllermodellist,string &sql_);
//	bool static ReadPumpModel(CSqliteAcess &sqliteAcess,std::vector<gPump_Model> &pumpmodellist,string &sql_);
//	bool static ReadTowerModel(CSqliteAcess &sqliteAcess,std::vector<gCT_Model> &towermodellist,string &sql_);
//	bool static ReadWaterSoilModel(CSqliteAcess &sqliteAcess,std::vector<gWaterSoil_Model> &wsmodellist,string &sql_);
//public:
//	bool static SaveEquipmentModel(std::vector<gCH_Model> &chllermodellist,
//									std::vector<gPump_Model> &pumpmodellist,
//									std::vector<gCT_Model> &towermodellist,
//									std::vector<gWaterSoil_Model> &wsmodellist);
//
//public:
//	//获取当前设备的属性及模型
//	bool static SaveChillerLibAndModel(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SavePumpLibAndModel(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveTowerLibAndModel(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveWSLibAndModel(CSqliteAcess &sqliteAcess,string &sql_);
//	bool static SaveOtherLib(CSqliteAcess &sqliteAcess,string &sql_);
//
//private:
//	bool static savechillerlib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCHLib> &chLibList);
//	bool static savechillermodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCH_Model> &chModelList);
//	bool static savepumplib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gPumpLib> &pumpLibList);
//	bool static savepumpmodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gPump_Model> &pumpModelList);
//	bool static savetowerlib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCTLib> &ctLibList);
//	bool static savetowermodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gCT_Model> &ctModelList);
//
//	bool static savewslib(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gWSLib> &wsLibList);
//	bool static savewsmodel(CSqliteAcess &sqliteAcess,string &sql_,std::vector<gWaterSoil_Model> &wsModelList);

};

