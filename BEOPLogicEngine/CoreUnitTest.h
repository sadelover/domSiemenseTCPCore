#pragma once
class CCoreUnitTest
{
public:
	CCoreUnitTest(void);
	~CCoreUnitTest(void);

	bool RunTest();
	bool Test00();
	bool Test01();
	bool Test02();

	bool Test03();

	bool Test04();		//≤‚ ‘DTU∑¢ÀÕ ˝æ›

	bool Test05();		//≤‚ ‘modbus∂¡–¥

	bool Test06();		//≤‚ ‘bacnet

	bool Test08();      //test logic get history

	bool RunCommon();

	bool TestDeleteDBAndRebuild();
};

