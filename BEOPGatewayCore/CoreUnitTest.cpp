#include "StdAfx.h"
#include "CoreUnitTest.h"
#include "BeopGatewayCoreWrapper.h"


CCoreUnitTest::CCoreUnitTest(void)
{
}


CCoreUnitTest::~CCoreUnitTest(void)
{
}

bool CCoreUnitTest::RunTest()
{
	_tprintf(L"=====Enter Testing for gatewaycore!=====\n");
	bool bSuccess = true;
	


	//if(!Test03())
	//	return false;

	//if(!TestDeleteDBAndRebuild())
	//	return false;

	//if(!TestDeleteDBAndRebuild())
	//	return false;


	//if(!Test00())
	//	return false;
	//if(!Test01())
	//	return false;
	//if(!Test02())
	//	return false;

	//if(!Test04())
	//	return false;

	/*if(!Test05())
		return false;*/

	/*if(!Test06())
		return false;*/
	return true;
}

bool CCoreUnitTest::RunCommon()
{
	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Run();

	return true;

}

bool CCoreUnitTest::Test01()
{
	bool bSuccess = true;


	return bSuccess;
}

bool CCoreUnitTest::Test02()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST02.s3db";
	wstring strTestDllFilePath = cstrFile + L"\\Test\\PIDMultiIOtest.dll";
	wstring strTestOldDllFilePath = cstrFile + L"\\Test\\PIDMultiIOcommon.dll";


	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Init(strTestFilePath);
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"CT_PID", L"1");
	coreWrapper.m_pDataAccess_Arbiter->InsertLogicParameters(L"CT_PID", L"CT_PID.dll", L"2", L"1", L"", L"", L""); //修改策略周期


	coreWrapper.Reset();

	return true;
}


bool CCoreUnitTest::Test03()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\Wanda_Qingdao_Store_20140820.s3db";


	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Init(strTestFilePath);
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"sitemode", L"0");
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"1Click", L"1");


	coreWrapper.Reset();

	return true;
}
