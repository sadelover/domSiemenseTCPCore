
#ifndef _SQLITE_ACESS_H_
#define _SQLITE_ACESS_H_
extern "C"
{
#include "sqlite3.h"
};


#include "../../Tools/DllDefine.h"
#include "vector"
#include "string"
using namespace std;
/*******************************************************
sqlite文件标题的操作，写入时要GBToUTF8，读取时UTF8ToGB
不过写入表中的汉字不需要转
********************************************************/

class DLLEXPORT CSqliteAcess
{
public:
	CSqliteAcess(const std::string lpszPath,const std::string lpszPsw = "RNB.beop-2013");
	~CSqliteAcess(void);

public:
	sqlite3*						GetDB() const;
	sqlite3_stmt*                   GetDataSet() const;
	int								SqlExe(const char* sqlr);
	bool							GetOpenState();
	int								OpenDataBase(const char* pathName,const std::string lpszPsw = "RNB.beop-2013");
	int								CloseDataBase();

	int								CreateTable(const char* sqlr);
	int								InquiryPumpModel(const int pumpModelID);
	int								BeginTransaction();	
	int								CommitTransaction();
	int								RollbackTransaction();
	int								SqlQuery(const char* sqlr);
	int								SqlNext();
	int								SqlFinalize();
	int								getColumn_Count();
	double							getColumn_Double(const int i);
	int								getColumn_Int(const int i);
	const char*						getColumn_Text(const int i);
	const void*						getColumn_Blob(const int i);
	int								getColumn_Bytes(const int i);
	char*							GetLocale();
	int								SqlBind_Blob(char *pBlock,int iOrder ,int nSize);

	int								InsertRecordIntoLibImage(const int nID, const char* lpszName, const char* lpszSpecy, void* pBlock, const char* lpszFileType, const char* lpszRemark, const int bIsSyatemImage, const int nSize);
	int								InsertRecordIntoLibImageAnimation(const int nID, const char* lpszName, const char* lpszSpecy, void* pBlock, const char* lpszFileType, const char* lpszRemark, const int bIsSyatemImage, const int nSize);
	int								SqlStep();
	//清空project_config表
	int								DeleteProjectConfigData();
	//插入数据到project_config表
	int								InsertRecordToProjectConfig(const int nID, const char* lpszName, const int nWidth, const int nHeight, const int nVersion = 3, const char* lpszXupTime = "");
	//清空page表
	int								DeleteAllPageData();
	//插入数据到page表
	int								InsertRecordToPage(const char* lpszType, const int nStartColor, const int nEndColor, const int nImageID, const int nID, const char* lpszName, const int nShowOrder, const int nBackGroundColor, 
														const int nWidth, const int nHeight, const int nGroupID, const int nXPos=0, const int nYPos=0);
	//清空page_contain_elements表
	int								DeleteAllContainElements();
	//插入数据到page_contain_elements表
	int								InsertRecordToContainElements(const int nID, const char* lpszName, const int nType, const int nXPos, const int nYPos, const int nWidth, const int nHeight, const char* lpszDataComID, 
									const int nLayer, const int nPageID, const int nRotateAngle, const int nLinkPageID,const char* lpszShowString,const int nPicID, const int nSubType = 0);
	//清空event表
	int								DeleteAllEventData();
	//插入数据到event表
	int								InsertRecordToEvent(const int nPageID, const int nID, const char* lpszEventContent, const char* lpszBindContent);
	//清空list_line表
	int								DeleteAllLineData();
	//插入数据到list_line表
	int								InsertRecordToLine(const int nPageID, const int nID, const int nInnerColor, const int nOutterColor, const char* lpszPointList, const int nWidth, const int nLayer);
	//清空list_text表
	int								DeleteAllTextData();
	//插入数据到list_text表
	int								InsertRecordToText(const int nID, const int nPageID, const char* lpszInitialString, const int nXPos, const int nYPos,
														const int nWidth, const int nHeight, const int nFontSize, const int nFontColor, const char* lpszFontName,
														const char* lpszDataComID, const int nShowMode, const double dTransparence, const int nBorderType,
														const int nBorderColor, const int nLayer, const int nAlign);
	//清空list_graphics
	int								DeleteAllGraphics();
	//插入数据到list_graphics
	int								InsertRecordToGraphics(const int nID, const int nPageID, const int nType, const int nLineColor, const int nLineStyle, const int nLineWidth, const int nFillColor, const char* lpszPointList,
														const int nFrontPageID, const double dTransparence, const int nIsBorder3D, const int nLayer);
	//清空list_point
	int								DeleteAllPoint();

	////插入opc数据到list_point
	//int								InsertRecordToOPCPoint(const int nType, const char* lpszName, const char* lpszSource, const int nProperty, const char* lpszDescription, const char* lpszUnit,
	//													const double dHigh, const double dHighHigh, const double dLow, const double dLowLow, const char* lpszParam1,const char* lpszParam2,
	//													const char* lpszParam3,const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8,
	//													const char* lpszParam9,const char* lpszParam10);	

	//插入opc数据到list_point
	int								InsertRecordToOPCPoint(const int nIndex, const int nType, const char* lpszName, const char* lpszSource, const int nProperty, const char* lpszDescription, const char* lpszUnit,
		const double dHigh, const double dHighHigh, const double dLow, const double dLowLow, const char* lpszParam1,const char* lpszParam2,
		const char* lpszParam3,const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8,
		const char* lpszParam9,const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,
		const char* lpszParam15,const int nParam16,const int nParam17,const int nParam18);	

	//插入Logic数据到list_point
	int								InsertRecordToLogicPoint(const int nType, const char* lpszName, const char* lpszLogicType, const char* lpszParam1, const char* lpszParam2, const char* lpszParam3,
															const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8,const char* lpszParam9,
															const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,const char* lpszParam15,
															const char* lpszParam16,const char* lpszParam17,const char* lpszParam18,const char* lpszParam19,const char* lpszParam20,const char* lpszParam21,
															const char* lpszParam22,const char* lpszParam23,const char* lpszParam24,const char* lpszParam25,const char* lpszParam26,const char* lpszParam27,
															const char* lpszParam28,const char* lpszParam29,const char* lpszParam30,const char* lpszDescription);
	//插入Alarm数据到list_point
	int								InsertRecordToAlarmPoint(const int nType, const char* lpszName, const char* lpszAlarmType, const char* lpszParam1, const char* lpszParam2, const char* lpszParam3,
															const char* lpszParam4,const char* lpszParam5,const char* lpszParam6,const char* lpszParam7,const char* lpszParam8,const char* lpszParam9,
															const char* lpszParam10,const char* lpszParam11,const char* lpszParam12,const char* lpszParam13,const char* lpszParam14,const char* lpszParam15,
															const char* lpszParam16,const char* lpszParam17,const char* lpszParam18,const char* lpszParam19,const char* lpszParam20,const char* lpszDescription);
	//清空所有有用的表
	int								DeleteAllUsefulTables();
	//清空组的表
	int								DeleteAllGroupData();
	//插入数据到组
	int								InsertRecordToPageGroup(const int nGroupID, const char* lpszGroupName);
	//更新工程配置表
	int								UpdateProjectConfig(const int nID, const char* lpszName, const int nWidth, const int nHeight, const int nVersion = 3, const char* lpszXupTime = "");
	//插入数据到lib_animation
	int								InsertRecordToLibAnimation(const int nID, const char* lpszName, const char* lpszAnimationList);
	//插入数据到list_dashboard
	int								InsertRecordToListDashBoard(const int nPageID, const int nID, const char* lpszBindPointName);
	//清空list_dashboard表的数据
	int								DeleteAllDashBoardData();
public:
	sqlite3*						m_db;
	sqlite3_stmt*					m_stmt;
	bool							m_bOpenState;
	char*							m_lpszLocale;
	char							m_lpszDBPath[MAX_PATH];
};
#endif