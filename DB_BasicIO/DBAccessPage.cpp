
#include "stdafx.h"

#include "DBInterfaceExp.h"
#include "DBAccessPage.h"
#include "CreateTableSql.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/Util/UtilString.h"  //bird
#include "../Tools/Scope_Ptr.h"
//#include "../ParseXML/XMLDoc.h"
#include "../TinyXml/tinyxml.h"  //bird 2.25

#include "MysqlDeleter.h"

//#include "vld.h"

using namespace Project::Tools;

#define DBNAMEPREFIX "MonitorViewDB_"

bool DBAccessPage::CreateProject( CEqmDrawProject* project, string& dbname )
{
	SYSTEMTIME sttime;
	GetLocalTime(&sttime);
	char temp[50];
	memset(temp, 0x00, sizeof(temp));
	sprintf_s(temp, sizeof(temp), "%d%d%d%d%d%d%d", sttime.wYear, sttime.wMonth, sttime.wDay, \
					sttime.wHour, sttime.wMinute, sttime.wSecond, sttime.wMilliseconds);
	
	dbname = DBNAMEPREFIX;
	dbname += temp;
	
	wstring dbname_w = Project::Tools::AnsiToWideChar(dbname.c_str());
	project->SetDatabaseName(dbname_w);
	ostringstream sqlstream;
	sqlstream << "create database " << dbname << " DEFAULT CHARACTER SET utf8";
	string sql = sqlstream.str();

	if (!Execute(sql))
	{
		string error = GetMysqlError();
		// log the error
		return false;
	}
	
	SelectDB(dbname.c_str());

	if (Execute(sql_create_project) 
		&& Execute(sql_create_pagelist)
		&& Execute(sql_create_pagecontainequipment) 
		&& Execute(sql_create_pagecontainpipe)
		&& Execute(sql_create_pagecontaintext)
		&& Execute(sql_create_PageContainGraphic)
		&& Execute(sql_create_view_pagemaxident) 
		&& Execute(sql_create_plcpointmap)
		&& Execute(sql_create_picturelib)
		 )
	{
		string projectname;
		Project::Tools::WideCharToUtf8(project->GetProjectName(), projectname);
		
		sqlstream.str("");
		sqlstream	<< "insert into project values(" 
					<< project->GetProjectID()
					<< ", '"<< projectname <<", '"<< project->GetBackGroundColor()<< ")";
		sql = sqlstream.str();
		string sqlPic = "insert into pagecontainpic values (1,'Chiller.png',4),(2,'pump.png',4),(3,'CT.png',4),(4,'Valve.png',4),(5,'UpT.png',4),(6,'DownT.png',4),(7,'UpQ.png',4),(8,'DP.png',4),(9,'redarrow.png',4),(10,'bluearrow.png',4),(11,'redlight.png',4),(12,'greenlight.png',4),(13,'yellowlight.png',4),(14,'boolbtn.png',4),(15,'incbtn.png',4),(16,'decbtn.png',4),(17,'unknowDevice.png',4),(18,'unknowDevice.png',4)";
		if (Execute(sql)&&Execute(sqlPic))
			return true;
		else
		{
			string error = GetMysqlError();
			DropDB(dbname.c_str());
			return false;
		}
	}
	else
	{
		DropDB(dbname.c_str());
		return false;
	}
	
	return true;
}

DBAccessPage::~DBAccessPage()
{}

DBAccessPage::DBAccessPage()
{}


// first . reomove the page
// then , add the page.
bool DBAccessPage::SavePage( CEqmDrawPage* page )
{
	assert(page != NULL);
	// remove the pagecontainXXX table entries.	
	ostringstream sqlstream;
	sqlstream	<< "delete from pagelist where pageid = " 
				<< page->GetID();
	string sql = sqlstream.str();

	// this will start a transaction
	if (StartTransaction())
	{
		// delete the page first. rollback if failed.
		if (!Execute(sql))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}

		// insert into the pagelist.
		sqlstream.str("");
		string pagename;
		Project::Tools::WideCharToUtf8(page->GetPageName(), pagename);
		sqlstream	<< "insert into pagelist values(" 
					<< page->GetID() << ",'" 
					<< pagename <<  "'," 
					<< page->GetProjectID() << ", "
					<< page->GetPageShowOrder() << ")";
		sql = sqlstream.str();
		if (!Execute(sql))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		
		//save to pagecontainequipment
		if (!SavePageEquipments(page))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		
		// save to pagecontainpipes
		if (!SavePagePipes(page))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		
		//save pagecontaintexts
		if (!SavePageTexts(page))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}

		if (!SavePage2DGraphic(page))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}

		
		return Commit();
	}						
	
	return false;
}

bool DBAccessPage::SaveProject( CEqmDrawProject* project )
{
	assert(project != NULL);
	// first, it will delete all the pages.
	// start a transaction
	if (StartTransaction())
	{
		if ( !SaveProjectInfo(project) )
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		if ( !Execute("delete from pagecontainpic") )
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		
		if ( !SavePictueInfo(project) )
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		// delete all the pages
		//
		if (!Execute("delete from pagelist"))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		//
		vector<CEqmDrawPage*>& pagelist = project->GetPageList();
		vector<CEqmDrawPage*>::iterator it = pagelist.begin(); 
		for (;it != pagelist.end(); it++)
		{
			CEqmDrawPage* page = *it;
			if (!SavePage(page))
			{
				string error = GetMysqlError();
				RollBack();
				return false;
			}
		}
		return Commit();
	}	// end of outer if.

	return false;
}

bool DBAccessPage::DeletePage(CEqmDrawPage* page)
{
	ostringstream sqlstream;
	sqlstream << "delete from pagelist where pageid = " << page->GetID();
	string sql = sqlstream.str();

	// this will start a transaction
	if (StartTransaction())
	{
		// delete the page first.
		if (!Execute(sql))
		{
			string error = GetMysqlError();
			RollBack();
			return false;
		}
		return Commit();
	}	

	return false;
}

//delete the project
// and delete the database
// drop the database
bool DBAccessPage::DeleteProject()
{
	return DropDB(m_mysql.db);
}

// we create view of the three table idents.
// and we select max id from the view.
int DBAccessPage::GetMaxIDOFPageObjects( int pageid )
{
	string sql = "select max(max_id) from page_max_ident";
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;

	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	//if there is no entry in the table, the function will retrun 0
	MYSQL_ROW row = FetchRow(result);	
	int id = ATOI(row[0]);

	return id;
}

// get the max value of the pagelist.ident
int DBAccessPage::GetMaxIDOFPageList()
{
	// get
	string sql = "select max(PageID) from pagelist";
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;

	//if there is no entry in the table, the function will retrun 0
	MYSQL_ROW row = FetchRow(result);
	int id = ATOI(row[0]);
	FreeReturnResult(result);

	return id;
}

bool DBAccessPage::ReadPage( int pageid,  CEqmDrawPage* page)
{
	assert(page != NULL);
	if (!m_isconnected)
		return false;
	
	// read table pagecontainequipment
	if (!ReadPageEquipments(page))
		return false;
	
	// read table pagecontaintext
	if (!ReadPageTexts(page))
		return false;

	// read table pagecontainpipe.
	if (!ReadPagePipes(page))
		return false;
	
	if (!ReadPage2DGraphic(page))
		return false;

	return true;
}

bool DBAccessPage::ReadProject(CEqmDrawProject* project)
{	
	assert(project != NULL);
	// read project name and id
	if (!ReadProjectInfo(project))
		return false;
	
	wstring dbname = Project::Tools::AnsiToWideChar(m_mysql.db);
	project->SetDatabaseName(dbname);
	// read all pages.
	try
	{
		if ( !ReadPicInfo(project) )
		{
			return false;
		}
		ostringstream sqlstream;
		sqlstream << "select * from pagelist where projectid=" 
				  << project->GetProjectID()
				  << " order by pageshoworder";
		string sql = sqlstream.str();
		MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
		if (result == NULL)
			return false;
		
		scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
		MYSQL_ROW row;
		while(row = FetchRow(result))
		{
			int id = ATOI(row[PAGELIST_ID]);
			wstring pagename;
			Project::Tools::UTF8ToWideChar(row[PAGELIST_NAME], pagename);
			int showorder = ATOI(row[PAGELIST_SHOWORDER]);
			CEqmDrawPage* page = new CEqmDrawPage(id, pagename, project->GetProjectID(),showorder);
			if (!ReadPage(id, page))
			{
				page->ClearAll();
				delete page;
				return false;
			}	
			project->AddPage(page);
		}
	}
	catch(...)
	{
		return false;
	}

	return true;
}

bool DBAccessPage::ReadAllProjectFromServer( vector<ProjectDB>& resultlist )
{	
	// get the database names;
	vector<string> dbnames;
	if (!ReadDatabaseList(dbnames))
		return false;

	// fetch all the project names
	vector<string>::iterator it = dbnames.begin();
	for (; it != dbnames.end(); it++)
	{
		string sql = "select name from ";
		sql += *it;
		sql += ".project ";
		
		// get the project name and picture.
		if (mysql_query(&m_mysql, sql.c_str()))
			continue;
		
		MYSQL_RES* result = mysql_use_result(&m_mysql);
		if (result != NULL)
		{			
			scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
			MYSQL_ROW row = mysql_fetch_row(result);
			if (row != NULL)
			{
				// if there is result, then create and save the info.			
				wstring projectname ;
				Project::Tools::UTF8ToWideChar(row[0], projectname);

				ProjectDB pdb;
				pdb.projectname = projectname;
				pdb.dbname = *it;
				resultlist.push_back(pdb);
			}
		}		
	}
	
	return true; 
}

bool DBAccessPage::ReadPlcPointMap( CPlcDataPointMap& plcpointmap )
{
	string sql = "select * from PLCPointMap order by ident";
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;
	
	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	plcpointmap.Clear();
	MYSQL_ROW row;
	while(row = FetchRow(result))
	{
		DataPointMapEntry entry;
		//memset(&entry, 0x00, sizeof(entry));

		entry.index = ATOI(row[PPM_IDENT]);
		Project::Tools::UTF8ToWideChar(row[PPM_PLCNAME], entry.plcname);
		Project::Tools::UTF8ToWideChar(row[PPM_SHORTNAME], entry.shortname);
		entry.rwproperty = (PLCVALUEPROPERTY)ATOI(row[PPM_RWPROPERTY]);
		Project::Tools::UTF8ToWideChar(row[PPM_REMARK], entry.description);	
		Project::Tools::UTF8ToWideChar(row[PPM_UNIT], entry.unit);	
		plcpointmap.AddEntry(entry);	
	}
	return true;
}

bool DBAccessPage::SavePlcPointMap( CPlcDataPointMap& plcpointmap )
{
	const map<int, DataPointMapEntry>& pointmap = plcpointmap.GetPointMap();
	if (pointmap.empty())
		return true;

	// clear the table.
	if (!Execute("delete from PLCPointMap"))
		return false;
	
	ostringstream sqlstream;
	sqlstream << "insert into PLCPointMap values";
	
	map<int, DataPointMapEntry>::const_iterator it = pointmap.begin();
	for (; it != pointmap.end(); it++)
	{
		DataPointMapEntry pentry = (*it).second;
		
		string plcname;
		Project::Tools::WideCharToUtf8(pentry.plcname, plcname);
		string shortname;
		Project::Tools::WideCharToUtf8(pentry.shortname, shortname);
		string remark;
		Project::Tools::WideCharToUtf8(pentry.description, remark);
		string unit;
		Project::Tools::WideCharToUtf8(pentry.unit, unit);

		sqlstream	<< "(" 
					<< pentry.index << ",'" 
					<< plcname << "','" 
					<< shortname << "'," 
					<< pentry.rwproperty << ",'"  
					<< remark << "','" 
					<< unit<< "'),"	;	
	}
	
	string sql = sqlstream.str();
	if (!sql.empty())
	{
		sql.erase(--sql.end()); // delete the last ","
		if (!Execute(sql))
			return false;
	}
	
	return true;
}

//bird tinyxml 2.25
bool DBAccessPage::LoadPlcPointMapFromXml( const wstring& filepath, CPlcDataPointMap& pointmap)
{
	// clear the table.
	if (!Execute("delete from PLCPointMap"))
		return false;

	//// first, we load the document.
    TiXmlDocument xmlDoc;

    const string strFileName = UtilString::ConvertWideCharToMultiByte(filepath);
    const bool ifload = xmlDoc.LoadFile(strFileName.c_str() );
    if(!ifload)
        return false;

    TiXmlElement* pElemRoot = xmlDoc.RootElement();
    assert(pElemRoot);
    if(!pElemRoot)
        return false;
		
    TiXmlElement* pElem = pElemRoot->FirstChildElement();
    assert(pElem);

    string str;
    int tempInt = 0;
    pointmap.Clear();
    DataPointMapEntry entry;

    while(pElem)
    {
        str = pElem->Value();
        if(str != "Data" )
            break;

        //OPC id
        pElem->Attribute("index", &tempInt);
        entry.index = tempInt;

        // get shortname
        str =pElem->Attribute("name");
        entry.shortname  = UtilString::ConvertMultiByteToWideChar(str);

        //plc name
        str = pElem->Attribute("value");
        entry.plcname = UtilString::ConvertMultiByteToWideChar(str);

        //read write
        str = pElem->Attribute("RWProperty" );
        entry.rwproperty = (str == ("R")) ? PLC_READ : PLC_WRITE;

        //remark
        str = pElem->Attribute("remark");
        entry.description = UtilString::ConvertMultiByteToWideChar(str);

        //unit
        str = pElem->Attribute("Unit");
        entry.unit =  UtilString::ConvertMultiByteToWideChar(str);

        pointmap.AddEntry(entry);

        pElem = pElem->NextSiblingElement();
    }

	return SavePlcPointMap(pointmap);
}

bool DBAccessPage::GetDevicePropertyList( vector<DataPointMapEntry>& resultlist, int start )
{
	ostringstream sqlstream;
	sqlstream << "select * from PLCPointMap where ident >= " 
			  << start 
			  << " and ident < " 
			  << start + 99 
			  <<  " order by ident";

	string sql = sqlstream.str();
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return NULL;
		
	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	MYSQL_ROW row;
	while(row = FetchRow(result))
	{
		DataPointMapEntry entry;
		//memset(&entry, 0x00, sizeof(entry));

		entry.index = ATOI(row[PPM_IDENT]);
		Project::Tools::UTF8ToWideChar(row[PPM_PLCNAME], entry.plcname);
		Project::Tools::UTF8ToWideChar(row[PPM_SHORTNAME], entry.shortname);
		entry.rwproperty = (PLCVALUEPROPERTY)ATOI(row[PPM_RWPROPERTY]);
		Project::Tools::UTF8ToWideChar(row[PPM_REMARK], entry.description);	
		Project::Tools::UTF8ToWideChar(row[PPM_REMARK], entry.unit);		

		resultlist.push_back(entry);
	}

	return true;
}

bool DBAccessPage::ReadPageEquipments( CEqmDrawPage* page )
{
	ostringstream sqlstream;
	int pageid = page->GetID();

	sqlstream << "select * from pagecontainequipment where pageid = " << page->GetID() << endl;
	string sql = sqlstream.str();
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;

	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);

	MYSQL_ROW row;
	while (row = FetchRow(result))
	{
		int id = ATOI(row[PCE_Ident]);
		//pageid is ommited here since we have it.
		int eqmtype = ATOI(row[PCE_EqmType]);
		int eqmsubtype = ATOI(row[PCE_EqmSubType]);
		int xpos = ATOI(row[PCE_XPos]);
		int ypos = ATOI(row[PCE_YPos]);		
		int datacomid = ATOI(row[PCE_DataComID]);
		int rotateangle = ATOI(row[PCE_RotateAngle]);   
		wstring strEqmName;
		Project::Tools::UTF8ToWideChar(row[PCE_EqmName], strEqmName);     // equipment name new added
		int width = ATOI(row[PCE_Width]);
		int height = ATOI(row[PCE_Height]);

		string strPointList = row[PCE_PORTPOINTLIST];

		double dStepInterval = ATOF(row[PCE_STEPINTERVAL]);

		double dUpperLimit = ATOF(row[PCE_UPPERLIMIT]);

		double dDownLimit  = ATOF(row[PCE_DOWNLIMIT]);

		CEqmDrawDevice* device = new CEqmDrawDevice((EQMDRAWTYPE)eqmtype, Gdiplus::PointF((Gdiplus::REAL)xpos, (Gdiplus::REAL)ypos), 
								pageid, id, datacomid, rotateangle, strEqmName, 
								(EQUIPMENTSUBTYPE)eqmsubtype, width, strPointList, height, dStepInterval, dUpperLimit, dDownLimit);
		page->AddDevice(device);
	}
	
	return true;
}

bool DBAccessPage::ReadPageTexts( CEqmDrawPage* page )
{
	// query table pagecontaintext
	ostringstream sqlstream;
	
	int pageid = page->GetID();

	sqlstream << "select * from pagecontaintext where pageid = "  << pageid;
	string sql = sqlstream.str();

	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;
	
	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	MYSQL_ROW row3;
	while (row3 = FetchRow(result))
	{
		int id = ATOI(row3[PCT_IDENT]);
		wstring initialvalue;
		Project::Tools::UTF8ToWideChar(row3[PCT_INITIALIALVALUE], initialvalue);
		int xpos = ATOI(row3[PCT_XPOS]);
		int ypos = ATOI(row3[PCT_YPOS]);
		int height = ATOI(row3[PCT_HEIGHT]);
		int width = ATOI(row3[PCT_WIDTH]);
		int fontsize = ATOI(row3[PCT_FONTSIZE]);
		int fontstyle = ATOI(row3[PCT_FONTSTYLE]);
		int datacomid = ATOI(row3[PCT_DATACOMID]);
		int fontcolor = ATOI(row3[PCT_FONTCOLOR]);
		int	showmode = ATOI(row3[PCT_SHOWMODE]);
		double transparence = ATOF(row3[PCT_TRANSPARENCE]);

		CEqmDrawText* text = new CEqmDrawText(Gdiplus::PointF((Gdiplus::REAL)xpos, (Gdiplus::REAL)ypos), pageid, id, width, 
									height, fontsize, fontstyle, datacomid,
									initialvalue, fontcolor, (TEXTSHOWMODE)showmode,
									transparence);
		page->AddText(text);
	}
	
	return  true;
}

bool DBAccessPage::ReadPagePipes( CEqmDrawPage* page )
{
	// query table pagecontainpipe
	ostringstream sqlstream;
	
	int pageid = page->GetID();

	sqlstream << "select * from pagecontainpipe where pageid = " << pageid;
	string sql = sqlstream.str();
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;
	
	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	MYSQL_ROW row2;
	while (row2 = FetchRow(result))
	{
		int id = ATOI(row2[PCP_IDENT]);
		//pageid
		PipeColor pipecolor;
		pipecolor.innercolor = ATOI(row2[PCP_INNERCOLOR]);
		pipecolor.outercolor = ATOI(row2[PCP_OUTERCOLOR]);
		string strPointList = row2[PCP_STRPOINTLIST];
		CEqmDrawPipe* pipe = new CEqmDrawPipe(pageid, id, pipecolor, strPointList );
		page->AddPipe(pipe);
	}
	
	return true;
}

bool DBAccessPage::SavePageEquipments( CEqmDrawPage* page )
{
	assert(page != NULL);
	list<CEqmDrawDevice*>& devicelist = page->GetDrawDeviceList();
	if (!devicelist.empty())
	{
		ostringstream sqlstream;
		sqlstream << "insert into pagecontainequipment values";
		list<CEqmDrawDevice*>::iterator it = devicelist.begin();
		for (; it != devicelist.end(); it++)
		{
			string strEqmName;
			strEqmName.clear();
			(*it)->SetPointList((*it)->GetPointList());
			Project::Tools::WideCharToUtf8((*it)->GetEqmName(), strEqmName);
			sqlstream	<< "(" << (*it)->GetID() << "," 
						<< (*it)->GetPageID() << ","
						<< (*it)->GetEqmType() << "," 
						<< (*it)->GetEquipmentSubType() << ","
						<< (*it)->GetCenterPos().X << ","
						<< (*it)->GetCenterPos().Y << "," 
						<< (*it)->GetDataComID() << "," 
						<< (*it)->GetRotateAngle() << ",'" 
						<< strEqmName << "'," 
						<< (*it)->GetWidth() << "," 
						<< (*it)->GetHeight() << ",'"	
						<< (*it)->GetPointStringList() << "',"
						<< (*it)->GetStepInterval() << ","
						<< (*it)->GetUpperLimit() << ","
						<< (*it)->GetDownLimit() <<  "),";
		}

		string sql = sqlstream.str();		
		sql.erase(--sql.end());
		//插入不成功则回滚
		return Execute(sql);
	}

	return true;
}

bool DBAccessPage::SavePageTexts( CEqmDrawPage* page )
{
	// insert into the table <pagecontaintext>
	//一次插入多行以提高速度
	assert(page != NULL);

	list<CEqmDrawText*>& textlist = page->GetPageTextList();
	if (!textlist.empty())
	{
		ostringstream sqlstream;
		sqlstream << "insert into  pagecontaintext values " ;
		list<CEqmDrawText*>::iterator it = textlist.begin();
		for (;it != textlist.end(); it++)
		{
			string initialvalue;
			Project::Tools::WideCharToUtf8((*it)->GetInitialstring(), initialvalue);
			sqlstream	<< "(" << (*it)->GetID() << "," 
						<< (*it)->GetPageID() << ",'"
						<< initialvalue << "'," 
						<< (*it)->GetCenterPos().X <<  ","
						<< (*it)->GetCenterPos().Y << "," 
						<< (*it)->GetHeight() << ","
						<< (*it)->GetWidth() << "," 
						<< (*it)->GetFontSize() << ","
						<< (*it)->GetFontStyle() << "," 
						<< (*it)->GetComID() << ","
						<< (*it)->GetFontColor() << "," 
						<< (*it)->GetTextShowMode() << ","
						<< (*it)->GetTransparence() << "),";
		}
		string sql = sqlstream.str();
		
		//删除最后一个',',并执行查询
		sql.erase(--sql.end());			
		return Execute(sql);
				
	}

	return true;
}

bool DBAccessPage::SavePagePipes( CEqmDrawPage* page )
{
	assert(page != NULL);

	list<CEqmDrawPipe*>& pipelist = page->GetPagePipeList();
	if (!pipelist.empty())
	{
		ostringstream sqlstream;
		sqlstream << "insert into  pagecontainpipe values " ;
		list<CEqmDrawPipe*>::iterator it = pipelist.begin(); 
		for (;it != pipelist.end(); it++)
		{
			(*it)->SetPointLIst((*it)->GetPointList());
			sqlstream	<< "(" 
						<< (*it)->GetID() << "," 
						<< (*it)->GetPageID() << ","
						<< (*it)->GetPipeColor().innercolor << "," 
						<< (*it)->GetPipeColor().outercolor  << ",'" 
						<< (*it)->GetPointStringList() << "'),";
		}
		string sql = sqlstream.str();

		//删除最后一个',',并执行查询			
		sql.erase(--sql.end());	
		return	Execute(sql);				
	}
	return true;
}

bool DBAccessPage::ReadProjectInfo( CEqmDrawProject* project )
{
	try 
	{
		string sql = "select * from project";
		MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
		if (result == NULL)
			return false;
		scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
		MYSQL_ROW row = FetchRow(result);
		if (row == NULL)
			return false;
		int projectid = ATOI(row[PROJECT_ID]);
		project->SetProjectID(projectid);
		wstring proejctname;
		Project::Tools::UTF8ToWideChar(row[PROJECT_Name], proejctname);
		project->SetProjectName(proejctname);
		DWORD dwBackColor = ATOI(row[BACKCOLOR]);
		project->SetBackGroundColor(dwBackColor);
		return true;
	}catch(...)
	{
	}

	return false;
}

bool DBAccessPage::ReadDatabaseList( vector<string>& dbnames )
{

	MYSQL_RES* result = QueryAndReturnResult("show databases like 'monitorviewdb%%'");
	if (result == NULL)
	{
		string error = GetMysqlError();
		return false;
	}

	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	MYSQL_ROW row;
	while(row = FetchRow(result))
	{
		string name  = row[0];
		dbnames.push_back(name);
	}

	return true;
}

bool DBAccessPage::ReadPage2DGraphic( CEqmDrawPage* page )
{
	ostringstream sqlstream;
	sqlstream << "select * from pagecontaingraphic where pageid = " << page->GetID();
	string sql = sqlstream.str();
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;
	
	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	MYSQL_ROW row = NULL;
	while(row = FetchRow(result))
	{
		int id = ATOI(row[PCG_ident]);
		int pageid = ATOI(row[PCG_pageid]);
		int graphictype = ATOI(row[PCG_GraphicType]);
		int linecolor = ATOI(row[PCG_LineColor]);
		int linestyle = ATOI(row[PCG_LineStyle]);
		int linewidth = ATOI(row[PCG_LineWidth]);
		int filledcolor = ATOI(row[PCG_FilledColor]);
		string strpointlist = row[PCG_PointList];	
		int forwardpageid = ATOI(row[PCG_ForwardPageID]);
		double transparence = ATOF(row[PCG_TRANSPARENCE]); //no use, bird add comment 3.1

        //CEqmDrawGraphic(EQMDRAWTYPE type, const Gdiplus::PointF& centerpos, int pageid, 
        //    int ident,  int linecolor,int filledcolor,
        //    int linewidth, int linestype, const string& pointlist,
        //    int forwardpageid, int width = 0, int height = 0);
		CEqmDrawGraphic* pgrapic = new CEqmDrawGraphic(
			(EQMDRAWTYPE)graphictype, Gdiplus::PointF(0,0), pageid, 
							id, linecolor, filledcolor, linewidth, linestyle, 
							strpointlist, forwardpageid, 0, 0);  
		if (pgrapic != NULL)
		{
			page->Add2DGraphic(pgrapic);
		}	
	}
	
	return true;
}

bool DBAccessPage::SavePage2DGraphic( CEqmDrawPage* page )
{
	assert(page != NULL);

	list<CEqmDrawGraphic*>& graphiclist = page->Get2DGraphicList();
	if (!graphiclist.empty())
	{
		ostringstream sqlstream;
		sqlstream << "insert into pagecontaingraphic values " ;
		list<CEqmDrawGraphic*>::iterator it = graphiclist.begin();
		for (;it != graphiclist.end(); it++)
		{
			assert(*it != NULL);
			CEqmDrawGraphic* graphic = (*it);
			graphic->SetPointList(graphic->GetPointList());
			sqlstream	<< "(" << graphic->GetID() << "," 
						<< graphic->GetPageID() << ","
						<< graphic->GetEqmType() << "," 
						<< graphic->GetLineColor() <<  ","
						<< graphic->GetLineStyle() << "," 
						<< graphic->GetLineWidth() << ","
						<< graphic->GetFilledColor() << ",'" 
						<< graphic->GetPointStringList() << "',"
						<< graphic->GetForwardPageID() << ","
						<< graphic->GetTransparent() << "),";
		}
		string sql = sqlstream.str();

		//删除最后一个',',并执行查询
		sql.erase(--sql.end());			
		return Execute(sql);
	}

	return true;
}

bool DBAccessPage::SavePictueInfo( CEqmDrawProject* project )
{
	assert(project != NULL);


	if (!project->GetEqmDrawPic()->GetPicList().empty())
	{
		ostringstream sqlstream;
		sqlstream << "insert into pagecontainpic values " ;
		map<int, PicProterty>::iterator it = project->GetEqmDrawPic()->GetPicList().begin();
		for (;it != project->GetEqmDrawPic()->GetPicList().end(); it++)
		{
			if ( (*it).second.type< 0 || (*it).second.type> 20 )
				continue;
			sqlstream	<< "(" << (*it).second.type << ",'" 
				<< (*it).second.strPicPath << "',"
				<< (*it).second.nNum << "),";
		}
		string sql = sqlstream.str();

		//删除最后一个',',并执行查询
		sql.erase(--sql.end());			
		return Execute(sql);
	}

	return true;
}

bool DBAccessPage::ReadPicInfo( CEqmDrawProject* project )
{
	ostringstream sqlstream;
	sqlstream << "select * from pagecontainpic ";
	string sql = sqlstream.str();
	MYSQL_RES* result = QueryAndReturnResult(sql.c_str());
	if (result == NULL)
		return false;

	scope_ptr_holder<MYSQL_RES, Mysql_Result_Deleter> resultholder(result);
	MYSQL_ROW row = NULL;
	project->GetEqmDrawPic()->GetPicList().clear();
	while(row = FetchRow(result))
	{
		PicProterty pic;
		pic.type = ATOI(row[PCP_ident]);
		pic.strPicPath = row[PCP_Picpath];
		pic.nNum = ATOI(row[PCP_PicStateNum]);
		project->GetEqmDrawPic()->AddPicture(pic);	
	}
	project->GetEqmDrawPic()->InitBitmapList();
	return true;
}

bool DBAccessPage::SaveProjectInfo( CEqmDrawProject* project )
{
	assert(project != NULL);


	ostringstream sqlstream;
	sqlstream	<< "update project set BackGroundColor = "<< project->GetBackGroundColor()<< ")";
	string sql = sqlstream.str();

	//删除最后一个',',并执行查询
	sql.erase(--sql.end());			
	return Execute(sql);

	return true;
}