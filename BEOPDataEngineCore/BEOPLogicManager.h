#pragma once
#include "DataBaseSetting.h"
#include "DLLObject.h"
#include "ScheduleThread.h"

class CLogicBase;
class CLogicDllThread;

typedef std::vector<std::wstring>  vec_wstr;
typedef CLogicBase* (*pfnLogicBase)();



struct sDllStore
{
	string	nID;
	string	strDllName;
	string	strImportTime;
	string	strAuthor;
	string	strPeriodicity;
	string	strDllContent;
	string	strRunStatus;
	string unitproperty01;
	string	unitproperty02;
	string unitproperty03;
	string	unitproperty04;
	string unitproperty05;

};

struct sPointWatch
{
	int		nID;
	string	strName;
	string	strContent;

	string unitproperty01;
	string	unitproperty02;
	string unitproperty03;
	string	unitproperty04;
	string unitproperty05;
};

struct sParamConfig
{

	string	strVName;
	string	strPName;
	string	strPType;
	string	strVexplain;
	string	strDllName;
	string	nINorOut;

	string unitproperty01;
	string	unitproperty02;
	string unitproperty03;
	string	unitproperty04;
	string unitproperty05;
};



class  CBEOPLogicManager
{
public:
	CBEOPLogicManager(wstring strDBFileName,CWnd *pWnd);
	~CBEOPLogicManager(void);

	bool ThreadExitFinish();

	bool StoreDllToDB(CDLLObject* dllOb, wstring strDllPath);
	bool InsertDLLtoDB(CDLLObject* dllOb,char *pBlock,int nSize);
	bool ReadDLLfromDB(const wstring &s3dbpath,std::vector<CDLLObject*> &dllObList);
	bool UpdateDLLfromDB(const wstring &s3dbpath, CDLLObject *pDllOb);
	bool IfExistDll(const wstring &s3dbpath);
	void SortDllName(std::vector<wstring> &iniDllList,std::vector<vec_wstr> &DllList);
	bool LoadDllDetails();

	CDLLObject * GetDLLObjectByName(wstring dllname);

	bool InitSchedule();
	bool Exit();

	int	GetAllImportDll();

	BOOL CreateFolder(wstring strPath);
	BOOL FolderExist(wstring strPath);
	BOOL FileExist(wstring strFileName);
	BOOL CopyDllFile(wstring selFilePath,wstring DllName);
	BOOL SaveMemToFile( const char* pBlock, const int nSize, wstring szFileName,CDLLObject* pDllOb);
	bool UpdateDllObjectByFile(wstring strDllOriginalName, wstring strFilePath);

	bool	SaveParametertoDB(int nInOrOutput, wstring strDllName,  wstring strVariableName, wstring strLinkDefine, wstring strLinkType);
	bool    GetInputParameterfromDB(wstring strDllName, std::vector<vec_wstr> &inputPara);
	bool    GetOutputParameterfromDB(wstring strDllName, std::vector<vec_wstr> &outputPara);
	bool	CompareInputParameter(std::vector<vec_wstr> &inputPara,std::vector<vec_wstr> &Ini_inputPara);
	bool	CompareOutputParameter(std::vector<vec_wstr> &outputPara,std::vector<vec_wstr> &Ini_outputPara);

	bool    SaveSpanTimetoDB(wstring DllName,double SpanTime);
	bool    SetTimeSpan(CDLLObject *pObject, double fSpanTime);
	std::vector<vec_wstr> SeparateParameter(wstring strLongPatameter);
	wstring LinkParameter(std::vector<vec_wstr> &matrixpara);

	CDLLObject * FindDLLObject(wstring strDllName);
	CLogicDllThread * FindDLLThreadByName(wstring strDllThreadName);
	bool	LoadDllThreadRelationship(CLogicDllThread *pDllThread);

	bool InitRelationships();
	void 			PrintLog(const wstring &strLog,bool bSaveLog = true);
public:
	std::vector<vec_wstr> m_vecCurDllInputParameterList;
	std::vector<vec_wstr> m_vecCurDllOutputParameterList;
	std::vector<wstring> one_para;

	Project::Tools::Mutex	m_lock;

	wstring m_strDBFileName;


	bool m_bExitThread;
	bool m_bThreadExitFinished;

	std::vector<wstring>	 m_vecDllPath_Rep;
	std::vector<wstring>     m_vecSelDllName_Rep; 
	std::vector<wstring>	 m_vecSelDllName;   
	std::vector<CDLLObject*> vecImportDLLList;
	std::vector<CLogicDllThread *> m_vecDllThreadList;
	int						 m_iDeleteItem;
	double m_calcinternal;
	std::vector<wstring>	m_vecCurDllName_iniLoad;

public:
	CBEOPDataAccess *m_pDataAccess;
	CLogicBase            *m_pCLogicBaseOptimize;
	CScheduleThread* m_pSchedule;



	CWnd *m_pWnd;
};

