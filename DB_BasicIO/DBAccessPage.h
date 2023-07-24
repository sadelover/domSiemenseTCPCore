
#ifndef DBACCESSPAGE_H__
#define DBACCESSPAGE_H__

#pragma  once

/*
	This class implement the access of the page elements.				
*/
//#include "../EqmDraw/EqmDrawDevice.h"
//#include "../EqmDraw/EqmDrawPipe.h"
//#include "../EqmDraw/EqmDrawText.h"
//#include "../EqmDraw/EqmDrawProject.h"
//#include "../EqmDraw/EqmDrawGraphic.h"
#include "../OPCdll/CPLCDataPoint.h"

#include "DBInterfaceImp.h"
#include "DatabaseSession.h"
#include "../ComponentDraw/EqmDrawDevice.h"
#include "../ComponentDraw/EqmDrawGraphic.h"
#include "../ComponentDraw/EqmDrawPicture.h"
#include "../ComponentDraw/EqmDrawPipe.h"
#include "../ComponentDraw/EqmDrawText.h"
#include "../ComponentDraw/EqmDrawProject.h"

#include <string>
#include <ostream>
#include <sstream>
#include <map>
#include <vector>

using namespace std;

enum PAGE_CONTAIN_EQUIPMENT
{
	PCE_Ident,			// id
	PCE_PageID,			// pageid
	PCE_EqmType,		// EqmType
	PCE_EqmSubType,		// EqmSubType
	PCE_XPos,			// center x pos
	PCE_YPos,			// center y pos	
	PCE_DataComID,		// data comid
	PCE_RotateAngle,	// rotate angel
	PCE_EqmName,		// equipment name
	PCE_Width,			// width
	PCE_Height,			// height
	PCE_PORTPOINTLIST,
	PCE_STEPINTERVAL,   //间距
	PCE_UPPERLIMIT,     //上限
	PCE_DOWNLIMIT       //下限
};

enum PAGE_CONTAIN_TEXT
{	
	PCT_IDENT,
	PCT_PAGEID,
	PCT_INITIALIALVALUE,
	PCT_XPOS,
	PCT_YPOS,
	PCT_HEIGHT,
	PCT_WIDTH,
	PCT_FONTSIZE,
	PCT_FONTSTYLE,
	PCT_DATACOMID,
	PCT_FONTCOLOR,
	PCT_SHOWMODE,
	PCT_TRANSPARENCE,	//透明度
};

enum PAGE_CONTAIN_PIPE
{
	PCP_IDENT,
	PCP_PAGEID,
	PCP_INNERCOLOR,
	PCP_OUTERCOLOR,
	PCP_STRPOINTLIST
};

enum PAGE_CONTAIN_GRAPHIC
{
	PCG_ident,
	PCG_pageid,
	PCG_GraphicType,
	PCG_LineColor,
	PCG_LineStyle,
	PCG_LineWidth,
	PCG_FilledColor,
	PCG_PointList,
	PCG_ForwardPageID,
	PCG_TRANSPARENCE,	//透明度
};



enum PROJECT
{
	PROJECT_ID,
	PROJECT_Name,
	BACKCOLOR
};


enum PAGELIST
{
	PAGELIST_ID,
	PAGELIST_NAME,
	PAGELIST_PROJECTID,
	PAGELIST_SHOWORDER
};

enum PLCPointMap
{
	PPM_IDENT,
	PPM_PLCNAME,
	PPM_SHORTNAME,
	PPM_RWPROPERTY,
	PPM_REMARK,
	PPM_UNIT
};

enum PAGE_CONTAIN_PICTURE
{
	PCP_ident,
	PCP_Picpath,
	PCP_PicStateNum
};



//用来显示uppc client的弹出界面
enum DEVICEINFOTYPE
{
	DEVICEINFOTYPE_CHILLER,			//冷机
	DEVICEINFOTYPE_COOLINGPUMP,		//冷却泵
	DEVICEINFOTYPE_VALVE,			//阀门
	DEVICEINFOTYPE_COOLINGTOWER,	//冷却塔
	DEVICEINFOTYPE_PRICHILLEDPUMP,	//一次冷冻泵
	DEVICEINFOTYPE_SECCHILLEDPUMP,	//二次冷冻泵
};


// table create statement.

struct ProjectDB
{
	wstring projectname;
	string dbname;
};

class DBDLL DBAccessPage : public CDatabaseSeesion
{
public:
	// constructor
	DBAccessPage();
	~DBAccessPage();

public:
	// create project.
	// this function will create the database needed for this project.
	// and returns the database name.
	bool	CreateProject(CEqmDrawProject* project, string& dbname);
	
	// this class will update contents the table project.
	bool	UpdateProjectInfo();
	
	// 保存接口
	/*	save one page,this function will update the info of the 
		table pagecontaindevice, pagecontainpipe, pagecontaintext
	*/
	bool	SavePage(CEqmDrawPage* page);
	
	/*
		This function will save all the pages of the project.
		first, it will delete all the pages ,
		and then insert all the pages into the database.
	*/
	bool	SaveProject(CEqmDrawProject* project);
	
		
	//删除接口,支持 删除一整页
	//删除单个设备单个设备的功能

	// this function will delete the object in the pagecontain<XXX>.
	
	// this function will delete the pageid entry in the table  "pagelist".
	bool	DeletePage(CEqmDrawPage* page);
	bool	DeleteProject();

	//取得页面上对象的最大ID
	// this function will query the table pagecontaindevice,
	// pagecontainpipe, pagecontaintext, and get the max id of the specified page.
	int		GetMaxIDOFPageObjects(int pageid);

	// this function will return the max value of the table pagelist.
	int		GetMaxIDOFPageList();

	//读工程
	// read once page can be devided into reading more pages.
	//read one page, this function will scans the table "pagecontaindevice",
	// "pagecontaintext",  "pagecontainpipe";
	bool ReadPage(int pageid, CEqmDrawPage* page);

	// this function will first get the page list.
	// then searches the pagecontaintable .
	bool	ReadProject(CEqmDrawProject* project);
	bool	ReadAllProjectFromServer(vector<ProjectDB>& resultliust);

	//PLC点位表相关接口
	bool	ReadPlcPointMap(CPlcDataPointMap& plcpointmap);
	
	// insert into the point map to the database.
	bool	SavePlcPointMap(CPlcDataPointMap& plcpointmap);

	//从xml文件导入到数据库的表中
	bool	LoadPlcPointMapFromXml(const wstring& filepath, 
									CPlcDataPointMap& pointmap);

	bool	GetDevicePropertyList(vector<DataPointMapEntry>& resultlist, int start);

private:
	//读一个页面中的设备
	bool	ReadPageEquipments(CEqmDrawPage* page);

	//读一个页面的文本
	bool	ReadPageTexts(CEqmDrawPage* page);

	//读一个页面的管道.
	bool	ReadPagePipes(CEqmDrawPage* page);

	//读一个页面的2d图形
	bool	ReadPage2DGraphic(CEqmDrawPage* page);

	//保存一个页面中的设备
	bool	SavePageEquipments(CEqmDrawPage* page);

	//保存一个页面的文本
	bool	SavePageTexts(CEqmDrawPage* page);

	//保存一个页面的管道.
	bool	SavePagePipes(CEqmDrawPage* page);

	bool	SavePage2DGraphic(CEqmDrawPage* page);

	bool    ReadProjectInfo(CEqmDrawProject* project);

	bool	ReadDatabaseList(vector<string>& dbnames);

	bool    ReadPicInfo(CEqmDrawProject* page);
	bool    SavePictueInfo( CEqmDrawProject* page );
	bool    SaveProjectInfo(CEqmDrawProject* project);
};

#endif







